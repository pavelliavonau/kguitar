#include "chordanalyzer.h"

#include <klocale.h>

ChordAnalyzer::ChordAnalyzer(QString name)
{
	// Memorize name
	this->name = name.replace(" ", "").replace("(", "").replace(")", "").upper();

	// Reset the steps info
	for (int i = 0; i < 6; i++)	 {
		step[i] = 0;
		fixed[i] = false;
	}
}

bool ChordAnalyzer::analyze()
{
	// C  Db D	Eb E  F	 F# G  G# A	 Bb B
	// 0  1	 2	3  4  5	 6	7  8  9	 10 11

	QChar tc = name[0];

	// Get tonic
	switch ((char) tc)  {
	case 'C': tonic = 0; break;
	case 'D': tonic = 2; break;
	case 'E': tonic = 4; break;
	case 'F': tonic = 5; break;
	case 'G': tonic = 7; break;
	case 'A': tonic = 9; break;
	case 'H':
	case 'B': tonic = 11; break; // GREYFIX: understand B differently if Jazz active
	default:
		msg = i18n("Can't understand tonic from given chord name");
		return FALSE;
	}

	pos = 1;

	// Try to fix tonic with sharps and flats
	while (name[pos] != QChar::null) {
		if (name[pos] == 'B')  {
			tonic = (tonic - 1) % 12;
		} else if (name[pos] == '#')  {
			tonic = (tonic + 1) % 12;
		} else {
			break;
		}
		pos++;
	}

	// Set default steps (major triad)
	step[0] = 3;
	step[1] = 2;

	// Main analyze loop - ! REMEMBER, UPPERCASE !
	while (name[pos] != QChar::null) {
		int oldpos = pos;

		// Check "maj7" for dominant seventh
		if (checkNext("MAJ7"))
			if (!setStep(2, 3, "maj7"))
				return FALSE;

		// Check "min" for minor third step
		if (checkNext("MIN"))
			if (!setStep(0, 2, "min"))
				return FALSE;

		// Check "m" for minor third step
		if (checkNext("M"))
			if (!setStep(0, 2, "m"))
				return FALSE;

		// Check "7M" for dominant seventh
		if (checkNext("7M"))
			if (!setStep(2, 3, "7M"))
				return FALSE;

		// Check "7" for minor seventh
		if (checkNext("7"))
			if (!setStep(2, 2, "7"))
				return FALSE;

		// Check "6" for sixth
		if (checkNext("6"))
			if (!setStep(2, 1, "6"))
				return FALSE;

		// Check "sus2" for second step
		if (checkNext("SUS2"))
			if (!setStep(0, 1, "sus2"))
				return FALSE;

		// Check "s2" for second step
		if (checkNext("S2"))
			if (!setStep(0, 1, "s2"))
				return FALSE;

		// Check "sus4" for second step
		if (checkNext("SUS4"))
			if (!setStep(0, 4, "sus4"))
				return FALSE;

		// Check "s4" for second step
		if (checkNext("S4"))
			if (!setStep(0, 4, "s4"))
				return FALSE;

		// Check "no3" for no third step
		if (checkNext("NO3"))
			if (!setStep(0, 0, "no3"))
				return FALSE;

		// Check "no5" for no fifth step
		if (checkNext("NO5"))
			if (!setStep(1, 0, "no5"))
				return FALSE;

		// Check "aug" for augmented chord
		if (checkNext("AUG"))
			if (!setStep(1, 3, "aug"))
				return FALSE;

		// Check "dim"
		if (checkNext("DIM"))  {
			if (!setStep(0, 2, "dim"))
				return FALSE;
			if (!setStep(1, 1, "dim"))
				return FALSE;
		}

		// Check "5" power chord
		if (checkNext("5"))  {
			if (!setStep(0, 0, "5"))
				return FALSE;
			if (!setStep(1, 2, "5"))
				return FALSE;
			if (!setStep(2, 0, "5"))
				return FALSE;
			if (!setStep(3, 0, "5"))
				return FALSE;
			if (!setStep(4, 0, "5"))
				return FALSE;
			if (!setStep(5, 0, "5"))
				return FALSE;
		}

		// Stumbled across unrecognizable symbol
		if (oldpos == pos) {
			msg = i18n("Can't understand notation: \"%1\"").arg(name.mid(pos));
			return FALSE;
		}
	}

	return TRUE;
}

// Checks if the sample string comes next in the line of analyze
bool ChordAnalyzer::checkNext(QString sample)
{
	for (uint i = 0; i < sample.length(); i++)
		if (name[pos + i] != sample[i])
			return FALSE;

	pos += sample.length();
	return TRUE;
}

// Sets and fixes the step "index" to value "value", if still not
// fixed. If already fixed, bails out complaining about "reason"
bool ChordAnalyzer::setStep(int index, int value, QString reason)
{
	if (fixed[index] && step[index] != value) {
		msg = i18n("Modifier \"%1\" can't be used here because of previous symbols in chord name").arg(reason);
		return FALSE;
	} else {
		step[index] = value;
		fixed[index] = TRUE;
		return TRUE;
	}
}
