#include "config.h"
#include "fretboard.h"
#include "data/tabtrack.h"
#include "settings.h"
#include "data/tabsong.h"

#include <QPainter>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QAbstractItemView>

#define FRET_DIVISOR     1.05946

#define INLAY_FILL_COLOR 205, 214, 221
// #define FRET_COLOR_1     144, 151, 166
// #define FRET_COLOR_2      77,  84,  99
#define STRING_COLOR_1   230, 230, 230
#define STRING_COLOR_2   166, 166, 166

#define FINGER_COLOR     44,  77, 240
#define SCALE_COLOR      239, 207, 0, 128

// Inlay marks array

// ===========  0  1  2  3  4  5  6  7  8  9 10 11 12
static int marks[] = { 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 2,
// =========== 13 14 15 16 17 18 19 20 21 22 23 24
                0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 2 };


static int steptemplate[][12] = {
// == C  C# D  D# E  F  F# G  G# A  A# B
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Nothing
	{ 1, 0, 2, 0, 3, 0, 0, 5, 0, 0, 0, 7 }, // C Pentatonic
	{ 1, 0, 2, 0, 3, 4, 0, 5, 0, 6, 0, 7 }, // C Natural Major
	{ 1, 0, 2, 3, 0, 4, 0, 5, 6, 0, 7, 0 }, // C Natural Minor
	{ 1, 0, 2, 0, 3, 4, 0, 5, 6, 0, 0, 7 }, // C Harmonic Major (major 6-)
	{ 1, 0, 2, 3, 0, 4, 0, 5, 6, 0, 0, 7 }, // C Harmonic Minor (minor 7+)
	{ 1, 0, 2, 0, 3, 4, 0, 5, 0, 6, 0, 7 }, // C Melodic Major
	{ 1, 0, 2, 0, 3, 4, 0, 5, 0, 6, 0, 7 }, // C Melodic Minor // GREYFIX
	{ 1, 0, 2, 0, 3, 4, 0, 5, 0, 6, 7, 0 }, // C Mixolydian     (major 7-)
	{ 1, 0, 2, 0, 3, 0, 4, 5, 0, 6, 0, 7 }, // C Lydian         (major 4+)
	{ 1, 0, 2, 3, 0, 4, 0, 5, 0, 6, 7, 0 }, // C Dorian         (minor 6+)
	{ 1, 2, 0, 3, 0, 4, 0, 5, 6, 0, 7, 0 }, // C Phrygian       (minor 2-)
	{ 1, 0, 2, 0, 3, 4, 0, 5, 0, 6, 0, 7 }, // C Locrian       // GREYFIX
};

Fretboard::Fretboard(QAbstractItemView *tv_, QWidget *parent)
	: QWidget(parent)
	, tonic(0)
	, mode(0)
{
	tv = tv_;

	scaleback = NULL;
	back = NULL;
	wood = new QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kguitar/pics/rosewood.jpg"));
	fret = new QImage(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kguitar/pics/fret.png"));
	zeroFret = new QImage(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kguitar/pics/zerofret.png"));

	setFocusPolicy(Qt::WheelFocus); // the strongest focus gainer
	setAutoFillBackground(true);
}

Fretboard::~Fretboard()
{
	delete scaleback;
	delete back;
	delete wood;
	delete fret;
	delete zeroFret;
}

QSizePolicy Fretboard::sizePolicy()
{
	return QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
}

void Fretboard::currentBarChangedSlot(QModelIndex current, QModelIndex)
{
	if( !current.isValid() )
		return;

	if(trk()->trackMode() != TabTrack::FretTab) {
		setFixedHeight(0);
		return;
	}

	setFixedHeight(trk()->string * STRING_HEIGHT);
}

void Fretboard::paintEvent(QPaintEvent *)
{
	if(trk()->trackMode() != TabTrack::FretTab)
		return;

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.setBrush(QBrush(QColor(FINGER_COLOR)));
	int y = height() - STRING_HEIGHT / 2 - FINGER_RADIUS;
	for (int i = 0; i < trk()->string; i++) {
		int a = trk()->c[trk()->x].a[i];
		if ((a >= 0) && (a <= trk()->frets)) {
			int x = (a == 0) ? (int) fr[0] / 2 : (int) (fr[a] + fr[a - 1]) / 2;
			p.drawEllipse(x - FINGER_RADIUS, y, FINGER_RADIUS * 2, FINGER_RADIUS * 2);
		}
		y -= STRING_HEIGHT;
	}
}

void Fretboard::mousePressEvent(QMouseEvent *e)
{
	handleMouse(e);
}

void Fretboard::mouseMoveEvent(QMouseEvent *e)
{
	handleMouse(e);
}

void Fretboard::handleMouse(QMouseEvent *e)
{
	if(trk()->trackMode() != TabTrack::FretTab)
		return;

	int y = trk()->string - (e->y() / STRING_HEIGHT) - 1;
	int x = 0;
	if (e->x() > fr[0]) {
		for (int i = 1; i <= trk()->frets; i++) {
			if (e->x() <= fr[i]) {
				x = i;
				break;
			}
		}
	}
	emit buttonPress(y, x, e->button());
}

void Fretboard::mouseReleaseEvent(QMouseEvent *e)
{
	emit buttonRelease(e->button());
}

void Fretboard::resizeEvent(QResizeEvent *)
{
	recalculateSizes();
	drawBackground();
}

// Funky fret physical sizes calculation
void Fretboard::recalculateSizes()
{
	if(trk()->trackMode() != TabTrack::FretTab)
		return;

	double l = width() - ZERO_FRET_WIDTH;

	// Step 1: get fret sizes according to iterative algorithm
	for (int i = 0; i <= trk()->frets; i++) {
		fr[i] = width() - l;
		l /= FRET_DIVISOR;
	}

	// Step 2: normalize total frets width to full width of widget to
	// reclaim free space (on real guitar it's strumming area there)
	l = ((double) width()) / ((double) (width() - l));
	for (int i = 0; i <= trk()->frets; i++)
		fr[i] *= l;
}

// Draw background according to new widget sizes
void Fretboard::drawBackground()
{
	if(trk()->trackMode() != TabTrack::FretTab)
		return;

	qDebug() << "drawBackground - start";

	if (back != NULL)
		delete back;
	back = new QPixmap(width(), height());
	QPainter p(back);
	p.drawTiledPixmap(0, 0, width(), height(), *wood);
	p.setRenderHint(QPainter::Antialiasing);

	QImage scaledFret = fret->scaled(fret->width(), height());
	p.drawImage(0, 0, zeroFret->scaled(ZERO_FRET_WIDTH, height()));

	p.setBrush(QBrush(QColor(INLAY_FILL_COLOR)));

	// Draw frets
	for (int i = 1; i <= trk()->frets; i++) {
// 		p.setPen(QColor(FRET_COLOR_1));
// 		p.drawLine((int) fr[i], 0, (int) fr[i], height());
// 		p.setPen(QColor(FRET_COLOR_2));
// 		p.drawLine((int) fr[i] - 1, 0, (int) fr[i] - 1, height());
// 		p.drawLine((int) fr[i] + 1, 0, (int) fr[i] + 1, height());
		// Draw frets
		p.drawImage((int) fr[i] - 1, 0, scaledFret);
		// Draw inlay marks, if applicable
		if (marks[i] == 0)
			continue;
		switch (Settings::melodyEditorInlay())  {
		case 0: // none
			break;
		case 1: // center dots
			if (marks[i] == 1) {
				p.drawEllipse((int) ((fr[i - 1] + fr[i]) / 2) - INLAY_RADIUS,
				              height() / 2 - INLAY_RADIUS,
				              INLAY_RADIUS * 2, INLAY_RADIUS * 2);
			} else {
				p.drawEllipse((int) ((fr[i - 1] + fr[i]) / 2) - INLAY_RADIUS,
				              height() / 3 - INLAY_RADIUS,
				              INLAY_RADIUS * 2, INLAY_RADIUS * 2);
				p.drawEllipse((int) ((fr[i - 1] + fr[i]) / 2) - INLAY_RADIUS,
				              height() * 2 / 3 - INLAY_RADIUS,
				              INLAY_RADIUS * 2, INLAY_RADIUS * 2);
			}
			break;
		case 2: // side dots
			if (marks[i] == 1) {
				p.drawEllipse((int) ((fr[i - 1] + fr[i]) / 2) - INLAY_RADIUS,
				              height() - 2 * INLAY_RADIUS - SIDE_BORDER,
				              INLAY_RADIUS * 2, INLAY_RADIUS * 2);
			} else {
				p.drawEllipse((int) ((fr[i - 1] + fr[i]) / 2) - INLAY_RADIUS,
				              height() - 2 * INLAY_RADIUS - SIDE_BORDER,
				              INLAY_RADIUS * 2, INLAY_RADIUS * 2);
				p.drawEllipse((int) ((fr[i - 1] + fr[i]) / 2) - INLAY_RADIUS,
				              height() - 4 * INLAY_RADIUS - 2 * SIDE_BORDER,
				              INLAY_RADIUS * 2, INLAY_RADIUS * 2);
			}
			break;
		case 3: // blocks
			{
				int h = height() * ((marks[i] == 1) ? 7 : 9) / 10;
				p.drawRect((int) ((4 * fr[i - 1] + fr[i]) / 5),
				           (height() - h) / 2, (int) (3 * (fr[i] - fr[i - 1]) / 5), h);
			}
			break;
		case 4: // trapezoids
			{
				QPolygon ar(4);
				int h1, h2;
				if (marks[i] == 1) {
					h1 = height() * 2 / 3;
					h2 = height() * 7 / 10;
				} else {
					h1 = height() * 8 / 10;
					h2 = height() * 9 / 10;
				}
				int x1 = (int) (1 * (fr[i] - fr[i - 1]) / 5 + fr[i - 1]);
				int x2 = (int) (4 * (fr[i] - fr[i - 1]) / 5 + fr[i - 1]);
				ar.putPoints(0, 4, x1, h1, x2, h2, x2, height() - h2, x1, height() - h1);
				p.drawPolygon(ar);
			}
			break;
		case 5: // shark fins
			{
				QPolygon ar(3);
				int x1 = (int) (1 * (fr[i] - fr[i - 1]) / 8 + fr[i - 1]);
				int x2 = (int) (7 * (fr[i] - fr[i - 1]) / 8 + fr[i - 1]);
				ar.putPoints(0, 3, x1, height() / 8, x2, height() / 8, x1, height() * 7 / 8);
				p.drawPolygon(ar);
			}
			break;
		}
	}

	// Draw strings
	for (int i = 0; i < trk()->string; i++) {
		int y = i * STRING_HEIGHT + STRING_HEIGHT / 2;
		p.setPen(QColor(STRING_COLOR_1));
		p.drawLine(0, y, width(), y);
		p.setPen(QColor(STRING_COLOR_2));
		p.drawLine(0, y - 1, width(), y - 1);
		p.drawLine(0, y + 1, width(), y + 1);
	}

	qDebug() << "drawBackground - end";
	drawScaleBack();
}

void Fretboard::drawScaleBack()
{
	qDebug() << "drawScaleBack - start";

	if (scaleback != NULL)
		delete scaleback;

	scaleback = new QPixmap(width(), height());
	QPainter p(scaleback);

	p.drawPixmap(0, 0, *back);
	p.setRenderHint(QPainter::Antialiasing);

	// Calculate mode scale steps

	// Array by all possible notes, 0 = not used, 1 = root, 2 = second, etc
	int step[12];
	int now; // pointer for current value in step[] array

	now = tonic;
	for (int i = 0; i < 12; i++) {
		step[now] = steptemplate[mode][i];
		now = (now + 1) % 12;
	}

	// Mark notes in scale, as designated by step[] array
	int y = height() - STRING_HEIGHT + SCALE_BORDER;

	for (int i = 0; i < trk()->string; i++) {
		now = trk()->tune[i] % 12;
		for (int j = 0; j < trk()->frets; j++) {
			if (step[now]) {
				p.setBrush(QBrush(QColor(SCALE_COLOR)));
				int x = (j == 0) ? SCALE_BORDER : (int) (fr[j - 1] + SCALE_BORDER);
				p.drawRoundRect(x, y, (int) (fr[j] - x - SCALE_BORDER),
								STRING_HEIGHT - 2 * SCALE_BORDER,
								SCALE_ROUND, SCALE_ROUND);
			}
			now = (now + 1) % 12;
		}
		y -= STRING_HEIGHT;
	}

	QPalette palette;
	palette.setBrush(backgroundRole(), QBrush(*scaleback));
	setPalette(palette);
	qDebug() << "drawScaleBack - done";
}

TabTrack *Fretboard::trk()
{
	Q_ASSERT( tv->selectionModel()->currentIndex().isValid() );
	TabTrack* track = tv->model()->data(tv->currentIndex(), TabSong::TrackPtrRole).value<TabTrack*>();
	Q_ASSERT(track);
	return track;
}

void Fretboard::setTonic(int tonic_)
{
	if (tonic != tonic_) {
		tonic = tonic_;
		drawScaleBack();
	}
}

void Fretboard::setMode(int mode_)
{
	if (mode != mode_) {
		mode = mode_;
		drawScaleBack();
	}
}
