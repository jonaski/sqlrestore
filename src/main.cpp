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

#include <singleapplication.h>
#include <singlecoreapplication.h>

#include <QObject>
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

#ifdef GLIB_FOUND
  g_set_application_name(QCoreApplication::applicationName().toLocal8Bit());
#endif

  logging::Init();
#ifdef GLIB_FOUND
  g_log_set_default_handler(reinterpret_cast<GLogFunc>(&logging::GLog), nullptr);
#endif

  CommandlineOptions options;
  {
    // Only start a core application now so we can check if there's another instance without requiring an X server.
    // This MUST be done before parsing the commandline options so QTextCodec gets the right system locale for filenames.
    SingleCoreApplication core_app(argc, argv, true, SingleCoreApplication::Mode::User);
    // Parse commandline options - need to do this before starting the full QApplication so it works without an X server
    if (!options.Parse()) return 1;
    if (core_app.isSecondary()) {
      if (options.is_empty()) {
        qLog(Info) << "SQLRestore is already running - activating existing window (1)";
      }
      if (core_app.sendMessage(options.Serialize(), 5000)) {
        return 0;
      }
      else {
        qLog(Error) << "Failed to send message to primary instance.";
      }
    }
  }

  qLog(Info) << "SQLRestore" << SQLRESTORE_VERSION_DISPLAY;

  // Seed the random number generators.
  Utilities::Seed();
  assert(Utilities::GetRandomStringWithCharsAndNumbers(20) != Utilities::GetRandomStringWithCharsAndNumbers(20));

  // important: Do not remove this.
  // This must also be done as a SingleApplication, in case SingleCoreApplication was compiled with a different appdata.
  SingleApplication a(argc, argv, true, SingleApplication::Mode::User);
  if (a.isSecondary()) {
    if (options.is_empty()) {
      qLog(Info) << "SQLRestore is already running - activating existing window (2)";
    }
    if (a.sendMessage(options.Serialize(), 5000)) {
      return 0;
    }
    else {
      qLog(Error) << "Failed to send message to primary instance.";
    }
  }

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
  QObject::connect(&a, SIGNAL(receivedMessage(quint32, QByteArray)), &w, SLOT(CommandlineOptionsReceived(quint32, QByteArray)));
  int ret = a.exec();
  return ret;

}
