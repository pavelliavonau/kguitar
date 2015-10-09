#include "settings.h"

#include <qstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>

QString Settings::noteNames[9][12] = {
	{"C",  "C#", "D",  "D#", "E",  "F",	 "F#", "G",  "G#", "A",  "A#", "B"},
	{"C",  "Db", "D",  "Eb", "E",  "F",	 "Gb", "G",  "Ab", "A",  "Bb", "B"},
	{"C",  "Db", "D",  "Eb", "E",  "F",	 "F#", "G",  "G#", "A",  "Bb", "B"},
	{"C",  "C#", "D",  "D#", "E",  "F",	 "F#", "G",  "G#", "A",  "A#", "H"},
	{"C",  "Db", "D",  "Eb", "E",  "F",	 "Gb", "G",  "Ab", "A",  "Hb", "H"},
	{"C",  "Db", "D",  "Eb", "E",  "F",	 "F#", "G",  "G#", "A",  "Hb", "H"},
	{"C",  "C#", "D",  "D#", "E",  "F",	 "F#", "G",  "G#", "A",  "B" , "H"},
	{"C",  "Db", "D",  "Eb", "E",  "F",	 "Gb", "G",  "Ab", "A",  "B" , "H"},
	{"C",  "Db", "D",  "Eb", "E",  "F",	 "F#", "G",  "G#", "A",  "B" , "H"}
};

KSharedConfigPtr Settings::config;

QString Settings::noteName(int num)
{
	if (num < 0 || num > 11)
		return i18n("Unknown");

	int option = config->group("General").readEntry("NoteNames", 2);

	if (option < 0 || option > 8)
		option = 2;

	return noteNames[option][num];
}

QString Settings::maj7Name()
{
	switch (config->group("General").readEntry("Maj7", 0)) {
	case 1: return "maj7";
	case 2: return "dom7";
	default: return "7M";
	}
}

QString Settings::flatName()
{
	if (config->group("General").readEntry("FlatPlus", 0) == 1) {
		return "b";
	} else {
		return "-";
	}
}

QString Settings::sharpName()
{
	if (config->group("General").readEntry("FlatPlus", 0) == 1) {
		return "#";
	} else {
		return "+";
	}
}

int Settings::midiPort()
{
	return config->group("MIDI").readEntry("Port", 0);
}

int Settings::melodyEditorInlay()
{
    return config->group("MelodyEditor").readEntry("Inlay", 1);
}

int Settings::melodyEditorAction(int num)
{
    return config->group("MelodyEditor").readEntry(QString("Action%1").arg(num), 0);
}

bool Settings::melodyEditorAdvance(int num)
{
    return config->group("MelodyEditor").readEntry(QString("Advance%1").arg(num), FALSE);
}

int Settings::texTabSize()
{
    return config->group("MusiXTeX").readEntry(QString("TabSize"), 2);
}

bool Settings::texShowBarNumber()
{
    return config->group("MusiXTeX").readEntry("ShowBarNumber", TRUE);
}

bool Settings::texShowStr()
{
    return config->group("MusiXTeX").readEntry("ShowStr", TRUE);
}

bool Settings::texShowPageNumber()
{
    return config->group("MusiXTeX").readEntry("ShowPageNumber", TRUE);
}

bool Settings::texExportMode()
{
    return config->group("MusiXTeX").readEntry("ExportMode", 0);
}

int Settings::printingStyle()
{
    return config->group("Printing").readEntry("Style", 0);
}
