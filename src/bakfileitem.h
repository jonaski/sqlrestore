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

#ifndef BAKFILEITEM_H
#define BAKFILEITEM_H

#include "config.h"

#include <memory>

#include <QList>
#include <QString>
#include <QDateTime>

class BakFileItem : public std::enable_shared_from_this<BakFileItem> {

 public:
  explicit BakFileItem();
  explicit BakFileItem(const QString &filename, const quint64 &file_size, const QDateTime &modified, const bool compressed, const QString &file_type);
  ~BakFileItem();

  QString filename() const { return filename_; }
  quint64 file_size() const { return file_size_; }
  QDateTime modified() const { return modified_; }
  bool compressed() const { return compressed_; }
  QString file_type() const { return file_type_; }
  bool is_valid() const { return true; }
  bool operator==(BakFileItem other) const;
  bool operator!=(BakFileItem other) const;
  void clear();

 protected:
   QString filename_;
   quint64 file_size_;
   QDateTime modified_;
   bool compressed_;
   QString file_type_;

};

typedef std::shared_ptr<BakFileItem> BakFileItemPtr;
typedef QList<BakFileItemPtr> BakFileItemList;

Q_DECLARE_METATYPE(BakFileItemPtr)
Q_DECLARE_METATYPE(QList<BakFileItemPtr>)

#endif  // BAKFILEITEM_H
