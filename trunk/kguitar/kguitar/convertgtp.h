#ifndef CONVERTGTP_H
#define CONVERTGTP_H

#include "global.h"
#include "convertbase.h"

class TabSong;

/**
 * Converter to/from standard GTP files.
 *
 * Manages conversions of tabulatures between internal KGuitar
 * structures and GTP files.
 */
class ConvertGtp: public ConvertBase {
public:
	/**
	 * Prepares converter to work, reads all the options, etc.
	 */
	ConvertGtp(TabSong *);

	/**
	 * Called to save current data from TabSong into GTP tabulature
	 * format file named fileName.
	 */
	virtual bool save(QString fileName);

	/**
	 * Called to load data from GTP tabulature format file named
	 * fileName into TabSong.
	 */
	virtual bool load(QString fileName);

private:
	void readDelphiString(char *c);
	int readDelphiInteger();

	QDataStream *stream;
};

#endif
