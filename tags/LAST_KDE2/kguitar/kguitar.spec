# Note that this is NOT a relocatable package
%define ver      0.4.1
%define rel      1
%define prefix   /usr

Summary:   KDE guitarist tabulature environment
Name:      kguitar 
Version:   %ver
Release:   %rel
Copyright: GPL
Group:     Applications/Multimedia
Source0:   kguitar-%{PACKAGE_VERSION}.tar.bz2
URL:       http://kguitar.sourceforge.net/ 
BuildRoot: /tmp/kguitar-%{PACKAGE_VERSION}-root
Vendor:    KGuitar team <yakshin@online.ru>
Packager:  Nicolas Vignal <nicolas.vignal@fnac.net>
Docdir: %{prefix}/doc

%description
KGuitar is a free full-featured KDE guitarist environment, that's
built using tabulature concepts. It includes tab editor, chord
construction tools, supports import/export of popular tab formats
and offers MIDI capabilities.

Install KGuitar if you play guitar or any other fretted instrument
and have to work with chords and tabulatures.

%prep
rm -rf %{builddir}

%setup
touch `find . -type f`

%build
if [ -z "$KDEDIR" ]; then
        export KDEDIR=%{prefix}
fi
CXXFLAGS="$RPM_OPT_FLAGS" CFLAGS="$RPM_OPT_FLAGS" ./configure \
	--prefix=$KDEDIR --with-install-root=$RPM_BUILD_ROOT
make

%install
if [ -z "$KDEDIR" ]; then
        export KDEDIR=%{prefix}
fi
rm -rf $RPM_BUILD_ROOT
make install-strip

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > \
	$RPM_BUILD_DIR/file.list.%{name}
find . -type f | sed -e 's,^\.,\%attr(-\,root\,root) ,' \
	-e '/\/config\//s|^|%config|' >> \
	$RPM_BUILD_DIR/file.list.%{name}
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> \
	$RPM_BUILD_DIR/file.list.%{name}
echo "%docdir $KDEDIR/doc/kde" >> $RPM_BUILD_DIR/file.list.%{name}

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf %{builddir}
rm -f $RPM_BUILD_DIR/file.list.%{name}

%files -f ../file.list.%{name}
