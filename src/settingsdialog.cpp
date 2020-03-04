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
      mainwindow_(parent),
      ui_(new Ui_SettingsDialog),
      testserver_(new TestServerDialog(this)),
      db_connector_(new DBConnector(this)),
      test_in_progress_(false),
      connection_open_(false) {

  ui_->setupUi(this);

  //db_connector_->moveToThread(db_connector->thread());

  connect(ui_->buttonBox, SIGNAL(accepted()), SLOT(SaveAndClose()));
  connect(ui_->buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(Clicked(QAbstractButton*)));
  connect(ui_->button_test, SIGNAL(clicked()), SLOT(TestServer()));

  connect(ui_->button_select_local_path, SIGNAL(clicked()), SLOT(SelectLocalPath()));

  connect(db_connector_, SIGNAL(ConnectionSuccess(QString, QString)), SLOT(ConnectionSuccess(QString, QString)));
  connect(db_connector_, SIGNAL(ConnectionFailure(QString)), SLOT(ConnectionFailure(QString)));
  connect(db_connector_, SIGNAL(ConnectionClosed()), SLOT(ConnectionClosed()));

  ui_->button_select_local_path->setIcon(IconLoader::Load("document-open-folder"));

  for (const QString &driver : QSqlDatabase::drivers()) {
    if (!driver.toUpper().contains("ODBC")) continue;
    ui_->drivers->addItem(driver, driver);
  }

  LoadGeometry();

}

SettingsDialog::~SettingsDialog() {

  SaveGeometry();

  //delete db_connector_;
  delete ui_;

}

void SettingsDialog::showEvent(QShowEvent*) {
  LoadGeometry();
  Load();
}

void SettingsDialog::closeEvent(QCloseEvent*) {
  SaveGeometry();
}

void SettingsDialog::LoadGeometry() {

  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (s.contains("geometry")) {
    restoreGeometry(s.value("geometry").toByteArray());
  }
  s.endGroup();

  SetPosition();

}

void SettingsDialog::SaveGeometry() {

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("geometry", saveGeometry());
  s.endGroup();

}

void SettingsDialog::SetPosition() {
  adjustSize();
  move(QPoint(mainwindow_->pos().x() + (mainwindow_->width() / 2) - (width() / 2), mainwindow_->pos().y() + (mainwindow_->height() / 2) - (height() / 2)));
}

void SettingsDialog::Clicked(QAbstractButton *button) {
  if (button == ui_->buttonBox->button(QDialogButtonBox::Apply)) Save();
}

void SettingsDialog::SaveAndClose() {
  SaveGeometry();
  Save();
  QDialog::accept();
}

void SettingsDialog::Load() {

  QStringList odbc_drivers;

#ifdef Q_OS_UNIX
  {
    QSettings odbc_inst("/etc/unixODBC/odbcinst.ini", QSettings::IniFormat);
    odbc_drivers = odbc_inst.childGroups();
  }
#endif

#ifdef Q_OS_WIN
  {
    WCHAR buf[2001];
    WORD buf_size = 2000;
    WORD cb_buf_out;
    WCHAR *pszBuf = buf;
    if (SQLGetInstalledDriversW(buf, buf_size, &cb_buf_out)) {
      do {
        QString driver = QString::fromStdWString(pszBuf);
        if (driver.toUpper().contains("SQL"))
          odbc_drivers << driver;
        pszBuf = wcschr(pszBuf, '\0' ) + 1;
      }
      while (pszBuf[1] != '\0');
    }
    else {
      qLog(Error) << "SQLGetInstalledDriversW failed";
    }
  }
#endif

  if (odbc_drivers.isEmpty()) // Fallback to hardcoded list.
    odbc_drivers = QStringList() << "ODBC Driver 17 for SQL Server"
                                 << "ODBC Driver 13.1 for SQL Server"
                                 << "ODBC Driver 13 for SQL Server"
                                 << "SQL Server Native Client 11.0"
                                 << "ODBC Driver 11 for SQL Server"
                                 << "SQL Server Native Client 10.0"
                                 << "ODBC Driver 10 for SQL Server"
                                 << "SQL Server"
                                 << "FreeTDS";

  ui_->odbc_drivers->clear();
  for (const QString &driver : odbc_drivers) {
    ui_->odbc_drivers->addItem(driver, driver);
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
  s.setValue("driver", ui_->drivers->currentText());
  s.setValue("odbc_driver", ui_->odbc_drivers->currentText());
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

  if (ui_->drivers->currentText().isEmpty()) {
    testserver_->Failure(tr("Missing SQL driver"));
    return;
  }
  if (ui_->odbc_drivers->currentText().isEmpty()) {
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

  QFuture<DBConnectResult> future = QtConcurrent::run(this, &SettingsDialog::Connect);
  QFutureWatcher<DBConnectResult> *watcher = new QFutureWatcher<DBConnectResult>(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(ConnectFinished()));

}

DBConnectResult SettingsDialog::Connect() {

  QMutexLocker l(db_connector_->Mutex());
  return db_connector_->Connect(ui_->drivers->currentText(), ui_->odbc_drivers->currentText(), ui_->server->text(), ui_->trusted_connection->isChecked(), ui_->username->text(), ui_->password->text(), 4, true);

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
  if ( !ui_->local_path->text().isEmpty() && QFileInfo(ui_->local_path->text()).exists() ) {
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
