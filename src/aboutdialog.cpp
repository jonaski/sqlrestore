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
#include <boost/version.hpp>
#include <magic.h>
#include <zlib.h>

#include <QtGlobal>
#include <QCoreApplication>
#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QString>
#include <QIcon>
#include <QPoint>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QKeySequence>
#include <QMovie>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMouseEvent>

#include "iconloader.h"

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent), dopefish_(new QMovie(":/pictures/dopefish.gif")) {

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

  html += tr("SQL Restore is a SQL batch restore program by %1.").arg("<a href=\"https://jkvinge.net/\">Jonas Kvinge</a>");
  html += "<br />";
  html += tr("The program is free software, released under GPL. The source code is available on GitHub %1").arg("<a href=\"https://github.com/jonaski/sqlrestore\">here</a>");
  html += "<br />";
  html += tr("You should have received a copy of the GNU General Public License along with this program.  If not, see %1").arg("<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>");
  html += "</p>";

  html += "<p>";

  html += tr("If you like this program and can make use of it, consider sponsoring or donating.");
  html += "<br />";
  html += tr("To sponsor me visit %1.").arg("<a href=\"https://github.com/sponsors/jonaski\">my GitHub sponsors profile</a>");
  html += "<br />";
  html += tr("Funding developers through GitHub Sponsors is one more way to contribute to open source projects you appreciate, it helps developers get the resources they need, and recognize contributors working behind the scenes to make open source better for everyone.");
  html += "<br />";
  html += tr("You can also make a one-time payment through %1.").arg("<a href=\"https://paypal.me/jonaskvinge\">paypal.me/jonaskvinge</a>");
  html += "<br />";

  html += "</p>";

  ui_.text->setText(html);

  html.clear();

  html += "<p>";
  html += tr("This program uses %1 %2 which is licensed under %3.").arg("<a href=\"https://www.boost.org/\">Boost C++ Libraries</a>").arg(QString("%1.%2.%3<br />").arg(BOOST_VERSION / 100000).arg(BOOST_VERSION / 100 % 1000).arg(BOOST_VERSION % 100)).arg("<a href=\"http://www.boost.org/users/license.html\">The Boost License</a>");
  html += "</p>";
  ui_.label_boost_text->setText(html);

  html.clear();

  html += "<p>";
  html += tr("This program uses %1 %2 which is licensed under %3.").arg("<a href=\"https://www.qt.io/\">Qt</a>").arg(qVersion()).arg("<a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">The GNU General Public License</a>");
  html += "</p>";
  ui_.label_qt_text->setText(html);

  html.clear();

  html += "<p>";
  html += tr("This program uses %1 %2 which is licensed under %3.").arg("<a href=\"https://www.darwinsys.com/file/\">libmagic (darwinsys file)</a>").arg(magic_version()).arg("<a href=\"https://opensource.org/licenses/BSD-3-Clause\">The BSD license</a>");
  html += "</p>";
  ui_.label_darwinsysfile_text->setText(html);

  html.clear();

  html += "<p>";
  html += tr("This program uses %1 %2 which is licensed under %3.").arg("<a href=\"https://www.zlib.net/\">zlib</a>").arg(ZLIB_VERSION).arg("<a href=\"https://www.zlib.net/zlib_license.html\">The zlib License</a>");
  html += "</p>";
  ui_.label_zlib_text->setText(html);

  html.clear();

  ui_.buttonBox->button(QDialogButtonBox::Close)->setShortcut(QKeySequence::Close);

  connect(ui_.buttonBox, SIGNAL(accepted()), SLOT(Close()));
  connect(ui_.buttonBox, SIGNAL(rejected()), SLOT(Close()));

}

void AboutDialog::showEvent(QShowEvent*) {

  setMinimumHeight(0);
  setMaximumHeight(9000);
  adjustSize();
  // Set fixed height and workaround bottom spacer taking up to much space.
  setFixedHeight(height() - ui_.spacer_bottom->geometry().height() + 15);
  adjustSize();

}

void AboutDialog::closeEvent(QCloseEvent*) {

  HideDopeFish();

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
