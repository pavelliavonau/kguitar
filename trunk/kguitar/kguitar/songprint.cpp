/***************************************************************************
 * songprint.cpp: implementation of SongPrint class
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

// LVIFIX: HACK HACK HACK ! means that xpos/ypos handling needs rethinking

// Notes on vertical spacing:
// Assuming we can accomodate notes one octave above (stem up) and below
// (stem down) the staff, height required is 4 * ystepst (staff)
// + 2 * 7 * ystepst (space above and below). Vertical spacing between
// staffs of between staff and tabbar is 3 * ystepst.
// The tabbar needs (nstrings - 1) * ysteptb + 3 * ysteptb (space below).
// Line spacing is 2 * ysteptb.
//
// Total space required for staff + tabbar + spacing is:
// 21 * ystepst + (nstrings + 4) * ysteptb

// Notes on horizontal spacing:
// If the first note in a measure has an accidental, then space before
// first note must be increased.
// Space between notes depends (especially for short notes) on both
// accidentals and flags/beams. Therefore accidentals and beams need to be
// determined before calculating horizontal note spacing.

// LVIFIX: check width of lower part note stems (seems a bit too thin)

// LVIFIX: "link with previous" doesn't work well
//         - prints undefined effects
//           (because TabColumn's e[i] are filled with random data)
//         - if at start of bar, links to left margin

// LVIFIX: "ringing" and "link with previous" don't work well together
//         doesn't work in midi export either

// LVIFIX: rests in lower voice are not supported

// LVIFIX: notes in triplet are (sometimes ?) beamed incorrectly

// LVIFIX: check and/or improve handling of extSpAftNote

// LVIFIX: print "8va" below G-clef (when applicable (always, right now))

#include "accidentals.h"
#include "songprint.h"
#include "tabsong.h"
#include "tabtrack.h"
#include "trackprint.h"
#include "settings.h"

#include <kprinter.h>
#include <qmemarray.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprinter.h>
#include <kglobal.h>

static const QString notes[7] = {"C", "D", "E", "F", "G", "A", "B"};

/***************************************************************************
 * class SongPrint
 ***************************************************************************/

// SongPrint constructor

SongPrint::SongPrint()
{
	p = new QPainter;
	trp = new TrackPrint;
}

// SongPrint destructor

SongPrint::~SongPrint()
{
	delete p;
	delete trp;
}

// draw header of song song, page n
// note: initializes ypostb

void SongPrint::drawPageHdr(int n, TabSong *song)
{
	p->setFont(fHdr1);
	p->drawText(0, hdrh1, song->title + " - " + song->author);
	QString pgNr;
	pgNr.setNum(n);
	QFontMetrics fm  = p->fontMetrics();
	int brnw = fm.boundingRect(pgNr).width();
	p->setFont(fHdr2);
	p->drawText(pprw - brnw, hdrh1, pgNr);
	p->setFont(fHdr3);
	p->drawText(0, hdrh1 + hdrh2, "Transcribed by " + song->transcriber);
	ypostb = hdrh1 + hdrh2 + hdrh3;
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

void SongPrint::drawStrCntAt(int x, int n, const QString s)
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

// debug: show xpos as arrow below the staff

void SongPrint::drawXpos()
{
	p->setPen(pLnBl);
	p->drawLine(xpos, yposst, xpos, yposst + 2 * wNote);
	p->drawLine(xpos - wNote / 2, yposst + wNote, xpos, yposst);
	p->drawLine(xpos + wNote / 2, yposst + wNote, xpos, yposst);
	p->drawLine(xpos - wNote / 2, yposst + wNote,
				xpos + wNote / 2, yposst + wNote);
}

// return width (of barline) to erase for string s

int SongPrint::eraWidth(const QString s)
{
	QFontMetrics fm = p->fontMetrics();
	const int brw8  = fm.boundingRect("8").width();
	const int brws  = fm.boundingRect(s).width();
	return (int) (brws + 0.4 * brw8);
}

// helper function to initialize font and check exactly this font was found
// return true on succes

// LVIFIX: according to the documentation, the following should
// supply the font family actually used.
// p->setFont(fFeta);
// QFontInfo info = p->fontInfo();
// QString actualFamily = info.family();
// Unfortunately, instead it returns the font family requested.
// This prevents checking if the correct font is used.
// Workaround is to use the rawName, which starts with
// "<family>-<pointsize>:"
// Another issue is that font "TeX feta-nummer10" cannot be found.
// Tested on RedHat 8.0 with Qt 3.0.5.

static bool initExactFont(const QString fn, int fs, QFont & fnt)
{
	QString an;					// actual name
	QString rn;					// requested name

	rn.setNum(fs);
	rn = fn + "-" + rn;
	fnt = QFont(fn, fs);
	an = fnt.rawName().section(':', 0, 0);
	// LVIFIX: debug
	// cout << "rn='" << rn << "'" << endl;
	// cout << "an='" << an << "'" << endl;
	return (rn == an);
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

	// following fonts must be found: if not, printing of notes is disabled
	QString fn;
	fFetaFnd = true;
	fn = "TeX feta19";
	if (!initExactFont(fn, 18, fFeta)) {
		kdDebug() << "KGuitar: could not find font '" << fn << "'" << endl;
		fFetaFnd = false;
	}
	fn = "TeX feta-nummer10";
	if (!initExactFont(fn, 10, fFetaNr)) {
		// LVIFIX: font is not found (don't know why), but acceptable
		// replacement is, therefore suppress error
		// cout << "KGuitar: could not find font '" << fn << "'" << endl;
		// fFetaFnd = false;
	}
}

// initialize paper format and font dependent metrics

void SongPrint::initMetrics(QPaintDevice *printer)
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
	ysteptb = (int) (0.9 * fm.ascent());
	tabfw = 4 * br8w;
	tabpp =     br8w;
	tsgfw = 5 * br8w;
	tsgpp = 2 * br8w;
	nt0fw = 2 * br8w;
	ntlfw =     br8w / 2;
	// determine font-dependent page header metrics
	p->setFont(fHdr1);
	fm  = p->fontMetrics();
	hdrh1 = fm.ascent();
	p->setFont(fHdr3);
	fm  = p->fontMetrics();
	hdrh2 = (int) (1.5 *fm.height());
	hdrh3 = 2 * ysteptb;
	p->setFont(fHdr2);
	fm  = p->fontMetrics();
	hdrh4 = 2 * fm.height();
	// determine font-dependent staff metrics
	QChar c;
	QRect r;
	p->setFont(fFeta);
	fm  = p->fontMetrics();
	c = 0x24;
	r = fm.boundingRect(c);
	ystepst = (int) (0.95 * r.height());
	wNote = r.width();
}

// initialize pens
// LVIFIX: which penwidth ?
// penwidth 2 is OK on my deskjet for printing quality = normal
// penwidth 3 is OK on my deskjet for printing quality = presentation

void SongPrint::initPens()
{
	const int lw = 2;
	pLnBl = QPen(Qt::black, lw);
	pLnWh = QPen(Qt::white, lw);
}

// init printing style variables

void SongPrint::initPrStyle()
{
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
	// LVIFIX: feta font handling to be delegated to TrackPrint
	if (!fFetaFnd) {
		stNts = false;
	}
}

// print TabSong song on KPrinter printer

void SongPrint::printSong(KPrinter *printer, TabSong *song)
{
//	cout << "SongPrint::printSong(" << printer << ", " << song << ")" << endl;

// LVIFIX: sometimes KGuitar crashes when print preview pops up.
// check if using QPrinter instead fixes the crashes,
// which would suggest that KPrinter is the cause.

// choose either KPrinter
	KPrinter * prntr = printer;
// or choose QPrinter
//	QPrinter * prntr = new QPrinter(QPrinter::HighResolution);
//	if (!prntr->setup())
//		return;
// end of choice

	// start painting on printer
	if (!p->begin(prntr))
		return;

	// initialize fonts, must be done before initMetrics
	// (metrics depend on fonts)
	initFonts();

	// initialize metrics
	initMetrics(prntr);

	// initialize pens
	initPens();
	p->setPen(pLnBl);

	// init printing style variables
	initPrStyle();

	// now also initialize the TrackPrint
	trp->initFonts();
	trp->setPainter(p);
	trp->initMetrics();
	trp->initPens();
	trp->initPrStyle();

	// print page header
	int pgNr = 1;
	drawPageHdr(pgNr, song);

	// ypostb now is where either the empty space above the staff,
	// the top barline of the first tab bar, or the top of the track name
	// should be

	uint trkPr = 0;				// tracks printed

	// loop while tracks left in the song
	while (trkPr < (song->t).count()) {

		TabTrack *trk = (song->t).at(trkPr);

		// Determine voices for each note
		trk->calcVoices();

		// Determine step/alter/octave/accidental for each note
		trk->calcStepAltOct();

		// Determine beams for this track
		trk->calcBeams();

		// print the track header
		if ((song->t).count() > 1)
		{
			p->setFont(fHdr2);
			QFontMetrics fm  = p->fontMetrics();
			p->drawText(0, ypostb + fm.ascent(), trk->name);
			ypostb += hdrh4;
		}

		int l = 0;					// line nr in the current track
		uint brsPr = 0;				// bars printed
		int bn = 0;					// current bar nr

		// precalculate bar widths
		QMemArray<int> bew(trk->b.size());
		QMemArray<int> bw(trk->b.size());
		for (uint bn = 0; bn < trk->b.size(); bn++) {
			bew[bn] = trp->barExpWidth(bn, trk);
			bw[bn]  = trp->barWidth(bn, trk);
		}

		// loop while bars left in the track
		while (brsPr < trk->b.size()) {

			if (stNts) {
				// move yposst to the bottom staff line
				// i.e. move down (7 + 4) staff linesteps
				yposst = ypostb + (7 + 4) * ystepst;
				xpos = 0;
				// draw empty staff at xPos, yPosst
				// LVIFIX: HACK HACK HACK !
				// drawStLns(pprw - 1);
				trp->yposst = yposst;
				trp->xpos = xpos;
				trp->drawStLns(pprw - 1);
				if (stTab) {
					// move ypostb to the top bar line
					// (where it would be if there as no staff)
					// i.e. move down 7 (undershoot) + 3 (linefeed)
					// staff linesteps
					ypostb = yposst + (7 + 3) * ystepst;
				}
			} else {
				yposst = ypostb;
			}
			if (stTab) {
				// move ypostb to the bottom bar line
				// i.e. move down (nstrings - 1) * tab bar linesteps
				ypostb = ypostb + (trk->string - 1) * ysteptb;
				xpos = 0;
				// draw empty tab bar at xPos, yPos
				// LVIFIX: HACK HACK HACK !
				// drawBarLns(pprw - 1, trk);
				trp->ypostb = ypostb;
				trp->xpos = xpos;
				trp->drawBarLns(pprw - 1, trk);
			}

			xpos += 1;				// first vertical line
			// LVIFIX: HACK HACK HACK !
			// drawKey(l, trk);
			trp->xpos = xpos;
			trp->drawKey(l, trk);

			xpos += tabfw;			// "TAB"
			// LVIFIX: HACK HACK HACK !
			// drawKeySig(trk);		// key signature (note: updates xpos)
			trp->xpos = xpos;
			trp->drawKeySig(trk);		// key signature (note: updates xpos)
			xpos = trp->xpos;

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
					// LVIFIX: HACK HACK HACK !
					// drawBar(bn, trk, 0);
					trp->xpos = xpos;
					trp->yposst = yposst;
					trp->ypostb = ypostb;
					trp->drawBar(bn, trk, 0);
					xpos = trp->xpos;
					bn++;
				}
			} else {
				// not the last line, add extra space
				// calculate extra space left to distribute divided by bars left,
				// to prevent accumulation of round-off errors
				int extSpLeft = pprw - xpos - totWidth - 1;
				for (uint i = 0; i < nBarsOnLine; i++) {
					int extSpInBar = extSpLeft / (nBarsOnLine - i);
					// LVIFIX: HACK HACK HACK !
					// drawBar(bn, trk, extSpInBar);
					trp->xpos = xpos;
					trp->yposst = yposst;
					trp->ypostb = ypostb;
					trp->drawBar(bn, trk, extSpInBar);
					xpos = trp->xpos;
					extSpLeft -= extSpInBar;
					bn++;
				}
			}

			brsPr += nBarsOnLine;

			// move to top of next line
			// tab, when printed, is below staff and determines the linefeed
			// required: 3 ysteptb (undershoot) + 2 ysteptb (linefeed)
			// if no tab, assume only staff:
			// required: 7 ystepst (undershoot) + 3 ystepst (linefeed)
			l++;
			if (stTab) {
				ypostb += (3 + 2) * ysteptb;
			} else {
				ypostb += (7 + 3) * ysteptb;
			}

			// determine height required for next line
			// LVIFIX: correct for track name when necessary
			int yreq = 0;
			if (stNts) {
				// add staff height
				yreq += (7 + 4 + 7) * ystepst;
			}
			if (stTab) {
				// add tab bar height
				yreq += (trk->string - 1 + 3) * ysteptb;
			}
			if (stNts && stTab) {
				// add staff to tab bar spacing
				yreq += 3 * ystepst;
			}

			// move to next page if necessary
			if (ypostb + yreq > pprh) {
				prntr->newPage();
				pgNr++;
				drawPageHdr(pgNr, song);
				// print the track header
				if ((song->t).count() > 1)
				{
					p->setFont(fHdr2);
					QFontMetrics fm  = p->fontMetrics();
					p->drawText(0, ypostb + fm.ascent(), trk->name);
					ypostb += hdrh4;
				}
			}

		} // end while (brsPr ...

		// move to the next track
		trkPr++;

	} // end while (trkPr ...

	p->end();			// send job to printer
}
