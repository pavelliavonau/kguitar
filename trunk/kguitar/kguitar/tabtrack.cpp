#include "accidentals.h"
#include "globaloptions.h"
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

// Returns the bar number for column c
int TabTrack::barNr(int c)
{
	int i = 0;
	int res = -1;
	while (i < b.size()) {
		if (i + 1 >= b.size()) {
			if (b[i].start <= c)
				break;
		} else {
			if ((b[i].start <= c) && (c < b[i+1].start))
				break;
		}
		i++;
	}
	// LVIFIX: what to do if c < 0 ?
	if (c < 0)
		res = -1;
	else
		res = i;
	return res;
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

// Returns a cumulative duration of all columns that belong to current
// bar, determined with track parameters
Q_UINT16 TabTrack::currentBarDuration()
{
	Q_UINT16 dur = 0;
	for (int i = b[xb].start; i <= lastColumn(xb); i++)
		dur += c[i].fullDuration();
	return dur;
}

// Calculates and returns maximum current bar duration, based on time
// signature
Q_UINT16 TabTrack::maxCurrentBarDuration()
{
	return 4 * 120 * b[xb].time1 / b[xb].time2;
}

// Returns the duration of the note in column t and string i
// Ringing and bar end are taken into account
Q_UINT16 TabTrack::noteDuration(uint t, int i)
{
	Q_UINT16 dur = 0;
	for (int j = 0; j < noteNrCols(t, i); j++)
		dur += c[t + j].fullDuration();
	return dur;
}

// Returns the number of columns used by the note in column t and string i
// Ringing and bar end are taken into account
// LVIFIX: returns 0 for a "rest" column
int TabTrack::noteNrCols(uint t, int i)
{
	if ((t >= c.size()) || (i < 0) || (i >= string)) {
		return 0;
	}
	if (c[t].a[i] == NULL_NOTE) {
		return 0;
	}
	if (c[t].e[i] == EFFECT_LETRING) {
		int b  = barNr(t);
		int lc = lastColumn(b);	// last column of current bar
		// ringing: find next note, stop ringing or end of bar
		if (t == lc) {
			// note is in last column of bar
			return 1;
		}
		int cc = t + 1;			// current column
		while ((cc < lc)
				&& (c[cc].a[i] == NULL_NOTE)
				&& (c[cc].e[i] != EFFECT_STOPRING))
			cc++;
		int res = cc - t;
		// if necessary, add last column of bar
		if ((cc == lc)
			&& (c[cc].a[i] == NULL_NOTE)
			&& (c[cc].e[i] != EFFECT_STOPRING))
				res++;
		return res;
	} else {
		// not ringing: length is one column
		return 1;
	}
	return 0;
}

// Inserts n columns at current cursor position
void TabTrack::insertColumn(int n)
{
	c.resize(c.size() + n);
	for (int i = c.size() - n; i > x; i--)
		c[i] = c[i - n];
	for (int i = 0; i < n; i++)
		for (int j = 0; j < MAX_STRINGS; j++) {
			 c[x + i].a[j] = -1;
			 c[x + i].e[i] = 0;
		}
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
// Stop ringing is the only effect allowed when no note is played at x,y
void TabTrack::addFX(char fx)
{
	if ((c[x].a[y]>=0) || ((c[x].a[y]==NULL_NOTE) && (fx==EFFECT_STOPRING))) {
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

	QMemArray<TabColumn> an;        // Collected columns information
	int nn = 0;                     // Number of already made columns

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

#ifdef WITH_TSE3
// Generate a single midi data list for TSE3 from all current track
// tabulature.
TSE3::PhraseEdit *TabTrack::midiTrack()
{
	TSE3::PhraseEdit *midi = new TSE3::PhraseEdit();

	// Initial setup, patches, midi volumes, choruses, etc.
	midi->insert(TSE3::MidiEvent(TSE3::MidiCommand(TSE3::MidiCommand_ProgramChange,
												   channel - 1, globalMidiPort, patch), 0));

	long timer = 0;
	int midilen = 0, duration;
	uchar pitch;

	for (uint x = 0; x < c.size(); x++) {
		// Calculate real duration (including all the linked beats)
		midilen = c[x].fullDuration();
		while ((x + 1 < c.size()) && (c[x + 1].flags & FLAG_ARC)) {
			x++;
			midilen += c[x].fullDuration();
		}

		// Note on/off events
		for (int i = 0; i < string; i++) {
			if (c[x].a[i] == -1)  continue;

			if (c[x].a[i] == DEAD_NOTE) {
				pitch = tune[i];
				duration = 5;
			} else {
				pitch = c[x].a[i] + tune[i];
				duration = midilen;
			}

			if (c[x].flags & FLAG_PM)
				duration = duration / 2;

			if (c[x].e[i] == EFFECT_ARTHARM)
				pitch += 12;
			if (c[x].e[i] == EFFECT_HARMONIC) {
				switch (c[x].a[i]) {
				case 3:  pitch += 28; break;
				case 4:  pitch += 24; break;
				case 5:  pitch += 19; break;
				case 7:  pitch += 12; break;
				case 9:  pitch += 19; break;
				case 12: pitch += 0;  break;
				case 16: pitch += 12; break;    // same as 9th fret
				case 19: pitch += 0;  break;	// same as 7th fret
				case 24: pitch += 0;  break;    // same as 5th fret
				}
			}

			midi->insert(TSE3::MidiEvent(TSE3::MidiCommand(TSE3::MidiCommand_NoteOn, channel - 1,
			                                               globalMidiPort, pitch, 0x60),
			                             timer, 0x60, timer + duration));
//			cout << "Inserted note pitch " << (int) pitch << ", start " << timer << ", duration " << duration << "\n";
		}
		timer += midilen;
	}
	return midi;
}
#endif

// Functions used to calculate TabColumn "volatile" data, that need access
// to more than one column.
// Used by MusicXML export and PostScript output.

// Determine step/alter/octave/accidental for each note

void  TabTrack::calcStepAltOct()
{
	int t;
	// initialize all data
	for (t = 0; t < c.size(); t++) {
		for (int i = 0; i < string; i++) {
			c[t].stp[i] = ' ';
			c[t].alt[i] = 0;
			c[t].oct[i] = 0;
			c[t].acc[i] = Accidentals::None;
		}
	}
	// calculate data for each bar
	for (uint bn = 0; bn < b.size(); bn++) {
		Accidentals accSt;
		accSt.resetToKeySig();
		// loop t over all columns in this bar and calculate saoa
		for (t = b[bn].start; (int) t <= lastColumn(bn); t++) {
			accSt.startChord();
			for (int i = 0; i < string; i++) {
				if (c[t].a[i] > -1) {
					accSt.addPitch(tune[i] + c[t].a[i]);
				}
			}
			accSt.calcChord();
			for (int i = 0; i < string; i++) {
				if (c[t].a[i] > -1) {
					Accidentals::Accid tmpacc = Accidentals::None;
					int tmpalt = 0;
					int tmpoct = 0;
					QString tmpnam = " ";
					accSt.getNote(tune[i] + c[t].a[i],
									tmpnam, tmpalt, tmpoct, tmpacc);
					c[t].stp[i] = tmpnam.at(0).latin1();
					c[t].alt[i] = tmpalt;
					c[t].oct[i] = tmpoct;
					c[t].acc[i] = tmpacc;
				}
			}
		}
	}
}

// Determine voices for each note

void  TabTrack::calcVoices()
{
	int t;
	// initialize all data
	for (t = 0; t < c.size(); t++) {
		for (int i = 0; i < string; i++) {
			c[t].v[i] = -1;
		}
	}

	// if only one single voice, allocate all notes to voice 1
	if (!hasMultiVoices()) {
		t = 0;
		while (t < c.size()) {
			for (int i = 0; i < string; i++) {
				if (c[t].a[i] != NULL_NOTE) {
					c[t].v[i] = 1;
				}
			}
			t++;
		}
		return;
	}

	// loop through track and allocate voice 0
	t = 0;
	while (t < c.size()) {
		// find all lowest notes of equal length in this column
		int  lntlen = 0;		// lowest note length
		for (int i = 0; i < string; i++) {
			if (c[t].a[i] != NULL_NOTE) {
				if (lntlen == 0) {
					// lowest note in this column, allocate to voice 0
					c[t].v[i] = 0;
					lntlen = noteNrCols(t, i);
				} else {
					if (lntlen == noteNrCols(t, i)) {
						// same length as lowest note in this column,
						// allocate to voice 0
						c[t].v[i] = 0;
					}
				}
			}
		}
		// move to next note
		if (lntlen == 0) {
			t++;
		} else {
			t += lntlen;
		}
	}
	// loop through track again and allocate remaining notes to voice 1
	// LVIFIX: cannot handle more than two voices:
	// print error when more voices are found
	t = 0;
	while (t < c.size()) {
		for (int i = 0; i < string; i++) {
			if ((c[t].a[i] != NULL_NOTE) && (c[t].v[i] == -1)) {
				c[t].v[i] = 1;
			}
		}
		t++;
	}
	// in multiple voice mode, if a column contains more than one notes
	// in voice 0, but none in voice 1, then allocate all but lowest note
	// to voice 1
	if (hasMultiVoices()) {
		t = 0;
		while (t < c.size()) {
			int v0 = 0;			// number of notes in voice 1
			int v1 = 0;			// number of notes in voice 1
			for (int i = 0; i < string; i++) {
				if (c[t].v[i] == 0) {
					v0++;
				}
				if (c[t].v[i] == 1) {
					v1++;
				}
			}
			if ((v0 > 1) && (v1 == 0)) {
				int na = 0;		// number of allocated notes
				for (int i = 0; i < string; i++) {
					if (c[t].a[i] != NULL_NOTE) {
						if (na == 0) {
							c[t].v[i] = 0;
						} else {
							c[t].v[i] = 1;
						}
						na++;
					}
				}
			}
			t++;
		} // while (t ...
	}
}

// return true if track has multiple voices
// i.e. at least one note is ringing

bool TabTrack::hasMultiVoices()
{
	for (int t = 0; t < c.size(); t++) {
		for (int i = 0; i < string; i++) {
			if (c[t].e[i] == EFFECT_LETRING) {
				return true;
			}
		}
	}
	return false;
}

// determine if d equals an exact note duration

bool TabTrack::isExactNoteDur(int d)
{
	switch (d) {
	case  15: return true;
	case  30: return true;
	case  60: return true;
	case 120: return true;
	case 240: return true;
	case 480: return true;
	}
	return false;
}

// get note head type tp and number of dots dt for column t in voice v
// if no note in voice v, then return false
// if note found but no valid type/dot combination, then return true,
// but tp = dt = 0;

bool TabTrack::getNoteTypeAndDots(int t, int v, int & tp, int & dt)
{
	// defaults: no note, no dots
	tp = 0;
	dt = 0;
	// find a note in voice v
	int i;
	for (i = string-1; i >= 0; i--) {
		if ((c[t].a[i] != NULL_NOTE)
			&& (c[t].v[i] == v)) {
			break;
		}
	}
	if (i == -1) {
		// no note in this voice
		return false;
	}
	int dur = noteDuration(t, i);
	// try no dots
	tp = dur;
	dt = 0;
	if (isExactNoteDur(tp)) {
		return true;
	}
	// try one dot (duration = type * 3 / 2)
	tp = dur * 2 / 3;
	dt = 1;
	if (isExactNoteDur(tp)) {
		return true;
	}
	// try two dots (duration = type * 7 / 4)
	tp = dur * 4 / 7;
	dt = 2;
	if (isExactNoteDur(tp)) {
		return true;
	}

	// no valid note type / dot combination found
	tp = 0;
	dt = 0;
	return true;
}
