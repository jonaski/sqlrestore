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

#include <QString>
#include <QStringList>
#include <QtDebug>

#include "logging.h"
#include "scopedresult.h"

ScopedResult::ScopedResult(QObject *parent) : QObject(parent), pending_(true), success_(false) {}

ScopedResult::ScopedResult(const QString &filename, QObject *parent) : QObject(parent), filename_(filename), pending_(true), success_(false) {}

ScopedResult::~ScopedResult() {

  if (pending_)
    qLog(Error) << filename_ << "Still pending!";

  if (success_) {
    qLog(Debug) << "Success";
    emit Status(tr("Success"));
    emit Success();
  }
  else {
    qLog(Error) << "Failure";
    emit Failure(errors_);
  }
  emit Finished(success_);
  emit Finished(filename_, success_, errors_);

}

void ScopedResult::failure(const QString &error) {

  pending_ = false;
  success_ = false;

  if (!error.isEmpty()) {
    qLog(Error) << error;
    errors_ << error;
  }

}

void ScopedResult::failure(const QStringList &errors) {

  pending_ = false;
  success_ = false;
  for (const QString &error : errors) {
    qLog(Error) << error;
    errors_ << error;
  }

}

void ScopedResult::success() {

  pending_ = false;
  success_ = true;

}
