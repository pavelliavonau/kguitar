#include "config.h"

#include "songview.h"
#include "songviewcommands.h"
#include "global.h"
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
#include "midilist.h"

#include <kapp.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kxmlgui.h>
#include <kxmlguiclient.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kcommand.h>

#include <qclipboard.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmultilinedit.h>
#include <qheader.h>
#include <qdir.h>

#ifdef WITH_TSE3
#include <tse3/Song.h>
#include <tse3/PhraseEdit.h>
#include <tse3/Part.h>
#include <tse3/Track.h>
#include <tse3/Metronome.h>
#include <tse3/MidiScheduler.h>
#include <tse3/plt/Alsa.h>
#include <tse3/plt/OSS.h>
#include <tse3/Transport.h>
#include <tse3/Error.h>
#endif


SongView::SongView(KXMLGUIClient *_XMLGUIClient, KCommandHistory *_cmdHist,
				   QWidget *parent = 0, const char *name = 0): QWidget(parent, name)
{
#ifdef WITH_TSE3
	scheduler = 0L;
	initScheduler();
#endif

	midiInUse = FALSE;
	midiStopPlay = FALSE;

	song = new TabSong(i18n("Unnamed"), 120);
	song->t.append(new TabTrack(FretTab, i18n("Guitar"), 1, 0, 25, 6, 24));

	split = new QSplitter(this);
	split->setOrientation(QSplitter::Vertical);

#ifdef WITH_TSE3
	tv = new TrackView(song, _XMLGUIClient, _cmdHist, scheduler, split);
#else
	tv = new TrackView(song, _XMLGUIClient, _cmdHist, split);
#endif

	splitv = new QSplitter(split);
 	splitv->setOrientation(QSplitter::Horizontal);

	tl = new TrackList(song, _XMLGUIClient, splitv);
	tl->setSelected(tl->firstChild(), TRUE);
	tp = new TrackPane(song, tl->header()->height(), tl->firstChild()->height(), splitv);

	connect(tl, SIGNAL(newTrackSelected(TabTrack *)), tv, SLOT(selectTrack(TabTrack *)));
	connect(tp, SIGNAL(newTrackSelected(TabTrack *)), tv, SLOT(selectTrack(TabTrack *)));
	connect(tp, SIGNAL(newBarSelected(uint)), tv, SLOT(selectBar(uint)));
	connect(tv, SIGNAL(paneChanged()), tp, SLOT(update()));

	QBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(split);

	cmdHist = _cmdHist;
}

SongView::~SongView()
{
	delete song;
}

// Refreshes all the views and resets all minor parameters in the
// song. Should be called every time when the song's just got loaded
// or imported.
void SongView::refreshView()
{
	tv->setCurt(song->t.first());
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
	TabTrack* newtr = new TabTrack(FretTab, "", song->freeChannel(), 0, 25, 6, 24);

	song->t.append(newtr);
	tv->setCurt(newtr);

	// Special case - if user declined track properties dialog during
	// track creation, then he doesn't seem to want the new track, so
	// we'll destroy it.

	if (!setTrackProperties()) {
		tv->setCurt(oldtr);
		song->t.removeLast();
		return FALSE;
	}

	return TRUE;
}

// Deletes the currently selected track in the song
void SongView::trackDelete()
{
	// Check that we won't delete the only last track in the list
	if (song->t.getFirst() != song->t.getLast()) {
		TabTrack *newsel;

		// If we delete the last track, make sure we'll get the item
		if (song->t.last() == tv->trk()) {
			newsel = song->t.prev();
		} else {
			song->t.findRef(tv->trk());
			newsel = song->t.next();
		}

		song->t.remove(tv->trk());
		tv->setCurt(newsel);
		tv->updateRows();
		tv->update();
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
	if (tv->trk()->trackMode() == DrumTab) {
		KMessageBox::sorry(this, i18n("Can't generate a bass line from drum track"));
		return;
	}

	TabTrack *origtrk = tv->trk();

	if (trackNew()) {
		TabTrack *newtrk = tv->trk();
		newtrk->c.resize(origtrk->c.size());
		ChordSelector cs(origtrk);

		int note;
		bool havenote;

		for (uint i = 0; i < origtrk->c.size(); i++) {
			for (uint k = 0; k < origtrk->string; k++) {
				cs.setApp(k, origtrk->c[i].a[k]);
			}

			cs.detectChord();
			havenote = ((ChordListItem *) cs.chords->item(0));

			if (havenote) {
				note = ((ChordListItem *) cs.chords->item(0))->tonic();
				kdDebug() << "Column " << i << ", detected tonic " << note_name(note) << endl;
			} else {
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
			if (havenote) {
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
		newtrk->setTrackMode((TrackMode) st->mode->currentItem());

		// Fret tab
		if (st->mode->currentItem() == FretTab) {
			SetTabFret *fret = (SetTabFret *) st->modespec;
			newtrk->string = fret->string();
			newtrk->frets = fret->frets();
			for (int i = 0; i < newtrk->string; i++)
				newtrk->tune[i] = fret->tune(i);
		}

		// Drum tab
		if (st->mode->currentItem() == DrumTab) {
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
		tv->trk()->setTrackMode((TrackMode) st->mode->currentItem());

		// Fret tab
		if (st->mode->currentItem() == FretTab) {
			SetTabFret *fret = (SetTabFret *) st->modespec;
			tv->trk()->string = fret->string();
			tv->trk()->frets = fret->frets();
			for (int i = 0; i < tv->trk()->string; i++)
				tv->trk()->tune[i] = fret->tune(i);
		}

		// Drum tab
		if (st->mode->currentItem() == DrumTab) {
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

// Dialog to set song's properties
void SongView::songProperties()
{
	SetSong *ss = new SetSong();
	ss->title->setText(song->title);
	ss->title->setReadOnly(isBrowserView);
	ss->author->setText(song->author);
	ss->author->setReadOnly(isBrowserView);
	ss->transcriber->setText(song->transcriber);
	ss->transcriber->setReadOnly(isBrowserView);
	ss->comments->setText(song->comments);
	ss->comments->setReadOnly(isBrowserView);

	if (ss->exec()) {
		cmdHist->addCommand(new SetSongPropCommand(song, ss->title->text(), ss->author->text(),
												   ss->transcriber->text(), ss->comments->text()));
	}

	delete ss;
}

void SongView::playTrack()
{
#ifdef WITH_TSE3
	kdDebug() << "SongView::playTrack" << endl;

	if (midiInUse) {
		kdDebug() << "   ** Sorry you are playing a track/song!!" << endl;
		return;
	}

	midiInUse = TRUE;
	midiStopPlay = FALSE;

	midiList.clear();

	MidiData::getMidiList(tv->trk(), midiList, TRUE);

	playMidi(midiList, FALSE);
#endif
}

void SongView::playSong()
{
#ifdef WITH_TSE3
	kdDebug() << "SongView::playSong" << endl;

	if (midiInUse) {
		kdDebug() << "   ** Sorry you are playing a track/song!!" << endl;
		return;
	}

	midiInUse = TRUE;
	midiStopPlay = FALSE;

	midiList.clear();

	QListIterator<TabTrack> it(song->t);
	for (; it.current(); ++it) {
		TabTrack *trk = it.current();
		MidiData::getMidiList(trk, midiList, TRUE);
	}

	playMidi(midiList);
#endif
}

void SongView::stopPlayTrack()
{
#ifdef WITH_TSE3
	kdDebug() << "SongView::stopPlayTrack" << endl;

	if (midiInUse) midiStopPlay = TRUE;
#endif
}

void SongView::playMidi(MidiList &ml, bool playSong = TRUE)
{
#ifdef WITH_TSE3
	kdDebug() << "SongView::playMidi" << endl;

	if (ml.isEmpty()) {
		midiStopPlay = TRUE;
		midiInUse = FALSE;
		kdDebug() << "    MidiList is empty!! Nothing to play." << endl;
		return;
	}

	if (!scheduler)
		if (!initScheduler()) {
			KMessageBox::error(this, i18n("Error opening MIDI device!"));
			midiInUse = FALSE;
			return;
		}

	kdDebug() << "  Scheduler: " << scheduler << endl;

	MidiEvent *e;
	TSE3::PhraseEdit phraseEdit;

	if (playSong) {
		QListIterator<TabTrack> it(song->t);
		for (; it.current(); ++it) {
			TabTrack *trk = it.current();
			phraseEdit.insert(TSE3::MidiEvent(TSE3::MidiCommand(TSE3::MidiCommand_ProgramChange,
																trk->channel - 1, 0 /*port*/,
																trk->patch), 0));
			//## and MIDI commands for Volume, Chorus, etc.
		}
	} else {
		phraseEdit.insert(TSE3::MidiEvent(TSE3::MidiCommand(TSE3::MidiCommand_ProgramChange,
															tv->trk()->channel - 1, 0 /*port*/,
															tv->trk()->patch), 0));
		//## and MIDI commands for Volume, Chorus, etc.
	}

	long lasttimestamp = 0;
	int lastduration = 0;
	Q_UINT8 lastchn = 0;

	for (e = ml.first(); e != 0; e = ml.next()) {
		phraseEdit.insert(TSE3::MidiEvent(TSE3::MidiCommand(TSE3::MidiCommand_NoteOn, e->chn, 0/*port*/,
															e->data1/*note*/, e->data2 /*velocity*/),
										  e->timestamp, 0/*velocity*/, e->timestamp + e->duration));
		lasttimestamp = e->timestamp;
		lastduration = e->duration;
		lastchn = e->chn;
	}

	lasttimestamp += lastduration;
	phraseEdit.insert(TSE3::MidiEvent(TSE3::MidiCommand(TSE3::MidiCommand_NoteOn, lastchn, 0/*port*/,
														0, 0), lasttimestamp, 0, lasttimestamp + lastduration));

	// Now assemble the Song
	TSE3::Song _song(1);
	TSE3::Phrase *phrase = phraseEdit.createPhrase(_song.phraseList());
	TSE3::Part *part   = new TSE3::Part(0, phraseEdit.lastClock());
	part->setPhrase(phrase);
	_song[0]->insert(part);

	// Create transport objects
	TSE3::Metronome metronome;

	TSE3::Transport transport(&metronome, scheduler);

    // Play and wait for the end
	transport.play(&_song, 0);

	while (transport.status() != TSE3::Transport::Resting) {
		kapp->processEvents();
		if (midiStopPlay)
			transport.stop();
		else
			transport.poll();
	}

	midiInUse = FALSE;
	phraseEdit.clearSelection();

#endif
}

#ifdef WITH_TSE3
bool SongView::initScheduler()
{
	if (!scheduler) {
		try {
			scheduler = AlsaFactory.createScheduler();
			kdDebug() << "TSE3 ALSA MIDI Scheduler created" << endl;
		}
		catch (TSE3::MidiSchedulerError e) {
			kdDebug() << "cannot create an ALSA MIDI Scheduler" << endl;
		}

		if (!scheduler) {
			try {
				scheduler = OSSFactory.createScheduler();
				kdDebug() << "TSE3 OSS MIDI Scheduler created" << endl;
			}
			catch (TSE3::MidiSchedulerError e) {
				kdDebug() << "cannot create an OSS MIDI Scheduler" << endl;
			}
		}

		if (!scheduler) {
			kdDebug() << "ERROR opening MIDI device / Music can't be played" << endl;
			midiInUse = FALSE;
			return FALSE;
		}
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

}

void SongView::slotSelectAll()
{
	tv->trk()->xsel = 0;
	tv->trk()->x =  tv->trk()->c.size() - 1;
	tv->trk()->sel = TRUE;

	tv->update();
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
