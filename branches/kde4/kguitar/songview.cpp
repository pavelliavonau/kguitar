#include "config.h"

#include "songview.h"
#include "songviewcommands.h"
#include "global.h"
#include "settings.h"
#include "trackview.h"
#include "tracklist.h"
#include "trackpane.h"
#include "trackdrag.h"
#include "tabsong.h"
#include "tabtrack.h"
#include "settrack.h"
#include "settabfret.h"
#include "settabdrum.h"
#include "setsong.h"
#include "chord.h"
#include "chordlist.h"
#include "chordlistitem.h"
#include "songprint.h"
#include "melodyeditor.h"

#include <klocale.h>
#include <kdebug.h>
#include <kxmlguiclient.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <k3command.h>

#include <qclipboard.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <q3textedit.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3BoxLayout>
#include <Q3VBoxLayout>
#include <QApplication>

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

SongView::SongView(KXMLGUIClient *_XMLGUIClient, K3CommandHistory *_cmdHist,
				   QWidget *parent, const char *name): QWidget(parent, name)
{
#ifdef WITH_TSE3
	scheduler = 0L;
	initMidi();
#endif

	midiInUse = FALSE;
	midiStopPlay = FALSE;

	ro = FALSE;

	m_song = new TabSong(i18n("Unnamed"), 120);
	m_song->t.append(new TabTrack(TabTrack::FretTab, i18n("Guitar"), 1, 0, 25, 6, 24));

	split = new QSplitter(this);
	split->setOrientation(Qt::Vertical);

#ifdef WITH_TSE3
	tv = new TrackView(m_song, _XMLGUIClient, _cmdHist, scheduler, split);
#else
	tv = new TrackView(m_song, _XMLGUIClient, _cmdHist, split);
#endif

	splitv = new QSplitter(split);
 	splitv->setOrientation(Qt::Horizontal);

	tl = new TrackList(m_song, _XMLGUIClient, splitv);
	tl->setSelected(tl->firstChild(), TRUE);
	tp = new TrackPane(m_song, tl->header()->height(), tl->firstChild()->height(), splitv);

	me = new MelodyEditor(tv, split);

	connect(tl, SIGNAL(trackSelected(TabTrack *)), tv, SLOT(selectTrack(TabTrack *)));
	connect(tp, SIGNAL(trackSelected(TabTrack *)), tv, SLOT(selectTrack(TabTrack *)));
	connect(tp, SIGNAL(barSelected(uint)), tv, SLOT(selectBar(uint)));
	connect(tv, SIGNAL(paneChanged()), tp, SLOT(update()));
	connect(tv, SIGNAL(barChanged()), tp, SLOT(repaintCurrentTrack()));

	// synchronize tracklist and trackpane at vertical scrolling
	connect(tl, SIGNAL(contentsMoving(int, int)), tp, SLOT(syncVerticalScroll(int, int)));

	// let higher-level widgets know that we have a changed song if it
	// was changed in TrackView
	connect(tv, SIGNAL(songChanged()), this, SIGNAL(songChanged()));

	Q3BoxLayout *l = new Q3VBoxLayout(this);
	l->addWidget(split);

	cmdHist = _cmdHist;

	sp = new SongPrint();
	tv->initFonts(sp->fFeta, sp->fFetaNr);
}

SongView::~SongView()
{
	delete m_song;
	delete sp;

#ifdef WITH_TSE3
	if (scheduler) {
		transport->detachCallback(tracker);
		delete tracker;
		delete transport;
		delete metronome;
		delete scheduler;
	}
#endif
}

// Refreshes all the views and resets all minor parameters in the
// song. Should be called every time when the song's just got loaded
// or imported.
void SongView::refreshView()
{
	tv->setCurrentTrack(m_song->t.first());
	tv->updateRows();
	tv->repaint();
	tl->updateList();
	tl->setSelected(tl->firstChild(), TRUE);
	tp->updateList();
}

// Creates a new track in the song
bool SongView::trackNew()
{
	TabTrack* oldtr = tv->trk();
	TabTrack* newtr = new TabTrack(TabTrack::FretTab, "", m_song->freeChannel(), 0, 25, 6, 24);

	m_song->t.append(newtr);
	tv->setCurrentTrack(newtr);

	// Special case - if user declined track properties dialog during
	// track creation, then he doesn't seem to want the new track, so
	// we'll destroy it.

	if (!setTrackProperties()) {
		tv->setCurrentTrack(oldtr);
		m_song->t.removeLast();
		return FALSE;
	}

	return TRUE;
}

// Deletes the currently selected track in the song
void SongView::trackDelete()
{
	// Check that we won't delete the only last track in the list
	if (m_song->t.getFirst() != m_song->t.getLast()) {
		TabTrack *newsel;

		// If we delete the last track, make sure we'll get the item
		if (m_song->t.last() == tv->trk()) {
			newsel = m_song->t.prev();
		} else {
			m_song->t.findRef(tv->trk());
			newsel = m_song->t.next();
		}

		m_song->t.remove(tv->trk());
		tv->setCurrentTrack(newsel);
		tv->updateRows();
		tv->repaintContents();
		tl->updateList();
		tp->updateList();

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
		ChordSelector cs(origtrk);

		int note;

		for (uint i = 0; i < origtrk->c.size(); i++) {
			for (uint k = 0; k < origtrk->string; k++)
				cs.setApp(k, origtrk->c[i].a[k]);

			cs.detectChord();

			if ((ChordListItem *) cs.chords->item(0)) {
				note = ((ChordListItem *) cs.chords->item(0))->tonic();
				kdDebug() << "Column " << i << ", detected tonic " << Settings::noteName(note) << endl;
			} else {
				note = -1;
				kdDebug() << "Column " << i << ", EMPTY " << endl;
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
	TabTrack *newtrk = new TabTrack(*(tv->trk()));
	SetTrack *st = new SetTrack(newtrk);

	if (st->exec()) {
		newtrk->name = st->title->text();
		newtrk->channel = st->channel->value();
		newtrk->bank = st->bank->value();
		newtrk->patch = st->patch->value();
		newtrk->setTrackMode((TabTrack::TrackMode) st->mode->currentItem());

		// Fret tab
		if (st->mode->currentItem() == TabTrack::FretTab) {
			SetTabFret *fret = (SetTabFret *) st->modespec;
			newtrk->string = fret->string();
			newtrk->frets = fret->frets();
			for (int i = 0; i < newtrk->string; i++)
				newtrk->tune[i] = fret->tune(i);
		}

		// Drum tab
		if (st->mode->currentItem() == TabTrack::DrumTab) {
			SetTabDrum *drum = (SetTabDrum *) st->modespec;
			newtrk->string = drum->drums();
			newtrk->frets = 0;
			for (int i = 0; i < newtrk->string; i++)
				newtrk->tune[i] = drum->tune(i);
		}

		// Check that cursor position won't fall over the track limits
		if (newtrk->y >= newtrk->string)
			newtrk->y = newtrk->string - 1;

		cmdHist->addCommand(new SetTrackPropCommand(tv, tl, tp, tv->trk(), newtrk));
		res = TRUE;
	}

	delete st;
	delete newtrk;
	return res;
}

// Sets track's properties called from trackNew
bool SongView::setTrackProperties()
{
	bool res = FALSE;
	SetTrack *st = new SetTrack(tv->trk());

	if (st->exec()) {
		tv->trk()->name = st->title->text();
		tv->trk()->channel = st->channel->value();
		tv->trk()->bank = st->bank->value();
		tv->trk()->patch = st->patch->value();
		tv->trk()->setTrackMode((TabTrack::TrackMode) st->mode->currentItem());

		// Fret tab
		if (st->mode->currentItem() == TabTrack::FretTab) {
			SetTabFret *fret = (SetTabFret *) st->modespec;
			tv->trk()->string = fret->string();
			tv->trk()->frets = fret->frets();
			for (int i = 0; i < tv->trk()->string; i++)
				tv->trk()->tune[i] = fret->tune(i);
		}

		// Drum tab
		if (st->mode->currentItem() == TabTrack::DrumTab) {
			SetTabDrum *drum = (SetTabDrum *) st->modespec;
			tv->trk()->string = drum->drums();
			tv->trk()->frets = 0;
			for (int i = 0; i < tv->trk()->string; i++)
				tv->trk()->tune[i] = drum->tune(i);
		}

		tv->selectTrack(tv->trk()); // artificially needed to emit newTrackSelected()
		tl->updateList();
		tp->updateList();
		res = TRUE;
	}

	delete st;
	return res;
}

void SongView::songProperties()
{
	SetSong ss(m_song->info, m_song->tempo, ro);

	if (ss.exec())
		cmdHist->addCommand(new SetSongPropCommand(this, ss.info(), ss.tempo()));
}

void SongView::playSong()
{
#ifdef WITH_TSE3
	kdDebug() << "SongView::playSong" << endl;

	if (midiInUse) {
		stopPlay();
		return;
	}

	midiInUse = TRUE;
	midiStopPlay = FALSE;

	if (!scheduler) {
		kdDebug() << "SongView::playSong: Scheduler not open from the beginning!" << endl;
		if (!initMidi()) {
			KMessageBox::error(this, i18n("Error opening MIDI device!"));
			midiInUse = FALSE;
			return;
		}
	}

	// Get song object
	TSE3::Song *tsong = m_song->midiSong(TRUE);

	int startclock = tv->trk()->cursortimer;

	// Init cursors
	for (TabTrack *trk = m_song->t.first(); trk; trk = m_song->t.next()) {
		if (trk->cursortimer < startclock) {
			trk->x--;
			trk->updateXB();
		}
	}

	// Play and wait for the end
	transport->play(tsong, startclock);
	tv->setPlaybackCursor(TRUE);

	do {
		kapp->processEvents();
		if (midiStopPlay)
			transport->stop();
		transport->poll();
	} while (transport->status() != TSE3::Transport::Resting);

	delete tsong;

	tv->setPlaybackCursor(FALSE);

	// Create and play panic sequence to stop all sounds
	playAllNoteOff();
#endif
}

void SongView::stopPlay()
{
#ifdef WITH_TSE3
	kdDebug() << "SongView::stopPlay" << endl;
	if (midiInUse)  midiStopPlay = TRUE;
#endif
}

#ifdef WITH_TSE3
// Plays so called "panic" events in various styles to shut off any
// stuck playing MIDI note
void SongView::playAllNoteOff()
{
	kdDebug() << "SongView::playSong: starting panic on stop" << endl;
	TSE3::Panic panic;
	panic.setAllNotesOff(TRUE);
// 	panic.setAllNotesOffManually(TRUE);
	transport->play(&panic, TSE3::Clock());

	do {
		kapp->processEvents();
		transport->poll();
	} while (transport->status() != TSE3::Transport::Resting);

	midiInUse = FALSE;

	kdDebug() << "SongView::playSong: completed panic on stop" << endl;
}

bool SongView::initMidi()
{
	if (!scheduler) {
		TSE3::MidiSchedulerFactory factory;
		try {
			scheduler = factory.createScheduler();
			kdDebug() << "MIDI Scheduler created" << endl;
		} catch (TSE3::MidiSchedulerError e) {
			kdDebug() << "cannot create MIDI Scheduler" << endl;
		}

		if (!scheduler) {
			kdDebug() << "ERROR opening MIDI device / Music can't be played" << endl;
			midiInUse = FALSE;
			return FALSE;
		}

		metronome = new TSE3::Metronome;
		transport = new TSE3::Transport(metronome, scheduler);
		tracker = new PlaybackTracker(this);
		transport->attachCallback(tracker);
	}
	return TRUE;
}
#endif

void SongView::slotCut()
{
	if (!tv->trk()->sel){
		KMessageBox::error(this, i18n("There is no selection!"));
		return;
	}

	QApplication::clipboard()->setData(new TrackDrag(highlightedTabs()));
	tv->deleteColumn(i18n("Cut to clipboard"));
}

void SongView::slotCopy()
{
	if (!tv->trk()->sel){
		KMessageBox::error(this, i18n("There is no selection!"));
		return;
	}

	QApplication::clipboard()->setData(new TrackDrag(highlightedTabs()));
}

void SongView::slotPaste()
{
	TabTrack *trk;

	if (TrackDrag::decode(QApplication::clipboard()->data(), trk))
        insertTabs(trk);

	tv->repaintContents();
}

void SongView::slotSelectAll()
{
	tv->trk()->xsel = 0;
	tv->trk()->x = tv->trk()->c.size() - 1;
	tv->trk()->sel = TRUE;

	tv->repaintContents();
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
	kdDebug() << "SongView::insertTabs(TabTrack* trk) " << endl;

	if (!trk)
		kdDebug() << "   trk == NULL" << endl;
	else kdDebug() << "   trk with data" << endl;

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

	cmdHist->addCommand(new InsertTabsCommand(tv, tv->trk(), trk));
}

void SongView::print(QPrinter *printer)
{
	sp->printSong(printer, m_song);
}

// Advances to the next column to monitor playback when event comes
// thru PlaybackTracker
void SongView::playbackColumn(int track, int x)
{
 	TabTrack *trk = m_song->t.at(track);
	if (tv->trk() == trk && trk->x != x)
		tv->setX(x);
}
