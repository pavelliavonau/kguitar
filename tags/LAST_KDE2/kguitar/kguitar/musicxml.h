/***************************************************************************
 * musicxml.h: definition of MusicXML classes
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

#include <qvector.h>
#include <qxml.h>

class QString;
class TabSong;
class TabTrack;

class MusicXMLParser : public QXmlDefaultHandler
{
public:
	MusicXMLParser(TabSong *);
	bool startDocument();
	bool startElement(const QString&, const QString&, const QString&,
	                  const QXmlAttributes&);
	bool endElement(const QString&, const QString&, const QString&);
	bool characters(const QString & ch);

private:
	bool addNote();
	bool addTrack();
	void initStNote();
	void initStScorePart();
	void initStStaffTuning();
	
	TabSong * ts;				// pointer to calling tabsong
	TabTrack * trk;				// pointer to current track
	QVector<QString> partIds;	// part (track) id's
	int x;						// current column
	int bar;					// current bar

	// state variables for parsing
	// general -- initialized in startDocument()
	QString stCha;				// characters collected
	// identification -- initialized in startDocument()
	QString stCrt;				// creator
	QString stEnc;				// encoder
	QString stTtl;				// title
	// measure -- initialized in startDocument()
	QString stBts;				// beats
	QString stBtt;				// beat-type
	// note -- initialized in initStNote()
	bool    stCho;				// chord with previous note
	int     stDts;				// dots (count)
	QString stFrt;				// fret
	bool    stRst;				// rest
	QString stStr;				// string
	QString stTyp;				// type
	// part (== track) -- initialized in initStScorePart()
	QString stPid;				// ID
	QString stPmc;				// MIDI channel
	QString stPmi;				// MIDI instrument
	QString stPnm;				// name
	// tuning -- initialized in initStStaffTuning()
	QString stPtl;				// tuning line
	QString stPto;				// tuning octave
	QString stPts;				// tuning step
};

class MusicXMLWriter
{
public:
	MusicXMLWriter(TabSong *);
	void write(QTextStream&);
private:
	void writeCol(QTextStream&, TabTrack *, int);
	void writePitch(QTextStream&, int, QString, QString);
	void writeStaffDetails(QTextStream&, TabTrack *);
	void writeTime(QTextStream&, int, int);
	TabSong * ts;				// pointer to calling tabsong
};
