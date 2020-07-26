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

#include <QSortFilterProxyModel>
#include <QAbstractItemModel>
#include <QList>
#include <QVariant>
#include <QString>

#include "bakfilefilter.h"

BakFileFilter::BakFileFilter(QObject *parent) : QSortFilterProxyModel(parent) {
  setDynamicSortFilter(true);
}

void BakFileFilter::sort(const int column, const Qt::SortOrder order) {
  sourceModel()->sort(column, order);  // QAbstractItemModel
}
 
void BakFileFilter::setFilterKeyColumns(const QList<qint32> &filter_columns) {

  filter_columns_.clear();
 
  for (const qint32 column : filter_columns) {
    filter_columns_ << column;
  }

}
 
bool BakFileFilter::filterAcceptsRow(const int source_row, const QModelIndex &source_parent) const {

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  QString filter = filterRegularExpression().pattern();
#else
  QString filter = filterRegExp().pattern().toLower();
#endif
  if (filter_columns_.isEmpty() || filter.isEmpty()) return true;

  bool ret = false;
  for (const qint32 column : filter_columns_) {
    QModelIndex idx = sourceModel()->index(source_row, column, source_parent);
    ret = (idx.data().toString().toLower().contains(filter));
    if (ret) return ret;
  }
  return ret;

}
