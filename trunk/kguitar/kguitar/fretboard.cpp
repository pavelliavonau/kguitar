#include "config.h"
#include "fretboard.h"

#include <qpainter.h>

#define STRING_HEIGHT    16
#define FRET_DIVISOR     1.05946
#define ZERO_FRET_WIDTH  32

Fretboard::Fretboard(TabTrack *_trk, QWidget *parent, const char *name)
{
	resize(600, 128);
	setTrack(_trk);
}

void Fretboard::setTrack(TabTrack *_trk)
{
	trk = _trk;
	setFixedHeight(trk->string * STRING_HEIGHT);
	recalculateSizes();
}

void Fretboard::paintEvent(QPaintEvent *e)
{
	QPainter p;
	p.begin(this);
	for (int i = 0; i <= trk->frets; i++) 
		p.drawLine(fr[i], 0, fr[i], height());
	p.end();
}

void Fretboard::mousePressEvent(QMouseEvent *e)
{
}

void Fretboard::resizeEvent(QResizeEvent *e)
{
	recalculateSizes();
}

void Fretboard::recalculateSizes()
{
	double l = width() - ZERO_FRET_WIDTH;

	for (int i = 0; i < trk->frets; i++) {
		fr[i] = (int) (width() - l);
		l /= FRET_DIVISOR;
	}
	fr[trk->frets] = width();
}
