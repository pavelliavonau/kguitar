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
INCLUDES = $(all_includes)

# claim, which subdirectories you want to install
# if you don't have subdirectories, comment it
SUBDIRS = toolbar pics

####### This part is very kguitar specific
# you can add here more. This one gets installed
bin_PROGRAMS = 	kguitar
lib_LTLIBRARIES = libkguitar.la

# Which sources should be compiled for kguitar.
libkguitar_la_SOURCES = application.cpp chord.cpp fingers.cpp fingerlist.cpp \
 trackview.cpp setsong.cpp tabsong.cpp tabtrack.cpp settrack.cpp \
 settabfret.cpp settabdrum.cpp radiustuner.cpp notespinbox.cpp \
 options.cpp chordlistitem.cpp chordlist.cpp timesig.cpp strumming.cpp \
 filebrowser.cpp midilist.cpp kguitar_shell.cpp

METASOURCES = AUTO

# the library search path.
kguitar_SOURCES = main.cpp
kguitar_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kguitar_LDADD   = $(LIB_KFILE) libkguitar.la

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = application.h chord.h fingers.h fingerlist.h trackview.h \
 setsong.h tabsong.h tabtrack.h settrack.h settabfret.h settabdrum.h \
 radiustuner.h notespinbox.h options.h chordlistitem.h chordlist.h \
 timesig.h strumming.h filebrowser.h midilist.h kguitar_shell.h

libkguitar_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module -no-undefined
libkguitar_la_LIBADD = $(LIB_KFILE) -lkparts -lkmid

rcdir = $(kde_datadir)/kguitar
rc_DATA = kguitarui.rc kguitar_part.rc kguitar_konq.rc

applnkdir = $(kde_appsdir)/Multimedia
applnk_DATA = kguitar.desktop

mimedir = $(kde_mimedir)/application
mime_DATA = x-kguitar.desktop

servicesdir = $(kde_servicesdir)
services_DATA = kguitar_component.desktop

#messages:
#	$(XGETTEXT) -C -ki18n $(kguitar_SOURCES) -o ../po/kguitar.pot

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o ../po/kguitar.pot; \
	fi


KDE_ICON = AUTO

# EXTRA_DIST = x-kguitar.desktop

#install-data-local:
#	$(mkinstalldirs) $(kde_mimedir)
#	$(mkinstalldirs) $(kde_mimedir)/application
#	$(INSTALL_DATA) x-kguitar.desktop $(kde_mimedir)/application/x-kguitar.desktop

#uninstall-local:
#	-rm -f $(kde_mimedir)/application/x-kguitar.desktop

# OLD Makefile.am ===========================================================

# EXTRA_DIST = kguitar.kdelnk kguitar.xpm kguitar-large.xpm kguitar-mini.xpm

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
#install-data-local: 
#	$(mkinstalldirs) $(APPSDIR)
#	$(INSTALL_DATA) kguitar.kdelnk $(APPSDIR)
#	$(mkinstalldirs) $(kde_icondir)
#	$(mkinstalldirs) $(kde_minidir)
#	$(mkinstalldirs) $(kde_icondir)/large
#	$(INSTALL_DATA) kguitar.xpm $(kde_icondir)
#	$(INSTALL_DATA) kguitar-large.xpm $(kde_icondir)/large/kguitar.xpm
#	$(INSTALL_DATA) kguitar-mini.xpm $(kde_minidir)/kguitar.xpm

# remove ALL you have installed in install-data-local or install-exec-local
#uninstall-local:
#	-rm -f $(APPSDIR)/kguitar.kdelnk
#	-rm -f $(kde_icondir)/kguitar.xpm
#	-rm -f $(kde_minidir)/kguitar.xpm
#	-rm -f $(kde_icondir)/large/kguitar.xpm


