#include "global.h"

#include "convertxml.h"
#include "musicxml.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qvaluelist.h>

// GREYFIX
#include <iostream>
using namespace std;

ConvertXml::ConvertXml(TabSong *song): ConvertBase(song), QXmlDefaultHandler()
{
}

bool ConvertXml::save(QString fileName)
{
	// Initialize output stream
	QFile f(fileName);
	if (!f.open(IO_WriteOnly))
		return FALSE;
	QTextStream s(&f);
	write(s);

	f.close();

	return TRUE;
}

bool ConvertXml::load(QString fileName)
{
	MusicXMLErrorHandler errHndlr;
	QFile xmlFile(fileName);
	QXmlInputSource source(xmlFile);
	QXmlSimpleReader reader;
	reader.setContentHandler(this);
	reader.setErrorHandler(&errHndlr);
	errHndlr.setParser(this);
	reader.parse(source);

    return TRUE;
}

// local conversion functions

// convert KGuitar notelength to MusicXML note type

static QString kgNoteLen2Mxml(const int kgNoteLen)
{
	switch (kgNoteLen)
	{
	case 480:
		return "whole";
	case 240:
		return "half";
	case 120:
		return "quarter";
	case  60:
		return "eighth";
	case  30:
		return "16th";
	case  15:
		return "32th";
	default:
		return "";
	}
}


// convert MusicXML string number to KGuitar string number
// this transformation is symmetrical, but depends on # strings
//
// bass(5):  EADGB
// MusicXML: 54321
// KGuitar:  01234
//
// guitar:   EADGBE
// MusicXML: 654321
// KGuitar:  012345

static int mxmlStr2Kg(const int mxmlStr, const int nStrings)
{
	return nStrings - mxmlStr;
}

// convert MusicXML note type to KGuitar notelength

static int mxmlNoteType2Kg(const QString& mxmlNoteType)
{
	if (mxmlNoteType == "whole") {
		return 480;
	} else if (mxmlNoteType == "half") {
		return 240;
	} else if (mxmlNoteType == "quarter") {
		return 120;
	} else if (mxmlNoteType == "eighth") {
		return  60;
	} else if (mxmlNoteType == "16th") {
		return  30;
	} else if (mxmlNoteType == "32th") {
		return  15;
	} else {
		return 0;
	}
}

// given pitch, allocate string/fret
// might also come in handy in the MIDI load function
// in:  pitch, tabtrack, current column
// out: result (TRUE=OK), (mxml)string, fret

// simple version:
// will only allocate notes on the highest possible string
// no conflict solving capabilities
// example: an A will always be allocated on the A string
// if a B is also needed, this will fail
// possible solutions would include:
// put B on 7th fret on E string
// put A on 5th fret on E string, B on 2nd fret on A string

// strange things will happen if strings are tuned in reverse order

static bool allocStrFrt(int pitch, TabTrack * trk, int col,
                        unsigned int& str, unsigned int& frt)
{
	// remove some boundary conditions
	// if no strings at all, we're out of luck
	if (trk->string == 0) {
		return FALSE;
	}
	// if pitch is lower than lowest string, we're out of luck
	if (pitch < trk->tune[0]) {
		return FALSE;
	}

	int kgStr;
	if (trk->string == 1) {
		// must be on the one-and-only string
		kgStr = 0;
	} else {
		// assume on highest string
		kgStr = (trk->string)-1;
		// but search for lower string to use
		// note that # strings >= 2
		for (int i = 0; i < (trk->string)-1; i++) {
			if ((trk->tune[i] <= pitch) && (pitch < trk->tune[i+1])) {
				kgStr = i;
			}
		}
	}

	// check string not in use
	if (trk->c[col].a[kgStr] >= 0) {
		return FALSE;
	}

	str = mxmlStr2Kg(kgStr, trk->string);
	frt = pitch - trk->tune[kgStr];
	return TRUE;
}

// helpers for NMusicXMLExport::calcDivisions

typedef QValueList<int> IntVector;
static IntVector integers;
static IntVector primes;

static void addInt(int len) {
	IntVector::Iterator it = integers.find(len);
	if (it == integers.end()) {
		integers.append(len);
	}
}
	
// check if all integers can be divided by div

static bool canDivideBy(int div) {
	bool res = true;
	for (unsigned int i = 0; i < integers.count(); i++) {
		if ((integers[i] <= 1) || ((integers[i] % div) != 0)) {
			res = false;
		}
	}
	return res;
}

// divide all integers by div

static void divideBy(int div) {
	for (unsigned int i = 0; i < integers.count(); i++) {
		integers[i] /= div;
	}
}

// Loop over all voices in all staffs and determine a suitable value for divisions.

// Length of time in MusicXML is expressed in "units", which should allow expressing all time values
// as an integral number of units. Divisions contains the number of units in a quarter note.
// The way KGuitar stores note length meets this requirement, but sets divisions to a very
// large number: length of a quarter note is 120. Solution is to collect all time values required,
// and divide them by the highest common denominator, which is implemented as a series of
// divisions by prime factors. Initialize the list with 120 to make sure a quarter note can always
// be written as an integral number of units.

void ConvertXml::calcDivisions() {
//	cout << "ConvertXml::calcDivisions()" << endl;
	IntVector::Iterator it;

	// init
	integers.clear();
	primes.clear();
	integers.append(120);		// quarter note length
	primes.append(2);
	primes.append(3);
	primes.append(5);
	primes.append(7);		// initialize with required prime numbers

// need to use note and rest duration as exported to MusicXML
// thus match ConvertXml::write's main loop

	TabTrack *trk;
	// loop over all tracks
	for (unsigned int it = 0; it < song->t.count(); it++) {
		trk = song->t.at(it);
		trk->calcVoices();	// LVIFIX: is this necessary ?
//		cout << "part id=P" << it+1 << endl;

		// loop over all bars
		for (uint ib = 0; ib < trk->b.size(); ib++) {
//			cout << "measure number=" << ib + 1 << endl;

			// loop over all voices in this bar
			for (int i = 0; i < 2; i++) {
				// write only voice 1 in single voice tracks,
				// write all voices in multi voice tracks
				if ((i == 1) || trk->hasMultiVoices()) {
//					cout << "voice number=" << i + 1 << endl;
					// loop over all columns in this bar
					for (int x = trk->b[ib].start;
							x <= trk->lastColumn(ib); /* nothing */) {
/*
						int tp;
						int dt;
						bool tr;
						bool res;
						res = trk->getNoteTypeAndDots(x, i, tp, dt, tr);
							// LVIFIX: error handling ?
							// false means no note in this column/voice
							// LVIFIX: add rest handling (see writecol)
						cout
							<< "x=" << x
							<< " res=" << res
							<< " tp=" << tp
							<< " dt=" << dt
							<< " tr=" << tr
							<< endl;
*/
						QTextStream dummy;
						x += writeCol(dummy, trk, x, i, false);
					} // end for (uint x = 0; ....
				} // end if ((i == 1) || ...
			} // end for (int i = 0; ...

		} // end for (uint ib = 0; ...

	} // end for (unsigned int it = 0; ...

	// do it: divide by all primes as often as possible
	for (unsigned int u = 0; u < primes.count(); u++) {
		while (canDivideBy(primes[u])) {
			divideBy(primes[u]);
		}
	}

//	cout << "res=" << integers[0] << endl;
	divisions = integers[0];
}


// write tabsong to QTextStream os

void ConvertXml::write(QTextStream& os)
{
	calcDivisions();
	os << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
	   << endl;
	os << "<!DOCTYPE score-partwise PUBLIC" << endl;
	os << "    \"-//Recordare//DTD MusicXML 1.0 Partwise//EN\"" << endl;
	os << "    \"http://www.musicxml.org/dtds/partwise.dtd\">" << endl;
	os << endl;
	os << "<score-partwise>\n";
	os << "\t<work>\n";
	os << "\t\t<work-title>" << song->title << "</work-title>\n";
	os << "\t</work>\n";

// identification
	os << "\n";
	os << "\t<identification>\n";
	os << "\t\t<creator type=\"composer\">" << song->author << "</creator>\n";
	os << "\t\t<encoding>\n";
	os << "\t\t\t<encoder>" << song->transcriber << "</encoder>\n";
	os << "\t\t\t<software>KGuitar</software>\n";
	os << "\t\t</encoding>\n";
	os << "\t</identification>\n";

// part list
	os << "\n";
	os << "\t<part-list>\n";
	// loop over all tracks
	for (unsigned int it = 0; it < song->t.count(); it++) {
		os << "\t\t<score-part id=\"P" << it+1 << "\">\n";
		os << "\t\t\t<part-name>" << song->t.at(it)->name << "</part-name>\n";
		// LVIFIX: fill-in real instrument-name instead of "Guitar"
		// note: in DTD 0.6 score-instrument may appear zero or more times
		//       within a score-part
		// note: in DTD 0.7a score-instrument apparently is required
		os << "\t\t\t<score-instrument id=\"P" << it+1
		   << "-I" << it+1 << "\">\n";
		os << "\t\t\t\t<instrument-name>" << "Guitar"
		   << "</instrument-name>\n";
		os << "\t\t\t</score-instrument>\n";
		os << "\t\t\t<midi-instrument id=\"P" << it+1
		   << "-I" << it+1 << "\">\n";
		os << "\t\t\t\t<midi-channel>" << song->t.at(it)->channel
		   << "</midi-channel>\n";
		os << "\t\t\t\t<midi-bank>" << song->t.at(it)->bank
		   << "</midi-bank>\n";
		os << "\t\t\t\t<midi-program>" << song->t.at(it)->patch
		   << "</midi-program>\n";
		os << "\t\t\t</midi-instrument>\n";
		os << "\t\t</score-part>\n";
	} // end for (unsigned int it = 0; ...
	os << "\t</part-list>\n";

// parts
	TabTrack *trk;
	// loop over all tracks
	for (unsigned int it = 0; it < song->t.count(); it++) {
		trk = song->t.at(it);
		trk->calcVoices();
		trk->calcStepAltOct();
		trk->calcBeams();
		os << "\n";
		os << "\t<part id=\"P" << it+1 << "\">\n";

		// loop over all bars
		for (uint ib = 0; ib < trk->b.size(); ib++) {
			os << "\t\t<measure number=\"" << ib + 1 << "\">\n";
			if (ib == 0) {
				// First bar: write all attributes
				os << "\t\t\t<attributes>\n";
				os << "\t\t\t\t<divisions>" << divisions << "</divisions>\n";
				os << "\t\t\t\t<key>\n";
				os << "\t\t\t\t\t<fifths>" << trk->b[0].keysig << "</fifths>\n";
				// LVIFX: re-enable when KGuitar supports major/minor modes
				// os << "\t\t\t\t\t<mode>major</mode>\n";
				os << "\t\t\t\t</key>\n";
				writeTime(os, trk->b[0].time1, trk->b[0].time2);
				os << "\t\t\t\t<staves>2</staves>\n";
				os << "\t\t\t\t<clef number=\"1\">\n";
				os << "\t\t\t\t\t<sign>G</sign>\n";
				os << "\t\t\t\t\t<line>2</line>\n";
				os << "\t\t\t\t\t<clef-octave-change>-1</clef-octave-change>\n";
				os << "\t\t\t\t</clef>\n";
				os << "\t\t\t\t<clef number=\"2\">\n";
				os << "\t\t\t\t\t<sign>TAB</sign>\n";
				os << "\t\t\t\t\t<line>5</line>\n";
				os << "\t\t\t\t</clef>\n";
				writeStaffDetails(os, trk);
				os << "\t\t\t</attributes>\n";
				os << "\t\t\t<sound tempo=\"" << song->tempo << "\"/>\n";
			} else {
				// LVIFIX write time sig if changed
			}

			// loop over all voices in this bar
			for (int i = 0; i < 2; i++) {
				// write only voice 1 in single voice tracks,
				// write all voices in multi voice tracks
				if ((i == 1) || trk->hasMultiVoices()) {
					// loop over all columns in this bar
					for (int x = trk->b[ib].start;
							x <= trk->lastColumn(ib); /* nothing */) {
/*
						int tp;
						int dt;
						bool tr;
						if (!trk->getNoteTypeAndDots(x, i, tp, dt, tr)) {
							// LVIFIX: error handling ?
						}
*/
						x += writeCol(os, trk, x, i, true);
					} // end for ((i == 1) || ....
				} // end if (trk->hasMulti ...
			} // end for (int i = 0; ...

			os << "\t\t</measure>\n";
			os << "\n";
		} // end for (uint ib = 0; ...

		os << "\t</part>\n";
	} // end for (unsigned int it = 0; ...
	os << "\n";
	os << "</score-partwise>\n";
}

// write accidental of midi note n to QTextStream os

QString ConvertXml::strAccid(Accidentals::Accid acc)
{
	QString s;
	switch (acc) {
		case Accidentals::Natural: s = "natural"; break;
		case Accidentals::Sharp:   s = "sharp";   break;
		case Accidentals::Flat:    s = "flat";    break;
		default:                   s = "unknown"; break;
	}
	return s;
}

// write beam bm at level l to QTextStream os

static void writeBeam(QTextStream& os, int l, char bm)
{
	if (bm == 'n') {
		return;
	}
	os << "\t\t\t\t<beam number=\"";
	os << l << "\">";
	switch (bm) {
		case 'b': os << "backward hook"; break;
		case 'c': os << "continue";      break;
		case 'e': os << "end";           break;
		case 'f': os << "forward hook";  break;
		case 's': os << "begin";         break;
		default:  /* nothing */ ;        break;
	}
	os << "</beam>\n";
}

// write beams of voice v of column x of TabTrack trk to QTextStream os

void ConvertXml::writeBeams(QTextStream& os, TabTrack * trk, int x, int v)
{
	StemInfo * stxt = 0;
	if (v == 0) {
		stxt = & trk->c[x].stl;
	} else {
		stxt = & trk->c[x].stu;
	}
	/*
	kdDebug()
		<< "writeBeams()"
		<< " x=" << x
		<< " v=" << v
		<< " l1..3=" << stxt->l1 << stxt->l2 << stxt->l3 << endl;
	*/
	writeBeam(os, 1, stxt->l1);
	writeBeam(os, 2, stxt->l2);
	writeBeam(os, 3, stxt->l3);
}

// write voice v of column x of TabTrack trk to QTextStream os
// if wrt is true then write MusicXML else calculate timing only
// update triplet state
// return ncols used

// writeCol must write rest:
// single voice: only in voice 1
// multi voice: only in voice 0
// LVIFIX: cause of this is that in a single voice track all notes
// are allocated to voice 1 to force stem up when printing

int ConvertXml::writeCol(QTextStream& os, TabTrack * trk, int x, int v, bool wrt)
{
	// debug: dump this column
	/*
	os << "x=" << x;
	os << " a[i]/e[i]/v[i]=";
	for (int i = 0; i < trk->string; i++) {
		os << " " << (int) trk->c[x].a[i]
		   << "/" << (int) trk->c[x].e[i]
		   << "/" << (int) trk->c[x].v[i];
	}
	os << " fl=" << (int) trk->c[x].flags;
	os << endl;
	*/
	// end debug: dump this column

	if (trk->b[trk->barNr(x)].start == x) {
		// do start of bar specific init
		tEndPrev  = 0;
		trpCnt    = 0;
		tStartCur = 0;
		if ((v != 0) && trk->hasMultiVoices()) {
			if (wrt) {
				// write backup
				os << "\t\t\t<backup>" << endl;
				os << "\t\t\t\t<duration>"
				   << trk->currentBarDuration() * divisions / 120
				   << "</duration>\n";
				os << "\t\t\t</backup>" << endl;
			} else {
				addInt(trk->currentBarDuration());
//				cout << "backup " << trk->currentBarDuration() << endl;
			}
		}
	}
	// debug info
	/*
	os << "tEndPrev=" << tEndPrev << " tStartCur=" << tStartCur;
	if (tStartCur - tEndPrev) {
		os << " -> forward: " << tStartCur - tEndPrev;
	}
	os << endl;
	*/
	// end debug info
	if (tStartCur - tEndPrev) {
		if (wrt) {
			os << "\t\t\t<forward>" << endl;
			os << "\t\t\t\t<duration>"
			   << (tStartCur - tEndPrev) * divisions / 120
			   << "</duration>\n";
			os << "\t\t\t\t<voice>" << v + 1 << "</voice>\n";
			os << "\t\t\t</forward>" << endl;
		} else {
			addInt(tStartCur - tEndPrev);
//			cout << "forward " << tStartCur - tEndPrev << endl;
		}
		tEndPrev = tStartCur;
	}

	int dots = 0;				// # dots
	bool triplet = false;		// triplet flag
	int duration = 0;			// note duration (incl. dot/triplet)
	int fret;
	int length = 0;				// note length (excl. dot/triplet)
	int nCols = 1;				// # columns used (default 1)
	int nNotes = 0;				// # notes printed in this column
	int nRests = 0;				// # rests printed in this column
	int res;

	// LVIFIX: error handling ?
	res = trk->getNoteTypeAndDots(x, v, length, dots, triplet);
/*
	if (!wrt) {
		cout
		<< "x=" << x
		<< " res=" << res
		<< " tp=" << length
		<< " dt=" << dots
		<< " tr=" << triplet
		<< endl;
	}
*/

	// tie handling:
	// if the next column is linked with this one, start a tie
	// if the this column is linked with the previous one, end a tie
	// LVIFIX:
	// KGuitar stores the second column of a tie as a rest (an empty column),
	// while MusicXML requires notes there. Therefore take the notes from the
	// previous column.
	// See also: songprint.cpp SongPrint::drawBar()
	// LVIFIX:
	// "previous" should be "first column of the set of tied columns"
	// (there may be more than two)
	bool tieStart = FALSE;
	bool tieStop  = FALSE;
	int  xt = x;				// x where tie starts
	if (((unsigned)(x+1) < trk->c.size()) && (trk->c[x+1].flags & FLAG_ARC)) {
		tieStart = TRUE;
	}
	if ((x > 0) && (trk->c[x].flags & FLAG_ARC)) {
		tieStop = TRUE;
		xt = x - 1;				// LVIFIX: handle more than one tie
	}

	// triplet handling:
	// - reset after third note of triplet
	// - count notes while inside triplet
	if (trpCnt >= 3) {
		trpCnt = 0;
	}
	if (triplet) {
		trpCnt++;
	}

	// print all notes
	for (int i = trk->string - 1; i >= 0 ; i--) {
		if ((trk->c[xt].a[i] > -1) && (trk->c[x].v[i] == v)) {
			nNotes++;
			if (nNotes == 1) {
				// first note: calc duration etc.
				// for regular column, include ringing etc.
				// for column linked to previous, use column length
				if (trk->c[x].flags & FLAG_ARC) {
					length = trk->c[x].l;
					duration = length;
					// LVIFIX: dot and triplet handling required here ?
					// E.g. use trk->getNoteTypeAndDots()
					dots = 0;
					triplet = false;
					nCols = 1;
				} else {
					duration = length;
					// LVIFIX: allow more than one dot ?
					if (dots) {
						duration = duration * 3 / 2;
					}
					if (triplet) {
						duration = duration * 2 / 3;
					}
					nCols = trk->noteNrCols(x, i);
				}
			}
			fret = trk->c[xt].a[i];
			// if this note has an effect legato, start slur
			// if previous note has an effect legato, stop slur
			// EFFECT_LEGATO means hammer-on/pull-off, depending on
			// if the next note's pitch is higher or lower than this note's
			// MusicXML requires both the <hammer-on>/<pull-off> and <slur>
			// EFFECT_SLIDE is assumed to mean a slide on the fretboard
			// (not a bottleneck slide). Use a <glissando> in MusicXML.
			bool legStart = (((unsigned)(x+1) < trk->c.size())
							  && (trk->c[x].e[i] == EFFECT_LEGATO));
			bool legStop  = ((x > 0)
							  && (trk->c[x-1].e[i] == EFFECT_LEGATO));
			QString legStartType;
			QString legStopType;
			if (((unsigned)(x+1) < trk->c.size())
				&& (trk->c[x].a[i] < trk->c[x+1].a[i])) {
				legStartType = "hammer-on";
			} else {
				legStartType = "pull-off";
			}
			if ((x > 0)
				&& (trk->c[x-1].a[i] < trk->c[x].a[i])) {
				legStopType = "hammer-on";
			} else {
				legStopType = "pull-off";
			}
			bool sliStart = (((unsigned)(x+1) < trk->c.size())
							  && (trk->c[x].e[i] == EFFECT_SLIDE));
			bool sliStop  = ((x>0)
							  && (trk->c[x-1].e[i] == EFFECT_SLIDE));
			if (wrt) {
				os << "\t\t\t<note>\n";
				if (nNotes > 1) {
					os << "\t\t\t\t<chord/>\n";
				}
				os << "\t\t\t\t<pitch>\n";
				os << "\t\t\t\t\t<step>" << trk->c[xt].stp[i] << "</step>\n";
				if (trk->c[xt].alt[i] != '\0')
					os << "\t\t\t\t\t<alter>" << (int) trk->c[xt].alt[i]
						<< "</alter>\n";
				os << "\t\t\t\t\t<octave>" << (int) trk->c[xt].oct[i]
					<< "</octave>\n";
				os << "\t\t\t\t</pitch>\n";
				os << "\t\t\t\t<duration>" << duration * divisions / 120 << "</duration>\n";
				if (tieStart) {
					os << "\t\t\t\t<tie type=\"start\"/>\n";
				}
				if (tieStop) {
					os << "\t\t\t\t<tie type=\"stop\"/>\n";
				}
				os << "\t\t\t\t<voice>" << v + 1 << "</voice>\n";
				os << "\t\t\t\t<type>" << kgNoteLen2Mxml(length) << "</type>\n";
				if (dots) {
					os << "\t\t\t\t<dot/>\n";
				}
				if (trk->c[x].acc[i] != Accidentals::None) {
					os << "\t\t\t\t<accidental>" << strAccid(trk->c[x].acc[i])
						<< "</accidental>\n";
				}
				if (trpCnt) {
					os << "\t\t\t\t<time-modification>\n";
					os << "\t\t\t\t\t<actual-notes>3</actual-notes>\n";
					os << "\t\t\t\t\t<normal-notes>2</normal-notes>\n";
					os << "\t\t\t\t</time-modification>\n";
				}
				if (v == 0) {
					os << "\t\t\t\t<stem>down</stem>\n";
				} else {
					os << "\t\t\t\t<stem>up</stem>\n";
				}
				writeBeams(os, trk, x, v);
				os << "\t\t\t\t<notations>\n";
				if (legStop) {
					os << "\t\t\t\t\t<slur type=\"stop\"/>\n";
				}
				if (legStart) {
					os << "\t\t\t\t\t<slur type=\"start\"/>\n";
				}
				if (sliStop) {
					os << "\t\t\t\t\t<glissando type=\"stop\"/>\n";
				}
				if (sliStart) {
					os << "\t\t\t\t\t<glissando type=\"start\""
							" line-type=\"solid\"/>\n";
				}
				if (tieStart) {
					os << "\t\t\t\t\t<tied type=\"start\"/>\n";
				}
				if (tieStop) {
				os << "\t\t\t\t\t<tied type=\"stop\"/>\n";
				}
				if (trpCnt == 1) {
					os << "\t\t\t\t\t<tuplet type=\"start\"/>\n";
				}
				if (trpCnt == 3) {
					os << "\t\t\t\t\t<tuplet type=\"stop\"/>\n";
				}
				os << "\t\t\t\t\t<technical>\n";
				os << "\t\t\t\t\t\t<string>" << mxmlStr2Kg(i, trk->string)
				   << "</string>\n";
				os << "\t\t\t\t\t\t<fret>" << fret << "</fret>\n";
				if (legStop) {
					os << "\t\t\t\t\t\t<" << legStopType
					   << " type=\"stop\"/>\n";
				}
				if (legStart) {
					os << "\t\t\t\t\t\t<" << legStartType
					   << " type=\"start\"/>\n";
				}
				os << "\t\t\t\t\t</technical>\n";
				os << "\t\t\t\t</notations>\n";
				os << "\t\t\t</note>\n";
			} else {
				addInt(duration);
//				cout << "note " << duration << endl;
			}
		}
	}
	// if no notes in this column, it is a rest
	// rests are printed:
	// single voice: only in voice 1
	// multi voice: only in voice 0
	if (nNotes == 0) {
		length = trk->c[x].l;
		// note scaling: quarter note = 48
		// LVIFIX: use divisions instead
		duration = length;
		// LVIFIX: dot and triplet handling required here ?
		if (trk->c[x].flags & FLAG_DOT) {
			duration = duration * 3 / 2;
		}
		if (trk->c[x].flags & FLAG_TRIPLET) {
			duration = duration * 2 / 3;
		}
		if (((v == 1) && !trk->hasMultiVoices())
			|| ((v == 0) && trk->hasMultiVoices())) {
			if (wrt) {
				os << "\t\t\t<note>\n";
				os << "\t\t\t\t<rest/>\n";
				os << "\t\t\t\t<duration>" << duration * divisions / 120 << "</duration>\n";
				os << "\t\t\t\t<voice>" << v + 1 << "</voice>\n";
				os << "\t\t\t\t<type>" << kgNoteLen2Mxml(length) << "</type>\n";
				if (trk->c[x].flags & FLAG_DOT) {
					os << "\t\t\t\t<dot/>\n";
				}
				os << "\t\t\t</note>\n";
			} else {
				addInt(duration);
//				cout << "rest " << duration << endl;
			}
			nRests++;
		}
	}

	tStartCur += duration;
	if (nNotes || nRests) {
		tEndPrev += duration;
	}

	if (trk->lastColumn(trk->barNr(x)) == x) {
		// end of bar specifics: forward if necessary
		if (v != 0) {
			// write forward
			// debug info
			/*
			os << "forward needed ? ";
			os << "tEndPrev=" << tEndPrev << " tStartCur=" << tStartCur;
			if (tStartCur - tEndPrev) {
				os << " -> forward: " << tStartCur - tEndPrev;
			}
			os << endl;
			*/
			// end debug info
			if (tStartCur - tEndPrev) {
				if (wrt) {
					os << "\t\t\t<forward>" << endl;
					os << "\t\t\t\t<duration>"
					   << (tStartCur - tEndPrev) * divisions / 120
					   << "</duration>\n";
					os << "\t\t\t\t<voice>" << v + 1 << "</voice>\n";
					os << "\t\t\t</forward>" << endl;
				} else {
					addInt(tStartCur - tEndPrev);
//					cout << "forward " << tStartCur - tEndPrev << endl;
				}
			}
		}
	}

	return nCols;
}

// write midi note number as step/alter/octave to QTextStream os

void ConvertXml::writePitch(QTextStream& os,
                                int n, QString tabs, QString prfx)
{
	int alt = 0;
	int oct = 0;
	Accidentals::Accid acc = Accidentals::None;
	QString nam = "";

	accSt.getNote(n, nam, alt, oct, acc);
	os << tabs << "<" << prfx << "step>" << nam
	   << "</" << prfx << "step>\n";
	if (alt) {
		os << tabs << "<" << prfx << "alter>" << alt
		   << "</" << prfx << "alter>\n";
	}
	os << tabs << "<" << prfx << "octave>" << oct
	   << "</" << prfx << "octave>\n";
}

// write staff details of TabTrack trk to QTextStream os

void ConvertXml::writeStaffDetails(QTextStream& os, TabTrack * trk)
{
	// note: writePitch uses accSt, which has to be initialized first
	// Initialize the accidentals
	accSt.resetToKeySig();
	// calculate accidentals
	accSt.startChord();
	for (int i = 0; i < trk->string; i++) {
		accSt.addPitch(trk->tune[i]);
	}
	accSt.calcChord();
	os << "\t\t\t\t<staff-details number=\"2\">\n";
	os << "\t\t\t\t\t<staff-type>alternate</staff-type>\n";
	os << "\t\t\t\t\t<staff-lines>" << (int) trk->string << "</staff-lines>\n";
	for (int i = 0; i < trk->string; i++) {
		os << "\t\t\t\t\t<staff-tuning line=\""
		   << i + 1 << "\">\n";
		writePitch(os, trk->tune[i], "\t\t\t\t\t\t", "tuning-");
		os << "\t\t\t\t\t</staff-tuning>\n";
	}
	os << "\t\t\t\t</staff-details>\n";
}

// write time signature to QTextStream os

void ConvertXml::writeTime(QTextStream& os, int bts, int btt)
{
	os << "\t\t\t\t<time>\n";
	os << "\t\t\t\t\t<beats>" << bts << "</beats>\n";
	os << "\t\t\t\t\t<beat-type>" << btt << "</beat-type>\n";
	os << "\t\t\t\t</time>\n";
}

// ----------------------------------------------------------

// Class ConvertXml

// set document locator handler

void ConvertXml::setDocumentLocator(QXmlLocator *locator)
{
	lctr = locator;
}

// start of document handler

bool ConvertXml::startDocument()
{
	// init tabsong
	song->tempo = 120;			// default start tempo
	song->t.clear();				// no tracks read yet: clear track list
	song->title = "";				// default title
	song->author = "";			// default author
	song->transcriber = "";		// default transcriber
	song->comments = "";			// default comments
// 	song->filename = "";			// is set in KGuitarPart::slotOpenFile()
	// init global variables: clear part list
	partIds.clear();
	// init global variables: characters collected
	stCha = "";
	// init global variables: identification
	stCrt = "";
	stEnc = "";
	stTtl = "";
	// init global variables: measure, default to 4/4
	// do note re-init between measures
	stBts = "4";
	stBtt = "4";
	stDiv = "";
	stFif = "";
	iDiv = 0;
	return TRUE;
}

// start of element handler

// Note: on reading the following input
//
// <score-part id="P1">
//   <part-name></part-name>
//     <score-instrument id="P1-I1">
//       <instrument-name>Voice</instrument-name>
//
// the parser calls
//
// startElement("score-part")
// characters("\n")
// characters("  ")
// startElement("part-name")
// endElement("part-name")
// characters("\n")
// characters("    ")
// startElement("score-instrument")
// characters("\n")
// characters("      ")
// startElement("instrument-name")
// characters("Voice")
// endElement("instrument-name")
//
// As characters() is not called between startElement("part-name") and
// endElement("part-name"), stCha needs to be cleared at each startElement().
// Failing to do so results in reading (previous) whitespace for empty
// elements such as the part-name in this example.

bool ConvertXml::startElement( const QString&, const QString&,
                                   const QString& qName,
                                   const QXmlAttributes& attributes)
{
	stCha = "";		// see note above
	if (false) {
	} else if (qName == "glissando") {
		QString tp = attributes.value("type");
		if (tp == "start") {
			stGls = TRUE;
		}
	} else if (qName == "hammer-on") {
		QString tp = attributes.value("type");
		if (tp == "start") {
			stHmr = TRUE;
		}
	} else if (qName == "measure") {
		// add a bar (measure) to the track
		// if not first bar: copy attributes from previous bar
		// note: first bar's default attributes set in TabTrack's constructor
		// LVIFIX: maybe don't add first measure here
		// (already done in TabTrack's constructor) ?
		if (trk) {
			bar++;
			trk->b.resize(bar);
			trk->b[bar-1].start=x;
			if (bar > 1) {
				trk->b[bar-1].time1=trk->b[bar-2].time1;
				trk->b[bar-1].time2=trk->b[bar-2].time2;
			}
		}
		tStartCur = -1;			// undefined
    } else if (qName == "note") {
    	// re-init note specific variables
		initStNote();
	} else if (qName == "part") {
		// start of track data found
    	// use part id to switch to correct track
		QString id = attributes.value("id");
		int index = -1;
		for (unsigned int i = 0; i < partIds.size(); i++) {
			if (id.compare(*partIds.at(i)) == 0) {
				index = i;
			}
		}
		if (index == -1) {
			// track id not found
			trk = NULL;
		} else {
			// init vars for track reading
			x = 0;
			bar = 0;
			song->t.at(index);
			trk = song->t.current();
			tEndCur = 0;
		}
	} else if (qName == "pull-off") {
		QString tp = attributes.value("type");
		if (tp == "start") {
			stPlo = TRUE;
		}
	} else if (qName == "score-part") {
		// start of track definition found
	    // re-init score part specific variables
		initStScorePart();
	    stPid = attributes.value("id");
	} else if (qName == "sound") {
		song->tempo = attributes.value("tempo").toInt();
	} else if (qName == "staff-tuning") {
	    // re-init staff tuning specific variables
		initStStaffTuning();
	    stPtl = attributes.value("line");
	} else if (qName == "tie") {
		QString tp = attributes.value("type");
		if (tp == "stop") {
			stTie = TRUE;
		}
	} else {
	    // others (silently) ignored
	}
	return TRUE;
}

// end of element handler

bool ConvertXml::endElement( const QString&, const QString&,
                                  const QString& qName)
{
	QString Str;
	if (qName == "actual-notes") {
	    stAno = stCha;
	} else if (qName == "alter") {
	    stAlt = stCha;
	} else if (qName == "attributes") {
		// update this bar's attributes
		if (trk) {
			trk->b[bar-1].time1=stBts.toInt();
			trk->b[bar-1].time2=stBtt.toInt();
			trk->b[bar-1].keysig=stFif.toInt();
		}
	} else if (qName == "backup") {
		tStartCur = -1;
		tEndCur -= stDur.toInt() * 120 / iDiv;
	} else if (qName == "beats") {
	    stBts = stCha;
	} else if (qName == "beat-type") {
	    stBtt = stCha;
	} else if (qName == "chord") {
	    stCho = TRUE;
	} else if (qName == "creator") {
	    stCrt = stCha;
	} else if (qName == "divisions") {
	    stDiv = stCha;
		iDiv = stDiv.toInt();
		if (iDiv <= 0) {
			kdDebug() << "illegal divisions value: " << stDiv << endl;
		}
	} else if (qName == "dot") {
	    stDts++;
	} else if (qName == "duration") {
	    stDur = stCha;
	} else if (qName == "encoder") {
	    stEnc = stCha;
	} else if (qName == "fifths") {
	    stFif = stCha;
	} else if (qName == "forward") {
		tStartCur = -1;
		tEndCur += stDur.toInt() * 120 / iDiv;
	} else if (qName == "fret") {
	    stFrt = stCha;
	} else if (qName == "identification") {
		song->title       = stTtl;
		song->author      = stCrt;
		song->transcriber = stEnc;
		song->comments    = "";
	} else if (qName == "measure") {
		// forward to end of track in case of incomplete last voice
		int td = trk->trackDuration();
		if (tEndCur < td) {
			tEndCur = td;
			x = trk->c.size();
		}
	} else if (qName == "midi-bank") {
	    stPmb = stCha;
	} else if (qName == "midi-channel") {
	    stPmc = stCha;
	} else if (qName == "midi-program") {
	    stPmp = stCha;
	} else if (qName == "normal-notes") {
	    stNno = stCha;
	} else if (qName == "note") {
	    return addNote();
	} else if (qName == "octave") {
	    stOct = stCha;
	} else if (qName == "part") {
	    trk = NULL;
	} else if (qName == "part-name") {
	    stPnm = stCha;
	} else if (qName == "rest") {
	    stRst = TRUE;
	} else if (qName == "score-part") {
	    bool res = addTrack();
	    // re-init score part specific variables
		initStScorePart();
		return res;
	} else if (qName == "score-timewise") {
			reportError("not supported: score-timewise");
			return false;
	} else if (qName == "staff-lines") {
		stPtn = stCha;
		if (trk) {
			int nStr = stPtn.toInt();
			if ((nStr < 1) || (nStr > MAX_STRINGS))
				nStr = MAX_STRINGS;
			trk->string = nStr;
		}
	} else if (qName == "staff-tuning") {
		if (trk) {
			int iPtl = stPtl.toInt();
			int iPto = stPto.toInt();
			// LVIFIX: Check max_strings
			trk->tune[iPtl - 1]
				= accSt.sao2Pitch(stPts, 0 /* LVIFIX */, iPto);
		}
	} else if (qName == "step") {
	    stStp = stCha;
	} else if (qName == "string") {
	    stStr = stCha;
	} else if (qName == "tuning-step") {
	    stPts = stCha;
	} else if (qName == "tuning-octave") {
	    stPto = stCha;
	} else if (qName == "type") {
	    stTyp = stCha;
	} else if (qName == "work-title") {
	    stTtl = stCha;
	// following elements are explicitly ignored, usually because sub-
	// and superelements handle all the work, sometimes because features
	// are not supported by KGuitar and sometimes they are
	// simply not necessary
	} else if (
			   qName == "accidental-mark"	// not supported by KG
			|| qName == "articulations"
			// LVIFIX: add more ...
			|| qName == "work"		// not supported by KG
			|| qName == "work-number"	// not supported by KG
//			|| qName == ""
		  ) {
	} else {
		// LVIFIX: define what to do, e.g. silently ignore unknown
		// elements, or report these as warning
		// for the time being, the last option is chosen
		Str = "skipping <" + qName + ">";
		reportWarning(Str);
	    // ignore
	}
	return TRUE;
}

// character(s) handler

bool ConvertXml::characters(const QString& ch)
{
	stCha = ch;
	return TRUE;
}

// add a note to the current track

bool ConvertXml::addNote()
{
    // string conversions
	bool ok1;
	bool ok2;
	bool ok3;
	bool ok4;
	bool ok5;
	bool ok6;
	unsigned int frt = stFrt.toUInt(&ok1);
	unsigned int str = stStr.toUInt(&ok2);
	unsigned int ano = stAno.toUInt(&ok3);
	unsigned int nno = stNno.toUInt(&ok4);
	         int alt = stAlt.toInt( &ok5);
	unsigned int oct = stOct.toUInt(&ok6);
	int          len = mxmlNoteType2Kg(stTyp);

	// sanity checks
	if ((trk == NULL) || (len == 0)) {
		initStNote();
		return TRUE;			// LVIFIX: how to report error ?
	}

	int  nnDur   = len;			// new note duration incl dot/triplet
	uint nnFlags = 0;			// new note flags

	// handle dot (LVIFIX: more than one not supported by KGuitar)
    if (stDts) {
		nnDur = nnDur * 3 / 2;
	    nnFlags |= FLAG_DOT;
	}

	// handle triplets
	if (ok3 && ok4 && (ano == 3) && (nno == 2)) {
		nnDur = nnDur * 2 / 3;
		nnFlags |= FLAG_TRIPLET;
	}

	// append note to current track
	if (stCho) {
		if (tStartCur < 0) {
			// LVIFIX: report error ?
			kdDebug() << "<chord> at start of measure of after backup/forward"
			          << endl;
			// pretend to be appending
			tStartCur = tEndCur;
		}
		tEndCur = tStartCur + nnDur;
	} else {
		tStartCur = tEndCur;
		tEndCur += nnDur;
	}
	int ncols = trk->insertColumn(tStartCur, tEndCur);
	x = trk->x + 1;

	if (stRst) {
		// kdDebug() << "rest, l=" << len << endl;
	}

	// if not rest or tie: fill in fret (if rest: frets stay -1)
	// placed here on purpose:
	// in case of failure, fret stays -1 which is a safe value
	if (!stRst && !stTie) {
		if (!ok1 || !ok2) {
			// no valid frt/str: try if stp/alt/oct can be used instead
			// note: alt may be missing
			if ((stStp == "") || !ok6) {
				// no valid stp/alt/oct
				initStNote();
				return TRUE;	// LVIFIX: how to report error ?
			} else {
				Accidentals acc;
				int pitch = acc.sao2Pitch(stStp, alt, oct);
				if (!allocStrFrt(pitch, trk, x-1, str, frt)) {
					kdDebug() << "ConvertXml::addNote() ";
					kdDebug() << "string/fret allocation failed, ";
					kdDebug() << "column=" << x << endl;
				}
			}
		}
		// set string/fret
		// LVIFIX: check valid range for frt and str
		int kgStr = mxmlStr2Kg(str, trk->string);
		trk->c[x-1].a[kgStr] = frt;
		// if note spans multiple columns, then set ringing
		if (ncols > 1) {
			trk->c[x-1].e[kgStr] = EFFECT_LETRING;
			// stop ringing at column x+ncols-1 (if it exists)
			// needed only if x-1 has note and x+ncols-1 hasn't
			if ((unsigned) x < (trk->c.size() - ncols + 1)) {
				if (trk->c[x+ncols-1].a[kgStr] < 0) {
					trk->c[x+ncols-1].e[kgStr] = EFFECT_STOPRING;
				}
			}
		}
		// handle slide, hammer-on and pull-off
		// last two get higher priority
		// (EFFECT_LEGATO overwrites EFFECT_SLIDE)
		if (stGls) {
			trk->c[x-1].e[kgStr] = EFFECT_SLIDE;
		}
		if (stHmr || stPlo) {
			trk->c[x-1].e[kgStr] = EFFECT_LEGATO;
		}
	}

	// handle tie
	if (stTie && (x>0)) {
		trk->c[x-1].flags |= FLAG_ARC;
	}

    // re-init note specific variables
	initStNote();
	return TRUE;
}

// add a track to the current song

bool ConvertXml::addTrack()
{
	// new track found, append it
	// note: TabTracks contructor initializes all required member variables,
	// tune[0..5] is set to guitar standard tuning
	// LVIFIX (in tabtrack.cpp): init all other members of TabTrack.tune[]
	// current code gives uninitialized tuning if #strings > 6, but no tuning
	// specified
	TabTrack * trk = new TabTrack(
		TabTrack::FretTab,      // _tm LVIFIX: no support for drumtrack
		stPnm,                  // _name
		stPmc.toInt(),          // _channel
		stPmb.toInt(),          // _bank
		stPmp.toInt(),          // _patch (=program)
		6,                      // _string (default value)
		24                      // _frets (default value)
	);
	song->t.append(trk);
	// don't want any columns yet, as that would interfere
	// with the first note's timing
	// LVIFIX: add a single column to empty tracks after loading mxml
	trk->c.resize(0);
	// remember part id to track nr mapping
	QString *sp = new QString(stPid);
	int sz = partIds.size();
	partIds.resize(sz+1);
	partIds.insert(sz, sp);
	return TRUE;
}

// initialize note state variables

void ConvertXml::initStNote()
{
	stAlt = "";
	stAno = "";
	stCho = FALSE;
	stDts = 0;
	stDur = "";
	stFrt = "";
	stGls = FALSE;
	stHmr = FALSE;
	stNno = "";
	stOct = "";
	stPlo = FALSE;
	stRst = FALSE;
	stStp = "";
	stStr = "";
	stTie = FALSE;
	stTyp = "";
}

// initialize part state variables

void ConvertXml::initStScorePart()
{
    stPid = "";
    stPmb = "";
    stPmc = "";
    stPmp = "";
    stPnm = "";
}

// initialize tuning state variables

void ConvertXml::initStStaffTuning()
{
    stPtl = "";
//    stPtn = "";
    stPto = "";
    stPts = "";
}


// helpers for the parser

// report all (fatal and non-fatal) errors
// LVIFIX: in future, might show a dialog

void ConvertXml::reportAll(const QString& lvl, const QString& err)
{
//	QString filename(parser_params.fname);
	QString filename("<add filename>");	// LVIFIX
	QString fullErr;
	QString linenr;
	linenr.setNum(lctr->lineNumber());
	fullErr  = "";
	fullErr += lvl;
	fullErr += ": In ";
	fullErr += filename;
	fullErr += " line ";
	fullErr += linenr;
	fullErr += ": ";
	fullErr += err;
	fullErr += "\n";
	cerr << fullErr;
}


// report a warning (non-fatal error, i.e. one which allows parsing to continue)

void ConvertXml::reportWarning(const QString& err)
{
	reportAll("Warning", err);
}


// report a fatal error

void ConvertXml::reportError(const QString& err)
{
	reportAll("Error", err);
}

