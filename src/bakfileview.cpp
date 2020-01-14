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

#include <QTreeView>
#include <QAbstractItemView>
#include <QAbstractItemModel>

#include "bakfilemodel.h"
#include "bakfileview.h"
#include "bakfileheader.h"

BakFileView::BakFileView(QWidget *parent) : QTreeView(parent), header_(new BakFileHeader(Qt::Horizontal, this, this)) {

  header_->setSortIndicator(BakFileModel::Column_Modified, Qt::DescendingOrder);
  header_->setSectionsMovable(true);
  setHeader(header_);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setMouseTracking(true);
  setSortingEnabled(true);
  setRootIsDecorated(false);

}

void BakFileView::setModel(QAbstractItemModel *model) {

  QTreeView::setModel(model);
  header_->setModel(model);

}

void BakFileView::Init() {

  header_->SetColumnWidth(BakFileModel::Column_Filename, 0.37);
  header_->SetColumnWidth(BakFileModel::Column_FileSize, 0.08);
  header_->SetColumnWidth(BakFileModel::Column_Modified, 0.10);
  header_->SetColumnWidth(BakFileModel::Column_Compressed, 0.05);
  header_->SetColumnWidth(BakFileModel::Column_FileType, 0.30);

}

void BakFileView::SelectAll() {
  selectAll();
}

void BakFileView::UnSelectAll() {
  selectionModel()->clearSelection();
}
