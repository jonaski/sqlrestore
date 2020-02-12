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

#include <QtGlobal>
#include <QWidget>
#include <QApplication>
#include <QList>
#include <QString>
#include <QRegExp>
#include <QTimer>
#include <QFrame>
#include <QEvent>
#include <QKeyEvent>

#include "qsearchfield.h"
#include "bakfilemodel.h"
#include "bakfileviewcontainer.h"
#include "bakfileview.h"
#include "bakfilefilter.h"
#include "ui_bakfileviewcontainer.h"

const int BakFileViewContainer::kFilterDelayMs = 100;
const int BakFileViewContainer::kFilterDelaySizeThreshold = 100;

BakFileViewContainer::BakFileViewContainer(QWidget *parent)
    : QWidget(parent),
      ui_(new Ui_BakFileViewContainer),
      model_(nullptr),
      proxy_(nullptr),
      filter_timer_(new QTimer(this)) {

  ui_->setupUi(this);

  ui_->toolbar->setStyleSheet("QFrame { border: 0px; }");

  filter_timer_->setSingleShot(true);
  filter_timer_->setInterval(kFilterDelayMs);
  connect(filter_timer_, SIGNAL(timeout()), this, SLOT(UpdateFilter()));
  connect(ui_->filter, SIGNAL(textChanged(QString)), SLOT(MaybeUpdateFilter()));
  ui_->filter->installEventFilter(this);

}

BakFileViewContainer::~BakFileViewContainer() { delete ui_; }

BakFileView *BakFileViewContainer::view() const { return ui_->view; }

void BakFileViewContainer::Init(BakFileModel *model, BakFileFilter *proxy) {

  model_ = model;
  proxy_ = proxy;
  proxy_->setFilterKeyColumns(QList<qint32>() << BakFileModel::Column_Filename << BakFileModel::Column_FileType);
  proxy_->sort(BakFileModel::Column_Modified, Qt::AscendingOrder);
  ui_->filter->setText(proxy_->filterRegExp().pattern());

}

void BakFileViewContainer::MaybeUpdateFilter() {

  if (model_->rowCount() < kFilterDelaySizeThreshold || ui_->filter->text().isEmpty()) {
    UpdateFilter();
  }
  else {
    filter_timer_->start();
  }

}

void BakFileViewContainer::UpdateFilter() {

  proxy_->setFilterFixedString(ui_->filter->text());

}

void BakFileViewContainer::FocusOnFilter(QKeyEvent *event) {

  ui_->filter->setFocus();

  switch (event->key()) {
    case Qt::Key_Backspace:
      break;

    case Qt::Key_Escape:
      ui_->filter->clear();
      break;

    default:
      ui_->filter->setText(ui_->filter->text() + event->text());
      break;
  }
}

bool BakFileViewContainer::eventFilter(QObject *object, QEvent *event) {

  if (object == ui_->filter) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *e = static_cast<QKeyEvent*>(event);
      switch (e->key()) {
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Return:
        case Qt::Key_Enter:
          view()->setFocus(Qt::OtherFocusReason);
          QApplication::sendEvent(ui_->view, event);
          return true;
        case Qt::Key_Escape:
          ui_->filter->clear();
          return true;
        default:
          break;
      }
    }
  }
  return QWidget::eventFilter(object, event);

}
