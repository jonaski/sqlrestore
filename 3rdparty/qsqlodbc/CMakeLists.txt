cmake_minimum_required(VERSION 3.0)

add_definitions(-DQT_PLUGIN -DQT_STATICPLUGIN -DQT_NO_CAST_TO_ASCII -DQT_NO_CAST_FROM_ASCII -DUNICODE)

set(QODBC-SOURCES qsql_odbc_main.cpp qsql_odbc.cpp)
set(QODBC-MOC-HEADERS qsql_odbc_main.h qsql_odbc_p.h)
qt5_wrap_cpp(QODBC-SOURCES-MOC ${QODBC-MOC-HEADERS})

if(NOT _Qt5Core_PRIVATE_DIRS_EXIST)
  message(FATAL_ERROR "Mising Qt5 Core private headers.")
endif()

if(NOT _Qt5Sql_PRIVATE_DIRS_EXIST)
  message(FATAL_ERROR "Mising Qt5 Sql private headers.")
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src ${Qt5Core_PRIVATE_INCLUDE_DIRS} ${Qt5Sql_PRIVATE_INCLUDE_DIRS})

add_library(qsqlodbc STATIC ${QODBC-SOURCES} ${QODBC-SOURCES-MOC})
set_property(TARGET qsqlodbc PROPERTY QT_STATICPLUGIN 1)
target_link_libraries(qsqlodbc odbc Qt5::Core Qt5::Sql)
