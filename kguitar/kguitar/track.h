#ifndef TRACK_H
#define TRACK_H

// GREYFIX - change to global.h
#define MAX_STRINGS 12

#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

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

typedef struct {
    uint l;                             // Duration of note or chord
    char a[MAX_STRINGS];                // Number of fret
    char e[MAX_STRINGS];                // Effect parameter
} TabColumn;

typedef struct {
    uint start;                         // Starting column
    uchar time1,time2;                  // Time signature
    bool showsig;                       // Show time signature
} TabBar;

class TabTrack
{
public:
    TabTrack(TrackMode _tm, QString _name, int _channel,
	     int _bank, uchar _patch, uchar _string, uchar _frets);

    QArray<TabColumn> c;                // Array of columns
    QArray<TabBar> b;                   // Array of bars

    uchar string;                       // Number of strings
    uchar frets;                        // Number of frets
    uchar tune[MAX_STRINGS];            // Tuning, if appicable

    TrackMode trackmode() { return tm; }

    uchar channel;                      // MIDI channel
    int bank;                           // MIDI bank
    uchar patch;                        // MIDI patch

//    QListIterator<TabColumn> xi(QListT<TabColumn>);  // Current tab col iterator

    QString name;                       // Track text name

    uint x;                             // Current tab column
    uint xb;                            // Current tab bar
    uint y;                             // Current tab string
private:
    TrackMode tm;                       // Track mode
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

    bool load_from_kg(QString fileName);        // Native format - kg
    bool save_to_kg(QString fileName);
    bool load_from_gtp(QString fileName);       // Guitar Pro format
    bool save_to_gtp(QString fileName);
    bool load_from_mid(QString fileName);       // MIDI files
    bool save_to_mid(QString fileName);
    bool load_from_tab(QString fileName);       // ASCII tabulatures
    bool save_to_tab(QString fileName);
private:
    void writeCentered(QTextStream *s, QString l);
};

#endif
