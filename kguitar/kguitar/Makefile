# Compiling
SYSCONF_CXX		= g++

QTDIR			= /usr

# Compiling with support libraries
SYSCONF_CXXFLAGS_X11	= -I/usr/X11R6/include
SYSCONF_CXXFLAGS_QT	= -I/usr/include/qt
SYSCONF_CXXFLAGS_OPENGL	= -I/usr/X11R6/include

# Compiline YACC output
SYSCONF_CXXFLAGS_YACC     = -Wno-unused -Wno-parentheses

# Linking with support libraries
# X11
SYSCONF_LFLAGS_X11	= -L/usr/X11R6/lib
SYSCONF_LIBS_X11	= -lX11 -lXext
# Qt, Qt+OpenGL
SYSCONF_LFLAGS_QT	= -L$(QTDIR)/lib
SYSCONF_LIBS_QT		= -lqt
SYSCONF_LIBS_QT_OPENGL	= -lqgl
# OpenGL
SYSCONF_LFLAGS_OPENGL	= -L/usr/X11R6/lib
SYSCONF_LIBS_OPENGL	= -lMesaGL -lMesaGLU -lXmu -lXext -lm
# Yacc
SYSCONF_LIBS_YACC	= 

# Linking applications
SYSCONF_LINK		= g++
SYSCONF_LFLAGS		= 
SYSCONF_LIBS		= 

# Link flags shared objects
SYSCONF_LFLAGS_SHOBJ	= -shared

# Meta-object compiler
SYSCONF_MOC		= /usr/bin/moc

# Linking shared libraries
#   - Build the $(TARGET) library, eg. lib$(TARGET).so.0.0
#   - Place target in $(DESTDIR) - which has a trailing /
#   - Usually needs to incorporate $(VER_MAJ) and $(VER_MIN)
#
SYSCONF_LINK_SHLIB	= g++
SYSCONF_LINK_TARGET_SHARED	= lib$(TARGET).so.$(VER_MAJ).$(VER_MIN)
SYSCONF_LINK_LIB_SHARED	=  $(SYSCONF_LINK_SHLIB) -shared -Wl,-soname,lib$(TARGET).so.$(VER_MAJ) \
				     $(LFLAGS) -o $(SYSCONF_LINK_TARGET_SHARED) \
				     $(OBJECTS) $(OBJMOC) $(LIBS); \
				 mv $(SYSCONF_LINK_TARGET_SHARED) $(DESTDIR); \
				 cd $(DESTDIR); \
				 rm -f lib$(TARGET).so lib$(TARGET).so.$(VER_MAJ); \
				 ln -s $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so; \
				 ln -s $(SYSCONF_LINK_TARGET_SHARED) lib$(TARGET).so.$(VER_MAJ)

# Linking static libraries
#   - Build the $(TARGET) library, eg. lib$(TARGET).a
#   - Place target in $(DESTDIR) - which has a trailing /
#
SYSCONF_AR		= ar cqs
SYSCONF_LINK_TARGET_STATIC = lib$(TARGET).a
SYSCONF_LINK_LIB_STATIC	= rm -f $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC) ; \
				 $(SYSCONF_AR) $(DESTDIR)$(SYSCONF_LINK_TARGET_STATIC) $(OBJECTS) $(OBJMOC) 
# Compiling application source
SYSCONF_CXXFLAGS	= -pipe -O2 -fno-strength-reduce -Wall -w
# SYSCONF_CXXFLAGS	= -pipe -ggdb -fno-strength-reduce
# Compiling library source
SYSCONF_CXXFLAGS_LIB	= -O2 -fno-strength-reduce -fPIC
# Compiling shared-object source
SYSCONF_CXXFLAGS_SHOBJ	= -O2 -fno-strength-reduce -fPIC
# Default link type (stati linking is still be used where required)
SYSCONF_LINK_LIB	= $(SYSCONF_LINK_LIB_SHARED)
SYSCONF_LINK_TARGET	= $(SYSCONF_LINK_TARGET_SHARED)

####### End of automatically generated section
####### END

####### Compiler, tools and options

CXX	=	$(SYSCONF_CXX)
CXXFLAGS=	$(SYSCONF_CXXFLAGS_QT) $(SYSCONF_CXXFLAGS)
LFLAGS	=	$(SYSCONF_LFLAGS_QT) $(SYSCONF_LFLAGS_X11) $(SYSCONF_LFLAGS)
LIBS	=	$(SYSCONF_LIBS_QT) $(SYSCONF_LIBS_X11) $(SYSCONF_LIBS)
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
