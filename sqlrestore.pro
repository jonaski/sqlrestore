SQLRESTORE_VERSION_DISPLAY = 0.0.1
SQLRESTORE_VERSION_PACKAGE = 0.0.1

SOURCES += src/main.cpp \
           src/mainwindow.cpp \
           src/application.cpp \
           src/commandlineoptions.cpp \
           src/logging.cpp \
           src/utilities.cpp \
           src/metatypes.cpp \
           src/iconloader.cpp \
           src/dbconnector.cpp \
           src/scopedresult.cpp \
           src/qsearchfield_nonmac.cpp \
           src/bakfilebackend.cpp \
           src/bakfilemodel.cpp \
           src/bakfileviewcontainer.cpp \
           src/bakfileview.cpp \
           src/bakfileheader.cpp \
           src/bakfileitem.cpp \
           src/bakfilefilter.cpp \
           src/backupbackend.cpp \
           src/aboutdialog.cpp \
           src/settingsdialog.cpp \
           src/testserverdialog.cpp \
           3rdparty/singleapplication/singleapplication.cpp \
           3rdparty/singleapplication/singleapplication_p.cpp \
           3rdparty/singleapplication/singlecoreapplication.cpp \
           3rdparty/singleapplication/singlecoreapplication_p.cpp

HEADERS += src/mainwindow.h \
           src/application.h \
           src/dbconnector.h \
           src/scopedresult.h \
           src/qsearchfield.h \
           src/bakfilebackend.h \
           src/bakfilemodel.h \
           src/bakfileviewcontainer.h \
           src/bakfileview.h \
           src/bakfileheader.h \
           src/backupbackend.h \
           src/bakfilefilter.h \
           src/aboutdialog.h \
           src/settingsdialog.h \
           src/testserverdialog.h \
           3rdparty/singleapplication/singleapplication.h \
           3rdparty/singleapplication/singleapplication_p.h \
           3rdparty/singleapplication/singlecoreapplication.h \
           3rdparty/singleapplication/singlecoreapplication_p.h

FORMS +=   src/mainwindow.ui \
           src/aboutdialog.ui \
           src/bakfileviewcontainer.ui \
           src/settingsdialog.ui \
           src/testserverdialog.ui

RESOURCES = data/data.qrc data/icons.qrc

#QMAKE_CC = /home/jonas/mxe-qt/usr/bin/x86_64-w64-mingw32.static-gcc
#QMAKE_CXX = /home/jonas/mxe-qt/usr/bin/x86_64-w64-mingw32.static-g++
#INCLUDEPATH += /home/jonas/mxe-qt/usr/x86_64-w64-mingw32.static/include

CONFIG += qt thread debug
QT += core widgets gui network sql concurrent
DEFINES += QUAZIP_STATIC

INCLUDEPATH += $$PWD $$PWD/src

LIBS += -lmagic -lregex -lquazip -lodbccp32

TARGET = sqlrestore

RC_ICONS = dist/windows/sqlrestore.ico
RC_FILE += dist/windows/windres.rc


#QMAKE_SUBSTITUTES += version.h.in

version.input = src/version.h.in
version.output = version.h
QMAKE_SUBSTITUTES += version

#config.input = src/config.h.in
#config.output = config.h
#QMAKE_SUBSTITUTES += config
