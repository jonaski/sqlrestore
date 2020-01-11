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

#ifndef BAKFILEFILTER_H
#define BAKFILEFILTER_H

#include "config.h"

#include <QSortFilterProxyModel>
#include <QList>

#include "bakfilemodel.h"

class FilterTree;

class BakFileFilter : public QSortFilterProxyModel {
  Q_OBJECT

 public:
  explicit BakFileFilter(QObject *parent = nullptr);

  void sort(const int column, const Qt::SortOrder order = Qt::AscendingOrder);
  void setFilterKeyColumns(const QList<qint32> &filter_columns);
 
 protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
 
 private:
  QList<qint32> filter_columns_;
};

#endif  // BAKFILEFILTER_H
