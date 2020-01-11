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

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <QSqlDriver>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSettings>

#include "logging.h"
#include "dbconnector.h"
#include "scopedresult.h"
#include "settingsdialog.h"

int DBConnector::sNextConnectionID = 1;
QMutex DBConnector::sNextConnectionIDMutex;
int DBConnector::sDefaultLoginTimeout = 5;

DBConnector::DBConnector(QObject *parent) : QObject(parent), mutex_(QMutex::Recursive), trusted_connection_(false), login_timeout_(sDefaultLoginTimeout) {

  {
    QMutexLocker l(&sNextConnectionIDMutex);
    connection_id_ = sNextConnectionID++;
  }

  ReloadSettings();

}

DBConnector::~DBConnector() {}

void DBConnector::ReloadSettings() {

  QSettings s;
  s.beginGroup(SettingsDialog::kSettingsGroup);
  driver_ = s.value("driver").toString();
  odbc_driver_ = s.value("odbc_driver").toString();
  server_ = s.value("server").toString();
  trusted_connection_ = s.value("trusted_connection").toBool();
  username_ = s.value("username").toString();
  QByteArray password = s.value("password").toByteArray();
  if (password.isEmpty()) password_.clear();
  else password_ = QString::fromUtf8(QByteArray::fromBase64(password));
  login_timeout_ = s.value("login_timeout", sDefaultLoginTimeout).toInt();
  s.endGroup();

}

void DBConnector::ConnectAsync() {
  ConnectAsync(driver_, odbc_driver_, server_, trusted_connection_, username_, password_, login_timeout_);
}

void DBConnector::ConnectAsync(const QString &driver, const QString &odbc_driver, const QString &server, const bool trusted_connection, const QString &username, const QString &password, const int login_timeout, const bool test) {
  metaObject()->invokeMethod(this, "Connect", Qt::QueuedConnection, Q_ARG(QString, driver), Q_ARG(QString, odbc_driver), Q_ARG(QString, server), Q_ARG(bool, trusted_connection), Q_ARG(QString, username), Q_ARG(QString, password), Q_ARG(int, login_timeout), Q_ARG(bool, test));

}

DBConnectResult DBConnector::Connect() {
  return Connect(driver_, odbc_driver_, server_, trusted_connection_, username_, password_, login_timeout_, false);
}

DBConnectResult DBConnector::Connect(const QString &driver, const QString &odbc_driver, const QString &server, const bool trusted_connection, const QString &username, const QString &password, const int login_timeout, const bool test) {

  // The signals Connecting, ConnectionFailure and ConnectionSuccess updates the progressbar in all mainwindows.
  // WORKAROUND:
  // Emitting the signals 2 times seems to be the only way to force updating the UI immediately.
  // (adjustSize() and update() even multiple times doesnt help).

  if (driver.isEmpty() || odbc_driver.isEmpty() || server.isEmpty() || (!trusted_connection && username.isEmpty()) || (!trusted_connection && password.isEmpty())) {
    const QString error = tr("Missing SQL server settings");
    qLog(Error) << error;
    emit ConnectionFailure(error);
    emit ConnectionFailure(error);
    return DBConnectResult(false, QSqlDatabase(), error);
  }

  QMutexLocker l(&connect_mutex_);
  const QString connection_id = QString("%1_thread_%2").arg(connection_id_).arg(reinterpret_cast<quint64>(QThread::currentThread()));
  QSqlDatabase db;
  if (QSqlDatabase::connectionNames().contains(connection_id)) {
    db = QSqlDatabase::database(connection_id);
  }
  else {
    db = QSqlDatabase::addDatabase(driver, connection_id);
  }

  QString connection_string = QString("Driver={%1};").arg(odbc_driver);
  connection_string.append("Server=");
  connection_string.append(server);
  connection_string.append(";");
  if (trusted_connection) {
    connection_string.append("Trusted_Connection=Yes;");
  }
  else {
    connection_string.append("Uid=");
    connection_string.append(username);
    connection_string.append(";");
    connection_string.append("Pwd=");
    connection_string.append(password);
    connection_string.append(";");
  }
  connection_string.append("Encrypt=no;");

  if (db.isOpen()) {
    if (db.databaseName() == connection_string) {
      emit ConnectionSuccess(odbc_driver, server);
      emit ConnectionSuccess(odbc_driver, server);
      return DBConnectResult(true, db);
    }
    // If the connection string has changed, close the current connection.
    db.close();
  }

  qLog(Debug) << "Connecting using connection id" << connection_id << "connection string" << connection_string << "in thread" << QThread::currentThread();

  db.setDatabaseName(connection_string);

  if (login_timeout > 0) {
    db.setConnectOptions(QString("SQL_ATTR_LOGIN_TIMEOUT=%1;").arg(login_timeout));
  }

  emit Connecting(odbc_driver, server);
  emit Connecting(odbc_driver, server);

  if (!db.open()) {
    const QString error = db.lastError().text();
    qLog(Error) << error;
    emit ConnectionFailure(error);
    emit ConnectionFailure(error);
    return DBConnectResult(false, db, error);
  }

  qLog(Info) << "Connected to" << server;
  emit ConnectionSuccess(odbc_driver, server);
  emit ConnectionSuccess(odbc_driver, server);

  if (test) {
    db.close();
    QSqlDatabase::removeDatabase(connection_id);
    return DBConnectResult(true);
  }
  else {
    return DBConnectResult(true, db);
  }

}

void DBConnector::CloseAsync() {

  metaObject()->invokeMethod(this, "Close", Qt::QueuedConnection);

}

void DBConnector::Close() {

  QMutexLocker l(&connect_mutex_);

  const QString connection_id = QString("%1_thread_%2").arg(connection_id_).arg(reinterpret_cast<quint64>(QThread::currentThread()));

  // Try to find an existing connection for this thread
  if (QSqlDatabase::connectionNames().contains(connection_id)) {
    {
      QSqlDatabase db = QSqlDatabase::database(connection_id);
      if (db.isOpen()) {
        db.close();
        qLog(Debug) << "Closed database with connection id" << connection_id;
      }
    }
    QSqlDatabase::removeDatabase(connection_id);
  }

  emit ConnectionClosed();

}
