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

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(LINUX ON)
endif()
if (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
  set(FREEBSD ON)
endif()
if (${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
  set(OPENBSD ON)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -U__STRICT_ANSI__ -Wall -Wextra -Wpedantic -Woverloaded-virtual -fpermissive")

option(BUILD_WERROR "Build with -Werror" ON)
if(BUILD_WERROR)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif(BUILD_WERROR)

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

find_library(MAGIC_LIBRARIES NAMES magic libmagic.dll HINTS /usr/lib /usr/lib64)

pkg_check_modules(GLIB glib-2.0)
if(GLIB_FOUND)
  set(HAVE_GLIB ON)
endif()

# QT
set(QT_MIN_VERSION 5.9)
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Widgets Network Sql Concurrent)
set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Network Qt5::Sql Qt5::Concurrent)

# SingleApplication
find_package(SingleApplication QUIET)
find_package(SingleCoreApplication QUIET)

if (SINGLEAPPLICATION_FOUND AND SINGLECOREAPPLICATION_FOUND)
  option(USE_SYSTEM_SINGLEAPPLICATION "Use system SingleApplication/SingleCoreApplication libraries" OFF)
else(SINGLEAPPLICATION_FOUND AND SINGLECOREAPPLICATION_FOUND)
  option(USE_SYSTEM_SINGLEAPPLICATION "Use system SingleApplication/SingleCoreApplication libraries" OFF)
endif(SINGLEAPPLICATION_FOUND AND SINGLECOREAPPLICATION_FOUND)

if(USE_SYSTEM_SINGLEAPPLICATION)
  if (NOT SINGLEAPPLICATION_FOUND OR NOT SINGLECOREAPPLICATION_FOUND)
    message(FATAL_ERROR "Missing SingleApplication or SingleCoreApplication")
  endif()
  message(STATUS "Using system SingleApplication and SingleCoreApplication libraries")
else(USE_SYSTEM_SINGLEAPPLICATION)
  message(STATUS "Using builtin SingleApplication and SingleCoreApplication libraries")
  add_subdirectory(3rdparty/singleapplication)
  set(SINGLEAPPLICATION_LIBRARIES singleapplication)
  set(SINGLECOREAPPLICATION_LIBRARIES singlecoreapplication)
  set(SINGLEAPPLICATION_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/singleapplication)
  set(SINGLECOREAPPLICATION_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/singleapplication)
  unset(SINGLEAPPLICATION_LIBRARY_DIRS)
  unset(SINGLECOREAPPLICATION_LIBRARY_DIRS)
endif(USE_SYSTEM_SINGLEAPPLICATION)

# QUAZIP
find_package(QuaZip)


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

if(APPLE)
  option(USE_BUNDLE "Bundle macOS dependencies" OFF)
elseif(WIN32)
  option(USE_BUNDLE "Bundle Windows dependencies" ON)
endif()
if (APPLE AND USE_BUNDLE AND NOT USE_BUNDLE_DIR)
  set(USE_BUNDLE_DIR "../PlugIns")
endif()


# Subdirectories
add_subdirectory(src)
add_subdirectory(dist)

# Uninstall support
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
