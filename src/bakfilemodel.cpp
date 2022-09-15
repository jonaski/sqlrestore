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

#include <memory>
#include <algorithm>
#include <functional>

#include <QAbstractListModel>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QDateTime>
#include <QFlags>
#include <QtDebug>

#include "logging.h"
#include "utilities.h"

#include "bakfilemodel.h"
#include "bakfileitem.h"

BakFileModel::BakFileModel(QObject *parent) : QAbstractListModel(parent), sort_column_(Column_Modified), sort_order_(Qt::AscendingOrder) {}

QString BakFileModel::column_name(const Column column) {

  switch (column) {
    case Column_Filename:       return tr("Filename");
    case Column_FileSize:       return tr("Size");
    case Column_Modified:       return tr("Date");
    case Column_Compressed:     return tr("Compressed");
    case Column_FileType:       return tr("Type");
    default:                    qLog(Error) << "No such column" << column;;
  }
  return QString("");

}

QVariant BakFileModel::headerData(int section, Qt::Orientation, int role) const {

  if (role != Qt::DisplayRole && role != Qt::ToolTipRole) return QVariant();

  const QString name = column_name(static_cast<BakFileModel::Column>(section));
  if (!name.isEmpty()) return name;

  return QVariant();

}


QVariant BakFileModel::data(const QModelIndex &idx, int role) const {

  if (!idx.isValid()) return QVariant();

  BakFileItemPtr item = items_[idx.row()];
  switch (role) {
    case Qt::DisplayRole:
      switch (idx.column()) {
        case Column_Filename:
          return item->filename();
        case Column_FileSize:
          return Utilities::PrettySize(item->file_size());
        case Column_Modified:
          return item->modified();
        case Column_Compressed:
          return item->compressed();
        case Column_FileType:
          return item->file_type();
        default:
          break;
      }
    default:
      break;
  }

  return QVariant();

}

Qt::ItemFlags BakFileModel::flags(const QModelIndex &index) const {

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (index.isValid()) return flags | Qt::ItemIsDragEnabled;

  return Qt::ItemIsDropEnabled;

}

void BakFileModel::sort(int column, Qt::SortOrder order) {

  sort_column_ = column;
  sort_order_ = order;

  BakFileItemList new_items(items_);
  BakFileItemList::iterator begin = new_items.begin();

  if (column == Column_Filename) {
    std::stable_sort(begin, new_items.end(), std::bind(&BakFileModel::CompareItems, Column_Modified, order, std::placeholders::_1, std::placeholders::_2));
    std::stable_sort(begin, new_items.end(), std::bind(&BakFileModel::CompareItems, Column_Filename, order, std::placeholders::_1, std::placeholders::_2));
  }
  else {
    std::stable_sort(begin, new_items.end(), std::bind(&BakFileModel::CompareItems, column, order, std::placeholders::_1, std::placeholders::_2));
  }

  emit layoutAboutToBeChanged();

  BakFileItemList old_items = items_;
  items_ = new_items;

  QMap<const BakFileItem*, int> new_rows;
  for (int i = 0; i < new_items.length() ; ++i) {
    new_rows[new_items[i].get()] = i;
  }

  for (const QModelIndex &idx : persistentIndexList()) {
    const BakFileItem *item = old_items[idx.row()].get();
    changePersistentIndex(idx, index(new_rows[item], idx.column(), idx.parent()));
  }

  emit layoutChanged();

}

bool BakFileModel::CompareItems(const int column, const Qt::SortOrder order, std::shared_ptr<BakFileItem> _a, std::shared_ptr<BakFileItem> _b) {

  std::shared_ptr<BakFileItem> a = order == Qt::AscendingOrder ? _a : _b;
  std::shared_ptr<BakFileItem> b = order == Qt::AscendingOrder ? _b : _a;

  switch (column) {
    case Column_Filename:     return QString::localeAwareCompare(a->filename().toLower(), b->filename().toLower()) < 0;
    case Column_FileSize:     return a->file_size() < b->file_size();
    case Column_Modified:     return a->modified() < b->modified();
    case Column_Compressed:   return a->compressed() == b->compressed();
    case Column_FileType:     return QString::localeAwareCompare(a->file_type().toLower(), b->file_type().toLower()) < 0;
    default:                  qLog(Error) << "No such column" << column;
  }

  return false;

}

void BakFileModel::AddedFiles(BakFileItemList items) {

  const int pos = -1;

  const int start = pos == -1 ? items_.count() : pos;
  const int end = start + items.count() - 1;

  beginInsertRows(QModelIndex(), start, end);
  for (int i = start; i <= end; ++i) {
    BakFileItemPtr item = items[i - start];
    items_.insert(i, item);
  }
  endInsertRows();

  emit dataChanged(index(0, 0), index(rowCount() - 1, 0));

  sort(sort_column_, sort_order_);

}

void BakFileModel::UpdatedFiles(BakFileItemList items) {

  QList<BakFileItemPtr>::iterator i = items.begin();
  while (i != items.end()) {
    if (items_.contains(*i)) {
      int item_pos = items_.indexOf(*i);
      emit dataChanged(index(item_pos, 0), index(item_pos, ColumnCount - 1));
    }
    ++i;
  }

}

void BakFileModel::DeletedFiles(BakFileItemList items) {

  QList<BakFileItemPtr>::iterator i = items.begin();
  while (i != items.end()) {
    if (items_.contains(*i)) {
      BakFileItemPtr item = *i;
      int item_pos = items_.indexOf(*i);
      beginRemoveRows(QModelIndex(), item_pos, item_pos);
      items_.removeAll(*i);
      endRemoveRows();
      QModelIndex idx_topleft = index(item_pos, 0);
      QModelIndex idx_bottomright = index(item_pos, rowCount() - 1);
      emit dataChanged(idx_topleft, idx_bottomright);
    }
    ++i;
  }

}
