#ifndef CONVERTGP3_H
#define CONVERTGP3_H

#include "global.h"
#include "convertbase.h"

class TabSong;

/**
 * Converter to/from standard GP3 files.
 *
 * This dirty piece of code is made by Sylvain "Sly" Vignaud
 * Contact him for infos at:
 * vignsyl@iit.edu
 *
 * Manages conversions of tabulatures between internal KGuitar
 * structures and GP3 files.
 */
class ConvertGp3: public ConvertBase {
public:
	/**
	 * Prepares converter to work, reads all the options, etc.
	 */
	ConvertGp3(TabSong *);

	/**
	 * Called to save current data from TabSong into GP3 tabulature
	 * format file named fileName.
	 */
	virtual bool save(QString fileName);

	/**
	 * Called to load data from GP3 tabulature format file named
	 * fileName into TabSong.
	 */
	virtual bool load(QString fileName);
};

#endif
