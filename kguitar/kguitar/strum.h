// STRUM.H - library of various strumming patterns

#include "strumlib.h"

// The library itself.

// First field of record should be an array with bitmasked strum
// pattern. Please add up ST macros with "|" operator to get the
// proper results. See the examples below.

// Second field should be with durations of notes of corresponding
// bitmasked patterns in the first array, all in MIDI note length
// notation.

// Third field should be a human-readable name of given strumming
// scheme.

strummer lib_strum[] = {
	{ {},
	  { 1 },
	  "Chord" },
	{ { ST1, ST4, ST5, ST6, ST5, ST4 },
	  { 60,  60,  60,  60,  60,  60, 0 },
	  "6/8 Arpegio" },
	{ { ST1, ST4, ST5 | ST6, ST4 },
	  { 120, 120, 120,       120, 0 },
	  "4/4 Arpegio" },
	{ { ST1, ST4 | ST5 | ST6, ST2, ST4 | ST5 | ST6 },
	  { 120, 120,             120, 120, 0 },
	  "4/4 Normal Bass Pick" },
	{ { ST1, ST4 | ST5 | ST6, ST4 | ST5 | ST6 },
	  { 120, 120,             120, 0 },
	  "3/4 Waltz Bass Pick" },
	{ { ST1, ST4 | ST5 | ST6, ST2, ST4 | ST5 | ST6 },
	  { 180, 60,              180, 60, 0 },
	  "4/4 Deep Bass Pick" },

	{ {}, { 0 } }
};
