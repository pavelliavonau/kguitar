#ifndef CHORDLISTITEM_H
#define CHORDLISTITEM_H

#include <q3listbox.h>
#include "global.h"

/**
 * Item for ChordList - a chord fingering.
 *
 * This item is able to hold all chord-specific data, such as tonic
 * and all steps presence. Automatically forms proper text chord name
 * from this data.
 */
class ChordListItem: public Q3ListBoxText {
public:
	ChordListItem(int _tonic, int _bass, int s3, int s5, int s7,
	              int s9, int s11, int s13);
	int tonic() { return t; };
	int step(int x) { return s[x]; };

private:
	int t;
	int s[6];
};

#endif
