project(kguitar)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

include_directories(${KDE4_INCLUDES} ${QT_INCLUDES})

add_subdirectory(doc)
add_subdirectory(kguitar)
add_subdirectory(kguitar_shell)
add_subdirectory(icons)

CONFIGURE_FILE(config.h.in kguitar/config.h)
CONFIGURE_FILE(config.h.in kguitar_shell/config.h)
