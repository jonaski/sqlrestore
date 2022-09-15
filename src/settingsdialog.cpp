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

#include <boost/scope_exit.hpp>

#include <QtGlobal>

#ifdef Q_OS_WIN
#  include <windef.h>
#  include <odbcinst.h>
#endif

#include <QDialog>
#include <QWidget>
#include <QCoreApplication>
#include <QSettings>
#include <QMutex>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QSqlDatabase>
#include <QVariant>
#include <QByteArray>
#include <QStringList>
#include <QChar>
#include <QIODevice>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QFlags>
#include <QPoint>
#include <QComboBox>
#include <QAbstractButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QShowEvent>
#include <QCloseEvent>

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "testserverdialog.h"

#include "dbconnector.h"
#include "utilities.h"
#include "iconloader.h"
#include "logging.h"

const char *SettingsDialog::kSettingsGroup = "Settings";

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent),
      ui_(new Ui_SettingsDialog),
      testserver_(new TestServerDialog(this)),
      db_connector_(new DBConnector(this)),
      test_in_progress_(false),
      connection_open_(false) {

  ui_->setupUi(this);

  //db_connector_->moveToThread(db_connector->thread());

  connect(ui_->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::SaveAndClose);
  connect(ui_->buttonBox, &QDialogButtonBox::clicked, this, &SettingsDialog::Clicked);
  connect(ui_->button_test, &QAbstractButton::clicked, this, &SettingsDialog::TestServer);

  connect(ui_->button_select_local_path, &QAbstractButton::clicked, this, &SettingsDialog::SelectLocalPath);

  connect(db_connector_, &DBConnector::ConnectionSuccess, this, &SettingsDialog::ConnectionSuccess);
  connect(db_connector_, &DBConnector::ConnectionFailure, this, &SettingsDialog::ConnectionFailure);
  connect(db_connector_, &DBConnector::ConnectionClosed, this, &SettingsDialog::ConnectionClosed);

  ui_->button_select_local_path->setIcon(IconLoader::Load("document-open-folder"));

  for (const QString &driver : QSqlDatabase::drivers()) {
    if (!driver.contains("ODBC", Qt::CaseInsensitive)) continue;
    ui_->drivers->addItem(driver, driver);
  }

}

SettingsDialog::~SettingsDialog() {

  //delete db_connector_;
  delete ui_;

}

void SettingsDialog::showEvent(QShowEvent*) {

  setMinimumHeight(0);
  setMaximumHeight(9000);
  adjustSize();
  // Set fixed height and workaround bottom spacer taking up to much space.
  setFixedHeight(height() - ui_->spacer_bottom->geometry().height() + 15);
  adjustSize();

  Load();

}

void SettingsDialog::Clicked(QAbstractButton *button) {
  if (button == ui_->buttonBox->button(QDialogButtonBox::Apply)) Save();
}

void SettingsDialog::SaveAndClose() {

  Save();
  QDialog::accept();

}

void SettingsDialog::Load() {

  QList<QPair<QString, QString>> odbc_drivers;

#ifdef Q_OS_UNIX
  {
    QSettings odbc_inst("/etc/unixODBC/odbcinst.ini", QSettings::IniFormat);
    for (const QString &group : odbc_inst.childGroups()) {
      odbc_inst.beginGroup(group);
      if (odbc_inst.contains("Description")) {
        odbc_drivers << qMakePair(group, odbc_inst.value("Description").toString());
      }
      else {
        odbc_drivers << qMakePair(group, group);
      }
      odbc_inst.endGroup();
    }
  }
#endif

#ifdef Q_OS_WIN
  {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    wchar_t buf[2001];
    wchar_t *pszBuf = buf;
#else
    char buf[2001];
    char *pszBuf = buf;
#endif
    WORD buf_size = 2000;
    WORD cb_buf_out;
    if (SQLGetInstalledDrivers(pszBuf, buf_size, &cb_buf_out)) {
      do {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QString driver = QString::fromWCharArray(pszBuf);
#else
        QString driver = QString(pszBuf);
#endif
        if (driver.toUpper().contains("SQL")) {
          odbc_drivers << qMakePair(driver, driver);
        }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        pszBuf = wcschr(pszBuf, '\0' ) + 1;
#else
        pszBuf = strchr(pszBuf, '\0' ) + 1;
#endif
      }
      while (pszBuf[1] != '\0');
    }
    else {
      qLog(Error) << "SQLGetInstalledDrivers failed";
    }
  }
#endif

  ui_->odbc_drivers->clear();

  if (odbc_drivers.isEmpty()) { // Fallback to hardcoded list.
    QStringList odbc_drivers_fixed = QStringList() << "ODBC Driver 17 for SQL Server"
                                                   << "ODBC Driver 13.1 for SQL Server"
                                                   << "ODBC Driver 13 for SQL Server"
                                                   << "SQL Server Native Client 11.0"
                                                   << "ODBC Driver 11 for SQL Server"
                                                   << "SQL Server Native Client 10.0"
                                                   << "ODBC Driver 10 for SQL Server"
                                                   << "SQL Server"
                                                   << "FreeTDS";
    for (const QString &odbc_driver : odbc_drivers_fixed) {
      ui_->odbc_drivers->addItem(odbc_driver, odbc_driver);
    }
  }
  else {
    for (const QPair<QString, QString> &odbc_driver : odbc_drivers) {
      ui_->odbc_drivers->addItem(odbc_driver.second, odbc_driver.first);
    }
  }

  QSettings s;
  s.beginGroup(kSettingsGroup);

  ComboBoxLoadFromSettings(s, ui_->drivers, "driver", ui_->drivers->count() > 0 ? ui_->drivers->itemData(0).toString() : "");
  ComboBoxLoadFromSettings(s, ui_->odbc_drivers, "odbc_driver", ui_->odbc_drivers->count() > 0 ? ui_->odbc_drivers->itemData(0).toString() : "");
  ui_->server->setText(s.value("server").toString());
  ui_->trusted_connection->setChecked(s.value("trusted_connection", false).toBool());
  ui_->login_timeout->setValue(s.value("login_timeout", DBConnector::sDefaultLoginTimeout).toInt());
  ui_->username->setText(s.value("username").toString());
  QByteArray password = s.value("password").toByteArray();
  if (password.isEmpty()) ui_->password->clear();
  else ui_->password->setText(QString::fromUtf8(QByteArray::fromBase64(password)));
  ui_->remote_path->setText(s.value("remote_path", QDir::toNativeSeparators(QCoreApplication::applicationDirPath())).toString());
  ui_->local_path->setText(s.value("local_path", QDir::toNativeSeparators(QCoreApplication::applicationDirPath())).toString());

  s.endGroup();

}

void SettingsDialog::Save() {

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("driver", ui_->drivers->currentData());
  s.setValue("odbc_driver", ui_->odbc_drivers->currentData());
  s.setValue("server", ui_->server->text());
  s.setValue("trusted_connection", ui_->trusted_connection->isChecked());
  s.setValue("username", ui_->username->text());
  s.setValue("password", QString::fromUtf8(ui_->password->text().toUtf8().toBase64()));
  s.setValue("login_timeout", ui_->login_timeout->value());
  s.setValue("remote_path", ui_->remote_path->text());
  s.setValue("local_path", ui_->local_path->text());
  s.endGroup();

  emit SettingsChanged();

}

void SettingsDialog::TestServer() {

  testserver_->show();
  if (test_in_progress_) {
    return;
  }

  if (ui_->drivers->currentData().toString().isEmpty()) {
    testserver_->Failure(tr("Missing SQL driver"));
    return;
  }
  if (ui_->odbc_drivers->currentData().toString().isEmpty()) {
    testserver_->Failure(tr("Missing ODBC driver"));
    return;
  }
  if (ui_->server->text().isEmpty()) {
    testserver_->Failure(tr("Missing SQL server"));
    return;
  }
  if (!ui_->trusted_connection->isChecked()) {
    if (ui_->username->text().isEmpty()) {
      testserver_->Failure(tr("Missing SQL username"));
      return;
    }
    if (ui_->password->text().isEmpty()) {
      testserver_->Failure(tr("Missing SQL password"));
      return;
    }
  }

  test_in_progress_ = true;
  testserver_->Start(ui_->odbc_drivers->currentText(), ui_->server->text());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  QFuture<DBConnectResult> future = QtConcurrent::run(&SettingsDialog::Connect, this);
#else
  QFuture<DBConnectResult> future = QtConcurrent::run(this, &SettingsDialog::Connect);
#endif
  QFutureWatcher<DBConnectResult> *watcher = new QFutureWatcher<DBConnectResult>(this);
  watcher->setFuture(future);
  connect(watcher, &QFutureWatcherBase::finished, this, &SettingsDialog::ConnectFinished);

}

DBConnectResult SettingsDialog::Connect() {

  QMutexLocker l(db_connector_->Mutex());
  return db_connector_->Connect(ui_->drivers->currentData().toString(), ui_->odbc_drivers->currentData().toString(), ui_->server->text(), ui_->trusted_connection->isChecked(), ui_->username->text(), ui_->password->text(), 4, true);

}

void SettingsDialog::ConnectFinished() {

  QFutureWatcher<DBConnectResult> *watcher = static_cast<QFutureWatcher<DBConnectResult>*>(sender());
  DBConnectResult result = watcher->result();
  watcher->deleteLater();

  if (!test_in_progress_) return;
  test_in_progress_ = false;
  if (result.success_) testserver_->Success();
  else testserver_->Failure(result.error_);

}

void SettingsDialog::ConnectionSuccess(const QString&, const QString&) {

  if (!test_in_progress_) return;

  //connection_open_ = true;
  testserver_->Success();
  test_in_progress_ = false;
  //db_connector_->CloseAsync();

}

void SettingsDialog::ConnectionFailure(const QString &error) {

  if (!test_in_progress_) return;

  test_in_progress_ = false;
  testserver_->Failure(error);

}

void SettingsDialog::ConnectionClosed() {

  if (!test_in_progress_ || !connection_open_) return;

  connection_open_ = false;
  test_in_progress_ = false;

}

void SettingsDialog::ComboBoxLoadFromSettings(const QSettings &s, QComboBox *combobox, const QString &setting, const QString &default_value) {

  const QString &value = s.value(setting, default_value).toString();
  int i = combobox->findData(value);
  if (i == -1) i = combobox->findData(default_value);
  if (i == -1) return;
  combobox->setCurrentIndex(i);

}

void SettingsDialog::SelectLocalPath() {

  QString current_path;
  if (!ui_->local_path->text().isEmpty() && QFile::exists(ui_->local_path->text())) {
    current_path = ui_->local_path->text();
  }
  else {
    current_path = QDir::currentPath();
  }

  QFileDialog dialog;
  dialog.setFileMode(QFileDialog::Directory);
  dialog.setOption(QFileDialog::ShowDirsOnly);
  QString path = dialog.getExistingDirectory(this, tr("Select local backup path"), current_path, QFileDialog::ShowDirsOnly);
  if (path.isEmpty()) return;

  QFileInfo info(path);

  if (!info.exists()) {
    return;
  }
  if (!info.isWritable()) {
    QMessageBox box(QMessageBox::Critical, tr("Path not writable."), tr("%1 is not writable.").arg(path), QMessageBox::Close);
    box.setWindowFlags(box.windowFlags() | Qt::WindowStaysOnTopHint);
    box.exec();
    return;
  }

  // Check local backup path permissions.
  // Check that we can write to a new file.
  // If UNC paths are used, the first check we did will not fail even when lacking permissions.

  QString tmpfile = Utilities::GetRandomStringWithCharsAndNumbers(20) + ".tmp";
  QString tmpfile_local = path;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
  if (tmpfile_local.back() != QDir::separator()) {
#else
  if (tmpfile_local.at(tmpfile_local.size() - 1) != QDir::separator()) {
#endif
    tmpfile_local.append(QDir::separator());
  }
  tmpfile_local.append(tmpfile);

  BOOST_SCOPE_EXIT(&tmpfile_local) {
    if (!tmpfile_local.isEmpty() && QFile::exists(tmpfile_local)) {
      QFile::remove(tmpfile_local);
    }
  } BOOST_SCOPE_EXIT_END

  QFile test_file(tmpfile_local);
  BOOST_SCOPE_EXIT(&test_file) {
    if (test_file.isOpen()) {
      test_file.close();
    }
  } BOOST_SCOPE_EXIT_END
  if (!test_file.open(QIODevice::WriteOnly)) {
    tmpfile_local.clear();
    QMessageBox box(QMessageBox::Critical, tr("Path is not writable."), tr("%1 is not writable.").arg(path), QMessageBox::Close);
    box.setWindowFlags(box.windowFlags() | Qt::WindowStaysOnTopHint);
    box.exec();
    return;
  }
  test_file.close();
  if (test_file.exists()) test_file.remove();
  tmpfile_local.clear();

  ui_->local_path->setText(QDir::toNativeSeparators(path));

}
