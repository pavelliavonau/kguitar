#include "trackview.h"
#include "tabsong.h"
#include "chord.h"
#include "timesig.h"

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

#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <stdlib.h>


#define VERTSPACE 30
#define VERTLINE 10
#define HORDUR 4
#define HORCELL 8
#define TIMESIGSIZE 14
#define HORSCALE 10
#define ABBRLENGTH 25

#define BOTTOMDUR	VERTSPACE+VERTLINE*(s+1)

TrackView::TrackView(TabSong *s, KXMLGUIClient *_XMLGUIClient, DeviceManager *_dm,
					 QWidget *parent = 0, const char *name = 0): QTableView(parent, name)
{
	setTableFlags(Tbl_autoVScrollBar | Tbl_smoothScrolling);
	setFrameStyle(Panel | Sunken);
	setBackgroundMode(PaletteBase);

	setNumCols(1);

	setFocusPolicy(QWidget::StrongFocus);

    m_XMLGUIClient = _XMLGUIClient;
	midi = _dm;

	song = s;
	setCurt(s->t.first());

	updateRows();

	lastnumber = -1;
}

TrackView::~TrackView()
{
}

void TrackView::selectTrack(TabTrack *trk)
{
	setCurt(trk);
	updateRows();
	update();
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
	int ycoord = 0;
	if (rowYPos(n, &ycoord))
		repaint(0, ycoord, width(), cellHeight());
}

void TrackView::repaintCurrentCell()
{
	repaintCellNumber(curt->xb);
}

void TrackView::repaintCurrentColumn()
{
	//VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2

	int ycoord = 0;
	if (rowYPos(curt->xb, &ycoord))
		repaint(selxcoord, ycoord, VERTLINE + 1, cellHeight());
}

// Checks is current bar is fully visible, and, if it's not, tries to
// do minimal scrolling to ensure the full visibility
void TrackView::ensureCurrentVisible()
{
	int ch = cellHeight();

	if ((curt->xb + 1) * ch > yOffset() + height())
		setYOffset((curt->xb + 1) * ch - height());
	else if (curt->xb * ch < yOffset())
		setYOffset(curt->xb * ch);
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
	curt->c[curt->x].l = l;
	repaintCurrentCell();
}

void TrackView::linkPrev()
{
	curt->c[curt->x].flags ^= FLAG_ARC;
	for (uint i = 0; i < MAX_STRINGS; i++) {
		curt->c[curt->x].a[i] = -1;
		curt->c[curt->x].e[i] = 0;
	}
	repaintCurrentCell();
	lastnumber = -1;
}

void TrackView::addHarmonic()
{
	curt->addFX(EFFECT_HARMONIC);
	repaintCurrentCell();
	lastnumber = -1;
}

void TrackView::addArtHarm()
{
	curt->addFX(EFFECT_ARTHARM);
	repaintCurrentCell();
	lastnumber = -1;
}

void TrackView::addLegato()
{
	curt->addFX(EFFECT_LEGATO);
	repaintCurrentCell();
	lastnumber = -1;
}

void TrackView::insertChord()
{
	int a[MAX_STRINGS];

	ChordSelector cs(midi, curt);
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
		curt->insertStrum(cs.scheme(), a);
	}

	update();
	lastnumber = -1;
}

// Determine horizontal offset between two columns - n and n+1
int TrackView::horizDelta(uint n)
{
	int res = (curt->c[n].flags & FLAG_DOT ?
	           curt->c[n].l*3/2 : curt->c[n].l) / HORSCALE * HORCELL;
	if (res < HORCELL)
		res = HORCELL;
	return res;
}

void TrackView::paintCell(QPainter *p, int row, int col)
{
	if (row >= curt->b.size()) {
		kdDebug() << "Drawing the bar out of limits!" << endl;
		return;
	}

	uint bn = row;						// Drawing only this bar

	//	int last = curt->lastColumn(bn);

	QString tmp;

	uint s = curt->string - 1;
	int i;

	for (i = 0; i <= s; i++)
		p->drawLine(0, VERTSPACE + (s - i) * VERTLINE,
					width(), VERTSPACE + (s - i) * VERTLINE);

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
		p->setFont(QFont("helvetica", TIMESIGSIZE, QFont::Bold));
		tmp.setNum(curt->b[bn].time1);
		p->drawText(20, VERTSPACE + VERTLINE * s / 4 - TIMESIGSIZE / 2,
					TIMESIGSIZE, TIMESIGSIZE, AlignCenter, tmp);
		tmp.setNum(curt->b[bn].time2);
		p->drawText(20, VERTSPACE + VERTLINE * s * 3 / 4 - TIMESIGSIZE / 2,
					 TIMESIGSIZE, TIMESIGSIZE, AlignCenter, tmp);
	}

	p->setFont(QFont("helvetica", VERTLINE));
	p->setBrush(KGlobalSettings::baseColor());

	// Drum abbreviations markings

	if (curt->trackMode() == DrumTab) {
		p->setPen(NoPen);
		for (i = 0; i <= s; i++) {
			p->drawRect(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
						ABBRLENGTH, VERTLINE + 1);
			p->drawText(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
						ABBRLENGTH, VERTLINE, AlignCenter, drum_abbr[curt->tune[i]]);
		}
		xpos += ABBRLENGTH + 10; lastxpos += ABBRLENGTH + 10;
		p->setPen(SolidLine);
	}

	for (uint t = curt->b[bn].start; t <= curt->lastColumn(bn); t++) {
		// Drawing duration marks

		// Draw connection with previous, if applicable
		if ((t > 0) && (t>curt->b[bn].start) && (curt->c[t-1].l == curt->c[t].l))
			xdelta = lastxpos + VERTLINE / 2;
		else
			xdelta = xpos + VERTLINE / 2 + HORDUR;

		switch (curt->c[t].l) {
		case 15:  // 1/32
			p->drawLine(xpos + VERTLINE / 2, BOTTOMDUR + VERTLINE - 4,
						xdelta, BOTTOMDUR + VERTLINE - 4);
		case 30:  // 1/16
			p->drawLine(xpos + VERTLINE / 2, BOTTOMDUR + VERTLINE - 2,
						xdelta, BOTTOMDUR + VERTLINE - 2);
		case 60:  // 1/8
			p->drawLine(xpos + VERTLINE / 2, BOTTOMDUR + VERTLINE,
						xdelta, BOTTOMDUR + VERTLINE);
		case 120: // 1/4 - a long vertical line, so we need to find the highest note
			for (i = s;((i >= 0) && (curt->c[t].a[i] == -1)); i--);

			// If it's an empty measure at all - draw the vertical line from bottom
			if (i < 0)  i = 1;

			p->drawLine(xpos + VERTLINE / 2, VERTSPACE + VERTLINE * (s - i) + VERTLINE / 2,
						xpos + VERTLINE / 2, BOTTOMDUR + VERTLINE);
		case 240: // 1/2
			p->drawLine(xpos + VERTLINE / 2, BOTTOMDUR + 3,
						xpos + VERTLINE / 2, BOTTOMDUR + VERTLINE);
		case 480: // whole
			break;
		}

		// Draw dot

		if (curt->c[t].flags & FLAG_DOT)
			p->drawRect(xpos + VERTLINE / 2 + 3, BOTTOMDUR + 5, 2, 2);

		// Draw arcs to backward note

		if (curt->c[t].flags & FLAG_ARC)
			p->drawArc(lastxpos + VERTLINE / 2, BOTTOMDUR + 9,
					   xpos-lastxpos, 10, 0, -180 * 16);

		// Draw palm muting

		if (curt->c[t].flags & FLAG_PM)
			p->drawText(xpos + VERTLINE / 2, 0, VERTLINE * 2, VERTLINE,
						AlignCenter, "P.M.");

		// Length of interval to next column - adjusted if dotted

		xdelta = horizDelta(t);

		// Draw the number column

		p->setPen(NoPen);
		for (i = 0; i <= s; i++) {
			if (curt->c[t].a[i] != -1) {
				if (curt->c[t].a[i] == DEAD_NOTE)
					tmp = "X";
				else
					tmp.setNum(curt->c[t].a[i]);
				p->drawRect(xpos,VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
							VERTLINE, VERTLINE + 1);
				p->drawText(xpos,VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
							VERTLINE, VERTLINE, AlignCenter, tmp);
			}

			if (t == curt->x) {
				selxcoord = xpos;
			}

			if (t == curt->xsel) {
				selx2coord = xpos;
			}

			// Draw effects
			switch (curt->c[t].e[i]) {
			case EFFECT_HARMONIC:
				p->drawText(xpos + VERTLINE + 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
							VERTLINE, VERTLINE, AlignCenter, "H");
				break;
			case EFFECT_ARTHARM:
				p->drawText(xpos + VERTLINE + 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
							VERTLINE * 2, VERTLINE, AlignCenter, "AH");
				break;
			case EFFECT_LEGATO:
				p->setPen(SolidLine);
				p->drawArc(xpos + VERTLINE, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
						   xdelta - VERTLINE, 10, 0, 180 * 16);
				p->drawText(xpos + VERTLINE + 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
							VERTLINE * 2, VERTLINE, AlignCenter, "PO");
				p->setPen(NoPen);
				break;
			}
		}

		p->setPen(SolidLine);

		lastxpos = xpos;
		xpos += xdelta;
	}

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
				p->drawRect(0, 0, selx2coord + VERTLINE + 1, cellHeight());
		} else if ((selxcoord != -1) && (selx2coord == -1)) {
			if (curt->xsel > curt->lastColumn(bn))
				p->drawRect(selxcoord, 0, cellWidth(), cellHeight());
			else
				p->drawRect(0, 0, selxcoord + VERTLINE + 1, cellHeight());
		} else { // both are -1
			int x1 = KMIN(curt->x, curt->xsel);
			int x2 = KMAX(curt->x, curt->xsel);
			if ((x1 < curt->b[bn].start) && (x2 > curt->lastColumn(bn)))
				p->drawRect(0, 0, cellWidth(), cellHeight());
		}
	}

	if (selxcoord != -1) {
		p->drawRect(selxcoord, VERTSPACE + (s - curt->y) * VERTLINE - VERTLINE / 2,
					VERTLINE, VERTLINE + 1);
	}

// 	p->setBrush(KGlobalSettings::baseColor());
	p->setRasterOp(Qt::CopyROP);

	p->setBrush(SolidPattern);
}

void TrackView::resizeEvent(QResizeEvent *e)
{
	QTableView::resizeEvent(e); // GREYFIX ? Is it C++-correct?
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

	curt->c[curt->x].a[from] = -1;
	curt->c[curt->x].a[to] = n;

    // ...also for the effect parameter
	curt->c[curt->x].e[to] = curt->c[curt->x].e[from];
	curt->c[curt->x].e[from] = 0;

	curt->y = to;
	return TRUE;
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

		// Sophisticated construction to mark all or only one bar with
		// new sig, depending on user's selection of checkbox

		for (uint i = curt->xb;
			 i < (sts->toend->isChecked() ? curt->b.size() : curt->xb+1);
			 i++) {
			curt->b[i].time1 = time1;
			curt->b[i].time2 = time2;
		}
	}

	emit paneChanged();
	lastnumber = -1;
}

void TrackView::keyLeft()
{
	if (curt->sel) {
		curt->sel = FALSE;
		repaint();
	} else {
		moveLeft();
	}
}

void TrackView::keyRight()
{
	if (curt->sel) {
		curt->sel = FALSE;
		repaint();
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
		curt->c.resize(curt->c.size()+1);
		curt->x++;
		for (uint i = 0; i < MAX_STRINGS; i++) {  // Set it for all strings,
			curt->c[curt->x].a[i] = -1;           // so we didn't get crazy
			curt->c[curt->x].e[i] = 0;            // data - alinx
		}
		curt->c[curt->x].l = curt->c[curt->x - 1].l;
		curt->c[curt->x].flags = 0;
		updateRows();
		repaintCurrentCell();
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
		repaintCurrentColumn();
	}
	lastnumber = -1;
}

void TrackView::transposeUp()
{
	if (curt->y+1 < curt->string)
		if (moveFinger(curt->y, 1))
			repaintCurrentColumn();
	lastnumber = -1;
}

void TrackView::moveDown()
{
	if (curt->y > 0) {
		curt->y--;
		repaintCurrentColumn();
	}
	lastnumber = -1;
}

void TrackView::transposeDown()
{
	if (curt->y > 0)
		if (moveFinger(curt->y, -1))
			repaintCurrentColumn();
	lastnumber = -1;
}

void TrackView::deadNote()
{
	if (curt->c[curt->x].flags & FLAG_ARC)
		curt->c[curt->x].flags -= FLAG_ARC;
	curt->c[curt->x].a[curt->y] = DEAD_NOTE;

	repaintCurrentCell();
	emit paneChanged();
	lastnumber = -1;
}

void TrackView::deleteNote()
{
	if (curt->c[curt->x].a[curt->y] != -1) {
		curt->c[curt->x].a[curt->y] = -1;
		curt->c[curt->x].e[curt->y] = 0;
		repaintCurrentColumn();
		emit paneChanged();
	}
	lastnumber = -1;
}

void TrackView::deleteColumn()
{
	if (curt->c.size() > 1) {
		curt->removeColumn(1);
		if (curt->x == curt->c.size())
			curt->x--;
		updateRows();
	}

	update();
	emit paneChanged();
	lastnumber = -1;
}

void TrackView::insertColumn()
{
	curt->insertColumn(1);

	update();
	emit paneChanged();
	lastnumber = -1;
}

void TrackView::palmMute()
{
	curt->c[curt->x].flags ^= FLAG_PM;
	repaintCurrentCell();
	lastnumber = -1;
}

void TrackView::keyPeriod()
{
	curt->c[curt->x].flags ^= FLAG_DOT; // It's XOR :-)
	repaintCurrentCell();
	lastnumber = -1;
}

void TrackView::keyPlus()
{
	if (curt->c[curt->x].l < 480)
		curt->c[curt->x].l *= 2;
	repaintCurrentCell();
	lastnumber = -1;
}

void TrackView::keyMinus()
{
	if (curt->c[curt->x].l > 15)
		curt->c[curt->x].l /= 2;
	repaintCurrentCell();
	lastnumber = -1;
}

void TrackView::arrangeTracks()
{
	curt->arrangeBars();
	emit statusBarChanged();
	updateRows();
	update();

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

	if ((totab <= curt->frets) && (curt->c[curt->x].a[curt->y] != totab)) {
		curt->c[curt->x].a[curt->y] = totab;
		repaintCurrentColumn();
		emit paneChanged();
	}
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
		tmpWidget = m_XMLGUIClient->factory()->container("trackviewpopup", m_XMLGUIClient);

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

		uint tabrow = findRow(e->pos().y());

		// Clicks on non-existing rows are not allowed
		if (tabrow >= curt->b.size())
			return;

		clickpt.setX(xOffset() + e->pos().x());
		clickpt.setY(yOffset() + e->pos().y());

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
			repaint();
	}
}


