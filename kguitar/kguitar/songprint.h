/***************************************************************************
 * songprint.h: definition of SongPrint class
 *
 * This file is part of KGuitar, a KDE tabulature editor
 *
 * copyright (C) 2002-2003 the KGuitar development team
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
#include <qpen.h>
#include <qstring.h>

#include "accidentals.h"

class KPrinter;
class QPainter;
class TabSong;
class TabTrack;

/***************************************************************************
 * class SongPrint
 ***************************************************************************/

class SongPrint {
public:
	SongPrint();
	~SongPrint();
	void printSong(KPrinter *printer, TabSong *song);
private:
	int barExpWidth(int bn, TabTrack *trk);
	int barWidth(int bn, TabTrack *trk);
	void calcBeams(TabTrack *trk);
	int colWidth(int cl, TabTrack *trk);
	void drawBar(int bn, TabTrack *trk, int es);
	void drawBarLns(int w, TabTrack *trk);
	void drawBeam(int x1, int x2, int y, char tp, char dir);
	void drawBeams(int bn, char dir, TabTrack *trk);
	void drawKey(int l, TabTrack *trk);
	void drawKeySig(TabTrack *trk);
	void drawLetRing(int x, int y);
	void drawNtHdCntAt(int x, int y, int t, Accidentals::Accid a);
	void drawNtStmCntAt(int x, int yl, int yh, int t, char dir);
	void drawPageHdr(int n, TabSong *song);
	void drawRstCntAt(int x, int y, int t);
	void drawStLns(int w);
	void drawStrCntAt(int x, int y, const QString s);
	void drawXpos();
	int eraWidth(const QString s);
	bool findHiLo(int cl, int v, TabTrack *trk, int & hi, int & lo);
	void initFonts();
	void initMetrics(KPrinter *printer);
	void initPens();
	void initPrStyle();
	int line(const QString step, int oct);
	// Almost all functions use a pointer to the same painter, instead of
	// making it a parameter for all functions make it a member variable
	QPainter *p;
	// Variables describing paper dimensions
	int pprh;					// height
	int pprw;					// width
	// Variables describing staff dimensions
	int wNote;					// width 1/4 notehead
	int ystepst;				// y step from line to line
	// Variables describing tab bar dimensions
	int ysteptb;				// y step from line to line
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
	// Variables describing layout of the page header
	int hdrh1;					// height line 1 (title/author/pagenr)
	int hdrh2;					// height line 2 (transcriber)
	int hdrh3;					// space between line 2 and music or trkname
	int hdrh4;					// height trkname (top of text to top of music)
	// Fonts used
	bool fFetaFnd;				// true if feta fonts found
	QFont fFeta;				// used for notes on the staff
	QFont fFetaNr;				// used for time signature on the staff
	QFont fHdr1;				// used for headers (title/author)
	QFont fHdr2;				// used for headers (pagenr/trkname)
	QFont fHdr3;				// used for headers (transcriber)
	QFont fTBar1;				// used for notes on the tab bar
	QFont fTBar2;				// used for notes on the tab bar
	QFont fTSig;				// used for time signature
	// Pens used
	QPen pLnBl;					// used for black lines & text
	QPen pLnWh;					// used for white lines
	// The current write location
	int xpos;
	int yposst;					// on the staff
	int ypostb;					// on the tab bar
	// Variables describing printing style
	bool stNts;					// print notes
	bool stTab;					// print tab
};

#endif // SONGPRINT_H
