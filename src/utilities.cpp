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

#include <QtGlobal>

#include <cstdlib>
#include <ctime>

#include <QList>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QChar>
#include <QLocale>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#  include <QRandomGenerator>
#endif

#include "utilities.h"

namespace Utilities {

QString GetEnv(const QString &key) {
  return QString::fromLocal8Bit(qgetenv(key.toLocal8Bit()));
}

void SetEnv(const char *key, const QString &value) {

#ifdef Q_OS_WIN32
  putenv(QString("%1=%2").arg(key, value).toLocal8Bit().constData());
#else
  setenv(key, value.toLocal8Bit().constData(), 1);
#endif

}

void Seed() {

  time_t t = time(nullptr);
  srand(t);
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  qsrand(t);
#endif

}

QString PrettySize(const qint64 bytes) {

  QString ret;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
  QLocale locale;
  ret = locale.formattedDataSize(bytes);
#else
  if (bytes > 0) {
    if (bytes <= 1000)
      ret = QString::number(bytes) + " bytes";
    else if (bytes <= 1000 * 1000)
      ret = QString::asprintf("%.1f KB", float(bytes) / 1000);
    else if (bytes <= 1000 * 1000 * 1000)
      ret = QString::asprintf("%.1f MB", float(bytes) / (1000 * 1000));
    else
      ret = QString::asprintf("%.1f GB", float(bytes) / (1000 * 1000 * 1000));
  }
#endif

  return ret;

}

QString PrettyTime(int seconds) {

  int hours = seconds / (60 * 60);
  int minutes = (seconds / 60) % 60;
  seconds %= 60;

  QString ret;
  if (hours) ret = QString("%d:%02d:%02d").arg(hours).arg(minutes).arg(seconds);
  else ret = QString("%d:%02d").arg(minutes).arg(seconds);

  return ret;

}

QString WordyTime(quint64 seconds) {

  quint64 days = seconds / (60 * 60 * 24);

  if (days) return (days == 1 ? QString("1 day") : QString("%1 days").arg(days));
  else return PrettyTime(seconds - days * 60 * 60 * 24);

}

QString GetRandomString(const int len, const QString &UseCharacters) {

   QString randstr;
   for (int i = 0; i < len; ++i) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
     const qint64 index = QRandomGenerator::global()->bounded(0, UseCharacters.length());
#else
     const qint64 index = qrand() % UseCharacters.length();
#endif
     QChar nextchar = UseCharacters.at(index);
     randstr.append(nextchar);
   }
   return randstr;

}

QString GetRandomStringWithChars(const int len) {
  const QString UseCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
  return GetRandomString(len, UseCharacters);
}

QString GetRandomStringWithCharsAndNumbers(const int len) {
  const QString UseCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
  return GetRandomString(len, UseCharacters);
}

QString CryptographicRandomString(const int len) {
  const QString UseCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~");
  return GetRandomString(len, UseCharacters);
}

}  // namespace Utilities
