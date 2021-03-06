/***************************************************************************
 * trackprint.h: definition of TrackPrint class
 *
 * This file is part of KGuitar, a KDE tabulature editor
 *
 * copyright (C) 2003-2004 the KGuitar development team
 ***************************************************************************/

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file COPYING for more information.
 ***************************************************************************/

#ifndef TRACKPRINT_H
#define TRACKPRINT_H

#include "accidentals.h"

#include <QPen>

class KgFontMap;
class TabTrack;
class QFont;
class QPainter;

class TrackPrint
{
	friend class SongPrint;

public:
	TrackPrint();
	~TrackPrint();
	int barExpWidth(int bn, TabTrack *trk);
	int barWidth(int bn, TabTrack *trk);
	int colWidth(int cl, TabTrack *trk);
	void drawBar(int bn, TabTrack *trk, int es, int& sx, int& sx2, bool doDraw = true);
	void drawBarLns(int w, TabTrack *trk);
	int drawKKsigTsig(int bn, TabTrack *trk, bool doDraw, bool fbol, bool flop);
	void drawStLns(const QRect& rect);
	int getFirstColOffs(int bn, TabTrack *trk, bool fbol = true);
	void initFonts(QFont *f1, QFont *f2, QFont *f3, QFont *f4, QFont *f5);
	void initMetrics();
	void initPens();
	void initPrStyle();
	void initPrStyle(int prStyle);
	void setOnScreen(bool scrn = true);
	void setPainter(QPainter *paint);
	QFont *fetaFontPtr(){ return fFeta; }
	int calcYPosTb(int numOfStrings);
	int calcYPosSt(int top = 0);
	int yPosTb() const { return ypostb; }
	int bottomTbMargin() const;
	int bottomStMargin() const;
	// LVIFIX: these probably should not be public
	// The current write location
	int xpos;
	int yposst;					// on the staff
private:
	int ypostb;					// on the tab bar
public:
	// Variables describing staff dimensions
	int wNote;					// width 1/4 notehead
	int ystepst;				// y step from line to line
	// Variables describing tab bar dimensions
	int ysteptb;				// y step from line to line
	int br8h;					// bounding box "8" height
	int br8w;					// bounding box "8" width
	// Pens used
	QPen pLnBl;					// used for black lines & text
	QPen pLnWh;					// used for white lines

	int zoomLevel;
	bool viewscore;

private:
	void drawBeam(int x1, int x2, int y, char tp, char dir);
	void drawBeams(int bn, char dir, TabTrack *trk);
	int drawKey(TabTrack *trk, bool doDraw, bool flop);
	int drawKeySig(TabTrack *trk, bool doDraw);
	void drawLetRing(int x, int y);
	void drawNtHdCntAt(int x, int y, int t, Accidentals::Accid a);
	void drawNtStmCntAt(int x, int yl, int yh, int t, char dir);
	void drawRstCntAt(int x, int y, int t);
	void drawStrCntAt(int x, int y, const QString s);
	int drawTimeSig(int bn, TabTrack *trk, bool doDraw);
	int eraWidth(const QString s);
	bool findHiLo(int cl, int v, TabTrack *trk, int & hi, int & lo);
	int line(const QString step, int oct);

	// Almost all functions use a pointer to the same painter, instead of
	// making it a parameter for all functions make it a member variable
	QPainter *p;
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
	int tsgppScore;
	int nt0fw;
	int ntlfw;
	// Fonts used
	QFont *fTBar1;				// used for notes on the tab bar
	QFont *fTBar2;				// used for notes on the tab bar
	QFont *fTSig;				// used for time signature
	QFont *fFeta;				// used for notes on the staff
	QFont *fFetaNr;				// used for time signature on the staff
	// Variables describing printing style
	bool stNts;					// print notes
	bool stTab;					// print tab
	// Print mode: on screen or on paper
	bool onScreen;
	// Font map
	KgFontMap *fmp;
};

#endif
