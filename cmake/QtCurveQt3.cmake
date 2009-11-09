FILE(GLOB GLOB_PATHS_BIN /usr/lib/qt-3*/bin/)
FIND_PATH(QT_PLUGINS_DIR imageformats
  $ENV{QTDIR}/plugins
  ${GLOB_PATHS_BIN}
  /usr/local/qt/plugins
  /usr/lib/qt/plugins
  /usr/lib/qt3/plugins
  /usr/share/qt3/plugins
  )

MACRO(QTCURVE_QT_WRAP_CPP outfiles )
  # get include dirs
  GET_DIRECTORY_PROPERTY(moc_includes_tmp INCLUDE_DIRECTORIES)
  SET(moc_includes)

  FOREACH(it ${ARGN})
    GET_FILENAME_COMPONENT(outfilename ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfilename}.moc)
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
        COMMAND ${QT_MOC_EXECUTABLE}
        ARGS -o ${outfile} ${infile}
        DEPENDS ${infile})
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH(it)

ENDMACRO(QTCURVE_QT_WRAP_CPP)
