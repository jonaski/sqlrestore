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
#include <QDebug>

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent):QDialog(parent) {

  ui_.setupUi(this);
  setWindowFlags(this->windowFlags()|Qt::WindowStaysOnTopHint);
  setWindowTitle(tr("About SQL Restore"));

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

}
