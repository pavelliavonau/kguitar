// Global defines

#define MAX_STRINGS   12
#define MAX_FRETS     24
#define NUMFRETS      5

// Global utility functions

#include <qglobal.h>
#include <config.h>
#include <kdebug.h>

class QString;

QString midi_patch_name(int);

extern QString drum_abbr[128];

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#define FALSE false
#define TRUE true

// Define if both "old" and "new" drawing code must be used at the same time
// Undefine to use only the "new" drawing code
#undef USE_BOTH_OLD_AND_NEW
// #define USE_BOTH_OLD_AND_NEW
