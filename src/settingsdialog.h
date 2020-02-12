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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QObject>
#include <QSettings>
#include <QString>

class QWidget;
class QComboBox;
class QAbstractButton;
class QShowEvent;
class QCloseEvent;
class Ui_SettingsDialog;
class DBConnector;
class DBConnectResult;
class TestServerDialog;

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsDialog(QWidget *parent = nullptr);
  ~SettingsDialog();

  static const char *kSettingsGroup;

 protected:
  void showEvent(QShowEvent*);
  void closeEvent(QCloseEvent*);

 private:
  void SetPosition();
  void Load();
  void ComboBoxLoadFromSettings(const QSettings &s, QComboBox *combobox, const QString &setting, const QString &default_value);
  DBConnectResult Connect();
  void LoadGeometry();
  void SaveGeometry();

 signals:
  void SettingsChanged();

 private slots:
  void Clicked(QAbstractButton *button);
  void Save();
  void SaveAndClose();
  void TestServer();
  void ConnectFinished();
  void ConnectionFailure(const QString &error);
  void ConnectionSuccess(const QString &odbc_driver, const QString &server);
  void ConnectionClosed();
  void SelectLocalPath();

 private:
  QWidget *mainwindow_;
  Ui_SettingsDialog *ui_;
  TestServerDialog *testserver_;
  DBConnector *db_connector_;
  bool test_in_progress_;
  bool connection_open_;

};

#endif  // SETTINGSDIALOG_H
