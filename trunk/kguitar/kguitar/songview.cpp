#include "songview.h"
#include "global.h"
#include "trackview.h"
#include "tracklist.h"
#include "trackpane.h"
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

#include <qsplitter.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmultilinedit.h>
#include <qheader.h>
#include <qdir.h>

#include <libkmid/deviceman.h>
#include <libkmid/midimapper.h>
#include <libkmid/fmout.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>   // kill is declared on signal.h on bsd, not sys/signal.h
#include <sys/signal.h>

SongView::SongView(KXMLGUIClient *_XMLGUIClient, QWidget *parent = 0, const char *name = 0)
    : QWidget(parent, name)
{
	//MIDI INIT STUFF
	QString fmPatch, fmPatchDir;
	fmPatch = locate("data", "kmid/fm/std.o3");

	if (!fmPatch.isEmpty()) {
		QFileInfo *fi = new QFileInfo(fmPatch);
		fmPatchDir = fi->dirPath().latin1();
		fmPatchDir += "/";
		globalHaveMidi = TRUE;

		FMOut::setFMPatchesDirectory(fmPatchDir);

		kdDebug() << "KGMidiInit: FMPatchesDirectory: " << fmPatchDir << endl;
	} else {
		kdDebug() << "KGMidiInit: Can't find FMPatches from KMid !! " << endl;
		kdDebug() << "            ***** MIDI not ready !! *****" << endl;
		globalHaveMidi = FALSE;
	}

	midi = new DeviceManager( /*mididev*/ -1);

	if (midi->initManager() == 0)
		kdDebug() << "KGMidiInit: midi->initManager()...  OK" << endl;

	MidiMapper *map = new MidiMapper(NULL); // alinx - for future option in Optiondialog
											// Maps are stored in:
											// "$DKEDIR/share/apps/kmid/maps/*.map"
	midi->setMidiMap(map);

	midi->openDev();
	midi->initDev();

	midiInUse = FALSE;
	midiStopPlay = FALSE;


	song = new TabSong(i18n("Unnamed"), 120);
	song->t.append(new TabTrack(FretTab, i18n("Guitar"), 1, 0, 25, 6, 24));

	split = new QSplitter(this);
	split->setOrientation(QSplitter::Vertical);

	tv = new TrackView(song, _XMLGUIClient, midi, split);
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
}

SongView::~SongView()
{
	delete song;

	kdDebug() << "Closing device" << endl;
	midi->closeDev();

	kdDebug() << "Deleting devicemanager" << endl;
	delete midi;
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

	if (!trackProperties()) {
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
		ChordSelector cs(devMan(), origtrk);

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

			for (uint k = 0; k < newtrk->string; k++) {
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

		tv->setCurt(tv->trk()); // artificially needed to emit newTrackSelected()
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
		song->title = ss->title->text();
		song->author = ss->author->text();
		song->transcriber = ss->transcriber->text();
		song->comments = ss->comments->text();
	}

	delete ss;
}

void SongView::playTrack()
{
#ifdef HAVE_MIDI
	kdDebug() << "SongView::playTrack with pid:" << getpid() << endl;

	if (midiInUse) {
		kdDebug() << "   ** Sorry you are playing a track!!" << endl;
		return;
	}

	midiInUse = TRUE;
	midiStopPlay = FALSE;

	midiList.clear();

	MidiData::getMidiList(tv->trk(), midiList); // ALINXFIX: at this time only one track...

	playMidi(midiList);
#endif
}

void SongView::stopPlayTrack()
{
#ifdef HAVE_MIDI
	kdDebug() << "SongView::stopPlayTrack" << endl;

	if (midiInUse) midiStopPlay = TRUE;
#endif
}

void SongView::playMidi(MidiList &ml)
{
#ifdef HAVE_MIDI
	kdDebug() << "SongView::playMidi" << endl;

	if (ml.isEmpty()) {
		midiStopPlay = TRUE;
		midiInUse = FALSE;
		kdDebug() << "    MidiList is empty!! Nothing to play." << endl;
		return;
	}

	kdDebug() << "    Parent1 pid: " << getpid() << endl;

	int defDevice = midi->defaultDevice(); // get the device for the child
	if (defDevice == -1) {
		kdDebug() << "There is no device available" << endl;
		return;
	}

	midi->closeDev(); // close MidiDevice for child process
	delete midi;

	int status;
	pid_t m_pid;

	QApplication::flushX();

	m_pid = fork();       // create child process

	if (m_pid == -1) {
		kdDebug() << "    **** Error: can't fork a child process!!" << endl;
		return;
	}

	if (m_pid == 0) {      // ***** child process *****

		kdDebug() << "    --child process with pid: " << getpid() << " and parent pid: " << getppid() << endl;

		// create own MidiDevice for child process
		QString fmPatch, fmPatchDir;
		fmPatch = locate("data", "kmid/fm/std.o3");

		if (!fmPatch.isEmpty()) {

			QFileInfo *fi = new QFileInfo(fmPatch);
			fmPatchDir = fi->dirPath().latin1();
			fmPatchDir += "/";
			globalHaveMidi = TRUE;

			FMOut::setFMPatchesDirectory(fmPatchDir);

			kdDebug() << "      child process: FMPatchesDirectory: " << fmPatchDir << endl;
		}
		else {
			kdDebug() << "      child process: Can't find FMPatches from KMid !! ** MIDI not ready !! ***" << endl;
			globalHaveMidi = FALSE;
		}

		DeviceManager *c_midi;

		kdDebug() << "      child process: c_midi = new DeviceManager(-1)" << endl;
		c_midi = new DeviceManager(-1);

		if (c_midi->initManager() == 0)
			kdDebug() << "      child process: c_midi->initManager()...  OK" << endl;
		else {
			kdDebug() << "      child process: c_midi->initManager() FAILED *******" << endl;
			exit(EXIT_FAILURE);
		}

		MidiMapper *c_map = new MidiMapper(NULL); // alinx - for future option in Optiondialog
		                                          // Maps are stored in:
		                                          // "$DKEDIR/share/apps/kmid/maps/*.map"

		kdDebug() << "      child process: c_midi->setMidiMap()" << endl;
		c_midi->setMidiMap(c_map);

		kdDebug() << "      child process: c_midi->openDev()" << endl;
		c_midi->openDev();
		kdDebug() << "      child process: c_midi->initDev()" << endl;
		c_midi->initDev();
		kdDebug() << "      child process: c_midi->setDefaultDevice(" << defDevice << ")" << endl;
		c_midi->setDefaultDevice(defDevice);

		MidiEvent *e;
		long tempo;
		int tpcn = 4;          // ALINXFIX: TicksPerCuarterNote: make it as option

		c_midi->chnPatchChange(0, tv->trk()->patch);
		c_midi->tmrStart(tpcn);

		for (e = ml.first(); e != 0; e = ml.next()) {
			tempo = e->timestamp * 2;     // ALINXFIX: make the tempo as option

			c_midi->wait(tempo);
			c_midi->noteOn(0, e->data1, e->data2);
		}
		c_midi->wait(0);
		c_midi->sync();
		c_midi->tmrStop();

		sleep(1);
		exit(EXIT_SUCCESS);              // exit child process
	}
	else {                               // ****** parent process ******
		kdDebug() << "    Parent3 pid: " << getpid() << endl;

		pid_t child_pid;
		child_pid = m_pid;   // copy child pid for kill()

		while ((m_pid = waitpid(-1, &status, WNOHANG)) == 0) { //wait for child process
			kdDebug() << "    wait for child process (pid: " << child_pid << ")" << endl;

			kapp->processEvents();

			if (midiStopPlay) {
				kdDebug() << "====> try to stop the midi timer..." << endl;
				kill(child_pid, SIGTERM);
				waitpid(child_pid, NULL, 0);
			}
		}

		if (WIFEXITED(status) != 0)
			kdDebug() << "    child process: no error on exit" << endl;
		else
			kdDebug() << "    child process exit with error => " << WEXITSTATUS(status) << endl;

		midiInUse = FALSE;

		kdDebug() << "    -->reopen MidiDevice 'midi'..." << endl;

		kdDebug() << "    -->midi = new DeviceManager(-1)" << endl;
		midi = new DeviceManager(-1);

		if (midi->initManager() == 0)
			kdDebug() << "    -->midi->initManager()...  OK" << endl;
		else {
			kdDebug() << "    -->midi->initManager() FAILED *******" << endl;
			return;
		}
		kdDebug() << "    -->midi->openDev()" << endl;
		midi->openDev();      // reopen MidiDevice
		kdDebug() << "    -->midi->initDev()" << endl;
		midi->initDev();
		kdDebug() << "    -->midi->setDefaultDevice(" << defDevice << ")" << endl;
		midi->setDefaultDevice(defDevice);
	}
#endif
}

void SongView::slotCut()
{
	if (!tv->trk()->sel){
		KMessageBox::error(this, i18n("There is no selection!"));
		return;
	}

	int px = tv->trk()->x;
	int pxsel = tv->trk()->xsel;

	kdDebug() << "      x: " << px << endl;
	kdDebug() << "   xsel: " << pxsel << endl;
	kdDebug() << "    " << tv->trk()->sel << endl;

	int pdelta;

	if (px <= pxsel)
		pdelta = pxsel - px;
	else pdelta = px - pxsel;

	pdelta++;

	kdDebug() << "   pdelta: " << pdelta << endl;
}

void SongView::slotCopy()
{
	if (!tv->trk()->sel){
		KMessageBox::error(this, i18n("There is no selection!"));
		return;
	}

	int px = tv->trk()->x;
	int pxsel = tv->trk()->xsel;
	kdDebug() << "      x: " << px << endl;
	kdDebug() << "   xsel: " << pxsel << endl;
	kdDebug() << "    " << tv->trk()->sel << endl;

	int pdelta;

	if (px <= pxsel)
		pdelta = pxsel - px;
	else pdelta = px - pxsel;

	pdelta++;

	kdDebug() << "   pdelta: " << pdelta << endl;
}

void SongView::slotPaste()
{
}

void SongView::slotSelectAll()
{
	tv->trk()->xsel = 0;
	tv->trk()->x =  tv->trk()->c.size() - 1;
	tv->trk()->sel = TRUE;

	tv->update();
}

