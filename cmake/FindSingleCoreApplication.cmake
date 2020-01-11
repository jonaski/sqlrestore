include(FindPackageHandleStandardArgs)

find_package(PkgConfig QUIET)

pkg_check_modules(SINGLECOREAPPLICATION QUIET singlecoreapplication)

if(NOT SINGLECOREAPPLICATION_LIBRARIES)
  find_library(SINGLECOREAPPLICATION_LIBRARY NAMES singlecoreapplication singlecoreapplication.dll HINTS /usr/lib /usr/lib64 /usr/lib/x86_64-linux-gnu)
  set(SINGLECOREAPPLICATION_LIBRARIES ${SINGLECOREAPPLICATION_LIBRARY})
endif(NOT SINGLECOREAPPLICATION_LIBRARIES)

if(NOT SINGLECOREAPPLICATION_INCLUDE_DIRS)
  find_path(SINGLECOREAPPLICATION_INCLUDE_DIR singlecoreapplication.h HINTS /usr/include /usr/local/include /include PATH_SUFFIXES singlecoreapplication)
  set(SINGLECOREAPPLICATION_INCLUDE_DIRS ${SINGLECOREAPPLICATION_INCLUDE_DIR})
endif(NOT SINGLECOREAPPLICATION_INCLUDE_DIRS)

if(SINGLECOREAPPLICATION_LIBRARIES AND SINGLECOREAPPLICATION_INCLUDE_DIRS)
  set(SINGLECOREAPPLICATION_FOUND 1)
endif(SINGLECOREAPPLICATION_LIBRARIES AND SINGLECOREAPPLICATION_INCLUDE_DIRS)

find_package_handle_standard_args(SingleCoreApplication DEFAULT_MSG SINGLECOREAPPLICATION_LIBRARIES SINGLECOREAPPLICATION_INCLUDE_DIRS)
mark_as_advanced(SINGLECOREAPPLICATION_LIBRARIES SINGLECOREAPPLICATION_INCLUDE_DIRS)
