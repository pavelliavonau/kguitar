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

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

KDE_ICON = AUTO

# this Makefile creates both a KPart application and a KPart
#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kguitar
#lib_LTLIBRARIES = libkguitar.la

# the application source, library search path, and link libraries
kguitar_SOURCES = main.cpp kguitar.cpp
#kguitar_SOURCES = main.cpp
kguitar_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kguitar_LDADD   = $(LIB_KPARTS)
#kguitar_LDADD   = $(LIB_KPARTS) libkguitarpart.la
#kguitar_LDADD = $(LIB_KFILE) libkguitar.la

# this is where the desktop file will go
shelldesktopdir   = $(kde_appsdir)/Multimedia
shelldesktop_DATA = kguitar.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kguitar
shellrc_DATA = kguitar_shell.rc

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

#########################################################################
# MusiXTeX SUPPORT SECTION
#########################################################################

# Install kgtabs.tex if configure found TeTeX

install-data-local: $(INSTALLKGTABS)

uninstall-local: $(UNINSTALLKGTABS)

$(INSTALL_KGTABS):
	@echo "Copy kgtabs.tex to $(TEXMF)/tex/generic/kgtabs"
	$(mkinstalldirs) $(TEXMF)/tex/generic/kgtabs
	$(INSTALL_DATA) $(srcdir)/kgtabs.tex $(TEXMF)/tex/generic/kgtabs/kgtabs.tex
	@echo "Running texhash..."
	@texhash

$(UNINSTALL_KGTABS):
	@echo "Delete kgtabs.tex"
	-rm -f $(TEXMF)/tex/generic/kgtabs/kgtabs.tex
	@echo "Running texhash..."
	@texhash
