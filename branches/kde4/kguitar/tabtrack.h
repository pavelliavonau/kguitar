#ifndef TABTRACK_H
#define TABTRACK_H

#include "global.h"

#include <qmemarray.h>
#include <qrect.h>

#ifdef WITH_TSE3
#include <tse3/PhraseEdit.h>
#endif

/**
 * Represents one bar of a song.
 *
 * Stores start column index, time and key signatures.
 */
typedef struct {
	int start;                          // Starting column
	uchar time1,time2;                  // Time signature
	short keysig;                       // Key signature
} TabBar;

#include "tabcolumn.h"

/**
 * Represents one track of a song.
 *
 * Includes a collection of columns (array c), bars that organize them
 * (array b), MIDI settings (channel/bank/patch), and lots of other
 * necessary stuff.
 */
class TabTrack {
public:
	/**
	 * Enum to designate various track modes.
	 */
	typedef enum {
		FretTab,
		DrumTab
	} TrackMode;

	TabTrack(TrackMode _tm, QString _name, int _channel,
			 int _bank, uchar _patch, char _string, char _frets);

	/**
	 * Array of columns.
	 */
	QMemArray<TabColumn> c;

	/**
	 * Array of bars.
	 */
	QMemArray<TabBar> b;

	/**
	 * Number of strings
	 */
	uchar string;

	/**
	 * Number of frets.
	 */
	uchar frets;

	/**
	 * Tuning, if applicable.
	 */
	uchar tune[MAX_STRINGS];

	TrackMode trackMode() { return tm; }
	void setTrackMode(TrackMode t) { tm = t; }

	uchar channel;                      // MIDI channel
	int bank;                           // MIDI bank
	uchar patch;                        // MIDI patch

//	  QListIterator<TabColumn> xi(QListT<TabColumn>);  // Current tab col iterator

	QString name;                       // Track text name

	int x;                              // Current tab column
	int xb;                             // Current tab bar
	int y;                              // Current tab string

	int cursortimer;                    // After MIDI calculations -
	                                    // timer value on current
	                                    // column, otherwise -
	                                    // undefined

	bool sel;                           // Selection mode enabled
	int xsel;                           // If yes, then selection start column

#ifdef WITH_TSE3
	static TSE3::MidiCommand encodeTimeTracking(int track, int x);
	static void decodeTimeTracking(TSE3::MidiCommand mc, int &track, int &x);
#endif

	int barNr(int c);
	int lastColumn(int n);
	bool showBarSig(int n);
	bool barStatus(int n);
	Q_UINT16 currentBarDuration();
	int trackDuration();
	Q_UINT16 maxCurrentBarDuration();
	Q_UINT16 noteDuration(uint t, int i);
	int noteNrCols(uint t, int i);
	int findCStart(int t, int & dur);
	int findCEnd(int t, int & dur);
	bool isRingingAt(int str, int col);

	void removeColumn(int n);
	void insertColumn(int n);
	int insertColumn(int ts, int te);
	void splitColumn(int col, int dur);
	void arrangeBars();
	void addFX(char fx);
	void updateXB();
	void calcBeams();
	void calcStepAltOct();
	void calcVoices();
	bool hasMultiVoices();
	bool isExactNoteDur(int d);
	bool getNoteTypeAndDots(int t, int v, int & tp, int & dt, bool & tr);

#ifdef WITH_TSE3
	TSE3::PhraseEdit *midiTrack(bool tracking = FALSE, int tracknum = 0);
#endif

private:
	void addNewColumn(TabColumn dat, int len, bool *arc);

	TrackMode tm;                       // Track mode
};

#endif
