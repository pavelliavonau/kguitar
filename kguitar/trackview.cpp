#include "trackview.h"
#include "trackviewcommands.h"
#include "data/tabsong.h"
#include "ui/chord/chordeditor.h"
#include "keysig.h"
#include "timesig.h"
#include "songview.h"
#include "fretboard.h"
#include "settings.h"
#include "ui/rhythmeditor.h"

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kmenu.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qpen.h>
#include <qnamespace.h>
#include <qcursor.h>
#include <qstyle.h>
#include <QResizeEvent>
#include <QMouseEvent>

#include "trackprint.h"
#include "songprint.h"
#include "bardelegate.h"

#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <QUndoStack>

#include <stdlib.h>		// required for declaration of abs()
#include <algorithm>

// LVIFIX: note differences between "old" (in trackview.cpp) and "new" drawing code (in trackprint.cpp):
// - erase width around tab column numbers is "as tight as possible", while the cursor is a bit wider,
//   which leads to minor reverse-video artefacts under the cursor
// - starting bars (very thick and thick one) not implemented
// - harmonics use a diamond instead of text in the new code
// - palm muting uses a small cross instead of text in the new code

static const int FONT_SCALE = 11;

static const double NORMAL_FONT_FACTOR            = FONT_SCALE * 0.8;
static const double TIME_SIG_FONT_FACTOR          = FONT_SCALE * 1.4;
static const double SMALL_CAPTION_FONT_FACTOR     = FONT_SCALE * 0.7;
static const double SCORE_FONT_FACTOR             = FONT_SCALE * 2.8;
static const double SCORE_SIG_FONT_FACTOR         = SCORE_FONT_FACTOR * .45;

TrackView::TrackView(TabSong *s, KXMLGUIClient *_XMLGUIClient, QUndoStack *_cmdHist,
#ifdef WITH_TSE3
                     TSE3::MidiScheduler *_scheduler,
#endif
                     QWidget *parent)
	: QTableView(parent)
	, curt(nullptr)
	, barsPerRow(4000) // stub
{
	setFrameStyle(Panel | Sunken);
	setBackgroundRole(QPalette::Base);

	setFocusPolicy(Qt::StrongFocus);

	horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	setVerticalScrollMode(ScrollPerPixel);
	setHorizontalScrollMode(ScrollPerPixel);

	xmlGUIClient = _XMLGUIClient;
	cmdHist = _cmdHist;

	song = s;

	normalFont = new QFont(KGlobalSettings::generalFont());
	if (normalFont->pointSize() == -1) {
		normalFont->setPixelSize((int) NORMAL_FONT_FACTOR);
	} else {
		normalFont->setPointSizeF(NORMAL_FONT_FACTOR);
	}

	smallCaptionFont = new QFont(*normalFont);
	if (smallCaptionFont->pointSize() == -1) {
		smallCaptionFont->setPixelSize((int) SMALL_CAPTION_FONT_FACTOR);
	} else {
		smallCaptionFont->setPointSizeF(SMALL_CAPTION_FONT_FACTOR);
	}

	timeSigFont = new QFont(*normalFont);
	if (timeSigFont->pointSize() == -1) {
		timeSigFont->setPixelSize((int) (TIME_SIG_FONT_FACTOR));
	} else {
		timeSigFont->setPointSizeF(TIME_SIG_FONT_FACTOR);
	}
	timeSigFont->setBold(TRUE);

	lastnumber = -1;

#ifdef WITH_TSE3
	scheduler = _scheduler;
#endif

	trp = new TrackPrint;
	trp->setOnScreen();
	const int lw = 1;
	trp->pLnBl = QPen(Qt::black, lw);
	trp->pLnWh = QPen(Qt::white, lw);
	trp->zoomLevel = 10;

	auto delegate = new BarDelegate(this);
	delegate->setTrackPrintPtr(trp);
	setItemDelegate(delegate);
	connect(this, SIGNAL(playbackCursorChanged(bool)), delegate, SLOT(setPlaybackCursor(bool)));

	setModel( song );

	viewScore(false);

	initFonts();
	updateRows();		// depends on trp's font metrics
}

TrackView::~TrackView()
{
	delete normalFont;
	delete smallCaptionFont;
	delete timeSigFont;
	delete trp;
	delete fetaFont;
	delete fetaNrFont;
}

TabTrack *TrackView::trk()
{
	auto curIndex = selectionModel()->currentIndex();

	Q_ASSERT(curIndex.isValid());
//	if(!curIndex.isValid())
//		curt = nullptr;
//	else
		curt = model()->data(curIndex, TabSong::TrackPtrRole).value<TabTrack*>();

	Q_ASSERT(curt);

	return curt;
}

void TrackView::initFonts()
{
	kDebug() << "TrackView::initFonts\n";
	fetaFont   = new QFont("FreeSerif", SCORE_FONT_FACTOR);
	fetaNrFont = new QFont("FreeSerif", SCORE_SIG_FONT_FACTOR);
	fetaNrFont->setBold(true);

	trp->initFonts(normalFont, smallCaptionFont, timeSigFont, fetaFont, fetaNrFont);

	QPainter paint(this);
	trp->setPainter(&paint);
	trp->initMetrics();
}

SongPrint* TrackView::buildSongPrintHelper()
{
  return new SongPrint(fetaFont, fetaNrFont);
}

int TrackView::rowBar(int bar)
{
	return bar / barsPerRow;
}

int TrackView::colBar(int bar)
{
	return bar - bar / barsPerRow * barsPerRow;
}

int TrackView::barByRowCol(int row, int col)
{
	return row * barsPerRow + col;
}

void TrackView::currentBarChangedSlot(QModelIndex current, QModelIndex)
{
	if( !current.isValid() )
		return;

	uint n = current.column();
	if (n != (uint) trk()->xb && n < (uint) curt->bars().size()) {
		curt->x = curt->bars()[n].start;
		curt->xb = n;
		ensureCurrentVisible();
		emit barChanged();
		emit columnChanged();
	}
	lastnumber = -1;
}

void TrackView::setZoomLevel(int newZoomLevel)
{
	if (newZoomLevel > 0) {
		trp->zoomLevel = newZoomLevel;
		updateRows();
		viewport()->update();
	}
}

void TrackView::zoomIn()
{
	setZoomLevel(trp->zoomLevel - 1);
}

void TrackView::zoomOut()
{
	setZoomLevel(trp->zoomLevel + 1);
}

// Set zoom level dialog
void TrackView::zoomLevelDialog()
{
	// GREYFIX
}

void TrackView::updateRows()
{
//    int cw = trp->barWidth(0, trk());
//	if (cw < 10)
//		cw = 10;

////	barsPerRow = (width() - 2 - QStyle::PM_ScrollBarExtent) / cw;
//	if (barsPerRow < 1)
//		barsPerRow = 1;

//	int ch = (int) ((TOPSPTB + curt->string + BOTSPTB) * trp->ysteptb);
//#ifdef USE_BOTH_OLD_AND_NEW
//	// note: cannot make row height dependent on viewscore without making too many
//	// changes to the "old" drawing code: use fixed height
//	ch += 3 * VERTLINE * 2 + VERTLINE * (curt->string - 1);
//	ch += (int) ((TOPSPST + NLINEST - 1 + BOTSPST) * trp->ystepst);
//	ch += (int) (ADDSPST * trp->ystepst);
//#else
//	if (viewscore && fetaFont) {
//		ch += (int) ((TOPSPST + NLINEST - 1 + BOTSPST) * trp->ystepst);
//		ch += (int) (ADDSPST * trp->ystepst);
//	}
//#endif

	ensureCurrentVisible();

	resizeColumnsToContents();
	resizeRowsToContents();
}

void TrackView::repaintCurrentBar()
{
	auto index = model()->index( rowBar(curt->xb), colBar(curt->xb) );
	update( index );
	emit paneChanged();
}

void TrackView::repaintCurrentColumn()
{
	//VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2

	//	int ycoord = 0;
//	if (rowYPos(curt->xb, &ycoord)) // GREYFIX - what was it all about?

	// GREYFIX: some crazy things going here about what coordinate
	// system to use. I'm totally screwed up trying to figure it out,
	// until I do, just update whole cell.

// 	repaint(selxcoord, cellHeight() * curt->xb - contentsY(), HORCELL + 1, cellHeight());

	repaintCurrentBar();
// 	emit paneChanged();
}

void TrackView::ensureCurrentVisible()
{
	//ensureCellVisible(rowBar(curt->xb), colBar(curt->xb));
}

// Process a mouse press of fret "fret" in current column on string
// "num". Depending on given "button" mouse state flags, additional
// things may happen.
void TrackView::melodyEditorPress(int num, int fret, Qt::MouseButton button = Qt::NoButton)
{
	if (button & Qt::LeftButton)
		melodyEditorAction(num, fret, 0);
	if (button & Qt::MidButton)
		melodyEditorAction(num, fret, 1);
	if (button & Qt::RightButton)
		melodyEditorAction(num, fret, 2);
}

// Execute one of melody editors actions, as defined in
// globalMelodyEditorAction array
void TrackView::melodyEditorAction(int num, int fret, int action)
{
	// GREYFIX: make it *one* undo transaction
	switch (Settings::melodyEditorAction(action)) {
	case 0: // no action
		break;
	case 1: // set note
		setFinger(num, fret);
		break;
	case 3: // set 022 power chord
		setFinger(num + 2, fret + 2);
		break;
	case 2: // set 02 power chord
		setFinger(num + 1, fret + 2);
		setFinger(num, fret);
		break;
	case 5: // set 0022 power chord
		setFinger(num + 3, fret + 2);
		setFinger(num + 2, fret + 2);
		break;
	case 4: // set 00 power chord
		setFinger(num + 1, fret);
		setFinger(num, fret);
		break;
	case 6: // delete note
		setFinger(num, NULL_NOTE);
		break;
	}
}

// Process a mouse release in melody editor. Depending on given
// "button" mouse state flags, additional things, such as proceeding
// to next column, may happen.
void TrackView::melodyEditorRelease(Qt::MouseButton button)
{
	if (((button & Qt::LeftButton)  && (Settings::melodyEditorAdvance(0))) ||
		((button & Qt::MidButton)   && (Settings::melodyEditorAdvance(1))) ||
		((button & Qt::RightButton) && (Settings::melodyEditorAdvance(2))))  {
		if (curt->sel) {
			curt->sel = FALSE;
			viewport()->update();
		}
		moveRight();
	}
}

// Add tab number insertion command on current column, string "num",
// setting fret number "fret". Perform various checks, including
// no repeats for same insertion.
void TrackView::setFinger(int num, int fret)
{
	if (num < 0 || num >= curt->string)
		return;
	if (fret > curt->frets)
		return;
	if (curt->c[curt->x].a[num] == fret)
		return;

	curt->y = num;
	cmdHist->push(new InsertTabCommand(this, curt, fret));
	repaintCurrentColumn();
	emit columnChanged();
}

int TrackView::finger(int num)
{
	return curt->c[curt->x].a[num];
}

void TrackView::setLength(int l)
{
	//only if needed
	if (curt->c[curt->x].l != l)
		cmdHist->push(new SetLengthCommand(this, curt, l));
}

void TrackView::linkPrev()
{
	cmdHist->push(new SetFlagCommand(this, curt, FLAG_ARC));
	lastnumber = -1;
}

void TrackView::addHarmonic()
{
	if (curt->c[curt->x].a[curt->y] >= 0)
		cmdHist->push(new AddFXCommand(this, curt, EFFECT_HARMONIC));
	lastnumber = -1;
}

void TrackView::addArtHarm()
{
	if (curt->c[curt->x].a[curt->y] >= 0)
		cmdHist->push(new AddFXCommand(this, curt, EFFECT_ARTHARM));
	lastnumber = -1;
}

void TrackView::addLegato()
{
	if (curt->c[curt->x].a[curt->y] >= 0)
		cmdHist->push(new AddFXCommand(this, curt, EFFECT_LEGATO));
	lastnumber = -1;
}

void TrackView::addSlide()
{
	if (curt->c[curt->x].a[curt->y] >= 0)
		cmdHist->push(new AddFXCommand(this, curt, EFFECT_SLIDE));
	lastnumber = -1;
}

void TrackView::addLetRing()
{
	if (curt->c[curt->x].a[curt->y] >= 0)
		cmdHist->push(new AddFXCommand(this, curt, EFFECT_LETRING));
	else
		cmdHist->push(new AddFXCommand(this, curt, EFFECT_STOPRING));
	lastnumber = -1;
}

// Call the chord constructor dialog and may be parse something from it
void TrackView::insertChord()
{
	int a[MAX_STRINGS];

	ChordEditor cs(
#ifdef WITH_TSE3
	                 scheduler,
#endif
	                 curt);

	for (int i = 0; i < curt->string; i++)
		cs.setApp(i, curt->c[curt->x].a[i]);

	// required to detect chord from tabulature
	cs.detectChord();

	int i;

	// set fingering right if frets > 5
	for (i = 0; i < curt->string; i++)
		a[i] = cs.app(i);
	cs.fng->setFingering(a);

	if (cs.exec()) {
		for (i = 0; i < curt->string; i++)
			a[i] = cs.app(i);
		cmdHist->push(new InsertStrumCommand(this, curt, cs.scheme(), a));
	}

	lastnumber = -1;
}

// Call rhythm construction dialog and may be parse something from it
void TrackView::rhythmer()
{
	RhythmEditor re;

	if (re.exec())
		cmdHist->push(new InsertRhythm(this, curt, re.quantizedDurations()));

	lastnumber = -1;
}

// Determine horizontal offset between two columns - n and n+1

int TrackView::horizDelta(uint n)
{
#ifdef USE_BOTH_OLD_AND_NEW
	int res = curt->c[n].fullDuration() * HORCELL / trp->zoomLevel;
// 	if (res < HORCELL)
// 		res = HORCELL;
#else
	int res = trp->colWidth(n, curt);
#endif
	return res;
}

#ifdef USE_BOTH_OLD_AND_NEW
void TrackView::drawLetRing(QPainter *p, int x, int y)
{
	p->setPen(Qt::SolidLine);
	p->drawLine(x, y, x - HORCELL / 3, y - VERTLINE / 3);
	p->drawLine(x, y, x - HORCELL / 3, y + VERTLINE / 3);
	p->setPen(Qt::NoPen);
}
#endif

void TrackView::resizeEvent(QResizeEvent *e)
{
	QTableView::resizeEvent(e); // GREYFIX ? Is it C++-correct?
	//updateRows();
}

bool TrackView::moveFinger(int from, int dir)
{
	int n0 = curt->c[curt->x].a[from];
	int n = n0;
	if (n < 0)
		return FALSE;

	int to = from;

	do {
		to += dir;
		if ((to < 0) || (to >= curt->string))
			return FALSE;
		n = n0 + curt->tune[from] - curt->tune[to];
		if ((n < 0) || (n > curt->frets))
			return FALSE;
	} while (curt->c[curt->x].a[to] != -1);

	cmdHist->push(new MoveFingerCommand(this, curt, from, to, n));
	emit columnChanged();

	return TRUE;
}

// LVIFIX: eventually KGuitar should support changing the key at the start
// of a new bar. For the time being, we don't: the key is the same for the
// whole track and is stored in the first bar

void TrackView::keySig()
{
	int oldsig = curt->bars()[0].keysig;
	if ((oldsig <= -8) || (8 <= oldsig)) {
		// LVIFIX: report error ???
		oldsig = 0;
	}

	SetKeySig sks(oldsig);

	if (sks.exec()) {
		curt->bars()[0].keysig = sks.keySignature();
		// LVIFIX: undo info
	}

	updateRows();
	lastnumber = -1;
}

void TrackView::timeSig()
{
	SetTimeSig sts(curt->bars()[curt->xb].time1, curt->bars()[curt->xb].time2);

	if (sts.exec())
		cmdHist->push(new SetTimeSigCommand(this, curt, sts.toend->isChecked(),
		                                          sts.time1(), sts.time2()));

	lastnumber = -1;
}

void TrackView::keyLeft()
{
	if (curt->sel) {
		curt->sel = FALSE;
		viewport()->update();
	} else {
		moveLeft();
	}
}

void TrackView::keyRight()
{
	if (curt->sel) {
		curt->sel = FALSE;
		viewport()->update();
	} else {
		moveRight();
	}
}

void TrackView::keyLeftBar()
{
	if (curt->sel) {
		curt->sel = FALSE;
		viewport()->update();
	} else {
		moveLeftBar();
	}
}

void TrackView::keyRightBar()
{
	if (curt->sel) {
		curt->sel = FALSE;
		viewport()->update();
	} else {
		moveRightBar();
	}
}

void TrackView::keyHome()
{
	if (curt->sel) {
		curt->sel = FALSE;
		viewport()->update();
	} else {
		moveHome();
	}
}

void TrackView::keyEnd()
{
	if (curt->sel) {
		curt->sel = FALSE;
		viewport()->update();
	} else {
		moveEnd();
	}
}

void TrackView::keyCtrlHome()
{
	if (curt->sel) {
		curt->sel = FALSE;
		viewport()->update();
	} else {
		moveCtrlHome();
	}
}

void TrackView::keyCtrlEnd()
{
	if (curt->sel) {
		curt->sel = FALSE;
		viewport()->update();
	} else {
		moveCtrlEnd();
	}
}

void TrackView::moveLeft()
{
	if (curt->x > 0) {
		if (curt->bars()[curt->xb].start == curt->x) {
			curt->x--;
			repaintCurrentBar();
			curt->xb--;
			ensureCurrentVisible();
			emit barChanged();
		} else {
			curt->x--;
		}
		repaintCurrentBar();
		emit columnChanged();
	}
	lastnumber = -1;
}

void TrackView::moveRight()
{
	if ((curt->x + 1) == curt->c.size()) {
		cmdHist->push(new AddColumnCommand(this, curt));
		emit columnChanged();
	} else {
		if (curt->bars().size() == curt->xb + 1)
			curt->x++;
		else {
			if (curt->bars()[curt->xb + 1].start == curt->x + 1) {
				curt->x++;
				repaintCurrentBar();
				curt->xb++;
				ensureCurrentVisible();
				emit barChanged();
			} else {
				curt->x++;
			}
		}
		repaintCurrentBar();
		emit columnChanged();
	}
	lastnumber = -1;
}

void TrackView::moveLeftBar()
{
	if (curt->x > curt->bars()[curt->xb].start) {
		moveHome();
	} else {
		moveLeft();
		moveHome();
	}
}

void TrackView::moveRightBar()
{
	if (curt->x == curt->lastColumn(curt->xb)) {
		moveRight();
	} else if (curt->x == curt->bars()[curt->xb].start) {
		moveEnd();
		moveRight();
	} else {
		moveEnd();
	}
}

void TrackView::moveHome()
{
	curt->x = curt->bars()[curt->xb].start;
	repaintCurrentBar();
	emit columnChanged();
}

void TrackView::moveEnd()
{
	curt->x = curt->lastColumn(curt->xb);
	repaintCurrentBar();
	emit columnChanged();
}

void TrackView::moveCtrlHome()
{
	curt->x = 0;
	curt->xb = 0;
	ensureCurrentVisible();
	viewport()->update();
	emit barChanged();
	emit columnChanged();
}

void TrackView::moveCtrlEnd()
{
	curt->x = curt->c.size() - 1;
	curt->xb = curt->bars().size() - 1;
	ensureCurrentVisible();
	viewport()->update();
	emit barChanged();
	emit columnChanged();
}

void TrackView::selectLeft()
{
	if (!curt->sel) {
		curt->sel = TRUE;
		curt->xsel = curt->x;
		repaintCurrentBar();
	} else {
		moveLeft();
	}
}

void TrackView::selectRight()
{
	if (!curt->sel) {
		curt->sel = TRUE;
		curt->xsel = curt->x;
		repaintCurrentBar();
	} else {
		moveRight();
	}
}

void TrackView::moveUp()
{
	if (curt->y+1 < curt->string) {
		curt->y++;
		if (curt->sel)
			repaintCurrentBar();
		else
			repaintCurrentColumn();
	}
	lastnumber = -1;
}

void TrackView::transposeUp()
{
	if (curt->y+1 < curt->string)
		moveFinger(curt->y, 1);
	lastnumber = -1;
}

void TrackView::moveDown()
{
	if (curt->y > 0) {
		curt->y--;
		if (curt->sel)
			repaintCurrentBar();
		else
			repaintCurrentColumn();
	}
	lastnumber = -1;
}

void TrackView::transposeDown()
{
	if (curt->y > 0)
		moveFinger(curt->y, -1);
	lastnumber = -1;
}

void TrackView::deadNote()
{
	cmdHist->push(new SetFlagCommand(this, curt, DEAD_NOTE));
	emit columnChanged();
	lastnumber = -1;
}

void TrackView::deleteNote()
{
	if (curt->c[curt->x].a[curt->y] != -1) {
		cmdHist->push(new DeleteNoteCommand(this, curt));
		emit columnChanged();
	}
	lastnumber = -1;
}

void TrackView::deleteColumn()
{
	cmdHist->push(new DeleteColumnCommand(this, curt));
	emit columnChanged();
	lastnumber = -1;
}

void TrackView::deleteColumn(QString name)
{
	cmdHist->push(new DeleteColumnCommand(name, this, curt));
	emit columnChanged();
}

void TrackView::insertColumn()
{
	cmdHist->push(new InsertColumnCommand(this, curt));
	emit columnChanged();
	lastnumber = -1;
}

void TrackView::palmMute()
{
	cmdHist->push(new SetFlagCommand(this, curt, FLAG_PM));
	lastnumber = -1;
}

void TrackView::dotNote()
{
	cmdHist->push(new SetFlagCommand(this, curt, FLAG_DOT));
	lastnumber = -1;
}

void TrackView::tripletNote()
{
	cmdHist->push(new SetFlagCommand(this, curt, FLAG_TRIPLET));
	lastnumber = -1;
}

void TrackView::keyPlus()
{
	if (curt->c[curt->x].l < 480)
		setLength(curt->c[curt->x].l * 2);
	lastnumber = -1;
}

void TrackView::keyMinus()
{
	if (curt->c[curt->x].l > 15)
		setLength(curt->c[curt->x].l / 2);
	lastnumber = -1;
}

void TrackView::arrangeTracks()
{
	cmdHist->clear();       // because columns will be changed
	curt->arrangeBars();
	emit barChanged();
	updateRows();
	viewport()->update();

	emit paneChanged();
	emit columnChanged();
}

void TrackView::insertTab(int num)
{
	int totab = num;

	// Allow making two-digit fret numbers pressing two keys sequentially
	if ((lastnumber != -1) && (lastnumber * 10 + num <= curt->frets)) {
		totab = lastnumber * 10 + num;
		lastnumber = -1;
	} else {
		lastnumber = num;
	}

	if ((totab <= curt->frets) && (curt->c[curt->x].a[curt->y] != totab))
		cmdHist->push(new InsertTabCommand(this, curt, totab));
	emit columnChanged();
}

void TrackView::arrangeBars()
{
	song->arrangeBars();
	emit barChanged();
	emit columnChanged();
	updateRows();
}

void TrackView::mousePressEvent(QMouseEvent *e)
{
	lastnumber = -1;

	// RightButton pressed
	if (e->button() == Qt::RightButton) {
		QWidget *tmpWidget = 0;
		tmpWidget = xmlGUIClient->factory()->container("trackviewpopup", xmlGUIClient);

		if (!tmpWidget) {
			kDebug() << "TrackView::contentsMousePressEvent => no container widget" << endl;
			return;
		}

		if (!tmpWidget->inherits("QMenu")) {
			kDebug() << "TrackView::contentsMousePressEvent => container widget is not QMenu" << endl;
			return;
		}

		QMenu *menu(static_cast<QMenu*>(tmpWidget));
		menu->popup(QCursor::pos());
	}

	// LeftButton pressed
	if (e->button() == Qt::LeftButton) {
		bool found = FALSE;
		const QPoint& clickpt = e->pos();

		auto index = indexAt(e->pos());
		if(!index.isValid())
			return;

		if( e->modifiers() & Qt::ControlModifier )
			selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
		else
			selectionModel()->setCurrentIndex(index, QItemSelectionModel::Current | QItemSelectionModel::Clear);

		uint bn = index.column();//barByRowCol(index.row(), index.column());

		// Clicks on non-existing rows are not allowed
		if (bn >= trk()->bars().size())
			return;

		QRect index_rect = visualRect(index);
		int xpos = trp->getFirstColOffs(bn, curt) + index_rect.left();
		int xdelta = index_rect.left();
		int lastxpos = index_rect.left();

		for (uint j=curt->bars()[bn].start;
		     j < (bn < curt->bars().size()-1 ? curt->bars()[bn+1].start : curt->c.size());
		     j++) {

			// Length of interval to next column - adjusted if dotted
			xdelta = horizDelta(j);

			// Current column X area is half of the previous duration and
			// half of current duration

			if ((clickpt.x() >= (lastxpos + xpos) / 2) &&
				(clickpt.x() <= xpos + xdelta / 2)) {
				curt->x = j;
				// We won't calculate xb from x as in updateXB(), but
				// would just use what we know.
				curt->xb = bn;

				const int vertline = trp->ysteptb;
				trp->calcYPosSt(index_rect.top());
				const int vertspace = trp->calcYPosTb(curt->string); // LVIFIX: better name, this is not the vertical space but the lowest tab line's y coord
				curt->y = - ((int) (clickpt.y() - vertline / 2) - vertspace) / vertline;

				if (curt->y < 0)
					curt->y = 0;
				if (curt->y >= curt->string)
					curt->y = curt->string-1;

				curt->sel = FALSE;

				emit columnChanged();
				emit barChanged();
				found = TRUE;
				break;
			}

			lastxpos = xpos;
			xpos += xdelta;
		}

		if (found)
			viewport()->update();
	}
}

void TrackView::setX(int x)
{
	if (curt->x == x)  return;

	if (x < (int) curt->c.size()) {
		curt->x = x;
		int oldxb = curt->xb;
		curt->updateXB();
		if (oldxb == curt->xb) {
			repaintCurrentBar();
		} else {
			update();
			ensureCurrentVisible();
		}
		emit columnChanged();
		lastnumber = -1;
	}
}

void TrackView::disablePlaybackCursor()
{
	setPlaybackCursor(false);
}

void TrackView::setPlaybackCursor(bool pc)
{
	emit playbackCursorChanged(pc);
	update();
}

void TrackView::viewScore(bool on)
{
//	cout << "TrackView::viewScore(on=" << on << ")" << endl;
	trp->viewscore = on;
	updateRows();
}


bool TrackView::isIndexHidden(const QModelIndex &) const
{
	return false;
}
