// Global defines

#define MAX_STRINGS   12
#define MAX_FRETS     24
#define NUMFRETS      5

// Global utility functions

#include <qglobal.h>
#include <config.h>

class QString;

QString note_name(int);
Q_UINT16 dot2len(int len, bool dot);
void len2dot(int l, int *len, bool *dot);
QString midi_patch_name(int);

extern QString drum_abbr[128];
extern bool isBrowserView;
