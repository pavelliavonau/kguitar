#ifndef CONVERTMIDI_H
#define CONVERTMIDI_H

#include "global.h"
#include "convertbase.h"

class TabSong;
class QTextStream;

/**
 * Converter to/from standard MIDI files.
 *
 * Manages conversions of tabulatures between internal KGuitar
 * structures and MIDI files.
 */
class ConvertMidi: public ConvertBase {
public:
	/**
	 * Prepares converter to work, reads all the options, etc.
	 */
	ConvertMidi(TabSong *);

	/**
	 * Called to save current data from TabSong into MIDI tabulature
	 * format file named fileName.
	 */
	virtual bool save(QString fileName);

	/**
	 * Called to load data from MIDI tabulature format file named
	 * fileName into TabSong.
	 */
	virtual bool load(QString fileName);
};

#endif
