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

#include <memory>

#include <QDialog>
#include <QMovie>
#include <QPixmap>

#include "testserverdialog.h"
#include "ui_testserverdialog.h"

class QCloseEvent;

TestServerDialog::TestServerDialog(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui_TestServerDialog),
  spinner_(new QMovie(":/pictures/spinner.gif")) {

  ui_->setupUi(this);

}

TestServerDialog::~TestServerDialog() { delete ui_; }

void TestServerDialog::Start(const QString &odbc_driver, const QString &server) {

  ui_->text->setText(tr("Connecting to %1 using %2...").arg(server, odbc_driver));
  ui_->icon->clear();
  ui_->icon->setMovie(spinner_.get());
  spinner_->start();

}
void TestServerDialog::Stop() {

  ui_->icon->clear();
  spinner_->stop();

}

void TestServerDialog::Failure(const QString &error) {

  Stop();
  ui_->icon->setPixmap(QPixmap(":/icons/64x64/dialog-error.png"));
  ui_->text->setText(error);

}
  
void TestServerDialog::Success() {

  Stop();
  ui_->icon->setPixmap(QPixmap(":/icons/64x64/dialog-ok-apply.png"));
  ui_->text->setText(tr("Connection successful!"));
  
}


void TestServerDialog::closeEvent(QCloseEvent*) {}
