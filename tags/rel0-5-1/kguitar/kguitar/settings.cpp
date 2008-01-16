#include "settings.h"

#include <qstring.h>
#include <kconfig.h>
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

KConfig* Settings::config = NULL;

QString Settings::noteName(int num)
{
	if (num < 0 || num > 11)
		return i18n("Unknown");

	config->setGroup("General");
	int option = config->readNumEntry("NoteNames", 2);

	if (option < 0 || option > 8)
		option = 2;

	return noteNames[option][num];
}

QString Settings::maj7Name()
{
	config->setGroup("General");
	switch (config->readNumEntry("Maj7", 0)) {
	case 1: return "maj7";
	case 2: return "dom7";
	default: return "7M";
	}
}

QString Settings::flatName()
{
	config->setGroup("General");
	if (config->readNumEntry("FlatPlus", 0) == 1) {
		return "b";
	} else {
		return "-";
	}
}

QString Settings::sharpName()
{
	config->setGroup("General");
	if (config->readNumEntry("FlatPlus", 0) == 1) {
		return "#";
	} else {
		return "+";
	}
}

int Settings::midiPort()
{
	config->setGroup("MIDI");
	return config->readNumEntry("Port", 0);
}

int Settings::melodyEditorInlay()
{
    config->setGroup("MelodyEditor");
	return config->readNumEntry("Inlay", 1);
}

int Settings::melodyEditorAction(int num)
{
    config->setGroup("MelodyEditor");
	return config->readNumEntry(QString("Action%1").arg(num), 0);
}

bool Settings::melodyEditorAdvance(int num)
{
    config->setGroup("MelodyEditor");
	return config->readBoolEntry(QString("Advance%1").arg(num), FALSE);
}

int Settings::texTabSize()
{
    config->setGroup("MusiXTeX");
	return config->readNumEntry(QString("TabSize"), 2);
}

bool Settings::texShowBarNumber()
{
    config->setGroup("MusiXTeX");
	return config->readBoolEntry("ShowBarNumber", TRUE);
}

bool Settings::texShowStr()
{
    config->setGroup("MusiXTeX");
	return config->readBoolEntry("ShowStr", TRUE);
}

bool Settings::texShowPageNumber()
{
    config->setGroup("MusiXTeX");
	return config->readBoolEntry("ShowPageNumber", TRUE);
}

bool Settings::texExportMode()
{
    config->setGroup("MusiXTeX");
	return config->readNumEntry("ExportMode", 0);
}

int Settings::printingStyle()
{
    config->setGroup("Printing");
	return config->readNumEntry("Style", 0);
}
