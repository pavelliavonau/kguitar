#ifndef CONVERTKG_H
#define CONVERTKG_H

#include "global.h"
#include "convertbase.h"

class TabSong;
class QDataStream;

/**
 * Reader/writer for native KGuitar KG tabulature format.
 */
class ConvertKg: public ConvertBase {
public:
	/**
	 * Prepares converter to work, reads all the options, etc.
	 */
	ConvertKg(TabSong *);

	/**
	 * Called to save current data from TabSong into ASCII tabulature
	 * format file named fileName.
	 */
	virtual bool save(QString fileName);

	/**
	 * Called to load data from ASCII tabulature format file named
	 * fileName into TabSong.
	 */
	virtual bool load(QString fileName);
};

#endif
