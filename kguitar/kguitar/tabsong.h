#ifndef TABSONG_H
#define TABSONG_H

#include "global.h"

#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

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
	QList<TabTrack> t;					// Track data
	QString title;						// Title of the song
	QString author;						// Author of the tune
	QString transcriber;				// Who made the tab
	QString comments;					// Comments

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
