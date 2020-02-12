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

#include <QObject>
#include <QThread>

#include "application.h"
#include "bakfilebackend.h"
#include "dbconnector.h"
#include "backupbackend.h"

Application::Application(QObject* parent)
  : QObject(parent),
  bakfile_backend_(nullptr),
  db_connector_(nullptr),
  backup_backend_(nullptr)
  {

  bakfile_backend_ = new BakFileBackend();
  MoveToNewThread(bakfile_backend_, QThread::IdlePriority);

  db_connector_ = new DBConnector();
  MoveToNewThread(db_connector_, QThread::IdlePriority);

  backup_backend_ = new BackupBackend();
  MoveToNewThread(backup_backend_, QThread::LowPriority);

}

Application::~Application() {

  for (QObject *object : objects_in_threads_) {
    object->deleteLater();
  }

  for (QThread *thread : threads_) {
    thread->quit();
  }

  for (QThread *thread : threads_) {
    thread->wait();
  }

}

void Application::MoveToNewThread(QObject *object, const QThread::Priority priority) {

  QThread *thread = new QThread(this);

  MoveToThread(object, thread);

  thread->start(priority);
  threads_ << thread;

}

void Application::MoveToThread(QObject *object, QThread *thread) {

  object->setParent(nullptr);
  object->moveToThread(thread);
  objects_in_threads_ << object;

}
