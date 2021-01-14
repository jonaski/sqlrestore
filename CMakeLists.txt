# SQL Restore
# Copyright 2019, Jonas Kvinge <jonas@jkvinge.net>

project(sqlrestore)
cmake_minimum_required(VERSION 3.0)
if(CMAKE_VERSION VERSION_GREATER 3.0)
  cmake_policy(SET CMP0054 NEW)
endif()

include(CheckCXXCompilerFlag)
include(CheckIncludeFiles)
include(FindPkgConfig)
include(FindPackageHandleStandardArgs)
include(cmake/Version.cmake)
include(cmake/Rpm.cmake)
include(cmake/Deb.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND COMPILE_OPTIONS
  $<$<COMPILE_LANGUAGE:C>:-std=c99>
  $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
  -U__STRICT_ANSI__
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
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-old-style-cast>
  $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
)

if(APPLE)
  list(APPEND COMPILE_OPTIONS -Wno-unused-parameter)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  list(APPEND COMPILE_OPTIONS -Wno-language-extension-token)
endif()

option(BUILD_WERROR "Build with -Werror" ON)
if(BUILD_WERROR)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif(BUILD_WERROR)

add_compile_options(${COMPILE_OPTIONS})

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions(-DNDEBUG)
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
  #add_definitions(-DQT_NO_WARNING_OUTPUT)
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  set(DEBUG ON)
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
find_package(Backtrace QUIET)
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

option(BUILD_WITH_QT5 "Build with Qt 5")
option(BUILD_WITH_QT6 "Build with Qt 6")

if (NOT BUILD_WITH_QT5 AND NOT BUILD_WITH_QT6)
 set(BUILD_WITH_QT5 ON)
endif()

if(BUILD_WITH_QT5)
  set(QT_MAJOR_VERSION 5)
elseif(BUILD_WITH_QT6)
  set(QT_MAJOR_VERSION 6)
else()
  message(FATAL_ERROR "You need to set either BUILD_WITH_QT5 or BUILD_WITH_QT6.")
endif()

set(QT_COMPONENTS Core Concurrent Widgets Network Sql)

if(QT_MAJOR_VERSION EQUAL 5)
  set(QT_MIN_VERSION 5.9)
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS ${QT_COMPONENTS})

set(QtCore_LIBRARIES Qt${QT_MAJOR_VERSION}::Core)
set(QtConcurrent_LIBRARIES Qt${QT_MAJOR_VERSION}::Concurrent)
set(QtGui_LIBRARIES Qt${QT_MAJOR_VERSION}::Gui)
set(QtWidgets_LIBRARIES Qt${QT_MAJOR_VERSION}::Widgets)
set(QtNetwork_LIBRARIES Qt${QT_MAJOR_VERSION}::Network)
set(QtSql_LIBRARIES Qt${QT_MAJOR_VERSION}::Sql)
set(QT_LIBRARIES Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Concurrent Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Widgets Qt${QT_MAJOR_VERSION}::Network Qt${QT_MAJOR_VERSION}::Sql)

# QUAZIP
find_package(QuaZip REQUIRED)

# QODBC - Build a modified QODBC driver that works with FreeTDS on UNIX renamed QODBCX
if (UNIX AND _Qt5Core_PRIVATE_DIRS_EXIST AND _Qt5Sql_PRIVATE_DIRS_EXIST AND Qt5Sql_VERSION VERSION_GREATER_EQUAL 5.14.1 AND NOT APPLE)
  set(HAVE_QSQLODBCX ON)
  add_subdirectory(3rdparty/qsqlodbc)
  include_directories(3rdparty/qsqlodbc)
endif ()

if (WIN32)
  # RC compiler
  string(REPLACE "gcc" "windres" CMAKE_RC_COMPILER_INIT ${CMAKE_C_COMPILER})
  enable_language(RC)
  SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> <SOURCE> -I ${CMAKE_SOURCE_DIR}/dist/windows")
endif(WIN32)

# Optional bits
if(WIN32)
  option(ENABLE_WIN32_CONSOLE "Show the windows console even outside Debug mode" OFF)
endif(WIN32)


# Subdirectories
add_subdirectory(src)
add_subdirectory(dist)

# Uninstall support
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
