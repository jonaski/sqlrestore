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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QRegularExpression>

namespace Utilities {

static const QRegularExpression kValidFatCharacters("[^a-zA-Z0-9!#\\$%&'()\\-@\\^_`{}~/. ]");
static const QRegularExpression kInvalidFatCharacters("[\"*\\:<>?|/.]");

QString GetEnv(const QString &key);
void SetEnv(const char *key, const QString &value);
void Seed();
QString GetRandomString(const int len, const QString &UseCharacters);
QString GetRandomStringWithChars(const int len);
QString GetRandomStringWithCharsAndNumbers(const int len);
QString CryptographicRandomString(const int len);
QString PrettySize(const qint64 bytes);
QString PrettyTime(int seconds);
QString WordyTime(quint64 seconds);

}  // namespace Utilities

#endif  // UTILITIES_H
