#include "trackview.h"
#include "track.h"

#include <kapp.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qpen.h>
#include <qkeycode.h>

TrackView::TrackView(QWidget *parent,const char *name): QTableView(parent,name)
{
//    setTableFlags(Tbl_autoVScrollBar | Tbl_smoothScrolling);
    setTableFlags(Tbl_autoVScrollBar | Tbl_autoHScrollBar | Tbl_smoothScrolling);
    setFrameStyle(Panel | Sunken);
    setBackgroundMode(PaletteBase);

    setNumCols(1);

    setFocusPolicy(QWidget::StrongFocus);

    song = new TabSong("Unnamed",120);
    song->t.append(new TabTrack(GuitarTab,"Guitar",1,0,25,6,24));

    curt = song->t.first();

    uchar standtune[6]={40,45,50,55,59,64};

    for (int i=0;i<6;i++)
	curt->tune[i]=standtune[i];

    curt->c.resize(1);
    curt->b.resize(1);

    for (int i=0;i<MAX_STRINGS;i++)
	curt->c[0].a[i]=-1;
    curt->c[0].l=120;

    curt->b[0].start=0;
    curt->b[0].time1=4;
    curt->b[0].time2=4;
    curt->b[0].showsig=TRUE;
    
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
    curt->c[curt->x].a[num]=fret;
}

int TrackView::finger(int num)
{
    return curt->c[curt->x].a[num];
}

void TrackView::setLength(int l)
{
    curt->c[curt->x].l=l;
    repaint();
}

void TrackView::paintCell(QPainter *p, int row, int col)
{
    uint bn = row;                      // Drawing only this bar

    int last;
    if (curt->b.size()==bn+1)     // Current bar is the last one
	last = curt->c.size()-1;        // Draw till the last note
    else                                // Else draw till the end of this bar
	last = curt->b[bn+1].start-1;

    QString tmp;

    uint s = curt->string-1;
    uint i;

    for (i=0;i<=s;i++)
	p->drawLine(0,VERTSPACE+(s-i)*VERTLINE,width(),VERTSPACE+(s-i)*VERTLINE);

    int xpos=10,xdelta;

    // Starting bars - very thick and thick one

    if (bn==0) {
	p->setBrush(SolidPattern);
	p->drawRect(0,VERTSPACE,5,VERTLINE*s);
	p->drawRect(8,VERTSPACE,2,VERTLINE*s);
	xpos+=10;
    }

    // Time signature

    if (curt->b[bn].showsig) {
	p->setFont(QFont("helvetica",18,QFont::Bold));
	tmp.setNum(curt->b[bn].time1);
	p->drawText(xpos,VERTSPACE+VERTLINE*s/3,VERTLINE,VERTLINE,
		    AlignCenter,tmp);
	tmp.setNum(curt->b[bn].time2);
	p->drawText(xpos,VERTSPACE+VERTLINE*s/3*2,VERTLINE,VERTLINE,
		    AlignCenter,tmp);
	xpos+=20;
    }

    p->setFont(QFont("helvetica",VERTLINE));
    p->setBrush(KApplication::getKApplication()->windowColor);
    
    for (int t=curt->b[bn].start;t<=last;t++) {
	// Drawing duration marks

        switch (curt->c[t].l) {
	case 15:  // 1/32
	    p->drawLine(xpos+VERTLINE/2,BOTTOMDUR+VERTLINE-4,
			xpos+VERTLINE/2+HORDUR,BOTTOMDUR+VERTLINE-4);
	case 30:  // 1/16
	    p->drawLine(xpos+VERTLINE/2,BOTTOMDUR+VERTLINE-2,
			xpos+VERTLINE/2+HORDUR,BOTTOMDUR+VERTLINE-2);
	case 60:  // 1/8
	    p->drawLine(xpos+VERTLINE/2,BOTTOMDUR+VERTLINE,
			xpos+VERTLINE/2+HORDUR,BOTTOMDUR+VERTLINE);
	case 120: // 1/4 - a long vertical line, so we need to find the highest note
	    for (i=s;((i>=0) && (curt->c[t].a[i]==-1));i--);

	    // If it's an empty measure at all - draw the vertical line from bottom
	    if (i<0)  i=0;

	    p->drawLine(xpos+VERTLINE/2,VERTSPACE+VERTLINE*(s-i)+VERTLINE/2,
			xpos+VERTLINE/2,BOTTOMDUR+VERTLINE);
	case 240: // 1/2
	    p->drawLine(xpos+VERTLINE/2,BOTTOMDUR+3,
			xpos+VERTLINE/2,BOTTOMDUR+VERTLINE);
	case 480: // whole
	    break;
	}

	// Draw the number column
	
	p->setPen(NoPen);
	for (i=0;i<=s;i++) {
	    if ((t==curt->x) && 
		(curt->y==i)) {
		p->setBrush(KApplication::getKApplication()->selectColor);
		p->drawRect(xpos,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
			    VERTLINE,VERTLINE+1);
		p->setBrush(KApplication::getKApplication()->windowColor);
		if (curt->c[t].a[i]!=-1) {
		    tmp.setNum(curt->c[t].a[i]);
		    p->setPen(KApplication::getKApplication()->selectTextColor);
		    p->drawText(xpos,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
				VERTLINE,VERTLINE,AlignCenter,tmp);
		    p->setPen(NoPen);
		}
	    } else {
		if (curt->c[t].a[i]!=-1) {
		    tmp.setNum(curt->c[t].a[i]);
		    p->drawRect(xpos,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
				VERTLINE,VERTLINE+1);
		    p->drawText(xpos,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
				VERTLINE,VERTLINE,AlignCenter,tmp);
		}
	    }
	}

	p->setPen(SolidLine);

	// Length of interval
	xdelta=(curt->c[t].l)/20*HORCELL;
	if (xdelta<HORCELL)
	    xdelta=HORCELL;
	xpos+=xdelta;
    }

    p->setBrush(SolidPattern);
}

void TrackView::resizeEvent(QResizeEvent *e)
{
    QTableView::resizeEvent(e); // GREYFIX ? Is it C++-correct?
    setCellWidth(width());
    setCellHeight(VERTSPACE*2+VERTLINE*(curt->string-1));
}

bool TrackView::moveFinger(int from, int dir)
{
    int n0=curt->c[curt->x].a[from];
    int n=n0;
    if (n<0)
	return FALSE;

    int to=from;

    do {
	to+=dir;
	if ((to<0) || (to>=curt->string))
	    return FALSE;
	n=n0+curt->tune[from]-curt->tune[to];
	if ((n<0) || (n>curt->frets))
	    return FALSE;
    } while (curt->c[curt->x].a[to]!=-1);

    curt->c[curt->x].a[from]=-1;
    curt->c[curt->x].a[to]=n;

    curt->y=to;
    return TRUE;
}

void TrackView::arrangeBars()
{
    int barlen = 480;
    int barnum = 1;
    int cbl = 0;                        // Current bar length

    curt->b[0].start=0;

    for (int i=0;i<curt->c.size();i++) {
	cbl+=curt->c[i].l;
	if (cbl>barlen) {
	    curt->b.resize(barnum+1);
	    curt->b[barnum].start = i;
	    // GREYFIX - preserve other possible time signatures
	    curt->b[barnum].time1 = curt->b[barnum-1].time1;
	    curt->b[barnum].time2 = curt->b[barnum-1].time2;

	    if ((curt->b[barnum].time1 != curt->b[barnum-1].time1) ||
		(curt->b[barnum].time2 != curt->b[barnum-1].time2))
		curt->b[barnum].showsig=TRUE;
	    else 
		curt->b[barnum].showsig=FALSE;

	    barnum++;
	    cbl-=barlen;
	}
    }
    updateRows();
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
	    if (curt->b[curt->xb].start==curt->x)
		curt->xb--;
	    curt->x--;
	}
	break;
    case Key_Right:
	if (curt->x+1==curt->c.size()) {
	    curt->c.resize(curt->c.size()+1);
	    curt->x++;
	    for (int i=0;i<curt->string;i++)
		curt->c[curt->x].a[i]=-1;
	    curt->c[curt->x].l=120;
	    updateRows();
	} else {
	    if (curt->b.size()==curt->xb+1)
		curt->x++;
	    else {
		if (curt->b[curt->xb+1].start==curt->x+1)
		    curt->xb++;
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
	if (curt->y<curt->string-1) {
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
	num=num-'0'; // GREYFIX - may be a bad thing to do

	// Allow making two-digit fret numbers pressing two keys sequentally
	if (lastnumber*10+num<=curt->frets)
	    num=lastnumber*10+num;

	curt->c[curt->x].a[curt->y]=num;
	lastnumber=num;
	break;
    case Key_Delete:
	if (e->state()==ControlButton) {
// 	    if (curt->c.count()>1) {          // GREYFIX - deletion of column
// 		curt->c.remove();
// 		updateRows();
// 	    }
	} else 
	    curt->c[curt->x].a[curt->y]=-1;
	break;
    case Key_Insert:
// 	curt->c.insert(curt->c.at(),new TabColumn());
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
	arrangeBars();
	break;
    default:
	e->ignore();
	return;
    }

    update();
    e->accept();
}
