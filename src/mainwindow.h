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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "config.h"

#include <memory.h>

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QMovie>
#include <QItemSelection>

#include "bakfilemodel.h"
#include "bakfileview.h"
#include "bakfileitem.h"
#include "backupresult.h"

class QSortFilterProxyModel;
class QListView;
class QLabel;
class QAbstractButton;
class QShowEvent;
class QCloseEvent;
class Ui_MainWindow;
class Application;
class CommandlineOptions;
class AboutDialog;
class SettingsDialog;
class BakFileFilter;
class Application;
class BackupBackend;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(Application *app, const CommandlineOptions &options, QWidget *parent = nullptr);
  ~MainWindow();

  void CommandlineOptionsReceived(const CommandlineOptions &options);

 private:
  void LoadGeometry();
  void SaveGeometry();
  void EnableFileLoading();
  void DisableFileLoading();

 protected:

 private slots:
  void CommandlineOptionsReceived(const quint32 instance_id, const QByteArray &string_options);
  void ReloadSettings();

  void ScanInProgress();
  void FileLoadProgress(const int value);
  void FileLoadError(const QString &error);
  void FileSelectionChanged(const QItemSelection&, const QItemSelection&);

  void Connecting(const QString &odbc_driver, const QString &server);
  void ConnectionSuccess(const QString &odbc_driver, const QString &server);
  void ConnectionFailure(const QString &error);

  void Reset();
  void MaybeExit();
  void Cancel();
  void Restore();

  void RestoreSuccess();
  void RestoreFailure(const QStringList&);
  void RestoreFinished(const QString &filename, const bool success, const QStringList &errors);
  void RestoreComplete();

 signals:
  void QueueRestores(BakFileItemList files);

 private:
  static const char *kSettingsGroup;
  Ui_MainWindow *ui_;
  Application *app_;
  AboutDialog *about_;
  SettingsDialog *settingsdialog_;

  BakFileModel *bak_file_model_;
  BakFileFilter *bakfile_sort_model_;
  QListView *bak_file_view_;

  std::unique_ptr<QMovie> spinner_;
  BackupBackend *backup_backend_;
  QLabel *statusbar_label_;
  BakFileItemList files_;
  bool initialized_;
  bool file_scan_in_progress_;
  bool files_loaded_;
  bool connected_;
  QString file_load_error_;
  QString connection_status_;
  QList<BackupResult> jobs_finished_;

};

#endif  // MAINWINDOW_H
