# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# just set the variable
APPSDIR = $(kde_appsdir)/Multimedia
# set the include path for X, qt and KDE
INCLUDES= @all_includes@
# claim, which subdirectories you want to install
SUBDIRS = toolbar

#DEFS = @DEFS@ -I. -I$(srcdir) -I.. -DDOTRACE
####### This part is very kguitar specific
# you can add here more. This one gets installed 
bin_PROGRAMS = kguitar

# Which sources should be compiled for kguitar.
kguitar_SOURCES	= application.cpp chord.cpp fingers.cpp fingerlist.cpp \
 main.cpp trackview.cpp \
 application.moc.cpp chord.moc.cpp fingers.moc.cpp fingerlist.moc.cpp \
 trackview.moc.cpp

# the library search path
kguitar_LDFLAGS = @all_libraries@ @KDE_RPATH@

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kguitar_LDADD  = -lkdeui -lkdecore -lqt -lX11 -lXext @LIBSOCKET@

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = application.h chord.h fingers.h fingerlist.h trackview.h

# just to make sure, automake makes them 
BUILTSOURCES =	application.moc.cpp chord.moc.cpp fingers.moc.cpp \
 fingerlist.moc.cpp trackview.moc.cpp

EXTRA_DIST = kguitar.kdelnk kguitar.xpm kguitar-large.xpm

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
CLEANFILES = $(BUILTSOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ki18n $(kguitar_SOURCES)

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) kguitar.kdelnk $(APPSDIR)
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kguitar.xpm $(kde_icondir)
	$(INSTALL_DATA) kguitar-large.xpm $(kde_icondir)/large/kguitar.xpm
	$(INSTALL_DATA) kguitar-mini.xpm $(kde_icondir)/mini/kguitar.xpm
	$(mkinstalldirs) $(kde_datadir)/kguitar/toolbar/
	$(INSTALL_DATA) toolbar/chord.xpm $(kde_datadir)/kguitar/toolbar/chord.xpm

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/kguitar.kdelnk
	-rm -f $(kde_icondir)/kguitar.xpm
	-rm -f $(kde_icondir)/large/kguitar.xpm
	-rm -f $(kde_datadir)/kguitar/toolbar/chord.xpm

# add a dependency for every moc file to be full portable
application.moc.cpp: application.h
	$(MOC) application.h -o application.moc.cpp

chord.moc.cpp: chord.h
	$(MOC) chord.h -o chord.moc.cpp

fingers.moc.cpp: fingers.h
	$(MOC) fingers.h -o fingers.moc.cpp

fingerlist.moc.cpp: fingerlist.h
	$(MOC) fingerlist.h -o fingerlist.moc.cpp

trackview.moc.cpp: trackview.h
	$(MOC) trackview.h -o trackview.moc.cpp
