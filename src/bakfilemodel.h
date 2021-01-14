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

#ifndef BAKFILEMODEL_H
#define BAKFILEMODEL_H

#include <memory>

#include <QtGlobal>
#include <QObject>
#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QVariant>
#include <QString>

#include "bakfileitem.h"

class BakFileModel : public QAbstractListModel {
  Q_OBJECT

 public:
  explicit BakFileModel(QObject *parent);

  enum Column {
    Column_Filename = 0,
    Column_FileSize,
    Column_Modified,
    Column_Compressed,
    Column_FileType,
    ColumnCount
  };
  static QString column_name(const Column column);

  const BakFileItemPtr &item_at(const int idx) const { return items_[idx]; }

  int rowCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return items_.size(); }
  void sort(int column, Qt::SortOrder order) override;

 private:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QVariant data(const QModelIndex &idx, int role) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return ColumnCount; }
  void InsertFileItems(const BakFileItemList &items_in);
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  static bool CompareItems(const int column, const Qt::SortOrder order, std::shared_ptr<BakFileItem> _a, std::shared_ptr<BakFileItem> _b);

 private slots:
  void AddedFiles(BakFileItemList);
  void UpdatedFiles(BakFileItemList);
  void DeletedFiles(BakFileItemList);

 private:
  BakFileItemList items_;
  int sort_column_;
  Qt::SortOrder sort_order_;

};

#endif  // BAKFILEMODEL_H
