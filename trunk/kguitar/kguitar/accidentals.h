/***************************************************************************
 * accidentals.h: definition of Accidentals class
 *
 * This file is part of KGuitar, a KDE tabulature editor
 *
 * copyright (C) 2002-2003 the KGuitar development team
 ***************************************************************************/

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file COPYING for more information.
 ***************************************************************************/

#ifndef ACCIDENTALS_H
#define ACCIDENTALS_H

class QString;

class Accidentals
{
public:
	enum Accid { None, Natural, Sharp, Flat };
	Accidentals();
	void addPitch(int pitch);
	void calcChord();
	int getKeySig();
	bool getNote(int pitch, QString& stp, int& alt, int& oct, Accid& acc);
	void resetToKeySig();
	int  sao2Pitch(const QString& stp, int alt, int oct);
	void setKeySig(int sig);
	void startChord();
private:
	void markInUse(int i, int nlh, Accid a);
	int  normalize(int pitch);
	static const int stPerOct = 12;	// # semitones (half steps) per octave
	int keySig;
	bool notes_av[stPerOct];		// notes available
	bool notes_req[stPerOct];		// notes requested for this chord
	Accid old_acc_state[stPerOct];	// accidental state for all notes, before
	Accid new_acc_state[stPerOct];	// and after calcChord
	int out_root_note[stPerOct];	// notes to be printed
	Accid out_accidental[stPerOct];	// accidentals to be printed
};

#endif
