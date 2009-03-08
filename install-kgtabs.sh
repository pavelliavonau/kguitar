#!/bin/sh
#
# Script for installing kgtabs.tex in UNIX systems with TeTeX.
# kgtabs.tex is required to use exported MusiXTeX tabs from KGuitar
#
# http://kguitar.sourceforge.net
# Alex Brand <alinx@users.sourceforge.net>


if [ ! -x /usr/local/bin/kpsetool -a ! -x /usr/bin/kpsetool -a ! -x /bin/kpsetool ]
then
	echo
	echo "***ERROR: Can't find the program: kpsetool"
	echo
	exit 1
fi

echo "Search TeTeX..."
TEXMF=`kpsetool -v '$TEXMF' | sed 's/!//g'`

if [ -d $TEXMF ]
then
	echo "Make Directory..."
	mkdir -p $TEXMF/tex/generic/kgtabs
	echo "Copying kgtabs.tex..."
	cp $PWD/kguitar/kgtabs.tex $TEXMF/tex/generic/kgtabs
	texhash
else
	echo
	echo "***ERROR: Can't install kgtabs.tex"
	echo
fi


