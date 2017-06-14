cmake_policy(SET CMP0063 NEW)

set(kguitar_SRCS
	main.cpp
	kguitar.cpp
)

add_executable(kguitar ${kguitar_SRCS})

set_target_properties(kguitar PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# TODO: determine required modules
target_link_libraries(kguitar
	KF5::Parts
	)

########### install files ###############
install(TARGETS kguitar ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.kguitar.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kguitar_shell.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kguitar)
install(FILES kguitar_konq.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kguitar)
