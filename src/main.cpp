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
#include "version.h"

#include <QtGlobal>

#include <assert.h>

#ifdef GLIB_FOUND
#  include <glib.h>
#endif

#include <QObject>
#include <QApplication>
#include <QCoreApplication>
#include <QIODevice>
#include <QFileDevice>
#include <QFile>
#include <QByteArray>
#include <QString>
#include <QLoggingCategory>
#include <QSettings>
#include <QtDebug>

#include "main.h"
#include "logging.h"
#include "commandlineoptions.h"
#include "application.h"
#include "utilities.h"
#include "metatypes.h"
#include "mainwindow.h"

int main(int argc, char* argv[]) {

#if defined(Q_OS_WIN32) || defined(Q_OS_MACOS)
  QCoreApplication::setApplicationName("SQLRestore");
  QCoreApplication::setOrganizationName("SQLRestore");
#else
  QCoreApplication::setApplicationName("sqlrestore");
  QCoreApplication::setOrganizationName("sqlrestore");
#endif
  QCoreApplication::setApplicationVersion(SQLRESTORE_VERSION_DISPLAY);
  QCoreApplication::setOrganizationDomain("jkvinge.net");

  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

#ifdef GLIB_FOUND
  g_set_application_name(QCoreApplication::applicationName().toLocal8Bit());
#endif

  logging::Init();

#ifdef GLIB_FOUND
  g_log_set_default_handler(reinterpret_cast<GLogFunc>(&logging::GLog), nullptr);
#endif

  CommandlineOptions options;
  {
    // Parse commandline options - need to do this before starting the full QApplication so it works without an X server
    QCoreApplication core_app(argc, argv);
    if (!options.Parse()) return 1;
  }

  qLog(Info) << "SQLRestore" << SQLRESTORE_VERSION_DISPLAY;

  // Seed the random number generators.
  Utilities::Seed();
  assert(Utilities::GetRandomStringWithCharsAndNumbers(20) != Utilities::GetRandomStringWithCharsAndNumbers(20));

  QApplication a(argc, argv);

#ifdef Q_OS_UNIX
  {
    QSettings s;
    if (!QFile::exists(s.fileName())) {
      QFile file(s.fileName());
      file.open(QIODevice::WriteOnly);
    }
    // Set -rw-------
    QFile::setPermissions(s.fileName(), QFile::ReadOwner | QFile::WriteOwner);
  }
#endif

  // Resources
  Q_INIT_RESOURCE(data);
  Q_INIT_RESOURCE(icons);

#ifdef DEBUG
  QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
#endif

  SQLRestore_Metatypes::RegisterMetaTypes();
  Application app;
  MainWindow w(&app, options);
  int ret = a.exec();
  return ret;

}
