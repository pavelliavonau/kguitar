#ifndef TABTRACK_H
#define TABTRACK_H

#include "global.h"

#include <qmemarray.h>
#include <qstring.h>
#include <qrect.h>

#ifdef WITH_TSE3
#include <tse3/PhraseEdit.h>
#endif

typedef enum {
	FretTab,
	DrumTab
} TrackMode;

typedef struct {
	int start;                          // Starting column
	uchar time1,time2;                  // Time signature
	short keysig;						// Key signature
} TabBar;

#include "tabcolumn.h"

class TabTrack {
public:
	TabTrack(TrackMode _tm, QString _name, int _channel,
			 int _bank, uchar _patch, char _string, char _frets);

	QMemArray<TabColumn> c;             // Array of columns
	QMemArray<TabBar> b;                // Array of bars

	uchar string;                       // Number of strings
	uchar frets;                        // Number of frets
	uchar tune[MAX_STRINGS];            // Tuning, if applicable

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

	bool sel;                           // Selection mode enabled
	int xsel;                           // If yes, then selection start column

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
	TSE3::PhraseEdit *midiTrack();
#endif

private:
	void addNewColumn(TabColumn dat, int len, bool *arc);

	TrackMode tm;                       // Track mode
};

#endif
