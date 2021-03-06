/***************************************************************************
 * trackprint.cpp: implementation of TrackPrint class
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

// TrackPrint prints the contents of a single bar. It is used by both
// TrackView (draws onscreen) and SongPrint (prints on paper).
//
// key and keysig must be printed:
// - on first bar of line on paper (decided by SongPrint)
// - or if changed by current bar  (decided by TrackPrint)
//
// timesig must be printed:
// - on first bar of track         (decided by TrackPrint)
// - or if changed by current bar  (decided by TrackPrint)
//
// "TAB" must be replaced by string or drum names:
// - on first bar of page          (decided by SongPrint)
// note: set fieldwidth(key + keysig) to max(fw(key)+fw(keysig), fw(names))

// LVIFIX: this file needs to be redesigned
// - code cleanup
// - xpos/ypos interface
// - adapt linewidth to resolution of output device

#include "trackprint.h"

#include "settings.h"

#include <iostream>		// required for cout and friends
//Added by qt3to4:
#include <QPolygon>
using namespace std;		// required for cout and friends

#include <qfontmetrics.h>
#include <qpainter.h>

#include "global.h"
#include "kgfontmap.h"
#include "data/tabtrack.h"

// definitions for the "new" drawing code layout
#define TOPSPTB                         3   // top space tabbar in ysteptb units
#define BOTSPTB                         3   // bottom space tabbar in ysteptb units
#define ADDSPST                         1.5 // additional top space staff in ystepst units
#define TOPSPST                         7.5 // top space staff in ystepst units
#define BOTSPST                         1.5 // bottom space staff in ystepst units
#define NLINEST                         5   // number of staff lines

// TrackPrint constructor. Initialize font metrics with reasonable guesstimates, as required
// by TrackView::updateRows(). Correct values will later be set by initMetrics.

TrackPrint::TrackPrint()
{
//	cout << "TrackPrint::TrackPrint() @ " << this << endl;
	br8h     = 10;
	br8w     = 10;
	onScreen = FALSE;
	p        = 0;
	wNote    = 10;
	ystepst  = 10;
	ysteptb  = 10;
	fmp      = new KgFontMap;
}

// TrackPrint destructor.

TrackPrint::~TrackPrint()
{
	delete fmp;
}

// return expandable width in pixels of bar bn in track trk
// this part of the bar is expanded to fill a line completely
// extra space will be added between notes

int TrackPrint::barExpWidth(int bn, TabTrack *trk)
{
	int w = 0;
	for (uint t = trk->bars()[bn].start; (int) t <= trk->lastColumn(bn); t++)
		w += colWidth(t, trk);
	return w;
}

// return width in pixels of bar bn in track trk

int TrackPrint::barWidth(int bn, TabTrack *trk)
{
	if (onScreen)
		return 480 * br8w * trk->bars()[bn].time1 / trk->bars()[bn].time2 / zoomLevel +
			tsgfw + nt0fw + ntlfw + (int) (5.5 * br8w);

	int w = 0;
	for (uint t = trk->bars()[bn].start; ((int) t) <= trk->lastColumn(bn); t++)
		w += colWidth(t, trk);
	// LVIFIX: when KGuitar supports changing the key at the start of any bar,
	// calculate space for keysig here
	if (trk->showBarSig(bn))
		w += tsgfw;				// space for timesig
	w += nt0fw;					// space before first note
	int cl = trk->bars()[bn].start;	// first column of bar
	int wacc = 0;				// width accidental
	// LVIFIX: replace by hasAccidental(int cl)
	for (int i = 0; i < trk->string; i++) {
		// if first column has note with accidental, add space
		if ((trk->c[cl].a[i] > -1)
			&& (trk->c[cl].acc[i] != Accidentals::None)) {
			wacc = (int) (0.9 * wNote);
		}
	}
	w += wacc;					// space for accidental
	w += ntlfw;					// space after last note
	w += 1;						// LVIFIX: the trailing vertical line
	return w;
}

// return width in pixels of column cl in track trk
// if on screen:
// depends on note length and font
// if printing:
// depends on note length, font and effect
// magic number "21" scales quarter note to about one centimeter
// LVIFIX: make logarithmic ???

int TrackPrint::colWidth(int cl, TabTrack *trk)
{
	// cout << "colWidth(" << cl << ")";
	int w;
	w = trk->c[cl].l;
	// cout << " xpos=" << xpos;
	// cout << " br8w=" << br8w;
	// cout << " wNote=" << wNote;
	// cout << " l=" << w;
	// adjust for dots and triplets
	if (trk->c[cl].flags & FLAG_DOT)
		w = (int) (w * 1.5);
	if (trk->c[cl].flags & FLAG_TRIPLET)
		w = (int) (w * 2 / 3);
	w *= br8w;
	if (onScreen) {
		w /= zoomLevel;
		return w;
	}
	w /= 21;
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
			if (w < 2 * ysteptb)
				w = 2 * ysteptb;
	}
	if (trk->c[cl].flags & FLAG_PM) {
			if (w < 2 * ysteptb)
				w = 2 * ysteptb;
	}

	// corrections that apply only when printing notes
	if (stNts) {
		int emsa = 0;			// extra minimum space between notes due to acc.
		int emsf = 0;			// extra minimum space between notes due to flag
		// not the last column in a track
		// and not the last column in a bar
		if ((cl < ((int) trk->c.size() - 1))
			&& (cl != trk->lastColumn(trk->barNr(cl)))) {
			for (unsigned int i = 0; i < trk->string; i++) {
				// if next column has note with accidental, add space
				if ((trk->c[cl + 1].a[i] > -1)
					&& (trk->c[cl + 1].acc[i] != Accidentals::None)) {
					emsa = (int) (0.6 * wNote);
					// if note in voice 0 or 1 in this column has a flag
					// and it is not beamed, add space
					// LVIFIX: fix test
					int dt;
					bool res0;
					bool res1;
					int tp0;
					int tp1;
					bool tr;
					res0 = trk->getNoteTypeAndDots(cl, 0, tp0, dt, tr);
					res1 = trk->getNoteTypeAndDots(cl, 1, tp1, dt, tr);
					if ((res0 && (tp0<=60) && (trk->c[cl].stl.l1 == 'n'))
						|| (res1 && (tp1<=60) && (trk->c[cl].stu.l1 == 'n'))) {
						emsf = (int) (0.6 * wNote);
					}
				}
			}
		}
		int ms = (int) (1.5 * wNote);	// minimum space between notes
		ms += emsa;
		ms += emsf;
		if (w < ms) {
			w = ms;
		}
		// cout << " emsa=" << emsa;
	}
	// cout << " w=" << w << endl;
	return w;
}

// draw bar bn's contents starting at xpos,ypostb adding extra space es
// also update selection x coordinates for trackview

void TrackPrint::drawBar(int bn, TabTrack *trk, int es, int& sx, int& sx2, bool doDraw)
{
//	cout << "TrackPrint::drawBar(" << bn << ", " << trk << ", " << es << ")" << " xpos=";

	TabTrack *curt = trk;		// LVIFIX

	int lastxpos = tsgpp;
	int extSpAftNote = 0;		// extra space, divided over the notes
	int xdelta = 0;				// used for drawing beams, legato and slide
	bool ringing[MAX_STRINGS];
	uint s = curt->string - 1;
	int i = 0;
	int trpCnt = 0;				// triplet count

	for (uint i = 0; i <= s; i++) {
		ringing[i] = FALSE;
	}

/*
	// print timesig if necessary
	// LVIFIX: may need to center horizontally
	if (trk->showBarSig(bn)) {
		int brth;
		QFontMetrics fm = p->fontMetrics();
		QString time;
		int y;
		if (stNts) {
			// staff
			p->setFont(*fFetaNr);
			fm = p->fontMetrics();
			// calculate vertical position:
			// exactly halfway between top and bottom string
			y = yposst - ystepst * 2;
			// center the timesig at this height
			// use spacing of 0.2 * char height
			time.setNum(trk->bars()[bn].time1);
			brth = fm.boundingRect(time).height();
			y -= (int) (0.1 * brth);
			p->drawText(xpos + tsgpp, y, time);
			time.setNum(trk->bars()[bn].time2);
			y += (int) (1.2 * brth);
			p->drawText(xpos + tsgpp, y, time);
		}
		if (stTab) {
			// tab bar
			p->setFont(*fTSig);
			fm = p->fontMetrics();
			// calculate vertical position:
			// exactly halfway between top and bottom string
			y = ypostb - ysteptb * (trk->string - 1) / 2;
			// center the timesig at this height
			// use spacing of 0.2 * char height
			time.setNum(trk->bars()[bn].time1);
			brth = fm.boundingRect(time).height();
			y -= (int) (0.1 * brth);
			p->drawText(xpos + tsgpp, y, time);
			time.setNum(trk->bars()[bn].time2);
			y += (int) (1.2 * brth);
			p->drawText(xpos + tsgpp, y, time);
			p->setFont(*fTBar1);
		}
		if (stNts || stTab) {
			xpos += tsgfw;
		}
	} else {
		if (onScreen) {
			xpos += tsgfw;
		}
	}
*/

	// space before first note
	xpos += nt0fw;
	bool needWacc = FALSE;
	int cl = trk->bars()[bn].start;		// first column of bar
	int wacc = (int) (0.9 * wNote);		// width accidental
	// LVIFIX: replace by hasAccidental(int cl)
	for (int i = 0; i < trk->string; i++) {
		// if first column has note with accidental, add space
		if ((trk->c[cl].a[i] > -1)
			&& (trk->c[cl].acc[i] != Accidentals::None)) {
			// LVIFIX: make global const, used twice
			needWacc = TRUE;
		}
	}
	if (onScreen || needWacc) {
		xpos += wacc;
	}

	// init expandable space left for space distribution calculation
	int barExpWidthLeft = barExpWidth(bn, trk);

	int effvsz = 0;		// effect vertical size (depends on onScreen)

	// loop t over all columns in this bar and print them
	for (uint t = trk->bars()[bn].start; (int) t <= trk->lastColumn(bn); t++) {

		// tie handling
		int  tt = t;				// t where tie starts
		if ((t > 0) && (trk->c[t].flags & FLAG_ARC)) {
			tt = t - 1;				// LVIFIX: handle more than one tie
		}

		// triplet handling:
		// - reset after third note of triplet
		// - count notes while inside triplet
		if (trpCnt >= 3) {
			trpCnt = 0;
		}
		if (trk->c[t].flags & FLAG_TRIPLET) {
			trpCnt++;
		} else {
			trpCnt = 0;
		}

		if (stTab) {

			// Drawing duration marks
			// Draw connection with previous, if applicable
			if ((t > 0) && (t > (unsigned) curt->bars()[bn].start)
						&& (curt->c[t-1].l == curt->c[t].l))
				xdelta = lastxpos;
			else
				xdelta = xpos + ysteptb / 2;

			if(doDraw) {
				p->setPen(pLnBl);
				switch (curt->c[t].l) {
				case 15:  // 1/32
					p->drawLine(xpos,   (int) (ypostb + 1.6 * ysteptb),
					xdelta, (int) (ypostb + 1.6 * ysteptb));
				case 30:  // 1/16
					p->drawLine(xpos,   (int) (ypostb + 1.8 * ysteptb),
					            xdelta, (int) (ypostb + 1.8 * ysteptb));
				case 60:  // 1/8
					p->drawLine(xpos,   ypostb + 2 * ysteptb,
					            xdelta, ypostb + 2 * ysteptb);
				case 120: // 1/4 - a long vertical line, so we need to find the highest note
					for (i = s;((i >= 0) && (curt->c[t].a[i] == -1)); i--);

					// If it's an empty measure at all - draw the vertical line from bottom
					if (i < 0)  i = 1;

					p->drawLine(xpos, ypostb - i * ysteptb + ysteptb / 2,
					            xpos, ypostb + 2 * ysteptb);
					break;		// required to prevent print preview artefact
				case 240: // 1/2
						p->drawLine(xpos, ypostb + 1 * ysteptb,
						            xpos, ypostb + 2 * ysteptb);
						break;
				case 480: // whole
						break;
				} // end switch (curt->c[t].l)
			}

			// Draw dot is not here, see: "Draw the number column"

			// Length of interval to next column - adjusted if dotted
			// calculated here because it is required by triplet code

			xdelta = colWidth(t, trk);
			extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;

			if(doDraw) {
				// Draw triplet
				if ((trpCnt == 1) || (trpCnt == 2)) {
					 // draw horizontal line to next note
					p->drawLine(xpos + xdelta + extSpAftNote,
					            (int) (ypostb + 2.5 * ysteptb),
					            xpos,
					            (int) (ypostb + 2.5 * ysteptb));
				}
				if ((trpCnt == 1) || (trpCnt == 3)) {
				// draw vertical line
					p->drawLine(xpos,
					            (int) (ypostb + 2.3 * ysteptb),
					            xpos,
					            (int) (ypostb + 2.5 * ysteptb));
				}
				if (trpCnt == 2) {
					 // draw "3"
					p->setFont(*fTBar2);
					drawStrCntAt(xpos, -3, "3");
					p->setFont(*fTBar1);
				}
			}

			// Draw arcs to backward note

			if (curt->c[t].flags & FLAG_ARC) {
					if (onScreen) {
							effvsz = ysteptb;
					} else {
							effvsz = ysteptb / 2;
					}
					if(doDraw)
						p->drawArc(lastxpos, ypostb + 2 * ysteptb + 1,
						           xpos - lastxpos, effvsz, 0, -180 * 16);
			}

			// Draw palm muting moved to "draw effects" ...

		} // end if (stTab ...

		// start drawing notes

		// tie handling:
		// KGuitar stores the second column of a tie as a rest (an empty column).
		// Therefore take the notes from the previous column.
		// LVIFIX:
		// "previous" should be "first column of the set of tied columns"
		// (there may be more than two)
		// See also: musicxml.cpp MusicXMLWriter::writeCol()

		if (stNts) {

			// print notes
			int ln = 0;				// line where note is printed
			int nhPrinted = 0;		// # note heads printed
			int yl = 0;				// ypos (line) lowest note head
			int yh = 0;				// ypos (line) highest note head
			/*
			cout << "SongPrint::drawBar() draw column"
				<< " t=" << t
				<< " tt=" << tt
				<< endl;
			for (int i = 0; i < 2; i++) {
				int dt;
				int tp;
				bool tr;
				bool res;
				res = trk->getNoteTypeAndDots(t, i, tp, dt, tr);
				cout
					<< "getNoteTypeAndDots(t)"
					<< " i=" << i
					<< " res=" << res
					<< " tp=" << tp
					<< " dt=" << dt
					<< endl;
			}
			for (int i = 0; i < 2; i++) {
				bool res;
				res = findHiLo(tt, i, trk, yh, yl);
				cout
					<< "findHiLo(tt)"
					<< " i=" << i
					<< " res=" << res
					<< " yh=" << yh
					<< " yl=" << yl
					<< endl;
			}
			*/
			int dt;
			bool res1;
			bool res2;
			int tp;
			bool tr;
			// print voice 0
			res1 = trk->getNoteTypeAndDots(t, 0, tp, dt, tr);
			res2 = findHiLo(tt, 0, trk, yh, yl);
			if (res1 && res2) {
				// voice 0 found
				for (int i = 0; i < trk->string; i++) {
					if ((trk->c[tt].a[i] > -1) && (trk->c[t].v[i] == 0)) {
						ln = line((QChar) trk->c[tt].stp[i], trk->c[tt].oct[i]);
						if(doDraw)
							drawNtHdCntAt(xpos, ln, tp, trk->c[tt].acc[i]);
						nhPrinted++;
						// Draw dot, must be at odd line -> set lsbit
						// LVIFIX: add support for double dot
						QString s;
						if (dt && fmp->getString(KgFontMap::Dot, s)) {
							int y = ln | 1;
							if(doDraw) {
								p->setFont(*fFeta);
								p->drawText((int) (xpos + 0.8 * wNote),
								            yposst - ystepst * y / 2, s);
							}
						}
					}
				}
				if (trk->c[t].stl.l1 != 'n') {
					// note is beamed, don't draw lower stem and flag
					if(doDraw)
						drawNtStmCntAt(xpos, yl, yh, 0, 'd');
					// remember position
					trk->c[t].stl.bp.setX((int) (xpos - 0.45 * wNote));
					int yhd = yposst - (int) (ystepst * ((-0.4 + yl) / 2));
					trk->c[t].stl.bp.setY(yhd);
				} else {
					if(doDraw)
						drawNtStmCntAt(xpos, yl, yh, tp, 'd');
				}
			}
			// print voice 1
			res1 = trk->getNoteTypeAndDots(t, 1, tp, dt, tr);
			res2 = findHiLo(tt, 1, trk, yh, yl);
			if (res1 && res2) {
				// voice 1 found
				for (int i = 0; i < trk->string; i++) {
					if ((trk->c[tt].a[i] > -1) && (trk->c[t].v[i] == 1)) {
						ln = line((QChar) trk->c[tt].stp[i], trk->c[tt].oct[i]);
						if(doDraw)
							drawNtHdCntAt(xpos, ln, tp, trk->c[tt].acc[i]);
						nhPrinted++;
						// Draw dot, must be at odd line -> set lsbit
						// LVIFIX: add support for double dot
						QString s;
						if (dt && fmp->getString(KgFontMap::Dot, s)) {
							int y = ln | 1;
							if(doDraw) {
								p->setFont(*fFeta);
								p->drawText((int) (xpos + 0.8 * wNote),
								            yposst - ystepst * y / 2, s);
							}
						}
					}
				}
				if (trk->c[t].stu.l1 != 'n') {
					// note is beamed, don't draw upper stem and flag
					if(doDraw)
						drawNtStmCntAt(xpos, yl, yh, 0, 'u');
					// remember position
					trk->c[t].stu.bp.setX((int) (xpos + 0.45 * wNote));
					int yhd = yposst - (int) (ystepst * ((0.4 + yh) / 2));
					trk->c[t].stu.bp.setY(yhd);
				} else {
					if(doDraw)
						drawNtStmCntAt(xpos, yl, yh, tp, 'u');
				}
			}

			// if no note printed, print rest
			if (nhPrinted == 0) {
				if(doDraw)
					drawRstCntAt(xpos, 4, trk->c[t].l);
			}

		} // end if (stNts ...

		// end drawing notes

		if (stTab && doDraw) {

			// Draw the number column including effects
			p->setFont(*fTBar1);
			int ew_2 = 0;			// used for positioning effects
			QString note = "";
//			cout << " " << xpos;
			for (unsigned int i = 0; i < trk->string; i++) {
				if (trk->c[t].a[i] != -1) {
					if (curt->c[t].a[i] == DEAD_NOTE)
						note = "X";
					else
						note.setNum(trk->c[t].a[i]);
					// Draw dot
					if (curt->c[t].flags & FLAG_DOT)
						note += ".";
						drawStrCntAt(xpos, i, note);
					// cell width is needed later
					ew_2 = eraWidth(note) / 2;
					if (ringing[i]) {
						drawLetRing(xpos - ew_2, i);
						ringing[i] = FALSE;
					}
				}
				if ((curt->c[t].a[i] == -1)
				     && (curt->c[t].e[i] == EFFECT_STOPRING)) {
					if (ringing[i]) {
						int ew_3 = eraWidth("0") / 4;
						drawLetRing(xpos - ew_3, i);
						ringing[i] = FALSE;
					}
				}

				// Draw effects
				// GREYFIX - use lastxpos, not xdelta

				switch (curt->c[t].e[i]) {
				case EFFECT_HARMONIC:
					{
						QPolygon a(4);
						// size of diamond
						int sz_2 = ysteptb / 4;
						// leftmost point of diamond
						int x = xpos + ew_2;
						int y = ypostb - i * ysteptb;
						// initialize diamond shape
						a.setPoint(0, x,        y     );
						a.setPoint(1, x+sz_2,   y+sz_2);
						a.setPoint(2, x+2*sz_2, y     );
						a.setPoint(3, x+sz_2,   y-sz_2);
						// erase tab line
						p->setPen(pLnWh);
						p->drawLine(x, y, x+2*sz_2, y);
						p->setPen(pLnBl);
						// draw (empty) diamond
						p->drawPolygon(a);
					}
					break;
				case EFFECT_ARTHARM:
					{
						QPolygon a(4);
						// size of diamond
						int sz_2 = ysteptb / 4;
						// leftmost point of diamond
						int x = xpos + ew_2;
						int y = ypostb - i * ysteptb;
						// initialize diamond shape
						a.setPoint(0, x,        y     );
						a.setPoint(1, x+sz_2,   y+sz_2);
						a.setPoint(2, x+2*sz_2, y     );
						a.setPoint(3, x+sz_2,   y-sz_2);
						// draw filled diamond
						QBrush blbr(Qt::black);
						p->setBrush(blbr);
						p->drawPolygon(a);
						p->setBrush(Qt::NoBrush);
					}
					break;
				case EFFECT_LEGATO:
					// draw arc to next note
					// the arc should be as wide as the line between
					// this note and the next. see drawStrCntAt.
					// extra space between notes must also be added
					// LVIFIX: also write "HO" or "PO"
					if (onScreen) {
						effvsz = ysteptb;
					} else {
						effvsz = ysteptb / 2;
					}
					if ((t < curt->c.size() - 1) && (curt->c[t + 1].a[i] >= 0)) {
						extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;
						p->drawArc(xpos + ew_2, ypostb - i * ysteptb - ysteptb / 2,
								   xdelta + extSpAftNote - 2 * ew_2, effvsz,
								   0, 180 * 16);
						QString hopo = "";
						if (curt->c[t + 1].a[i] > curt->c[t].a[i]) {
							hopo = "HO";
						} else {
							hopo = "PO";
						}
						p->setFont(*fTBar2);
						drawStrCntAt(xpos + (xdelta + extSpAftNote) / 2, i, hopo);
						p->setFont(*fTBar1);
					}
					break;
				case EFFECT_SLIDE:
					// the slide symbol should be as wide as the line
					// between this note and the next. see drawStrCntAt.
					// extra space between notes must also be added
					if (onScreen) {
						effvsz = ysteptb / 2;
					} else {
						effvsz = ysteptb / 3;
					}
					if ((t < curt->c.size() - 1) && (curt->c[t + 1].a[i] >= 0)) {
						extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;
						if (curt->c[t + 1].a[i] > curt->c[t].a[i]) {
							p->drawLine(xpos + ew_2,
										ypostb - i * ysteptb + effvsz,
										xpos + xdelta + extSpAftNote - ew_2,
										ypostb - i * ysteptb - effvsz);
						} else {
							p->drawLine(xpos + ew_2,
										ypostb - i * ysteptb - effvsz,
										xpos + xdelta + extSpAftNote - ew_2,
										ypostb - i * ysteptb + effvsz);
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
					int sz_2 = 0;
					if (onScreen) {
						sz_2 = ysteptb / 3;
					} else {
						sz_2 = ysteptb / 4;
					}
					int x    = xpos + ew_2;
					int y    = ypostb - i * ysteptb;
					p->drawLine(x, y - sz_2, x + sz_2, y + sz_2);
					p->drawLine(x, y + sz_2, x + sz_2, y - sz_2);
				}

			} // end for (unsigned int i = 0 ... (end draw the number column ...)

		} // end if (stTab ...

		// update selection x coordinates for trackview
		if ((int) t == curt->x)
			sx  = xpos;
		if ((int) t == curt->xsel)
			sx2 = xpos;

		lastxpos = xpos;
		xpos += colWidth(t, trk);

		// calculate and add extra space
		int extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;
		xpos += extSpAftNote;
		es -= extSpAftNote;
		barExpWidthLeft -= colWidth(t, trk);

	} // end for (uint t ... (end loop t over all columns ...)

//	cout << endl;

	// draw beams
	if (stNts && doDraw) {
		drawBeams(bn, 'd', trk);
		drawBeams(bn, 'u', trk);
	}

	// space after last note
	if (! onScreen) {
		xpos += ntlfw;
	} else {
		int bw = barWidth(bn, trk);
		if (xpos >= bw)
			xpos = bw - 1;
	}

	// end bar
	if (stTab && doDraw) {
		// show notes still ringing at end of bar
		for (unsigned int i = 0; i <= s; i++) {
			if (ringing[i]) {
				int ew_3 = eraWidth("0") / 4;
				drawLetRing(xpos - ew_3, i);
				ringing[i] = FALSE;
			}
		}
		// draw vertical line
//		p->drawLine(xpos, ypostb,
//		            xpos, ypostb - (trk->string - 1) * ysteptb);
    }
//	if (stNts && doDraw) {
//		// draw vertical line
//		p->drawLine(xpos, yposst,
//		            xpos, yposst - 4 * ystepst);
//	}
	// LVIFIX
	xpos += 1;
}

// draw bar lines at xpos,ypostb width w for all strings of track trk

void TrackPrint::drawBarLns(int w, TabTrack *trk)
{
	const int lstStr = trk->string - 1;
	// vertical lines at xpos and xpos+w-1
	p->setPen(pLnBl);
	if (!onScreen) {
		p->drawLine(xpos, ypostb, xpos, ypostb - lstStr * ysteptb);
		p->drawLine(xpos + w - 1, ypostb, xpos + w - 1, ypostb - lstStr * ysteptb);
	}
	// horizontal lines from xpos to xpos+w-1
	for (int i = 0; i < lstStr+1; i++) {
		p->drawLine(xpos, ypostb - i * ysteptb,
					xpos + w - 1, ypostb - i * ysteptb);
	}
}

// draw a single beam

void TrackPrint::drawBeam(int x1, int x2, int y, char tp, char dir)
{
	int yh;
	int yl;
	if (dir != 'd') {
		yh = y;
		yl = y - (int) (0.4 * ystepst);
	} else {
		yh = y + (int) (0.4 * ystepst);
		yl = y;
	}
	QPolygon a;
	QBrush brush(Qt::black, Qt::SolidPattern);
	p->setBrush(brush);
	switch (tp) {
	case 'b':
		x2 = x1;
		x1 = x1 - (int) (0.6 * ystepst);
		break;
	case 'f':
		x2 = x1 + (int) (0.6 * ystepst);
		break;
	case 'c':
	case 's':
		// nothing to be done for 'c' and 's'
		break;
	default:
		return;
	}
	a.setPoints(4,
		x1, yh,
		x2, yh,
		x2, yl,
		x1, yl
	);
	p->drawPolygon(a);
}

// draw beams of bar bn, all other info to be found in StemInfo stl/stu

void TrackPrint::drawBeams(int bn, char dir, TabTrack *trk)
{
	// cout << "SongPrint::drawBeams(" << bn << ", " << dir << ")" << endl;
	StemInfo * stxt = 0;
	for (uint t = trk->bars()[bn].start; (int) t <= trk->lastColumn(bn); t++) {
		/*
		if (dir != 'd') {
			stxt = & trk->c[t].stu;
		} else {
			stxt = & trk->c[t].stl;
		}
		cout
			<< "t=" << t
			<< " l1..3=" << stxt->l1 << stxt->l2 << stxt->l3 << endl;
		*/
	}
	int yextr = 0;
	for (uint t = trk->bars()[bn].start; (int) t <= trk->lastColumn(bn); t++) {
		if (dir != 'd') {
			stxt = & trk->c[t].stu;
		} else {
			stxt = & trk->c[t].stl;
		}
		if (stxt->l1 == 's') {
			// determine beam height: depends on highest/lowest note
			// LVIFIX: support angled beams
			uint i = t;
			if (dir != 'd') {
				yextr = trk->c[i].stu.bp.y();
			} else {
				yextr = trk->c[i].stl.bp.y();
			}
			i++;
			while ((int) i <= trk->lastColumn(bn)) {
				if (dir != 'd') {
					if (trk->c[i].stu.bp.y() < yextr) {
						yextr = trk->c[i].stu.bp.y();
					}
					if (trk->c[i].stu.l1 == 'e') {
						break;
					}
				} else {
					if (trk->c[i].stl.bp.y() > yextr) {
						yextr = trk->c[i].stl.bp.y();
					}
					if (trk->c[i].stl.l1 == 'e') {
						break;
					}
				}
				i++;
			}
		}
		if (stxt->l1 != 'n') {
			// draw stem
			int x1 = stxt->bp.x();
			int x2 = 0;
			if ((int) t < trk->lastColumn(bn)) {
				if (dir != 'd') {
					x2 = trk->c[t+1].stu.bp.x();
				} else {
					x2 = trk->c[t+1].stl.bp.x();
				}
			}
			int ydir;
			int yh;
			int yl;
			if (dir != 'd') {
				ydir = 1;
				yh = yextr - ydir * (int) (3.5 * ystepst);
				yl = stxt->bp.y();
			} else {
				ydir = -1;
				yh = stxt->bp.y();
				yl = yextr - ydir * (int) (3.5 * ystepst);
			}
			p->setPen(pLnBl);
			p->drawLine(x1, yl, x1, yh);
			// draw beams
			if (dir != 'd') {
				drawBeam(x1, x2, yh, stxt->l1, dir);
				yh = yh + (int) (0.8 * ystepst);
				drawBeam(x1, x2, yh, stxt->l2, dir);
				yh = yh + (int) (0.8 * ystepst);
				drawBeam(x1, x2, yh, stxt->l3, dir);
			} else {
				drawBeam(x1, x2, yl, stxt->l1, dir);
				yl = yl - (int) (0.8 * ystepst);
				drawBeam(x1, x2, yl, stxt->l2, dir);
				yl = yl - (int) (0.8 * ystepst);
				drawBeam(x1, x2, yl, stxt->l3, dir);
			}
		}
	}
}

// draw clef at xpos,yposst
// draw key at xpos,ypostb for all strings of track trk
// at the first line (l == 0), string names are printed
// at all other lines the text "TAB"
// note: print drum names instead in case of drumtrack

// LVIFIX: should return something like
// left margin (b28w ?)
// + max(width(key), width(drumnames))
// + right margin (b28w ?)

// Note: when called from mousepress the painter p is not active, which causes a crash
// when p->setFont() is called. Also applies to drawKeySig() and drawTimeSig().

int TrackPrint::drawKey(TabTrack *trk, bool doDraw, bool flop)
{
	int res = 0;
// Debug: show field width as horizontal line
//	int xstart = xpos;

	if (stTab) {
		if (doDraw) {
			p->setFont(*fTBar1);
		}
		const int lstStr = trk->string - 1;
		if (flop) {
			for (int i = 0; i < lstStr + 1; i++) {
				if (trk->trackMode() == TabTrack::DrumTab) {
					if (doDraw) {
						drawStrCntAt(xpos + tabpp + 3 * br8w / 2,
								 i,
								 drum_abbr[trk->tune[i]]);
					}
					res = 5 * br8w;
				} else {
					if (doDraw) {
						drawStrCntAt(xpos + tabpp + br8w / 2,
								 i,
								 Settings::noteName(trk->tune[i] % 12));
					}
					res = (int) (2.5 * br8w);
				}
			}
		} else {
			// calculate vertical position:
			// exactly halfway between top and bottom string
			// center "TAB" at this height, use spacing of 0.25 * char height
// 			if (doDraw) {
// 				QFontMetrics fm  = p->fontMetrics();
// 				int y = ypostb - ysteptb * lstStr / 2;
// 				int br8h = fm.boundingRect("8").height();
// 				y -= (int) ((0.5 + 0.25) * br8h);
// 				p->drawText(xpos + tabpp, y, "T");
// 				y += (int) ((1.0 + 0.25) * br8h);
// 				p->drawText(xpos + tabpp, y, "A");
// 				y += (int) ((1.0 + 0.25) * br8h);
// 				p->drawText(xpos + tabpp, y, "B");
// 			}
// 			res = (int) (2.5 * br8w);
		}
		if (onScreen)
			res = (int) (2.5 * br8w);
	}

	if (stNts && flop) {
		QString s;
		if (doDraw && fmp->getString(KgFontMap::G_Clef, s)) {
			// draw clef
			p->setFont(*fFeta);
			// LVIFIX: determine correct location (both clef and key)
			p->drawText(xpos + tabpp, yposst, s);
		}
		res = 4 * br8w;		// note: may increase res
	}

	if (doDraw || onScreen) {
		xpos += res;
// Debug: show field width as horizontal line
//		p->setPen(pLnBl);
//		p->drawLine(xstart, ypostb - 80, xpos, ypostb - 80);
	}
	return res;
}

// Key signature accidental placement table
// if keySig > 0, start at F and work to the right, notes are sharpened
// if keySig < 0, start at B and work to the left, notes are flattened
//                               F   C   G   D   A   E   B
static int accPosSharpTab[7] = { 3,  0,  4,  1, -2,  2, -1};
static int accPosFlatTab[7]  = {-4,  0, -3,  1, -2,  2, -1};

// draw key signature at xpos,yposst

int TrackPrint::drawKeySig(TabTrack *trk, bool doDraw)
{
	int res = 0;
// Debug: show field width as horizontal line
//	int xstart = xpos;
	if (stNts) {
		if (doDraw) {
			p->setFont(*fFeta);
		}
		int ypos;
		int sig = trk->bars()[0].keysig;
		if ((sig <= -8) || (8 <= sig)) {
			sig = 0;
		}
		if (sig != 0) {
			if (doDraw) {
				xpos += wNote;
			}
			res += wNote;
		}
		QString s;
		bool symFound;
		if (sig > 0) {
			symFound = fmp->getString(KgFontMap::Sharp_Sign, s);
			for (int i = 0; i < sig; i++) {
				ypos = accPosSharpTab[i];
				if (doDraw && symFound) {
					p->drawText(xpos, yposst - (ypos + 5) * ystepst / 2, s);
					xpos += (int) (0.8 * wNote);
				}
				res += (int) (0.8 * wNote);
			}
		} else if (sig < 0) {
			symFound = fmp->getString(KgFontMap::Flat_Sign, s);
			for (int i = 0; i > sig; i--) {
				ypos = accPosFlatTab[i + 6];
				if (doDraw && symFound) {
					p->drawText(xpos, yposst - (ypos + 5) * ystepst / 2, s);
					xpos += (int) (0.7 * wNote);
				}
				res += (int) (0.7 * wNote);
			}
		}
	}
// Debug: show field width as horizontal line
//	if (doDraw) {
//		p->setPen(pLnBl);
//		p->drawLine(xstart, ypostb - 78, xpos, ypostb - 78);
//	}
	return res;
}

// draw:
// - key + keysig (if fbol or changed)
// - timesig      (if first bar of track or changed)
// return xpixels used
// actually draws only when doDraw is true

// LVIFIX: use fbol ?

int TrackPrint::drawKKsigTsig(int bn, TabTrack *trk, bool doDraw, bool /* fbol */, bool flop)
{
	int res = 0;
	res += drawKey(trk, doDraw, flop);
	res += drawKeySig(trk, doDraw);
	res += drawTimeSig(bn, trk, doDraw);
	return res;
}

// draw "let ring" with point of arrowhead at x on string y
// LVIFIX: use xpos too ?

void TrackPrint::drawLetRing(int x, int y)
{
	p->drawLine(x,               ypostb - y * ysteptb,
				x - ysteptb / 3, ypostb - y * ysteptb - ysteptb / 3);
	p->drawLine(x,               ypostb - y * ysteptb,
				x - ysteptb / 3, ypostb - y * ysteptb + ysteptb / 3);
}

// draw notehead of type t with accidental a centered at x on staff line y
// note: lowest = 0, highest = 8
// uses yposst but ignores xpos
// LVIFIX: use xpos too ?

// LVIFIX: move 1/2 note head "a little bit" to the left

void TrackPrint::drawNtHdCntAt(int x, int y, int t, Accidentals::Accid a)
{
	// draw auxiliary lines
	int xdl = (int) (0.8 * wNote);	// x delta left of origin
	int xdr = (int) (0.8 * wNote);	// x delta right of origin
	p->setPen(pLnBl);
	int auxLine = y / 2;
	while (auxLine < 0) {
		p->drawLine(x - xdl, yposst - auxLine * ystepst,
		            x + xdr, yposst - auxLine * ystepst);
		auxLine++;
	}
	while (auxLine > 4) {
		p->drawLine(x - xdl, yposst - auxLine * ystepst,
		            x + xdr, yposst - auxLine * ystepst);
		auxLine--;
	}
	// draw note head
	KgFontMap::Symbol noteHead;
	if (t == 480) {
		// whole
		noteHead = KgFontMap::Whole_Note;
	} else if (t == 240) {
		// 1/2
		noteHead = KgFontMap::White_NoteHead;
	} else {
		// others
		noteHead = KgFontMap::Black_NoteHead;
	}
	p->setFont(*fFeta);
	QString s;
	if (fmp->getString(noteHead, s)) {
		p->drawText(x - wNote / 2, yposst - ystepst / 2 * (y - 1), s);
	}
	// draw accidentals
	KgFontMap::Symbol acc = KgFontMap::UndefinedSymbol;	// undefined symbol
	int accxposcor = 0;			// accidental xpos correction
	if (a == Accidentals::Sharp) {
		acc = KgFontMap::Sharp_Sign;
	} else if (a == Accidentals::Flat) {
		acc = KgFontMap::Flat_Sign;
		accxposcor = (int) (0.35 * wNote);
	} else if (a == Accidentals::Natural) {
		acc = KgFontMap::Natural_Sign;
		accxposcor = (int) (0.35 * wNote);
	}
	if (acc != KgFontMap::UndefinedSymbol && fmp->getString(acc, s)) {
		p->drawText((int) (x - 1.4 * wNote) + accxposcor,
				yposst - ystepst / 2 * (y - 2), s);
	}
}

// draw notestem and flag of type t and direction dir centered at x
// for notes on staff lines yl .. yh
// note: lowest = 0, highest = 8
// uses yposst but ignores xpos
// if t==0, draws only notestem between notes
// LVIFIX: use xpos too ?

// LVIFIX: lower stem doesn't touch upper stem
// LVIFIX: draw stem "a little bit" more to the left

void TrackPrint::drawNtStmCntAt(int x, int yl, int yh, int t, char dir)
{
	KgFontMap::Symbol flag = KgFontMap::UndefinedSymbol;
	int yoffset = 0;						// y offset flags
	switch (t) {
	case 0:   // none
		break;
	case 15:  // 1/32
		flag = (dir != 'd') ? KgFontMap::ThirtySecond_Flag : KgFontMap::ThirtySecond_FlagInv;
		yoffset = (int) (-1.3 * ystepst);
		break;
	case 30:  // 1/16
		flag = (dir != 'd') ? KgFontMap::Sixteenth_Flag : KgFontMap::Sixteenth_FlagInv;
		yoffset = (int) (-0.5 * ystepst);
		break;
	case 60:  // 1/8
		flag = (dir != 'd') ? KgFontMap::Eighth_Flag : KgFontMap::Eighth_FlagInv;
		break;
	case 120: // 1/4
		break;
	case 240: // 1/2
		break;
	case 480: // whole
		return;
	default:
		; // do nothing
	} // end switch (t)
	p->setPen(pLnBl);
	// draw stem (lower part)
	int xs;
	if (dir != 'd') {
		xs = (int) (x + 0.45 * wNote);		// x pos stem
	} else {
		xs = (int) (x - 0.45 * wNote);		// x pos stem
	}
	if (yl != yh) {
		int yld = yposst - (int) (ystepst * ((0.2 + yl) / 2));
		int yhd = yposst - (int) (ystepst * ((0.4 + yh) / 2));
		p->drawLine(xs, yld,
					xs, yhd);
	}
	if (dir != 'd') {
		// up
		if (t != 0) {
			QString s;
			// draw stem (upper part)
			if (fmp->getString(KgFontMap::Stem, s)) {
				p->drawText(xs, yposst - ystepst * yh / 2, s);
			}
			// draw flag(s)
			if ((flag != KgFontMap::UndefinedSymbol) && fmp->getString(flag, s)) {
				int yFlag = yposst - ystepst * yh / 2
						+ yoffset;
				p->drawText(xs, yFlag, s);
			}
		}
	} else {
		// down
		if (t != 0) {
			QString s;
			// draw stem (lower part)
			if (fmp->getString(KgFontMap::StemInv, s)) {
				p->drawText(xs, yposst - ystepst * yl / 2, s);
			}
			// draw flag(s)
			if ((flag != KgFontMap::UndefinedSymbol) && fmp->getString(flag, s)) {
				int yFlag = yposst - ystepst * yl / 2
						- yoffset;
				p->drawText(xs, yFlag, s);
			}
		}
	}
}

// draw rest of type t centered at x on staff line y
// note: lowest = 0, highest = 8
// uses yposst but ignores xpos
// LVIFIX: use xpos too ?

void TrackPrint::drawRstCntAt(int x, int y, int t)
{
	Q_UNUSED(y);

	KgFontMap::Symbol restSym;
	int yoffset = 0;
	switch (t) {
	case 15:  // 1/32
		restSym = KgFontMap::ThirtySecond_Rest;
		break;
	case 30:  // 1/16
		restSym = KgFontMap::Sixteenth_Rest;
		break;
	case 60:  // 1/8
		restSym = KgFontMap::Eighth_Rest;
		break;
	case 120: // 1/4
		restSym = KgFontMap::Quarter_Rest;
		break;
	case 240: // 1/2
		restSym = KgFontMap::Half_Rest;
		break;
	case 480: // whole
		restSym = KgFontMap::Whole_Rest;
		yoffset = 2;
		break;
	default:
		return; // do nothing
	} // end switch (t)
	QString s;
	if (fmp->getString(restSym, s)) {
		p->setFont(*fFeta);
		p->drawText(x - wNote / 2, yposst /** (y + yoffset) / 2*/, s);
	}
}

// draw staff lines at xpos,yposst width w

void TrackPrint::drawStLns(const QRect &rect)
{
	//const int lstStL = 4;
	// vertical lines at xpos and xpos+w-1
	p->setPen(pLnBl);

	QString s;
	fmp->getString(KgFontMap::Five_Line_Staff, s);

	QFontMetrics fm(*fFeta, p->device());

	p->setFont(*fFeta);
	int x = rect.left();
	// horizontal lines
	while ( x < rect.right() ) {
		QRect brect = fm.boundingRect( s );
		p->drawText(x, yposst /*- ystepst*/, s);
		x += brect.width();
	}

//	p->drawLine(xpos, yposst,
//				xpos, yposst - lstStL * ystepst);
//	p->drawLine(xpos + w - 1, yposst,
//				xpos + w - 1, yposst - lstStL * ystepst);

//	if (stTab) {
//		p->drawLine(xpos, yposst,
//					xpos, yposst + (7 + 3) * ystepst);
//		p->drawLine(xpos + w - 1, yposst,
//					xpos + w - 1, yposst + (7 + 3) * ystepst);
//	}
}

// draw string s centered at x on string n
// erase both tab and possible vertical line at location of string
// uses ypostb but ignores xpos
// LVIFIX: use xpos too ?

// As most characters don't start at the basepoint, we need to center
// the bounding rectangle, i.e. offset the character in the x direction
// by (left + right) / 2.
// Strictly speaking this needs to be done in the y dir too, but here
// the error is very small.

void TrackPrint::drawStrCntAt(int x, int n, const QString s)
{
	QFontMetrics fm = p->fontMetrics();
	const int yOffs = fm.boundingRect("8").height() / 2;
	const QRect r   = fm.boundingRect(s);
	int xoffs       = - (r.left() + r.right()) / 2;
	p->setPen(pLnWh);
	int ew_2 = eraWidth(s) / 2;
	p->drawLine(x - ew_2, ypostb - n * ysteptb,
				x + ew_2, ypostb - n * ysteptb);
	p->drawLine(x, ypostb - n * ysteptb - ysteptb / 2,
				x, ypostb - n * ysteptb + ysteptb / 2);
	p->setPen(pLnBl);
	p->drawText(x + xoffs, ypostb - n * ysteptb + yOffs, s);
}

// print timesig if necessary
// LVIFIX: may need to center horizontally
int TrackPrint::drawTimeSig(int bn, TabTrack *trk, bool doDraw)
{
	int res = 0;
// Debug: show field width as horizontal line
//	int xstart = xpos;
	if (trk->showBarSig(bn)) {
		if (doDraw) {
			int brth;
			QFontMetrics fm = p->fontMetrics();
			QString time;
			int y;
			if (stNts) {
				// staff
				p->setFont(*fFetaNr);
				fm = p->fontMetrics();
				// calculate vertical position:
				// exactly halfway between top and bottom string
				y = yposst - ystepst * 2.5;
				// center the timesig at this height
				// use spacing of 0.2 * char height
				time.setNum(trk->bars()[bn].time1);
				brth = fm.boundingRect(time).height();
				y -= (int) (0.1 * brth);
				p->drawText(xpos + tsgppScore, y, time);
				time.setNum(trk->bars()[bn].time2);
				y += (int) (1.2 * brth);
				p->drawText(xpos + tsgppScore, y, time);
			}
			if (stTab) {
				// tab bar
				p->setFont(*fTSig);
				fm = p->fontMetrics();
				// calculate vertical position:
				// exactly halfway between top and bottom string
				y = ypostb - ysteptb * (trk->string - 1) / 2;
				// center the timesig at this height
				// use spacing of 0.2 * char height
				time.setNum(trk->bars()[bn].time1);
				brth = fm.boundingRect(time).height();
				y -= (int) (0.1 * brth);
				p->drawText(xpos + tsgpp, y, time);
				time.setNum(trk->bars()[bn].time2);
				y += (int) (1.2 * brth);
				p->drawText(xpos + tsgpp, y, time);
				p->setFont(*fTBar1);
			}
			if (stNts || stTab) {
				xpos += tsgfw;
			}
		}
		if (stNts || stTab) {
			res += tsgfw;
		}
	} else if (onScreen) {
		res += tsgfw;
		xpos += tsgfw;
	}
// Debug: show field width as horizontal line
//	if (doDraw) {
//		p->setPen(pLnBl);
//		p->drawLine(xstart, ypostb - 76, xpos, ypostb - 76);
//	}
	return res;
}

// return width (of barline) to erase for string s

int TrackPrint::eraWidth(const QString s)
{
	QFontMetrics fm = p->fontMetrics();
	const int brw8  = fm.boundingRect("8").width();
	const int brws  = fm.boundingRect(s).width();
	return (int) (brws + 0.4 * brw8);
}

// find line of highest/lowest note in column cl for voice v in tabtrack trk
// returns false if not found
// precondition: calcStepAltOct() and calcVoices() must have been called

bool TrackPrint::findHiLo(int cl, int v, TabTrack *trk, int & hi, int & lo)
{
	bool found = false;
	hi = 0;						// prevent uninitialized variable
	lo = 0;						// prevent uninitialized variable
	// loop over all strings
	/*
	cout << "v=" << v;
	*/
	for (int i = 0; i < trk->string; i++) {
	/*
		cout
			<< " i=" << i
			<< " v[i]=" << (int) trk->c[cl].v[i]
			<< endl;
	*/
		if (trk->c[cl].v[i] == v) {
			int ln = line((QChar) trk->c[cl].stp[i], trk->c[cl].oct[i]);
			if (found) {
				// found note in this voice, but not the first
				if (ln < lo) lo = ln;
				if (ln > hi) hi = ln;
			} else {
				// found first note in this voice
				lo = ln;
				hi = ln;
			}
			found = true;
		}
	}
	return found;
}

// return x offset in pixels of bar bn's first column in track trk
// used by TrackView::mousePressEvent() to convert mouse click to column
// depends on whether key, keysig, timesig and first columns accidentals
// need to be drawn
// drawing key and keysig is determined by caller and indicated by fbol
// (first bar on line)

int TrackPrint::getFirstColOffs(int bn, TabTrack *trk, bool fbol)
{
	bool doDraw = false;
	return drawKKsigTsig(bn, trk, doDraw, fbol, bn == 0)
		+ nt0fw			// see drawBar (space before first note)
		+ (int) (0.9 * wNote);	// see drawBar (space before first note)
//	return (int) (6 * br8w + 0.9 * wNote + 2);	// LVIFIX: correct value
}

// initialize fonts

void TrackPrint::initFonts(QFont *f1, QFont *f2, QFont *f3, QFont *f4, QFont *f5)
{
//	cout << "TrackPrint::initFonts()" << endl;
	fTBar1   = f1;
	fTBar2   = f2;
	fTSig    = f3;
	fFeta    = f4;
	fFetaNr  = f5;
}

// initialize paper format and font dependent metrics
// note: some metrics depend on onScreen: setOnScreen() must have been called

void TrackPrint::initMetrics()
{
//	cout << "TrackPrint::initMetrics()" << endl;
	// determine font-dependent bar metrics
	QFontMetrics fm(*fTBar1);
	br8h = fm.boundingRect("8").height();
	br8w = fm.boundingRect("8").width();
//	cout << "br8w=" << br8w;
	ysteptb = (int) (0.9 * fm.ascent());
	tabfw = 4 * br8w;
	tabpp =     br8w;
	tsgfw = 3 * br8w;
	tsgpp = 1 * br8w;
	nt0fw = 2 * br8w;
	ntlfw =     br8w / 2;
	if (onScreen) {
		ysteptb = (int) (0.95 * fm.ascent());
		tsgfw = (int) (4.5 * br8w);
		tsgpp = 2 * br8w;
	}

	fm = QFontMetrics(*fFetaNr);
	tsgppScore = fm.boundingRect("8").width();
	if( onScreen )
	  tsgppScore *= 2;

	// determine font-dependent staff metrics
	QString s;
	if (fFeta && fmp->getString(KgFontMap::Black_NoteHead, s)) {
		fm  = QFontMetrics(*fFeta);
		QRect r = fm.boundingRect(s);
		ystepst = (int) (0.183 * r.height());
		wNote   = r.width();
	} else {
		ystepst = 0;
		wNote   = 0;
	}
//	cout << " wNote=" << wNote << endl;
}

// initialize pens
// LVIFIX: which penwidth ?
// penwidth 2 is OK on my deskjet for printing quality = normal
// penwidth 3 is OK on my deskjet for printing quality = presentation

void TrackPrint::initPens()
{
//	cout << "TrackPrint::initPens()" << endl;
	const int lw = 2;
	pLnBl = QPen(Qt::black, lw);
	pLnWh = QPen(Qt::white, lw);
}

// init printing style variables

void TrackPrint::initPrStyle()
{
	initPrStyle(Settings::printingStyle());
}

void TrackPrint::initPrStyle(int prStyle)
{
//	cout << "TrackPrint::initPrStyle()" << endl;
	// check what was configured
	switch (prStyle) {
	case 0:
		// (full) tab only
		stNts = false;
		stTab = true;
		break;
	case 1:
		// notes
		stNts = true;
		stTab = false;
		break;
	case 2:
		// notes + (full) tab
		stNts = true;
		stTab = true;
		break;
	case 3:
		// notes + (minimum) tab
		// not implemented yet, fall through to default
		// break;
	default:
		stNts = false;
		stTab = true;
	}
	// no notes if feta fonts not found
	if (!fFeta) {
		stNts = false;
	}
}

// return staffline where note must be drawn (lowest = 0, highest = 8)

static const QString notes[7] = {"C", "D", "E", "F", "G", "A", "B"};

int TrackPrint::line(const QString step, int oct)
{
	const int ClefOctCh = -1;
	int cn = 0;				// if note not found, default to "C"
	for (int i = 0; i < 7; i++) {
		if (notes[i] == step) {
			cn = i;
		}
	}
	// magic constant "30" maps G3 to the second-lowest staffline
	// note implicit clef-octave-change of -1
	return cn + 7 * (oct - ClefOctCh) - 30;
}

// set on screen mode

void TrackPrint::setOnScreen(bool scrn)
{
//	cout << "TrackPrint::setOnScreen(scrn=" << scrn << ")" << endl;
	onScreen = scrn;
}

void TrackPrint::setPainter(QPainter *paint)
{
	p = paint;
}

int TrackPrint::calcYPosTb(int numOfStrings)
{
	return ypostb = yposst + (int) ((TOPSPTB + numOfStrings - 0.5) * ysteptb) + bottomStMargin();
}

int TrackPrint::calcYPosSt(int top)
{
	if(!viewscore)
		return yposst = top;
	else
		return yposst = top + (int) ((TOPSPST + NLINEST - 1) * ystepst);
}

int TrackPrint::bottomTbMargin() const
{
	return ysteptb * BOTSPTB;
}

int TrackPrint::bottomStMargin() const
{
	if(viewscore)
		return ystepst * BOTSPST;
	else
		return 0;
}
