#ifndef PLAYBACKTRACKER_H
#define PLAYBACKTRACKER_H
#include "config.h"
#ifdef WITH_TSE3

#include <tse3/Transport.h>
#include <QThread>

#define KGUITAR_MIDI_COMMAND 14
#define KGUITAR_MIDI_PORT    0

class TSE3::Song;

class PlaybackTracker: public QThread, TSE3::TransportCallback {
	Q_OBJECT
public:
	PlaybackTracker(QObject *parent = 0);
	~PlaybackTracker();

	void playSong(TSE3::Song *song, int startClock);
	/**
	 * Tries to stop currently running playback. Returns true is playback is running
	 * and the stop process iniated. Real stop would be done by playing thread somewhat
	 * later in time.
	 */
	bool stop();
	void playAllNoteOff();

	virtual void Transport_MidiOut(TSE3::MidiCommand c);
	virtual void Transport_MidiIn(TSE3::MidiCommand c);

protected:
	void run();

signals:
	void playColumn(int track, int x);

private:
	void init();

	TSE3::MidiScheduler *scheduler;
	TSE3::Transport *transport;
	TSE3::Metronome *metronome;
	TSE3::Song *song;
	int startClock;

	bool midiStop;
};

#endif // WITH_TSE3
#endif // PLAYBACKTRACKER_H
