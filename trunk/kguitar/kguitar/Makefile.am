# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to.
# kde_icondir       Where your icon should go to.
# kde_sounddir      Where system sounds should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES = $(all_includes) $(TSE3INCLUDE)

# claim, which subdirectories you want to install
# if you don't have subdirectories, comment it
SUBDIRS = toolbar pics

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

KDE_ICON = AUTO

# this Makefile creates both a KPart application and a KPart
#########################################################################
# KPART SECTION
#########################################################################
kde_module_LTLIBRARIES = libkguitarpart.la

# the Part's source, library search path, and link libraries
libkguitarpart_la_SOURCES = kguitar_part.cpp songview.cpp chord.cpp   \
fingers.cpp fingerlist.cpp trackview.cpp setsong.cpp tabsong.cpp      \
tabtrack.cpp tabcolumn.cpp settrack.cpp settabmidibase.ui	      \
settabmidi.cpp settabfret.cpp settabdrum.cpp radiustuner.cpp	      \
notespinbox.cpp chordlistitem.cpp chordlist.cpp timesig.cpp	      \
strumming.cpp tracklist.cpp trackpane.cpp trackdrag.cpp		      \
trackviewcommands.cpp songviewcommands.cpp songprint.cpp musicxml.cpp \
rhythmer.cpp accidentals.cpp keysig.cpp trackprint.cpp fretboard.cpp  \
melodyeditor.cpp options.cpp optionspage.cpp optionsmusictheory.cpp   \
optionsmelodyeditor.cpp optionsexportmusixtex.cpp optionsmidi.cpp     \
optionsprinting.cpp playbacktracker.cpp chordanalyzer.cpp	      \
optionsexportascii.cpp settings.cpp convertascii.cpp kgfontmap.cpp

libkguitarpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkguitarpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) $(LIBTSE3) -lkdeprint

#libkguitar_la_LDFLAGS = $(all_libraries) -module -no-undefined

# this is where the desktop file will go 
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = kguitar_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/kguitar
partrc_DATA = kguitar_part.rc

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = kguitar_part.h songview.h chord.h fingers.h	    \
fingerlist.h trackview.h setsong.h tabsong.h tabtrack.h tabcolumn.h \
settabmidi.h settrack.h settabfret.h settabdrum.h radiustuner.h	    \
notespinbox.h chordlistitem.h chordlist.h timesig.h strumming.h	    \
tracklist.h trackpane.h trackdrag.h trackviewcommands.h		    \
songviewcommands.h songprint.h musicxml.h rhythmer.h accidentals.h  \
keysig.h trackprint.h fretboard.h melodyeditor.h options.h	    \
optionspage.h optionsmelodyeditor.h optionsmusictheory.h	    \
optionsexportmusixtex.h optionsmidi.h optionsprinting.h		    \
playbacktracker.h chordanalyzer.h optionsexportascii.h settings.h   \
convertascii.h convertbase.h kgfontmap.h

#rc_DATA = kguitar_shell.rc kguitar_part.rc kguitar_konq.rc

mimedir = $(kde_mimedir)/application
mime_DATA = x-kguitar.desktop

#########################################################################
# MISC SECTION
#########################################################################

#messages:
#    $(XGETTEXT) -C -ki18n $(kguitar_SOURCES) -o ../po/kguitar.pot

messages:
	LIST=`find . -name \*.h -o -name \*.cpp`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) -ki18n $$LIST -o ../po/kguitar-0.4.9.pot; \
	fi

#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kguitar.pot

#here we need exceptions
songview.lo: songview.cpp
	$(LTCXXCOMPILE) $(USE_EXCEPTIONS) -c $<
