#include "trackview.h"
#include "track.h"

#include <kapp.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qpen.h>
#include <qkeycode.h>

TrackView::TrackView(QWidget *parent,const char *name): QTableView(parent,name)
{
    setTableFlags(Tbl_autoVScrollBar | Tbl_smoothScrolling);
    setFrameStyle(Panel | Sunken);
    setBackgroundMode(PaletteBase);

    setNumCols(1);
    setNumRows(5);

    setFocusPolicy(QWidget::StrongFocus);

    song = new TabSong("Unnamed",120);
    song->t.append(new TabTrack(0,25,6));

    curt = song->t.first();
    
    uchar standtune[6]={40,45,50,55,59,64};

    curt->setTuning(standtune);
//    curt->xi=QListIterator<TabColumn>(curt->c);
    curt->x=0;
    curt->y=0;

    curt->c.append(new TabColumn());
    curt->c.append(new TabColumn());
    curt->c.append(new TabColumn());
    curt->c.append(new TabColumn());
}

TrackView::~TrackView()
{
    delete song;
}

void TrackView::setFinger(int num,int fret)
{
    curt->c.at(curt->x)->a[num]=fret;
}

void TrackView::paintCell(QPainter *p, int row, int col)
{
    TabColumn *tc;
    QString tmp;

    int s = curt->string()-1;

    for (int i=0;i<curt->string();i++)
	p->drawLine(0,VERTSPACE+(s-i)*VERTLINE,width(),VERTSPACE+(s-i)*VERTLINE);

    p->setFont(QFont("helvetica",VERTLINE));
    p->setBrush(KApplication::getKApplication()->windowColor);

    int xpos=10;

    for (tc=curt->c.first();tc!=0;tc=song->t.getFirst()->c.next()) {
	p->setPen(NoPen);
	for (int i=0;i<curt->string();i++) {
	    if ((curt->c.at()==curt->x) && 
		(curt->y==i)) {
		p->setBrush(KApplication::getKApplication()->selectColor);
		p->drawRect(xpos,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
			    VERTLINE,VERTLINE);
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
				VERTLINE,VERTLINE);
		    p->drawText(xpos,VERTSPACE+(s-i)*VERTLINE-VERTLINE/2,
				VERTLINE,VERTLINE,AlignCenter,tmp);
		}
	    }
	}

	// Drawing duration marks

	p->setPen(SolidLine);
        switch (tc->l) {
	case 6: // 1/32
	    p->drawLine(xpos+VERTLINE/2,BOTTOMDUR+VERTLINE-4,
			xpos+VERTLINE/2+HORDUR,BOTTOMDUR+VERTLINE-4);
	case 5: // 1/16
	    p->drawLine(xpos+VERTLINE/2,BOTTOMDUR+VERTLINE-2,
			xpos+VERTLINE/2+HORDUR,BOTTOMDUR+VERTLINE-2);
	case 4: // 1/8
	    p->drawLine(xpos+VERTLINE/2,BOTTOMDUR+VERTLINE,
			xpos+VERTLINE/2+HORDUR,BOTTOMDUR+VERTLINE);
	case 3: // 1/4
	    p->drawLine(xpos+VERTLINE/2,BOTTOMDUR,
			xpos+VERTLINE/2,BOTTOMDUR+VERTLINE);
	case 2: // 1/2
	    p->drawLine(xpos+VERTLINE/2,BOTTOMDUR+3,
			xpos+VERTLINE/2,BOTTOMDUR+VERTLINE);
	case 1: // whole
	    break;
	}

	// Length of interval
	xpos+=(7-tc->l)*HORCELL;
    }

    p->setBrush(SolidPattern);
}

void TrackView::resizeEvent(QResizeEvent *e)
{
    QTableView::resizeEvent(e); // GREYFIX ? Is it C++-correct?
    setCellWidth(width());
    setCellHeight(VERTSPACE*2+VERTLINE*(curt->string()-1));
}

bool TrackView::moveFinger(int from, int dir)
{
    int n0=curt->c.at(curt->x)->a[from];
    int n=n0;
    if (n<0)
	return FALSE;

    int to=from;

    do {
	to+=dir;
	if ((to<0) || (to>=curt->string()))
	    return FALSE;
	n=n0+curt->tune(from)-curt->tune(to);
	if (n<0)
	    return FALSE;
    } while (curt->c.at(curt->x)->a[to]!=-1);

    curt->c.at(curt->x)->a[from]=-1;
    curt->c.at(curt->x)->a[to]=n;

    curt->y=to;
    return TRUE;
}

void TrackView::keyPressEvent(QKeyEvent *e)
{
    int num = e->ascii();

    switch (e->key()) {
    case Key_Left:
	if (curt->x>0)
	    curt->x--;
	break;
    case Key_Right:
	if (curt->x+1==curt->c.count())
	    curt->c.append(new TabColumn());
	curt->x++;
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
	if (curt->y<curt->string()-1) {
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
	curt->c.at(curt->x)->a[curt->y]=num;
	break;
    case Key_Delete:
	curt->c.at(curt->x)->a[curt->y]=-1;
	break;
    case Key_Plus:
	curt->c.at(curt->x)->l--;
	break;
    case Key_Minus:
	curt->c.at(curt->x)->l++;
	break;	
    default:
	e->ignore();
	return;
    }
    update();
    e->accept();
}
