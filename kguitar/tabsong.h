#ifndef TABSONG_H
#define TABSONG_H

#include "global.h"

#include <q3ptrlist.h>
#include <qstring.h>
#include <q3textstream.h>
#include <qmap.h>

#ifdef WITH_TSE3
#include <tse3/Song.h>
#endif

#include "tabtrack.h"

/**
 * Represents tabulature-based song in memory.
 *
 * Stores a collection of TabTracks and misc song info, such as
 * metainfo and tempo info.
 */
class TabSong {
public:
	TabSong(QString _title, int _tempo);
	int tempo;

	/**
	 * Holds a list of tracks that the song consists of.
	 */
	Q3PtrList<TabTrack> t;

	/**
	 * Map of metainformation. Can hold lots of strings, referenced by
	 * another lot of strings as a key. Generally, we follow Ogg
	 * Vorbis comment recommendations for key names, for example,
	 * info["TITLE"] should return title of the song.
	 */
	QMap<QString, QString> info;

	/**
	 * Find the minimal free channel, for example, to use for new
	 * track
	 */
	int freeChannel();

	/**
	 * Returns length of longest track in bars
	 */
	uint maxLen();

	void arrangeBars();
#ifdef WITH_TSE3
	TSE3::Song *midiSong(bool tracking = FALSE);
#endif
};

#endif
