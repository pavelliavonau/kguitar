set(kguitar_SRCS
	main.cpp
	kguitar.cpp
)

kde4_add_executable(kguitar ${kguitar_SRCS})

target_link_libraries(kguitar ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

########### install files ###############
install(TARGETS kguitar ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kguitar.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kguitar_shell.rc DESTINATION ${DATA_INSTALL_DIR}/kguitar)
install(FILES kguitar_konq.rc DESTINATION ${DATA_INSTALL_DIR}/kguitar)
