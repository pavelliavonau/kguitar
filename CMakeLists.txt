project(kguitar)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.5.0")

set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(ECM 1.7.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)

# TODO: determine required modules
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Widgets Xml)
find_package(KF5 REQUIRED COMPONENTS
    Config
#    ConfigWidgets
#    DBusAddons
#    DocTools
#    IconThemes
    I18n
#    IdleTime
#    Notifications
#    NotifyConfig
#    TextWidgets
#    XmlGui
#    WindowSystem
    KDELibs4Support
    Parts
    )

# ======================================================================

# - Find TSE3
# Find the TSE3 headers and libraries.
#
#  TSE3_INCLUDE_DIRS - where to find tse3/*.h, etc.
#  TSE3_LIBRARIES    - List of libraries when using TSE3.
#  TSE3_FOUND        - True if TSE3 is found.

# Look for the header file.
find_path(TSE3_INCLUDE_DIR NAMES tse3/TSE3.h)
mark_as_advanced(TSE3_INCLUDE_DIR)

# Look for the library.
find_library(TSE3_LIBRARY NAMES tse3)
mark_as_advanced(TSE3_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set TSE3_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TSE3 DEFAULT_MSG TSE3_LIBRARY TSE3_INCLUDE_DIR)

if(TSE3_FOUND)
	set(TSE3_LIBRARIES ${TSE3_LIBRARY})
	set(TSE3_INCLUDE_DIRS ${TSE3_INCLUDE_DIR})
endif(TSE3_FOUND)

# ======================================================================

option(WITH_TSE3 "Build KGuitar with TSE3 midi sequencer support" ${TSE3_FOUND})
if(WITH_TSE3 AND NOT TSE3_FOUND)
	message(FATAL_ERROR "WITH_TSE3 enabled but not found")
endif(WITH_TSE3 AND NOT TSE3_FOUND)
if(WITH_TSE3)
	# for moc can properly generate playbacktracker.h
	add_definitions(-DWITH_TSE3=1)
endif(WITH_TSE3)

# TSE3 and other code requires exceptions enabled
kde_enable_exceptions()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

add_definitions(
    -D_DEFAULT_SOURCE
    -DQT_DEPRECATED_WARNINGS
    -DKDE_DEPRECATED_WARNINGS
    )

add_subdirectory(kguitar)
add_subdirectory(kguitar_shell)
add_subdirectory(icons)
add_subdirectory(doc)
add_subdirectory(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

configure_file(config.h.in kguitar/config.h)
configure_file(config.h.in kguitar_shell/config.h)

