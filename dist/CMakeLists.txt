execute_process(COMMAND env LC_ALL="en_US.utf8" date "+%a %b %d %Y" OUTPUT_VARIABLE RPM_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND env LC_ALL=C date "+%a, %-d %b %Y %H:%M:%S %z" OUTPUT_VARIABLE DEB_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/windows/sqlrestore.nsi.in ${CMAKE_CURRENT_SOURCE_DIR}/windows/sqlrestore.nsi @ONLY)
