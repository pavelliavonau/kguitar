#ifndef CHORDLISTITEM_H
#define CHORDLISTITEM_H

#include <qlistbox.h>
#include "global.h"
#include "globaloptions.h"

class ChordListItem: public QListBoxText {
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
