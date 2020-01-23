include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)

pkg_check_modules(QUAZIP QUIET quazip5)
if (NOT QUAZIP_FOUND)
  pkg_check_modules(QUAZIP QUIET quazip)
endif(NOT QUAZIP_FOUND)

if(QUAZIP_LIBRARIES MATCHES "quazip5;Qt5Core")
  set(QUAZIP_LIBRARIES "quazip5")
endif()

if(NOT QUAZIP_LIBRARIES)
  if(APPLE)
    set(QUAZIPLIB "quazip")
  else(APPLE)
    set(QUAZIPLIB "quazip5")
  endif(APPLE)
  find_library(QUAZIP_LIBRARY NAMES ${QUAZIPLIB} libquazip5.a libquazip.a quazip5.dll quazip.dll HINTS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64 /usr/lib/x86_64-linux-gnu)
  set(QUAZIP_LIBRARIES ${QUAZIP_LIBRARY})
endif(NOT QUAZIP_LIBRARIES)

if (QUAZIP_LIBRARIES MATCHES ".*.a$")
  add_definitions("-DQUAZIP_STATIC")
endif()

if(NOT QUAZIP_INCLUDE_DIRS MATCHES ".*quazip.*")
  unset(QUAZIP_INCLUDE_DIRS CACHE)
  find_path(QUAZIP_INCLUDE_DIR quazip.h HINTS /usr/include /usr/local/include /include PATH_SUFFIXES quazip5 quazip)
  set(QUAZIP_INCLUDE_DIRS ${QUAZIP_INCLUDE_DIR})
endif(NOT QUAZIP_INCLUDE_DIRS MATCHES ".*quazip.*")

if(QUAZIP_LIBRARIES AND QUAZIP_INCLUDE_DIRS)
  set(QUAZIP_FOUND 1)
endif(QUAZIP_LIBRARIES AND QUAZIP_INCLUDE_DIRS)

find_package_handle_standard_args(QuaZip DEFAULT_MSG QUAZIP_LIBRARIES QUAZIP_INCLUDE_DIRS)
mark_as_advanced(QUAZIP_INCLUDE_DIRS QUAZIP_LIBRARIES)
