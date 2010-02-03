#include "convertmidi.h"
#include "settings.h"

#include <kconfig.h>
#include <qfile.h>
#include <q3textstream.h>

#ifdef WITH_TSE3
#include <tse3/Track.h>
#include <tse3/Part.h>
#include <tse3/MidiFile.h>
#include <tse3/TSE3MDL.h>
#include <tse3/TempoTrack.h>
#include <string>
#endif

ConvertMidi::ConvertMidi(TabSong *song): ConvertBase(song)
{
	// GREYTODO: really needed here?
//	Settings::config->setGroup("MIDI");
}

bool ConvertMidi::save(QString fileName)
{
#ifdef WITH_TSE3
	TSE3::MidiFileExport exp;
	exp.save((const char *) fileName.local8Bit(), song->midiSong());
	// GREYFIX: pretty ugly unicode string to standard string hack
	return TRUE;
#else
	return FALSE;
#endif
}

bool ConvertMidi::load(QString)
{
	// GREYFIX: todo loading from MIDI tabs
	return FALSE;
}
