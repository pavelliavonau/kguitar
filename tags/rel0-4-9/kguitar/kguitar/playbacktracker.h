#ifndef PLAYBACKTRACKER_H
#define PLAYBACKTRACKER_H
#include "config.h"
#ifdef WITH_TSE3

#include <tse3/Transport.h>

#define KGUITAR_MIDI_COMMAND 14
#define KGUITAR_MIDI_PORT    0

class SongView;

class PlaybackTracker: public TSE3::TransportCallback {
public:
	PlaybackTracker(SongView *);
	virtual void Transport_MidiOut(TSE3::MidiCommand c);
	virtual void Transport_MidiIn(TSE3::MidiCommand c);

private:
	SongView *sv;
};

#endif // WITH_TSE3
#endif // PLAYBACKTRACKER_H
