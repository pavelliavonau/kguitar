#include "fingers.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qcheckbox.h>
#include <qevent.h>
#include <qbrush.h>
#include <qstring.h>

Fingering::Fingering(int strings,QWidget *parent,const char *name): QFrame(parent,name)
{
  firstFret = 1;
  numstr = strings;
  
  setFixedSize(numstr*SCALE+2*BORDER+FRETTEXT,NUMFRETS*SCALE+SCALE+2*BORDER+2*SPACER);
  setFrameStyle(Panel | Sunken);
  setBackgroundMode(PaletteBase);
  
  clear();
}

void Fingering::clear()
{
  for (int i=0;i<numstr;i++) { appl[i]=0; };
  repaint();
  emit chordChange();
}

void Fingering::setFinger(int string, int fret)
{
  if (appl[string]!=fret) {
    appl[string]=fret;
    repaint();
    emit chordChange();
  }
}

void Fingering::setFirstFret(int fret)
{
  for (int i=0;i<numstr;i++)
    if (appl[i]>0)
      appl[i]=appl[i]-firstFret+fret;

  firstFret=fret;
  repaint();
  emit chordChange();
}

void Fingering::mouseHandle(const QPoint &pos,bool domute)
{
  int i=(pos.x()-BORDER-FRETTEXT)/SCALE;
  int j=0;
  if (pos.y()>BORDER+SCALE+2*SPACER) {
    j=(pos.y()-BORDER-SCALE-2*SPACER)/SCALE+firstFret;
  }

  if ((domute) && (appl[i]==j))
    j=-1;

  if (!((i<0) || (i>=numstr) || (j>=firstFret+NUMFRETS)))
    setFinger(i,j); 
}

void Fingering::mouseMoveEvent( QMouseEvent *e )
{
  mouseHandle(e->pos(),FALSE);
}

void Fingering::mousePressEvent( QMouseEvent *e )
{
  if (e->button()==LeftButton)
    mouseHandle(e->pos(),TRUE);
}

void Fingering::drawContents(QPainter *p)
{
  int barre,eff;
  QString fs;
  fs.setNum(firstFret);

  // Horizontal separator line

  p->drawLine(BORDER+FRETTEXT,BORDER+SCALE+SPACER,BORDER+FRETTEXT+numstr*SCALE,BORDER+SCALE+SPACER);
  
  // Horizontal lines

  for (int i=0;i<=NUMFRETS;i++) {
    p->drawLine(SCALE/2+BORDER+FRETTEXT,BORDER+SCALE+2*SPACER+i*SCALE,
		SCALE/2+BORDER+numstr*SCALE-SCALE+FRETTEXT,BORDER+SCALE+2*SPACER+i*SCALE);
  }

  // Beginning fret number

  p->drawText(0,BORDER+2*SPACER,fs);

  // Vertical lines and fingering

  for (int i=0;i<numstr;i++) {
    p->drawLine(i*SCALE+BORDER+SCALE/2+FRETTEXT,BORDER+SCALE+2*SPACER,
		i*SCALE+BORDER+SCALE/2+FRETTEXT,BORDER+SCALE+2*SPACER+NUMFRETS*SCALE);
    if (appl[i]==-1) {
      p->drawLine(i*SCALE+BORDER+CIRCBORD+FRETTEXT,BORDER+CIRCBORD,
		  i*SCALE+BORDER+SCALE-CIRCBORD+FRETTEXT,BORDER+SCALE-CIRCBORD);
      p->drawLine(i*SCALE+BORDER+SCALE-CIRCBORD+FRETTEXT,BORDER+CIRCBORD,
		  i*SCALE+BORDER+CIRCBORD+FRETTEXT,BORDER+SCALE-CIRCBORD);
    } else if (appl[i]==0) {
      p->setBrush(NoBrush);
      p->drawEllipse(i*SCALE+BORDER+CIRCBORD+FRETTEXT,BORDER+CIRCBORD,CIRCLE,CIRCLE);
    } else {
      p->setBrush(SolidPattern);
      p->drawEllipse(i*SCALE+BORDER+CIRCBORD+FRETTEXT,BORDER+SCALE+2*SPACER+(appl[i]-firstFret)*SCALE+
		     CIRCBORD,CIRCLE,CIRCLE);
    };
  }

  // Analyze & draw barre

  p->setBrush(SolidPattern);

  for (int i=0;i<NUMFRETS;i++) {
    barre=0;
    while ((appl[numstr-barre-1]>=(i+firstFret)) || (appl[numstr-barre-1]==-1)) {
      barre++;
      if (barre>numstr-1)
	break;
    }

    while ((appl[numstr-barre]!=(i+firstFret)) && (barre>1)) {
      barre--;
    }

    eff=0;
    for (int j=numstr-barre;j<numstr;j++) {
      if (appl[j]!=-1)
	eff++;
    }

    if (eff>2) {
      p->drawRect((numstr-barre)*SCALE+SCALE/2+BORDER+FRETTEXT,BORDER+SCALE+2*SPACER+i*SCALE+CIRCBORD,
		  (barre-1)*SCALE,CIRCLE);
    }
  }
}

// void Fingering::paintEvent( QPaintEvent * e )
// {
//     int starti = pos2index( e->rect().left() );
//     int stopi  = pos2index( e->rect().right() );
//     int startj = pos2index( e->rect().top() );
//     int stopj  = pos2index( e->rect().bottom() );

//     if (stopi > maxi)
//         stopi = maxi;
//     if (stopj > maxj)
//         stopj = maxj;

//     QPainter paint( this );

//     for ( int i = starti; i <= stopi; i++ ) {
//         for ( int j = startj; j <= stopj; j++ ) {
//             if ( cells[current][i][j] )
//                 // could avoid this if cells[!current][i][j] and this
//                 // is a repaint(FALSE)... probably not worth it
//                 qDrawShadePanel( &paint, index2pos( i ), index2pos( j ),
//                                  SCALE - 1, SCALE - 1, colorGroup() );
//             else if ( cells[!current][i][j] )
//                 paint.eraseRect( index2pos( i ), index2pos( j ),
//                                  SCALE - 1, SCALE - 1);
//         }
//     }

//     drawFrame( &paint );
// }
