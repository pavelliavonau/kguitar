/***************************************************************************
 * musicxml.cpp: implementation of MusicXML classes
 *
 * This file is part of KGuitar, a KDE tabulature editor
 *
 * copyright (C) 2002-2003 the KGuitar development team
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
// harmonics
// input error reporting

// LVIFIX:
// add bounds checking on all toInt() results
// check <score-partwise> (score-timewise is not supported)

// LVIFIX:
// MIDI bank, channel and instrument handling: where is it 0- or 1-based ?
// MusicXML 0.6 common.dtd:
// bank numbers range from 1 to 16,384.
// channel numbers range from 1 to 16.
// note numbers range from 1 to 128.
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

// LVIFIX:
// all tracks are written in the guitar-specific format using two staves:
// - standard notation with clef-octave-change = -1
// - TAB (in alternate staff)

// LVIFIX:
// clef-octave-change, although present in the MusicXML file written,
// is not really supported:
// it is ignored when reading the file
// it is not used throughout KGuitar

// LVIFIX:
// accidentals are ignored in staff-tuning

// LVIFIX:
// check value of <backup>

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
// 	ts->filename = "";			// is set in KGuitarPart::slotOpenFile()
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

bool MusicXMLParser::startElement( const QString&, const QString&, 
                                   const QString& qName, 
                                   const QXmlAttributes& attributes)
{
	if (qName == "glissando") {
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
			ts->t.at(index);
			trk = ts->t.current();
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
			cout << "illegal divisions value: " << stDiv << endl;
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
		ts->title       = stTtl;
		ts->author      = stCrt;
		ts->transcriber = stEnc;
		ts->comments    = "";
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
	if (stCho) {
		if (tStartCur < 0) {
			// LVIFIX: report error ?
			cout << "<chord> at start of measure of after backup/forward"
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
		// cout << "rest, l=" << len << endl;
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
		// set string/fret
		// LVIFIX: check valid range for frt and str
		int kgStr = mxmlStr2Kg(str, trk->string);
		trk->c[x-1].a[kgStr] = frt;
		// if note spans multiple columns, then set ringing
		if (ncols > 1) {
			trk->c[x-1].e[kgStr] = EFFECT_LETRING;
			// stop ringing at column x+ncols-1 (if it exists)
			// needed only if x-1 has note and x+ncols-1 hasn't
			if (x < (trk->c.size() - ncols + 1)) {
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

bool MusicXMLParser::addTrack()
{
	// new track found, append it
	// note: TabTracks contructor initializes all required member variables,
	// tune[0..5] is set to guitar standard tuning
	// LVIFIX (in tabtrack.cpp): init all other members of TabTrack.tune[]
	// current code gives uninitialized tuning if #strings > 6, but no tuning
	// specified
	TabTrack * trk = new TabTrack(
		FretTab,				// _tm LVIFIX: no support for drumtrack
		stPnm,					// _name
		stPmc.toInt(),			// _channel
		stPmb.toInt(),			// _bank
		stPmp.toInt(),			// _patch (=program)
		6,						// _string (default value)
		24						// _frets (default value)
	);
	ts->t.append(trk);
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

void MusicXMLParser::initStNote()
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
	os << "    \"-//Recordare//DTD MusicXML 0.7a Partwise//EN\"" << endl;
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
	// loop over all tracks
	for (unsigned int it = 0; it < ts->t.count(); it++) {
		os << "\t\t<score-part id=\"P" << it+1 << "\">\n";
		os << "\t\t\t<part-name>" << ts->t.at(it)->name << "</part-name>\n";
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
		os << "\t\t\t\t<midi-channel>" << ts->t.at(it)->channel
		   << "</midi-channel>\n";
		os << "\t\t\t\t<midi-bank>" << ts->t.at(it)->bank
		   << "</midi-bank>\n";
		os << "\t\t\t\t<midi-program>" << ts->t.at(it)->patch
		   << "</midi-program>\n";
		os << "\t\t\t</midi-instrument>\n";
		os << "\t\t</score-part>\n";
	} // end for (unsigned int it = 0; ...
	os << "\t</part-list>\n";

// parts
	TabTrack *trk;
	// loop over all tracks
	for (unsigned int it = 0; it < ts->t.count(); it++) {
		trk = ts->t.at(it);
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
				os << "\t\t\t\t<divisions>48</divisions>\n";
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
				os << "\t\t\t<sound tempo=\"" << ts->tempo << "\"/>\n";
			} else {
				// LVIFIX write time sig if changed
			}

			// loop over all voices in this bar
			for (int i = 0; i < 2; i++) {
				// write only voice 1 in single voice tracks,
				// write all voices in multi voice tracks
				if ((i == 1) || trk->hasMultiVoices()) {
					// loop over all columns in this bar
					for (uint x = trk->b[ib].start;
							x <= trk->lastColumn(ib); /* nothing */) {
						int tp;
						int dt;
						bool tr;
						if (!trk->getNoteTypeAndDots(x, i, tp, dt, tr)) {
							// LVIFIX: error handling ?
						}
						x += writeCol(os, trk, x, i);
					} // end for (uint x = 0; ....
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

QString MusicXMLWriter::strAccid(Accidentals::Accid acc)
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

void MusicXMLWriter::writeBeams(QTextStream& os, TabTrack * trk, int x, int v)
{
	StemInfo * stxt = 0;
	if (v == 0) {
		stxt = & trk->c[x].stl;
	} else {
		stxt = & trk->c[x].stu;
	}
	/*
	cout
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
// update triplet state
// return ncols used

// writeCol must write rest:
// single voice: only in voice 1
// multi voice: only in voice 0
// LVIFIX: cause of this is that in a single voice track all notes
// are allocated to voice 1 to force stem up when printing

int MusicXMLWriter::writeCol(QTextStream& os, TabTrack * trk, int x, int v)
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
			// write backup
			// note scaling: quarter note = 48
			os << "\t\t\t<backup>" << endl;
			os << "\t\t\t\t<duration>" << trk->currentBarDuration() * 2 / 5
				<< "</duration>\n";
			os << "\t\t\t</backup>" << endl;
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
		os << "\t\t\t<forward>" << endl;
		os << "\t\t\t\t<duration>" << tStartCur - tEndPrev
		   << "</duration>\n";
		os << "\t\t\t\t<voice>" << v + 1 << "</voice>\n";
		os << "\t\t\t</forward>" << endl;
		tEndPrev = tStartCur;
	}

	int dots = 0;				// # dots
	bool triplet = false;		// triplet flag
	int duration;				// note duration (incl. dot/triplet)
	int fret;
	int length = 0;				// note length (excl. dot/triplet)
	int nCols = 1;				// # columns used (default 1)
	int nNotes = 0;				// # notes printed in this column
	int nRests = 0;				// # rests printed in this column

	if (!trk->getNoteTypeAndDots(x, v, length, dots, triplet)) {
		// LVIFIX: error handling ?
	}

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
				// for column linked to previous, use colun length
				if (trk->c[x].flags & FLAG_ARC) {
					length = trk->c[x].l;
					// note scaling: quarter note = 48
					duration = length * 2 / 5;
					// LVIFIX: dot and triplet handling required here ?
					// E.g. use trk->getNoteTypeAndDots()
					dots = 0;
					triplet = false;
					nCols = 1;
				} else {
					// note scaling: quarter note = 48
					duration = length * 2 / 5;
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
			os << "\t\t\t\t<duration>" << duration << "</duration>\n";
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
		}
	}
	// if no notes in this column, it is a rest
	// rests are printed:
	// single voice: only in voice 1
	// multi voice: only in voice 0
	if (nNotes == 0) {
		length = trk->c[x].l;
		// note scaling: quarter note = 48
		duration = length * 2 / 5;
		// LVIFIX: dot and triplet handling required here ?
		if (trk->c[x].flags & FLAG_DOT) {
			duration = duration * 3 / 2;
		}
		if (trk->c[x].flags & FLAG_TRIPLET) {
			duration = duration * 2 / 3;
		}
		if (((v == 1) && !trk->hasMultiVoices())
			|| ((v == 0) && trk->hasMultiVoices())) {
			os << "\t\t\t<note>\n";
			os << "\t\t\t\t<rest/>\n";
			os << "\t\t\t\t<duration>" << duration << "</duration>\n";
			os << "\t\t\t\t<voice>" << v + 1 << "</voice>\n";
			os << "\t\t\t\t<type>" << kgNoteLen2Mxml(length) << "</type>\n";
			if (trk->c[x].flags & FLAG_DOT) {
				os << "\t\t\t\t<dot/>\n";
			}
			os << "\t\t\t</note>\n";
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
				os << "\t\t\t<forward>" << endl;
				os << "\t\t\t\t<duration>" << tStartCur - tEndPrev
				   << "</duration>\n";
				os << "\t\t\t\t<voice>" << v + 1 << "</voice>\n";
				os << "\t\t\t</forward>" << endl;
			}
		}
	}

	return nCols;
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

void MusicXMLWriter::writeTime(QTextStream& os, int bts, int btt)
{
	os << "\t\t\t\t<time>\n";
	os << "\t\t\t\t\t<beats>" << bts << "</beats>\n";
	os << "\t\t\t\t\t<beat-type>" << btt << "</beat-type>\n";
	os << "\t\t\t\t</time>\n";
}
