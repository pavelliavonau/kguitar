#ifndef CONVERTGTP_H
#define CONVERTGTP_H

#include "global.h"
#include "convertbase.h"
//Added by qt3to4:
#include <Q3MemArray>

class TabSong;

/**
 * Converter to/from Guitar Pro files.
 *
 * Manages conversions of tabulatures between internal KGuitar
 * structures and Guitar Pro files.
 */
class ConvertGtp: public ConvertBase {
public:
	/**
	 * Prepares converter to work, reads all the options, etc.
	 */
	ConvertGtp(TabSong *song);

	/**
	 * Called to save current data from TabSong into Guitar Pro
	 * tabulature format file named fileName.
	 */
	virtual bool save(QString fileName);

	/**
	 * Called to load data from Guitar Pro tabulature format
	 * file named fileName into TabSong.
	 */
	virtual bool load(QString fileName);

private:
	void readSignature();
	void readSongAttributes();
	void readTrackDefaults();
	void readBarProperties();
	void readTrackProperties();
	void readTabs();
	void readColumnEffects(TabTrack *trk, int x);
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
	 * Reads Pascal-like serialized string. It is fixed-length buffer
	 * of maxlen bytes, which contains from the very beginning a byte
	 * of real length and then string data.
	 */
	QString readPascalString(int maxlen);

	/**
	 * Reads word-sized Pascal-like serialized string
	 */
	QString readWordPascalString();

	/**
	 * Read Delphi-serialized integer
	 */
	int readDelphiInteger();

	/**
	 * Advances the pointer by n bytes forward, skipping them.
	 */
	void skipBytes(int n);

	/**
	 * Version of Guitar Pro file
	 */
	int versionMajor, versionMinor;

	enum {
		TRACK_MAX_NUMBER = 32,
		LYRIC_LINES_MAX_NUMBER = 5,
		STRING_MAX_NUMBER = 7
	};

	int numBars, numTracks;

	int trackPatch[TRACK_MAX_NUMBER * 2];
	/**
	 * Temporary variable to remember bar data, to propagate it on
	 * all tracks later. GREYFIX: this won't be needed later.
	 */
	Q3MemArray<TabBar> bars;

	QDataStream *stream;

	/**
	 * Human-readable name of current stage of parsing. Should be
	 * set by stage starting functions. Used for debugging / error
	 * messages.
	 */
	QString currentStage;

	/**
	 * Strong sanity checks, enabled by default. Looks for most
	 * values to lie in reasonable range. Can break loading of very
	 * specific files, thus it should be possible to turn it off.
	 */
	bool strongChecks;
};

#endif
