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
// depends on note length, font and effect
// magic number "21" scales quarter note to about one centimeter
// LVIFIX: make logarithmic ???

int SongPrint::colWidth(int cl, TabTrack *trk)
{
	int w;
	w = trk->c[cl].l;
	w *= br8w;
	w /= 21;
	// adjust for dots and triplets
	if (trk->c[cl].flags & FLAG_DOT)
		w = (int) (w * 1.5);
	if (trk->c[cl].flags & FLAG_TRIPLET)
		w = (int) (w * 2 / 3);
	// make sure column is wide enough
	if (w < 2 * br8w)
		w = 2 * br8w;
	// make sure effects fit in column
	const int lstStr = trk->string - 1;
	for (int i = 0; i < lstStr + 1; i++) {
		if (   trk->c[cl].e[i] == EFFECT_ARTHARM
			|| trk->c[cl].e[i] == EFFECT_HARMONIC
			|| trk->c[cl].e[i] == EFFECT_LEGATO
			|| trk->c[cl].e[i] == EFFECT_SLIDE)
			if (w < 2 * ystep)
				w = 2 * ystep;
	}
	if (trk->c[cl].flags & FLAG_PM) {
			if (w < 2 * ystep)
				w = 2 * ystep;
	}
	return w;
}

// draw bar bn's contents starting at xpos,ypos adding extra space es

void SongPrint::drawBar(int bn, TabTrack *trk, int es)
{
	TabTrack *curt = trk;	// LVIFIX

	int lastxpos = 0;		// fix compiler warning
	int extSpAftNote;
	int xdelta;
	bool ringing[MAX_STRINGS];
	uint s = curt->string - 1;
	int i;

	for (uint i = 0; i <= s; i++) {
		ringing[i] = FALSE;
	}

	// print timesig if necessary
	if (trk->showBarSig(bn)) {
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
		p->setFont(fTBar1);
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
		if ((t > 0) && (t > (unsigned) curt->b[bn].start)
					&& (curt->c[t-1].l == curt->c[t].l))
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

		// Draw dot is not here, see: "Draw the number column"

		// Length of interval to next column - adjusted if dotted
		// calculated here because it is required by triplet code

		xdelta = colWidth(t, trk);

		// Draw triplet - GREYFIX: ugly code, needs to be fixed
		// somehow... Ideally, triplets should be drawn in a second
		// loop, after everything else would be done.

		if (curt->c[t].flags & FLAG_TRIPLET) {
 			if ((curt->c.size() >= t + 1) && (t) &&
 				(curt->c[t - 1].flags & FLAG_TRIPLET) &&
 				(curt->c[t + 1].flags & FLAG_TRIPLET) &&
				(curt->c[t - 1].l == curt->c[t].l) &&
				(curt->c[t + 1].l == curt->c[t].l)) {
				extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;
				p->drawLine(xpos + xdelta + extSpAftNote, ypos + 3 * ystep,
							xpos + xdelta + extSpAftNote, ypos + 3 * ystep - 3);
				p->drawLine(xpos + xdelta + extSpAftNote, ypos + 3 * ystep,
							lastxpos, ypos + 3 * ystep);
				p->drawLine(lastxpos, ypos + 3 * ystep,
							lastxpos, ypos + 3 * ystep - 3);
				p->setFont(fTBar2);
				drawStrFccAt(xpos, -4, "3");
				p->setFont(fTBar1);
 			} else {
				if (!(((curt->c.size() >= t + 2) &&
					   (curt->c[t + 1].flags & FLAG_TRIPLET) &&
					   (curt->c[t + 2].flags & FLAG_TRIPLET) &&
					   (curt->c[t + 1].l == curt->c[t].l) &&
					   (curt->c[t + 2].l == curt->c[t].l)) ||
					  ((t >= 2) &&
					   (curt->c[t - 1].flags & FLAG_TRIPLET) &&
					   (curt->c[t - 2].flags & FLAG_TRIPLET) &&
					   (curt->c[t - 1].l == curt->c[t].l) &&
					   (curt->c[t - 2].l == curt->c[t].l)))) {
					p->setFont(fTBar2);
					drawStrFccAt(xpos, -4, "3");
					p->setFont(fTBar1);
				}
			}
		}

		// Draw arcs to backward note

		if (curt->c[t].flags & FLAG_ARC)
			p->drawArc(lastxpos, ypos + 2 * ystep + 1,
					   xpos - lastxpos, ystep / 2 + 1, 0, -180 * 16);

		// Draw palm muting

		/* LVIFIX: moved to "draw effects" ...
		if (curt->c[t].flags & FLAG_PM) {
			p->setFont(fTBar2);
			QString pm = "PM";
			drawStrFccAt(xpos, trk->string, pm);
			p->setFont(fTBar1);
		}
		*/

		// Draw the number column
		for (int i = 0; i < trk->string; i++) {
			if (trk->c[t].a[i] != -1) {
				QString note;
				if (curt->c[t].a[i] == DEAD_NOTE)
					note = "X";
				else
					note.setNum(trk->c[t].a[i]);
				// Draw dot
				if (curt->c[t].flags & FLAG_DOT)
					note += ".";
				drawStrFccAt(xpos, i, note);
				if (ringing[i]) {
					// LVIFIX: replace 2 and 4 by (font-dependent) calculation
					p->drawLine(xpos - 4, ypos - i * ystep,
								xpos - 4 - 2, ypos - i * ystep - 2);
					p->drawLine(xpos - 4, ypos - i * ystep,
								xpos - 4 - 2, ypos - i * ystep + 2);
					ringing[i] = FALSE;
				}
			}

			// Draw effects
			// GREYFIX - use lastxpos, not xdelta

			switch (curt->c[t].e[i]) {
			case EFFECT_HARMONIC:
				{
					QPointArray a(4);
					// leftmost point of diamond
					// LVIFIX: replace 4 by (font-dependent) calculation
					int x = xpos + 4;
					int y = ypos - i * ystep; // + ystep / 2;
					// initialize diamond shape
					a.setPoint(0, x,   y  );
					a.setPoint(1, x+2, y+2);
					a.setPoint(2, x+4, y  );
					a.setPoint(3, x+2, y-2);
					// erase tab line
					p->setPen(Qt::white);
					p->drawLine(x, y, x+4, y);
					p->setPen(Qt::black);
					// draw (empty) diamond
					p->drawPolygon(a);
				}
				break;
			case EFFECT_ARTHARM:
				{
					QPointArray a(4);
					// leftmost point of diamond
					// LVIFIX: replace 4 by (font-dependent) calculation
					int x = xpos + 4;
					int y = ypos - i * ystep; // + ystep / 2;
					// initialize diamond shape
					a.setPoint(0, x,   y  );
					a.setPoint(1, x+2, y+2);
					a.setPoint(2, x+4, y  );
					a.setPoint(3, x+2, y-2);
					// draw filled diamond
					QBrush blbr(Qt::black);
					p->setBrush(blbr);
					p->drawPolygon(a);
					p->setBrush(Qt::NoBrush);
				}
				break;
			case EFFECT_LEGATO:
				// draw arc to next note
				// LVIFIX: replace 4 by (font-dependent) calculation
				// the arc should (probably) be as wide as the line between
				// this note and the next. see drawStrFccAt
				// extra space between notes must also be added
				if ((t < curt->c.size() - 1) && (curt->c[t + 1].a[i] >= 0)) {
					extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;
					p->drawArc(xpos + 4, ypos - i * ystep - ystep / 2,
							   xdelta + extSpAftNote - 2 * 4, ystep / 2,
							   0, 180 * 16);
				}
				break;
			case EFFECT_SLIDE:
				// LVIFIX: replace 4 by (font-dependent) calculation
				// the slide symbol should (probably) be as wide as the line
				// between this note and the next. see drawStrFccAt
				// extra space between notes must also be added
				if ((t < curt->c.size() - 1) && (curt->c[t + 1].a[i] >= 0)) {
					extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;
					if (curt->c[t + 1].a[i] > curt->c[t].a[i]) {
						p->drawLine(xpos + 4, ypos - i * ystep + ystep / 2 - 2,
									xpos + xdelta + extSpAftNote - 4,
									ypos - i * ystep - ystep / 2 + 2);
					} else {
						p->drawLine(xpos + 4, ypos - i * ystep - ystep / 2 + 2,
									xpos + xdelta + extSpAftNote - 4,
									ypos - i * ystep + ystep / 2 - 2);
					}
				}
				break;
			case EFFECT_LETRING:
				ringing[i] = TRUE;
				break;
			} // end switch (curt->c[t].e[i])

			// draw palm muting as little cross behind note
			if (curt->c[t].flags & FLAG_PM
				&& trk->c[t].a[i] != -1) {
				// LVIFIX: replace 4 by (font-dependent) calculation
				int x = xpos + 4;
				int y = ypos - i * ystep; // + ystep / 2;
				p->drawLine(x, y-2, x+4, y+2);
				p->drawLine(x, y+2, x+4, y-2);
			}
			
		} // end for (int i = 0 ...
		
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
// note: print drum names instead in case of drumtrack

void SongPrint::drawKey(int l, TabTrack *trk)
{
	p->setFont(fTBar1);
	const int lstStr = trk->string - 1;
	if (l == 0) {
		for (int i = 0; i < lstStr + 1; i++) {
			if (trk->trackMode() == DrumTab) {
				drawStrFccAt(tabpp + br8w / 2,
							 i,
							 drum_abbr[trk->tune[i]]);
			} else {
				drawStrFccAt(tabpp + br8w / 2,
							 i,
							 note_name(trk->tune[i] % 12));
			}
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

// draw header of song song, page n

void SongPrint::drawPageHdr(int n, TabSong *song)
{
	// LVIFIX: replace magic numbers by font-dependent constants
	// LVIFIX: add page number
	p->setFont(fHdr1);
	p->drawText(20, 30, song->title + " - " + song->author);
	QString pgNr;
	pgNr.setNum(n);
	p->setFont(fHdr2);
	p->drawText(pprw - 20, 30, pgNr);
	p->setFont(fHdr3);
	p->drawText(20, 50, "Transcribed by " + song->transcriber);
	ypos = 80;					// LVIFIX: replace magic number
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

// initialize fonts

void SongPrint::initFonts()
{
	fHdr1  = QFont("Helvetica", 12, QFont::Bold);
	fHdr2  = QFont("Helvetica", 10, QFont::Normal);
	fHdr3  = QFont("Helvetica",  8, QFont::Normal);
	fTBar1 = QFont("Helvetica",  8, QFont::Bold);
	fTBar2 = QFont("Helvetica",  7, QFont::Normal);
	fTSig  = QFont("Helvetica", 12, QFont::Bold);
}

// initialize paper format and font dependent metrics

void SongPrint::initMetrics(KPrinter *printer)
{
	// determine width/height of printer surface
	QPaintDeviceMetrics pdm(printer);
	pprh  = pdm.height();
	pprw  = pdm.width();
	// determine font-dependent bar metrics
	p->setFont(fTBar1);
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

	// initialize fonts, must be done before initMetrics
	// (metrics depend on fonts)
	initFonts();

	// initialize variables
	initMetrics(printer);

	// print page header
	int pgNr = 1;
	drawPageHdr(pgNr, song);
	
	uint trkPr = 0;				// tracks printed

	// loop while bars left in the track
	while (trkPr < (song->t).count()) {

		TabTrack *trk = (song->t).at(trkPr);

		// print the track header
		if ((song->t).count() > 1)
		{
			p->setFont(fHdr2);
			p->drawText(20, ypos, trk->name);
			ypos += 20;
		}
		ypos += 40;

		int l = 0;					// line nr in the current track
		uint brsPr = 0;				// bars printed
		int bn = 0;					// current bar nr

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
			// move to next page if necessary
			if (ypos + 4 * ystep > pprh) {
				printer->newPage();
				pgNr++;
				drawPageHdr(pgNr, song);
				ypos += 40;
			}

		} // end while (brsPr ...

		// move to the next track
		trkPr++;

	} // end while (trkPr ...
			
	p->end();			// send job to printer
}
