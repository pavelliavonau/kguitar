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

	int freeChannel();
	uint maxLen();
	void arrangeBars();
#ifdef WITH_TSE3
	TSE3::Song *midiSong(bool tracking = FALSE);
#endif

	bool loadFromKg(QString fileName);		// Native format - kg
	bool saveToKg(QString fileName);
	bool loadFromGtp(QString fileName);		// Guitar Pro format
	bool saveToGtp(QString fileName);
	bool loadFromGp3(QString fileName);		// Guitar Pro 3 format
	bool saveToGp3(QString fileName);
	bool loadFromMid(QString fileName);		// MIDI files
	bool saveToMid(QString fileName);
	bool saveToTse3(QString fileName);        // TSE3MDL files
	bool saveToTexTab(QString fileName);		// MusiXTeX/kgtabs.tex tabulatures
	bool saveToTexNotes(QString fileName);	// MusiXTeX notes
	bool loadFromXml(QString fileName);		// MusicXML format
	bool saveToXml(QString fileName);

private:
	Q_UINT32 readVarLen(QDataStream *s);
	void writeVarLen(QDataStream *s, uint value);
	void writeTempo(QDataStream *s, uint value);
	QString tab(bool chord, int string, int fret);
	QString getNote(QString note, int duration, bool dot);
	QString cleanString(QString str);

	void readDelphiString(QDataStream *s, char *c);
	int readDelphiInteger(QDataStream *s);
};

#endif
