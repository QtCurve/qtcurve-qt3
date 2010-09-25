project(QtCurve-KDE3)
cmake_minimum_required(VERSION 2.4.0)

set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_GENERATOR "DEB;RPM;TGZ")
set(DEBIAN_PACKAGE_DESCRIPTION "QtCurve for Qt3/KDE3")
set(DEBIAN_PACKAGE_SECTION "kde")
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "6")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_CONTACT "Craig Drummond <craig.p.drummond@gmail.com>")
set(QTCURVE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set(QTCURVE_VERSION_FULL "${QTCURVE_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${QTCURVE_VERSION_FULL}")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${QTCURVE_VERSION_FULL}")
include(CPack)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if (QTC_BUILD_CONFIG_MODULE)
    if (NOT QTC_QT_ONLY)
        find_package(KDE3)
        if (NOT KDE3_FOUND)
            set(QTC_QT_ONLY true)
            message("INFO: KDE3 not found! Compiling for Qt only! (Not building the config module)")
        endif (NOT KDE3_FOUND)
    endif (NOT QTC_QT_ONLY)
else (QTC_BUILD_CONFIG_MODULE)
    set(QTC_QT_ONLY true)
endif (QTC_BUILD_CONFIG_MODULE)

if (NOT QTC_QT_ONLY)
    include(KDE3Macros)
else (NOT QTC_QT_ONLY)
    set(QT_MT_REQUIRED TRUE)
    find_package(Qt3 REQUIRED)
    set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )
    include(QtCurveQt3)
    if (QT_PLUGINS_DIR MATCHES NOTFOUND)
        message(SEND_ERROR "ERROR: Failed to detemine Qt3 plugins folder. Please pass -DQT_PLUGINS_DIR=<folder> to cmake")
    endif (QT_PLUGINS_DIR MATCHES NOTFOUND)
endif (NOT QTC_QT_ONLY)

if (NOT QTC_QT_ONLY)
exec_program(kde-config
             ARGS --prefix --expandvars
             OUTPUT_VARIABLE CMAKE_INSTALL_PREFIX)
endif (NOT QTC_QT_ONLY)

exec_program(kde-config
             ARGS --install locale --expandvars
             OUTPUT_VARIABLE KDE3_LOCALE_PREFIX)

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
if (NOT QTC_QT_ONLY)
    add_definitions(${QT_DEFINITIONS} ${KDE3_DEFINITIONS})
    include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE3_INCLUDES})
else (NOT QTC_QT_ONLY)
    add_definitions(${QT_DEFINITIONS})
    include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
endif (NOT QTC_QT_ONLY)

set(DATA_INSTALL_DIR ${KDE3PREFIX}/share/apps)

if(QTC_DEFAULT_TO_KDE3)
    message("INFO: Defaulting to reading KDE3 settings if KDE_SESSION_VERSION is not set")
else(QTC_DEFAULT_TO_KDE3)
    message("INFO: Defaulting to reading KDE4 settings if KDE_SESSION_VERSION is not set")
endif(QTC_DEFAULT_TO_KDE3)

add_subdirectory(style)
if (NOT QTC_QT_ONLY)
    if (QTC_BUILD_CONFIG_MODULE)
        message("**************************************************************\n"
                "Warning: The KDE3 config module is deprecated, and is no longer\n"
                "         actively maintained. This module NO LONGER COMPILES!\n"
                "         It is recommended to use the KDE4 version instead.\n"
                "**************************************************************\n")
        add_subdirectory(config)
        add_subdirectory(po)
    endif (QTC_BUILD_CONFIG_MODULE)
    add_subdirectory(themes)
    add_subdirectory(colors)
endif (NOT QTC_QT_ONLY)
