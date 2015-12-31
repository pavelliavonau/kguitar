#ifndef TABTRACK_H
#define TABTRACK_H

#include "global.h"

#include <QRect>

#ifdef WITH_TSE3
#include <tse3/PhraseEdit.h>
#endif

/**
 * Represents one bar of a song.
 *
 * Stores start column index, time and key signatures.
 */
struct TabBar{
	TabBar(int _start = -1, uchar _time1 = 0, uchar _time2 = 0, short _keysig = 0);

	bool isValid() const;

	int start;                          // Starting column
	uchar time1,time2;                  // Time signature
	short keysig;                       // Key signature
};

Q_DECLARE_METATYPE(TabBar)

#include "tabcolumn.h"

/**
 * Represents one track of a song.
 *
 * Includes a collection of columns (array c), bars that organize them
 * (array b), MIDI settings (channel/bank/patch), and lots of other
 * necessary stuff.
 */
class TabTrack {
	friend class TabSong;

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
	 * Copy constructor
	 */
	TabTrack(TabTrack *trk);

	/**
	 * Array of columns.
	 */
	QVector<TabColumn> c;

	/**
	 * Array of bars.
	 */
private:
	QVector<TabBar> b;
public:
	// TODO: remove no const version
	QVector<TabBar>& bars() { return b; }

	const QVector<TabBar>& bars() const { return b; }

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

	int barNr(int c);
	int lastColumn(int n);
	bool showBarSig(int n);
	bool barStatus(int n);
	quint16 currentBarDuration();
	quint16 barDuration(int bn);
	int trackDuration();
	quint16 maxCurrentBarDuration();
	quint16 noteDuration(uint t, int i);
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
	static TSE3::MidiCommand encodeTimeTracking(int track, int x);
	static void decodeTimeTracking(TSE3::MidiCommand mc, int &track, int &x);
#endif

private:
	void addNewColumn(TabColumn dat, int len, bool *arc);

	TrackMode tm;                       // Track mode
};

Q_DECLARE_METATYPE(TabTrack*)

#endif
