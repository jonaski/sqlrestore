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

#ifndef BAKFILEHEADER_H
#define BAKFILEHEADER_H

#include "config.h"

#include <QHeaderView>
#include <QList>
#include <QVector>

class QAbstractItemModel;
class QMouseEvent;
class QResizeEvent;

class BakFileView;

class BakFileHeader : public QHeaderView {
  Q_OBJECT

 public:
  explicit BakFileHeader(Qt::Orientation orientation, BakFileView *view, QWidget *parent = nullptr);

  typedef double ColumnWidthType;

  void setModel(QAbstractItemModel* model);
  void SetColumnWidth(const int logical, const ColumnWidthType width);

 private:
  void NormaliseWidths(const QList<int>& sections = QList<int>());
  void UpdateWidths(const QList<int>& sections = QList<int>());

 protected:
  // QWidget
  void mouseMoveEvent(QMouseEvent* e);
  void resizeEvent(QResizeEvent* event);

 private slots:
  void SectionResized(const int logical, const int old_size, const int new_size);

 private:
  BakFileView *view_;
  QVector<ColumnWidthType> column_widths_;
  bool in_mouse_move_event_;

};

#endif  // BAKFILEHEADER_H

