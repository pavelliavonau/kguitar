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

// LVIFIX: this file needs to be redesigned
// - code cleanup
// - xpos/ypos interface
// - adapt linewidth to resolution of output device

#include "settings.h"

#include <iostream>		// required for cout and friends
using namespace std;		// required for cout and friends

#include <qfontmetrics.h>
#include <qpainter.h>

#include "accidentals.h"
#include "global.h"
#include "tabtrack.h"
#include "trackprint.h"

TrackPrint::TrackPrint()
{
//	cout << "TrackPrint::TrackPrint() @ " << this << endl;
	p = 0;
	onScreen = FALSE;
}

// return expandable width in pixels of bar bn in track trk
// this part of the bar is expanded to fill a line completely
// extra space will be added between notes

int TrackPrint::barExpWidth(int bn, TabTrack *trk)
{
	int w = 0;
	for (uint t = trk->b[bn].start; (int) t <= trk->lastColumn(bn); t++)
		w += colWidth(t, trk);
	return w;
}

// return width in pixels of bar br in track trk

int TrackPrint::barWidth(int bn, TabTrack *trk)
{
	int w = 0;
	for (uint t = trk->b[bn].start; (int) t <= trk->lastColumn(bn); t++)
		w += colWidth(t, trk);
	// LVIFIX: when KGuitar supports changing the key at the start of any bar,
	// calculate space for keysig here
	if (trk->showBarSig(bn))
		w += tsgfw;				// space for timesig
	w += nt0fw;					// space before first note
	int cl = trk->b[bn].start;	// first column of bar
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
// depends on note length only
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
		w *= 7;
		w /= 30;
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

void TrackPrint::drawBar(int bn, TabTrack *trk, int es, int& sx, int& sx2)
{
//	cout << "TrackPrint::drawBar(" << bn << ", " << trk << ", " << es << ")" << endl;

	TabTrack *curt = trk;		// LVIFIX

	int lastxpos = 0;			// fix compiler warning
	int extSpAftNote = 0;		// extra space, divided over the notes
	int xdelta = 0;				// used for drawing beams, legato and slide
	bool ringing[MAX_STRINGS];
	uint s = curt->string - 1;
	int i = 0;
	int trpCnt = 0;				// triplet count

	for (uint i = 0; i <= s; i++) {
		ringing[i] = FALSE;
	}

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
			time.setNum(trk->b[bn].time1);
			brth = fm.boundingRect(time).height();
			y -= (int) (0.1 * brth);
			p->drawText(xpos + tsgpp, y, time);
			time.setNum(trk->b[bn].time2);
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
			time.setNum(trk->b[bn].time1);
			brth = fm.boundingRect(time).height();
			y -= (int) (0.1 * brth);
			p->drawText(xpos + tsgpp, y, time);
			time.setNum(trk->b[bn].time2);
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

	// space before first note
	xpos += nt0fw;
	bool needWacc = FALSE;
	int cl = trk->b[bn].start;		// first column of bar
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

	// loop t over all columns in this bar and print them
	for (uint t = trk->b[bn].start; (int) t <= trk->lastColumn(bn); t++) {

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
			if ((t > 0) && (t > (unsigned) curt->b[bn].start)
						&& (curt->c[t-1].l == curt->c[t].l))
				xdelta = lastxpos;
			else
				xdelta = xpos + ysteptb / 2;

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
			case 480: // whole
				break;
			} // end switch (curt->c[t].l)

			// Draw dot is not here, see: "Draw the number column"

			// Length of interval to next column - adjusted if dotted
			// calculated here because it is required by triplet code

			xdelta = colWidth(t, trk);
			extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;

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

			// Draw arcs to backward note

			if (curt->c[t].flags & FLAG_ARC)
				p->drawArc(lastxpos, ypostb + 2 * ysteptb + 1,
						   xpos - lastxpos, ysteptb / 2 + 1, 0, -180 * 16);

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
						drawNtHdCntAt(xpos, ln, tp, trk->c[tt].acc[i]);
						nhPrinted++;
						// Draw dot, must be at odd line -> set lsbit
						// LVIFIX: add support for double dot
						if (dt) {
							QString s;
							s = QChar(0xA7);
							int y = ln | 1;
							p->setFont(*fFeta);
							p->drawText((int) (xpos + 0.8 * wNote),
										yposst - ystepst * y / 2, s);
						}
					}
				}
				if (trk->c[t].stl.l1 != 'n') {
					// note is beamed, don't draw lower stem and flag
					drawNtStmCntAt(xpos, yl, yh, 0, 'd');
					// remember position
					trk->c[t].stl.bp.setX((int) (xpos - 0.45 * wNote));
					int yhd = yposst - (int) (ystepst * ((-0.4 + yl) / 2));
					trk->c[t].stl.bp.setY(yhd);
				} else {
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
						drawNtHdCntAt(xpos, ln, tp, trk->c[tt].acc[i]);
						nhPrinted++;
						// Draw dot, must be at odd line -> set lsbit
						// LVIFIX: add support for double dot
						if (dt) {
							QString s;
							s = QChar(0xA7);
							int y = ln | 1;
							p->setFont(*fFeta);
							p->drawText((int) (xpos + 0.8 * wNote),
										yposst - ystepst * y / 2, s);
						}
					}
				}
				if (trk->c[t].stu.l1 != 'n') {
					// note is beamed, don't draw upper stem and flag
					drawNtStmCntAt(xpos, yl, yh, 0, 'u');
					// remember position
					trk->c[t].stu.bp.setX((int) (xpos + 0.45 * wNote));
					int yhd = yposst - (int) (ystepst * ((0.4 + yh) / 2));
					trk->c[t].stu.bp.setY(yhd);
				} else {
					drawNtStmCntAt(xpos, yl, yh, tp, 'u');
				}
			}

			// if no note printed, print rest
			if (nhPrinted == 0) {
				drawRstCntAt(xpos, 4, trk->c[t].l);
			}

		} // end if (stNts ...

		// end drawing notes

		if (stTab) {

			// Draw the number column including effects
			p->setFont(*fTBar1);
			int ew_2 = 0;			// used for positioning effects
			QString note = "";
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
						QPointArray a(4);
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
						QPointArray a(4);
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
					if ((t < curt->c.size() - 1) && (curt->c[t + 1].a[i] >= 0)) {
						extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;
						p->drawArc(xpos + ew_2, ypostb - i * ysteptb - ysteptb / 2,
								   xdelta + extSpAftNote - 2 * ew_2, ysteptb / 2,
								   0, 180 * 16);
					}
					break;
				case EFFECT_SLIDE:
					// the slide symbol should be as wide as the line
					// between this note and the next. see drawStrCntAt.
					// extra space between notes must also be added
					if ((t < curt->c.size() - 1) && (curt->c[t + 1].a[i] >= 0)) {
						extSpAftNote = (colWidth(t, trk) * es) / barExpWidthLeft;
						if (curt->c[t + 1].a[i] > curt->c[t].a[i]) {
							p->drawLine(xpos + ew_2,
										ypostb - i * ysteptb + ysteptb / 3 - 2,
										xpos + xdelta + extSpAftNote - ew_2,
										ypostb - i * ysteptb - ysteptb / 3 + 2);
						} else {
							p->drawLine(xpos + ew_2,
										ypostb - i * ysteptb - ysteptb / 3 + 2,
										xpos + xdelta + extSpAftNote - ew_2,
										ypostb - i * ysteptb + ysteptb / 3 - 2);
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
					int sz_2 = ysteptb / 4;
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

	// draw beams
	if (stNts) {
		drawBeams(bn, 'd', trk);
		drawBeams(bn, 'u', trk);
	}

	// space after last note
	if (! onScreen) {
		xpos += ntlfw;
	}

	// end bar
	if (stTab) {
		// show notes still ringing at end of bar
		for (unsigned int i = 0; i <= s; i++) {
			if (ringing[i]) {
				int ew_3 = eraWidth("0") / 4;
				drawLetRing(xpos - ew_3, i);
				ringing[i] = FALSE;
			}
		}
		// draw vertical line
		p->drawLine(xpos, ypostb,
		            xpos, ypostb - (trk->string - 1) * ysteptb);
	}
	if (stNts) {
		// draw vertical line
		p->drawLine(xpos, yposst,
		            xpos, yposst - 4 * ystepst);
	}
	// LVIFIX
	xpos += 1;
}

// draw bar lines at xpos,ypostb width w for all strings of track trk

void TrackPrint::drawBarLns(int w, TabTrack *trk)
{
	const int lstStr = trk->string - 1;
	// vertical lines at xpos and xpos+w-1
	p->setPen(pLnBl);
	p->drawLine(xpos, ypostb, xpos, ypostb - lstStr * ysteptb);
	p->drawLine(xpos + w - 1, ypostb, xpos + w - 1, ypostb - lstStr * ysteptb);
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
	QPointArray a;
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

void TrackPrint::drawBeams(int bn, char dir,	TabTrack *trk)
{
	// cout << "SongPrint::drawBeams(" << bn << ", " << dir << ")" << endl;
	StemInfo * stxt = 0;
	for (uint t = trk->b[bn].start; (int) t <= trk->lastColumn(bn); t++) {
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
	for (uint t = trk->b[bn].start; (int) t <= trk->lastColumn(bn); t++) {
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

void TrackPrint::drawKey(int l, TabTrack *trk)
{
	if (stNts) {
		// draw clef
		QString s;
		s = QChar(0x6A);
		p->setFont(*fFeta);
		// LVIFIX: determine correct location (both clef and key)
		p->drawText(xpos + tabpp, yposst - ystepst, s);
	}

	if (stTab) {
		p->setFont(*fTBar1);
		const int lstStr = trk->string - 1;
		if (l == 0) {
			for (int i = 0; i < lstStr + 1; i++) {
				if (trk->trackMode() == TabTrack::DrumTab) {
					drawStrCntAt(xpos + tabpp + 3 * br8w / 2,
								 i,
								 drum_abbr[trk->tune[i]]);
				} else {
					drawStrCntAt(xpos + tabpp + br8w / 2,
								 i,
								 Settings::noteName(trk->tune[i] % 12));
				}
			}
		} else {
			// calculate vertical position:
			// exactly halfway between top and bottom string
			// center "TAB" at this height, use spacing of 0.25 * char height
			QFontMetrics fm  = p->fontMetrics();
			int y = ypostb - ysteptb * lstStr / 2;
			int br8h = fm.boundingRect("8").height();
			y -= (int) ((0.5 + 0.25) * br8h);
			p->drawText(xpos + tabpp, y, "T");
			y += (int) ((1.0 + 0.25) * br8h);
			p->drawText(xpos + tabpp, y, "A");
			y += (int) ((1.0 + 0.25) * br8h);
			p->drawText(xpos + tabpp, y, "B");
		}
	}
}

// Key signature accidental placement table
// if keySig > 0, start at F and work to the right, notes are sharpened
// if keySig < 0, start at B and work to the left, notes are flattened
//                               F   C   G   D   A   E   B
static int accPosSharpTab[7] = { 3,  0,  4,  1, -2,  2, -1};
static int accPosFlatTab[7]  = {-4,  0, -3,  1, -2,  2, -1};

// draw key signature at xpos,yposst

void TrackPrint::drawKeySig(TabTrack *trk)
{
	QString s;
	if (stNts) {
		p->setFont(*fFeta);
		int ypos;
		int sig = trk->b[0].keysig;
		if ((sig <= -8) || (8 <= sig)) {
			sig = 0;
		}
		if (sig != 0) {
			xpos += wNote;
		}
		if (sig > 0) {
			s = QChar(0x201c);
			for (int i = 0; i < sig; i++) {
				ypos = accPosSharpTab[i];
				p->drawText(xpos, yposst - (ypos + 5) * ystepst / 2, s);
				xpos += (int) (0.8 * wNote);
			}
		} else if (sig < 0) {
			s = QChar(0x201e);
			for (int i = 0; i > sig; i--) {
				ypos = accPosFlatTab[i + 6];
				p->drawText(xpos, yposst - (ypos + 5) * ystepst / 2, s);
				xpos += (int) (0.7 * wNote);
			}
		}
	}
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
	int noteHead = 0;
	if (t == 480) {
		// whole
		noteHead = 0x22;
	} else if (t == 240) {
		// 1/2
		noteHead = 0x23;
	} else {
		// others
		noteHead = 0x24;
	}
	QString s;
	s = QChar(noteHead);
	p->setFont(*fFeta);
	p->drawText(x - wNote / 2, yposst - ystepst * y / 2, s);
	// draw accidentals
	int acc = 0;				// accidental char code
	int accxposcor = 0;			// accidental xpos correction
	if (a == Accidentals::Sharp) {
		acc = 0x201c;
	} else if (a == Accidentals::Flat) {
		acc = 0x201e;
		accxposcor = (int) (0.35 * wNote);
	} else if (a == Accidentals::Natural) {
		acc = 0x201d;
		accxposcor = (int) (0.35 * wNote);
	}
	s = QChar(acc);
	p->drawText((int) (x - 1.4 * wNote) + accxposcor,
				yposst - ystepst * y / 2, s);
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
	int flagCh = 0;
	int yoffset = 0;						// y offset flags
	switch (t) {
	case 0:   // none
		break;
	case 15:  // 1/32
		flagCh = (dir != 'd') ? 0x5C :   0x61;
		yoffset = (int) (-1.3 * ystepst);
		break;
	case 30:  // 1/16
		flagCh = (dir != 'd') ? 0x5B : 0x2018;
		yoffset = (int) (-0.5 * ystepst);
		break;
	case 60:  // 1/8
		flagCh = (dir != 'd') ? 0x5A :   0x5F;
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
			s = QChar(0x64);
			p->drawText(xs, yposst - ystepst * yh / 2, s);
			// draw flag(s)
			s = QChar(flagCh);
			int yFlag = yposst - ystepst * yh / 2
						- (int) (3.5 * ystepst)
						+ yoffset;
			p->drawText(xs, yFlag, s);
		}
	} else {
		// down
		if (t != 0) {
			QString s;
			// draw stem (lower part)
			s = QChar(0x65);
			p->drawText(xs, yposst - ystepst * yl / 2, s);
			// draw flag(s)
			s = QChar(flagCh);
			int yFlag = yposst - ystepst * yl / 2
						+ (int) (3.5 * ystepst)
						+ yoffset;
			p->drawText(xs, yFlag, s);
		}
	}
}

// draw rest of type t centered at x on staff line y
// note: lowest = 0, highest = 8
// uses yposst but ignores xpos
// LVIFIX: use xpos too ?

void TrackPrint::drawRstCntAt(int x, int y, int t)
{
	int restSym = 0;
	int yoffset = 0;
	switch (t) {
	case 15:  // 1/32
		restSym = 0x02D9;
		break;
	case 30:  // 1/16
		restSym = 0xAF;
		break;
	case 60:  // 1/8
		restSym = 0x02D8;
		break;
	case 120: // 1/4
		restSym = 0x02C7;
		break;
	case 240: // 1/2
		restSym = 0xB4;
		break;
	case 480: // whole
		restSym = 0x60;
		yoffset = 2;
		break;
	default:
		return; // do nothing
	} // end switch (t)
	QString s;
	s = QChar(restSym);
	p->setFont(*fFeta);
	p->drawText(x - wNote / 2, yposst - ystepst * (y + yoffset) / 2, s);
}

// draw staff lines at xpos,yposst width w

void TrackPrint::drawStLns(int w)
{
	const int lstStL = 4;
	// vertical lines at xpos and xpos+w-1
	p->setPen(pLnBl);
	p->drawLine(xpos, yposst,
				xpos, yposst - lstStL * ystepst);
	p->drawLine(xpos + w - 1, yposst,
				xpos + w - 1, yposst - lstStL * ystepst);
	// horizontal lines from xpos to xpos+w-1
	for (int i = 0; i < lstStL+1; i++) {
		p->drawLine(xpos, yposst - i * ystepst,
					xpos + w - 1, yposst - i * ystepst);
	}
	if (stTab) {
		p->drawLine(xpos, yposst,
					xpos, yposst + (7 + 3) * ystepst);
		p->drawLine(xpos + w - 1, yposst,
					xpos + w - 1, yposst + (7 + 3) * ystepst);
	}
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
	p->setFont(*fTBar1);
	QFontMetrics fm  = p->fontMetrics();
	br8h = fm.boundingRect("8").height();
	br8w = fm.boundingRect("8").width();
	ysteptb = (int) (0.9 * fm.ascent());
	tabfw = 4 * br8w;
	tabpp =     br8w;
	tsgfw = 5 * br8w;
	tsgpp = 2 * br8w;
	nt0fw = 2 * br8w;
	ntlfw =     br8w / 2;
	if (onScreen) {
		tsgfw = (int) (4.5 * br8w);
		tsgpp = 4 * br8w;
	}
	// determine font-dependent staff metrics
	QChar c;
	QRect r;
	if (fFeta) {
		p->setFont(*fFeta);
		fm  = p->fontMetrics();
		c   = 0x24;
		r   = fm.boundingRect(c);
		ystepst = (int) (0.95 * r.height());
		wNote   = r.width();
	} else {
		ystepst = 0;
		wNote   = 0;
	}
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
//	cout << "TrackPrint::initPrStyle()" << endl;
	// check what was configured
	switch (Settings::printingStyle()) {
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
