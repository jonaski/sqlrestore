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

#ifndef TESTSERVERDIALOG_H
#define TESTSERVERDIALOG_H

#include <memory>

#include <QDialog>
#include <QObject>
#include <QString>
#include <QMovie>
#include <QLabel>

#include "ui_testserverdialog.h"

class QCloseEvent;

class TestServerDialog : public QDialog {
  Q_OBJECT

 public:
  explicit TestServerDialog(QWidget *parent = nullptr);
  ~TestServerDialog();

  void setText(const QString &text) { ui_->text->setText(text); }
  void Start(const QString &odbc_driver, const QString &server);
  void Stop();
  void Failure(const QString &error);
  void Success();

 protected:
  void closeEvent(QCloseEvent*) override;

 private:
  Ui_TestServerDialog *ui_;
  std::unique_ptr<QMovie> spinner_;

};

#endif  // TESTSERVERDIALOG_H
