/***************************************************************************
 * kgfontmap.cpp: implementation of KgFontMap class
 *
 * This file is part of KGuitar, a KDE tabulature editor
 *
 * copyright (C) 2004 the KGuitar development team
 ***************************************************************************/

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file COPYING for more information.
 ***************************************************************************/

#include "kgfontmap.h"

// KgFontMap constructor

KgFontMap::KgFontMap()
{
	// UTF-16 map of Musical Symbols
	// http://unicode-table.com/en/blocks/musical-symbols/

	symToCharMap[Whole_Note]            = 0xDD5D;
	symToCharMap[White_NoteHead]        = 0xDD57;
	symToCharMap[Black_NoteHead]        = 0xDD58;

	symToCharMap[Stem]                  = 0xDD65;
//	symToCharMap[StemInv]               = 0xDD65;

	symToCharMap[Eighth_Flag]	    = 0xDD6E;
	symToCharMap[Sixteenth_Flag]	    = 0xDD6F;
	symToCharMap[ThirtySecond_Flag]	    = 0xDD70;

//	symToCharMap[Eighth_FlagInv]	    = 0xDD6E;
//	symToCharMap[Sixteenth_FlagInv]	    = 0xDD6F;
//	symToCharMap[ThirtySecond_FlagInv]  = 0xDD70;

	symToCharMap[Whole_Rest]            = 0xDD3B;
	symToCharMap[Half_Rest]             = 0xDD3C;
	symToCharMap[Quarter_Rest]          = 0xDD3D;
	symToCharMap[Eighth_Rest]           = 0xDD3E;
	symToCharMap[Sixteenth_Rest]        = 0xDD3F;
	symToCharMap[ThirtySecond_Rest]     = 0xDD40;

	symToCharMap[Flat_Sign]             = 0xDD2D;
	symToCharMap[Natural_Sign]          = 0xDD2E;
	symToCharMap[Sharp_Sign]            = 0xDD30;
	symToCharMap[Dot]                   = 0xDD6D;
	symToCharMap[Five_Line_Staff]       = 0xDD1A;
	symToCharMap[G_Clef]                = 0xDD1E;
}


// get string representation for a given symbol symbol into QString s
// return true (success) or false (failure)

bool KgFontMap::getString(Symbol sym, QString& s) const
{
	s = "";
	bool res = false;
	if (symToCharMap.contains(sym)) {
		s += QChar(0xD834);
		s += symToCharMap[sym];
		res = true;
	}
	return res;
}
