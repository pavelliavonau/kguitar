#include "fingerlist.h"
#include "global.h"
#include "tabtrack.h"

#include <qpainter.h>
#include <qcolor.h>

#include <kglobalsettings.h>

FingerList::FingerList(TabTrack *p, QWidget *parent, const char *name)
	: QTableView(parent, name)
{
    parm = p;

    setTableFlags(Tbl_autoVScrollBar | Tbl_smoothScrolling);
    setFrameStyle(Panel | Sunken);
    setBackgroundMode(PaletteBase);
    setFocusPolicy(StrongFocus);
    num = 0; curSel = -1; oldCol = 0; oldRow = 0;

    setCellWidth(ICONCHORD);
    setCellHeight(ICONCHORD);

	setMinimumSize(ICONCHORD + 2, ICONCHORD + 2);

    repaint();
}

void FingerList::clear()
{
    appl.resize(0);
    num = 0; curSel = -1;
    oldCol = 0; oldRow = 0;
}

void FingerList::switchAuto(bool update)
{
    setAutoUpdate(update);
}

void FingerList::addFingering(const int a[MAX_STRINGS], bool update)
{
    appl.resize((num + 1) * MAX_STRINGS);

    for (int i = 0; i < MAX_STRINGS; i++)
		appl[num].f[i] = a[i];

    num++;

    // num is overral number of chord fingerings. If it's 0 - then there are no
    // fingerings. In the appl array, indexes should be ranged from 0 to (num-1)
    setNumRows((num - 1) / perRow + 1);
    // GREYFIX: it's wrong to update number of rows every added fingering. It
    // should be updated only in the end of add session (e.g. chord calucation)
}

void FingerList::resizeEvent(QResizeEvent *e)
{
    perRow = width() / ICONCHORD;
    setNumCols(perRow);
    setNumRows((num - 1) / perRow + 1);
}

void FingerList::mousePressEvent(QMouseEvent *e)
{
    int col = e->x() / ICONCHORD;
    int row = (e->y() + yOffset()) / ICONCHORD;
    
    int n = row * perRow + col;
    
    if ((n >= 0) && (n < num)) {
		curSel = row * perRow + col;
		repaint(oldCol * ICONCHORD, oldRow * ICONCHORD - yOffset(),
				ICONCHORD, ICONCHORD);
		repaint(col * ICONCHORD, row * ICONCHORD - yOffset(),
				ICONCHORD, ICONCHORD);
		oldCol = col;
		oldRow = row;
		emit chordSelected(appl[curSel].f);
    }
}

void FingerList::paintCell(QPainter *p, int row, int col)
{
    int n = row*perRow+col;
    
    if (n<num) {
		int barre, eff;
		QColor back = KGlobalSettings::baseColor();
		QColor fore = KGlobalSettings::textColor();
		
		// Selection painting
		
		if (curSel == n) {
			back = KGlobalSettings::highlightColor();
			fore = KGlobalSettings::highlightedTextColor();
			
			p->setBrush(back);
			p->setPen(NoPen);
			p->drawRect(0,0,ICONCHORD-1,ICONCHORD-1);
			
			if (hasFocus()) {
				p->setBrush(NoBrush);
				p->setPen(fore);
				// GREYFIX - assumes only 2 styles - fix as to KDE2!
// 				if (KGlobalSettings::applicationStyle == WindowsStyle)
// 					p->drawWinFocusRect(0,0,ICONCHORD-1,ICONCHORD-1);
// 				else
					p->drawRect(1,1,ICONCHORD-3,ICONCHORD-3);
			}
		}
		
		p->setPen(fore);
		
		// Horizontal lines
		
		for (int i = 0; i <= NUMFRETS; i++)
			p->drawLine(SCALE/2+BORDER+FRETTEXT,BORDER+SCALE+2*SPACER+i*SCALE,
						SCALE/2+BORDER+parm->string*SCALE-SCALE+FRETTEXT,
						BORDER+SCALE+2*SPACER+i*SCALE);
		
		// Beginning fret number
		
		int firstFret = parm->frets;
		bool noff = TRUE;
		
		for (int i = 0; i < parm->string; i++) {
			if ((appl[n].f[i] < firstFret) && (appl[n].f[i] > 0))
				firstFret = appl[n].f[i];
			if (appl[n].f[i] > 5)
				noff = FALSE;
		}
		
		if (noff)
			firstFret = 1;
		
		if (firstFret > 1) {
			QString fs;
			fs.setNum(firstFret);
			p->drawText(BORDER, BORDER + SCALE + 2 * SPACER, 50, 50,
						AlignLeft | AlignTop, fs);
		}
		
		// Vertical lines and fingering
		
		for (int i = 0; i < parm->string; i++) {
			p->drawLine(i * SCALE + BORDER + SCALE / 2 + FRETTEXT,
						BORDER + SCALE + 2 * SPACER,
						i * SCALE + BORDER + SCALE / 2 + FRETTEXT,
						BORDER + SCALE + 2 * SPACER + NUMFRETS * SCALE);
			if (appl[n].f[i] == -1) {
				p->drawLine(i*SCALE+BORDER+CIRCBORD+FRETTEXT,BORDER+CIRCBORD,
							i*SCALE+BORDER+SCALE-CIRCBORD+FRETTEXT,
							BORDER+SCALE-CIRCBORD);
				p->drawLine(i*SCALE+BORDER+SCALE-CIRCBORD+FRETTEXT,BORDER+CIRCBORD,
							i*SCALE+BORDER+CIRCBORD+FRETTEXT,BORDER+SCALE-CIRCBORD);
			} else if (appl[n].f[i]==0) {
				p->setBrush(back);
				p->drawEllipse(i*SCALE+BORDER+CIRCBORD+FRETTEXT,BORDER+CIRCBORD,
							   CIRCLE,CIRCLE);
			} else {
				p->setBrush(fore);
				p->drawEllipse(i*SCALE+BORDER+CIRCBORD+FRETTEXT,
							   BORDER+SCALE+2*SPACER+(appl[n].f[i]-firstFret)*SCALE+
							   CIRCBORD,CIRCLE,CIRCLE);
			}
		}
		
		// Analyze & draw barre
		
		p->setBrush(fore);
		
		for (int i=0;i<NUMFRETS;i++) {
			barre=0;
			while ((appl[n].f[parm->string-barre-1]>=(i+firstFret)) ||
				   (appl[n].f[parm->string-barre-1]==-1)) {
				barre++;
				if (barre>parm->string-1)
					break;
			}
			
			while ((appl[n].f[parm->string-barre]!=(i+firstFret)) && (barre>1))
				barre--;
			
			eff = 0;
			for (int j = parm->string-barre; j < parm->string; j++) {
				if (appl[n].f[j] != -1)
					eff++;
			}
			
			if (eff > 2) {
				p->drawRect((parm->string-barre) * SCALE + SCALE / 2 +
							BORDER + FRETTEXT,
							BORDER + SCALE + 2 * SPACER + i * SCALE + CIRCBORD,
							(barre - 1) * SCALE, CIRCLE);
			}
		}  
		
		p->setBrush(NoBrush);
		p->setPen(SolidLine);
    }
}
