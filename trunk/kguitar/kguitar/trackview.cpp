#include "config.h"

#include "trackview.h"
#include "trackviewcommands.h"
#include "tabsong.h"
#include "chord.h"
#include "rhythmer.h"
#include "keysig.h"
#include "timesig.h"
#include "songview.h"

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kdebug.h>
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

#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <stdlib.h>		// required for declaration of abs()

#define VERTSPACE                       30
#define VERTLINE                        10
#define HORDUR                          4
#define HORCELL                         14
#define TIMESIGSIZE                     14
#define HORSCALE                        10
#define ABBRLENGTH                      25

#define BOTTOMDUR   VERTSPACE+VERTLINE*(s+1)

#define TIME_SIG_FONT_FACTOR            1.4
#define SMALL_CAPTION_FONT_FACTOR       0.7

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
	setCurt(s->t.first());

	updateRows();

 	smallCaptionFont = new QFont(KGlobalSettings::generalFont());
	if (smallCaptionFont->pointSize() == -1) {
		smallCaptionFont->setPixelSize((int) ((double) smallCaptionFont->pixelSize() * SMALL_CAPTION_FONT_FACTOR));
	} else {
		smallCaptionFont->setPointSizeFloat(smallCaptionFont->pointSizeFloat() * SMALL_CAPTION_FONT_FACTOR);
	}

  	timeSigFont = new QFont(KGlobalSettings::generalFont());
	if (timeSigFont->pointSize() == -1) {
		timeSigFont->setPixelSize((int) ((double) timeSigFont->pixelSize() * TIME_SIG_FONT_FACTOR));
	} else {
		timeSigFont->setPointSizeFloat(timeSigFont->pointSizeFloat() * TIME_SIG_FONT_FACTOR);
	}
  	timeSigFont->setBold(TRUE);

	lastnumber = -1;

#ifdef WITH_TSE3
	scheduler = _scheduler;
#endif
}

TrackView::~TrackView()
{
 	delete smallCaptionFont;
	delete timeSigFont;
}

void TrackView::selectTrack(TabTrack *trk)
{
	setCurt(trk);
	updateRows();
	repaintContents();
}

void TrackView::selectBar(uint n)
{
	if (n < curt->b.size()) {
		curt->x = curt->b[n].start;
		curt->updateXB();
		ensureCurrentVisible();
		emit statusBarChanged();
	}
	lastnumber = -1;
}

void TrackView::setCurt(TabTrack *trk)
{
	curt = trk;
	emit newTrackSelected();
}

void TrackView::updateRows()
{
	int ch = VERTSPACE * 2 + VERTLINE * (curt->string - 1);
	setNumRows(curt->b.size());
	setMinimumHeight(ch);
	setCellHeight(ch);
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
	repaintContents(selxcoord, cellHeight() * curt->xb, HORCELL + 1, cellHeight());
	emit paneChanged();
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

void TrackView::setFinger(int num, int fret)
{
	curt->c[curt->x].a[num] = fret;
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
	int res = curt->c[n].fullDuration() / HORSCALE * HORCELL;
	if (res < HORCELL)
		res = HORCELL;
	return res;
}

void TrackView::drawLetRing(QPainter *p, int x, int y)
{
	p->setPen(SolidLine);
	p->drawLine(x, y, x - HORCELL / 3, y - VERTLINE / 3);
	p->drawLine(x, y, x - HORCELL / 3, y + VERTLINE / 3);
	p->setPen(NoPen);
}

void TrackView::paintCell(QPainter *p, int row, int col)
{
	if (row >= int(curt->b.size())) {
		kdDebug() << "Drawing the bar out of limits!" << endl;
		return;
	}

	uint bn = row;						// Drawing only this bar

	QString tmp;
	bool ringing[MAX_STRINGS];
	int trpCnt = 0;						// triplet count

	int s = curt->string - 1;

	for (int i = 0; i <= s; i++) {
		p->drawLine(0, VERTSPACE + (s - i) * VERTLINE,
					width(), VERTSPACE + (s - i) * VERTLINE);
		ringing[i] = FALSE;
	}

	int xpos = 40, lastxpos = 20, xdelta;

	int selx2coord = -1;
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

	p->setFont(KGlobalSettings::generalFont());
	p->setBrush(KGlobalSettings::baseColor());

	// Drum abbreviations markings

	if (curt->trackMode() == DrumTab) {
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
				p->setFont(KGlobalSettings::generalFont());
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
					p->setFont(KGlobalSettings::generalFont());
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
			p->setFont(KGlobalSettings::generalFont());
		}

		// Draw arcs to backward note

		if (curt->c[t].flags & FLAG_ARC)
			p->drawArc(lastxpos + HORCELL / 2, BOTTOMDUR + 9,
					   xpos-lastxpos, 10, 0, -180 * 16);

		// Draw palm muting

		if (curt->c[t].flags & FLAG_PM) {
			p->setFont(*smallCaptionFont);
			p->drawText(xpos, VERTSPACE / 2, VERTLINE * 2, VERTLINE,
						AlignCenter, "P.M.");
			p->setFont(KGlobalSettings::generalFont());
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
 				p->setFont(KGlobalSettings::generalFont());
				break;
			case EFFECT_ARTHARM:
 				p->setFont(*smallCaptionFont);
				p->drawText(xpos + VERTLINE + 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE * 2 / 3,
							HORCELL * 2, VERTLINE, AlignCenter, "AH");
 				p->setFont(KGlobalSettings::generalFont());
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
 					p->setFont(KGlobalSettings::generalFont());
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

	// DRAW SELECTION

	p->setRasterOp(Qt::XorROP);
// 	p->setBrush(KGlobalSettings::highlightColor());
	if (curt->sel) {
		if ((selxcoord != -1) && (selx2coord != -1)) {
			int x1 = KMIN(selxcoord, selx2coord);
			int wid = abs(selx2coord - selxcoord) + VERTLINE + 1;
			p->drawRect(x1, 0, wid, cellHeight());
		} else if ((selxcoord == -1) && (selx2coord != -1)) {
			if (curt->x > curt->lastColumn(bn))
				p->drawRect(selx2coord, 0, cellWidth(), cellHeight());
			else
				p->drawRect(0, 0, selx2coord + HORCELL + 1, cellHeight());
		} else if ((selxcoord != -1) && (selx2coord == -1)) {
			if (curt->xsel > curt->lastColumn(bn))
				p->drawRect(selxcoord, 0, cellWidth(), cellHeight());
			else
				p->drawRect(0, 0, selxcoord + HORCELL + 1, cellHeight());
		} else { // both are -1
			int x1 = KMIN(curt->x, curt->xsel);
			int x2 = KMAX(curt->x, curt->xsel);
			if ((x1 < curt->b[bn].start) && (x2 > curt->lastColumn(bn)))
				p->drawRect(0, 0, cellWidth(), cellHeight());
		}
	}

	if (selxcoord != -1) {
		p->drawRect(selxcoord - 1, VERTSPACE + (s - curt->y) * VERTLINE - VERTLINE / 2 - 1,
					HORCELL + 2, VERTLINE + 2);
	}

// 	p->setBrush(KGlobalSettings::baseColor());
	p->setRasterOp(Qt::CopyROP);

	p->setBrush(SolidPattern);
}

void TrackView::resizeEvent(QResizeEvent *e)
{
	QGridView::resizeEvent(e); // GREYFIX ? Is it C++-correct?
	setCellWidth(width() - 2);
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

	SetKeySig *sks = new SetKeySig();
	sks->sig->setCurrentItem(7 - oldsig);

	if (sks->exec()) {
		int newsig = sks->sig->currentItem();
		curt->b[0].keysig = (short) (7 - newsig);
	}

	lastnumber = -1;
}

void TrackView::timeSig()
{
	SetTimeSig *sts = new SetTimeSig();

	sts->time1->setValue(curt->b[curt->xb].time1);

	switch (curt->b[curt->xb].time2) {
	case 1:	 sts->time2->setCurrentItem(0); break;
	case 2:	 sts->time2->setCurrentItem(1); break;
	case 4:	 sts->time2->setCurrentItem(2); break;
	case 8:	 sts->time2->setCurrentItem(3); break;
	case 16: sts->time2->setCurrentItem(4); break;
	case 32: sts->time2->setCurrentItem(5); break;
	}

	if (sts->exec()) {
		int time1 = sts->time1->value();
		int time2 = ((QString) sts->time2->currentText()).toUInt();

		cmdHist->addCommand(new SetTimeSigCommand(this, curt, sts->toend->isChecked(),
													time1, time2));
	}

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

void TrackView::moveLeft()
{
	if (curt->x > 0) {
		if (curt->b[curt->xb].start == curt->x) {
			curt->x--;
			repaintCurrentCell();
			curt->xb--;
			ensureCurrentVisible();
			emit statusBarChanged();
		} else {
			curt->x--;
		}
		repaintCurrentCell();
	}
	lastnumber = -1;
}

void TrackView::moveRight()
{
	if (curt->x + 1 == curt->c.size()) {
		cmdHist->addCommand(new AddColumnCommand(this, curt));
	} else {
		if (curt->b.size() == curt->xb + 1)
			curt->x++;
		else {
			if (curt->b[curt->xb + 1].start == curt->x + 1) {
				curt->x++;
				repaintCurrentCell();
				curt->xb++;
				ensureCurrentVisible();
				emit statusBarChanged();
			} else {
				curt->x++;
			}
		}
		repaintCurrentCell();
	}
	lastnumber = -1;
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
		else repaintCurrentColumn();
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
		else repaintCurrentColumn();
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
	lastnumber = -1;
}

void TrackView::deleteNote()
{
	if (curt->c[curt->x].a[curt->y] != -1)
		cmdHist->addCommand(new DeleteNoteCommand(this, curt));
	lastnumber = -1;
}

void TrackView::deleteColumn()
{
	cmdHist->addCommand(new DeleteColumnCommand(this, curt));
	lastnumber = -1;
}

void TrackView::deleteColumn(QString name)
{
	cmdHist->addCommand(new DeleteColumnCommand(name, this, curt));
}

void TrackView::insertColumn()
{
	cmdHist->addCommand(new InsertColumnCommand(this, curt));
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
	emit statusBarChanged();
	updateRows();
	repaintContents();

	emit paneChanged();
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
}

void TrackView::arrangeBars()
{
	song->arrangeBars();
	emit statusBarChanged();
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

		uint tabrow = rowAt(e->pos().y());

		// Clicks on non-existing rows are not allowed
		if (tabrow >= curt->b.size())
			return;

		clickpt.setX(contentsX() + e->pos().x());
		clickpt.setY(contentsY() + e->pos().y());

		int xpos=40, xdelta, lastxpos = 20;

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

				curt->y = curt->string - 1 -
						  ((int) (clickpt.y() - tabrow * cellHeight()) - VERTSPACE) / VERTLINE;

				if (curt->y<0)
					curt->y = 0;
				if (curt->y>=curt->string)
					curt->y = curt->string-1;

				curt->sel = FALSE;

				emit statusBarChanged();
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


