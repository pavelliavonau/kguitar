/***************************************************************************
 * musicxml.cpp: implementation of MusicXML classes
 *
 * This file is part of KGuitar, a KDE tabulature editor
 *
 * copyright (C) 2002-2004 the KGuitar development team
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

// LVIFIX:
// improve error reporting

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
// Note: the KGuitar user interface does not limit bank, channel and program
// to specific ranges and the instrument name is just a text input field

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

// remarks on Qt's error handling (tested on Qt 2.3.1 and 3.0.5)
// - MusicXMLErrorHandler::warning(), error() and errorString() are not called
//   by Qt's parser, only fatalError() is called
// - when one of ConvertXml's handlers returns false, fatalError is called
//   with msg="error triggered by consumer"
// - a single error may result in many fatalError() calls
// - when fatalError() is called, the parseException contains valid columnnr,
//   linenr and message, but public and systemId are empty
// - failure to properly match start en end elements (e.g. <aaa></bbb>)
//   results in a "tag mismatch" error. To be able to report which tags
//   don't match, startElement/endElement would have to maintain a stack.

#include "global.h"
#include "accidentals.h"
#include "musicxml.h"
#include "tabsong.h"
#include "tabtrack.h"

#include <qstring.h>
#include <q3valuelist.h>

#include "convertxml.h"

// Class MusicXMLErrorHandler

MusicXMLErrorHandler::MusicXMLErrorHandler()
{
	fatalReported = false;
	parser = 0;
}

bool MusicXMLErrorHandler::warning(const QXmlParseException& exception)
{
	kDebug() << "MusicXMLErrorHandler::warning"
		<< " col=" << exception.columnNumber()
		<< " line=" << exception.lineNumber()
		<< " msg=" << exception.message()
		<< " pid=" << exception.publicId()
		<< " sid=" << exception.systemId()
		<< endl;
	return true;	// continue parsing
}

bool MusicXMLErrorHandler::error(const QXmlParseException& exception)
{
	kDebug() << "MusicXMLErrorHandler::error"
		<< " col=" << exception.columnNumber()
		<< " line=" << exception.lineNumber()
		<< " msg=" << exception.message()
		<< " pid=" << exception.publicId()
		<< " sid=" << exception.systemId()
		<< endl;
	return true;	// continue parsing
}

bool MusicXMLErrorHandler::fatalError(const QXmlParseException& exception)
{
	if (exception.message() == "error triggered by consumer") {
		// no need to report this: should already have been done
		// by ConvertXml's handler
		fatalReported = true;
	} else {
		if (!fatalReported) {
			if (parser) {
				parser->reportError(exception.message());
			} else {
				kFatal() << "MusicXMLErrorHandler::fatalError"
					<< " parser=0" << endl;
			}
			fatalReported = true;
		}
	}
	return false;	// do not continue parsing
}

QString MusicXMLErrorHandler::errorString() const
{
	return "KGuitar musicxmlimport error string";
}

void MusicXMLErrorHandler::setParser(ConvertXml * p)
{
	parser = p;
}


