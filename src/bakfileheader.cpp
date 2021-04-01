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

#include <algorithm>
#include <numeric>

#include <QtGlobal>
#include <QHeaderView>
#include <QList>
#include <QResizeEvent>
#include <QMouseEvent>

#include "bakfilemodel.h"
#include "bakfileheader.h"

BakFileHeader::BakFileHeader(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent),
      in_mouse_move_event_(false) {

  setSectionsClickable(true);
  setSectionsMovable(true);
  setSectionResizeMode(QHeaderView::Interactive);
  setSortIndicator(BakFileModel::Column_Modified, Qt::DescendingOrder);

  connect(this, SIGNAL(sectionResized(int,int,int)), SLOT(SectionResized(int,int,int)));

}

void BakFileHeader::Init() {

  column_widths_.resize(count());
  std::fill(column_widths_.begin(), column_widths_.end(), 1.0 / count());

  for (int i = 0; i < count(); ++i) {
    column_widths_[i] = ColumnWidthType(sectionSize(i)) / width();
  }

  NormaliseWidths();
  UpdateWidths();

}

void BakFileHeader::SetColumnWidth(const int logical, const ColumnWidthType width) {

  column_widths_[logical] = width;

  QList<int> other_columns;
  for (int i = 0 ; i < count() ; ++i)
    if (!isSectionHidden(i) && i != logical)
      other_columns << i;

  NormaliseWidths(other_columns);

}

void BakFileHeader::NormaliseWidths(const QList<int> &sections) {

  const ColumnWidthType total_sum = std::accumulate(column_widths_.begin(), column_widths_.end(), 0.0);
  ColumnWidthType selected_sum = total_sum;

  if (!sections.isEmpty()) {
    selected_sum = 0.0;
    for (int i = 0 ; i < count() ; ++i) {
      if (sections.contains(i))
        selected_sum += column_widths_[i];
    }
  }

  if (total_sum != 0.0 && !qFuzzyCompare(total_sum, 1.0)) {
    const ColumnWidthType mult = (selected_sum + (1.0 - total_sum)) / selected_sum;
    for (int i = 0 ; i < column_widths_.count() ; ++i) {
      if (sections.isEmpty() || sections.contains(i))
        column_widths_[i] *= mult;
    }
  }

}

void BakFileHeader::UpdateWidths(const QList<int> &sections) {

  ColumnWidthType total_w = 0.0;

  for (int i = 0 ; i < column_widths_.count() ; ++i) {
    const ColumnWidthType w = column_widths_[i];
    int pixels = w * width();

    total_w += w;

    if (!sections.isEmpty() && !sections.contains(i))
      continue;

    if (pixels == 0 && !isSectionHidden(i))
      hideSection(i);
    else if (pixels != 0 && isSectionHidden(i)) {
      showSection(i);
    }

    if (pixels != 0)
      resizeSection(i, pixels);
  }

}

void BakFileHeader::resizeEvent(QResizeEvent *e) {

  QHeaderView::resizeEvent(e);
  UpdateWidths();

}

void BakFileHeader::mouseMoveEvent(QMouseEvent *e) {

  in_mouse_move_event_ = true;
  QHeaderView::mouseMoveEvent(e);
  in_mouse_move_event_ = false;

}

void BakFileHeader::SectionResized(const int logical, const int, const int new_size) {

  if (in_mouse_move_event_) {

    column_widths_[logical] = ColumnWidthType(new_size) / width();

    int visual = visualIndex(logical);
    QList<int> logical_sections_to_resize;
    for (int i = 0 ; i < count() ; ++i) {
      if (!isSectionHidden(i) && visualIndex(i) > visual)
        logical_sections_to_resize << i;
    }

    if (!logical_sections_to_resize.isEmpty()) {
      in_mouse_move_event_ = false;
      UpdateWidths(logical_sections_to_resize);
      NormaliseWidths(logical_sections_to_resize);
      in_mouse_move_event_ = true;
    }
  }

}
