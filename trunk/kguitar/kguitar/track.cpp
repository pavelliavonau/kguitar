#include "track.h"
#include "global.h"

#include <qfile.h>
#include <qdatastream.h>

TabTrack::TabTrack(TrackMode _tm, QString _name, int _channel,
		   int _bank, uchar _patch, uchar _string, uchar _frets)
{
    tm=_tm;
    name=_name;
    channel=_channel;
    bank=_bank;
    patch=_patch;
    string=_string;
    frets=_frets;
};

bool TabSong::load_from_kg(QString fileName)
{
    return FALSE;
/*
    QFile f(fileName);
    if (!f.open(IO_ReadOnly))
	return FALSE;

    QDataStream s(&f);

    // HEADER SIGNATURE
    char hdr[4];
    s.readRawBytes(hdr,3); // should be KG\0 header
    if (!((hdr[0]=='K') && (hdr[1]=='G') && (hdr[2]==0)))
	return FALSE;

    // FILE VERSION NUMBER
    Q_UINT8 ver;
    s >> ver; // we could only read version 1 files
    if (ver!=1)
	return FALSE;

    // HEADER SONG DATA
    s >> title;
    s >> author;
    s >> transcriber;
    s >> comments;
    s >> tempo;

    if (tempo<0) {
	printf("Bad tempo");
	return FALSE;
    }

    printf("Read headers...\n");

    // TRACK DATA
    int cnt;
    s >> cnt;                           // Track count
    
    if (cnt<=0) {
	printf("Bad track count");
	return FALSE;
    }

    t.clear();

    printf("Going to read %d track(s)...\n",cnt);

    Q_UINT16 i16;
    Q_UINT8 channel,patch,string,frets,tm,event,elength;
    Q_INT8 cn;
    QString tn;

    for (int i=0;i<cnt;i++) {
	s >> tm;                        // Track properties (Track mode)

	// GREYFIX - todo track mode check

	s >> tn;                        // Track name
	s >> channel;
	s >> i16;                       // Bank
	s >> patch;
	s >> string;
	s >> frets;

	if (string>MAX_STRINGS)
	    return FALSE;

	printf("Read a track of %d strings, bank=%d, patch=%d...\n",string,i16,patch);

	t.append(new TabTrack((TrackMode) tm,tn,channel,i16,patch,string,frets));

	printf("Appended a track...\n");

	for (int j=0;j<string;j++) {
	    s >> cn;
	    t.current()->tune[j] = cn;
	}

	printf("Read the tuning...\n");

	bool finished=FALSE;

	do {
	    s >> event;
	    s >> elength;

	    switch (event) {
	    case 'T':                   // Tab column
		t.current()->c.append(new TabColumn());
		for (int k=0;k<string;k++) {
		    s >> cn;
		    t.current()->c.current()->a[k] = cn;
		}
		s >> i16;
		t.current()->c.current()->l = i16;
		break;
	    case 'X':                   // End of track
		finished=TRUE;
		break;
	    default:
		printf("Non-fatal error: unknown event %c. Skipping...\n",event);
		for (int k=0;k<elength;k++)
		    s >> cn;
		break;
	    }
	} while (!finished);
    }

    f.close();

    return TRUE;
*/
}

bool TabSong::save_to_kg(QString fileName)
{
    return FALSE;
/*
    QFile f(fileName);
    if (!f.open(IO_WriteOnly))
	return FALSE;

    QDataStream s(&f);

    // HEADER SIGNATURE
    s.writeRawBytes("KG\0",3);

    // VERSION SIGNATURE
    s << (Q_UINT8) 1;

    // HEADER SONG DATA
    s << title;
    s << author;
    s << transcriber;
    s << comments;
    s << tempo;

    // TRACK DATA
    s << t.count();                     // Number of tracks

    QListIterator<TabTrack> it(t);
    for (;it.current();++it) {          // For every track
	TabTrack *trk = it.current();

	s << (Q_UINT8) trk->trackmode();// Track properties
	s << trk->name;
	s << (Q_UINT8) trk->channel;
	s << (Q_UINT16) trk->bank;
	s << (Q_UINT8) trk->patch;
	s << (Q_UINT8) trk->string;
	s << (Q_UINT8) trk->frets;
	for (int i=0;i<trk->string;i++)
	    s << (Q_UINT8) trk->tune[i];

	// TRACK EVENTS

 	QListIterator<TabColumn> ic(trk->c);

	Q_UINT8 tcsize=trk->string+2;

 	for (;ic.current();++ic) {
	    s << (Q_UINT8) 'T';         // Tab column events
	    s << (Q_UINT8) tcsize;      // Size of event
 	    TabColumn *col = ic.current();
	    for (int i=0;i<trk->string;i++)
		s << (Q_INT8) col->a[i];
	    s << (Q_INT16) col->l;      // Duration
 	}

	s << (Q_UINT8) 'X';             // End of track marker
    }

    f.close();

    return TRUE;
*/
}

bool TabSong::load_from_gtp(QString fileName)
{
    // Loading from Guitar Pro format here
    return FALSE;
}

bool TabSong::save_to_gtp(QString fileName)
{
    // Saving to Guitar Pro format here
    return FALSE;
}

bool TabSong::load_from_mid(QString fileName)
{
    // Loading from MIDI file here
    return FALSE;
}

bool TabSong::save_to_mid(QString fileName)
{
    return FALSE;
/*
    QFile f(fileName);
    if (!f.open(IO_WriteOnly))
	return FALSE;

    QDataStream s(&f);

    // HEADER SIGNATURE

    s.writeRawBytes("MThd",4);       
    s << (Q_INT32) 6;                   // Length?
    s << (Q_INT16) 0;                   // Format - GREYFIX
    s << (Q_INT16) t.count();           // Number of tracks
    s << (Q_INT16) 96;                  // Divisions

    // TRACK DATA

    QListIterator<TabTrack> it(t);
    for (;it.current();++it) {          // For every track
	TabTrack *trk = it.current();

	s.writeRawBytes("MTrk",4);      // Track header
	s << (Q_INT32) 0;               // Length - GREYFIX

	s << trk->c.count();            // Track columns

 	QListIterator<TabColumn> ic(trk->c);
 	for (;ic.current();++ic) {
 	    TabColumn *col = ic.current();
	    for (int i=0;i<trk->string;i++)
		s << (Q_INT8) col->a[i];
	    s << (Q_INT16) col->l;      // Duration
 	}
    }

    f.close();

    return TRUE;
*/
}

//////////////////////////////////////////////////////////////////////
// ASCII TAB loading/saving stuff

#define twidth          70

// Quick & easy centered text writing function
void TabSong::writeCentered(QTextStream *s, QString l)
{
    for (int i=0;i<(twidth-(int) l.length())/2;i++) {
	(*s) << ' ';
    }
    (*s) << l << '\n';
}

bool TabSong::load_from_tab(QString fileName)
{
    return FALSE;
}

bool TabSong::save_to_tab(QString fileName)
{
    return FALSE;
/*
    QFile f(fileName);
    if (!f.open(IO_WriteOnly))
	return FALSE;

    QTextStream s(&f);

    // SONG HEADER

    writeCentered(&s,title);
    s << '\n';
    writeCentered(&s,"Author: "+author);
    writeCentered(&s,"Transcribed by: "+transcriber);

    // GREYFIX - comments?

    s << "Tempo: " << tempo << "\n\n";

    // TRACK DATA

    QListIterator<TabTrack> it(t);

    int n=1;

    QString lin[MAX_STRINGS];
    QString tmp;

    for (;it.current();++it) {          // For every track	
	TabTrack *trk = it.current();

	s << "Track " << n << ": " << trk->name << "\n\n";

	// GREYFIX - channel, bank, patch, string, frets data

// 	for (int i=0;i<trk->string;i++)
// 	    s << (Q_UINT8) trk->tune[i];

	int minstart=1;
	for (int i=0;i<trk->string;i++)
	    if (note_name(trk->tune[i]%12).length()>1)
		minstart=2;
	    
	for (int i=0;i<trk->string;i++) {
	    lin[i]=note_name(trk->tune[i]%12);
	    if ((lin[i].length()==1) && (minstart>1))
		lin[i]=lin[i]+' ';
	    lin[i]=lin[i]+" |-";
	}

 	QListIterator<TabColumn> ic(trk->c);
	bool lng=FALSE;

 	for (;ic.current();++ic) {
 	    TabColumn *col = ic.current();
	    lng=FALSE;

	    for (int i=0;i<trk->string;i++)
		if (col->a[i]>=10)
		    lng=TRUE;

	    for (int i=0;i<trk->string;i++) {
		if (col->a[i]==-1) {
		    if (lng)
			lin[i]=lin[i]+"--";
		    else
			lin[i]=lin[i]+'-';
		} else {
		    tmp.setNum(col->a[i]);
		    if ((lng) && (col->a[i]<10))
			tmp='-'+tmp;
		    lin[i]=lin[i]+tmp;
		}
		for (uint j=0;j<(col->l/48);j++)
		    lin[i]=lin[i]+'-';
	    }

	    if (lin[0].length()>twidth) {
		for (int i=trk->string-1;i>=0;i--)
		    s << lin[i] << '\n';
		s << '\n';
		for (int i=0;i<trk->string;i++) {
		    lin[i]=note_name(trk->tune[i]%12);
		    if ((lin[i].length()==1) && (minstart>1))
			lin[i]=lin[i]+' ';
		    lin[i]=lin[i]+" |-";
		}
	    }
 	}

	for (int i=trk->string-1;i>=0;i--)
	    s << lin[i] << '\n';
	s << '\n';

	n++;   // Numerical track counter
    }

    f.close();

    return TRUE;
*/
}
