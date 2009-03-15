project(kguitar)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories(${KDE4_INCLUDES} ${QT_INCLUDES})

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
else(TSE3_FOUND)
	set(TSE3_LIBRARIES)
	set(TSE3_INCLUDE_DIRS)
endif(TSE3_FOUND)

# ======================================================================

set(WITH_TSE3 ${TSE3_FOUND})

add_subdirectory(doc)
add_subdirectory(kguitar)
add_subdirectory(kguitar_shell)
add_subdirectory(icons)

configure_file(config.h.in kguitar/config.h)
configure_file(config.h.in kguitar_shell/config.h)
