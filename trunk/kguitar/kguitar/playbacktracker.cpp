#include "playbacktracker.h"
#include "songview.h"
#include "tabtrack.h"

// GREYFIX
#include <stdio.h>

PlaybackTracker::PlaybackTracker(SongView *_sv): TransportCallback()
{
	sv = _sv;
}

void PlaybackTracker::Transport_MidiOut(TSE3::MidiCommand c)
{
	int track, x;
	if (c.status == KGUITAR_MIDI_COMMAND && c.port == KGUITAR_MIDI_PORT) {
		printf("TICK: cmd=%d port=%d data1=%d data2=%d ch=%d\n", c.status, c.port, c.data1, c.data2, c.channel);
		TabTrack::decodeTimeTracking(c, track, x);
		printf("TICK -----------> T%d, x=%d\n", track, x);
		sv->playbackColumn(track, x);
	} else {
		printf("MIDI: cmd=%d port=%d data1=%d data2=%d ch=%d\n", c.status, c.port, c.data1, c.data2, c.channel);
	}
}

void PlaybackTracker::Transport_MidiIn(TSE3::MidiCommand c) {}
