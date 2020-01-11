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

#include <QString>
#include <QDateTime>

#include "logging.h"
#include "bakfileitem.h"

BakFileItem::BakFileItem() : compressed_(false) {}
BakFileItem::BakFileItem(const QString &filename,
              const quint64 &file_size,
              const QDateTime &modified,
              const bool compressed,
              const QString &file_type) :
              filename_(filename),
              file_size_(file_size),
              modified_(modified),
              compressed_(compressed),
              file_type_(file_type) {

  //qLog(Debug) << "item for" << filename_ << "allocated.";

}
BakFileItem::~BakFileItem() {
  //qLog(Debug) << "Item for" << filename_ << "released.";
}

void BakFileItem::clear() {

  filename_.clear();
  file_size_ = -1;
  modified_ = QDateTime();
  compressed_ = false;
  file_type_.clear();

}

bool BakFileItem::operator==(BakFileItem other) const {

  //qLog(Debug) << __PRETTY_FUNCTION__ << filename_ << other.filename();

  return filename_ == other.filename() &&
         file_size_ == other.file_size() &&
         modified_ == other.modified() &&
         compressed_ == other.compressed() &&
         file_type_ == other.file_type();

}

bool BakFileItem::operator!=(BakFileItem other) const {

  //qLog(Debug) << __PRETTY_FUNCTION__ << filename_ << other.filename();

  return filename_ != other.filename() ||
         file_size_ != other.file_size() ||
         modified_ != other.modified() ||
         compressed_ != other.compressed() ||
         file_type_ != other.file_type();

}

