/***************************************************************************
 * accidentals.cpp: implementation of Accidentals class
 *
 * This file is part of KGuitar, a KDE tabulature editor
 *
 * copyright (C) 2002 the KGuitar development team
 ***************************************************************************/

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file COPYING for more information.
 ***************************************************************************/

// LVIFIX:
// add support for key signature

// LVIFIX:
// a sharp is printed on all F#'s in the same chord

#include <qstring.h>
#include "accidentals.h"

// class Accidentals -- given note pitch, handle all accidentals
//
// use as follows:
// foreach measure
//   resetToKeySig()
//   foreach chord in measure:
//     startChord()
//     foreach note in chord:
//       addPitch()
//     calcChord()
//     foreach note in chord:
//       getNote()

static const QString notes_flat[12]  = {"C",  "Db", "D",  "Eb", "E",  "F",
                                        "Gb", "G",  "Ab", "A",  "Bb", "B"};
static const QString notes_sharp[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                        "F#", "G",  "G#", "A",  "A#", "B"};

// Accidentals constructor

Accidentals::Accidentals()
{
	for (int i=0; i<stPerOct; i++) {
		notes_av[i]       = false;
		notes_req[i]      = false;
		old_acc_state[i]  = Natural;
		new_acc_state[i]  = Natural;
		out_root_note[i]  = 0;
		out_accidental[i] = None;
	}
}

// add pitch to chord

void
Accidentals::addPitch(int pitch)
{
	int noteNumber = normalize(pitch);
	notes_req[noteNumber] = true;
}

// do the work

void
Accidentals::calcChord()
{
	// init
	for (int i=0; i<stPerOct; i++) {
		// only naturals are available
		if (notes_sharp[i].length() == 1) {
			notes_av[i] = true;
		} else {
			notes_av[i] = false;
		}
		// default new_acc_state = old_acc_state
		new_acc_state[i] = old_acc_state[i];
		// init root note (dummy value)
		out_root_note[i] = 0;
		// init accidentals
		out_accidental[i] = Natural;
	}
	// pass 1: handle naturals
	// loop over all requested notes
	for (int i=0; i<stPerOct; i++) {
		// if note needed
		if (notes_req[i]) {
			// if note is a natural
			if (notes_sharp[i].length() == 1) {
				markInUse(i, i, Natural);
			}
		}
	}
	// pass 2: handle accidentals
	// loop over all requested notes
	for (int i=0; i<stPerOct; i++) {
		// if note needed
		if (notes_req[i]) {
			// if note is not a natural
			if (notes_sharp[i].length() != 1) {
				int nl = normalize(i - 1);	// lower note to try
				int nh = normalize(i + 1);	// higher note to try
				// first check of this pitch is already available
				// i.e. lower note available and acc_state is Sharp
				// or higher note available and acc_state is Flat
				if (notes_av[nl] && (old_acc_state[nl] == Sharp)) {
					// lower note: F# requested, F# used in previous chord
					markInUse(i, nl, Sharp);
				} else if (notes_av[nh] && (old_acc_state[nh] == Flat)) {
					// higher note: F# requested, Gb used in previous chord
					markInUse(i, nh, Flat);
				// pitch is not already available, make it
				} else if (notes_av[nl]) {
					// lower note: if F# req, if F av. use F + #
					markInUse(i, nl, Sharp);
				} else if (notes_av[nh]) {
					// higher note: if F# req, if G av. use G + b
					markInUse(i, nh, Flat);
				} else {
					// special case:
					// F# req, both F and G already used
					// for F use F + natural, for F# use F + #
					// new_acc_state is natural
					out_accidental[nl] = Natural;	// explicit natural
					out_root_note[i]   = nl;		// note name to use
					out_accidental[i]  = Sharp;
					new_acc_state[nl]  = Natural;
		        }
			}
		}
	}
	// copy new_acc_state into old_acc_state
	for (int i=0; i<stPerOct; i++) {
		old_acc_state[i] = new_acc_state[i];
	}
}

// get note info for given pitch

bool
Accidentals::getNote(int pitch, QString& stp, int& alt, int& oct, Accid& acc)
{
	int noteNumber = normalize(pitch);
	if (!notes_req[noteNumber]) {
		return false;
	}
	stp = notes_sharp[out_root_note[noteNumber]].left(1);
	oct = pitch / stPerOct;
	alt = pitch - (oct * stPerOct + out_root_note[noteNumber]);
	acc = out_accidental[noteNumber];
	return true;
}

// mark pitch i as composed of pitch nlh and accidental a

void
Accidentals::markInUse(int i, int nlh, Accid a)
{
	notes_av[nlh]      = false;
	new_acc_state[nlh] = a;
	out_root_note[i]   = nlh;
	if (old_acc_state[nlh] == new_acc_state[nlh]) {
		out_accidental[i] = None;
	} else {
		out_accidental[i] = a;
	}
}

// make sure note number is in range 0..11

int
Accidentals::normalize(int pitch)
{
	int noteNumber = pitch % stPerOct;
	if (noteNumber < 0) {
		noteNumber += stPerOct;
	}
	return noteNumber;
}

// reset to key signature

void
Accidentals::resetToKeySig()
{
	for (int i=0; i<stPerOct; i++) {
		old_acc_state[i] = Natural;
	}
}

// convert step (note name), alter (flat/sharp) and octave to pitch
// return -1 on failure

int
Accidentals::sao2Pitch(const QString& stp, int alt = 0, int oct = 0)
{
    int cn = -1;

	// search step in note name table
    for (int i = 0; i < 12; i++) {
		if (notes_sharp[i] == stp) {
			cn = i;
		}
		if (notes_flat[i] == stp) {
			cn = i;
		}
	}
	if (cn == -1) {
		return -1;
	}
	return oct * 12 + cn + alt;
}

// start a new chord

void
Accidentals::startChord()
{
	for (int i=0; i<stPerOct; i++) {
		notes_req[i] = false;
		out_root_note[i] = 0;
	}
}
