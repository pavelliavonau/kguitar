#ifndef CONVERTTSE3_H
#define CONVERTTSE3_H

#include "global.h"
#include "convertbase.h"
//Added by qt3to4:
#include <Q3TextStream>

class TabSong;
class Q3TextStream;

/**
 * Converter to/from standard TSE3 files.
 *
 * Manages conversions of tabulatures between internal KGuitar
 * structures and TSE3 files.
 */
class ConvertTse3: public ConvertBase {
public:
	/**
	 * Prepares converter to work, reads all the options, etc.
	 */
	ConvertTse3(TabSong *);

	/**
	 * Called to save current data from TabSong into TSE3 tabulature
	 * format file named fileName.
	 */
	virtual bool save(QString fileName);

	/**
	 * Called to load data from TSE3 tabulature format file named
	 * fileName into TabSong.
	 */
	virtual bool load(QString fileName);
};

#endif
