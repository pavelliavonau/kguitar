/***************************************************************************
 * songprint.cpp: implementation of SongPrint class
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

#include "songprint.h"
#include "tabsong.h"
#include "tabtrack.h"

#include <kprinter.h>
#include <qarray.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

// SongPrint constructor

SongPrint::SongPrint()
{
	p = new QPainter;
}

// SongPrint destructor

SongPrint::~SongPrint()
{
	delete p;
}

// return expandable width in pixels of bar bn in track trk
// this part of the bar is expanded to fill a line completely
// extra space will be added between notes

int SongPrint::barExpWidth(int bn, TabTrack *trk)
{
	int w = 0;
	for (uint t = trk->b[bn].start; (int) t <= trk->lastColumn(bn); t++)
		w += colWidth(t, trk);
	return w;
}

// return width in pixels of bar br in track trk

int SongPrint::barWidth(int bn, TabTrack *trk)
{
	int w = 0;
	for (uint t = trk->b[bn].start; (int) t <= trk->lastColumn(bn); t++)
		w += colWidth(t, trk);
	if (trk->showBarSig(bn))
		w += tsgfw;				// space for timesig
	w += nt0fw;					// space before first note
	w += ntlfw;					// space after last note
	w += 1;						// the trailing vertical line
	return w;
}

// return width in pixels of column cl in track trk
// depends on note length, font and possibly effect
// magic number "21" scales quarter note to about one centimeter
// LVIFIX: make logarithmic ???

int SongPrint::colWidth(int cl, TabTrack *trk)
{
	int w;
	w = trk->c[cl].l;
	w *= br8w;
	w /= 21;
	if (trk->c[cl].flags & FLAG_DOT)
		w = (int) (w * 1.5);
	if (w < 2 * br8w)
		w = 2 * br8w;
	return w;
}

// draw bar bn's contents starting at xpos,ypos adding extra space es

void SongPrint::drawBar(int bn, TabTrack *trk, int es)
{
	TabTrack *curt = trk;	// LVIFIX

	int lastxpos = 0;		// fix compiler warning
	int xdelta;
	uint s = curt->string - 1;
	int i;

	// print timesig if necessary
	if (trk->showBarSig(bn)) {
		QFont fTSig = QFont("Helvetica", 12, QFont::Bold);
		p->setFont(fTSig);
		QFontMetrics fm  = p->fontMetrics();
		// calculate vertical position:
		// exactly halfway between top and bottom string
		int y = ypos - ystep * (trk->string - 1) / 2;
		// center the timesig at this height, use two pixels spacing
		QString time;
		time.setNum(trk->b[bn].time1);
		y -= 1;
		p->drawText(xpos + tsgpp, y, time);
		time.setNum(trk->b[bn].time2);
		y += fm.boundingRect(time).height() + 2;
		p->drawText(xpos + tsgpp, y, time);
		p->setFont(fTBar);
		xpos += tsgfw;
	}

	// space before first note
	xpos += nt0fw;

	// init expandable space left for space distribution calculation
	int barExpWidthLeft = barExpWidth(bn, trk);

	// loop t over all columns in this bar
	for (uint t = trk->b[bn].start; (int) t <= trk->lastColumn(bn); t++) {

		// Drawing duration marks
		// Draw connection with previous, if applicable
		if ((t > 0) && (t>curt->b[bn].start) && (curt->c[t-1].l == curt->c[t].l))
			xdelta = lastxpos;
		else
			xdelta = xpos + ystep / 2;

		switch (curt->c[t].l) {
		case 15:  // 1/32
			p->drawLine(xpos, ypos + 2 * ystep - 4,
						xdelta, ypos + 2 * ystep - 4);
		case 30:  // 1/16
			p->drawLine(xpos, ypos + 2 * ystep - 2,
						xdelta, ypos + 2 * ystep - 2);
		case 60:  // 1/8
			p->drawLine(xpos, ypos + 2 * ystep,
						xdelta, ypos + 2 * ystep);
		case 120: // 1/4 - a long vertical line, so we need to find the highest note
			for (i = s;((i >= 0) && (curt->c[t].a[i] == -1)); i--);

			// If it's an empty measure at all - draw the vertical line from bottom
			if (i < 0)  i = 1;

			p->drawLine(xpos, ypos - i * ystep + ystep / 2,
						xpos, ypos + 2 * ystep);
			break;		// required to prevent print preview artefact
		case 240: // 1/2
			p->drawLine(xpos, ypos + 1 * ystep,
						xpos, ypos + 2 * ystep);
		case 480: // whole
			break;
		} // end switch (curt->c[t].l)

		// print this column
		for (int i = 0; i < trk->string; i++) {
			if (trk->c[t].a[i] != -1) {
				QString note;
				note.setNum(trk->c[t].a[i]);
				drawStrFccAt(xpos, i, note);
			}
		}
		lastxpos = xpos;
		xpos += colWidth(t, trk);

		// calculate and add extra space
		int extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;
		xpos += extSpAftNote;
		es -= extSpAftNote;
		barExpWidthLeft -= colWidth(t, trk);

	} // end for (uint t ...

	// space after last note
	xpos += ntlfw;

	// end bar with vertical line
	p->drawLine(xpos, ypos,
	            xpos, ypos - (trk->string - 1) * ystep);
	xpos += 1;
}

// draw bar lines at xpos,ypos width w for all strings of track trk

void SongPrint::drawBarLns(int w, TabTrack *trk)
{
	const int lstStr = trk->string - 1;
	// vertical lines at xpos and xpos+w-1
	p->drawLine(xpos, ypos, xpos, ypos - lstStr * ystep);
	p->drawLine(xpos + w - 1, ypos, xpos + w - 1, ypos - lstStr * ystep);
	// horizontal lines from xpos to xpos+w-1
	for (int i = 0; i < lstStr+1; i++) {
		p->drawLine(xpos, ypos - i * ystep, xpos + w - 1, ypos - i * ystep);
	}
}

// draw key at xpos,ypos for all strings of track trk
// at the first line (l == 0), string names are printed
// at all other lines the text "TAB"

void SongPrint::drawKey(int l, TabTrack *trk)
{
	p->setFont(fTBar);
	const int lstStr = trk->string - 1;
	if (l == 0) {
		for (int i = 0; i < lstStr + 1; i++) {
			drawStrFccAt(tabpp + br8w / 2,
						 i,
						 note_name(trk->tune[i] % 12));
		}
	} else {
		// calculate vertical position:
		// exactly halfway between top and bottom string
		// center "TAB" at this height, use two pixels spacing
		QFontMetrics fm  = p->fontMetrics();
		int y = ypos - ystep * lstStr / 2;
		int h8 = fm.boundingRect("8").height();
		y -= h8 / 2 + 2;
		p->drawText(xpos + tabpp, y, "T");
		y += h8 + 2;
		p->drawText(xpos + tabpp, y, "A");
		y += h8 + 2;
		p->drawText(xpos + tabpp, y, "B");
	}
}

// draw string s with first character centered at x on string n
// erase tab line at location of string
// uses ypos but ignores xpos
// LVIFIX: use xpos too ?

void SongPrint::drawStrFccAt(int x, int n, const QString s)
{
	QFontMetrics fm  = p->fontMetrics();
	const int yOffs  = fm.boundingRect("8").height() / 2;
	const int xOffsL = fm.boundingRect("8").width() / 2;
	const int xOffsR = fm.boundingRect(s).width() - xOffsL;
	p->setPen(Qt::white);
	p->drawLine(x - xOffsL - 1, ypos - n * ystep,
				x + xOffsR + 2, ypos - n * ystep);
	p->drawLine(x, ypos - n * ystep - ystep / 2,
				x, ypos - n * ystep + ystep / 2);
	p->setPen(Qt::black);
	p->drawText(x - xOffsL, ypos - n * ystep + yOffs, s);
}

// initialize paper format and font dependent metrics

void SongPrint::initMetrics(KPrinter *printer)
{
	// determine width/height of printer surface
	QPaintDeviceMetrics pdm(printer);
	pprh  = pdm.height();
	pprw  = pdm.width();
	// determine font-dependent bar metrics
	fTBar = QFont("Helvetica", 8, QFont::Bold);
	p->setFont(fTBar);
	QFontMetrics fm  = p->fontMetrics();
	br8h = fm.boundingRect("8").height();
	br8w = fm.boundingRect("8").width();
	ystep = fm.ascent() - 1;
	// LVIFIX: tune these values
	// note: font dependent
	tabfw = 20;
	tabpp =  5;
	tsgfw = 20;
	tsgpp = 10;
	nt0fw =  8;
	ntlfw =  2;
}

// print song song on printer printer

void SongPrint::printSong(KPrinter *printer, TabSong *song)
{
	// start painting on printer
	if (!p->begin(printer))
		return;

	// initialize variables
	initMetrics(printer);

	// print page header
	// LVIFIX: move to separate function to enable multi-page operation
	// LVIFIX: replace magic numbers by font-dependent constants
	QFont fHdr1 = QFont("Helvetica", 12, QFont::Bold);
	p->setFont(fHdr1);
	p->drawText(20, 30, song->title + " - " + song->author);
	QFont fHdr2 = QFont("Helvetica", 8, QFont::Normal);
	p->setFont(fHdr2);
	p->drawText(20, 50, "Transcribed by " + song->transcriber);
	ypos = 120;					// LVIFIX: replace magic number

	//
	int l = 0;					// current line nr
	uint brsPr = 0;				// bars printed
	int bn = 0;					// current bar nr

	// only the first track is printed
	TabTrack *trk = (song->t).first();

	// precalculate bar widths
	QArray<int> bew(trk->b.size());
	QArray<int> bw(trk->b.size());
	for (uint bn = 0; bn < trk->b.size(); bn++) {
		bew[bn] = barExpWidth(bn, trk);
		bw[bn]  = barWidth(bn, trk);
	}

	// loop while bars left in the track
	while (brsPr < trk->b.size()) {

		// draw empty tab bar at yPos
		xpos = 0;
		drawBarLns(pprw - 1, trk);
		xpos += 1;				// first vertical line
		drawKey(l, trk);
		xpos += tabfw;			// "TAB"

		// determine # bars fitting on this line
		// must be at least 1 (very long bar will be truncated)
		uint nBarsOnLine = 1;
		int totWidth = bw[bn];
		// while bars left and next bar also fits
		while (bn + nBarsOnLine < trk->b.size()
			   && totWidth + bw[bn + nBarsOnLine] < pprw - xpos) {
			totWidth += bw[bn + nBarsOnLine];
			nBarsOnLine++;
		}

		// print without extra space on last line,
		// with extra space on all others
		if (bn + nBarsOnLine >= trk->b.size()) {
			// last line, no extra space
			for (uint i = 0; i < nBarsOnLine; i++) {
				drawBar(bn, trk, 0);
				bn++;
			}
		} else {
			// not the last line, add extra space
			// calculate extra space left to distribute divided by bars left,
			// to prevent accumulation of round-off errors
			int extSpLeft = pprw - xpos - totWidth - 1;
			for (uint i = 0; i < nBarsOnLine; i++) {
				int extSpInBar = extSpLeft / (nBarsOnLine - i);
				drawBar(bn, trk, extSpInBar);
				extSpLeft -= extSpInBar;
				bn++;
			}
		}
		
		brsPr += nBarsOnLine;

		// move to next line
		l++;
		ypos += 10 * ystep;				// LVIFIX: replace magic number
		// LVIFIX: move to next page if necessary

	} // end while (brsPr ...
			
	p->end();			// send job to printer
}
