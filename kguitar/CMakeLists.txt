#########################################################################
# KPART SECTION
#########################################################################
#kde_module_LTLIBRARIES = libkguitarpart.la

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(kguitar_PART_SRCS
	kguitar_part.cpp
	songview.cpp
	ui/chord/chordeditor.cpp
	ui/chord/chordlistitem.cpp
	ui/chord/fingering.cpp
	ui/chord/fingerlist.cpp
	ui/chord/strumming.cpp
	ui/chord/chordanalyzer.cpp
	trackview.cpp
	ui/setsong.cpp
	data/tabsong.cpp
	data/tabtrack.cpp
	data/tabcolumn.cpp
	settrack.cpp
	settabmidibase.ui
	settabmidi.cpp
	settabfret.cpp
	settabdrum.cpp
	radiustuner.cpp
	notespinbox.cpp
	timesig.cpp
	tracklist.cpp
	trackpane.cpp
	trackdrag.cpp
	trackviewcommands.cpp
	songviewcommands.cpp
	songprint.cpp
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
	optionsexportascii.cpp
	settings.cpp
	convert/convertascii.cpp
	convert/convertxml.cpp
	convert/convertkg.cpp
	convert/convertmidi.cpp
	convert/converttse3.cpp
	convert/converttex.cpp
	convert/convertgtp.cpp
	convert/musicxml.cpp
	kgfontmap.cpp
	ui/rhythmeditor.cpp
)

kde4_add_ui_files(kguitar_PART_SRCS
	settabmidibase.ui
	ui/rhythmeditor.ui
	ui/setsong.ui
)

kde4_add_plugin(kguitarpart WITH_PREFIX ${kguitar_PART_SRCS})

target_link_libraries(kguitarpart
	${KDE4_KDEUI_LIBS}
	${KDE4_KPARTS_LIBS}
	${KDE4_KDE3SUPPORT_LIBS}
)
if(WITH_TSE3)
	target_link_libraries(kguitarpart ${TSE3_LIBRARIES})
endif(WITH_TSE3)

install(FILES kguitar_part.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kguitar_part.rc DESTINATION ${DATA_INSTALL_DIR}/kguitarpart)

install(TARGETS kguitarpart DESTINATION ${PLUGIN_INSTALL_DIR})

add_subdirectory(pics)
add_subdirectory(toolbar)
