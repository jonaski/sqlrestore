include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)

if(Qt6Core_FOUND)
  pkg_check_modules(QUAZIP QUIET quazip6)
else()
  pkg_check_modules(QUAZIP QUIET quazip5)
endif()

# New names for quazip1
if (NOT QUAZIP_FOUND)
  if(Qt6Core_FOUND)
    pkg_check_modules(QUAZIP QUIET quazip1-qt6)
  else()
    pkg_check_modules(QUAZIP QUIET quazip1-qt5)
  endif()
endif(NOT QUAZIP_FOUND)

if (NOT QUAZIP_FOUND)
  if(Qt6Core_FOUND)
    pkg_check_modules(QUAZIP QUIET QuaZip-Qt6)
  else()
    pkg_check_modules(QUAZIP QUIET QuaZip-Qt5)
  endif()
endif(NOT QUAZIP_FOUND)

if(QUAZIP_LIBRARIES MATCHES "quazip5;Qt5Core")
  set(QUAZIP_LIBRARIES "quazip5")
endif()

if(QUAZIP_LIBRARIES MATCHES "Qt5Quazip;Qt5Core")
  set(QUAZIP_LIBRARIES "Qt5Quazip")
endif()

if(NOT QUAZIP_LIBRARIES)

  if(Qt6Core_FOUND)
    set(QUAZIP_LIBRARY_NAMES libQt6Quazip.dll libQt6Quazip.a Qt6Quazip quazip6 quazip1-qt6 quazip QuaZip-Qt6)
  else()
    set(QUAZIP_LIBRARY_NAMES libQt5Quazip.dll libQt5Quazip.a Qt5Quazip quazip5 quazip1-qt5 quazip QuaZip-Qt5)
  endif()

  find_library(QUAZIP_LIBRARY NAMES ${QUAZIP_LIBRARY_NAMES} HINTS /usr/lib64 /usr/local/lib64 /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
  set(QUAZIP_LIBRARIES ${QUAZIP_LIBRARY})
endif(NOT QUAZIP_LIBRARIES)

if (QUAZIP_LIBRARIES MATCHES ".*.a$" OR CMAKE_CXX_COMPILER MATCHES ".*.static.*")
  add_definitions("-DQUAZIP_STATIC")
endif()

if(NOT QUAZIP_INCLUDE_DIRS MATCHES ".*quazip.*")
  unset(QUAZIP_INCLUDE_DIRS CACHE)
  if(Qt6Core_FOUND)
    set(INCLUDE_DIR_SUFFIXES quazip6 Qt6Quazip QuaZip-Qt6-1.0/quazip quazip)
  else()
    set(INCLUDE_DIR_SUFFIXES quazip5 Qt5Quazip QuaZip-Qt5-1.0/quazip quazip)
  endif()
  find_path(QUAZIP_INCLUDE_DIR quazip.h HINTS /usr/include /usr/local/include /include PATH_SUFFIXES ${INCLUDE_DIR_SUFFIXES})
  set(QUAZIP_INCLUDE_DIRS ${QUAZIP_INCLUDE_DIR})
endif(NOT QUAZIP_INCLUDE_DIRS MATCHES ".*quazip.*")

if(QUAZIP_LIBRARIES AND QUAZIP_INCLUDE_DIRS)
  set(QUAZIP_FOUND 1)
endif(QUAZIP_LIBRARIES AND QUAZIP_INCLUDE_DIRS)

find_package_handle_standard_args(QuaZip DEFAULT_MSG QUAZIP_LIBRARIES QUAZIP_INCLUDE_DIRS)
mark_as_advanced(QUAZIP_INCLUDE_DIRS QUAZIP_LIBRARIES)
