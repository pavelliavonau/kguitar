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
    
    int standtune[6]={40,45,50,55,59,64};

    curt->setTuning(standtune);
    curt->x=0;
    curt->y=4;

    curt->c.append(new TabColumn());
    curt->c.last()->a[4]=2;
    curt->c.append(new TabColumn());
    curt->c.last()->a[3]=4;
    curt->c.append(new TabColumn());
    curt->c.last()->a[4]=2;
    curt->c.append(new TabColumn());
    curt->c.last()->a[3]=0;
}

TrackView::~TrackView()
{
    delete song;
}

void TrackView::paintCell(QPainter *p, int row, int col)
{
    for (int i=0;i<6;i++)
	p->drawLine(0,VERTSPACE+i*VERTLINE,width(),VERTSPACE+i*VERTLINE);
    
    TabColumn *tc;
    QString tmp;

    p->setFont(QFont("helvetica",VERTLINE));
    p->setBrush(KApplication::getKApplication()->windowColor);
    p->setPen(NoPen);

    int xpos=10;

    for (tc=curt->c.first();tc!=0;tc=song->t.getFirst()->c.next()) {
	for (int i=0;i<curt->string();i++) {
	    if ((curt->c.at()==curt->x) && 
		(curt->y==i)) {
		p->setBrush(KApplication::getKApplication()->selectColor);
		p->drawRect(xpos,VERTSPACE+i*VERTLINE-VERTLINE/2,VERTLINE,VERTLINE);
		p->setBrush(KApplication::getKApplication()->windowColor);
		if (tc->a[i]!=-1) {
		    tmp.setNum(tc->a[i]);
		    p->setPen(KApplication::getKApplication()->selectTextColor);
		    p->drawText(xpos,VERTSPACE+i*VERTLINE-VERTLINE/2,VERTLINE,VERTLINE,AlignCenter,tmp);
		    p->setPen(NoPen);
		}
	    } else {
		if (tc->a[i]!=-1) {
		    tmp.setNum(tc->a[i]);
		    p->drawRect(xpos,VERTSPACE+i*VERTLINE-VERTLINE/2,VERTLINE,VERTLINE);
		    p->drawText(xpos,VERTSPACE+i*VERTLINE-VERTLINE/2,VERTLINE,VERTLINE,AlignCenter,tmp);
		}
	    }
	}
	xpos+=VERTLINE+VERTLINE/2;
    }

    p->setBrush(SolidPattern);
    p->setPen(SolidLine);
}

void TrackView::resizeEvent(QResizeEvent *e)
{
    QTableView::resizeEvent(e); // GREYFIX ? Is it C++-correct?
    setCellWidth(width());
    setCellHeight(VERTSPACE*2+VERTLINE*(curt->string()-1));
}

void TrackView::keyPressEvent(QKeyEvent *e)
{
    int num = e->ascii();

    switch (e->key()) {
    case Key_Left:  curt->x--;break;
    case Key_Right: curt->x++;break;
    case Key_Up:
	if (curt->y>0)
	    curt->y--;
	break;
    case Key_Down:
	if (curt->y<curt->string()-1)
	    curt->y++;
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
    default:
	e->ignore();
	return;
    }
    update();
    e->accept();
}
