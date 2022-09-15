# SQL Restore - Copyright 2019, Jonas Kvinge <jonas@jkvinge.net>

set(SOURCES
  mainwindow.cpp
  application.cpp
  logging.cpp
  utilities.cpp
  metatypes.cpp
  commandlineoptions.cpp
  iconloader.cpp
  scopedresult.cpp
  aboutdialog.cpp
  qsearchfield.cpp
  settingsdialog.cpp
  testserverdialog.cpp
  dbconnector.cpp
  backupbackend.cpp
  bakfileitem.cpp
  bakfilebackend.cpp
  bakfilemodel.cpp
  bakfileviewcontainer.cpp
  bakfileview.cpp
  bakfileheader.cpp
  bakfilefilter.cpp
)

set(HEADERS
  mainwindow.h
  application.h
  logging.h
  scopedresult.h
  aboutdialog.h
  qsearchfield.h
  settingsdialog.h
  testserverdialog.h
  dbconnector.h
  backupbackend.h
  bakfilebackend.h
  bakfilemodel.h
  bakfileviewcontainer.h
  bakfileview.h
  bakfileheader.h
  bakfilefilter.h
)

set(UI
  mainwindow.ui
  aboutdialog.ui
  bakfileviewcontainer.ui
  settingsdialog.ui
  testserverdialog.ui
)

set(RESOURCES ../data/data.qrc ../data/icons.qrc)
set(OTHER_SOURCES)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

qt_wrap_cpp(MOC ${HEADERS})
qt_wrap_ui(UIC ${UI})
qt_add_resources(QRC ${RESOURCES})

add_library(sqlrestore_lib STATIC
  ${SOURCES}
  ${MOC}
  ${UIC}
  ${QRC}
  ${POT}
  ${PO}
  ${OTHER_UIC_SOURCES}
)

target_include_directories(sqlrestore_lib SYSTEM PUBLIC
  ${GLIB_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  ${MAGIC_INCLUDE_DIRS}
  ${QUAZIP_INCLUDE_DIRS}
  ${SINGLEAPPLICATION_INCLUDE_DIRS}
  ${SINGLECOREAPPLICATION_INCLUDE_DIRS}
)

target_include_directories(sqlrestore_lib PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_directories(sqlrestore_lib PUBLIC
  ${ZLIB_LIBRARY_DIRS}
  ${MAGIC_LIBRARY_DIRS}
  ${GLIB_LIBRARY_DIRS}
  ${QUAZIP_LIBRARY_DIRS}
)

target_link_libraries(sqlrestore_lib PUBLIC
  ${CMAKE_THREAD_LIBS_INIT}
  ${ZLIB_LIBRARIES}
  ${MAGIC_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${QtCore_LIBRARIES}
  ${QtConcurrent_LIBRARIES}
  ${QtGui_LIBRARIES}
  ${QtWidgets_LIBRARIES}
  ${QtNetwork_LIBRARIES}
  ${QtSql_LIBRARIES}
  QuaZip::QuaZip
)

if(UNIX AND _Qt5Core_PRIVATE_DIRS_EXIST AND _Qt5Sql_PRIVATE_DIRS_EXIST AND Qt5Sql_VERSION VERSION_GREATER_EQUAL 5.14.1 AND NOT APPLE)
  target_link_libraries(sqlrestore_lib PRIVATE qsqlodbc)
endif()

if(WIN32)
  target_link_libraries(sqlrestore_lib PRIVATE odbccp32 regex shlwapi)
  if(CMAKE_CROSSCOMPILING)
    target_link_libraries(sqlrestore_lib PUBLIC bz2 harfbuzz freetype)
  endif()
  if(MSVC)
    target_link_libraries(sqlrestore_lib PUBLIC legacy_stdio_definitions)
  endif()
endif()

set(EXECUTABLE_OUTPUT_PATH ..)

# Show the console window in debug mode on Windows
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT ENABLE_WIN32_CONSOLE)
  set(SQLRESTORE-WIN32-FLAG WIN32)
endif()

# Resource file for windows
if(WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dist/windows/windres.rc.in ${CMAKE_CURRENT_BINARY_DIR}/windres.rc)
  set(SQLRESTORE-WIN32-RESOURCES windres.rc)
endif()

add_executable(sqlrestore ${SQLRESTORE-WIN32-FLAG} ${SQLRESTORE-WIN32-RESOURCES} main.cpp)
target_link_libraries(sqlrestore PRIVATE sqlrestore_lib)
install(TARGETS sqlrestore RUNTIME DESTINATION bin)

