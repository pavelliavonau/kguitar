#include <qlist.h>

#include "global.h"

class TabColumn
{
public:
    TabColumn() { for (int i=0;i<MAX_STRINGS;i++) {a[i]=-1;e[i]=0;}; };

    int a[MAX_STRINGS];                 // Number of fret
    int e[MAX_STRINGS];                 // Effect parameter
};

class TabTrack
{
public:
    TabTrack(int bank, int patch, int str) { mbank=bank;mpatch=patch;_string=str; };
    QList<TabColumn> c;                 // Tab columns

    void setTuning(const int t[MAX_STRINGS]) { for (int i=0;i<_string;i++)  tune[i]=t[i]; };
    int string() { return _string; }

    int x;                              // Current tab col
    int y;                              // Current tab row
private:
    int mbank;                          // MIDI bank
    int mpatch;                         // MIDI patch
    int _string;                        // Number of strings
    int tune[MAX_STRINGS];              // Tuning, if appicable
};

class TabSong
{
public:
    TabSong(QString _title, int _tempo) { tempo=_tempo; title=_title; };
    int tempo;
    QList<TabTrack> t;                  // Track data
private:
    QString title;                      // Title of the song
    QString author;                     // Author of the tune
    QString transcriber;                // Who made the tab
    QString comments;                   // Comments
};
