#ifndef TUNINGS_H
#define TUNINGS_H

// tunings.h - library of stringed instrument tunings

#include <klocale.h>
#include <qstring.h>

typedef struct {
	int strings;
	uchar shift[MAX_STRINGS];
	QString name;
} tuning;

// The library itself. Columns are:
//
// 1st - number of strings
// 2nd - array of tuning - midi note numbers (should be fairly easy to guess)
// 3rd - name of the tuning in the form "Instrument (strings): tuning"
//
// Various instruments should be separated by an empty line.

// Contributors:
//
// 04 Sep 2000 - Ukulele (4): standard - Alex Brand

// =============== MIDI note pitch reference table ===============
//     |  1 |  2 |  3 |  4 |  5 |  6 |
// ----+----+----+----+----+----+----+----------------------------
// C   | 12 | 24 | 36 | 48 | 60 | 72 |
// Db  | 13 | 25 | 37 | 49 | 61 | 73 |
// D   | 14 | 26 | 38 | 50 | 62 | 74 |
// Eb  | 15 | 27 | 39 | 51 | 63 | 75 |
// E   | 16 | 28 | 40 | 52 | 64 | 76 |
// F   | 17 | 29 | 41 | 53 | 65 | 77 |
// F#  | 18 | 30 | 42 | 54 | 66 | 78 |
// G   | 19 | 31 | 43 | 55 | 67 | 79 |
// G#  | 20 | 32 | 44 | 56 | 68 | 80 |
// A   | 21 | 33 | 45 | 57 | 69 | 81 |
// Bb  | 22 | 34 | 46 | 58 | 70 | 82 |
// B   | 23 | 35 | 47 | 59 | 71 | 83 |
// ---------------------------------------------------------------

tuning lib_tuning[] = {
	{ 1, {},                        I18N_NOOP("User defined") },

	{ 6, {40,45,50,55,59,64},       I18N_NOOP("Guitar") + QString(" (6): E (") + I18N_NOOP("standard") + QString(")") },
	{ 6, {39,44,49,54,58,63},       I18N_NOOP("Guitar") + QString(" (6): Eb") },
	{ 6, {38,43,48,53,57,62},       I18N_NOOP("Guitar") + QString(" (6): D") },
	{ 6, {38,45,50,55,59,64},       I18N_NOOP("Guitar") + QString(" (6): drop D") },
	{ 6, {36,43,48,55,60,64},       I18N_NOOP("Guitar") + QString(" (6): open C") },
	{ 6, {38,45,50,54,57,62},       I18N_NOOP("Guitar") + QString(" (6): open D") },
	{ 6, {40,47,52,56,59,64},       I18N_NOOP("Guitar") + QString(" (6): open E") },

	{ 7, {33,40,45,50,55,59,64},    I18N_NOOP("Guitar") + QString(" (7): rock") },
	{ 7, {38,43,47,50,55,59,62},    I18N_NOOP("Guitar") + QString(" (7): traditional") },

	{ 8, {33,40,45,50,55,59,64,69}, I18N_NOOP("Guitar") + QString(" (8): brahms") },

	{ 4, {28,33,38,43},             I18N_NOOP("Bass") + QString(" (4): E (") + I18N_NOOP("standard") + ")" },
	{ 4, {26,33,38,43},             I18N_NOOP("Bass") + QString(" (4): drop D") },

	{ 5, {28,33,38,43,47},          I18N_NOOP("Bass") + QString(" (5): ") + I18N_NOOP("standard") },

	{ 5, {67,50,55,59,62},          I18N_NOOP("Banjo") + QString(" (5): open G") },
	{ 5, {67,48,55,59,62},          I18N_NOOP("Banjo") + QString(" (5): drop C") },
	{ 5, {69,50,54,57,62},          I18N_NOOP("Banjo") + QString(" (5): open D") },
	{ 5, {67,50,55,58,62},          I18N_NOOP("Banjo") + QString(" (5): G minor") },
	{ 5, {67,50,55,57,62},          I18N_NOOP("Banjo") + QString(" (5): G modal") },

	{ 4, {55,62,69,76},             I18N_NOOP("Mandolin") + QString(" (4): ") + I18N_NOOP("standard") },

	{ 4, {57,50,54,59},             I18N_NOOP("Ukulele") + QString(" (4): ") + I18N_NOOP("standard") },

	{ 3, {64,64,69},                I18N_NOOP("Balalaika") + QString(" (3): ") + I18N_NOOP("traditional") },
	{ 3, {64,64,69},                I18N_NOOP("Balalaika") + QString(" (3): ") + I18N_NOOP("guitar-like") },
	{ 3, {57,57,62},                I18N_NOOP("Balalaika") + QString(" (3): ") + I18N_NOOP("secunda") },
	{ 3, {52,52,57},                I18N_NOOP("Balalaika") + QString(" (3): ") + I18N_NOOP("alt") },
	{ 3, {40,45,50},                I18N_NOOP("Balalaika") + QString(" (3): ") + I18N_NOOP("bass") },
	{ 3, {28,33,38},                I18N_NOOP("Balalaika") + QString(" (3): ") + I18N_NOOP("contrabass") },

	{ 3, {59,64,71},                I18N_NOOP("Shamisen") + QString(" (3): ") + I18N_NOOP("honchoshi") },
	{ 3, {59,66,71},                I18N_NOOP("Shamisen") + QString(" (3): ") + I18N_NOOP("niagari") },
	{ 3, {59,64,69},                I18N_NOOP("Shamisen") + QString(" (3): ") + I18N_NOOP("sansagari") },

	{ 0, {}, 0 }
};

#endif
