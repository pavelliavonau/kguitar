# Compiling
SYSCONF_CXX		= g++

QTDIR			= /usr

# Compiling with support libraries
SYSCONF_CXXFLAGS_X11	= -I/usr/X11R6/include
SYSCONF_CXXFLAGS_QT	= -I/usr/include/qt
SYSCONF_CXXFLAGS_KDE    = -I/usr/include/kde

# Linking with support libraries
# X11
SYSCONF_LFLAGS_X11	= -L/usr/X11R6/lib
SYSCONF_LIBS_X11	= -lX11 -lXext
# Qt, Qt+OpenGL
SYSCONF_LFLAGS_QT	= -L$(QTDIR)/lib
SYSCONF_LIBS_QT		= -lqt
# KDE
SYSCONF_LFLAGS_KDE      = -L$(KDEDIR)/lib
SYSCONF_LIBS_KDE        = -lkdecore -lkdeui

# Linking applications
SYSCONF_LINK		= g++
SYSCONF_LFLAGS		= 
SYSCONF_LIBS		= 

# Link flags shared objects
SYSCONF_LFLAGS_SHOBJ	= -shared

# Meta-object compiler
SYSCONF_MOC		= /usr/bin/moc

 SYSCONF_CXXFLAGS	= -pipe -O2 -fno-strength-reduce -Wall -w
#SYSCONF_CXXFLAGS	= -pipe -ggdb -fno-strength-reduce

CXX	=	$(SYSCONF_CXX)
CXXFLAGS=	$(SYSCONF_CXXFLAGS_KDE) $(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS)
LFLAGS	=	$(SYSCONF_LFLAGS_KDE) $(SYSCONF_LFLAGS_QT) $(SYSCONF_LFLAGS_X11) $(SYSCONF_LFLAGS)
LIBS	=	$(SYSCONF_LIBS_KDE) $(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_X11) $(SYSCONF_LIBS)
MOC	=	$(SYSCONF_MOC)

####### Target

TARGET	= kguitar
DESTDIR = ./
VER_MAJ = 0
VER_MIN = 1

####### Files

HEADERS =	application.h \
		fingers.h \
		global.h \
		chord.h \
		fingerlist.h
SOURCES =	application.cpp \
		chord.cpp \
		fingers.cpp \
		fingerlist.cpp \
		main.cpp
OBJECTS =	application.o \
		chord.o \
		fingers.o \
		fingerlist.o \
		main.o
SRCMOC	=	application.moc.cpp \
		chord.moc.cpp \
		fingers.moc.cpp \
		fingerlist.moc.cpp
OBJMOC	=	application.moc.o \
		chord.moc.o \
		fingers.moc.o \
		fingerlist.moc.o

####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .C .c

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(OBJECTS) $(OBJMOC) 
	$(SYSCONF_LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)

moc: $(SRCMOC)

tmake:
	tmake application.pro

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC)
	-rm -f *~ core

####### Compile

application.o: application.cpp \
		application.h \
		filesave.xpm \
		fileopen.xpm \
		fileprint.xpm

chord.o: chord.cpp chord.h

fingers.o: fingers.cpp fingers.h

fingerlist.o: fingerlist.cpp fingerlist.h

%.moc.o: %.moc %.h
	$(CXX) $(CXXFLAGS) $(INCPATH) -o $@ $<

%.moc.cpp: %.h
	$(MOC) $*.h -o $*.moc.cpp
