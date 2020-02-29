execute_process(COMMAND env LC_ALL="en_US.utf8" date "+%a %b %d %Y" OUTPUT_VARIABLE RPM_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND env LC_ALL=C date "+%a, %-d %b %Y %H:%M:%S %z" OUTPUT_VARIABLE DEB_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/maketarball.sh.in ${CMAKE_CURRENT_SOURCE_DIR}/scripts/maketarball.sh @ONLY)

if (UNIX AND NOT APPLE)
  install(FILES unix/sqlrestore.desktop DESTINATION share/applications)
  install(FILES ../data/icons/48x48/backup.png DESTINATION share/icons/hicolor/48x48/apps RENAME sqlrestore.png)
  install(FILES ../data/icons/64x64/backup.png DESTINATION share/icons/hicolor/64x64/apps RENAME sqlrestore.png)
  install(FILES ../data/icons/128x128/backup.png DESTINATION share/icons/hicolor/128x128/apps RENAME sqlrestore.png)
endif (UNIX AND NOT APPLE)

if (WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/windows/sqlrestore.nsi.in ${CMAKE_CURRENT_SOURCE_DIR}/windows/sqlrestore.nsi @ONLY)
endif()
