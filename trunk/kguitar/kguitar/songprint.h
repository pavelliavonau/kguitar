/***************************************************************************
 * songprint.h: definition of SongPrint class
 *
 * This file is part of KGuitar, a KDE tabulature editor
 *
 * copyright (C) 2002 the KGuitar development team
 ***************************************************************************/

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file COPYING for more information.
 ***************************************************************************/

#ifndef SONGPRINT_H
#define SONGPRINT_H

#include <qfont.h>
#include <qstring.h>

class KPrinter;
class QPainter;
class TabSong;
class TabTrack;

class SongPrint
{
public:
	SongPrint();
	~SongPrint();
	int barExpWidth(int bn, TabTrack *trk);
	int barWidth(int bn, TabTrack *trk);
	int colWidth(int cl, TabTrack *trk);
	void drawBar(int bn, TabTrack *trk, int es);
	void drawBarLns(int w, TabTrack *trk);
	void drawKey(int l, TabTrack *trk);
	void drawStrFccAt(int x, int y, const QString s);
	void initMetrics(KPrinter *printer);
	void printSong(KPrinter *printer, TabSong *song);
private:
	// Almost all functions use a pointer to the same painter, instead of
	// making it a parameter for all functions make it a member variable
	QPainter *p;
	// Variables describing paper dimensions
	int pprh;					// height
	int pprw;					// width
	// Variables describing tab bar dimensions
	int ystep;					// y step from line to line
	int br8h;					// bounding box "8" height
	int br8w;					// bounding box "8" width
	// Variables describing fields within the tab bar
	// ...fw is field width
	// ...pp is print position within field
	// tab.. is the TAB key
	// tsg.. is the time signature
	// nt0.. is space before the first note
	// ntl.. is space after the last note
	int tabfw;
	int tabpp;
	int tsgfw;
	int tsgpp;
	int nt0fw;
	int ntlfw;
	// Fonts used
	QFont fTBar;				// used for notes on the tab bar
	// The current write location
	int xpos;
	int ypos;
};

#endif // SONGPRINT_H
