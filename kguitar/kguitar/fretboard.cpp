#include "config.h"
#include "fretboard.h"
#include "tabtrack.h"

#include <qpainter.h>
#include <qsizepolicy.h>

#define STRING_HEIGHT    24
#define FRET_DIVISOR     1.05946
#define ZERO_FRET_WIDTH  24
#define INLAY_RADIUS     5

// Inlay marks array

// ============  0  1  2  3  4  5  6  7  8  9 10 11 12
bool marks[] = { 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 2,
// ============ 13 14 15 16 17 18 19 20 21 22 23 24
                 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 2 };

Fretboard::Fretboard(TabTrack *_trk, QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	resize(600, 128);
	setTrack(_trk);
}

QSizePolicy Fretboard::sizePolicy()
{
	return QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
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

	p.setBrush(qRgb(205, 214, 221));

	// Draw frets
	for (int i = 0; i <= trk->frets; i++) {
		p.drawLine((int) fr[i], 0, (int) fr[i], height());
		// Draw inlay marks, if applicable
		if (marks[i] == 1) {
			p.drawEllipse((int) ((fr[i - 1] + fr[i]) / 2) - INLAY_RADIUS,
						  height() / 2 - INLAY_RADIUS,
						  INLAY_RADIUS * 2, INLAY_RADIUS * 2);
		} else if (marks[i] == 2) {
			p.drawEllipse((int) ((fr[i - 1] + fr[i]) / 2) - INLAY_RADIUS,
						  height() / 3 - INLAY_RADIUS,
						  INLAY_RADIUS * 2, INLAY_RADIUS * 2);
			p.drawEllipse((int) ((fr[i - 1] + fr[i]) / 2) - INLAY_RADIUS,
						  height() * 2 / 3 - INLAY_RADIUS,
						  INLAY_RADIUS * 2, INLAY_RADIUS * 2);
		}
	}

	// Draw strings
	for (int i = 0; i < trk->string; i++)
		p.drawLine(0, i * STRING_HEIGHT + STRING_HEIGHT / 2,
		           width(), i * STRING_HEIGHT + STRING_HEIGHT / 2);

	p.end();
}

void Fretboard::mousePressEvent(QMouseEvent *e)
{
	int y = trk->string - (e->y() / STRING_HEIGHT) - 1;
	int x = 0;
	if (e->x() > fr[0]) {
		for (int i = 1; i <= trk->frets; i++) {
			if (e->x() <= fr[i]) {
				x = i;
				break;
			}
		}
	}
	
	emit buttonClicked(y, x, e->button());
}

void Fretboard::resizeEvent(QResizeEvent *e)
{
	recalculateSizes();
}

// Funky fret physical sizes calculation
void Fretboard::recalculateSizes()
{
	double l = width() - ZERO_FRET_WIDTH;

	// Step 1: get fret sizes according to iterative algorithm
	for (int i = 0; i <= trk->frets; i++) {
		fr[i] = width() - l;
		l /= FRET_DIVISOR;
	}

	// Step 2: normalize total frets width to full width of widget to
	// reclaim free space (on real guitar it's strumming area there)
	l = ((double) width()) / ((double) (width() - l));
	for (int i = 0; i <= trk->frets; i++)
		fr[i] *= l;
}
