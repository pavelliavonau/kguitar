/***************************************************************************
 * musicxml.h: definition of MusicXML classes
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

#ifndef MUSICXML_H
#define MUSICXML_H

#include <qvector.h>
#include <qxml.h>
#include "accidentals.h"

class QString;
class TabSong;
class TabTrack;

class ConvertXml;

class MusicXMLErrorHandler: public QXmlErrorHandler {
public:
	MusicXMLErrorHandler();
	bool warning(const QXmlParseException& exception);
	bool error(const QXmlParseException& exception);
	bool fatalError(const QXmlParseException& exception);
	QString errorString();
	void setParser(ConvertXml * p);
private:
	bool fatalReported;
	ConvertXml * parser;
};

#endif
