Name: kguitar
Summary: KGuitar -- Some description
Version: 0.4.1
Release: 1
Copyright: GPL
Group: X11/KDE/Utilities
Source: ftp.kde.org/pub/kde/unstable/apps/utils/kguitar-0.4.1.tar.gz
Packager: Mikhail Yakshin <greycat@users.sourceforge.net>
BuildRoot: /tmp/kguitar-0.4.1
Prefix: /opt/kde2

%description
A long description

%prep
rm -rf $RPM_BUILD_ROOT
%setup -n kguitar-0.4.1

%build
./configure --disable-debug --enable-final --prefix=%{prefix}
make

%install
make DESTDIR=$RPM_BUILD_ROOT install
find . -type f -o -type l | sed 's|^\.||' > $RPM_BUILD_ROOT/master.list

%clean
rm -rf $RPM_BUILD_ROOT

%files -f $RPM_BUILD_ROOT/master.list
