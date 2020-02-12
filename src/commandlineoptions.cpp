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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QIODevice>
#include <QDataStream>
#include <QBuffer>
#include <QByteArray>

#include "commandlineoptions.h"
#include "logging.h"

CommandlineOptions::CommandlineOptions() {}

bool CommandlineOptions::Parse() {

  QCommandLineParser parser;
  parser.addVersionOption();
  parser.process(QCoreApplication::arguments());
  return true;

}

QByteArray CommandlineOptions::Serialize() const {

  QBuffer buf;
  buf.open(QIODevice::WriteOnly);
  QDataStream s(&buf);
  s << *this;
  buf.close();
  return buf.data().toBase64();

}

void CommandlineOptions::Load(const QByteArray &serialized) {

  QByteArray copy = QByteArray::fromBase64(serialized);
  QBuffer buf(&copy);
  buf.open(QIODevice::ReadOnly);
  QDataStream s(&buf);
  s >> *this;

}

QDataStream &operator<<(QDataStream &s, const CommandlineOptions &a) {
  s << a.nothing_;
  return s;
}

QDataStream &operator>>(QDataStream &s, CommandlineOptions &a) {
  s >> a.nothing_;
  return s;
}

