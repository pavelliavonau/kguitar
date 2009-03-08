#include "fingers.h"
#include "tabtrack.h"
#include "settings.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qbrush.h>
#include <qstring.h>
#include <qscrollbar.h>

Fingering::Fingering(TabTrack *p, QWidget *parent, const char *name):
    QFrame(parent, name)
{
    parm = p;

    lastff = 1;

    setFixedSize(parm->string * SCALE + 2 * BORDER + FRETTEXT + SCROLLER,
	             NUMFRETS * SCALE + SCALE + 2 * BORDER + 2 * SPACER + NOTES);
    setFrameStyle(Panel | Sunken);
    setBackgroundMode(PaletteBase);

    ff = new QScrollBar(1, parm->frets-NUMFRETS + 1, 1, 5, 1,
	                    QScrollBar::Vertical, this);
    ff->setGeometry(width() - SCROLLER, 0, SCROLLER, height());
    connect(ff, SIGNAL(valueChanged(int)), SLOT(setFirstFret(int)));

    clear();
}

void Fingering::clear()
{
    for (int i = 0; i < parm->string; i++)
		appl[i] = -1;
    repaint();
    emit chordChange();
}

void Fingering::setFinger(int string, int fret)
{
    if (appl[string] != fret) {
        appl[string] = fret;
        repaint();
        emit chordChange();
    }
}

void Fingering::setFingering(const int a[MAX_STRINGS])
{
    int f = 24; // GREYFIX
    bool noff = TRUE;

    for (int i = 0; i < parm->string; i++) {
        if ((a[i] < f) && (a[i] > 0))
            f = a[i];
        if (a[i] > 5)
            noff = FALSE;
    }

    if (noff)
        f = 1;

    ff->setValue(f);

    for (int i = 0;i < MAX_STRINGS; i++)
        appl[i] = a[i];
    repaint();
    emit chordChange();
}

void Fingering::setFirstFret(int fret)
{
    for (int i = 0; i < parm->string; i++)
	if (appl[i] > 0)
	    appl[i] = appl[i] - lastff + fret;

    lastff = fret;

    repaint();
    emit chordChange();
}

void Fingering::mouseHandle(const QPoint &pos, bool domute)
{
    int i = (pos.x() - BORDER - FRETTEXT) / SCALE;
    int j = 0;
    if (pos.y() > BORDER + SCALE + 2 * SPACER)
        j = (pos.y() - BORDER - SCALE - 2 * SPACER) / SCALE + ff->value();

    if ((domute) && (appl[i] == j))
        j = -1;

    if (!((i < 0) || (i >= parm->string) || (j >= ff->value() + NUMFRETS)))
        setFinger(i, j);
}

void Fingering::mouseMoveEvent(QMouseEvent *e)
{
    mouseHandle(e->pos(), FALSE);
}

void Fingering::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == LeftButton)
	mouseHandle(e->pos(), TRUE);
}

void Fingering::drawContents(QPainter *p)
{
	int barre, eff;

	// Horizontal separator line

	p->drawLine(BORDER + FRETTEXT, BORDER + SCALE + SPACER,
	            BORDER + FRETTEXT + parm->string * SCALE, BORDER + SCALE + SPACER);

	// Horizontal lines

	for (int i = 0; i <= NUMFRETS; i++)
		p->drawLine(SCALE / 2 + BORDER + FRETTEXT, BORDER + SCALE + 2 * SPACER + i * SCALE,
		            SCALE / 2 + BORDER + parm->string * SCALE - SCALE + FRETTEXT,
		            BORDER +SCALE + 2 * SPACER + i * SCALE);

	// Beginning fret number

	QString tmp;
	tmp.setNum(ff->value());
	p->drawText(BORDER - SPACER, BORDER + SCALE + 2 * SPACER, 50, 50, AlignLeft | AlignTop, tmp);

	// Vertical lines, fingering and note names

	for (int i = 0; i < parm->string; i++) {
		p->drawLine( i * SCALE + BORDER + SCALE / 2 + FRETTEXT, BORDER + SCALE + 2 * SPACER,
		         i * SCALE + BORDER + SCALE / 2 + FRETTEXT,
		         BORDER + SCALE + 2 * SPACER + NUMFRETS *SCALE);
		if (appl[i] == -1) {
			p->drawLine(i * SCALE + BORDER + CIRCBORD + FRETTEXT, BORDER + CIRCBORD,
			            i * SCALE + BORDER + SCALE - CIRCBORD + FRETTEXT, BORDER + SCALE - CIRCBORD);
			p->drawLine(i * SCALE + BORDER + SCALE - CIRCBORD + FRETTEXT, BORDER +CIRCBORD,
			            i * SCALE + BORDER + CIRCBORD + FRETTEXT, BORDER + SCALE - CIRCBORD);
		} else {
			if (appl[i] == 0) {
				p->setBrush(NoBrush);
				p->drawEllipse(i * SCALE + BORDER + CIRCBORD + FRETTEXT,
				               BORDER + CIRCBORD, CIRCLE, CIRCLE);
			} else {
				p->setBrush(SolidPattern);
				p->drawEllipse(i * SCALE + BORDER + CIRCBORD + FRETTEXT,
				               BORDER + SCALE + 2 * SPACER + (appl[i] - ff->value()) * SCALE + CIRCBORD,
				               CIRCLE, CIRCLE);
			};
			p->drawText(BORDER + FRETTEXT + i * SCALE, BORDER + NUMFRETS * SCALE + SCALE + 2 * SPACER,
			            SCALE, 30, AlignHCenter | AlignTop,
			            Settings::noteName((appl[i] + parm->tune[i]) % 12));
		}
	}

	// Analyze & draw barre

	p->setBrush(SolidPattern);

	for (int i = 0; i < NUMFRETS; i++) {
		barre = 0;
		while ((appl[parm->string - barre - 1] >= (i + ff->value())) ||
		       (appl[parm->string - barre - 1] == -1)) {
			barre++;
			if (barre>parm->string - 1)
				break;
		}

		while ((appl[parm->string - barre] != (i + ff->value())) && (barre > 1))
			barre--;

		eff = 0;
		for (int j = parm->string - barre; j < parm->string; j++) {
			if (appl[j] != -1)
				eff++;
		}

		if (eff > 2) {
			p->drawRect((parm->string - barre) * SCALE + SCALE / 2 + BORDER + FRETTEXT,
			            BORDER + SCALE + 2 * SPACER + i * SCALE + CIRCBORD,
			            (barre - 1) * SCALE, CIRCLE);
		}
	}
}
