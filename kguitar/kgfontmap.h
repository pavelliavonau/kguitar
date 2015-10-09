/***************************************************************************
 * kgfontmap.h: definition of KgFontMap class
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

#ifndef KGFONTMAP_H
#define KGFONTMAP_H

#include <qmap.h>
#include <qstring.h>

class KgFontMap
{
public:
	enum Symbol {
//		KGuitar symbol		Unicode character name
//					nvu indicates name is not a valid Unicode name
//		Breve,			// MUSICAL SYMBOL BREVE not used (yet)
		Whole_Note,		// MUSICAL SYMBOL WHOLE NOTE
		White_NoteHead,		// MUSICAL SYMBOL VOID NOTEHEAD
		Black_NoteHead,		// MUSICAL SYMBOL NOTEHEAD BLACK
		Stem,			// MUSICAL SYMBOL COMBINING STEM
		StemInv,		// MUSICAL SYMBOL INVERTED COMBINING STEM NVU
		Eighth_Flag,		// MUSICAL SYMBOL COMBINING FLAG-1
		Sixteenth_Flag,		// MUSICAL SYMBOL COMBINING FLAG-2
		ThirtySecond_Flag,	// MUSICAL SYMBOL COMBINING FLAG-3
		Eighth_FlagInv,		// MUSICAL SYMBOL INVERTED COMBINING FLAG-1 NVU
		Sixteenth_FlagInv,	// MUSICAL SYMBOL INVERTED COMBINING FLAG-2 NVU
		ThirtySecond_FlagInv,	// MUSICAL SYMBOL INVERTED COMBINING FLAG-3 NVU
		Whole_Rest,		// MUSICAL SYMBOL WHOLE REST
		Half_Rest,		// MUSICAL SYMBOL HALF REST
		Quarter_Rest,		// MUSICAL SYMBOL QUARTER REST
		Eighth_Rest,		// MUSICAL SYMBOL EIGHTH REST
		Sixteenth_Rest,		// MUSICAL SYMBOL SIXTEENTH REST
		ThirtySecond_Rest,	// MUSICAL SYMBOL THIRTY-SECOND REST
		Flat_Sign,		// MUSIC FLAT SIGN
		Natural_Sign,		// MUSIC NATURAL SIGN
		Sharp_Sign,		// MUSIC SHARP SIGN
//		DoubleSharp_Sign,	// MUSICAL SYMBOL DOUBLE SHARP not used (yet)
//		DoubleFlat_Sign,	// MUSICAL SYMBOL DOUBLE FLAT not used (yet)
		Dot,			// MUSICAL SYMBOL COMBINING AUGMENTATION DOT
		G_Clef,			// MUSICAL SYMBOL G CLEF
		UndefinedSymbol		// undefined symbol for which getString
					// returns an empty string
					// equal to number of symbols in table
	};
	
	KgFontMap();
	bool getString(Symbol sym, QString& s) const;

private:
	QMap<Symbol,QChar>	symToCharMap;
};

#endif
