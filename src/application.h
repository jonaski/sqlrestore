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

#ifndef APPLICATION_H
#define APPLICATION_H

#include "config.h"

#include <QObject>
#include <QThread>
#include <QList>

class BakFileBackend;
class DBConnector;
class BackupBackend;

class Application : public QObject {
  Q_OBJECT

 public:
  explicit Application(QObject *parent = nullptr);
  ~Application();

  BakFileBackend *bakfile_backend() { return bakfile_backend_; }
  DBConnector *db_connector() { return db_connector_; }
  BackupBackend *backup_backend() { return backup_backend_; }

  void MoveToNewThread(QObject *object, const QThread::Priority priority);
  void MoveToThread(QObject *object, QThread *thread);

 private:
  QList<QObject*> objects_in_threads_;
  QList<QThread*> threads_;

  BakFileBackend *bakfile_backend_;
  DBConnector *db_connector_;
  BackupBackend *backup_backend_;

};

#endif  // APPLICATION_H
