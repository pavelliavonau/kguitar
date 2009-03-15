#########################################################################
# KPART SECTION
#########################################################################
#kde_module_LTLIBRARIES = libkguitarpart.la

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS -fexceptions)

set(kguitar_PART_SRCS
	kguitar_part.cpp
	songview.cpp
	chord.cpp
	fingers.cpp
	fingerlist.cpp
	trackview.cpp
	setsong.cpp
	tabsong.cpp
	tabtrack.cpp
	tabcolumn.cpp
	settrack.cpp
	settabmidibase.ui
	settabmidi.cpp
	settabfret.cpp
	settabdrum.cpp
	radiustuner.cpp
	notespinbox.cpp
	chordlistitem.cpp
	chordlist.cpp
	timesig.cpp
	strumming.cpp
	tracklist.cpp
	trackpane.cpp
	trackdrag.cpp
	trackviewcommands.cpp
	songviewcommands.cpp
	songprint.cpp
	musicxml.cpp
	rhythmer.cpp
	accidentals.cpp
	keysig.cpp
	trackprint.cpp
	fretboard.cpp
	melodyeditor.cpp
	options.cpp
	optionspage.cpp
	optionsmusictheory.cpp
	optionsmelodyeditor.cpp
	optionsexportmusixtex.cpp
	optionsmidi.cpp
	optionsprinting.cpp
	playbacktracker.cpp
	chordanalyzer.cpp
	optionsexportascii.cpp
	settings.cpp
	convertascii.cpp
	convertxml.cpp
	convertkg.cpp
	convertmidi.cpp
	converttse3.cpp
	converttex.cpp
	convertgtp.cpp
	kgfontmap.cpp
)

kde4_add_ui_files(kguitar_PART_SRCS
	settabmidibase.ui
)

kde4_add_plugin(kguitarpart WITH_PREFIX ${kguitar_PART_SRCS})

target_link_libraries(kguitarpart
	${KDE4_KDEUI_LIBS}
	${KDE4_KPARTS_LIBS}
	${KDE4_KDE3SUPPORT_LIBS}
	${TSE3_LIBRARIES}
)

install(FILES kguitar_part.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kguitar_part.rc DESTINATION ${DATA_INSTALL_DIR}/kguitarpart)

install(TARGETS kguitarpart DESTINATION ${PLUGIN_INSTALL_DIR})

add_subdirectory(pics)
add_subdirectory(toolbar)
