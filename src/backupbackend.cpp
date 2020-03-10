/*
   This file is part of SQL Restore
   Copyright 2019, Jonas Kvinge <jonas@jkvinge.net>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <QtGlobal>

#include <memory>
#include <boost/scope_exit.hpp>

#if defined(Q_OS_MACOS)
#  include <quazip/quazip.h>
#  include <quazip/quazipfile.h>
#  include <quazip/quazipfileinfo.h>
#  include <quazip/quacrc32.h>
#else
#  include <quazip5/quazip.h>
#  include <quazip5/quazipfile.h>
#  include <quazip5/quazipfileinfo.h>
#  include <quazip5/quacrc32.h>
#endif

#include <QObject>
#include <QCoreApplication>
#include <QMutex>
#include <QMap>
#include <QQueue>
#include <QVariant>
#include <QByteArray>
#include <QString>
#include <QRegExp>
#include <QChar>
#include <QIODevice>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStorageInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSettings>

#include "logging.h"
#include "utilities.h"
#include "dbconnector.h"
#include "backupbackend.h"
#include "scopedresult.h"
#include "settingsdialog.h"
#include "bakfileitem.h"

using namespace Utilities;

const int BackupBackend::kBufferChunkSize = 8192;
const int BackupBackend::kZipTailSize = 8192;
const char BackupBackend::kZipEndCentralSig[4] = { 0x50, 0x4B, 0x05, 0x06 };

BackupBackend::BackupBackend(QObject *parent) :
  QObject(parent),
  db_connector_(new DBConnector(this)),
  in_progress_(false),
  jobs_total_(0),
  jobs_complete_(0),
  jobs_remaining_(0),
  jobs_current_(0),
  cancel_requested_(false) {

  connect(this, SIGNAL(StartRestoreBackup(BakFileItemPtr)), SLOT(RestoreBackup(BakFileItemPtr)));

}

BackupBackend::~BackupBackend() {}

void BackupBackend::ReloadSettings() {

  QSettings s;
  s.beginGroup(SettingsDialog::kSettingsGroup);
  remote_path_ = s.value("remote_path", QDir::toNativeSeparators(QCoreApplication::applicationDirPath())).toString();
  local_path_ = s.value("local_path", QDir::toNativeSeparators(QCoreApplication::applicationDirPath())).toString();
  s.endGroup();

  db_connector_->ReloadSettings();

}

QString BackupBackend::LocalFilePath(const QString &filename) {

  QString ret = local_path_;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
  if (ret.back() != QDir::separator()) {
#else
  if (ret.isEmpty() || ret.at(ret.size() - 1) != QDir::separator()) {
#endif
    ret.append(QDir::separator());
  }
  ret.append(filename);
  return ret;

}

QString BackupBackend::RemoteFilePath(const QString &filename) {

  QString ret = remote_path_;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
  QChar lastchar = ret.back();
#else
  QChar lastchar = ret.at(ret.size() - 1);
#endif
  if (lastchar != QDir::separator() && lastchar != '/' && lastchar != '\\') {
    if (ret.contains('/')) ret.append('/');
    else if (ret.contains('\\')) ret.append('\\');
    else ret.append(QDir::separator());
  }
  ret.append(filename);
  return ret;

}

QString BackupBackend::ProductMajorVersionToString(const int product_major_version) {

  switch (product_major_version) {
    case 9: return "SQL Server 2005";
    case 10: return "SQL Server 2008";
    case 11: return "SQL Server 2012";
    case 12: return "SQL Server 2014";
    case 13: return "SQL Server 2016";
    case 14: return "SQL Server 2017";
    default: return "Unknown";
  }

}

QSqlDatabase BackupBackend::Connect(ScopedResult *r) {

  // Connect to the SQL server
  QMutexLocker l(db_connector_->Mutex());
  DBConnectResult result = db_connector_->Connect();
  if (!result.db_.isOpen()) {
    r->failure(result.error_);
    return QSqlDatabase();
  }
  return result.db_;

}

void BackupBackend::QueueRestores(BakFileItemList files) {

  jobs_total_ = 0;
  jobs_complete_ = 0;
  jobs_current_ = 0;
  queue_.clear();

  for (BakFileItemPtr bakfile : files) {
    if (!bakfile->is_valid()) continue;
    ++jobs_total_;
    queue_.enqueue(bakfile);
  }

  if (jobs_total_ > 1) {
    emit RestoreProgressAllMax(jobs_total_);
  }

  FlushQueue();

}

void BackupBackend::FlushQueue() {

  if (!queue_.isEmpty() && jobs_remaining_ == 0) {
    emit StartRestoreBackup(queue_.dequeue());
  }

}

void BackupBackend::DeleteQueue() {
  queue_.clear();
}

void BackupBackend::RestoreBackup(BakFileItemPtr fileitem) {

  RestoreStarted();

  ScopedResult r(fileitem->filename());
  connect(&r, SIGNAL(Status(QString)), SIGNAL(RestoreStatusCurrent(QString)));
  connect(&r, SIGNAL(Success()), SIGNAL(RestoreSuccess()));
  connect(&r, SIGNAL(Failure(QStringList)), SIGNAL(RestoreFailure(QStringList)));
  connect(&r, SIGNAL(Finished(QString, bool, QStringList)), SIGNAL(RestoreFinished(QString, bool, QStringList)));
  connect(&r, SIGNAL(Finished(bool)), SLOT(RestoreFinished(bool)), Qt::QueuedConnection);

  const QString header = tr("Restoring %1").arg(fileitem->filename());
  emit RestoreHeaderCurrent(header);
  UpdateRestoreStatus(header);

  emit RestoreProgressCurrentValue(0);

  if (RestoreCheckCancel(&r)) return;

  if (local_path_.isEmpty() || remote_path_.isEmpty()) {
    r.failure(tr("Missing backup paths."));
    return;
  }

  // Check file permissions
  {
    QFileInfo info(local_path_);
    if (!info.exists()) {
      r.failure(tr("Local backup path \"%1\" does not exist.").arg(local_path_));
      return;
    }
    if (!info.isWritable()) {
      r.failure(tr("Local backup path \"%1\" is not writable.").arg(local_path_));
      return;
    }
  }

  // Setup temp file.
  QString tmpfile_local;
  BOOST_SCOPE_EXIT(&tmpfile_local) {
    if (!tmpfile_local.isEmpty() && QFile::exists(tmpfile_local)) {
      QFile::remove(tmpfile_local);
    }
  }
  BOOST_SCOPE_EXIT_END

  // Make sure the random number generator is seeded in this thread.
  Seed();

  QString tmpfile = GetRandomStringWithCharsAndNumbers(20) + ".tmp";
  tmpfile_local = LocalFilePath(tmpfile);

  // Check local backup path permissions
  // Check that we can write to a new file.
  // If UNC paths are used, the first check we did will not fail even when lacking permissions.
  {
    QFile test_file(tmpfile_local);
    BOOST_SCOPE_EXIT(&test_file) {
      if (test_file.isOpen()) {
        test_file.close();
      }
    } BOOST_SCOPE_EXIT_END
    if (!test_file.open(QIODevice::WriteOnly)) {
      r.failure(tr("Local backup path \"%1\" is not writable.").arg(local_path_));
      return;
    }
    test_file.close();
    if (test_file.exists()) test_file.remove();
  }

  if (RestoreCheckCancel(&r)) return;

  // Check Connection to the SQL server before uncompressing file.
  UpdateRestoreStatus(tr("Connecting to SQL server."));
  QSqlDatabase db = Connect(&r);
  if (!db.isOpen()) {
    return;
  }

  if (RestoreCheckCancel(&r)) return;

  QString bakfile;

  if (fileitem->compressed()) {  // Unzip file if compressed

    QString zipfile = LocalFilePath(fileitem->filename());

    { // Look for the end of central directory signature
      // This will stop wasting time reading and CRC checking many of the incomplete/corrupt files.

      QFile src_file(zipfile);
      if (src_file.size() > (kBufferChunkSize*2)) {
        BOOST_SCOPE_EXIT(&src_file) {
          if (src_file.isOpen()) {
            src_file.close();
          }
        } BOOST_SCOPE_EXIT_END
        UpdateRestoreStatus(tr("Looking for end-of-central-directory signature in ZIP archive."));
        if (!src_file.open(QIODevice::ReadOnly)) {
          r.failure(tr("Unable to open ZIP archive \"%1\".: Error %2").arg(zipfile).arg(src_file.errorString()));
          return;
        }
        if (src_file.size() > kZipTailSize) {
          src_file.seek(src_file.size() - kZipTailSize);
        }
        QByteArray buf;
        while (src_file.bytesAvailable() > 0) {
          QByteArray buf_tmp = src_file.read(kBufferChunkSize);
          if (buf_tmp.isEmpty()) {
            r.failure(tr("Unable to read from ZIP archive \"%1\".: %2").arg(zipfile).arg(src_file.errorString()));
            src_file.close();
            return;
          }
          buf.append(buf_tmp);
        }
        src_file.close();

        char *inbuf = buf.data();
        char *inptr;
        bool found = false;
        for (inptr = inbuf + (buf.size() - 22) ; inptr >= inbuf ; --inptr) {
          if (*inptr == (unsigned char)0x50 && memcmp((char*) inptr, kZipEndCentralSig, 4) == 0) {
            found = true;
            break;
          }
        }
        if (!found) {
          r.failure(tr("End-of-central-directory signature not found in ZIP archive \"%1\". File is incomplete or corrupt.").arg(zipfile));
          return;
        }
      }
    }

   if (RestoreCheckCancel(&r)) return;

    UpdateRestoreStatus(tr("Uncompressing ZIP archive \"%1\"").arg(fileitem->filename()));

    QuaZip archive(zipfile);
    BOOST_SCOPE_EXIT(&archive) {
      if (archive.isOpen()) {
        archive.close();
      }
    } BOOST_SCOPE_EXIT_END
    if (!archive.open(QuaZip::mdUnzip)) {
      r.failure(tr("Unable to open ZIP archive \"%1\".: Error %2").arg(zipfile).arg(archive.getZipError()));
      return;
    }

    if (!archive.getFileNameList().isEmpty() && archive.goToFirstFile()) {

      // Open ZIP File
      QString currentfile = archive.getCurrentFileName();
      QuaZipFile zfile(archive.getZipName(), currentfile);
      BOOST_SCOPE_EXIT(&zfile) {
        if (zfile.isOpen()) {
          zfile.close();
        }
      } BOOST_SCOPE_EXIT_END
      if (!zfile.open(QIODevice::ReadOnly)) {
        r.failure(tr("Unable to open file \"%1\" in ZIP archive \"%2\" for reading.").arg(currentfile).arg(zipfile));
        archive.close();
        return;
      }

      QuaZipFileInfo64 zip_info;
      if (!zfile.getFileInfo(&zip_info)) {
       r.failure(tr("Unable to get file info for \"%1\" from ZIP archive \"%2\".").arg(currentfile).arg(zipfile));
        zfile.close();
        archive.close();
       return;
      }

      {
        // Check disk space
        QStorageInfo info(local_path_); // Doesn't work for UNC paths.
        if (info.isValid()) {
          qint64 disk_space_free = info.bytesAvailable();
          if (zfile.size() > disk_space_free) {
            zfile.close();
            archive.close();
            r.failure(tr("Not enough disk space on \"%1\", %2 is available, but %3 is needed to unzip %4.").arg(local_path_).arg(PrettySize(disk_space_free)).arg(PrettySize(zfile.size())).arg(currentfile));
            return;
          }
        }
      }

      // Open temp File
      QFile dst_file(tmpfile_local);
      BOOST_SCOPE_EXIT(&dst_file) {
        if (dst_file.isOpen()) {
          dst_file.close();
        }
      } BOOST_SCOPE_EXIT_END
      if (!dst_file.open(QIODevice::WriteOnly)) {
        zfile.close();
        archive.close();
        r.failure(tr("Unable to open temporary file \"%1\" for writing.: %2").arg(tmpfile_local).arg(dst_file.errorString()));
        return;
      }

      emit RestoreProgressCurrentValue(0);

      qint64 total_size_written = 0;
      QuaCrc32 checksum;
      while (zfile.bytesAvailable() > 0) {
        if (RestoreCheckCancel(&r)) {
          zfile.close();
          dst_file.close();
          archive.close();
          return;
        }
        QByteArray buf = zfile.read(kBufferChunkSize);
        if (buf.isEmpty()) {
          r.failure(tr("Unable to read file \"%1\" in ZIP archive \"%2\" (File possibly corrupt).: %3.").arg(currentfile).arg(zipfile).arg(zfile.errorString()));
          zfile.close();
          dst_file.close();
          archive.close();
          return;
        }
        checksum.update(buf);
        qint64 written = dst_file.write(buf.data(), buf.size());
        if (written != buf.size()) {
          r.failure(tr("Unable to write to temporary file \"%1\".: %2").arg(tmpfile_local).arg(dst_file.errorString()));
          zfile.close();
          dst_file.close();
          archive.close();
          return;
        }
        total_size_written += written;
        emit RestoreProgressCurrentValue((int)((float)total_size_written / (float)zfile.size() * 100.0));
      }
      dst_file.flush();
      dst_file.close();
      if (total_size_written < zfile.size()) {
        zfile.close();
        archive.close();
        r.failure(tr("Unexpected end of file while reading file \"%1\" in ZIP archive \"%2\". File is corrupt.").arg(currentfile).arg(zipfile));
        return;
      }
      if (checksum.value() != zip_info.crc) {
        r.failure(tr("CRC checksum failed for file \"%1\" in ZIP archive \"%2\". File is corrupt.").arg(currentfile).arg(zipfile));
        zfile.close();
        archive.close();
        return;
      }
      zfile.close();
    }
    else {
      r.failure(tr("Backup ZIP archive \"%1\" has no files.").arg(zipfile));
      archive.close();
      return;
    }
    archive.close();
    bakfile = RemoteFilePath(tmpfile);
  }
  else {
    bakfile = RemoteFilePath(fileitem->filename());
    tmpfile_local.clear();
  }

  emit RestoreProgressCurrentValue(0);

  if (RestoreCheckCancel(&r)) return;

  // Connect to the SQL server again
  UpdateRestoreStatus(tr("Connecting to SQL server."));
  db = Connect(&r);
  if (!db.isOpen()) {
    return;
  }

  if (RestoreCheckCancel(&r)) return;

  // Get server version
  int server_version = 0;
  UpdateRestoreStatus(tr("Getting SQL server version"));
  {
    QSqlQuery query(db);
    query.prepare("SELECT SERVERPROPERTY('ProductMajorVersion')");
    if (!query.exec()) {
      r.failure(QStringList() << query.lastError().text() << query.lastQuery());
      return;
    }
    while (query.next() && query.record().count() > 0) {
      server_version = QByteArray::fromHex(query.value(0).toByteArray()).toHex().toInt();
    }
  }

  if (RestoreCheckCancel(&r)) return;

  // Get header information from backup
  UpdateRestoreStatus(tr("Getting header information from %1").arg(bakfile));
  QMap<int, QString> dbnames;
  int db_version_highest = 0;
  bool backup_incorrect = false;
  {
    QSqlQuery query(db);
    query.prepare(QString("RESTORE HEADERONLY FROM DISK = '%1'").arg(bakfile));
    if (!query.exec()) {
      r.failure(QStringList() << query.lastError().text() << query.lastQuery());
      return;
    }
    while (query.next() && query.record().count() > 0) {
      int type = query.value("BackupType").toInt();
      if (type != 1) backup_incorrect = true;
      int db_position = query.value("Position").toInt();
      if (dbnames.contains(db_position)) {
        r.failure(tr("Backup file \"%1\" contains multiple databases in position %2.").arg(bakfile).arg(db_position));
        return;
      }
      QString db_name = query.value("DatabaseName").toString();
      int db_version = query.value("SoftwareVersionMajor").toInt();
      if (db_version > db_version_highest) db_version_highest = db_version;
      if (!db_name.isEmpty()) {
        dbnames.insert(db_position, db_name);
      }
    }
  }

  if (backup_incorrect) {
    r.failure(tr("SQL Backup \"%1\" is not a normal full database backup.").arg(bakfile));
    return;
  }

  if (db_version_highest <= 0 || dbnames.isEmpty()) {
    r.failure(tr("Unable to read SQL Backup \"%1\", it is most likely created on a newer SQL server.").arg(bakfile));
    return;
  }

  if (server_version != 0 && db_version_highest > server_version) {
    r.failure(tr("SQL Backup \"%1\" was created on %2 (%3), which is newer than this server, this server is %4 (%5). You need yo upgrade your SQL server.").arg(fileitem->filename()).arg(ProductMajorVersionToString(db_version_highest)).arg(db_version_highest).arg(ProductMajorVersionToString(server_version)).arg(server_version));
    return;
  }

  if (RestoreCheckCancel(&r)) return;

  // Verify bak file.

  UpdateRestoreStatus(tr("Verifying backup file \"%1\"").arg(bakfile));
  {
    QSqlQuery query(db);
    query.prepare("RESTORE VERIFYONLY FROM DISK = :bakfile");
    query.bindValue(":bakfile", bakfile);
    if (!query.exec()) {
      r.failure(QStringList() << query.lastError().text() << query.lastQuery());
      return;
    }
  }

  if (RestoreCheckCancel(&r)) return;

  // Get data and log paths

  QString datapath;
  QString logpath;

  UpdateRestoreStatus(tr("Getting DATA and LOG path for SQL server"));
  {
    QSqlQuery query(db);
    // FIXME: Use InstanceDefaultDataPath:
    //query.prepare("SELECT serverproperty('InstanceDefaultDataPath')");
    //query.prepare("SELECT physical_name from sys.master_files where name = :dbname");
    query.prepare("SELECT d.name DatabaseName, f.physical_name AS PhysicalName, f.type_desc TypeofFile FROM sys.master_files f INNER JOIN sys.databases d ON d.database_id = f.database_id WHERE d.name = :dbname");
    query.bindValue(":dbname", "master");
    if (!query.exec()) {
      r.failure(QStringList() << query.lastError().text() << query.lastQuery());
      return;
    }
    while (query.next() && query.record().count() > 0) {
      const QString type_of_file = query.value("TypeofFile").toString().toUpper();
      const QString physical_name = query.value("PhysicalName").toString();
      QString *p = nullptr;
      if (type_of_file == "ROWS") {
        p = &datapath;
      }
      else if (type_of_file == "LOG") {
        p = &logpath;
      }
      *p = query.value("PhysicalName").toString();
      int pos = p->lastIndexOf(QChar('/'));
      if (pos > 0) *p = p->left(pos);
      else {
        pos = p->lastIndexOf(QChar('\\'));
        if (pos > 0) *p = p->left(pos);
      }
    }
  }
  if (datapath.isEmpty() || logpath.isEmpty()) {
    r.failure(tr("Unable to get DATA or LOG path for SQL server."));
    return;
  }

  if (RestoreCheckCancel(&r)) return;

  emit RestoreProgressCurrentValue(0);

  if (RestoreCheckCancel(&r)) return;

  int progress = 0;
  for (QMap<int, QString>::const_iterator i = dbnames.constBegin() ; i != dbnames.constEnd() ; ++i) {

    ++progress;

    int dbposition = i.key();
    const QString &dbname = i.value();

    if (RestoreCheckCancel(&r)) return;

    const QString logname = dbname + "_log";
    QString db_datapath = datapath;
    QString db_logpath = logpath;
    QString old_logical_dbname = dbname;
    QString old_logical_logname = dbname + "_log";

    UpdateRestoreStatus(tr("Getting logical names for database \"%1\"").arg(dbname));
    {
      QSqlQuery query(db);
      query.prepare("RESTORE FILELISTONLY FROM DISK = :bakfile WITH FILE = :dbposition");
      query.bindValue(":bakfile", bakfile);
      query.bindValue(":dbposition", dbposition);
      if (!query.exec()) {
        r.failure(QStringList() << query.lastError().text() << query.lastQuery());
        return;
      }
      while (query.next()) {
        QString type = query.value("Type").toString().toUpper();
        if (type == "D") {
          old_logical_dbname = query.value("LogicalName").toString();
        }
        if (type == "L") {
          old_logical_logname = query.value("LogicalName").toString();
        }
      }
    }

    bool exists = false;
    {
      UpdateRestoreStatus(tr("Checking if database \"%1\" exists.").arg(dbname));
      QSqlQuery query(db);
      query.prepare("SELECT name, state_desc FROM sys.databases WHERE name = :dbname");
      query.bindValue(":dbname", dbname);
      if (!query.exec()) {
        r.failure(QStringList() << query.lastError().text() << query.lastQuery());
        return;
      }
      while (query.next()) {
        QString state = query.value("state_desc").toString();
        if (state != "RESTORING") exists = true;
      }
    }

    if (exists) {
      UpdateRestoreStatus(tr("Setting database \"%1\" to single user.").arg(dbname));
      QSqlQuery query(db);
      query.prepare("ALTER DATABASE " + dbname + " SET SINGLE_USER WITH ROLLBACK IMMEDIATE");
      if (!query.exec()) {
        r.failure(QStringList() << query.lastError().text() << query.lastQuery());
        return;
      }
    }

    if (exists) {
      UpdateRestoreStatus(tr("Getting system filenames for database \"%1\".").arg(dbname));
      QSqlQuery query(db);
      query.prepare("SELECT filename FROM " + dbname + "..sysfiles");
      if (!query.exec()) {
        r.failure(QStringList() << query.lastError().text() << query.lastQuery());
        return;
      }
      while (query.next()) {
        QString s = query.value(0).toString();
        QString *p = nullptr;
        if (s.toLower().contains(QRegExp(".*\\.mdf"))) {
          db_datapath = s;
          p = &db_datapath;
        }
        else if (s.toLower().contains(QRegExp(".*\\.ldf"))) {
          db_logpath = s;
          p = &db_logpath;
        }
        if (p) {
          int pos = p->lastIndexOf(QChar('/'));
          if (pos > 0) *p = p->left(pos);
          else {
            pos = p->lastIndexOf(QChar('\\'));
            if (pos > 0) *p = p->left(pos);
            else {
              p->clear();
            }
          }
        }
      }
    }
    if (db_datapath.isEmpty()) db_datapath = datapath;
    if (db_logpath.isEmpty()) db_logpath = logpath;

    const QString datafile = db_datapath + "\\" + dbname + ".mdf";
    const QString logfile = db_logpath + "\\" + dbname + "_log.ldf";
    {
      UpdateRestoreStatus(tr("Restoring database \"%1\".").arg(dbname));
      QSqlQuery query(db);
      query.prepare("RESTORE DATABASE :dbname FROM DISK = :bakfile WITH FILE = :dbposition, MOVE :old_logical_dbname TO :datafile, MOVE :old_logical_logname TO :logfile, NOUNLOAD, REPLACE");
      query.bindValue(":dbname", dbname);
      query.bindValue(":bakfile", bakfile);
      query.bindValue(":dbposition", dbposition);
      query.bindValue(":old_logical_dbname", old_logical_dbname);
      query.bindValue(":old_logical_logname", old_logical_logname);
      query.bindValue(":datafile", datafile);
      query.bindValue(":logfile", logfile);
      if (!query.exec()) {
        r.failure(QStringList() << query.lastError().text() << query.lastQuery());
        return;
      }
    }

    // Rename logical names to reflect new client numbers.
    if (dbname != old_logical_dbname) {
      UpdateRestoreStatus(tr("Setting logical names for database \"%1\".").arg(dbname));
      for (int i = 0 ; i < 3 ; ++i) {
        QString new_logical_dbname;
        if (i == 0) new_logical_dbname = dbname;
        else new_logical_dbname = dbname + QString("_").repeated(i);
        QSqlQuery query(db);
        query.prepare("ALTER DATABASE " + dbname + " MODIFY FILE (NAME = " + old_logical_dbname + ", NEWNAME = " +new_logical_dbname +")");
        if (query.exec()) {
          break;
        }
      }
    }
    if (logname != old_logical_logname) {
      UpdateRestoreStatus(tr("Setting logical names for database \"%1\".").arg(dbname));
      for (int i = 0 ; i < 3 ; ++i) {
        QString new_logical_logname;
        if (i == 0) new_logical_logname = logname;
        else new_logical_logname = logname + QString("_").repeated(i);
        QSqlQuery query(db);
        query.prepare("ALTER DATABASE " + dbname + " MODIFY FILE (NAME = " + old_logical_logname + ", NEWNAME = " + new_logical_logname +")");
        if (query.exec()) {
          break;
        }
      }
    }

    {
      UpdateRestoreStatus(tr("Setting database \"%1\" to multi user.").arg(dbname));
      QSqlQuery query(db);
      query.prepare("ALTER DATABASE " + dbname + " SET MULTI_USER");
      if (!query.exec()) {
        r.failure(QStringList() << query.lastError().text() << query.lastQuery());
        return;
      }
    }

    emit RestoreProgressCurrentValue((int)((float)progress / (float)dbnames.count() * 100.0));

  }

  UpdateRestoreStatus(tr("Success"));
  emit RestoreProgressCurrentValue(100);

  r.success();

}

void BackupBackend::RestoreStarted() {

  in_progress_ = true;
  ++jobs_remaining_;
  ++jobs_current_;

  if (jobs_total_ > 1) {
    emit RestoreHeaderAll(tr("Restoring backup %1 of %2.").arg(jobs_current_).arg(jobs_total_));
  }

}

void BackupBackend::RestoreFinished(const bool) {

  --jobs_remaining_;
  ++jobs_complete_;

  if (jobs_total_ > 1) {
    emit RestoreProgressAllValue(jobs_complete_);
  }

  if (jobs_remaining_ == 0 && queue_.isEmpty()) {
    in_progress_ = false;
    cancel_requested_ = false;
    emit RestoreComplete();
  }
  else {
    FlushQueue();
  }

}

bool BackupBackend::RestoreCheckCancel(ScopedResult *r) {

  if (cancel_requested_) {
    r->failure(tr("Restore cancelled."));
    return true;
  }
  return false;

}

void BackupBackend::UpdateRestoreStatus(const QString &message) {

  qLog(Debug) << message;
  emit RestoreStatusCurrent(message);

}
