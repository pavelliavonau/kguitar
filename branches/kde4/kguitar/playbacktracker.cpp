#include "playbacktracker.h"
#include "songview.h"
#include "tabtrack.h"

#include <kdebug.h>
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

PlaybackTracker::PlaybackTracker(QObject *parent)
	: QThread(parent), TransportCallback()
{
	scheduler = 0;
	init();
	song = 0;
	midiStop = false;
}

PlaybackTracker::~PlaybackTracker()
{
	if (scheduler) {
		if (transport) {
			transport->detachCallback(this);
			delete transport;
		}
		delete metronome;
		delete scheduler;
	}
}

void PlaybackTracker::init()
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
//			midiInUse = FALSE;
//			return FALSE;
		}

		metronome = new TSE3::Metronome;
		transport = new TSE3::Transport(metronome, scheduler);
		transport->attachCallback(this);
	}
}

void PlaybackTracker::playSong(TSE3::Song *song, int startClock)
{
	if (this->song)
		delete this->song;

	// GREYFIX needs mutex here!
	this->song = song;
	this->startClock = startClock;
	midiStop = false;

	if (!isRunning())
		start(LowPriority);
//	} else {
//		midiStop = true;
//		condition.wakeOne();
//	}
}

bool PlaybackTracker::stop()
{
	kdDebug() << "PlaybackTracker::stopPlay" << endl;
	if (isRunning()) {
		midiStop = true;
	} else {
		midiStop = false;
	}
	return midiStop;
}

void PlaybackTracker::run()
{
	kdDebug() << "run: prepare to start\n";
	// GREYFIX needs mutex here!
	TSE3::Song *song = this->song;
	int startClock = this->startClock;

	if (song) {
		kdDebug() << "run: starting\n";

//		transport->setLookAhead(5000);
		transport->setAdaptiveLookAhead(true);

		// Play and wait for the end
		transport->play(song, startClock);

		do {
			if (midiStop)
				transport->stop();
			transport->poll();
			msleep(50);
//			kdDebug() << "polling...\n";
		} while (transport->status() != TSE3::Transport::Resting);

		kdDebug() << "run: stopping\n";

		delete song;
		this->song = NULL;

		playAllNoteOff();
	}
	midiStop = false;
}

void PlaybackTracker::playAllNoteOff()
{
	kdDebug() << "starting panic on stop" << endl;
	TSE3::Panic panic;
	panic.setAllNotesOff(TRUE);
// 	panic.setAllNotesOffManually(TRUE);
	transport->play(&panic, TSE3::Clock());

	do {
		transport->poll();
	} while (transport->status() != TSE3::Transport::Resting);

	kdDebug() << "completed panic on stop" << endl;
}

void PlaybackTracker::Transport_MidiOut(TSE3::MidiCommand c)
{
	int track, x;
//	kdDebug() << "TICK: cmd=" << c.status << " port=" << c.port
//	          << " data1=" << c.data1 << " data2=" << c.data2
//	          << " ch=" << c.channel << endl;
	if (c.status == KGUITAR_MIDI_COMMAND && c.port == KGUITAR_MIDI_PORT) {
		TabTrack::decodeTimeTracking(c, track, x);
//		kdDebug() << "TICK -----------> T" << track << ", x=" << x << endl;
		emit playColumn(track, x);
	}
}

void PlaybackTracker::Transport_MidiIn(TSE3::MidiCommand) {}
#endif
