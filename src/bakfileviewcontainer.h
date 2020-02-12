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

#ifndef BAKFILEVIEWCONTAINER_H
#define BAKFILEVIEWCONTAINER_H

#include <QObject>
#include <QWidget>
#include <QString>

class QTimer;
class QEvent;
class QKeyEvent;

class Ui_BakFileViewContainer;
class BakFileModel;
class BakFileView;
class BakFileFilter;

class BakFileViewContainer : public QWidget {
  Q_OBJECT

 public:
  explicit BakFileViewContainer(QWidget *parent = nullptr);
  ~BakFileViewContainer();

  void Init(BakFileModel *model, BakFileFilter *proxy);
  BakFileView *view() const;
  bool eventFilter(QObject*, QEvent*);

 private slots:
  void MaybeUpdateFilter();
  void UpdateFilter();
  void FocusOnFilter(QKeyEvent*);

 private:
  static const int kFilterDelayMs;
  static const int kFilterDelaySizeThreshold;

  Ui_BakFileViewContainer *ui_;
  BakFileModel *model_;
  BakFileFilter *proxy_;
  QTimer *filter_timer_;

};

#endif  // BAKFILEVIEWCONTAINER_H
