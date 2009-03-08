#include "converttse3.h"

#ifdef WITH_TSE3
#include <tse3/TSE3MDL.h>
#include <string>
#endif

ConvertTse3::ConvertTse3(TabSong *song): ConvertBase(song) {}

bool ConvertTse3::save(QString fileName)
{
#ifdef WITH_TSE3
	TSE3::TSE3MDL mdl("KGuitar", 2);
	mdl.save((const char *) fileName.local8Bit(), song->midiSong());
	// GREYFIX: pretty ugly unicode string to standard string hack
	return TRUE;
#else
	return FALSE;
#endif
}

bool ConvertTse3::load(QString)
{
	return FALSE;
}
