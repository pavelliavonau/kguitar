#ifndef TABSONG_H
#define TABSONG_H

#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

#include "global.h"

#include "tabtrack.h"

class TabSong
{
public:
    TabSong(QString _title, int _tempo);
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
    bool save_to_tex_tab(QString fileName);     // MusiXTeX/tabdefs.tex tabulatures
	bool save_to_tex_notes(QString fileName);   // MusiXTeX notes
private:
    void writeCentered(QTextStream *s, QString l);
	Q_UINT32 readVarLen(QDataStream *s);
	void writeVarLen(QDataStream *s, uint value);
	void writeTempo(QDataStream *s, uint value);
    QString tab(bool chord, int string, int fret);
	QString getNote(QString note, int duration, bool dot);
	QString cleanString(QString str);
};

#endif
