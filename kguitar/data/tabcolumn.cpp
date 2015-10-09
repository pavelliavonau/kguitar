#include "tabcolumn.h"

// Gets full real duration of a column, including all effects caused
// by dots, triplets, etc

Q_UINT16 TabColumn::fullDuration()
{
	Q_UINT16 len = l;
	if (flags & FLAG_DOT)  len += len / 2;
	if (flags & FLAG_TRIPLET)  len = len * 2 / 3;
	return len;
}

// Sets the dots, triplets, base duration, etc, based on full
// duration, deriving right combination via trying several ones

void TabColumn::setFullDuration(Q_UINT16 len)
{
	int test = 480;

	flags &= (uint(-1) - FLAG_DOT - FLAG_TRIPLET);

	for (int i = 0; i < 6; i++) {
		if (test == len) {                  // Try normal duration first
			l = len;
			return;
		}
		if (test * 3 / 2 == len) {          // Try dotted duration (x1.5)
			l = len * 2 / 3;
			flags |= FLAG_DOT;
			return;
		}
		if (test * 2 / 3 == len) {          // Try triplet duration (x2/3)
			l = len * 3 / 2;
			flags |= FLAG_TRIPLET;
			return;
		}

		test /= 2;
	}

	kdDebug() << "Very strange full duration: " << len << ", can't detect, using 120" << endl;
	l = 120;
}

// Gives all flags except ones that affect duration

uint TabColumn::effectFlags()
{
	return flags & (uint(-1) - FLAG_DOT - FLAG_TRIPLET);
}
