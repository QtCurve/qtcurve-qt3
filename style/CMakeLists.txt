set(qtcurve_style_common_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/config_file.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/colorutils.c )
set_source_files_properties(${qtcurve_style_common_SRCS} PROPERTIES LANGUAGE CXX)

if (NOT QTC_QT_ONLY)
    set(qtcurve_std_SRCS qtcurve.cpp shortcuthandler.cpp pixmaps.h)
    set(qtcurve_SRCS ${qtcurve_std_SRCS} ${qtcurve_style_common_SRCS})
    add_definitions(-DQT_PLUGIN)
    include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/common)
    link_directories(${KDE3_LIB_DIR})
    kde3_automoc(${qtcurve_std_SRCS})
    add_library(qtcurve MODULE ${qtcurve_SRCS})
    set_target_properties(qtcurve PROPERTIES PREFIX "")
    target_link_libraries(qtcurve ${QT_LIBRARIES} kdefx)
    install(TARGETS qtcurve LIBRARY DESTINATION ${KDE3_LIB_DIR}/kde3/plugins/styles)
    install(FILES qtcurve.themerc DESTINATION ${KDE3PREFIX}/share/apps/kstyle/themes)
else (NOT QTC_QT_ONLY)
    set(qtcurve_MOC_CLASSES qtcurve.h shortcuthandler.h qtc_kstyle.h)
    QTCURVE_QT_WRAP_CPP(qtcurve_MOC_SRCS ${qtcurve_MOC_CLASSES})
    set(qtcurve_SRCS qtcurve.cpp shortcuthandler.cpp qtc_kstyle.cpp pixmaps.h ${qtcurve_style_common_SRCS})
    include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${QT_INCLUDE_DIR}  ${CMAKE_SOURCE_DIR}/common)
    add_library(qtcurve MODULE ${qtcurve_SRCS} ${qtcurve_MOC_SRCS})
    set_target_properties(qtcurve PROPERTIES PREFIX "")
    target_link_libraries(qtcurve ${QT_LIBRARIES})
    install(TARGETS qtcurve LIBRARY DESTINATION ${QT_PLUGINS_DIR}/styles)
endif (NOT QTC_QT_ONLY)

