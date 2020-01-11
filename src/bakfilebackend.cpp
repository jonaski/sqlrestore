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

#include "config.h"

#include <QtGlobal>

#include <unistd.h>
#include <magic.h>
#include <boost/scope_exit.hpp>

#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#  include <quazip/quazip.h>
#  include <quazip/quazipfileinfo.h>
#else
#  include <quazip5/quazip.h>
#  include <quazip5/quazipfileinfo.h>
#endif

#include <QCoreApplication>
#include <QStandardPaths>
#include <QThread>
#include <QFileSystemWatcher>
#include <QList>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QSettings>

#include "logging.h"
#include "bakfilebackend.h"
#include "bakfileitem.h"
#include "settingsdialog.h"

BakFileBackend::BakFileBackend(QObject *parent) :
  QObject(parent),
  watcher_(new QFileSystemWatcher(this)),
  timer_scan_(new QTimer(this)),
  initialized_(false),
  magic_(nullptr)
  {

  timer_scan_->setInterval(5000);
  timer_scan_->setSingleShot(true);

  connect(watcher_, SIGNAL(directoryChanged(QString)), SLOT(DirectoryChanged(QString)));
  connect(timer_scan_, SIGNAL(timeout()), SLOT(Scan()));
  
  LoadMagic(); // This needs to be done in the main thread.

}

BakFileBackend::~BakFileBackend() {
  files_.clear(); // Releases shared_ptr's
  magic_close(magic_);
}

void BakFileBackend::LoadMagic() {

  magic_ = magic_open(0);
  if (!magic_) {
    qLog(Error) << magic_error(magic_);
  }

  // Find the magic file! Point to nullptr first to look in the default directories.
  // On Windows we bundle the magic file in the application directory.
  // For static we load the magic file from Qt resources and dump it in the temp directory.
  QString magic_file;
  char *magic_char = nullptr;
  if (magic_ && magic_check(magic_, magic_char) == -1) {
    magic_file = QDir::toNativeSeparators(QCoreApplication::applicationDirPath()) + QDir::separator().toLatin1() + QString("magic.mgc");
    magic_char = magic_file.toUtf8().data();
    if (magic_check(magic_, magic_char) == -1) {
      magic_file = LoadMagicToTemp();
      magic_char = magic_file.toUtf8().data();
      if (magic_file.isEmpty()) {
        magic_close(magic_);
        magic_ = nullptr;
      }
      else if (magic_check(magic_, magic_char) == -1) {
        qLog(Error) << magic_error(magic_);
        magic_close(magic_);
        magic_ = nullptr;
      }
    }
  }

  if (magic_ && magic_load(magic_, magic_char) == -1) {
    qLog(Error) << magic_error(magic_);
    magic_close(magic_);
    magic_ = nullptr;
  }

  if (magic_) {
    if (magic_char) {
      qLog(Info) << "Using magic from" << magic_file;
    }
    else {
      qLog(Info) << "Using magic from system";
    }
  }

}

QString BakFileBackend::LoadMagicToTemp() {
    
  // Open source magic file
  QFile src_file(":magic.mgc");
  BOOST_SCOPE_EXIT(&src_file) {
    if (src_file.isOpen()) {
      src_file.close();
    }
  } BOOST_SCOPE_EXIT_END

  if (!src_file.open(QIODevice::ReadOnly)) {
    qLog(Error) << "Unable to open" << src_file.fileName() << "for reading" << src_file.errorString();
    return QString();
  }

#ifdef Q_OS_LINUX
  QString magic_dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
#else
  QString magic_dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#endif

  if (!QDir(magic_dir).exists() && !QDir().mkpath(magic_dir)) {
    qLog(Error) << "Unable to create directory" << magic_dir;
    return QString();
  }

  QString magic_file = magic_dir + QDir::separator() + "magic.mgc";
  if (QFile::exists(magic_file)) {
    QFile::remove(magic_file);
  }

  // Open temp magic file
  QFile dst_file(magic_file);
  BOOST_SCOPE_EXIT(&dst_file) {
    if (dst_file.isOpen()) {
      dst_file.close();
    }
  } BOOST_SCOPE_EXIT_END

  if (!dst_file.open(QIODevice::WriteOnly)) {
    src_file.close();
    qLog(Error) << "Unable to open" << dst_file.fileName() << "for writing" << dst_file.errorString();
    return QString();
  }

  if (src_file.bytesAvailable() == 0) {
    return QString();
  }

  while (src_file.bytesAvailable() > 0) {
    QByteArray buf = src_file.read(2048);
    if (buf.isEmpty()) {
      qLog(Error) << "Unable to read from" << src_file.fileName() << src_file.errorString();
      src_file.close();
      dst_file.close();
      return QString();
    }
    qint64 written = dst_file.write(buf.data(), buf.size());
    if (written != buf.size()) {
      qLog(Debug) << "Unable to write to" << dst_file.fileName() << dst_file.errorString();
      src_file.close();
      dst_file.close();
      return QString();
    }
  }

  src_file.close();
  dst_file.flush();
  dst_file.close();

  return magic_file;

}

void BakFileBackend::ReloadSettingsAsync() {
  metaObject()->invokeMethod(this, "ReloadSettings", Qt::QueuedConnection);
}

void BakFileBackend::ReloadSettings() {

  QString prev_local_path = local_path_;

  QSettings s;
  s.beginGroup(SettingsDialog::kSettingsGroup);
  local_path_ = s.value("local_path", QCoreApplication::applicationDirPath()).toString();
  s.endGroup();

  if (local_path_.isEmpty() || local_path_ != prev_local_path) {
    if (!prev_local_path.isEmpty()) watcher_->removePath(prev_local_path);
    if (local_path_.isEmpty()) {
      emit LoadError(tr("Missing local backup path."));
    }
    else {
      if (QDir(local_path_).exists()) {
        watcher_->addPath(local_path_);
      }
      else {
        emit LoadError(tr("Backup path %1 does not exist.").arg(local_path_));
      }
    }
    initialized_ = false;
    ScanAsync();  // Always trigger rescan, even when path is invalid to release current files from the previous path.
  }

  qLog(Debug) << "Watching" << watcher_->directories();

}

void BakFileBackend::DirectoryChanged(const QString &path) {

  if (path != local_path_) return;
  timer_scan_->start();

}

void BakFileBackend::ScanAsync() {

  if (initialized_)
    metaObject()->invokeMethod(this, "ScanDelayed", Qt::QueuedConnection);
  else
    metaObject()->invokeMethod(this, "Scan", Qt::QueuedConnection);

}

void BakFileBackend::ScanDelayed() {
  timer_scan_->start();
}
    
void BakFileBackend::Scan() {

  qLog(Debug) << "Scan" << QThread::currentThread();

  QString error;
  QStringList dir_files;
  if (local_path_.isEmpty()) {
    error = tr("Missing local backup path.");
  }
  else {
    QDir dir(local_path_);
    if (dir.exists()) {
      dir.setFilter(QDir::Files);
      dir_files = dir.entryList();
    }
    else {
      error = tr("Local backup path %1 does not exist.").arg(local_path_);
    }
  }

  QStringList files;
  BakFileItemList added_files;
  BakFileItemList updated_files;
  BakFileItemList deleted_files;

  int progress = 0;
  for (const QString &filename : dir_files) {
    ++progress;
    // Skip any temp file.
    if (filename.toLower().contains(QRegExp(".*\\.tmp$"))) {
      qLog(Error) << "Skipping temp file" << filename;
      emit LoadProgress((int)((float)progress / (float)dir_files.count() * 100.0));
      continue;
    }

    BakFileItemPtr new_fileitem(ScanFile(magic_, filename));

    if (!new_fileitem || !new_fileitem->is_valid()) { // Excludes invalid files.
      emit LoadProgress((int)((float)progress / (float)dir_files.count() * 100.0));
      continue;
    }

    // Actual files that should be kept.
    // If a file is updated and invalidated, this ensures it is removed.
    files << filename;

    if (files_.contains(filename)) {
      BakFileItemPtr old_fileitem = files_[filename];
      if (*new_fileitem != *old_fileitem) {
        qLog(Debug) << filename << "is changed";
        *old_fileitem = *new_fileitem;
        if (initialized_)
          emit UpdatedFiles(BakFileItemList() << old_fileitem);
        else
          updated_files << old_fileitem;
      }
    }
    else {
      qLog(Debug) << filename << "is new";
      files_.insert(filename, new_fileitem);
      if (initialized_)
        emit AddedFiles(BakFileItemList() << new_fileitem);
      else
        added_files << new_fileitem;
    }
    emit LoadProgress((int)((float)progress / (float)dir_files.count() * 100.0));
  }

  QMap<QString, BakFileItemPtr>::iterator i = files_.begin();
  while (i != files_.end()) {
    if (!files.contains(i.key())) {
      qLog(Debug) << i.key() << "is deleted";
      if (initialized_)
        emit DeletedFiles(BakFileItemList() << i.value());
      else
        deleted_files << i.value();
      i = files_.erase(i);
    }
    else {
      ++i;
    }
  }

  if (!deleted_files.isEmpty()) emit DeletedFiles(deleted_files);
  if (!added_files.isEmpty()) emit AddedFiles(added_files);
  if (!updated_files.isEmpty()) emit UpdatedFiles(updated_files);

  initialized_ = true;

  if (error.isEmpty()) {
    emit LoadProgress(100);
  }
  else {
    emit LoadError(error);
  }

}

BakFileItem *BakFileBackend::ScanFile(const magic_t magic, const QString &filename) {

  QString local_filename = local_path_ + QDir::separator() + filename;
  QString mime_data;
  if (magic) mime_data = magic_file(magic, local_filename.toLatin1().data());
  bool magic_check = false;
  bool compressed = false;
  if (mime_data.isEmpty()) {
    if (!filename.toLower().contains(QRegExp(".*\\.bak")) && !filename.toLower().contains(QRegExp(".*\\.ubk$")) && !filename.toLower().contains(QRegExp(".*\\.zip$"))) {
      return nullptr;
    }
  }
  else {
    magic_check = true;
    if (mime_data.contains(QRegExp("^Windows NTbackup archive NT.*: Microsoft SQL Server$"))) {
      compressed = false;
    }
    else if (mime_data.contains(QRegExp("^Zip archive data.*$"))) {
      compressed = true;
    }
    else {
      qLog(Error) << "Skipped file" << filename << "because it is not a SQL Backup or ZIP file" << mime_data;
      return nullptr;
    }
  }

  if (compressed || !magic_check) {
    QuaZip archive(local_filename);
    if (archive.open(QuaZip::mdUnzip)) {
      compressed = true;
      archive.close();
    }
  }

  QFileInfo info(local_filename);

  return new BakFileItem(filename, info.size(), info.lastModified(), compressed, mime_data);
    
}

