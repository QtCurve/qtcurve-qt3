project(QtCurve-KDE3)
cmake_minimum_required(VERSION 2.4.0)

set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_GENERATOR "DEB;RPM;TGZ")
set(DEBIAN_PACKAGE_DESCRIPTION "QtCurve for Qt3/KDE3")
set(DEBIAN_PACKAGE_SECTION "kde")
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "65")
set(CPACK_PACKAGE_VERSION_PATCH "3")
set(CPACK_PACKAGE_CONTACT "Craig Drummond <craig_p_drummond@yahoo.co.uk>")
set(QTCURVE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set(QTCURVE_VERSION_FULL "${QTCURVE_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${QTCURVE_VERSION_FULL}")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${QTCURVE_VERSION_FULL}")
include(CPack)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

find_package(KDE3 REQUIRED)
include(KDE3Macros)

exec_program(kde-config
             ARGS --prefix
             OUTPUT_VARIABLE CMAKE_INSTALL_PREFIX)

file(TO_CMAKE_PATH "$ENV{KDEDIRS}" _KDEDIRS)

# For KDE4 kde-config has been renamed to kde4-config
find_program(KDE4_KDECONFIG_EXECUTABLE
             NAMES kde4-config
             PATHS ${CMAKE_INSTALL_PREFIX}/bin ${_KDEDIRS} /usr/local/bin /usr/bin /opt/kde4/bin NO_DEFAULT_PATH)

if (NOT KDE4_KDECONFIG_EXECUTABLE)
   find_program(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config )
endif (NOT KDE4_KDECONFIG_EXECUTABLE)

if(KDE4_KDECONFIG_EXECUTABLE)
    exec_program(${KDE4_KDECONFIG_EXECUTABLE}
                 ARGS --prefix
                 OUTPUT_VARIABLE KDE4PREFIX)
endif(KDE4_KDECONFIG_EXECUTABLE)

if(NOT KDE4PREFIX)
    set(KDE4PREFIX ${KDE3PREFIX})
endif(NOT KDE4PREFIX)

configure_file (config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
add_definitions(${QT_DEFINITIONS} ${KDE3_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE3_INCLUDES})

set(DATA_INSTALL_DIR ${KDE3PREFIX}/share/apps)

if(QTC_DEFAULT_TO_KDE3)
    message("\n**************************************************************\n"
            "Information: Defaulting to reading KDE3 settings if \n"
            "             KDE_SESSION_VERSION is not set\n"
            "**************************************************************\n")
else(QTC_DEFAULT_TO_KDE3)
    message("\n**************************************************************\n"
            "Information: Defaulting to reading KDE4 settings if \n"
            "             KDE_SESSION_VERSION is not set\n"
            "**************************************************************\n")
endif(QTC_DEFAULT_TO_KDE3)

add_subdirectory(style)
add_subdirectory(config)
add_subdirectory(themes)
add_subdirectory(colors)
