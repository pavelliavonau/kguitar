#include "tabtrack.h"

TabTrack::TabTrack(TrackMode _tm, QString _name, int _channel,
				   int _bank, uchar _patch, char _string, char _frets)
{
	tm=_tm;
	name=_name;
	channel=_channel;
	bank=_bank;
	patch=_patch;
	string=_string;
	frets=_frets;

	// Simple & brutal initialization follows
	// GREYFIX: change to something that makes more sense

	uchar standtune[6] = {40, 45, 50, 55, 59, 64};

	for (int i = 0; i < 6; i++)
		tune[i] = standtune[i];

	c.resize(1);
	b.resize(1);

	for (int i = 0; i < MAX_STRINGS; i++) {
		c[0].a[i] = -1;
		c[0].e[i] = 0;
	}
	c[0].l = 120;
	c[0].flags = 0;

	b[0].start = 0;
	b[0].time1 = 4;
	b[0].time2 = 4;

	x = 0;
	xb = 0;
	y = 0;

	sel = FALSE;
	xsel = 0;
}

// Pretty sophisticated expression that determines if we can omit the time sig
bool TabTrack::showBarSig(int n)
{
	return !((n > 0) &&
			 (b[n - 1].time1 == b[n].time1) &&
			 (b[n - 1].time2 == b[n].time2));
}

// Returns the column that ends bar <n>. Thus bar <n> is all columns
// from b[n].start to lastColumn(n) inclusive
int TabTrack::lastColumn(int n)
{
	int last;
	if (b.size() == n + 1)       // Current bar is the last one
		last = c.size() - 1;      // Draw till the last note
	else							    // Else draw till the end of this bar
		last = b[n + 1].start - 1;
	if (last == -1)  last = 0;          // gotemfix: avoid overflow
	return last;
}

// Returns bar status - what to show in track pane
bool TabTrack::barStatus(int n)
{
	if (n >= b.size())
		return FALSE;

	bool res = FALSE;

	for (int i = b[n].start; i <= lastColumn(n); i++) {
		for (int k = 0; k < string; k++) {
			if (c[i].a[k] != -1) {
				res = TRUE;
				break;
			}
		}
		if (res)
			break;
	}

	return res;
}

// Inserts n columns at current cursor position
void TabTrack::insertColumn(int n)
{
	c.resize(c.size() + n);
	for (int i = c.size() - n; i > x; i--)
		c[i] = c[i - n];
	for (int i = 0; i < n; i++)
		for (int j = 0; j < MAX_STRINGS; j++)
			 c[x + i].a[j] = -1;
}

// Removes n columns starting with current cursor position
void TabTrack::removeColumn(int n)
{
	for (int i = x; i < c.size() - n; i++)
		c[i]=c[i+n];

	// Remove empty bars
	while (c.size()-n<=b[b.size()-1].start)
		b.resize(b.size()-1);

	c.resize(c.size()-n);

	if (x >= c.size())
		x = c.size() - 1;

	if (xb >= b.size())
		xb = b.size() - 1;
}

// Toggles effect FX on current cursor position
void TabTrack::addFX(char fx)
{
	if (c[x].a[y]>=0) {
		if (c[x].e[y] != fx)
			c[x].e[y] = fx;
		else
			c[x].e[y] = 0;
	}
}

// Find what measure the cursor is in by searching measure list
void TabTrack::updateXB()
{
	if (x>=b[b.size()-1].start)
		xb = b.size()-1;
	else
		for (int i=0; i<b.size()-1; i++)
			if ((x>=b[i].start) && (x<b[i+1].start)) {
				xb = i;
				break;
			}
}

// Add column "dat" to the end of the track, making this column span
// as many real columns as needed to sum up to duration of "value"

void TabTrack::addNewColumn(TabColumn dat, int value, bool *arc)
{
	// Cleverly sorted lookup array of all possible "standard" note durations
	// Norm  Dot  Triplet
	const int bits[] = {
		     720,
		480, 360, 320,
		240, 180, 160,
		120, 90,  80,
		60,  45,  40,
		30,  23,  20,
		15,       10,
		0
	};

	int toput = value; // What's left to distribute

	while (toput > 0) {
		int bit = toput;

		for (int i = 0; bits[i]; i++)
			if (toput >= bits[i]) {
				bit = bits[i];
				break;
			}

		toput -= bit;

		int last = c.size();
		c.resize(last + 1);
		c[last] = dat;
		c[last].setFullDuration(bit);
		if (*arc) {
			c[last].flags |= FLAG_ARC;
			for (int i = 0; i < MAX_STRINGS; i++)
				c[last].a[i] = -1;
		}
		*arc = TRUE;
	}
}

// Arrange the track: collect all columns into an additional array,
// then add them one by one to clean track, taking care about bar
// limits

void TabTrack::arrangeBars()
{
	int barnum = 1;

	// COLLECT ALL NOTES INFORMATION

	QArray<TabColumn> an;			// Collected columns information
	int nn = 0;						// Number of already made columns

	for (int i = 0; i < c.size(); i++) {
		if (!(c[i].flags & FLAG_ARC)) { // Add the note, if it's not arc
			nn++;
			an.resize(nn);
			an[nn-1] = c[i];
			an[nn-1].l = c[i].fullDuration();
		} else {						// Add to duration of previous note
			an[nn-1].l += c[i].fullDuration();
		}
	}

	// RECONSTRUCTING BARS & COLUMNS ARRAYS

	bool arc;
	int cl;
	int cbl = 480 * b[0].time1 / b[0].time2;

	b[0].start = 0;
	barnum = 0;

	c.resize(0);

	for (nn = 0; nn < an.size(); nn++) {
		cl = an[nn].l;
		arc = FALSE;

		while (cl > 0) {
			if (cl < cbl) {
				addNewColumn(an[nn], cl, &arc);
				cbl -= cl;
				cl = 0;
			} else {
				addNewColumn(an[nn], cbl, &arc);
				cl -= cbl;

				barnum++;
				if (b.size() < barnum + 1) {
					b.resize(barnum + 1);
					b[barnum].time1 = b[barnum-1].time1;
					b[barnum].time2 = b[barnum-1].time2;
				}
				b[barnum].start = c.size();
				cbl = 480 * b[barnum].time1 / b[barnum].time2;
			}
		}
	}

	// Clean up last bar if it's empty
	if (b[barnum].start == c.size())
		b.resize(barnum);

	// Make sure that cursor x is in legal range
	if (x >= c.size())
		x = c.size() - 1;

	// Find the bar the cursor is in
	updateXB();
}
