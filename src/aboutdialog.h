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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "config.h"

#include <memory>

#include <QDialog>
#include <QMovie>

#include "ui_aboutdialog.h"

class QShowEvent;
class QCloseEvent;
class QMouseEvent;

class AboutDialog : public QDialog {
  Q_OBJECT

 public:
  explicit AboutDialog(QWidget *parent = nullptr);

 protected:
  void showEvent(QShowEvent*);
  void closeEvent(QCloseEvent*);
  void mouseDoubleClickEvent(QMouseEvent *e);

 private:
  void SetPosition();
  void HideDopeFish();

 private slots:
  void Close();

 private:
  Ui::AboutDialog ui_;
  QWidget *mainwindow_;
  std::unique_ptr<QMovie> dopefish_;
};

#endif  // ABOUTDIALOG_H
