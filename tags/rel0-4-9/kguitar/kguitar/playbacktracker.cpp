#include "playbacktracker.h"

#include "songview.h"

// GREYFIX
#include <stdio.h>

PlaybackTracker::PlaybackTracker(SongView *_sv): TransportCallback()
{
	sv = _sv;
}

void PlaybackTracker::Transport_MidiOut(TSE3::MidiCommand c)
{
	if (c.status == KGUITAR_MIDI_COMMAND && c.port == KGUITAR_MIDI_PORT) {
		printf("TICK! %d\n", c.data1);
		sv->playbackNextColumn(c.channel, c.data1);
	} else {
		printf("MIDI: cmd=%d port=%d data1=%d data2=%d ch=%d\n", c.status, c.port, c.data1, c.data2, c.channel);
	}
}

void PlaybackTracker::Transport_MidiIn(TSE3::MidiCommand c) {}
