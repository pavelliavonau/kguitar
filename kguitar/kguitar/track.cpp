#include "track.h"

#include <qfile.h>
#include <qdatastream.h>

bool TabSong::load_from_kg(const char* fileName)
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

    printf("Read headers...\n");

    // TRACK DATA
    int cnt;
    s >> cnt;                           // Track count
    t.clear();

    printf("Going to read %d track(s)...\n",cnt);

    Q_UINT8 thetune[MAX_STRINGS];
    int ccnt;
    Q_UINT16 i16;
    Q_UINT8 patch,string;
    Q_INT8 cn;

    for (int i=0;i<cnt;i++) {
	s >> i16;                       // Track properties (bank)
	s >> patch;
	s >> string;

	if (string>MAX_STRINGS)
	    return FALSE;

	printf("Read a track of %d strings, bank=%d, patch=%d...\n",string,i16,patch);

	t.append(new TabTrack(i16,patch,string));

	printf("Appended a track...\n");

	for (int j=0;j<string;j++)
	    s >> thetune[j];

	printf("Read the tuning...\n");

	t.current()->setTuning(thetune);

	printf("Set the tuning...\n");
	
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

bool TabSong::save_to_kg(const char* fileName)
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

	s << (Q_UINT16) trk->bank();    // Track properties
	s << (Q_UINT8) trk->patch();
	s << (Q_UINT8) trk->string();
	for (int i=0;i<trk->string();i++)
	    s << (Q_UINT8) trk->tune(i);

	s << trk->c.count();            // Track columns

 	QListIterator<TabColumn> ic(trk->c);
 	for (;ic.current();++ic) {
 	    TabColumn *col = ic.current();
	    for (int i=0;i<trk->string();i++)
		s << (Q_INT8) col->a[i];
	    s << (Q_INT16) col->l;      // Duration
 	}
    }

    f.close();

    return TRUE;
}

bool TabSong::load_from_gtp(const char* fileName)
{
    // Loading from Guitar Pro format here
    return FALSE;
}

bool TabSong::save_to_gtp(const char* fileName)
{
    // Saving to Guitar Pro format here
    return FALSE;
}
