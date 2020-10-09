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

#ifndef DBCONNECTOR_H
#define DBCONNECTOR_H

#include <QObject>
#include <QMutex>
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#  include <QRecursiveMutex>
#endif
#include <QSqlDatabase>
#include <QString>

class DBConnectResult;

class DBConnector : public QObject {
  Q_OBJECT

 public:
  explicit DBConnector(QObject *parent = nullptr);
  ~DBConnector();

  static int sDefaultLoginTimeout;

  void ReloadSettings();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  QRecursiveMutex *Mutex() { return &mutex_; }
#else
  QMutex *Mutex() { return &mutex_; }
#endif
  void ConnectAsync();
  void ConnectAsync(const QString &driver, const QString &odbc_driver, const QString &server, const bool trusted_connection, const QString &username, const QString &password, const int login_timeout = 0, const bool test = false);
  void CloseAsync();

 signals:
  void Connecting(const QString &odbc_driver, const QString &server);
  void ConnectionSuccess(const QString &odbc_driver, const QString &server);
  void ConnectionFailure(const QString &error);
  void ConnectionClosed();

 public slots:
  DBConnectResult Connect();
  DBConnectResult Connect(const QString &driver, const QString &odbc_driver, const QString &server, const bool trusted_connection, const QString &username, const QString &password, const int login_timeout, const bool test = false);

  void Close();

 private:
  static QMutex sNextConnectionIDMutex;
  static int sNextConnectionID;

  QMutex connect_mutex_;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  QRecursiveMutex mutex_;
#else
  QMutex mutex_;
#endif
  int connection_id_;
  QString driver_;
  QString odbc_driver_;
  QString server_;
  bool trusted_connection_;
  QString username_;
  QString password_;
  int login_timeout_;

};

class DBConnectResult {
 public:
  explicit DBConnectResult(const bool success = false, const QSqlDatabase &db = QSqlDatabase(), const QString &error = QString()) : success_(success), db_(db), error_(error) {}
  bool success_;
  QSqlDatabase db_;
  QString error_;
};

#endif  // DBCONNECTOR_H
