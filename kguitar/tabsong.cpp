#include "tabsong.h"
#include "settings.h"

#include "musicxml.h"

#include <qxml.h>
#include <qfile.h>
#include <qdatastream.h>
#include <kconfig.h>

#ifdef WITH_TSE3
#include <tse3/Track.h>
#include <tse3/Part.h>
#include <tse3/TempoTrack.h>
#include <string>
#endif

TabSong::TabSong(QString _title, int _tempo)
{
	tempo = _tempo;
	info["TITLE"] = _title;
}

int TabSong::freeChannel()
{
	bool fc[17];
	for (int i = 1; i <= 16; i++)
		fc[i] = TRUE;

	for (int i = 0; i < t.size(); i++)
		fc[t[i].channel] = false;

	int res;
	for (res = 1; res <= 16; res++)
		if (fc[res])
			break;

	if (res > 16)
		res = 1;

	return res;
}

uint TabSong::maxLen()
{
	uint res = 0;

	for (int i = 0; i < t.size(); i++)
		res = t.at(i).b.size() > res ? t.at(i).b.size() : res;

	return res;
}

void TabSong::arrangeBars()
{
	// For every track
	for (int i = 0; i < t.size(); i++)
		t.at(i).arrangeBars();
}

#ifdef WITH_TSE3
// Assembles the whole TSE song from various tracks, generated with
// corresponding midiTrack() calls.
TSE3::Song *TabSong::midiSong(bool tracking)
{
	TSE3::Song *song = new TSE3::Song(0);

	// Create tempo track
	TSE3::Event<TSE3::Tempo> tempoEvent(tempo, TSE3::Clock(0));
	song->tempoTrack()->insert(tempoEvent);

	// Create data tracks
	int tn = 0;
	QListIterator<TabTrack> it(t);
	for (; it.current(); ++it) {
		TSE3::PhraseEdit *trackData = it.current()->midiTrack(tracking, tn);
		TSE3::Phrase *phrase = trackData->createPhrase(song->phraseList());
		TSE3::Part *part = new TSE3::Part(0, trackData->lastClock() + 1); // GREYFIX: this may be why last event got clipped?
		part->setPhrase(phrase);
		TSE3::Track *trk = new TSE3::Track();
		trk->insert(part);
		song->insert(trk);
		delete trackData;
		tn++;
	}

	return song;
}
#endif

void TabSong::addEmptyTrack()
{
	TabTrack *trk = new TabTrack(TabTrack::FretTab, i18n("Guitar"), 1, 0, 25, 6, 24);
	t.append(*trk);
}
