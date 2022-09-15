# SQL Restore
# Copyright 2019, Jonas Kvinge <jonas@jkvinge.net>

project(sqlrestore)
cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0054 NEW)

include(CheckCXXCompilerFlag)
include(CheckIncludeFiles)
include(FindPkgConfig)
include(FindPackageHandleStandardArgs)
include(cmake/Version.cmake)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  include(cmake/Rpm.cmake)
  include(cmake/Deb.cmake)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

option(BUILD_WERROR "Build with -Werror" OFF)

if(WIN32)
  option(ENABLE_WIN32_CONSOLE "Show the windows console even outside Debug mode" OFF)
endif()

if(MSVC)
  set(CMAKE_C_STANDARD 99)
else()
  set(CMAKE_C_STANDARD 11)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  list(APPEND COMPILE_OPTIONS /std:c++17)
else()
  list(APPEND COMPILE_OPTIONS
    $<$<COMPILE_LANGUAGE:C>:-std=c11>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
    -Wall
    -Wextra
    -Wpedantic
    -Wunused
    -Wuninitialized
    -Wredundant-decls
    -Wcast-align
    -Winit-self
    -Wmissing-include-dirs
    -Wmissing-declarations
    -Wstrict-overflow=2
    -Wunused-parameter
    -Wformat=2
    -Wdisabled-optimization
    -Wno-sign-conversion
    $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>
    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
  )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  list(APPEND COMPILE_OPTIONS -Wno-language-extension-token)
endif()

if(NOT MSVC AND BUILD_WERROR)
  list(APPEND COMPILE_OPTIONS -Werror)
endif()

if(COMPILE_OPTIONS)
  add_compile_options(${COMPILE_OPTIONS})
endif()

find_program(CCACHE_EXECUTABLE NAMES ccache)
if(CCACHE_EXECUTABLE)
  message(STATUS "ccache found: will be used for compilation and linkage")
  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_EXECUTABLE})
  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_EXECUTABLE})
endif()

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED)
find_package(Backtrace)
if(Backtrace_FOUND)
  set(HAVE_BACKTRACE ON)
endif()
find_package(ZLIB REQUIRED)

find_library(MAGIC_LIBRARIES NAMES magic libmagic.dll HINTS /usr/lib /usr/lib64)

pkg_check_modules(GLIB glib-2.0)
if(GLIB_FOUND)
  set(HAVE_GLIB ON)
endif()

# QT
set(QT_MAJOR_VERSION 6 CACHE STRING "Qt version to use (5 or 6), defaults to 6")

option(BUILD_WITH_QT5 "Use Qt 5" OFF)
option(BUILD_WITH_QT6 "Use Qt 6" OFF)

if(BUILD_WITH_QT5)
  set(QT_MAJOR_VERSION 5)
elseif(BUILD_WITH_QT6)
  set(QT_MAJOR_VERSION 6)
else()
  if(QT_MAJOR_VERSION EQUAL 5)
    set(BUILD_WITH_QT5 ON)
  elseif(QT_MAJOR_VERSION EQUAL 6)
    set(BUILD_WITH_QT6 ON)
  else()
    set(BUILD_WITH_QT6 ON)
    set(QT_MAJOR_VERSION 6)
  endif()
endif()

set(QT_DEFAULT_MAJOR_VERSION ${QT_MAJOR_VERSION})

set(QT_COMPONENTS Core Concurrent Network Sql Gui Widgets)

if(QT_MAJOR_VERSION EQUAL 5)
  set(QT_MIN_VERSION 5.12)
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS ${QT_COMPONENTS})

set(QtCore_LIBRARIES Qt${QT_MAJOR_VERSION}::Core)
set(QtConcurrent_LIBRARIES Qt${QT_MAJOR_VERSION}::Concurrent)
set(QtGui_LIBRARIES Qt${QT_MAJOR_VERSION}::Gui)
set(QtWidgets_LIBRARIES Qt${QT_MAJOR_VERSION}::Widgets)
set(QtNetwork_LIBRARIES Qt${QT_MAJOR_VERSION}::Network)
set(QtSql_LIBRARIES Qt${QT_MAJOR_VERSION}::Sql)

# QUAZIP
find_package(QuaZip-Qt6 REQUIRED)

if(BUILD_WITH_QT5 AND Qt5Core_VERSION VERSION_LESS 5.15.0)
  macro(qt_add_resources)
    qt5_add_resources(${ARGN})
  endmacro()
  macro(qt_wrap_cpp)
    qt5_wrap_cpp(${ARGN})
  endmacro()
  macro(qt_wrap_ui)
    qt5_wrap_ui(${ARGN})
  endmacro()
endif()

# QODBC - Build a modified QODBC driver that works with FreeTDS on UNIX renamed QODBCX
if (UNIX AND _Qt5Core_PRIVATE_DIRS_EXIST AND _Qt5Sql_PRIVATE_DIRS_EXIST AND Qt5Sql_VERSION VERSION_GREATER_EQUAL 5.14.1 AND NOT APPLE)
  set(HAVE_QSQLODBCX ON)
  add_subdirectory(3rdparty/qsqlodbc)
  include_directories(3rdparty/qsqlodbc)
endif ()

if(WIN32 AND NOT MSVC)
  # RC compiler
  string(REPLACE "gcc" "windres" CMAKE_RC_COMPILER_INIT ${CMAKE_C_COMPILER})
  enable_language(RC)
  SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> <SOURCE> -I ${CMAKE_SOURCE_DIR}/dist/windows")
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release")
  add_definitions(-DNDEBUG)
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

if(WIN32)
  add_definitions(-DUNICODE)
endif()

# Subdirectories
add_subdirectory(src)
add_subdirectory(dist)

# Uninstall support
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
