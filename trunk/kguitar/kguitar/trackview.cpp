#include "trackview.h"
#include "tabsong.h"

#include "timesig.h"

#include <kglobalsettings.h>
#include <iostream.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qpen.h>
#include <qkeycode.h>

#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>

TrackView::TrackView(QWidget *parent,const char *name): QTableView(parent,name)
{
	setTableFlags(Tbl_autoVScrollBar | Tbl_smoothScrolling);
	setFrameStyle(Panel | Sunken);
	setBackgroundMode(PaletteBase);

	setNumCols(1);

	setFocusPolicy(QWidget::StrongFocus);

	song = new TabSong("Unnamed",120);
	song->t.append(new TabTrack(GuitarTab,"Guitar",1,0,25,6,24));

	curt = song->t.first();

	uchar standtune[6] = {40, 45, 50, 55, 59, 64};

	for (int i=0;i<6;i++)
		curt->tune[i] = standtune[i];

	curt->c.resize(1);
	curt->b.resize(1);

	for (int i=0;i<MAX_STRINGS;i++) {
		curt->c[0].a[i] = -1;
		curt->c[0].e[i] = 0;
	}
	curt->c[0].l = 120;
	curt->c[0].flags = 0;

	curt->b[0].start=0;
	curt->b[0].time1=4;
	curt->b[0].time2=4;
	
	updateRows();

	curt->x=0;
	curt->xb=0;
	curt->y=0;
	lastnumber=0;
}

TrackView::~TrackView()
{
	delete song;
}

void TrackView::updateRows()
{
	setNumRows(curt->b.size());
}

void TrackView::setFinger(int num,int fret)
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
	repaint();
}

void TrackView::linkPrev()
{
	curt->c[curt->x].flags ^= FLAG_ARC;
	for (uint i=0;i<MAX_STRINGS;i++) {
		curt->c[curt->x].a[i] = -1;
		curt->c[curt->x].e[i] = 0;
	}
	update();
}

void TrackView::addHarmonic()
{
	curt->addFX(EFFECT_HARMONIC);
	update();
}

void TrackView::addArtHarm()
{
	curt->addFX(EFFECT_ARTHARM);
	update();
}

void TrackView::addLegato()
{
	curt->addFX(EFFECT_LEGATO);
	update();
}

#define VERTSPACE 30
#define VERTLINE 10
#define HORDUR 4
#define HORCELL 8
#define TIMESIGSIZE 14
#define HORSCALE 10

#define BOTTOMDUR	VERTSPACE+VERTLINE*(s+1)

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
	uint bn = row;						// Drawing only this bar

	int last;
	if (curt->b.size()==bn+1)           // Current bar is the last one
		last = curt->c.size()-1;		// Draw till the last note
	else								// Else draw till the end of this bar
		last = curt->b[bn+1].start-1;
	if(last==-1) last=0;//gotemfix: avoid overflow
	QString tmp;

	uint s = curt->string-1;
	uint i;

	for (i=0;i<=s;i++)
		p->drawLine(0,VERTSPACE+(s-i)*VERTLINE,width(),VERTSPACE+(s-i)*VERTLINE);
	
	int xpos=40, lastxpos=20, xdelta;

	// Starting bars - very thick and thick one

	if (bn==0) {
		p->setBrush(SolidPattern);
		p->drawRect(0,VERTSPACE,5,VERTLINE*s);
		p->drawRect(8,VERTSPACE,2,VERTLINE*s);
	}

	// Time signature

	if (curt->showBarSig(bn)) {
		p->setFont(QFont("helvetica",TIMESIGSIZE,QFont::Bold));
		tmp.setNum(curt->b[bn].time1);
		p->drawText(20,VERTSPACE+VERTLINE*s/3-TIMESIGSIZE/2,
					TIMESIGSIZE,TIMESIGSIZE,AlignCenter,tmp);
		tmp.setNum(curt->b[bn].time2);
		p->drawText(20,VERTSPACE+VERTLINE*s*2/3-TIMESIGSIZE/2,
					TIMESIGSIZE,TIMESIGSIZE,AlignCenter,tmp);
	}

	p->setFont(QFont("helvetica",VERTLINE));
	p->setBrush(KGlobalSettings::baseColor());
	for (uint t = curt->b[bn].start; t <= last; t++) {
		// Drawing duration marks
		
		// Draw connection with previous, if applicable
		if ((t>0) && (t>curt->b[bn].start) && (curt->c[t-1].l == curt->c[t].l))
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
			for (i=s;((i>=0) && (curt->c[t].a[i]==-1));i--);
			
			// If it's an empty measure at all - draw the vertical line from bottom
			if (i<0)  i=1;
			
			p->drawLine(xpos+VERTLINE/2,VERTSPACE+VERTLINE*(s-i)+VERTLINE/2,
						xpos+VERTLINE/2,BOTTOMDUR+VERTLINE);
		case 240: // 1/2
			p->drawLine(xpos+VERTLINE/2,BOTTOMDUR+3,
						xpos+VERTLINE/2,BOTTOMDUR+VERTLINE);
		case 480: // whole
			break;
		}
		
		// Draw dot
		
		if (curt->c[t].flags & FLAG_DOT)
			p->drawRect(xpos+VERTLINE/2+3, BOTTOMDUR+5, 2, 2);
		
		// Draw arcs to backward note
		
		if (curt->c[t].flags & FLAG_ARC)
			p->drawArc(lastxpos+VERTLINE/2, BOTTOMDUR+9,
					   xpos-lastxpos, 10,
					   0, -180 * 16);
		
		// Draw palm muting

		if (curt->c[t].flags & FLAG_PM)
			p->drawText(xpos + VERTLINE / 2, 0, VERTLINE * 2, VERTLINE,
						AlignCenter, "P.M.");

		// Length of interval to next column - adjusted if dotted
		
		xdelta = horizDelta(t);
		
		// Draw the number column
		
		p->setPen(NoPen);
		for (i = 0; i <= s; i++) {
			if ((t == curt->x) && (i == curt->y)) {
				p->setBrush(KGlobalSettings::highlightColor());
				p->drawRect(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
							VERTLINE, VERTLINE + 1);
				p->setBrush(KGlobalSettings::baseColor());
				if (curt->c[t].a[i] != -1) {
					if (curt->c[t].a[i]==DEAD_NOTE)
						tmp = "X";
					else
						tmp.setNum(curt->c[t].a[i]);
					p->setPen(KGlobalSettings::highlightedTextColor());
					p->drawText(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
								VERTLINE, VERTLINE, AlignCenter, tmp);
					p->setPen(NoPen);
				}
			} else {
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
			}
			
			// Draw effects
			switch (curt->c[t].e[i]) {
			case EFFECT_HARMONIC:
				p->drawText(xpos+VERTLINE+2,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
							VERTLINE,VERTLINE,AlignCenter,"H");
				break;
			case EFFECT_ARTHARM:
				p->drawText(xpos+VERTLINE+2,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
							VERTLINE*2,VERTLINE,AlignCenter,"AH");
				break;
			case EFFECT_LEGATO:
				p->setPen(SolidLine);
				p->drawArc(xpos+VERTLINE,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
						   xdelta-VERTLINE, 10, 0, 180*16);
				p->drawText(xpos+VERTLINE+2,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
							VERTLINE*2,VERTLINE,AlignCenter,"PO");
				p->setPen(NoPen);
				break;
			}
		}
		
		p->setPen(SolidLine);
		
		lastxpos = xpos;
		xpos += xdelta;
	}
	
	p->drawRect(xpos,VERTSPACE,1,VERTLINE*s);
	
	p->setBrush(SolidPattern);
}

void TrackView::resizeEvent(QResizeEvent *e)
{
	QTableView::resizeEvent(e); // GREYFIX ? Is it C++-correct?
	setCellWidth(width()-2);
	setCellHeight(VERTSPACE*2+VERTLINE*(curt->string-1));
}

bool TrackView::moveFinger(int from, int dir)
{
	int n0 = curt->c[curt->x].a[from];
	int n = n0;
	if (n<0)
	return FALSE;

	int to = from;
	
	do {
		to += dir;
		if ((to<0) || (to>=curt->string))
			return FALSE;
		n=n0+curt->tune[from]-curt->tune[to];
		if ((n<0) || (n>curt->frets))
			return FALSE;
	} while (curt->c[curt->x].a[to]!=-1);
	
	curt->c[curt->x].a[from] = -1;
	curt->c[curt->x].a[to] = n;
	
	curt->y = to;
	return TRUE;
}

void TrackView::timeSig()
{
	SetTimeSig *sts = new SetTimeSig();

	sts->time1->setValue(curt->b[curt->xb].time1);
	
	switch (curt->b[curt->xb].time2) {
	case 1:	 sts->time2->setCurrentItem(0);break;
	case 2:	 sts->time2->setCurrentItem(1);break;
	case 4:	 sts->time2->setCurrentItem(2);break;
	case 8:	 sts->time2->setCurrentItem(3);break;
	case 16: sts->time2->setCurrentItem(4);break;
	case 32: sts->time2->setCurrentItem(5);break;
	}

	if (sts->exec()) {
		int time1 = sts->time1->value();
		int time2 = ((QString) sts->time2->currentText()).toUInt();

		// Sophisticated construction to mark all or only one bar with
		// new sig, depending on user's selection of checkbox
		
		for (uint i=curt->xb;
			 i<(sts->toend->isChecked() ? curt->b.size() : curt->xb+1);
			 i++) {
			curt->b[i].time1 = time1;
			curt->b[i].time2 = time2;
		}
	}
}

void TrackView::keyPressEvent(QKeyEvent *e)
{
	int num = e->ascii();

	switch (e->key()) {
	case Key_1:
	case Key_2:
	case Key_3:
	case Key_4:
	case Key_5:
	case Key_6:
	case Key_7:
	case Key_8:
	case Key_9:
	case Key_0:
		break;
	default:
		lastnumber=0;
	}

	switch (e->key()) {
	case Key_Left:
		if (curt->x>0) {
			if (curt->b[curt->xb].start==curt->x) {
				curt->xb--;
				emit statusBarChanged();
			}
			curt->x--;
		}
		break;
	case Key_Right:
		if (curt->x+1==curt->c.size()) {
			curt->c.resize(curt->c.size()+1);
			curt->x++;
			for (uint i=0;i<curt->string;i++) {
				curt->c[curt->x].a[i] = -1;
				curt->c[curt->x].e[i] = 0;
			}
			curt->c[curt->x].l = curt->c[curt->x-1].l;
			curt->c[curt->x].flags = 0;
			updateRows();
		} else {
			if (curt->b.size()==curt->xb+1)
				curt->x++;
			else {
				if (curt->b[curt->xb+1].start==curt->x+1) {
					curt->xb++;
					emit statusBarChanged();
				}
				curt->x++;
			}
		}
		break;
	case Key_Down:
		if (curt->y>0) {
			if (e->state()==ControlButton)
				moveFinger(curt->y,-1);
			else 
				curt->y--;
		}
		break;
	case Key_Up:
		if (curt->y+1<curt->string) {
			if (e->state()==ControlButton)
				moveFinger(curt->y,1);
			else
				curt->y++;
		}
		break;
	case Key_1:
	case Key_2:
	case Key_3:
	case Key_4:
	case Key_5:
	case Key_6:
	case Key_7:
	case Key_8:
	case Key_9:
	case Key_0:
		if (curt->c[curt->x].flags & FLAG_ARC)
			curt->c[curt->x].flags -= FLAG_ARC;
		
		num=num-'0'; // GREYFIX - may be a bad thing to do
		
		// Allow making two-digit fret numbers pressing two keys sequentally
		if (lastnumber*10+num<=curt->frets)
			num=lastnumber*10+num;
		
		curt->c[curt->x].a[curt->y]=num;
		lastnumber=num;
		break;
	case Key_X:
		if (curt->c[curt->x].flags & FLAG_ARC)
			curt->c[curt->x].flags -= FLAG_ARC;
		curt->c[curt->x].a[curt->y] = DEAD_NOTE;
		break;
	case Key_H:
		addHarmonic();
		break;
	case Key_R:
		addArtHarm();
		break;
	case Key_P:
		addLegato();
		break;
	case Key_M:
		curt->c[curt->x].flags ^= FLAG_PM;
		break;
	case Key_Delete:
		if (e->state()==ControlButton) {
			if (curt->c.size()>1) {
				curt->removeColumn(1);
				if (curt->x==curt->c.size())
					curt->x--;
				updateRows();
			}
		} else {
			curt->c[curt->x].a[curt->y] = -1;
			curt->c[curt->x].e[curt->y] = 0;
		}
		break;
	case Key_Insert:
		curt->insertColumn(1);
		break;
	case Key_Plus:
		if (curt->c[curt->x].l<480)
			curt->c[curt->x].l*=2;
		break;
	case Key_Minus:
		if (curt->c[curt->x].l>15)
			curt->c[curt->x].l/=2;
		break;
	case Key_A:
		curt->arrangeBars();
		emit statusBarChanged();
		updateRows();
		break;
	case Key_Period:
		curt->c[curt->x].flags ^= FLAG_DOT; // It's XOR :-)
		break;
	case Key_L:
		linkPrev();
		break;
	default:
		e->ignore();
		return;
	}
	update();
	e->accept();
}
void TrackView::arrangeBars()
{
	song->arrangeBars();
	emit statusBarChanged();
	updateRows();


}
void TrackView::mousePressEvent(QMouseEvent *e)
{
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

			emit statusBarChanged();
			found = TRUE;
			break;
		}

		lastxpos = xpos;
		xpos += xdelta;
	}
	
// 	if (curt->c[tabrow].clickrect.contains(clickpt)) {
// 		found = TRUE;
// 		curt->x = j;
// 		recttop = curt->c[tabrow].clickrect.top();
// 		for (uint i=0;i<curt->string;i++) {
// 			if ((clickpt.y() >= (recttop+(curt->string - 1 - i)*VERTLINE-VERTLINE/2)) &&
// 				(clickpt.y() < (recttop + (curt->string - i)*VERTLINE - VERTLINE/2 + 1))) {
// 				curt->y = i;
// 			}
// 		}
// 		break;
// 	}


	if (found) {
		repaint();
		
		if (e->button() == RightButton)
//			popup->exec(QCursor::pos())
			;
		else if (e->button() == LeftButton) {
			
		}
	}
}
 
