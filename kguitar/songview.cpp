#include "config.h"

#include "songview.h"
#include "songviewcommands.h"
#include "global.h"
#include "settings.h"
#include "trackview.h"
#include "tracklist.h"
#include "trackpane.h"
#include "trackdrag.h"
#include "data/tabsong.h"
#include "data/tabtrack.h"
#include "settrack.h"
#include "settabfret.h"
#include "settabdrum.h"
#include "ui/setsong.h"
#include "ui/chord/chordeditor.h"
#include "ui/chord/chordlistitem.h"
#include "songprint.h"
#include "melodyeditor.h"
#include "fretboard.h"

#include <klocale.h>
#include <kdebug.h>
#include <kxmlguiclient.h>
#include <kmessagebox.h>
#include <QUndoStack>

#include <qclipboard.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <ktextedit.h>
#include <qdir.h>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QScrollBar>
#include <QMimeData>

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#include <tse3/Song.h>
#include <tse3/PhraseEdit.h>
#include <tse3/Part.h>
#include <tse3/Track.h>
#include <tse3/Metronome.h>
#include <tse3/MidiScheduler.h>
#include <tse3/Transport.h>
#include <tse3/Error.h>
#include "playbacktracker.h"
#endif

SongView::SongView(KXMLGUIClient *_XMLGUIClient, QUndoStack *_cmdHist,
				   QWidget *parent): QWidget(parent)
{
#ifdef WITH_TSE3
	playThread = new PlaybackTracker();
	connect(playThread, SIGNAL(playColumn(int, int)), this, SLOT(playbackColumn(int, int)));
#endif

	ro = FALSE;

	m_song = new TabSong(i18n("Unnamed"), 120);
	m_song->addEmptyTrack();

	split = new QSplitter();
	split->setOrientation(Qt::Vertical);
	split->setChildrenCollapsible(false);

#ifdef WITH_TSE3
	tv = new TrackView(m_song, _XMLGUIClient, _cmdHist, midiScheduler(), split);
	connect(playThread, SIGNAL(finished()), tv, SLOT(disablePlaybackCursor()));
#else
	tv = new TrackView(m_song, _XMLGUIClient, _cmdHist, split);
#endif

	splitv = new QSplitter(split);
	splitv->setOrientation(Qt::Horizontal);
	splitv->setChildrenCollapsible(false);

	tl = new TrackList(m_song, _XMLGUIClient, splitv);
//	tl->selectRow(0);
	tp = new TrackPane(tl->verticalHeader()->sectionSize(0), splitv);

	tp->setModel(m_song);
	me = new MelodyEditor(tv);

	QItemSelectionModel* s_model = new QItemSelectionModel(m_song, this);
	tv->setSelectionModel(s_model);
	tl->setSourceSelectionModel(s_model);
	tp->setSelectionModel(s_model);

	connect(s_model, SIGNAL(currentChanged(QModelIndex, QModelIndex)), tv,              SLOT(currentBarChangedSlot(QModelIndex, QModelIndex)));
	connect(s_model, SIGNAL(currentChanged(QModelIndex, QModelIndex)), me->fretboard(), SLOT(currentBarChangedSlot(QModelIndex, QModelIndex)));
	connect(tv, SIGNAL(paneChanged()), tp, SLOT(update()));
	connect(tv, SIGNAL(barChanged()), tp->viewport(), SLOT(update()));
	connect(tv, SIGNAL(barChanged()), tv->viewport(), SLOT(update()));

	s_model->setCurrentIndex(m_song->index(0,0), QItemSelectionModel::Current);
	me->drawBackground();

	// synchronize tracklist and trackpane at vertical scrolling
	connect(tp->verticalScrollBar(), SIGNAL(valueChanged(int)), tl->verticalScrollBar(), SLOT(setValue(int)));
	connect(tl->verticalScrollBar(), SIGNAL(valueChanged(int)), tp->verticalScrollBar(), SLOT(setValue(int)));

	// let higher-level widgets know that we have a changed song if it
	// was changed in TrackView
	connect(tv, SIGNAL(songChanged()), this, SIGNAL(songChanged()));

	QBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(split);
	l->addWidget(me);

	setLayout(l);

	cmdHist = _cmdHist;
}

SongView::~SongView()
{
	delete m_song;

#ifdef WITH_TSE3
	delete playThread;
#endif
}

// Refreshes all the views and resets all minor parameters in the
// song. Should be called every time when the song's just got loaded
// or imported.
void SongView::refreshView()
{
	tl->updateList();
	tv->selectionModel()->setCurrentIndex(m_song->index(0,0), QItemSelectionModel::Current);
}

// Creates a new track in the song
bool SongView::trackNew()
{
	auto oldindex = tv->selectionModel()->currentIndex();
	TabTrack* newtr = new TabTrack(TabTrack::FretTab, "", m_song->freeChannel(), 0, 25, 6, 24);

	int count = m_song->rowCount();
	m_song->insertRow(count);
	m_song->setData(m_song->index(count,0),QVariant::fromValue(newtr), TabSong::TrackPtrRole);
	tv->selectionModel()->setCurrentIndex(m_song->index(count, 0), QItemSelectionModel::Current);

	// Special case - if user declined track properties dialog during
	// track creation, then he doesn't seem to want the new track, so
	// we'll destroy it.

	if (!setTrackProperties()) {
		tv->selectionModel()->setCurrentIndex(oldindex, QItemSelectionModel::Current);
		m_song->removeRow(m_song->rowCount() - 1);
		return FALSE;
	}

	return TRUE;
}

// Deletes the currently selected track in the song
void SongView::trackDelete()
{
	// Check that we won't delete the only last track in the list
	if (m_song->rowCount() > 1) {
		int current = tv->selectionModel()->currentIndex().row();
		m_song->removeRow(current);

		//ALINXFIX: until trackDelete will be a command
		//          do safe things:
		cmdHist->clear();
	}
}

// Generates a new track with a basic bass line, derived from current
// track's rhythm
void SongView::trackBassLine()
{
	if (tv->trk()->trackMode() == TabTrack::DrumTab) {
		KMessageBox::sorry(this, i18n("Can't generate a bass line from drum track"));
		return;
	}

	TabTrack *origtrk = tv->trk();

	if (trackNew()) {
		TabTrack *newtrk = tv->trk();
		newtrk->c.resize(origtrk->c.size());
		ChordEditor cs(origtrk);

		int note;

		for (uint i = 0; i < origtrk->c.size(); i++) {
			for (uint k = 0; k < origtrk->string; k++)
				cs.setApp(k, origtrk->c[i].a[k]);

			cs.detectChord();

			if ((ChordListItem *) cs.chords->item(0)) {
				note = ((ChordListItem *) cs.chords->item(0))->tonic();
				kDebug() << "Column " << i << ", detected tonic " << Settings::noteName(note) << endl;
			} else {
				note = -1;
				kDebug() << "Column " << i << ", EMPTY " << endl;
			}

			for (uint k = 0; k < MAX_STRINGS; k++) {
				newtrk->c[i].a[k] = -1;
				newtrk->c[i].e[k] = 0;
			}

			newtrk->c[i].l = origtrk->c[i].l;
			newtrk->c[i].flags = origtrk->c[i].flags;

			// GREYFIX: make a better way of choosing a fret. This way
			// it can, for example, be over max frets number.
			if (note >= 0) {
				newtrk->c[i].a[0] = note - newtrk->tune[0] % 12;
				if (newtrk->c[i].a[0] < 0)  newtrk->c[i].a[0] += 12;
			}
		}
	};

	tv->arrangeTracks();
}

// Sets current track's properties
bool SongView::trackProperties()
{
	bool res = FALSE;
	TabTrack newtrk(*(tv->trk()));
	SetTrack st(&newtrk);

	if (st.exec()) {
		newtrk.name = st.title->text();
		newtrk.channel = st.channel->value();
		newtrk.bank = st.bank->value();
		newtrk.patch = st.patch->value();
		newtrk.setTrackMode((TabTrack::TrackMode) st.mode->currentIndex());

		// Fret tab
		if (st.mode->currentIndex() == TabTrack::FretTab) {
			SetTabFret *fret = (SetTabFret *) st.modespec;
			newtrk.string = fret->string();
			newtrk.frets = fret->frets();
			for (int i = 0; i < newtrk.string; i++)
				newtrk.tune[i] = fret->tune(i);
		}

		// Drum tab
		if (st.mode->currentIndex() == TabTrack::DrumTab) {
			SetTabDrum *drum = (SetTabDrum *) st.modespec;
			newtrk.string = drum->drums();
			newtrk.frets = 0;
			for (int i = 0; i < newtrk.string; i++)
				newtrk.tune[i] = drum->tune(i);
		}

		// Check that cursor position won't fall over the track limits
		if (newtrk.y >= newtrk.string)
			newtrk.y = newtrk.string - 1;

		cmdHist->push(new SetTrackPropCommand(tv, tl, tp, tv->trk(), &newtrk));
		res = TRUE;
	}

	return res;
}

// Sets track's properties called from trackNew
bool SongView::setTrackProperties()
{
	// TODO: reduce copypaste with previous function
	bool res = FALSE;
	SetTrack *st = new SetTrack(tv->trk());

	if (st->exec()) {
		tv->trk()->name = st->title->text();
		tv->trk()->channel = st->channel->value();
		tv->trk()->bank = st->bank->value();
		tv->trk()->patch = st->patch->value();
		tv->trk()->setTrackMode((TabTrack::TrackMode) st->mode->currentIndex());

		// Fret tab
		if (st->mode->currentIndex() == TabTrack::FretTab) {
			SetTabFret *fret = (SetTabFret *) st->modespec;
			tv->trk()->string = fret->string();
			tv->trk()->frets = fret->frets();
			for (int i = 0; i < tv->trk()->string; i++)
				tv->trk()->tune[i] = fret->tune(i);
		}

		// Drum tab
		if (st->mode->currentIndex() == TabTrack::DrumTab) {
			SetTabDrum *drum = (SetTabDrum *) st->modespec;
			tv->trk()->string = drum->drums();
			tv->trk()->frets = 0;
			for (int i = 0; i < tv->trk()->string; i++)
				tv->trk()->tune[i] = drum->tune(i);
		}

		tl->updateList();
		tp->updateList();
		res = TRUE;
	}

	delete st;
	return res;
}

void SongView::copySelTabsToClipboard()
{
  if (!tv->trk()->sel) {
    KMessageBox::error(this, i18n("There is no selection!"));
    return;
  }

  QMimeData* mdata = new QMimeData;
  mdata->setData( TrackDrag::TRACK_MIME_TYPE, TrackDrag::encode( highlightedTabs() ) );
  QApplication::clipboard()->setMimeData( mdata );
}

void SongView::songProperties()
{
	SetSong ss(m_song->info, m_song->tempo, ro);

	if (ss.exec() && !ro)
		cmdHist->push(new SetSongPropCommand(this, ss.info(), ss.tempo()));
}

void SongView::playSong()
{
#ifdef WITH_TSE3
	kDebug() << "SongView::playSong" << endl;

	// Try to stop a running song, return if we invoked stopping
	if (playThread->stop())
		return;

//	if (!scheduler) {
//		kDebug() << "SongView::playSong: Scheduler not open from the beginning!" << endl;
//		if (!initMidi()) {
//			KMessageBox::error(this, i18n("Error opening MIDI device!"));
//			midiInUse = FALSE;
//			return;
//		}
//	}

	// Get song object
	TSE3::Song *tsong = m_song->midiSong(TRUE);

	int startclock = tv->trk()->cursortimer;

	// Init cursors
	for(auto row = 0; row < m_song->rowCount(); row++) {
		auto index = m_song->index(row, 0);
		TabTrack* trk = index.data(TabSong::TrackPtrRole).value<TabTrack*>();
		if (trk->cursortimer < startclock) {
			trk->x--;
			trk->updateXB();
		}
	}

	tv->setPlaybackCursor(TRUE);
	playThread->playSong(tsong, startclock);
#endif
}

void SongView::stopPlay()
{
#ifdef WITH_TSE3
	playThread->stop();
#endif
}

#ifdef WITH_TSE3
bool SongView::initMidi()
{
	return TRUE;
}
#endif

void SongView::slotCut()
{
	copySelTabsToClipboard();

	tv->deleteColumn(i18n("Cut to clipboard"));
}

void SongView::slotCopy()
{
	copySelTabsToClipboard();
}

void SongView::slotPaste()
{
	TabTrack *trk;

	if (TrackDrag::decode(QApplication::clipboard()->mimeData(), trk))
		insertTabs(trk);

	tv->viewport()->update();
}

void SongView::slotSelectAll()
{
	tv->trk()->xsel = 0;
	tv->trk()->x = tv->trk()->c.size() - 1;
	tv->trk()->sel = TRUE;

	tv->viewport()->update();
}

TabTrack *SongView::highlightedTabs()
{
	if (!tv->trk()->sel)
		return NULL;

	TabTrack* trk = tv->trk();
	TabTrack* newtrk = new TabTrack(trk->trackMode(), "ClipboardTrack", trk->channel,
									trk->bank, trk->patch, trk->string, trk->frets);
	for (int i = 0; i < trk->string; i++)
		newtrk->tune[i] = trk->tune[i];

	uint pdelta, pstart, pend;

	if (trk->x <= trk->xsel) {
		pend = trk->xsel;
		pstart = trk->x;
	} else {
		pend = trk->x;
		pstart = trk->xsel;
	}

	pdelta = pend - pstart + 1;

	newtrk->c.resize(pdelta);
	int _s = pstart;

	for (uint i = 0; i < pdelta; i++) {
		for (uint k = 0; k < MAX_STRINGS; k++) {
				newtrk->c[i].a[k] = -1;
				newtrk->c[i].e[k] = 0;
		}

		newtrk->c[i].l = trk->c[_s].l;
		newtrk->c[i].flags = trk->c[_s].flags;

		for (uint k = 0; k < newtrk->string; k++) {
			newtrk->c[i].a[k] = trk->c[_s].a[k];
			newtrk->c[i].e[k] = trk->c[_s].e[k];
		}

		_s++;
	}

	return newtrk;
}

void SongView::insertTabs(TabTrack* trk)
{
	kDebug() << "SongView::insertTabs(TabTrack* trk) " << endl;

	if (!trk)
		kDebug() << "   trk == NULL" << endl;
	else kDebug() << "   trk with data" << endl;

	//ALINXFIX: Make it more flexible. (songviewcommands.cpp)
	QString msg(i18n("There are some problems:\n\n"));
	bool err = FALSE;
	bool errtune = FALSE;

	if (tv->trk()->trackMode() != trk->trackMode()) {
		msg += i18n("The clipboard data hasn't the same track mode.\n");
		err = TRUE;
	}
	if (tv->trk()->string != trk->string) {
		msg += i18n("The clipboard data hasn't the same number of strings.\n");
		err = TRUE;
	} else {
		for (int i = 0; i < tv->trk()->string; i++) {
			if (tv->trk()->tune[i] != trk->tune[i])
				errtune = TRUE;
			if (errtune) break;
		}
		if (errtune) {
			msg += i18n("The clipboard data hasn't the same tuneing.\n");
			err = TRUE;
		}
	}
	if (tv->trk()->frets != trk->frets) {
		msg += i18n("The clipboard data hasn't the same number of frets.\n");
		err = TRUE;
	}


	if (err) {
		msg += i18n("\n\nI'll improve this code. So some of these problems\n");
		msg += i18n("will be solved in the future.");
		KMessageBox::error(this, msg);
		return;
	}

	cmdHist->push(new InsertTabsCommand(tv, tv->trk(), trk));
}

void SongView::print(QPrinter *printer)
{
	QScopedPointer<SongPrint> sp( tv->buildSongPrintHelper() );
	sp->printSong(printer, m_song);
}

// Advances to the next column to monitor playback when event comes
// thru PlaybackTracker
void SongView::playbackColumn(int track, int x)
{
	auto index = m_song->index(track, 0);
	TabTrack* trk = index.data(TabSong::TrackPtrRole).value<TabTrack*>();
	if (tv->trk() == trk && trk->x != x)
	tv->setX(x);
}
