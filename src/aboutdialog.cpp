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

#include <memory>
#include <boost/version.hpp>
#include <magic.h>

#include <QCoreApplication>
#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QString>
#include <QFlags>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QKeySequence>
#include <QMovie>
#include <QMouseEvent>

#include "iconloader.h"
#include "logging.h"

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

class QShowEvent;
class QCloseEvent;

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent), mainwindow_(parent), dopefish_(new QMovie(":/pictures/dopefish.gif")) {

  ui_.setupUi(this);
  setWindowFlags(this->windowFlags()|Qt::WindowStaysOnTopHint);
  setWindowTitle(tr("About SQL Restore"));

  ui_.label_icon->setPixmap(IconLoader::Load("backup").pixmap(64));

  QFont title_font;
  title_font.setBold(true);
  title_font.setPointSize(title_font.pointSize() + 2);
  ui_.headline->setFont(title_font);
  QString html;
  html += "<p>";
  html += tr("About SQL Restore");
  html += "<br />";
  html += tr("Version %1").arg(QCoreApplication::applicationVersion());
  html += "</p>";
  html += tr("</p>");
  ui_.headline->setText(html);
  html.clear();
  html += "<p>";
  html += tr("SQL Restore is a SQL batch restore application by Jonas Kvinge.");
  html += "<br />";
  html += tr("Use at your own risk!");
  html += "</p>";
  ui_.text->setText(html);

  html.clear();
  html += "<p>";
  html += "<b>";
  html += tr("Technical details") + ":";
  html += "</b><br />";
  html += QString("Boost %1.%2.%3<br />").arg(BOOST_VERSION / 100000).arg(BOOST_VERSION / 100 % 1000).arg(BOOST_VERSION % 100);
  html += QString("Qt %1<br />").arg(qVersion());
  html += QString("Magic %1<br />").arg(magic_version());
  html += "</p>";
  ui_.technical->setText(html);

  adjustSize();
  updateGeometry();
  ui_.buttonBox->button(QDialogButtonBox::Close)->setShortcut(QKeySequence::Close);

  connect(ui_.buttonBox, SIGNAL(accepted()), SLOT(Close()));
  connect(ui_.buttonBox, SIGNAL(rejected()), SLOT(Close()));

}

void AboutDialog::showEvent(QShowEvent*) {
  SetPosition();
}

void AboutDialog::closeEvent(QCloseEvent*) {

  HideDopeFish();

}

void AboutDialog::SetPosition() {
  move(QPoint(mainwindow_->pos().x() + (mainwindow_->width() / 2) - (width() / 2), mainwindow_->pos().y() + (mainwindow_->height() / 2) - (height() / 2)));
}

void AboutDialog::Close() {

  HideDopeFish();
  hide();

}

void AboutDialog::HideDopeFish() {

  if (dopefish_->state() == QMovie::Running) {
    dopefish_->stop();
    ui_.label_icon->clear();
    ui_.label_icon->setPixmap(IconLoader::Load("backup").pixmap(64));
  }

}

void AboutDialog::mouseDoubleClickEvent(QMouseEvent *e) {

  if (e->button() == Qt::RightButton) {
    ui_.label_icon->clear();
    ui_.label_icon->setMovie(dopefish_.get());
    dopefish_->setSpeed(250);
    dopefish_->start();
  }

}
