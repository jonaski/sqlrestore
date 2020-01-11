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

#ifndef BACKUPRESULT_H
#define BACKUPRESULT_H

#include <QString>
#include <QStringList>

class BackupResult {
 public:
  BackupResult(const QString &filename, const bool success, const QStringList &errors) : filename_(filename), success_(success), errors_(errors) {}

  QString filename() { return filename_; }
  bool success() { return success_; }
  QStringList errors() { return errors_; }

 private:
  QString filename_;
  bool success_;
  QStringList errors_;
};

#endif  //  BACKUPRESULT_H
