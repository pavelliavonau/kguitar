#ifndef CONVERTGTP_H
#define CONVERTGTP_H

#include "global.h"
#include "convertbase.h"

class TabSong;

/**
 * Converter to/from Guitar Pro 4 files.
 *
 * Manages conversions of tabulatures between internal KGuitar
 * structures and GP4 files.
 */
class ConvertGtp: public ConvertBase {
public:
	/**
	 * Prepares converter to work, reads all the options, etc.
	 */
	ConvertGtp(TabSong *);

	/**
	 * Called to save current data from TabSong into GP4 tabulature
	 * format file named fileName.
	 */
	virtual bool save(QString fileName);

	/**
	 * Called to load data from GP4 tabulature format file named
	 * fileName into TabSong.
	 */
	virtual bool load(QString fileName);

private:
	bool readSignature();
	void readSongAttributes();
	void readTrackDefaults();
	void readBarProperties();
	void readTrackProperties();
	void readTabs();
	void readNote(TabTrack *trk, int x, int y);
	void readChromaticGraph();
	void readChord();

	/**
	 * Reads Delphi string in GPro format. Delphi string looks pretty
	 * much like: <max-length> <real-length> <real-length * bytes -
	 * string data>
	 */
	QString readDelphiString();

	/**
	 * Reads Pascal-like serialized string
	 */
	QString readPascalString();

	/**
	 * Reads word-sized Pascal-like serialized string
	 */
	QString readWordPascalString();

	/**
	 * Read Delphi-serialized integer
	 */
	int readDelphiInteger();

	enum {
		TRACK_MAX_NUMBER = 32,
		LYRIC_LINES_MAX_NUMBER = 5,
		STRING_MAX_NUMBER = 7
	};

	int numBars, numTracks;

	int trackPatch[TRACK_MAX_NUMBER * 2];

	QDataStream *stream;
};

#endif
