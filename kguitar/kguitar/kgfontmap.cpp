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


#include <iostream>		// required for cout and friends
using namespace std;		// required for cout and friends

#include "kgfontmap.h"

// KgFontMap constructor

KgFontMap::KgFontMap()
{
//	cout << "KgFontMap::KgFontMap()";
	symToCharMap[Whole_Note]		= 0x0022;
	symToCharMap[White_NoteHead]		= 0x0023;
	symToCharMap[Black_NoteHead]		= 0x0024;
	symToCharMap[Stem]			= 0x0064;
	symToCharMap[StemInv]			= 0x0065;
	symToCharMap[Eighth_Flag]		= 0x005A;
	symToCharMap[Sixteenth_Flag]		= 0x005B;
	symToCharMap[ThirtySecond_Flag]		= 0x005C;
	symToCharMap[Eighth_FlagInv]		= 0x005F;
	symToCharMap[Sixteenth_FlagInv]		= 0x2018;
	symToCharMap[ThirtySecond_FlagInv]	= 0x0061;
	symToCharMap[Whole_Rest]		= 0x0025;
	symToCharMap[Half_Rest]			= 0x0026;
	symToCharMap[Quarter_Rest]		= 0x0028;
	symToCharMap[Eighth_Rest]		= 0x0029;
	symToCharMap[Sixteenth_Rest]		= 0x002A;
	symToCharMap[ThirtySecond_Rest]		= 0x002B;
	symToCharMap[Flat_Sign]			= 0x201E;
	symToCharMap[Natural_Sign]		= 0x201D;
	symToCharMap[Sharp_Sign]		= 0x201C;
	symToCharMap[Dot]			= 0x00A7;
	symToCharMap[G_Clef]			= 0x006A;
//	cout << endl;
//	cout << "symToCharMap.count()=" << symToCharMap.count() << endl;
//	cout << "symToCharMap=" << endl;
//	for (int i=0; i<=UndefinedSymbol; i++) {
//		cout << "i=" << i << " symToCharMap[i]=" << hex << symToCharMap[(Symbol)i].unicode() << dec << endl;
//	}
}


// get string representation for a given symbol symbol into QString s
// return true (success) or false (failure)

bool KgFontMap::getString(Symbol sym, QString& s) const
{
	s = "";
	bool res = false;
//	cout << "KgFontMap::getString(" << sym << ")";
	if (symToCharMap.contains(sym)) {
		s = symToCharMap[sym];
		res = true;
	}
//	cout << " res=" << res << " s[0]=" << hex << s[0].unicode() << dec << endl;
	return res;
}
