#ifndef TABCOLUMN_H
#define TABCOLUMN_H

#include "global.h"

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
#define FLAG_TRIPLET	8

#define EFFECT_HARMONIC 1
#define EFFECT_ARTHARM  2
#define EFFECT_LEGATO   3
#define EFFECT_SLIDE    4

#define NULL_NOTE       -1
#define DEAD_NOTE       -2

class TabColumn {
public:
	int l;                              // Duration of note or chord
	char a[MAX_STRINGS];                // Number of fret
	char e[MAX_STRINGS];                // Effect parameter
	uint flags;                         // Various flags

	Q_UINT16 fullDuration();
	void setFullDuration(Q_UINT16 len);

	uint effectFlags();
};

#endif
