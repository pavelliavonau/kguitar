#ifndef TRACK_H
#define TRACK_H

// GREYFIX - change to global.h
#define MAX_STRINGS 12

#include <qlist.h>
#include <qstring.h>

typedef enum {
    GuitarTab,
    DrumTab
} TrackMode;

// Durations as in MIDI:
// 480 = whole
// 240 = half
// 120 = quarter
// 60  = eighth
// 30  = 16th
// 15  = 32nd

class TabColumn
{
public:
    TabColumn() { for (int i=0;i<MAX_STRINGS;i++) {a[i]=-1;e[i]=0;};l=120; };

    uint l;                             // Duration of note or chord
    char a[MAX_STRINGS];                // Number of fret
    char e[MAX_STRINGS];                // Effect parameter
};

class TabTrack
{
public:
    TabTrack(TrackMode _tm, int bank, uchar patch, uchar str) { tm=_tm;mbank=bank;mpatch=patch;_string=str;c.setAutoDelete(TRUE); };
    QList<TabColumn> c;                 // Tab columns

    void setTuning(const uchar t[MAX_STRINGS]) { for (int i=0;i<_string;i++)  _tune[i]=t[i]; };
    int string() { return _string; }
    int tune(int x) { return _tune[x]; }

    TrackMode trackmode() { return tm; }

    int bank() { return mbank; }
    int patch() { return mpatch; }

//    QListIterator<TabColumn> xi(QListT<TabColumn>);        // Current tab col iterator

    int x;                              // Current tab col
    int y;                              // Current tab row
private:
    TrackMode tm;                       // Track mode
    int mbank;                          // MIDI bank
    uchar mpatch;                       // MIDI patch
    uchar _string;                      // Number of strings
    uchar _tune[MAX_STRINGS];           // Tuning, if appicable
};

class TabSong
{
public:
    TabSong(QString _title, int _tempo) { tempo=_tempo;title=_title;t.setAutoDelete(TRUE); };
    int tempo;
    QList<TabTrack> t;                  // Track data
    QString title;                      // Title of the song
    QString author;                     // Author of the tune
    QString transcriber;                // Who made the tab
    QString comments;                   // Comments

    QString filename;                   // File name to save under

    bool load_from_kg(const char* fileName);
    bool save_to_kg(const char* fileName);
    bool load_from_gtp(const char* fileName);
    bool save_to_gtp(const char* fileName);
};

#endif
