#include "playbacktracker.h"
#include "songview.h"
#include "tabtrack.h"

#include <kdebug.h>

PlaybackTracker::PlaybackTracker(SongView *_sv): TransportCallback()
{
	sv = _sv;
}

void PlaybackTracker::Transport_MidiOut(TSE3::MidiCommand c)
{
	int track, x;
	kdDebug() << "TICK: cmd=" << c.status << " port=" << c.port
	          << " data1=" << c.data1 << " data2=" << c.data2
	          << " ch=" << c.channel << endl;
	if (c.status == KGUITAR_MIDI_COMMAND && c.port == KGUITAR_MIDI_PORT) {
		TabTrack::decodeTimeTracking(c, track, x);
		kdDebug() << "TICK -----------> T" << track << ", x=" << x << endl;
		sv->playbackColumn(track, x);
	}
}

void PlaybackTracker::Transport_MidiIn(TSE3::MidiCommand) {}
