#ifndef CONVERT_BASE_H
#define CONVERT_BASE_H

#include "global.h"
#include "tabsong.h"

/**
 * Abstract base class for all converters.
 *
 * Pure abstract base class to define basic structure of all
 * converters - classes that manage loading / saving of internal
 * KGuitar representation of tabulature data into some file format.
 */
class ConvertBase {
public:
	ConvertBase(TabSong *song) { this->song = song; };
	virtual ~ConvertBase() {};

	/**
	 * Called to save current data from TabSong into file named
	 * fileName.
	 */
	virtual bool save(QString fileName) = 0;

	/**
	 * Called to load data from file named fileName into TabSong.
	 */
	virtual bool load(QString fileName) = 0;

protected:
	TabSong *song;

};

#endif
