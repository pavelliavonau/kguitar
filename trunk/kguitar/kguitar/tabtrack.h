#ifndef TABTRACK_H
#define TABTRACK_H

#include "global.h"

#include <qarray.h>
#include <qstring.h>
#include <qrect.h>

typedef enum {
	FretTab,
	DrumTab
} TrackMode;

// Durations as in MIDI:
// 480 = whole
// 240 = half
// 120 = quarter
// 60  = eighth
// 30  = 16th
// 15  = 32nd

#define FLAG_ARC        1
#define FLAG_DOT        2
#define FLAG_PM         4

#define EFFECT_HARMONIC 1
#define EFFECT_ARTHARM  2
#define EFFECT_LEGATO   3
#define EFFECT_SLIDE    4

#define NULL_NOTE       -1
#define DEAD_NOTE       -2

typedef struct {
	uint l;                             // Duration of note or chord
	char a[MAX_STRINGS];                // Number of fret
	char e[MAX_STRINGS];                // Effect parameter
	uint flags;                         // Various flags
} TabColumn;

typedef struct {
	uint start;                         // Starting column
	uchar time1,time2;                  // Time signature
} TabBar;

class TabTrack {
public:
	TabTrack(TrackMode _tm, QString _name, int _channel,
			 int _bank, uchar _patch, uchar _string, uchar _frets);

	QArray<TabColumn> c;                // Array of columns
	QArray<TabBar> b;                   // Array of bars

	uchar string;                       // Number of strings
	uchar frets;                        // Number of frets
	uchar tune[MAX_STRINGS];            // Tuning, if appicable

	TrackMode trackMode() { return tm; }
	void setTrackMode(TrackMode t) { tm = t; }

	uchar channel;                      // MIDI channel
	int bank;                           // MIDI bank
	uchar patch;                        // MIDI patch

//	  QListIterator<TabColumn> xi(QListT<TabColumn>);  // Current tab col iterator

	QString name;                       // Track text name

	uint x;                             // Current tab column
	uint xb;                            // Current tab bar
	int y;                              // Current tab string

	bool sel;                           // Selection mode enabled
	uint xsel;                          // If yes, then selection start column

	int lastColumn(uint n);
	bool showBarSig(uint n);
	bool barStatus(uint n);

	void removeColumn(uint n);
	void insertColumn(uint n);
	void arrangeBars();
	void addFX(char fx);
	void updateXB();

private:
	TrackMode tm;                       // Track mode
};

#endif
