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
// input error handling
// number of strings
// triplet handling

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

#include "musicxml.h"
#include "tabsong.h"
#include "tabtrack.h"

#include <iostream.h>
#include <qstring.h>

// local conversion functions

static QString kgNoteLen2Mxml(const int);
static int mxmlSao2Kg(const QString&, const QString&, const QString&);
static int mxmlStr2Kg(const int);
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

// convert MusicXML step/alter/octave to KGuitar midi note number
// return -1 on failure

// LVIFIX: tuning-alter not supported

static QString notes_us[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                               "F#", "G",  "G#", "A",  "A#", "B"};

static int mxmlSao2Kg(const QString& mxmlStep,
                      const QString& mxmlAlter,
                      const QString& mxmlOctave)
{
    int cn = -1;

	// search step in note name table
    for (int i = 0; i < 12; i++) {
		if (mxmlStep == notes_us[i]) {
			cn = i;
		}
	}
	if (cn == -1) {
		return -1;
	}

	if (mxmlAlter != "") {
		// LVIFIX: add
	}

	return mxmlOctave.toInt() * 12 + cn;
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

static int mxmlStr2Kg(const int mxmlStr)
{
	// LVIFIX
	return 6 - mxmlStr;
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

// MusicXMLParser constructor
// LVIFIX: replace 6 by #strings

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
    if (qName == "note") {
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
	} else {
	    // others (silently) ignored
	}
	return TRUE;
}

// end of element handler

bool MusicXMLParser::endElement( const QString&, const QString&,
                                  const QString& qName)
{
	if (qName == "attributes") {
		// LVIFIX: attributes is optional, detect start of measure differently,
		// so it also works for measures without attributes
		
		// start of measure found
		if (trk) {
			bar++;
			trk->b.resize(bar);
			trk->b[bar-1].start=x;
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
	} else if (qName == "dots") {
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
	} else if (qName == "midi-channel") {
	    stPmc = stCha;
	} else if (qName == "midi-instrument") {
	    stPmi = stCha;
	} else if (qName == "note") {
	    return addNote();
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
		// LVIFIX set #strings on current track
		// if >6, init tune[...]
//    stPtn = stCha;
	} else if (qName == "staff-tuning") {
		if (trk) {
			trk->tune[mxmlStr2Kg(stPtl.toInt())]
				= mxmlSao2Kg(stPts, "" /* LVIFIX */, stPto);
		}
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
	unsigned int frt = stFrt.toUInt(&ok1);
	unsigned int str = stStr.toUInt(&ok2);
	int len = mxmlNoteType2Kg(stTyp);
	// sanity checks
	// LVIFIX: check valid range for frt and str
	if (trk == NULL)
	   return TRUE;			// LVIFIX: how to report error ?
	if (len == 0)
	   return TRUE;			// LVIFIX: how to report error ?
	if (!stRst && (!ok1 || !ok2))
	   return TRUE;			// LVIFIX: how to report error ?
	// all ok, append note
	// append note to current track
	if (stCho && (x > 0)) {
	    // chord with previous note (cannot be first note of track)
	    if (!stRst) {
		    trk->c[x-1].a[mxmlStr2Kg(stStr.toInt())] = stFrt.toInt();
		}
		// LVIFIX: as KGuitar does not support notes of different length
		// in a chord, length is set to the length of the shortest note
		if (len < trk->c[x-1].l) {
	      trk->c[x-1].l = len;
	      trk->c[x-1].flags = (stDts ? FLAG_DOT : 0);
		}
	} else {
		// single note or first note of chord
		x++;
		trk->c.resize(x);
		for (int k = 0; k < 6; k++) {
			trk->c[x-1].a[k] = -1;
			trk->c[x-1].e[k] = 0;
		}
		if (!stRst) {
			trk->c[x-1].a[mxmlStr2Kg(str)] = frt;
		}
	    trk->c[x-1].l = len;
	    trk->c[x-1].flags = (stDts ? FLAG_DOT : 0);
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
	ts->t.append(new TabTrack(
		FretTab,				// _tm LVIFIX: no support for drumtrack
		stPnm,					// _name
		stPmc.toInt(),			// _channel
		0,						// _bank LVIFIX: TBD
		stPmi.toInt(),			// _patch
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
	stCho = FALSE;
	stDts = 0;
	stFrt = "";
	stRst = FALSE;
	stStr = "";
	stTyp = "";
}

// initialize part state variables

void MusicXMLParser::initStScorePart()
{
    stPid = "";
    stPmc = "";
    stPmi = "";
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
		os << "\t\t\t<score-instrument id=\"S" << it+1 << "\">\n";
		// LVIFIX: add instrument-name ???
		os << "\t\t\t\t<midi-channel>" << ts->t.at(it)->channel
		   << "</midi-channel>\n";
		os << "\t\t\t\t<midi-instrument>" << ts->t.at(it)->patch
		   << "</midi-instrument>\n";
		os << "\t\t\t</score-instrument>\n";
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
			}
			writeCol(os, trk, x);
		}
		os << "\t\t</measure>\n";
		os << "\n";
		os << "\t</part>\n";
	}
	os << "\n";
	os << "</score-partwise>\n";
}

// write column x of TabTrack trk to QTextStream os
// LVIFIX triplet

void MusicXMLWriter::writeCol(QTextStream& os, TabTrack * trk, int x)
{
	int duration;				// note duration (incl. dot/triplet)
	int fret;
	int length;					// note length (excl. dot/triplet)
	int nNotes = 0;				// # notes in this column
	
	// duration is common for note and rest
	length = trk->c[x].l;
	// note scaling: quarter note = 48
	duration = length * 2 / 5;
	if (trk->c[x].flags & FLAG_DOT) {
		duration = duration * 3 / 2;
	}
	// print all notes
	for (int i = trk->string - 1; i >= 0 ; i--) {
		if (trk->c[x].a[i] > -1) {
			nNotes++;
			fret = trk->c[x].a[i];
			os << "\t\t\t<note>\n";
			if (nNotes > 1) {
				os << "\t\t\t\t<chord/>\n";
			}
			os << "\t\t\t\t<pitch>\n";
			writePitch(os, trk->tune[i] + fret, "\t\t\t\t\t", "");
			os << "\t\t\t\t</pitch>\n";
			os << "\t\t\t\t<duration>" << duration << "</duration>\n";
			os << "\t\t\t\t<type>" << kgNoteLen2Mxml(length) << "</type>\n";
			if (trk->c[x].flags & FLAG_DOT) {
				os << "\t\t\t\t<dot/>\n";
			}
			os << "\t\t\t\t<notations>\n";
			os << "\t\t\t\t\t<technical>\n";
			os << "\t\t\t\t\t\t<string>" << mxmlStr2Kg(i) << "</string>\n";
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
	QString noteName = notes_us[n % 12];
	os << tabs << "<" << prfx << "step>" << noteName.left(1)
	   << "</" << prfx << "step>\n";
	if (noteName.length() > 1) {
		if (noteName.mid(1, 1) == "#") {
			os << tabs << "<" << prfx << "alter>1</" << prfx << "alter>\n";
		} else if (noteName.mid(1, 1) == "b") {
			os << tabs << "<" << prfx << "alter>-1</" << prfx << "alter>\n";
		} else {
			// ignore
		}
	}
	os << tabs << "<" << prfx << "octave>" << n / 12
	   << "</" << prfx << "octave>\n";
}

// write staff details of TabTrack trk to QTextStream os

void MusicXMLWriter::writeStaffDetails(QTextStream& os, TabTrack * trk)
{
	os << "\t\t\t\t<staff-details>\n";
	os << "\t\t\t\t\t<staff-lines>" << (int) trk->string << "</staff-lines>\n";
	for (int i = 0; i < trk->string; i++) {
		os << "\t\t\t\t\t<staff-tuning line=\""
		   << mxmlStr2Kg(i) << "\">\n";
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