#include "trackview.h"
#include "trackviewcommands.h"
#include "tabsong.h"
#include "chord.h"
#include "rhythmer.h"
#include "keysig.h"
#include "timesig.h"
#include "songview.h"
#include "fretboard.h"
#include "settings.h"

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kxmlgui.h>
#include <kxmlguiclient.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qpen.h>
#include <qkeycode.h>
#include <qcursor.h>
#include <qstyle.h>

#include "trackprint.h"

#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <stdlib.h>		// required for declaration of abs()

// LVIFIX: note differences between "old" (in trackview.cpp) and "new" drawing code (in trackprint.cpp):
// - erase width around tab column numbers is "as tight as possible", while the cursor is a bit wider,
//   which leads to minor reverse-video artefacts under the cursor
// - starting bars (very thick and thick one) not implemented
// - harmonics use a diamond instead of text in the new code
// - palm muting uses a small cross instead of text in the new code

// Define if both "old" and "new" drawing code must be used at the same time
// Undefine to use only the "new" drawing code
#undef USE_BOTH_OLD_AND_NEW
// #define USE_BOTH_OLD_AND_NEW

#ifdef USE_BOTH_OLD_AND_NEW
#define VERTSPACE                      230 // between top of cell and first line
#define VERTLINE                        10 // between horizontal tabulature lines
#define HORDUR                          4
#define HORCELL                         14 // horizontal size of tab numbers column
#define TIMESIGSIZE                     14 // horizontal time sig size
#define ABBRLENGTH                      25 // drum abbreviations horizontal size

#define BOTTOMDUR   VERTSPACE+VERTLINE*(s+1)
#endif

#define NORMAL_FONT_FACTOR              0.8
#define TIME_SIG_FONT_FACTOR            1.4
#define SMALL_CAPTION_FONT_FACTOR       0.7

// definitions for the "new" drawing code layout
#define TOPSPTB                         3   // top space tabbar in ysteptb units
#define BOTSPTB                         3   // bottom space tabbar in ysteptb units
#define ADDSPST                         1.5 // additional top space staff in ystepst units
#define TOPSPST                         7.5 // top space staff in ystepst units
#define BOTSPST                         1.5 // bottom space staff in ystepst units
#define NLINEST                         5   // number of staff lines

TrackView::TrackView(TabSong *s, KXMLGUIClient *_XMLGUIClient, KCommandHistory *_cmdHist,
#ifdef WITH_TSE3
                     TSE3::MidiScheduler *_scheduler,
#endif
                     QWidget *parent, const char *name): QGridView(parent, name)
{
	setFrameStyle(Panel | Sunken);
	setBackgroundMode(PaletteBase);

	setNumCols(1);

	setFocusPolicy(QWidget::StrongFocus);

	xmlGUIClient = _XMLGUIClient;
	cmdHist = _cmdHist;

	song = s;
	setCurrentTrack(s->t.first());

	normalFont = new QFont(KGlobalSettings::generalFont());
	if (normalFont->pointSize() == -1) {
		normalFont->setPixelSize((int) ((double) normalFont->pixelSize() * NORMAL_FONT_FACTOR));
	} else {
		normalFont->setPointSizeFloat(normalFont->pointSizeFloat() * NORMAL_FONT_FACTOR);
	}

 	smallCaptionFont = new QFont(*normalFont);
	if (smallCaptionFont->pointSize() == -1) {
		smallCaptionFont->setPixelSize((int) ((double) smallCaptionFont->pixelSize() * SMALL_CAPTION_FONT_FACTOR));
	} else {
		smallCaptionFont->setPointSizeFloat(smallCaptionFont->pointSizeFloat() * SMALL_CAPTION_FONT_FACTOR);
	}

  	timeSigFont = new QFont(*normalFont);
	if (timeSigFont->pointSize() == -1) {
		timeSigFont->setPixelSize((int) ((double) timeSigFont->pixelSize() * TIME_SIG_FONT_FACTOR));
	} else {
		timeSigFont->setPointSizeFloat(timeSigFont->pointSizeFloat() * TIME_SIG_FONT_FACTOR);
	}
  	timeSigFont->setBold(TRUE);

	fetaFont   = 0;
	fetaNrFont = 0;
	
	lastnumber = -1;
	zoomLevel = 10;

#ifdef WITH_TSE3
	scheduler = _scheduler;
#endif

	playbackCursor = FALSE;

	trp = new TrackPrint;
	trp->setOnScreen();
	const int lw = 1;
	trp->pLnBl = QPen(Qt::black, lw);
	trp->pLnWh = QPen(Qt::white, lw);

	updateRows();		// depends on trp's font metrics
}

TrackView::~TrackView()
{
	delete normalFont;
	delete smallCaptionFont;
	delete timeSigFont;
	delete trp;
}

void TrackView::initFonts(QFont *f4, QFont *f5)
{
	fetaFont   = f4;
	fetaNrFont = f5;
	trp->initFonts(normalFont, smallCaptionFont, timeSigFont, fetaFont, fetaNrFont);
}

void TrackView::selectTrack(TabTrack *trk)
{
	setCurrentTrack(trk);
	updateRows();
	repaintContents();
}

void TrackView::selectBar(uint n)
{
	if (n != (uint) curt->xb && n < curt->b.size()) {
		curt->x = curt->b[n].start;
		curt->xb = n;
		ensureCurrentVisible();
		emit barChanged();
		emit columnChanged();
	}
	lastnumber = -1;
}

void TrackView::setCurrentTrack(TabTrack *trk)
{
	curt = trk;
	emit trackChanged(trk);
}

// Set new horizontal zoom level and update display accordingly
void TrackView::setZoomLevel(int newZoomLevel)
{
	if (newZoomLevel > 0) {
		zoomLevel = newZoomLevel;
		updateRows();
		repaintContents();
	}
}

void TrackView::zoomIn()
{
	setZoomLevel(zoomLevel - 1);
}

void TrackView::zoomOut()
{
	setZoomLevel(zoomLevel + 1);
}

// Set zoom level dialog
void TrackView::zoomLevelDialog()
{
	// GREYFIX
}

void TrackView::updateRows()
{
	int ch = (int) ((TOPSPTB + curt->string + BOTSPTB) * trp->ysteptb);
#ifdef USE_BOTH_OLD_AND_NEW
	// note: cannot make row height dependent on viewscore without making too many
	// changes to the "old" drawing code: use fixed height
	ch += 3 * VERTLINE * 2 + VERTLINE * (curt->string - 1);
	ch += (int) ((TOPSPST + NLINEST - 1 + BOTSPST) * trp->ystepst);
	ch += (int) (ADDSPST * trp->ystepst);
#else
	if (viewscore && fetaFont) {
		ch += (int) ((TOPSPST + NLINEST - 1 + BOTSPST) * trp->ystepst);
		ch += (int) (ADDSPST * trp->ystepst);
	}
#endif
	setNumRows(curt->b.size());
	setMinimumHeight(ch);
	setCellHeight(ch);
//	cout << "TrackView::updateRows()"
//	<< " ch=" << ch
//	<< " nr=" << curt->b.size()
//	<< endl;
}

void TrackView::repaintCellNumber(int n)
{
	repaintCell(n, 0);
}

void TrackView::repaintCurrentCell()
{
	repaintCellNumber(curt->xb);
	emit paneChanged();
}

void TrackView::repaintCurrentColumn()
{
	//VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2

	//	int ycoord = 0;
//	if (rowYPos(curt->xb, &ycoord)) // GREYFIX - what was it all about?

	// GREYFIX: some crazy things going here about what coordinate
	// system to use. I'm totally screwed up trying to figure it out,
	// until I do, just update whole cell.

// 	repaint(selxcoord, cellHeight() * curt->xb - contentsY(), HORCELL + 1, cellHeight());

	repaintCell(curt->xb, 0);
// 	emit paneChanged();
}

// Checks is current bar is fully visible, and, if it's not, tries to
// do minimal scrolling to ensure the full visibility
void TrackView::ensureCurrentVisible()
{
/*	int ch = cellHeight();

	if ((curt->xb + 1) * ch > yOffset() + height())
		setYOffset((curt->xb + 1) * ch - height());
	else if (curt->xb * ch < yOffset())
		setYOffset(curt->xb * ch);
*/ //GREYFIX
	ensureCellVisible(curt->xb, 0);
}

// Process a mouse press of fret "fret" in current column on string
// "num". Depending on given "button" mouse state flags, additional
// things may happen.
void TrackView::melodyEditorPress(int num, int fret, ButtonState button = NoButton)
{
	if (button & LeftButton)
		melodyEditorAction(num, fret, 0);
	if (button & MidButton)
		melodyEditorAction(num, fret, 1);
	if (button & RightButton)
		melodyEditorAction(num, fret, 2);
}

// Execute one of melody editors actions, as defined in
// globalMelodyEditorAction array
void TrackView::melodyEditorAction(int num, int fret, int action)
{
	// GREYFIX: make it *one* undo transaction
	switch (Settings::melodyEditorAction(action)) {
	case 0: // no action
		break;
	case 1: // set note
		setFinger(num, fret);
		break;
	case 3: // set 022 power chord
		setFinger(num + 2, fret + 2);
	case 2: // set 02 power chord
		setFinger(num + 1, fret + 2);
		setFinger(num, fret);
		break;
	case 5: // set 0022 power chord
		setFinger(num + 3, fret + 2);
		setFinger(num + 2, fret + 2);
	case 4: // set 00 power chord
		setFinger(num + 1, fret);
		setFinger(num, fret);
		break;
	case 6: // delete note
		setFinger(num, NULL_NOTE);
		break;
	}
}

// Process a mouse release in melody editor. Depending on given
// "button" mouse state flags, additional things, such as proceeding
// to next column, may happen.
void TrackView::melodyEditorRelease(ButtonState button)
{
	if (((button & LeftButton)  && (Settings::melodyEditorAdvance(0))) ||
		((button & MidButton)   && (Settings::melodyEditorAdvance(1))) ||
		((button & RightButton) && (Settings::melodyEditorAdvance(2))))  {
		if (curt->sel) {
			curt->sel = FALSE;
			repaintContents();
		}
		moveRight();
	}
}

// Add tab number insertion command on current column, string "num",
// setting fret number "fret". Perform various checks, including
// no repeats for same insertion.
void TrackView::setFinger(int num, int fret)
{
	if (num < 0 || num >= curt->string)
		return;
	if (fret > curt->frets)
		return;
	if (curt->c[curt->x].a[num] == fret)
		return;

	curt->y = num;
	cmdHist->addCommand(new InsertTabCommand(this, curt, fret));
	repaintCurrentColumn();
	emit columnChanged();
}

int TrackView::finger(int num)
{
	return curt->c[curt->x].a[num];
}

void TrackView::setLength(int l)
{
	//only if needed
	if (curt->c[curt->x].l != l)
		cmdHist->addCommand(new SetLengthCommand(this, curt, l));
}

void TrackView::linkPrev()
{
	cmdHist->addCommand(new SetFlagCommand(this, curt, FLAG_ARC));
	lastnumber = -1;
}

void TrackView::addHarmonic()
{
	if (curt->c[curt->x].a[curt->y] >= 0)
		cmdHist->addCommand(new AddFXCommand(this, curt, EFFECT_HARMONIC));
	lastnumber = -1;
}

void TrackView::addArtHarm()
{
	if (curt->c[curt->x].a[curt->y] >= 0)
		cmdHist->addCommand(new AddFXCommand(this, curt, EFFECT_ARTHARM));
	lastnumber = -1;
}

void TrackView::addLegato()
{
	if (curt->c[curt->x].a[curt->y] >= 0)
		cmdHist->addCommand(new AddFXCommand(this, curt, EFFECT_LEGATO));
	lastnumber = -1;
}

void TrackView::addSlide()
{
	if (curt->c[curt->x].a[curt->y] >= 0)
		cmdHist->addCommand(new AddFXCommand(this, curt, EFFECT_SLIDE));
	lastnumber = -1;
}

void TrackView::addLetRing()
{
	if (curt->c[curt->x].a[curt->y] >= 0)
		cmdHist->addCommand(new AddFXCommand(this, curt, EFFECT_LETRING));
	else
		cmdHist->addCommand(new AddFXCommand(this, curt, EFFECT_STOPRING));
	lastnumber = -1;
}

// Call the chord constructor dialog and may be parse something from it
void TrackView::insertChord()
{
	int a[MAX_STRINGS];

	ChordSelector cs(
#ifdef WITH_TSE3
	                 scheduler,
#endif
	                 curt);

	for (int i = 0; i < curt->string; i++)
		cs.setApp(i, curt->c[curt->x].a[i]);

	// required to detect chord from tabulature
	cs.detectChord();

	int i;

	// set fingering right if frets > 5
	for (i = 0; i < curt->string; i++)
		a[i] = cs.app(i);
	cs.fng->setFingering(a);

	if (cs.exec()) {
		for (i = 0; i < curt->string; i++)
			a[i] = cs.app(i);
		cmdHist->addCommand(new InsertStrumCommand(this, curt, cs.scheme(), a));
	}

	lastnumber = -1;
}

// Call rhythm construction dialog and may be parse something from it
void TrackView::rhythmer()
{
#ifdef WITH_TSE3
	Rhythmer r(scheduler);
#else
	Rhythmer r;
#endif

	if (r.exec())
		cmdHist->addCommand(new InsertRhythm(this, curt, r.quantized));

	lastnumber = -1;
}

// Determine horizontal offset between two columns - n and n+1

int TrackView::horizDelta(uint n)
{
#ifdef USE_BOTH_OLD_AND_NEW
	int res = curt->c[n].fullDuration() * HORCELL / zoomLevel;
// 	if (res < HORCELL)
// 		res = HORCELL;
#else
	int res = trp->colWidth(n, curt);	// LVIFIX: zoomLevel
#endif
	return res;
}

#ifdef USE_BOTH_OLD_AND_NEW
void TrackView::drawLetRing(QPainter *p, int x, int y)
{
	p->setPen(SolidLine);
	p->drawLine(x, y, x - HORCELL / 3, y - VERTLINE / 3);
	p->drawLine(x, y, x - HORCELL / 3, y + VERTLINE / 3);
	p->setPen(NoPen);
}
#endif

void TrackView::paintCell(QPainter *p, int row, int /*col*/)
{
//	cout << "TrackView::paintCell(row=" << row << ")" << endl;

	uint bn = row;                      // Drawing only this bar

	int selx2coord = -1;
	selxcoord = -1;
	
	if (row >= int(curt->b.size())) {
		kdDebug() << "Drawing the bar out of limits!" << endl;
		return;
	}

	trp->setPainter(p);
	// LVIFIX: initmetrics may be expensive but depends on p, init only once ?
	trp->initMetrics();
	// LVIFIX: do following calculations for the current bar only
	curt->calcVoices();
	curt->calcStepAltOct();
	curt->calcBeams();
	trp->yposst = 0;
	trp->xpos = -1;
	if (viewscore && fetaFont) {
		trp->initPrStyle(2);
		trp->yposst = (int) ((TOPSPST + NLINEST - 1 + BOTSPST) * trp->ystepst);
		trp->drawStLns(width());
	} else {
		trp->initPrStyle(0);
	}
 	trp->ypostb = trp->yposst
 	              + (int) ((TOPSPTB + curt->string - 0.5) * trp->ysteptb);
#ifdef USE_BOTH_OLD_AND_NEW
	// force new tabbar position close to old one
	trp->ypostb = (int) ((TOPSPST + NLINEST - 1) * trp->ystepst)
	              + (int) (BOTSPST * trp->ystepst)
	              + (int) ((TOPSPTB + curt->string) * trp->ysteptb);
#endif
	trp->drawBarLns(width(), curt);
//	trp->drawKey(row, curt);	// LVIFIX: make (some more) room between key and time sig
	bool doDraw = true;
	bool fbol = true;
	bool flop = (row == 0);
	(void) trp->drawKKsigTsig(row, curt, doDraw, fbol, flop);
	trp->drawBar(row, curt, 0, selxcoord, selx2coord);

	// connect tabbar and staff with vertical line at end of bar
	if (viewscore && fetaFont) {
		p->setPen(trp->pLnBl);
		p->drawLine(trp->xpos - 1, trp->yposst, trp->xpos - 1, trp->ypostb);
	}

	// DEBUG: DRAW VARIOUS GUIDE BORDERS

// 	p->setBrush(NoBrush);
// 	p->setPen(red);
// 	p->drawRect(cellRect());
// 	p->setPen(blue);
// 	p->drawRect(0, TOPSPTB * trp->ysteptb, cellWidth(), curt->string * trp->ysteptb);

	// DRAW SELECTION

	p->setRasterOp(Qt::XorROP);
	p->setBrush(KGlobalSettings::baseColor());

	const int horcell = (int) (2.6 * trp->br8w);
	const int vertline = trp->ysteptb;
	const int vertspace = trp->ypostb;

	if (playbackCursor) {
		// Draw MIDI playback cursor
		if (selxcoord != -1)
			p->drawRect(selxcoord - horcell / 2, 0, horcell + 1, cellHeight());

	} else {

		// Draw selection between selxcoord and selx2coord (if it exists)
		if (curt->sel) {
			if ((selxcoord != -1) && (selx2coord != -1)) {
				int x1 = KMIN(selxcoord, selx2coord);
				int wid = abs(selx2coord - selxcoord) + horcell + 1;
				p->drawRect(x1 - horcell / 2, 0, wid, cellHeight());
			} else if ((selxcoord == -1) && (selx2coord != -1)) {
				if (curt->x > curt->lastColumn(bn))
					p->drawRect(selx2coord - horcell / 2, 0, cellWidth(), cellHeight());
				else
					p->drawRect(0, 0, selx2coord + horcell / 2 + 1, cellHeight());
			} else if ((selxcoord != -1) && (selx2coord == -1)) {
				if (curt->xsel > curt->lastColumn(bn))
					p->drawRect(selxcoord - horcell / 2, 0, cellWidth(), cellHeight());
				else
					p->drawRect(0, 0, selxcoord + horcell / 2 + 1, cellHeight());
			} else { // both are -1
				int x1 = KMIN(curt->x, curt->xsel);
				int x2 = KMAX(curt->x, curt->xsel);
				if ((x1 < curt->b[bn].start) && (x2 > curt->lastColumn(bn)))
					p->drawRect(0, 0, cellWidth(), cellHeight());
			}
		}
		// Draw original cursor (still inverted)
		if (selxcoord != -1) {
			p->drawRect(selxcoord - horcell / 2,
				    vertspace + (0 - curt->y) * vertline - vertline / 2 - 2,
				    horcell,
				    vertline + 3);
		}
	}

	p->setRasterOp(Qt::CopyROP);

#ifdef USE_BOTH_OLD_AND_NEW
	QString tmp;
	bool ringing[MAX_STRINGS];
	int trpCnt = 0;                     // triplet count
	int lastPalmMute = 0;

	int s = curt->string - 1;

	for (int i = 0; i <= s; i++) {
		p->drawLine(0, VERTSPACE + (s - i) * VERTLINE,
		            width(), VERTSPACE + (s - i) * VERTLINE);
		ringing[i] = FALSE;
	}

	int xpos = 40, lastxpos = 20, xdelta;

	selxcoord = -1;

	// Starting bars - very thick and thick one

	if (bn == 0) {
		p->setBrush(SolidPattern);
		p->drawRect(0, VERTSPACE, 5, VERTLINE * s);
		p->drawRect(8, VERTSPACE, 2, VERTLINE * s);
	}

	// Time signature

	if (curt->showBarSig(bn)) {
 		p->setFont(*timeSigFont);
		tmp.setNum(curt->b[bn].time1);
		p->drawText(20, VERTSPACE + VERTLINE * s / 4 - TIMESIGSIZE / 2,
					TIMESIGSIZE, TIMESIGSIZE, AlignCenter, tmp);
		tmp.setNum(curt->b[bn].time2);
		p->drawText(20, VERTSPACE + VERTLINE * s * 3 / 4 - TIMESIGSIZE / 2,
					TIMESIGSIZE, TIMESIGSIZE, AlignCenter, tmp);
	}

	p->setFont(*normalFont);
	p->setBrush(KGlobalSettings::baseColor());

	// Drum abbreviations markings

	if (curt->trackMode() == TabTrack::DrumTab) {
		p->setPen(NoPen);
		for (int i = 0; i <= s; i++) {
			p->drawRect(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
						ABBRLENGTH, VERTLINE + 1);
			p->drawText(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
						ABBRLENGTH, VERTLINE, AlignCenter, drum_abbr[curt->tune[i]]);
		}
		xpos += ABBRLENGTH + 10; lastxpos += ABBRLENGTH + 10;
		p->setPen(SolidLine);
	}

	for (int t = curt->b[bn].start; t <= curt->lastColumn(bn); t++) {

		// triplet handling:
		// - reset after third note of triplet
		// - count notes while inside triplet
		if (trpCnt >= 3) {
			trpCnt = 0;
		}
		if (curt->c[t].flags & FLAG_TRIPLET) {
			trpCnt++;
		} else {
			trpCnt = 0;
		}

		// Drawing duration marks

		// Draw connection with previous, if applicable
		if ((t > 0) && (t > curt->b[bn].start) && (curt->c[t - 1].l == curt->c[t].l))
			xdelta = lastxpos + HORCELL / 2;
		else
			xdelta = xpos + HORCELL / 2 + HORDUR;

		switch (curt->c[t].l) {
		case 15:  // 1/32
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + VERTLINE - 4,
						xdelta, BOTTOMDUR + VERTLINE - 4);
		case 30:  // 1/16
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + VERTLINE - 2,
						xdelta, BOTTOMDUR + VERTLINE - 2);
		case 60:  // 1/8
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + VERTLINE,
						xdelta, BOTTOMDUR + VERTLINE);
		case 120: { // 1/4 - a long vertical line, so we need to find the highest note
			int i;
			for (i = s; ((i >= 0) && (curt->c[t].a[i] == -1)); i--);

			// If it's an empty measure at all - draw the vertical line from bottom
			if (i < 0)  i = s / 2;

			p->drawLine(xpos + HORCELL / 2, VERTSPACE + VERTLINE * (s - i) + VERTLINE / 2,
						xpos + HORCELL / 2, BOTTOMDUR + VERTLINE);
		}
		case 240: // 1/2
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + 3,
						xpos + HORCELL / 2, BOTTOMDUR + VERTLINE);
		case 480:; // whole
		}

		// Draw dot

		if (curt->c[t].flags & FLAG_DOT)
			p->drawRect(xpos + HORCELL / 2 + 3, BOTTOMDUR + 5, 2, 2);

		// Draw triplet - GREYFIX: ugly code, needs to be fixed
		// somehow... Ideally, triplets should be drawn in a second
		// loop, after everything else would be done.

		/*
		if (curt->c[t].flags & FLAG_TRIPLET) {
 			if ((curt->c.size() >= t + 1) && (t) &&
 				(curt->c[t - 1].flags & FLAG_TRIPLET) &&
 				(curt->c[t + 1].flags & FLAG_TRIPLET) &&
				(curt->c[t - 1].l == curt->c[t].l) &&
				(curt->c[t + 1].l == curt->c[t].l)) {
				p->drawLine(lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5,
							xpos * 2 - lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5);
				p->drawLine(lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 2,
							lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5);
				p->drawLine(xpos * 2 - lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 2,
							xpos * 2 - lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5);
				p->setFont(*smallCaptionFont);
				p->drawText(xpos, BOTTOMDUR + VERTLINE + 7, HORCELL, VERTLINE, AlignHCenter | AlignTop, "3");
				p->setFont(*normalFont);
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
					p->setFont(*smallCaptionFont);
					p->drawText(xpos, BOTTOMDUR + VERTLINE + 7, HORCELL, VERTLINE, AlignHCenter | AlignTop, "3");
					p->setFont(*normalFont);
				}
			}
		}
		*/

		// Length of interval to next column - adjusted if dotted
		// calculated here because it is required by triplet code

		xdelta = horizDelta(t);

		// Draw triplet - improved (? :-)) code
		if ((trpCnt == 1) || (trpCnt == 2)) {
			// draw horizontal line to next note
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5,
						xpos + HORCELL / 2 + xdelta, BOTTOMDUR + VERTLINE + 5);
		}
		if ((trpCnt == 1) || (trpCnt == 3)) {
			// draw vertical line
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 2,
						xpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5);
		}
		if (trpCnt == 2) {
			// draw "3"
			p->setFont(*smallCaptionFont);
			p->drawText(xpos, BOTTOMDUR + VERTLINE + 7, HORCELL, VERTLINE, AlignHCenter | AlignTop, "3");
			p->setFont(*normalFont);
		}

		// Draw arcs to backward note

		if (curt->c[t].flags & FLAG_ARC)
			p->drawArc(lastxpos + HORCELL / 2, BOTTOMDUR + 9,
					   xpos - lastxpos, 10, 0, -180 * 16);

		// Draw palm muting

		if (curt->c[t].flags & FLAG_PM) {
			if (lastPalmMute == 0)  {     // start drawing with "P.M."
				p->setFont(*smallCaptionFont);
				p->drawText(xpos, VERTSPACE / 2, VERTLINE * 2, VERTLINE,
							AlignCenter, "P.M.");
				p->setFont(*normalFont);
				lastPalmMute = 1;
			} else if (lastPalmMute == 1) {
				p->drawLine(lastxpos + VERTLINE * 2, VERTSPACE / 2 + VERTLINE / 2,
							xpos + HORCELL / 2, VERTSPACE / 2 + VERTLINE / 2);
				lastPalmMute = 2;
			} else {
				p->drawLine(lastxpos + HORCELL / 2, VERTSPACE / 2 + VERTLINE / 2,
							xpos + HORCELL / 2, VERTSPACE / 2 + VERTLINE / 2);
			}
		} else {
			if (lastPalmMute == 2) {
				p->drawLine(lastxpos + HORCELL / 2, VERTSPACE / 2 + VERTLINE / 2,
				            lastxpos + HORCELL / 2, VERTSPACE / 2 + VERTLINE);
			}
			lastPalmMute = 0;
		}

		// Draw the number column

		p->setPen(NoPen);
		for (int i = 0; i <= s; i++) {
			if (curt->c[t].a[i] != -1) {
				if (curt->c[t].a[i] == DEAD_NOTE)
					tmp = "X";
				else
					tmp.setNum(curt->c[t].a[i]);
				p->drawRect(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
							HORCELL, VERTLINE);
				p->drawText(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
							HORCELL, VERTLINE, AlignCenter, tmp);
				if (ringing[i]) {
					drawLetRing(p, xpos, VERTSPACE + (s - i) * VERTLINE);
					ringing[i] = FALSE;
				}
			}
			if ((curt->c[t].a[i] == -1)
			     && (curt->c[t].e[i] == EFFECT_STOPRING)) {
				if (ringing[i]) {
					drawLetRing(p, xpos, VERTSPACE + (s - i) * VERTLINE);
					ringing[i] = FALSE;
				}
			}

			if (t == curt->x)
				selxcoord = xpos;

			if (t == curt->xsel)
				selx2coord = xpos;

			// Draw effects
			// GREYFIX - use lastxpos, not xdelta

			switch (curt->c[t].e[i]) {
			case EFFECT_HARMONIC:
 				p->setFont(*smallCaptionFont);
				p->drawText(xpos + VERTLINE + 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE * 2 / 3,
							HORCELL, VERTLINE, AlignCenter, "H");
 				p->setFont(*normalFont);
				break;
			case EFFECT_ARTHARM:
 				p->setFont(*smallCaptionFont);
				p->drawText(xpos + VERTLINE + 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE * 2 / 3,
							HORCELL * 2, VERTLINE, AlignCenter, "AH");
 				p->setFont(*normalFont);
				break;
			case EFFECT_LEGATO:
 				p->setPen(SolidLine);
				p->drawArc(xpos + HORCELL, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
						   xdelta - HORCELL, 10, 0, 180 * 16);
				if ((t < curt->c.size() - 1) && (curt->c[t + 1].a[i] >= 0)) {
 					p->setFont(*smallCaptionFont);
					if (curt->c[t + 1].a[i] > curt->c[t].a[i]) {
						p->drawText(xpos + xdelta / 2 - HORCELL / 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 3,
									HORCELL * 2, VERTLINE, AlignCenter, "HO");
					} else if (curt->c[t + 1].a[i] < curt->c[t].a[i]) {
						p->drawText(xpos + xdelta / 2 - HORCELL / 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 3,
									HORCELL * 2, VERTLINE, AlignCenter, "PO");
					}
 					p->setFont(*normalFont);
				}
				p->setPen(NoPen);
				break;
			case EFFECT_SLIDE:
				p->setPen(SolidLine);
				if ((t < curt->c.size() - 1) && (curt->c[t + 1].a[i] >= 0)) {
					if (curt->c[t + 1].a[i] > curt->c[t].a[i]) {
						p->drawLine(xpos + HORCELL + 2, VERTSPACE + (s - i) * VERTLINE + VERTLINE / 2 - 1,
									xpos + xdelta, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2 + 1);
					} else {
						p->drawLine(xpos + HORCELL + 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2 + 1,
									xpos + xdelta, VERTSPACE + (s - i) * VERTLINE + VERTLINE / 2 - 1);
					}
				}
				p->setPen(NoPen);
				break;
			case EFFECT_LETRING:
				ringing[i] = TRUE;
				break;
			}
		}

		p->setPen(SolidLine);

		lastxpos = xpos;
		xpos += xdelta;
	}

	// Show notes still ringing at end of bar
	for (int i = 0; i <= s; i++) {
		if (ringing[i]) {
			drawLetRing(p, xpos - HORCELL / 3, VERTSPACE + (s - i) * VERTLINE);
			ringing[i] = FALSE;
		}
	}

	// End bar with vertical line
	p->setPen(SolidLine);
	p->drawRect(xpos, VERTSPACE, 1, VERTLINE * s);

	// Draw original cursor (still inverted)
	p->setRasterOp(Qt::XorROP);
// 	p->setBrush(KGlobalSettings::highlightColor());
	if (selxcoord != -1) {
		p->drawRect(selxcoord, VERTSPACE + (s - curt->y) * VERTLINE - VERTLINE / 2 - 1,
					HORCELL + 1, VERTLINE + 2);
	}

// 	p->setBrush(KGlobalSettings::baseColor());
	p->setRasterOp(Qt::CopyROP);
	p->setBrush(SolidPattern);
#endif // USE_BOTH_OLD_AND_NEW
}

void TrackView::resizeEvent(QResizeEvent *e)
{
	QGridView::resizeEvent(e); // GREYFIX ? Is it C++-correct?
	setCellWidth(width() - 2 - QStyle::PM_ScrollBarExtent);
}

bool TrackView::moveFinger(int from, int dir)
{
	int n0 = curt->c[curt->x].a[from];
	int n = n0;
	if (n < 0)
		return FALSE;

	int to = from;

	do {
		to += dir;
		if ((to < 0) || (to >= curt->string))
			return FALSE;
		n = n0 + curt->tune[from] - curt->tune[to];
		if ((n < 0) || (n > curt->frets))
			return FALSE;
	} while (curt->c[curt->x].a[to] != -1);

	cmdHist->addCommand(new MoveFingerCommand(this, curt, from, to, n));
	emit columnChanged();

	return TRUE;
}

// LVIFIX: eventually KGuitar should support changing the key at the start
// of a new bar. For the time being, we don't: the key is the same for the
// whole track and is stored in the first bar

void TrackView::keySig()
{
	int oldsig = curt->b[0].keysig;
	if ((oldsig <= -8) || (8 <= oldsig)) {
		// LVIFIX: report error ???
		oldsig = 0;
	}

	SetKeySig sks(oldsig);

	if (sks.exec()) {
		curt->b[0].keysig = sks.keySignature();
		// LVIFIX: undo info
	}

	updateRows();
	lastnumber = -1;
}

void TrackView::timeSig()
{
	SetTimeSig sts(curt->b[curt->xb].time1, curt->b[curt->xb].time2);

	if (sts.exec())
		cmdHist->addCommand(new SetTimeSigCommand(this, curt, sts.toend->isChecked(),
		                                          sts.time1(), sts.time2()));

	lastnumber = -1;
}

void TrackView::keyLeft()
{
	if (curt->sel) {
		curt->sel = FALSE;
		repaintContents();
	} else {
		moveLeft();
	}
}

void TrackView::keyRight()
{
	if (curt->sel) {
		curt->sel = FALSE;
		repaintContents();
	} else {
		moveRight();
	}
}

void TrackView::keyLeftBar()
{
	if (curt->sel) {
		curt->sel = FALSE;
		repaintContents();
	} else {
		moveLeftBar();
	}
}

void TrackView::keyRightBar()
{
	if (curt->sel) {
		curt->sel = FALSE;
		repaintContents();
	} else {
		moveRightBar();
	}
}

void TrackView::keyHome()
{
	if (curt->sel) {
		curt->sel = FALSE;
		repaintContents();
	} else {
		moveHome();
	}
}

void TrackView::keyEnd()
{
	if (curt->sel) {
		curt->sel = FALSE;
		repaintContents();
	} else {
		moveEnd();
	}
}

void TrackView::keyCtrlHome()
{
	if (curt->sel) {
		curt->sel = FALSE;
		repaintContents();
	} else {
		moveCtrlHome();
	}
}

void TrackView::keyCtrlEnd()
{
	if (curt->sel) {
		curt->sel = FALSE;
		repaintContents();
	} else {
		moveCtrlEnd();
	}
}

void TrackView::moveLeft()
{
	if (curt->x > 0) {
		if (curt->b[curt->xb].start == curt->x) {
			curt->x--;
			repaintCurrentCell();
			curt->xb--;
			ensureCurrentVisible();
			emit barChanged();
		} else {
			curt->x--;
		}
		repaintCurrentCell();
		emit columnChanged();
	}
	lastnumber = -1;
}

void TrackView::moveRight()
{
	if (((uint) (curt->x + 1)) == curt->c.size()) {
		cmdHist->addCommand(new AddColumnCommand(this, curt));
		emit columnChanged();
	} else {
		if (curt->b.size() == (uint) curt->xb + 1)
			curt->x++;
		else {
			if (curt->b[curt->xb + 1].start == curt->x + 1) {
				curt->x++;
				repaintCurrentCell();
				curt->xb++;
				ensureCurrentVisible();
				emit barChanged();
			} else {
				curt->x++;
			}
		}
		repaintCurrentCell();
		emit columnChanged();
	}
	lastnumber = -1;
}

void TrackView::moveLeftBar()
{
	if (curt->x > curt->b[curt->xb].start) {
		moveHome(); 
	} else {
		moveLeft();
		moveHome();
	}
}

void TrackView::moveRightBar()
{
	if (curt->x == curt->lastColumn(curt->xb)) {
		moveRight();
	} else if (curt->x == curt->b[curt->xb].start) {
		moveEnd();
		moveRight();
	} else {
		moveEnd();
	}
}

void TrackView::moveHome()
{
	curt->x = curt->b[curt->xb].start;
	repaintCurrentCell();
	emit columnChanged();
}

void TrackView::moveEnd()
{
	curt->x = curt->lastColumn(curt->xb);
	repaintCurrentCell();
	emit columnChanged();
}

void TrackView::moveCtrlHome()
{
	curt->x = 0;
	curt->xb = 0;
	ensureCurrentVisible();
	repaintContents();
	emit barChanged();
	emit columnChanged();
}

void TrackView::moveCtrlEnd()
{
	curt->x = curt->c.size() - 1;
	curt->xb = curt->b.size() - 1;
	ensureCurrentVisible();
	repaintContents();
	emit barChanged();
	emit columnChanged();
}

void TrackView::selectLeft()
{
	if (!curt->sel) {
		curt->sel = TRUE;
		curt->xsel = curt->x;
		repaintCurrentCell();
	} else {
		moveLeft();
	}
}

void TrackView::selectRight()
{
	if (!curt->sel) {
		curt->sel = TRUE;
		curt->xsel = curt->x;
		repaintCurrentCell();
	} else {
		moveRight();
	}
}

void TrackView::moveUp()
{
	if (curt->y+1 < curt->string) {
		curt->y++;
		if (curt->sel)
			repaintCurrentCell();
		else
			repaintCurrentColumn();
	}
	lastnumber = -1;
}

void TrackView::transposeUp()
{
	if (curt->y+1 < curt->string)
		moveFinger(curt->y, 1);
	lastnumber = -1;
}

void TrackView::moveDown()
{
	if (curt->y > 0) {
		curt->y--;
		if (curt->sel)
			repaintCurrentCell();
		else
			repaintCurrentColumn();
	}
	lastnumber = -1;
}

void TrackView::transposeDown()
{
	if (curt->y > 0)
		moveFinger(curt->y, -1);
	lastnumber = -1;
}

void TrackView::deadNote()
{
	cmdHist->addCommand(new SetFlagCommand(this, curt, DEAD_NOTE));
	emit columnChanged();
	lastnumber = -1;
}

void TrackView::deleteNote()
{
	if (curt->c[curt->x].a[curt->y] != -1) {
		cmdHist->addCommand(new DeleteNoteCommand(this, curt));
		emit columnChanged();
	}
	lastnumber = -1;
}

void TrackView::deleteColumn()
{
	cmdHist->addCommand(new DeleteColumnCommand(this, curt));
	emit columnChanged();
	lastnumber = -1;
}

void TrackView::deleteColumn(QString name)
{
	cmdHist->addCommand(new DeleteColumnCommand(name, this, curt));
	emit columnChanged();
}

void TrackView::insertColumn()
{
	cmdHist->addCommand(new InsertColumnCommand(this, curt));
	emit columnChanged();
	lastnumber = -1;
}

void TrackView::palmMute()
{
	cmdHist->addCommand(new SetFlagCommand(this, curt, FLAG_PM));
	lastnumber = -1;
}

void TrackView::dotNote()
{
	cmdHist->addCommand(new SetFlagCommand(this, curt, FLAG_DOT));
	lastnumber = -1;
}

void TrackView::tripletNote()
{
	cmdHist->addCommand(new SetFlagCommand(this, curt, FLAG_TRIPLET));
	lastnumber = -1;
}

void TrackView::keyPlus()
{
	if (curt->c[curt->x].l < 480)
		setLength(curt->c[curt->x].l * 2);
	lastnumber = -1;
}

void TrackView::keyMinus()
{
	if (curt->c[curt->x].l > 15)
		setLength(curt->c[curt->x].l / 2);
	lastnumber = -1;
}

void TrackView::arrangeTracks()
{
	cmdHist->clear();       // because columns will be changed
	curt->arrangeBars();
	emit barChanged();
	updateRows();
	repaintContents();

	emit paneChanged();
	emit columnChanged();
}

void TrackView::insertTab(int num)
{
	int totab = num;

	if (curt->c[curt->x].flags & FLAG_ARC)
		curt->c[curt->x].flags -= FLAG_ARC;

    // Allow making two-digit fret numbers pressing two keys sequentally
	if ((lastnumber != -1) && (lastnumber * 10 + num <= curt->frets)) {
		totab = lastnumber * 10 + num;
		lastnumber = -1;
	} else {
		lastnumber = num;
	}

	if ((totab <= curt->frets) && (curt->c[curt->x].a[curt->y] != totab))
		cmdHist->addCommand(new InsertTabCommand(this, curt, totab));
	emit columnChanged();
}

void TrackView::arrangeBars()
{
	song->arrangeBars();
	emit barChanged();
	emit columnChanged();
	updateRows();
}

void TrackView::mousePressEvent(QMouseEvent *e)
{
	lastnumber = -1;

	// RightButton pressed
	if (e->button() == RightButton) {
		QWidget *tmpWidget = 0;
		tmpWidget = xmlGUIClient->factory()->container("trackviewpopup", xmlGUIClient);

		if (!tmpWidget || !tmpWidget->inherits("KPopupMenu")) {
			kdDebug() << "TrackView::mousePressEvent => wrong container widget" << endl;
			return;
		}

		KPopupMenu *menu(static_cast<KPopupMenu*>(tmpWidget));
		menu->popup(QCursor::pos());
	}

	// LeftButton pressed
	if (e->button() == LeftButton) {
		bool found = FALSE;
		QPoint clickpt;

		uint tabrow = rowAt(contentsY() + e->pos().y());

		// Clicks on non-existing rows are not allowed
		if (tabrow >= curt->b.size())
			return;

		clickpt.setX(contentsX() + e->pos().x());
		clickpt.setY(contentsY() + e->pos().y());

		int xpos = trp->getFirstColOffs(tabrow, curt);
		int xdelta = 0;
		int lastxpos = 0;

		for (uint j=curt->b[tabrow].start;
			 j < (tabrow < curt->b.size()-1 ? curt->b[tabrow+1].start : curt->c.size());
			 j++) {

			// Length of interval to next column - adjusted if dotted
			xdelta = horizDelta(j);

			// Current column X area is half of the previous duration and
			// half of current duration

			if ((clickpt.x() >= (lastxpos + xpos) / 2) &&
				(clickpt.x() <= xpos + xdelta / 2)) {
				curt->x = j;
				// We won't calculate xb from x as in updateXB(), but
				// would just use what we know.
				curt->xb = tabrow;

				const int vertline = trp->ysteptb;
				const int vertspace = trp->ypostb; // LVIFIX: better name, this is not the vertical space but the lowest tab line's y coord
				curt->y = - ((int) (clickpt.y() - vertline / 2 - tabrow * cellHeight()) - vertspace) / vertline;

				if (curt->y<0)
					curt->y = 0;
				if (curt->y>=curt->string)
					curt->y = curt->string-1;

				curt->sel = FALSE;

				emit columnChanged();
				emit barChanged();
				found = TRUE;
				break;
			}

			lastxpos = xpos;
			xpos += xdelta;
		}

		if (found)
			repaintContents();
	}
}

void TrackView::setX(int x)
{
	if (x < (int) curt->c.size()) {
		curt->x = x;
		int oldxb = curt->xb;
		curt->updateXB();
		if (oldxb == curt->xb) {
			repaintCurrentCell();
		} else {
			repaintContents();
			ensureCurrentVisible();
		}
		emit columnChanged();
		lastnumber = -1;
	}
}

void TrackView::setPlaybackCursor(bool pc)
{
    playbackCursor = pc;
	repaintContents();
}
	
void TrackView::viewScore(bool on)
{
//	cout << "TrackView::viewScore(on=" << on << ")" << endl;
	viewscore = on;
	updateRows();
}
