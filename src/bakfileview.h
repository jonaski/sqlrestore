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

#ifndef BAKFILEVIEW_H
#define BAKFILEVIEW_H

#include "config.h"

#include <QTreeView>

class QAbstractItemModel;

class BakFileHeader;

class BakFileView : public QTreeView {
  Q_OBJECT

 public:
  explicit BakFileView(QWidget *parent = nullptr);

  void setModel(QAbstractItemModel *model);
  void Init();
  
 private slots:
  void SelectAll();
  void UnSelectAll();

 private:
  BakFileHeader *header_;

};

#endif  // BAKFILEVIEW_H
