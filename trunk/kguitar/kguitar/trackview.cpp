#include "trackview.h"
#include "track.h"

#include <kapp.h>

#include <qpainter.h>
#include <qpen.h>

TrackView::TrackView(QWidget *parent,const char *name): QTableView(parent,name)
{
    setTableFlags(Tbl_autoVScrollBar | Tbl_smoothScrolling);
    setFrameStyle(Panel | Sunken);
    setBackgroundMode(PaletteBase);

    setNumCols(1);
    setNumRows(5);

    song = new TabSong("Unnamed",120);
    song->t.append(new TabTrack(0,25,6));

    song->t.first()->c.append(new TabColumn());
    song->t.first()->c.last()->a[4]=2;
    song->t.first()->c.append(new TabColumn());
    song->t.first()->c.last()->a[3]=4;
    song->t.first()->c.append(new TabColumn());
    song->t.first()->c.last()->a[4]=2;
    song->t.first()->c.append(new TabColumn());
    song->t.first()->c.last()->a[3]=0;
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

    for (tc=song->t.first()->c.first();tc!=0;tc=song->t.first()->c.next()) {
	for (int i=0;i<song->t.first()->string();i++)
	    if (tc->a[i]!=-1) {
		tmp.setNum(tc->a[i]);
		p->drawRect(xpos,VERTSPACE+i*VERTLINE-VERTLINE/2,VERTLINE,VERTLINE);
		p->drawText(xpos,VERTSPACE+i*VERTLINE-VERTLINE/2,VERTLINE,VERTLINE,AlignCenter,tmp);
		xpos+=VERTLINE+VERTLINE/2;
	    }
    }

    p->setBrush(SolidPattern);
    p->setPen(SolidLine);
}

void TrackView::resizeEvent(QResizeEvent *e)
{
    QTableView::resizeEvent(e); // GREYFIX ? Is it C++-correct?
    setCellWidth(width());
    setCellHeight(VERTSPACE*2+VERTLINE*5); // GREYFIX hack
}
