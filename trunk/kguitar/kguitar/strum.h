// STRUM.H - library of various strumming patterns

#include "strumlib.h"

// The library itself.

// First field of record should be an array with bitmasked strum
// pattern. Please add up TR and BS macros with "+" operator to get
// the proper results. See the examples below.

// TR macros reference strings from highest to lowest, i.e. TR1
// references the top tab line with standard 6-string guitar.

// BS macros reference strings vice versa, from lowest to highest,
// i.e. BS1 references the bottom tab line with guitar.

// Second field should be with durations of notes of corresponding
// bitmasked patterns in the first array, all in MIDI note length
// notation.

// Third field should be a human-readable name of given strumming
// scheme.

strummer lib_strum[] = {
	{ {},
	  { 1 },
	  "Chord" },
	{ { BS1, TR3, TR2, TR1, TR2, TR3 },
	  { 60,  60,  60,  60,  60,  60, 0 },
	  "6/8 Arpegio" },
	{ { BS1, TR3, TR2 + TR1, TR3 },
	  { 120, 120, 120,       120, 0 },
	  "4/4 Arpegio" },
	{ { BS1, TR3, TR2, TR3, TR1, TR3, TR2, TR3 },
	  { 60,  60,  60,  60,  60,  60,  60,  60, 0 },
	  "8/8 Arpegio 1" },
	{ { BS1, TR3, TR2, TR3, TR1, TR2, TR3, TR2 },
	  { 60,  60,  60,  60,  60,  60,  60,  60, 0 },
	  "8/8 Arpegio 2" },
	{ { BS1, TR1 + TR2 + TR3, BS2, TR1 + TR2 + TR3 },
	  { 120, 120,             120, 120, 0 },
	  "4/4 Normal Bass Pick" },
	{ { BS1, TR1 + TR2 + TR3, TR1 + TR2 + TR3 },
	  { 120, 120,             120, 0 },
	  "3/4 Waltz Bass Pick" },
	{ { BS1, TR1 + TR2 + TR3, BS2, TR1 + TR2 + TR3 },
	  { 180, 60,              180, 60, 0 },
	  "4/4 Deep Bass Pick" },

	{ {}, { 0 } }
};
