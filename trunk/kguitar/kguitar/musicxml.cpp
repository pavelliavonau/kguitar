/***************************************************************************
 * musicxml.cpp: implementation of MusicXML classes
 *
 * This file is part of KGuitar, a KDE tabulature editor
 *
 * copyright (C) 2002 the KGuitar development team
 *
 * Copyright of the MusicXML file format:
 * (C) Recordare LLC. All rights reserved. http://www.recordare.com
 ***************************************************************************/

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file COPYING for more information.
 ***************************************************************************/

// LVIFIX missing features:
// effects
// input error reporting

// LVIFIX:
// slurs work about the same as ties, but:
// - need to be numbered in case of overlap (?)
// - only present in <notations>
// - need to distinguish between slides, hammers and pull-offs

// LVIFIX:
// add bounds checking on all toInt() results
// check <score-partwise> (score-timewise is not supported)

// LVIFIX:
// MIDI bank, channel and instrument handling: where is it 0- or 1-based ?
// MusicXML 0.6 common.dtd:
// bank numbers range from 1 to 16,384.
// channel numbers range from 1 to 16.
// program numbers range from 1 to 128.
// MIDI spec:
// channel 0..15
// patch (== program): 0..127
// KGuitar ???

// LVIFIX:
// saving a file with empty "author" property results in an empty <encoder>,
// which is read back as three blanks

// LVIFIX:
// reading an xml file with size 0 results in sig 11
// reading an xml file without part-list results in sig 11
// reading an xml file without midi-instrument results in chn=bank=patch=0

#include "global.h"
#include "accidentals.h"
#include "musicxml.h"
#include "tabsong.h"
#include "tabtrack.h"

#include <iostream.h>
#include <qstring.h>

// local conversion functions

static QString kgNoteLen2Mxml(const int);
static int mxmlStr2Kg(const int, const int);
static int mxmlNoteType2Kg(const QString&);

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

// MusicXMLParser constructor

MusicXMLParser::MusicXMLParser(TabSong * tsp)
	: QXmlDefaultHandler()
{
	// need to remember tabsong for callbacks
	ts = tsp;
}

// start of document handler

bool MusicXMLParser::startDocument()
{
	// init tabsong
	ts->tempo = 120;			// default start tempo
	ts->t.clear();				// no tracks read yet: clear track list
	ts->title = "";				// default title
	ts->author = "";			// default author
	ts->transcriber = "";		// default transcriber
	ts->comments = "";			// default comments
	ts->filename = "";			// is set in KGuitarPart::slotOpenFile()
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
	return TRUE;
}

// start of element handler

bool MusicXMLParser::startElement( const QString&, const QString&, 
                                   const QString& qName, 
                                   const QXmlAttributes& attributes)
{
	if (qName == "measure") {
		// add a bar (measure) to the track
		// if not first bar: copy attributes from previous bar
		// note: first bar's default attributes set in TabTrack's constructor
		// LVIFIX: maybe don't add first measure here
		// (already done in TabTrack's constructor)
		if (trk) {
			bar++;
			trk->b.resize(bar);
			trk->b[bar-1].start=x;
			if (bar > 1) {
				trk->b[bar-1].time1=trk->b[bar-2].time1;
				trk->b[bar-1].time2=trk->b[bar-2].time2;
			}
		}
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
			ts->t.at(index);
			trk = ts->t.current();
		}
	} else if (qName == "score-part") {
		// start of track definition found
	    // re-init score part specific variables
		initStScorePart();
	    stPid = attributes.value("id");
	} else if (qName == "sound") {
		ts->tempo = attributes.value("tempo").toInt();
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

bool MusicXMLParser::endElement( const QString&, const QString&,
                                  const QString& qName)
{
	if (qName == "actual-notes") {
	    stAno = stCha;
	} else if (qName == "alter") {
	    stAlt = stCha;
	} else if (qName == "attributes") {
		// update this bar's attributes
		if (trk) {
			trk->b[bar-1].time1=stBts.toInt();
			trk->b[bar-1].time2=stBtt.toInt();
		}
	} else if (qName == "beats") {
	    stBts = stCha;
	} else if (qName == "beat-type") {
	    stBtt = stCha;
	} else if (qName == "chord") {
	    stCho = TRUE;
	} else if (qName == "creator") {
	    stCrt = stCha;
	} else if (qName == "dot") {
	    stDts++;
	} else if (qName == "encoder") {
	    stEnc = stCha;
	} else if (qName == "fret") {
	    stFrt = stCha;
	} else if (qName == "identification") {
		ts->title       = stTtl;
		ts->author      = stCrt;
		ts->transcriber = stEnc;
		ts->comments    = "";
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
			// LVIFIX: Check max_strings
			trk->tune[mxmlStr2Kg(stPtl.toInt(), trk->string)]
				= accSt.sao2Pitch(stPts, 0 /* LVIFIX */, stPto.toInt());
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
	} else {
	    // ignore
	}
	return TRUE;
}

// character(s) handler

bool MusicXMLParser::characters(const QString& ch)
{
	stCha = ch;
	return TRUE;
}

// add a note to the current track

bool MusicXMLParser::addNote()
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
	if (!stCho || (x == 0)) {
		// single note or first note of track: add a new column
		x++;
		trk->c.resize(x);
		// initialize it
		for (int k = 0; k < trk->string; k++) {
			trk->c[x-1].a[k] = -1;
			trk->c[x-1].e[k] = 0;
		}
	    trk->c[x-1].l = len;
		trk->c[x-1].flags = nnFlags;
	} else {

		// note is part of existing column

		// LVIFIX: as KGuitar does not support notes of different length
		// in a chord, length is set to the length of the shortest note
		// note: use duration of note including dot and triplet
		//       shortest note also determines column's dot and triplet flag

		int ccDur = trk->c[x-1].l;			// current column duration
		uint ccFlags = trk->c[x-1].flags;	// current column flags
		if (ccFlags & FLAG_DOT) {
			ccDur = ccDur * 3 / 2;
		}
		if (ccFlags & FLAG_TRIPLET) {
			ccDur = ccDur * 2 / 3;
		}
		if (nnDur < ccDur) {
	      trk->c[x-1].l = len;
		  ccFlags &= ~FLAG_DOT;
		  ccFlags &= ~FLAG_TRIPLET;
		  ccFlags |= nnFlags;
		  trk->c[x-1].flags = ccFlags;
		}
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
					cout << "MusicXMLParser::addNote() ";
					cout << "string/fret allocation failed, ";
					cout << "column=" << x << endl;
				}
			}
		}
		// LVIFIX: check valid range for frt and str
		trk->c[x-1].a[mxmlStr2Kg(str, trk->string)] = frt;
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

bool MusicXMLParser::addTrack()
{
	// new track found, append it
	// note: TabTracks contructor initializes all required member variables,
	// tune[0..5] is set to guitar standard tuning
	// LVIFIX (in tabtrack.cpp): init all other members of TabTrack.tune[]
	// current code gives uninitialized tuning if #strings > 6, but no tuning
	// specified
	ts->t.append(new TabTrack(
		FretTab,				// _tm LVIFIX: no support for drumtrack
		stPnm,					// _name
		stPmc.toInt(),			// _channel
		stPmb.toInt(),			// _bank
		stPmp.toInt(),			// _patch (=program)
		6,						// _string (default value)
		24						// _frets (default value)
	));
	// remember part id to track nr mapping
	QString *sp = new QString(stPid);
	int sz = partIds.size();
	partIds.resize(sz+1);
	partIds.insert(sz, sp);
	return TRUE;
}

// initialize note state variables

void MusicXMLParser::initStNote()
{
	stAlt = "";
	stAno = "";
	stCho = FALSE;
	stDts = 0;
	stFrt = "";
	stNno = "";
	stOct = "";
	stRst = FALSE;
	stStp = "";
	stStr = "";
	stTie = FALSE;
	stTyp = "";
}

// initialize part state variables

void MusicXMLParser::initStScorePart()
{
    stPid = "";
    stPmb = "";
    stPmc = "";
    stPmp = "";
    stPnm = "";
}

// initialize tuning state variables

void MusicXMLParser::initStStaffTuning()
{
    stPtl = "";
//    stPtn = "";
    stPto = "";
    stPts = "";
}

// MusicXMLWriter constructor

MusicXMLWriter::MusicXMLWriter(TabSong * tsp)
{
  // need to remember tabsong
  ts = tsp;
}

// write tabsong to QTextStream os

void MusicXMLWriter::write(QTextStream& os)
{
	os << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
	   << endl;
	os << "<!DOCTYPE score-partwise PUBLIC" << endl;
	os << "    \"-//Recordare//DTD MusicXML 0.6 Partwise//EN\"" << endl;
	os << "    \"http://www.musicxml.org/dtds/partwise.dtd\">" << endl;
	os << endl;
	os << "<score-partwise>\n";
	os << "\t<work>\n";
	os << "\t\t<work-title>" << ts->title << "</work-title>\n";
	os << "\t</work>\n";

// identification
	os << "\n";
	os << "\t<identification>\n";
	os << "\t\t<creator type=\"composer\">" << ts->author << "</creator>\n";
	os << "\t\t<encoding>\n";
	os << "\t\t\t<encoder>" << ts->transcriber << "</encoder>\n";
	os << "\t\t\t<software>KGuitar</software>\n";
	os << "\t\t</encoding>\n";
	os << "\t</identification>\n";

// part list
	os << "\n";
	os << "\t<part-list>\n";
	for (unsigned int it = 0; it < ts->t.count(); it++) {
		os << "\t\t<score-part id=\"P" << it+1 << "\">\n";
		os << "\t\t\t<part-name>" << ts->t.at(it)->name << "</part-name>\n";
		// LVIFIX: add score-instrument if instrument-name is known
		// note: in DTD 0.6 score-instrument may appear zero or more times
		//       within a score-part
		// os << "\t\t\t<score-instrument id=\"P" << it+1
		//    << "-I" << it+1 << "\">\n";
		// os << "\t\t\t\t<instrument-name>" << "TBD"
		//    << "</instrument-name>\n";
		// os << "\t\t\t</score-instrument>\n";
		os << "\t\t\t<midi-instrument id=\"P" << it+1
		   << "-I" << it+1 << "\">\n";
		os << "\t\t\t\t<midi-channel>" << ts->t.at(it)->channel
		   << "</midi-channel>\n";
		os << "\t\t\t\t<midi-bank>" << ts->t.at(it)->bank
		   << "</midi-bank>\n";
		os << "\t\t\t\t<midi-program>" << ts->t.at(it)->patch
		   << "</midi-program>\n";
		os << "\t\t\t</midi-instrument>\n";
		os << "\t\t</score-part>\n";
	}
	os << "\t</part-list>\n";

// parts
	TabTrack *trk;
	for (unsigned int it = 0; it < ts->t.count(); it++) {
		trk = ts->t.at(it);
		uint bar = 0;
		os << "\n";
		os << "\t<part id=\"P" << it+1 << "\">\n";

		int trp = 0;			// triplet state (0=none, 1=1st, 2=2nd, 3=3rd)
		// loop over all columns
		for (uint x = 0; x < trk->c.size(); x++) {
			if (bar+1 < trk->b.size()) {	// This bar's not last
				if (((unsigned int) trk->b[bar+1].start) == x)
					bar++;				// Time for next bar
			}

			if ((bar < trk->b.size())
			    && (((unsigned int) trk->b[bar].start) == x)) {
				// New bar event
				if (bar > 0) {
					// End of previous measure
					os << "\t\t</measure>\n";
					os << "\n";
				}
				os << "\t\t<measure number=\"" << bar + 1 << "\">\n";
				if (bar == 0) {
					// First bar: write all attributes
					os << "\t\t\t<attributes>\n";
					os << "\t\t\t\t<divisions>48</divisions>\n";
					os << "\t\t\t\t<key>\n";
					os << "\t\t\t\t\t<fifths>0</fifths>\n";
					os << "\t\t\t\t\t<mode>major</mode>\n";
					os << "\t\t\t\t</key>\n";
					writeTime(os, trk->b[bar].time1, trk->b[bar].time2);
					os << "\t\t\t\t<clef>\n";
					os << "\t\t\t\t\t<sign>G</sign>\n";
					os << "\t\t\t\t\t<line>2</line>\n";
					os << "\t\t\t\t</clef>\n";
					writeStaffDetails(os, trk);
					os << "\t\t\t</attributes>\n";
					os << "\t\t\t<sound tempo=\"" << ts->tempo << "\"/>\n";
				} else {
					// LVIFIX write time sig if changed
				}
				// Initialize the accidentals
				accSt.resetToKeySig();
			}
			writeCol(os, trk, x, trp);
		}
		os << "\t\t</measure>\n";
		os << "\n";
		os << "\t</part>\n";
	}
	os << "\n";
	os << "</score-partwise>\n";
}

// write accidental of midi note n to QTextStream os

void MusicXMLWriter::writeAccid(QTextStream& os, int n, QString tabs)
{
	int alt = 0;
	int oct = 0;
	Accidentals::Accid acc = Accidentals::None;
	QString nam = "";

	accSt.getNote(n, nam, alt, oct, acc);
	if (acc != Accidentals::None) {
		QString s;
		switch (acc) {
			case Accidentals::Natural: s = "natural"; break;
			case Accidentals::Sharp:   s = "sharp";   break;
			case Accidentals::Flat:    s = "flat";    break;
			default:                   s = "unknown"; break;
		}
		os << tabs << "<accidental>" << s << "</accidental>\n";
	}
}

// write column x of TabTrack trk to QTextStream os

void MusicXMLWriter::writeCol(QTextStream& os, TabTrack * trk, int x, int& trp)
{
	int duration;				// note duration (incl. dot/triplet)
	int fret;
	int length;					// note length (excl. dot/triplet)
	int nNotes = 0;				// # notes in this column

	// tie handling:
	// if the next column is linked with this one, start a tie
	// if the this column is linked with the previous one, end a tie
	// LVIFIX:
	// KGuitar stores the second column of a tie as a rest (an empty column),
	// while MusicXML requires notes there. Therefore take the notes from the
	// previous column.
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
		xt = x - 1;				// LVIFIX
	}
	// duration is common for note and rest
	length = trk->c[x].l;
	// note scaling: quarter note = 48
	duration = length * 2 / 5;
	if (trk->c[x].flags & FLAG_DOT) {
		duration = duration * 3 / 2;
	}
	if (trk->c[x].flags & FLAG_TRIPLET) {
		duration = duration * 2 / 3;
	}
	// triplet handling:
	// - reset after third note of triplet
	// - count notes while inside triplet
	if (trp >= 3) {
		trp = 0;
	}
	if (trk->c[x].flags & FLAG_TRIPLET) {
		trp++;
	}
	// calculate accidentals
	accSt.startChord();
	for (int i = trk->string - 1; i >= 0 ; i--) {
		if (trk->c[xt].a[i] > -1) {
			fret = trk->c[xt].a[i];
			accSt.addPitch(trk->tune[i] + fret);
		}
	}
	accSt.calcChord();
	// print all notes
	for (int i = trk->string - 1; i >= 0 ; i--) {
		if (trk->c[xt].a[i] > -1) {
			nNotes++;
			fret = trk->c[xt].a[i];
			os << "\t\t\t<note>\n";
			if (nNotes > 1) {
				os << "\t\t\t\t<chord/>\n";
			}
			os << "\t\t\t\t<pitch>\n";
			writePitch(os, trk->tune[i] + fret, "\t\t\t\t\t", "");
			os << "\t\t\t\t</pitch>\n";
			os << "\t\t\t\t<duration>" << duration << "</duration>\n";
			if (tieStart) {
				os << "\t\t\t\t<tie type=\"start\"/>\n";
			}
			if (tieStop) {
				os << "\t\t\t\t<tie type=\"stop\"/>\n";
			}
			os << "\t\t\t\t<type>" << kgNoteLen2Mxml(length) << "</type>\n";
			if (trk->c[x].flags & FLAG_DOT) {
				os << "\t\t\t\t<dot/>\n";
			}
			if (trp) {
				os << "\t\t\t\t<time-modification>\n";
				os << "\t\t\t\t\t<actual-notes>3</actual-notes>\n";
				os << "\t\t\t\t\t<normal-notes>2</normal-notes>\n";
				os << "\t\t\t\t</time-modification>\n";
			}
			writeAccid(os, trk->tune[i] + fret, "\t\t\t\t");
			os << "\t\t\t\t<notations>\n";
			if (tieStart) {
				os << "\t\t\t\t\t<tied type=\"start\"/>\n";
			}
			if (tieStop) {
				os << "\t\t\t\t\t<tied type=\"stop\"/>\n";
			}
			if (trp == 1) {
				os << "\t\t\t\t\t<tuplet type=\"start\"/>\n";
			}
			if (trp == 3) {
				os << "\t\t\t\t\t<tuplet type=\"stop\"/>\n";
			}
			os << "\t\t\t\t\t<technical>\n";
			os << "\t\t\t\t\t\t<string>" << mxmlStr2Kg(i, trk->string)
			   << "</string>\n";
			os << "\t\t\t\t\t\t<fret>" << fret << "</fret>\n";
			os << "\t\t\t\t\t</technical>\n";
			os << "\t\t\t\t</notations>\n";
			os << "\t\t\t</note>\n";
		}
	}
	// if no notes in this column, it is a rest
	if (nNotes == 0) {
		os << "\t\t\t<note>\n";
		os << "\t\t\t\t<rest/>\n";
		os << "\t\t\t\t<duration>" << duration << "</duration>\n";
		os << "\t\t\t\t<type>" << kgNoteLen2Mxml(length) << "</type>\n";
		if (trk->c[x].flags & FLAG_DOT) {
			os << "\t\t\t\t<dot/>\n";
		}
		os << "\t\t\t</note>\n";
	}
}

// write midi note number as step/alter/octave to QTextStream os

void MusicXMLWriter::writePitch(QTextStream& os,
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

void MusicXMLWriter::writeStaffDetails(QTextStream& os, TabTrack * trk)
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
	os << "\t\t\t\t<staff-details>\n";
	os << "\t\t\t\t\t<staff-lines>" << (int) trk->string << "</staff-lines>\n";
	for (int i = 0; i < trk->string; i++) {
		os << "\t\t\t\t\t<staff-tuning line=\""
		   << mxmlStr2Kg(i, trk->string) << "\">\n";
		writePitch(os, trk->tune[i], "\t\t\t\t\t\t", "tuning-");
		os << "\t\t\t\t\t</staff-tuning>\n";
	}
	os << "\t\t\t\t</staff-details>\n";
}

// write time signature to QTextStream os

void MusicXMLWriter::writeTime(QTextStream& os, int bts, int btt)
{
	os << "\t\t\t\t<time>\n";
	os << "\t\t\t\t\t<beats>" << bts << "</beats>\n";
	os << "\t\t\t\t\t<beat-type>" << btt << "</beat-type>\n";
	os << "\t\t\t\t</time>\n";
}
