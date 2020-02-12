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

#include <QMainWindow>
#include <QWidget>
#include <QApplication>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QThread>
#include <QSettings>
#include <QVariant>
#include <QIcon>
#include <QMovie>
#include <QAction>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QStatusBar>
#include <QTextBrowser>

#include "logging.h"
#include "iconloader.h"
#include "application.h"
#include "dbconnector.h"
#include "backupbackend.h"
#include "commandlineoptions.h"
#include "aboutdialog.h"
#include "settingsdialog.h"
#include "bakfilebackend.h"
#include "bakfilemodel.h"
#include "bakfileviewcontainer.h"
#include "bakfileview.h"
#include "bakfilefilter.h"
#include "backupresult.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

const char *MainWindow::kSettingsGroup = "MainWindow";

MainWindow::MainWindow(Application *app, const CommandlineOptions &options, QWidget *parent) :
  QMainWindow(parent),
  ui_(new Ui_MainWindow),
  app_(app),
  about_(new AboutDialog(this)),
  settingsdialog_(new SettingsDialog(this)),
  bak_file_model_(new BakFileModel(this)),
  bakfile_sort_model_(new BakFileFilter(this)),
  spinner_(new QMovie(":/pictures/spinner.gif")),
  backup_backend_(app->backup_backend()),
  statusbar_label_(new QLabel(this)),
  initialized_(false),
  file_scan_in_progress_(false),
  files_loaded_(false),
  connected_(false) {

  qLog(Debug) << "Starting";

  ui_->setupUi(this);

  LoadGeometry();
  CommandlineOptionsReceived(options);

  ui_->action_about_qt->setIcon(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"));
  ui_->action_about->setIcon(IconLoader::Load("backup"));
  ui_->action_exit->setIcon(IconLoader::Load("application-exit"));
  ui_->action_settings->setIcon(IconLoader::Load("configure"));

  bakfile_sort_model_->setSourceModel(bak_file_model_);
  bakfile_sort_model_->setDynamicSortFilter(true);
  bakfile_sort_model_->setSortLocaleAware(true);
  bakfile_sort_model_->setFilterCaseSensitivity(Qt::CaseInsensitive);
  bakfile_sort_model_->sort(BakFileModel::Column_Modified, Qt::AscendingOrder);
  ui_->file_view_container->Init(bak_file_model_, bakfile_sort_model_);
  ui_->file_view_container->view()->setModel(bakfile_sort_model_);
  ui_->file_view_container->view()->Init();

  connect(ui_->action_exit, SIGNAL(triggered()), qApp, SLOT(quit()));
  connect(ui_->action_about, SIGNAL(triggered()), about_, SLOT(show()));
  connect(ui_->action_about_qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  connect(ui_->action_settings, SIGNAL(triggered()), settingsdialog_, SLOT(show()));
  connect(settingsdialog_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));

  connect(app_->db_connector(), SIGNAL(Connecting(QString, QString)), SLOT(Connecting(QString, QString)));
  connect(app_->db_connector(), SIGNAL(ConnectionSuccess(QString, QString)), SLOT(ConnectionSuccess(QString, QString)));
  connect(app_->db_connector(), SIGNAL(ConnectionFailure(QString)), SLOT(ConnectionFailure(QString)));

  connect(app_->bakfile_backend(), SIGNAL(ScanInProgress()), SLOT(ScanInProgress()));
  connect(app_->bakfile_backend(), SIGNAL(LoadProgress(int)), SLOT(FileLoadProgress(int)));
  connect(app_->bakfile_backend(), SIGNAL(LoadError(QString)), SLOT(FileLoadError(QString)));

  connect(app_->bakfile_backend(), SIGNAL(AddedFiles(BakFileItemList)), bak_file_model_, SLOT(AddedFiles(BakFileItemList)));
  connect(app_->bakfile_backend(), SIGNAL(UpdatedFiles(BakFileItemList)), bak_file_model_, SLOT(UpdatedFiles(BakFileItemList)));
  connect(app_->bakfile_backend(), SIGNAL(DeletedFiles(BakFileItemList)), bak_file_model_, SLOT(DeletedFiles(BakFileItemList)));

  connect(this, SIGNAL(QueueRestores(BakFileItemList)), app_->backup_backend(), SLOT(QueueRestores(BakFileItemList)));

  connect(app_->backup_backend(), SIGNAL(RestoreHeaderAll(QString)), ui_->header_all, SLOT(setText(QString)));

  connect(app_->backup_backend(), SIGNAL(RestoreHeaderCurrent(QString)), ui_->header_current, SLOT(setText(QString)));
  connect(app_->backup_backend(), SIGNAL(RestoreStatusCurrent(QString)), ui_->status_current, SLOT(setText(QString)));

  connect(app_->backup_backend(), SIGNAL(RestoreProgressAllValue(int)), ui_->progressbar_all, SLOT(setValue(int)));
  connect(app_->backup_backend(), SIGNAL(RestoreProgressAllMax(int)), ui_->progressbar_all, SLOT(setMaximum(int)));

  connect(app_->backup_backend(), SIGNAL(RestoreProgressCurrentValue(int)), ui_->progressbar_current, SLOT(setValue(int)));
  connect(app_->backup_backend(), SIGNAL(RestoreProgressCurrentMax(int)), ui_->progressbar_current, SLOT(setMaximum(int)));

  connect(app_->backup_backend(), SIGNAL(RestoreSuccess()), this, SLOT(RestoreSuccess()));
  connect(app_->backup_backend(), SIGNAL(RestoreFailure(QStringList)), this, SLOT(RestoreFailure(QStringList)));
  connect(app_->backup_backend(), SIGNAL(RestoreFinished(QString, bool, QStringList)), this, SLOT(RestoreFinished(QString, bool, QStringList)));
  connect(app_->backup_backend(), SIGNAL(RestoreComplete()), this, SLOT(RestoreComplete()), Qt::QueuedConnection);

  connect(ui_->file_view_container->view()->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(FileSelectionChanged(QItemSelection, QItemSelection)));

  statusbar_label_->setWordWrap(true);
  statusBar()->addWidget(statusbar_label_, maximumWidth());

  ui_->stackedWidget->setCurrentWidget(ui_->select_file);
  ui_->label_watcher_spinner->clear();
  ui_->label_watcher_spinner->setMovie(spinner_.get());
  DisableFileLoading();

  ui_->button_restore->setIcon(IconLoader::Load("go-next"));
  ui_->button_exit->setIcon(IconLoader::Load("application-exit"));
  ui_->button_cancel->setIcon(IconLoader::Load("button-cancel"));

  connect(ui_->button_restore, SIGNAL(clicked()), SLOT(Restore()));
  connect(ui_->button_back, SIGNAL(clicked()), SLOT(Reset()));
  connect(ui_->button_cancel, SIGNAL(clicked()), SLOT(Cancel()));
  connect(ui_->button_exit, SIGNAL(clicked()), SLOT(MaybeExit()));
  connect(ui_->button_select_all, SIGNAL(clicked()), ui_->file_view_container->view(), SLOT(SelectAll()));
  connect(ui_->button_unselect_all, SIGNAL(clicked()), ui_->file_view_container->view(), SLOT(UnSelectAll()));

  Reset();
  ReloadSettings();

  qLog(Debug) << "Started" << QThread::currentThread();
  initialized_ = true;

  show();

}

MainWindow::~MainWindow() {
  SaveGeometry();
  delete ui_;
}

void MainWindow::CommandlineOptionsReceived(const quint32, const QByteArray &string_options) {

  CommandlineOptions options;
  options.Load(string_options);

  if (options.is_empty()) {
    raise();
    show();
    activateWindow();
  }
  else
    CommandlineOptionsReceived(options);

}

void MainWindow::CommandlineOptionsReceived(const CommandlineOptions&) {}

void MainWindow::LoadGeometry() {

  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (s.contains("geometry")) {
    restoreGeometry(s.value("geometry").toByteArray());
  }
  s.endGroup();

}

void MainWindow::SaveGeometry() {

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("geometry", saveGeometry());
  s.endGroup();

}

void MainWindow::ReloadSettings() {

  app_->bakfile_backend()->ReloadSettingsAsync();
  app_->db_connector()->ReloadSettings();
  app_->backup_backend()->ReloadSettings();

  app_->db_connector()->ConnectAsync();

  // TODO
  //QFuture<DBConnectResult> future = QtConcurrent::run(this, &MainWindow::Connect);

}

void MainWindow::Reset() {

  files_.clear();
  ui_->textBrowser->clear();
  ui_->stackedWidget->setCurrentWidget(ui_->select_file);
  ui_->button_cancel->hide();
  ui_->button_exit->show();
  ui_->button_restore->show();
  ui_->button_restore->setEnabled(connected_ && !ui_->file_view_container->view()->selectionModel()->selectedRows().isEmpty());
  ui_->button_back->hide();
  ui_->button_select_all->show();
  ui_->button_unselect_all->show();
  ui_->button_select_all->setEnabled(bak_file_model_->rowCount() > ui_->file_view_container->view()->selectionModel()->selection().indexes().count());
  ui_->button_unselect_all->setEnabled(!ui_->file_view_container->view()->selectionModel()->selection().indexes().isEmpty());
  if (file_load_error_.isEmpty()) {
    statusbar_label_->setText(connection_status_);
  }
  else {
    statusbar_label_->setText(file_load_error_);
  }
  if (!ui_->widget_watcher->isHidden()) {
    DisableFileLoading();
  }

}

void MainWindow::EnableFileLoading() {

  if (spinner_->state() == QMovie::NotRunning) spinner_->start();
  ui_->widget_watcher->show();
  ui_->progress_watcher->show();
  ui_->label_watcher_spinner->show();
  if (files_loaded_) {
    ui_->label_watcher_text->setText(tr("Updating..."));
  }
  else {
    ui_->label_watcher_text->setText(tr("Loading..."));
  }
  ui_->label_watcher_text->adjustSize();
  updateGeometry();

}

void MainWindow::DisableFileLoading() {

  if (spinner_->state() == QMovie::Running) spinner_->stop();
  ui_->widget_watcher->hide();
  ui_->progress_watcher->hide();
  ui_->label_watcher_spinner->hide();
  ui_->progress_watcher->setValue(0);
  ui_->label_watcher_text->clear();
  ui_->label_watcher_text->adjustSize();
  updateGeometry();

}

void MainWindow::ScanInProgress() {

  file_scan_in_progress_ = true;
  file_load_error_.clear();
  if (ui_->stackedWidget->currentWidget() == ui_->select_file) {
    statusbar_label_->clear();
    if (ui_->widget_watcher->isHidden()) {
      EnableFileLoading();
    }
  }

}

void MainWindow::FileLoadProgress(const int value) {

  if (value == 100) {
    if (!ui_->widget_watcher->isHidden()) {
      DisableFileLoading();
    }
    files_loaded_ = true;
    statusbar_label_->setText(connection_status_);
    if (ui_->stackedWidget->currentWidget() == ui_->select_file) {
      ui_->button_restore->show();
      ui_->button_restore->setEnabled(connected_ && !ui_->file_view_container->view()->selectionModel()->selectedRows().isEmpty());
      ui_->button_select_all->setEnabled(ui_->file_view_container->view()->selectionModel()->selectedRows().count() < bakfile_sort_model_->rowCount());
      ui_->button_unselect_all->setEnabled(!ui_->file_view_container->view()->selectionModel()->selectedRows().isEmpty());
    }
  }
  else if (ui_->stackedWidget->currentWidget() == ui_->select_file) {
    if (ui_->widget_watcher->isHidden()) {
      EnableFileLoading();
    }
    ui_->progress_watcher->setValue(value);
  }

}

void MainWindow::FileLoadError(const QString &error) {

  file_load_error_ = error;
  if (!ui_->widget_watcher->isHidden()) {
    DisableFileLoading();
  }

  if (ui_->stackedWidget->currentWidget() == ui_->select_file) {
    statusbar_label_->setText(file_load_error_);
  }

  updateGeometry();

}

void MainWindow::FileSelectionChanged(const QItemSelection&, const QItemSelection&) {

  if (ui_->stackedWidget->currentWidget() == ui_->select_file) {
    ui_->button_restore->setEnabled(connected_ && !ui_->file_view_container->view()->selectionModel()->selectedRows().isEmpty());
    ui_->button_select_all->setEnabled(ui_->file_view_container->view()->selectionModel()->selectedRows().count() < bakfile_sort_model_->rowCount());
    ui_->button_unselect_all->setEnabled(!ui_->file_view_container->view()->selectionModel()->selectedRows().isEmpty());
  }

}

void MainWindow::Connecting(const QString&, const QString &server) {

  connected_ = false;

  connection_status_ = tr("Connecting to %1").arg(server);
  if (file_load_error_.isEmpty()) {
    statusbar_label_->setText(connection_status_);
  }

  if (ui_->stackedWidget->currentWidget() == ui_->select_file) {
    ui_->button_restore->setEnabled(false);
  }

}

void MainWindow::ConnectionSuccess(const QString&, const QString &server) {

  connected_ = true;

  connection_status_ = tr("Connected to %1").arg(server);
  if (file_load_error_.isEmpty()) {
    statusbar_label_->setText(connection_status_);
  }

  if (ui_->stackedWidget->currentWidget() == ui_->select_file) {
    ui_->button_restore->setEnabled(!ui_->file_view_container->view()->selectionModel()->selectedRows().isEmpty());
  }

}

void MainWindow::ConnectionFailure(const QString &error) {

  connected_ = false;

  connection_status_ = error;
  if (file_load_error_.isEmpty()) {
    statusbar_label_->setText(connection_status_);
  }

  if (ui_->stackedWidget->currentWidget() == ui_->select_file) {
    ui_->button_restore->setEnabled(false);
  }

}

void MainWindow::MaybeExit() {

  if (ui_->stackedWidget->currentWidget() != ui_->progress) {
    qApp->quit();
  }

}

void MainWindow::Cancel() {

  if (ui_->stackedWidget->currentWidget() == ui_->progress) {
    ui_->status_current->setText(tr("Cancelling restore..."));
    backup_backend_->CancelRestore();
  }
  else {
    Reset();
    hide();
  }

}

void MainWindow::Restore() {

  if (ui_->stackedWidget->currentWidget() == ui_->select_file) {
    files_.clear();
    QModelIndexList selection = ui_->file_view_container->view()->selectionModel()->selectedRows();
    for (const QModelIndex &idx : selection) {
      QModelIndex idx_item = bakfile_sort_model_->mapToSource(idx);
      if (!idx_item.isValid()) continue;
      BakFileItemPtr fileitem = bak_file_model_->item_at(idx_item.row());
      if (!fileitem->is_valid()) continue;
      files_ << fileitem;
    }
    if (!files_.empty()) {
      statusbar_label_->setText(connection_status_);
      if (!ui_->widget_watcher->isHidden()) {
        DisableFileLoading();
      }
      ui_->button_restore->hide();
      ui_->button_exit->hide();
      ui_->button_cancel->show();
      ui_->button_select_all->hide();
      ui_->button_unselect_all->hide();
      if (files_.count() > 1) {
        ui_->header_all->show();
        ui_->progressbar_all->show();
      }
      else {
        ui_->header_all->hide();
        ui_->progressbar_all->hide();
      }
      ui_->stackedWidget->setCurrentWidget(ui_->progress);
      emit QueueRestores(files_);
    }
  }

}

void MainWindow::RestoreSuccess() {

  ui_->progressbar_current->setMaximum(100);
  ui_->progressbar_current->setValue(100);
  ui_->status_current->setText("Success!");

}

void MainWindow::RestoreFailure(const QStringList &errors) {
  Q_UNUSED(errors);
}

void MainWindow::RestoreFinished(const QString &filename, const bool success, const QStringList &errors) {
  jobs_finished_ << BackupResult(filename, success, errors);
}

void MainWindow::RestoreComplete() {

  int failed = 0;
  for (BackupResult result : jobs_finished_) {
    if (result.success()) {
      ui_->textBrowser->append(tr("Restore of %1 was successful.").arg(result.filename()));
    }
    else {
      ui_->textBrowser->append(tr("Restoree of % 1 failed.").arg(result.filename()));
      for (const QString &i : result.errors()) {
        ui_->textBrowser->append(i);
      }
      ++failed;
    }
  }

  if (failed > 1) {
    ui_->textBrowser->append(tr("%1 of %2 restores failed.").arg(failed).arg(jobs_finished_.count()));
  }

  jobs_finished_.clear();
  files_.clear();

  ui_->button_cancel->hide();
  ui_->button_exit->show();
  ui_->button_back->show();
  ui_->stackedWidget->setCurrentWidget(ui_->summary);
  ui_->header_all->clear();
  ui_->header_current->clear();
  ui_->status_current->clear();
  ui_->progressbar_all->setMaximum(100);
  ui_->progressbar_all->setValue(0);
  ui_->progressbar_current->setMaximum(100);
  ui_->progressbar_current->setValue(0);

}
