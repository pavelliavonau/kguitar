#include "track.h"

#include <qfile.h>
#include <qdatastream.h>

bool TabSong::load_from_kg(QString fileName)
{
    QFile f(fileName);
    if (!f.open(IO_ReadOnly))
	return FALSE;

    QDataStream s(&f);

    // HEADER SIGNATURE
    char hdr[4];
    s.readRawBytes(hdr,3); // should be KG\0 header
    if (!((hdr[0]=='K') && (hdr[1]=='G') && (hdr[2]==0)))
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

    int ccnt;
    Q_UINT16 i16;
    Q_UINT8 patch,string,frets,tm;
    Q_INT8 cn;
    QString tn;

    for (int i=0;i<cnt;i++) {
	s >> tm;                        // Track properties (Track mode)

	// GREYFIX - todo track mode check

	s >> tn;                        // Track name
	s >> i16;                       // Bank
	s >> patch;
	s >> string;
	s >> frets;

	if (string>MAX_STRINGS)
	    return FALSE;

	printf("Read a track of %d strings, bank=%d, patch=%d...\n",string,i16,patch);

	t.append(new TabTrack((TrackMode) tm,tn,i16,patch,string,frets));

	printf("Appended a track...\n");

	for (int j=0;j<string;j++) {
	    s >> cn;
	    t.current()->tune[j] = cn;
	}

	printf("Read the tuning...\n");
	
	s >> ccnt;

	printf("Read the column header... Going to get %d...\n",ccnt);

	for (int j=0;j<ccnt;j++) {
	    t.current()->c.append(new TabColumn());
	    for (int k=0;k<string;k++) {
		s >> cn;
		t.current()->c.current()->a[k] = cn;
	    }
	    s >> i16;
	    t.current()->c.current()->l = i16;
	}
    }

    f.close();

    return TRUE;
}

bool TabSong::save_to_kg(QString fileName)
{
    QFile f(fileName);
    if (!f.open(IO_WriteOnly))
	return FALSE;

    QDataStream s(&f);

    // HEADER SIGNATURE
    s.writeRawBytes("KG\0",3);

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
	s << (Q_UINT16) trk->bank;
	s << (Q_UINT8) trk->patch;
	s << (Q_UINT8) trk->string;
	s << (Q_UINT8) trk->frets;
	for (int i=0;i<trk->string;i++)
	    s << (Q_UINT8) trk->tune[i];

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
}
