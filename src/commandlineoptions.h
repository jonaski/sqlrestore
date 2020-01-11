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

#ifndef COMMANDLINEOPTIONS_H
#define COMMANDLINEOPTIONS_H

#include "config.h"

#include <QDataStream>
#include <QByteArray>

class CommandlineOptions {
  friend QDataStream &operator<<(QDataStream &s, const CommandlineOptions &a);
  friend QDataStream &operator>>(QDataStream &s, CommandlineOptions &a);

 public:
  explicit CommandlineOptions();

  bool Parse();
  bool is_empty() const { return true; }
  QByteArray Serialize() const;
  void Load(const QByteArray &serialized);

 private:
  bool nothing_;

};

QDataStream &operator<<(QDataStream &s, const CommandlineOptions &a);
QDataStream &operator>>(QDataStream &s, CommandlineOptions &a);

#endif  // COMMANDLINEOPTIONS_H
