#ifndef SETTINGS_H
#define SETTINGS_H

#include "global.h"

class KConfig;
class QString;

/**
 * Pure static class to manage configuration stuff.
 *
 * Has static storage place (instead of global variable) for
 * application-wide configuration variable and methods to quickly
 * access necessary configuration properties.
 */
class Settings {
public:
	/**
	 * Cached configuration access variable
	 */
	static KConfig *config;

	/*
	 * Returns name of note in currently selected note naming
	 * convention.
	 */
	static QString noteName(int);

	/*
	 * Returns name of maj7 step as stated in currently selected
	 * naming convertion.
	 */
	static QString maj7Name();

	/*
	 * Returns symbol to represent flats as stated in currently
	 * selected naming convertion.
	 */
	static QString flatName();

	/*
	 * Returns symbol to represent sharps as stated in currently
	 * selected naming convertion.
	 */
	static QString sharpName();

	/**
	 * Global MIDI port selected for output.
	 */
	static int midiPort();

	/**
	 * MusiXTeX settings
	 */
	static int texTabSize();
	static bool texShowBarNumber();
	static bool texShowStr();
	static bool texShowPageNumber();
	static bool texExportMode();

	/**
	 * Printing style
	 */
	static int printingStyle();

	/**
	 * Melody editor
	 */

	static int melodyEditorInlay();

	/**
	 * Action code for melody editor button num pressed.
	 */
	static int melodyEditorAction(int num);

	/**
	 * Need to advance for melody editor button num pressed.
	 */
	static bool melodyEditorAdvance(int num);

private:
	static QString noteNames[9][12];
};

#endif
