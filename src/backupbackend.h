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

#ifndef BACKUPBACKEND_H
#define BACKUPBACKEND_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QQueue>
#include <QSqlDatabase>

#include "bakfileitem.h"

class DBConnector;
class ScopedResult;

class BackupBackend : public QObject {
  Q_OBJECT

 public:
  explicit BackupBackend(QObject *parent = nullptr);
  ~BackupBackend();

  void ReloadSettings();

 private:
  QSqlDatabase Connect(ScopedResult *r);
  QString LocalFilePath(const QString &filename);
  QString RemoteFilePath(const QString &filename);
  QString ProductMajorVersionToString(const int product_major_version);
  void FlushQueue();
  void DeleteQueue();
  void UpdateRestoreStatus(const QString &message);
  bool RestoreCheckCancel(ScopedResult *r);

 signals:
  void StartRestoreBackup(BakFileItemPtr fileitem);

  void Error(QString message);

  void RestoreHeaderAll(QString message);
  void RestoreHeaderCurrent(QString message);
  void RestoreStatusCurrent(QString message);
  void RestoreProgressAllValue(int);
  void RestoreProgressAllMax(int);
  void RestoreProgressCurrentValue(int);
  void RestoreSuccess();
  void RestoreFailure(QStringList errors);
  void RestoreFinished(QString filename, bool success, QStringList errors);
  void RestoreComplete();

 private slots:
  void QueueRestores(BakFileItemList bakfilelist);
  void RestoreStarted();
  void RestoreFinished(const bool success);
  void RestoreBackup(BakFileItemPtr fileitem);

 public slots:
  void CancelRestore() { cancel_requested_ = true; }

 private:
  static const int kBufferChunkSize;
  static const int kZipTailSize;
  static const char kZipEndCentralSig[4];
  DBConnector *db_connector_;
  QString local_path_;
  QString remote_path_;
  bool in_progress_;
  QQueue<BakFileItemPtr> queue_;
  int jobs_total_;
  int jobs_complete_;
  int jobs_remaining_;
  int jobs_current_;
  bool cancel_requested_;

};

#endif  // BACKUPBACKEND_H
