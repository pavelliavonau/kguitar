Summary: KDE guitarist environment, based on MIDI and tabulature concepts.
Name: kguitar
Version: 0.2
Copyright: GPL
Group: Applications/Multimedia
Source: http://downloads.sourceforge.net/kguitar/kguitar-0.2.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot

%description
KGuitar is a free full-featured KDE guitarist environment, that's
built using tabulature concepts. It includes tab editor, chord
construction tools, supports import/export of popular tab formats
and offers MIDI capabilities.

Install KGuitar if you play guitar or any other fretted instrument
and have to work with chords and tabulatures.

%prep
%setup -q
./configure

%build
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
make install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README COPYING ChangeLog

/usr/bin/kguitar
/usr/share/apps/kguitar/toolbar

%changelog
* Sun Mar 21 1999 Cristian Gafton <gafton@redhat.com> 
- auto rebuild in the new build environment (release 3)

* Wed Feb 24 1999 Preston Brown <pbrown@redhat.com> 
- Injected new description and group.

[ Some changelog entries trimmed for brevity.  -Editor. ]
>