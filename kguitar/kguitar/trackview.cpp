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

    curt->c.append(new TabColumn());
    updateRows();

    curt->y=0;
    lastnumber=0;
}

TrackView::~TrackView()
{
    delete song;
}

void TrackView::updateRows()
{
    setNumRows((curt->c.count()-1)/8+1);
}

void TrackView::setFinger(int num,int fret)
{
    curt->c.current()->a[num]=fret;
}

void TrackView::paintCell(QPainter *p, int row, int col)
{
    QListIterator<TabColumn> it(curt->c);

    int start = row*8;
    QString tmp;

    int s = curt->string-1;
    int i,cnt=0;

    for (i=0;i<start;i++)
	++it;

    for (i=0;i<=s;i++)
	p->drawLine(0,VERTSPACE+(s-i)*VERTLINE,width(),VERTSPACE+(s-i)*VERTLINE);

    p->setFont(QFont("helvetica",VERTLINE));
    p->setBrush(KApplication::getKApplication()->windowColor);

    int xpos=10,xdelta;
    
    for (;(it.current()) && (cnt<8);++it) {
	TabColumn *tc = it.current();

	// Drawing duration marks

        switch (tc->l) {
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
	    for (i=s;((i>=0) && (tc->a[i]==-1));i--);

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
	    if ((curt->c.current()==tc) && 
		(curt->y==i)) {
		p->setBrush(KApplication::getKApplication()->selectColor);
		p->drawRect(xpos,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
			    VERTLINE,VERTLINE+1);
		p->setBrush(KApplication::getKApplication()->windowColor);
		if (tc->a[i]!=-1) {
		    tmp.setNum(tc->a[i]);
		    p->setPen(KApplication::getKApplication()->selectTextColor);
		    p->drawText(xpos,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
				VERTLINE,VERTLINE,AlignCenter,tmp);
		    p->setPen(NoPen);
		}
	    } else {
		if (tc->a[i]!=-1) {
		    tmp.setNum(tc->a[i]);
		    p->drawRect(xpos,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
				VERTLINE,VERTLINE+1);
		    p->drawText(xpos,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
				VERTLINE,VERTLINE,AlignCenter,tmp);
		}
	    }
	}

	p->setPen(SolidLine);

	// Length of interval
	xdelta=(tc->l)/20*HORCELL;
	if (xdelta<HORCELL)
	    xdelta=HORCELL;
	xpos+=xdelta;
	cnt++;
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
    int n0=curt->c.current()->a[from];
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
    } while (curt->c.current()->a[to]!=-1);

    curt->c.current()->a[from]=-1;
    curt->c.current()->a[to]=n;

    curt->y=to;
    return TRUE;
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
	if (curt->c.at()>0)
	    curt->c.prev();
	break;
    case Key_Right:
	if (curt->c.at()+1==curt->c.count()) {
	    curt->c.append(new TabColumn());
	    updateRows();
	} else
	    curt->c.next();
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

	curt->c.current()->a[curt->y]=num;
	lastnumber=num;
	break;
    case Key_Delete:
	if (e->state()==ControlButton) {
	    if (curt->c.count()>1) {
		curt->c.remove();
		updateRows();
	    }
	} else 
	    curt->c.current()->a[curt->y]=-1;
	break;
    case Key_Insert:
	curt->c.insert(curt->c.at(),new TabColumn());
	break;
    case Key_Plus:
	if (curt->c.current()->l<480)
	    curt->c.current()->l*=2;
	break;
    case Key_Minus:
	if (curt->c.current()->l>15)
	    curt->c.current()->l/=2;
	break;	
    default:
	e->ignore();
	return;
    }

    update();
    e->accept();
}
