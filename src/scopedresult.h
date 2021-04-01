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

#ifndef SCOPEDRESULT_H
#define SCOPEDRESULT_H

#include <boost/noncopyable.hpp>

#include <QObject>
#include <QString>
#include <QStringList>

class ScopedResult : public QObject, boost::noncopyable {
  Q_OBJECT

 public:
  explicit ScopedResult(QObject *parent = nullptr);
  explicit ScopedResult(const QString &filename = QString(), QObject *parent = nullptr);
  ~ScopedResult();

  void failure(const QString &error = QString());
  void failure(const QStringList &errors);
  void success();
  void set_filename(const QString &filename) { filename_ = filename; }

 signals:
  void Started();
  void Finished(bool success);
  void Finished(QString filename, bool success, QStringList errors);
  void Status(QString message);
  void Success();
  void Failure(QString error);
  void Failure(QStringList errors);

 private:
  QString filename_;
  bool pending_;
  bool success_;
  QStringList errors_;
};

#endif  // SCOPEDRESULT_H
