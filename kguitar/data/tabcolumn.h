#ifndef TABCOLUMN_H
#define TABCOLUMN_H

#include <qpoint.h>

#include "global.h"
#include "accidentals.h"

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
#define EFFECT_LETRING	5
#define EFFECT_STOPRING	6

#define NULL_NOTE       -1
#define DEAD_NOTE       -2

/***************************************************************************
 * struct StemInfo
 ***************************************************************************/

// this struct holds info about a note stem:
// - where it is attached
// - if it has beams

struct StemInfo {
	QPoint bp;					// attach point
	char l1;					// level 1 beam
	char l2;					// level 2 beam
	char l3;					// level 3 beam
};

/**
 * Represents one column on tabulature song.
 */
class TabColumn {
public:
	TabColumn();
	int l;                              // Duration of note or chord
	char a[MAX_STRINGS];                // Number of fret
	char e[MAX_STRINGS];                // Effect parameter
	uint flags;                         // Various flags

	// TabColumn "volatile" data is calculated when needed,
	// see musicxml.cpp, songprint.cpp and tabtrack.cpp.
	// Used by MusicXML export and PostScript output.
	char v[MAX_STRINGS];				// Voice assigned to note
	char stp[MAX_STRINGS];				// Step
	char alt[MAX_STRINGS];				// Alter
	char oct[MAX_STRINGS];				// Octave
	Accidentals::Accid acc[MAX_STRINGS];// Acidental
	StemInfo stl;						// Lower stems
	StemInfo stu;						// Upper stems
	// End of volatile data

	/**
	 * Calculates and returns full duration of this column,
	 * i.e. multiplied by all necessary 2/3, 3/2 coffiecients.
	 */
	quint16 fullDuration();

	/**
	 * Sets full duration (i.e. "l" + determines automatically all
	 * necessary flags for 2/3 and 3/2 coefficients.
	 */
	void setFullDuration(quint16 len);

	uint effectFlags();
};

#endif
