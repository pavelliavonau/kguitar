#include "playbacktracker.h"
#include "songview.h"
#include "data/tabtrack.h"

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
	: QThread(parent), TSE3::TransportCallback()
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
			qDebug() << "MIDI Scheduler created";

			if (!scheduler) {
				qCritical() << "ERROR opening MIDI device / Music can't be played";
//			midiInUse = FALSE;
//			return FALSE;
			}

			metronome = new TSE3::Metronome;
			transport = new TSE3::Transport(metronome, scheduler);
			transport->attachCallback(this);
		} catch (const TSE3::Error &e) {
			qCritical() << "cannot create MIDI Scheduler";
			scheduler = NULL;
		}
	}
}

void PlaybackTracker::playSong(TSE3::Song *song, int startClock)
{
	if (!scheduler) {
		qWarning() << "PlaybackTracker::playSong - no scheduler";
		return;
	}

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
	if (!scheduler) {
		qWarning() << "PlaybackTracker::stopPlay - no scheduler";
		return false;
	}

	qDebug() << "PlaybackTracker::stopPlay";
	if (isRunning()) {
		midiStop = true;
	} else {
		midiStop = false;
	}
	return midiStop;
}

void PlaybackTracker::run()
{
	qDebug() << "run: prepare to start";
	// GREYFIX needs mutex here!
	TSE3::Song *song = this->song;
	int startClock = this->startClock;

	if (song) {
		qDebug() << "run: starting";

//		transport->setLookAhead(5000);
		transport->setAdaptiveLookAhead(true);

		// Play and wait for the end
		transport->play(song, startClock);

		do {
			if (midiStop)
				transport->stop();
			transport->poll();
//			qDebug() << "polling...";
		} while (transport->status() != TSE3::Transport::Resting);

		qDebug() << "run: stopping";

		delete song;
		this->song = NULL;

		playAllNoteOff();
	}
	midiStop = false;
}

void PlaybackTracker::playAllNoteOff()
{
	if (!scheduler) {
		qWarning() << "PlaybackTracker::playAllNoteOff - no scheduler";
		return;
	}

	qDebug() << "starting panic on stop";
	TSE3::Panic panic;
	panic.setAllNotesOff(TRUE);
// 	panic.setAllNotesOffManually(TRUE);
	transport->play(&panic, TSE3::Clock());

	do {
		transport->poll();
	} while (transport->status() != TSE3::Transport::Resting);

	qDebug() << "completed panic on stop";
}

void PlaybackTracker::Transport_MidiOut(TSE3::MidiCommand c)
{
	int track, x;
//	qDebug() << "TICK: cmd=" << c.status << " port=" << c.port
//	          << " data1=" << c.data1 << " data2=" << c.data2
//	          << " ch=" << c.channel;
	if (c.status == KGUITAR_MIDI_COMMAND && c.port == KGUITAR_MIDI_PORT) {
		TabTrack::decodeTimeTracking(c, track, x);
//		qDebug() << "TICK -----------> T" << track << ", x=" << x;
		emit playColumn(track, x);
	}
}

void PlaybackTracker::Transport_MidiIn(TSE3::MidiCommand) {}
#endif
