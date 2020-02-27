# SQL Restore - Copyright 2019, Jonas Kvinge <jonas@jkvinge.net>

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_BINARY_DIR})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GLIBCONFIG_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${QUAZIP_INCLUDE_DIRS})

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

option(USE_INSTALL_PREFIX "Look for data in CMAKE_INSTALL_PREFIX" ON)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

qt5_wrap_cpp(MOC ${HEADERS})
qt5_wrap_ui(UIC ${UI})
qt5_add_resources(QRC ${RESOURCES})

add_library(sqlrestore_lib STATIC
  ${SOURCES}
  ${MOC}
  ${UIC}
  ${QRC}
  ${POT}
  ${PO}
  ${OTHER_UIC_SOURCES}
)

target_link_libraries(sqlrestore_lib
  ${CMAKE_THREAD_LIBS_INIT}
  ${GLIB_LIBRARIES}
  ${MAGIC_LIBRARIES}
  ${QT_LIBRARIES}
  ${QUAZIP_LIBRARIES}
)

if (WIN32)
  target_link_libraries(sqlrestore_lib odbccp32 regex shlwapi)
endif()


set(EXECUTABLE_OUTPUT_PATH ..)

# Show the console window in debug mode on Windows
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT ENABLE_WIN32_CONSOLE)
  set(SQLRESTORE-WIN32-FLAG WIN32)
endif (NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT ENABLE_WIN32_CONSOLE)

# Resource file for windows
if(WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dist/windows/windres.rc.in ${CMAKE_CURRENT_BINARY_DIR}/windres.rc)
  set(SQLRESTORE-WIN32-RESOURCES windres.rc)
endif(WIN32)

add_executable(sqlrestore ${SQLRESTORE-WIN32-FLAG} ${SQLRESTORE-WIN32-RESOURCES} main.cpp)
target_link_libraries(sqlrestore sqlrestore_lib)
install(TARGETS sqlrestore RUNTIME DESTINATION bin)

