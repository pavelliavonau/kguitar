#ifndef CONVERT_ASCII_H
#define CONVERT_ASCII_H

#include "global.h"
#include "convertbase.h"

class TabSong;
class QTextStream;

/**
 * Converter to/from ASCII tabulature format.
 *
 * Manages conversions of tabulatures between internal KGuitar
 * structures and ASCII tabulature formats.
 */
class ConvertAscii: public ConvertBase {
public:
	/**
	 * Prepares converter to work, reads all the options, etc.
	 */
	ConvertAscii(TabSong *song_);

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

private:
	/**
	 * Prints out main song header.
	 */
	void writeHeader();

	/**
	 * Formats and prints out one track.
	 */
	void writeTrack(TabTrack *, int n);

	/**
	 * Starts new track, inits variables and prints track header.
	 */
	void startTrack(TabTrack *, int n);

	/**
	 * Starts new row.
	 */
	void startRow(TabTrack *trk);

	/**
	 * Adds column col of track trk to rendering arrays.
	 */
	void addColumn(TabTrack *trk, TabColumn *col);

	/**
	 * Flushes current bar rendering to row rendering. Flushes row if
	 * necessary.
	 */
	void flushBar(TabTrack *trk);

	/**
	 * Flushes current row rendering to stream.
	 */
	void flushRow(TabTrack *trk);

	/**
	 * Utility method to print out single string line, centered.
	 */
	void writeCentered(QString l);

	/**
	 * Options variable: duration to spaces mode.
	 */
	int durMode;

	/**
	 * Options variable: right margin of page (page width).
	 */
	int pageWidth;

	/**
	 * Minimal duration to put one blank for (i.e. we put only one
	 * blank for this duration and any durations less than this).
	 */
	int oneBlankDuration;

	/**
	 * Minimal offset of row start from the left edge. Either 1 or 2,
	 * depends of what row start contains (ex. "E" = 1, "Eb" = 2)
	 */
	uint minstart;

	/**
	 * Array of strings that contain ASCII tab representation of
	 * current bar.
	 */
	QString bar[MAX_STRINGS];

	/**
	 * Number of bar[], already appended to row[].
	 */
	int rowBars;

	/**
	 * Array of strings that contain ASCII tab representation of
	 * current row.
	 */
	QString row[MAX_STRINGS];

	/**
	 * I/O stream, used by converter.
	 */
	QTextStream *stream;
};

#endif
