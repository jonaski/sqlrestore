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

#ifndef BAKFILEBACKEND_H
#define BAKFILEBACKEND_H

#include <magic.h>

#include <QObject>
#include <QString>
#include <QMap>

#include "bakfileitem.h"

class QFileSystemWatcher;
class QTimer;

class BakFileBackend : public QObject {
  Q_OBJECT

 public:
  explicit BakFileBackend(QObject *parent = nullptr);
  ~BakFileBackend();

  void ReloadSettingsAsync();
  void CancelScan() { cancel_requested_ = true; }
  void Exit() { exit_requested_ = true; }

 private:
  void LoadMagic();
  QString WriteMagicToTemp();
  BakFileItem *ScanFile(const QString &filename);

 private slots:
  void DirectoryChanged(const QString &path);
  void ReloadSettings();
  void ScanAsync();
  void ScanDelayed();
  void Scan();

 signals:
  void ScanInProgress();
  void LoadProgress(int);
  void LoadError(QString);
  void AddedFiles(BakFileItemList);
  void UpdatedFiles(BakFileItemList);
  void DeletedFiles(BakFileItemList);

 private:
  QFileSystemWatcher *watcher_;
  QTimer *timer_scan_;
  QString local_path_;
  QMap<QString, BakFileItemPtr> files_;
  bool initialized_;
  bool cancel_requested_;
  bool exit_requested_;
  magic_t magic_;

};

#endif  // BAKFILEBACKEND_H
