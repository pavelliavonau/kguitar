#include "tabsong.h"
#include "global.h"
#include "application.h"

#include "midilist.h"

#include <qfile.h>
#include <qdatastream.h>

#include <kdebug.h>

TabSong::TabSong(QString _title, int _tempo)
{
	tempo = _tempo;
	title = _title;
	t.setAutoDelete(TRUE);
}

// Helper functions for duration conversion

// Dot + undotted length -> full length
Q_UINT16 dot2len(int len, bool dot)
{
	return (Q_UINT16) (dot ? len + len / 2 : len);
}

// Normal durations
int nordur[6] = {480, 240, 120, 60, 30, 15};
// Dotted durations
int dotdur[6] = {720, 360, 180, 90, 45, 23};

// Full length -> dot + undotted length
void len2dot(int l, int *len, bool *dot)
{
	for (uint i = 0; i < 6; i++) {
		if (nordur[i] == l) {
			*len = l;
			*dot = FALSE;
			return;
		}
		if (dotdur[i] == l) {
			*len = l * 2 / 3;
			*dot = TRUE;
			return;
		}
	}
}

// Find the minimal free channel, for example, to use for new track
int TabSong::freeChannel()
{
	bool fc[17];
	for (int i = 1; i <= 16; i++)
		fc[i] = TRUE;

	QListIterator<TabTrack> it(t);
	for (; it.current(); ++it)
		fc[it.current()->channel] = FALSE;

	int res;
	for (res = 1; res <= 16; res++)
		if (fc[res])
			break;

	if (res > 16)
		res = 1;

	return res;
}

// Returns length of longest track in bars
uint TabSong::maxLen()
{
	uint res = 0;

	QListIterator<TabTrack> it(t);
	for (; it.current(); ++it)
		res = it.current()->b.size() > res ? it.current()->b.size() : res;

	return res;
}

// KG format specs
// ===============
// It's really internal stuff of KGuitar and could be changed without any
// notices, but generally...

// General header:
// 3 bytes - 'K' 'G' 0 - general signature
// 1 byte  - version number of _file_format_. Should be 2 (older 1 format
//           included non-unicode strings)

// Song properties (strings in Qt format)
// string  - title
// string  - author
// string  - transcriber
// string  - comments
// 4 bytes - starting tempo number
// 4 bytes - number of tracks

// Then, track header and track data repeated for every track.
// Track header:
// 1 byte  - track mode
// string  - track name
// 1 byte  - MIDI channel
// 2 bytes - MIDI bank
// 1 byte  - MIDI patch
// 1 byte  - number of strings (x)
// 1 byte  - number of frets
// x bytes - tuning, one byte per string

// Track data - repeated event chunk.
// Event chunk:

// 1 byte  - event type (et)
// 1 byte  - event length (length of following data in bytes)

// et='X' - end of track, no more reading track chunks
// et='T' - tab column: x bytes - raw tab data, 2 bytes - duration of column
// et='C' - continuation of prev column: 2 bytes - duration addition
// et='E' - effect of prev column: x bytes - raw FX data
// et='F' - flag of prev column: 1 byte - raw flag data
// et='B' - new bar start
// et='S' - new time signature: 2 bytes - time1:time2

bool TabSong::load_from_kg(QString fileName)
{
	QFile f(fileName);
	if (!f.open(IO_ReadOnly))
		return FALSE;

	QDataStream s(&f);

	// HEADER SIGNATURE
	char hdr[4];
	s.readRawBytes(hdr, 3); // should be KG\0 header
	if (!((hdr[0] == 'K') && (hdr[1] == 'G') && (hdr[2] == 0)))
		return FALSE;

	// FILE VERSION NUMBER
	Q_UINT8 ver;
	s >> ver; // version 2 files are unicode KDE2 files
	if ((ver < 1) || (ver > 2))
		return FALSE;

	// HEADER SONG DATA
	s >> title;
	s >> author;
	s >> transcriber;
	s >> comments;
	s >> tempo;

	if (tempo < 0) {
		kdDebug() << "Bad tempo" << endl;
		return FALSE;
	}

	kdDebug() << "Read headers..." << endl;

	// TRACK DATA
	int cnt;
	s >> cnt; // Track count

	if (cnt <= 0) {
		kdDebug() << "Bad track count" << endl;
		return FALSE;
	}

	t.clear();

	kdDebug() << "Going to read " << cnt << " track(s)..." << endl;

	Q_UINT16 i16;
	Q_UINT8 channel, patch, string, frets, tm, event, elength;
	Q_INT8 cn;
	QString tn;

	for (int i = 0; i < cnt; i++) {
		s >> tm; // Track properties (Track mode)

		// GREYFIX - todo track mode check

		s >> tn; // Track name
		s >> channel;
		s >> i16; // Bank
		s >> patch;
		s >> string;
		s >> frets;

		if (string > MAX_STRINGS)
			return FALSE;

		kdDebug() << "Read a track of " << string << " strings," << endl;
		kdDebug() << "       bank = " << i16 << ", patch = " << patch << " ..." << endl;

		t.append(new TabTrack((TrackMode) tm,tn,channel,i16,patch,string,frets));

		kdDebug() << "Appended a track..." << endl;;

		for (int j = 0; j < string; j++) {
			s >> cn;
			t.current()->tune[j] = cn;
		}

		kdDebug() << "Read the tuning..." << endl;;

		bool finished = FALSE;

		int x = 0, bar = 1;
// uchar tcsize=t.current()->string+2;
		t.current()->c.resize(1);
		t.current()->b.resize(1);
		t.current()->b[0].start = 0;
		t.current()->b[0].time1 = 4;
		t.current()->b[0].time2 = 4;

		bool dot;
		int dur;
		kdDebug() << "reading events" << endl;;
		do {
			s >> event;
			s >> elength;

			switch (event) {
			case 'B':                   // Tab bar
				bar++;
				t.current()->b.resize(bar);
				t.current()->b[bar-1].start=x;
				t.current()->b[bar-1].time1=t.current()->b[bar-2].time1;
				t.current()->b[bar-1].time2=t.current()->b[bar-2].time2;
				break;
			case 'T':                   // Tab column
				x++;
				t.current()->c.resize(x);
				for (int k = 0; k < string; k++) {
					s >> cn;
					t.current()->c[x-1].a[k] = cn;
					t.current()->c[x-1].e[k] = 0;
				}
				s >> i16;
				len2dot(i16, &dur, &dot);
				t.current()->c[x-1].l = dur;
				t.current()->c[x-1].flags = (dot ? FLAG_DOT : 0);
				break;
			case 'E':                   // Effects of prev column
				if (x == 0) {			// Ignore if there were no tab cols
					kdDebug() << "Warning: FX column with no tab columns, ignoring..." << endl;
					break;
				}
				for (int k = 0; k < string; k++) {
					s >> cn;
					t.current()->c[x-1].e[k] = cn;
				}
				break;
			case 'F':                   // Flag of prev column
				if (x == 0) {			// Ignore if there were no tab cols
					kdDebug() << "Warning: flag with no tab columns, ignoring..." << endl;
					break;
				}
				s >> cn; t.current()->c[x-1].flags = cn;
				break;
			case 'L':					// Continuation of previous column
				x++;
				t.current()->c.resize(x);
				for (int k = 0; k < string; k++)
					t.current()->c[x-1].a[k] = -1;
				s >> i16;
				len2dot(i16, &dur, &dot);
				t.current()->c[x-1].l = dur;
				t.current()->c[x-1].flags = (dot ? FLAG_ARC | FLAG_DOT : FLAG_ARC);
				break;
			case 'S':                   // New time signature
				s >> cn; t.current()->b[bar-1].time1 = cn;
				s >> cn; t.current()->b[bar-1].time2 = cn;
				break;
			case 'X':					// End of track
				finished = TRUE;
				break;
			default:
				kdDebug() << "Warning: unknown event " << event << " Skipping..." << endl;
				for (int k = 0; k < elength; k++)
					s >> cn;
				break;
			}
		} while ((!finished) && (!s.eof()));

		t.current()->x = 0;
		t.current()->xb = 0;
		t.current()->y = 0;
	}

	f.close();

	return TRUE;
}

void TabSong::arrangeBars()
{
	QListIterator<TabTrack> it(t);
	for (; it.current(); ++it) {		// For every track
		TabTrack *trk = it.current();
		trk->arrangeBars();
	}
}

bool TabSong::save_to_kg(QString fileName)
{
	QFile f(fileName);
	if (!f.open(IO_WriteOnly))
		return FALSE;

	QDataStream s(&f);

	// HEADER SIGNATURE
	s.writeRawBytes("KG\0",3);

	// VERSION SIGNATURE
	s << (Q_UINT8) 2;

	// HEADER SONG DATA
	s << title;
	s << author;
	s << transcriber;
	s << comments;
	s << tempo;

	// TRACK DATA
	s << t.count();						// Number of tracks

	bool needfx = FALSE;				// Should we write FX event after tab?

	QListIterator<TabTrack> it(t);
	for (; it.current(); ++it) {		// For every track
		TabTrack *trk = it.current();

		s << (Q_UINT8) trk->trackMode();// Track properties
		s << trk->name;
		s << (Q_UINT8) trk->channel;
		s << (Q_UINT16) trk->bank;
		s << (Q_UINT8) trk->patch;
		s << (Q_UINT8) trk->string;
		s << (Q_UINT8) trk->frets;
		for (int i = 0; i<trk->string; i++)
			s << (Q_UINT8) trk->tune[i];

		// TRACK EVENTS

		Q_UINT8 tcsize = trk->string+2;
		uint bar = 1;

		s << (Q_UINT8) 'S';				// Time signature event
		s << (Q_UINT8) 2;				// 2 byte event length
		s << (Q_UINT8) trk->b[0].time1; // Time signature itself
		s << (Q_UINT8) trk->b[0].time2;

		for (uint x = 0; x < trk->c.size(); x++) {
			if (bar+1 < trk->b.size()) {	// This bar's not last
				if (trk->b[bar+1].start == x)
					bar++;				// Time for next bar
			}

			if ((bar < trk->b.size()) && (trk->b[bar].start == x)) {
				s << (Q_UINT8) 'B';     // New bar event
				s << (Q_UINT8) 0;
			}

			if (trk->c[x].flags & FLAG_ARC) {
				s << (Q_UINT8) 'L';		// Continue of previous event
				s << (Q_UINT8) 2;		// Size of event
				s << dot2len(trk->c[x].l, trk->c[x].flags & FLAG_DOT); // Duration
			} else {
				s << (Q_UINT8) 'T';		// Tab column events
				s << (Q_UINT8) tcsize;	// Size of event
				needfx = FALSE;
				for (int i = 0;i < trk->string; i++) {
					s << (Q_INT8) trk->c[x].a[i];
					if (trk->c[x].e[i])
						needfx = TRUE;
				}
				s << dot2len(trk->c[x].l, trk->c[x].flags & FLAG_DOT); // Duration
				if (needfx) {
					s << (Q_UINT8) 'E'; // Effect event
					s << (Q_UINT8) trk->string; // Size of event
					for (int i = 0; i < trk->string; i++)
						s << (Q_UINT8) trk->c[x].e[i];
				}
				if (trk->c[x].flags) {
					s << (Q_UINT8) 'F'; // Flag event
					s << (Q_UINT8) 1;   // Size of event
					s << (Q_UINT8) trk->c[x].flags;
				}
			}
		}

		s << (Q_UINT8) 'X';				// End of track marker
		s << (Q_UINT8) 0;				// Length of end track event
	}

	f.close();

	return TRUE;
}

bool TabSong::load_from_gtp(QString fileName)
{
	QFile f(fileName);
	if (!f.open(IO_ReadOnly))
		return FALSE;

	QDataStream s(&f);



    // Loading from Guitar Pro format here
    return FALSE;
}

bool TabSong::save_to_gtp(QString fileName)
{
    // Saving to Guitar Pro format here
    return FALSE;
}

Q_UINT32 TabSong::readVarLen(QDataStream *s)
{
	Q_UINT32 value;
	Q_UINT8 c;

	(*s) >> c;
	value = c;

	if (value & 0x80) {
		value &= 0x7f;
		do {
			(*s) >> c;
			value = (value << 7) + ((c & 0x7f));
		} while (c & 0x80);
	}

	return value;
}

bool TabSong::load_from_mid(QString fileName)
{
	return FALSE; // DISABLED

    QFile f(fileName);

    if (!f.open(IO_ReadOnly))
		return FALSE;

    QDataStream s(&f);

    // HEADER SIGNATURE

	char hdr[5];
    s.readRawBytes(hdr, 4);
	if ((hdr[0] != 'M') || (hdr[1] != 'T') ||
		(hdr[2] != 'h') || (hdr[3] != 'd'))
		return FALSE;

	Q_UINT32 tmp32;
	Q_UINT16 midiformat, tracknum, divisions;

	// LENGTH OF HEADER. SHOULD BE 6

    s >> tmp32;
	if (tmp32 != 6)
		return FALSE;

	s >> midiformat;                    // MIDI file format
	s >> tracknum;                      // Number of tracks
    s >> divisions;                     // Divisions

    // TRACK DATA

	int delta;
	Q_UINT8 evtype;
	Q_UINT8 tmp8, data1, data2;

	for (int tr = 0; tr < tracknum; tr++) {
		s.readRawBytes(hdr, 4);
		if ((hdr[0] != 'M') || (hdr[1] != 'T') ||
			(hdr[2] != 'r') || (hdr[3] != 'k'))
			return FALSE;
		s >> tmp32; // length
		kdDebug() << "Track length = " << tmp32 << endl;
		do {
			delta = readVarLen(&s);
			s >> evtype;

			// Meta event
			if (evtype == 0xff) {
				s >> tmp8; // meta event type
				tmp32 = readVarLen(&s);

				kdDebug() << "Meta event " << tmp8 << ", " << tmp32 << " bytes long " << endl;

				if (tmp8 == 0x2f)
					break;

				for (uint i = 0; i < tmp32; i++)
					s >> tmp8;
				continue;
			}

			int channel = evtype & 0xf;
			evtype &= 0xf0;

			s >> data1;
			s >> data2;

			kdDebug() << "Delta = " << delta << ", Channel = " << channel << endl;
			kdDebug() << "   Event = " << evtype << ", Data1 = " << data1 << endl;
			kdDebug() << "   Data2 = " << data2 << endl;

		} while (TRUE);
	}

    f.close();

    return FALSE;
}

void TabSong::writeVarLen(QDataStream *s, uint value)
{
	Q_UINT32 buffer;

	buffer = value & 0x7f;
	while ((value >>= 7) > 0) {
		buffer <<= 8;
		buffer |= 0x80;
		buffer += (value & 0x7f);
	}

	while (TRUE) {
		(*s) << (Q_UINT8) (buffer & 0xff);

		if (buffer & 0x80)
			buffer >>= 8;
		else
			return;
	}
}

void TabSong::writeTempo(QDataStream *s, uint value)
{
	(*s) << (Q_UINT8) (value >> 16);
	(*s) << (Q_UINT8) (value >> 8);
	(*s) << (Q_UINT8) value;
}

bool TabSong::save_to_mid(QString fileName)
{
    QFile f(fileName);

    if (!f.open(IO_WriteOnly))
		return FALSE;

    QDataStream s(&f);

    // HEADER SIGNATURE

    s.writeRawBytes("MThd", 4);
    s << (Q_UINT32) 6;                  // Length?
    s << (Q_UINT16) 1;                  // Format 1 - GREYFIX
    s << (Q_UINT16) (t.count() + 1);    // Number of tracks plus 1 tempo track
    s << (Q_UINT16) 120;                // Divisions

	// TRACK VARIABLES

	int sizepos;                        // Where to write track size in file
	int curpos;                         // Where we end up writing track

	// TEMPO TRACK

	s.writeRawBytes("MTrk", 4);
	sizepos = f.at();                   // Remember where to write track size
	s << (Q_UINT32) 0;                  // Store 0 as size temporarily

	// Song name
	s << (Q_UINT8) 0;
	s << (Q_UINT8) MIDI_META;
	s << (Q_UINT8) META_SEQUENCE_NAME;
	writeVarLen(&s, title.length());
	s.writeRawBytes(title, title.length());

	// Copyright
	s << (Q_UINT8) 0;
	s << (Q_UINT8) MIDI_META;
	s << (Q_UINT8) META_SEQUENCE_NAME;
	writeVarLen(&s, author.length());
	s.writeRawBytes(author, author.length());

	// First time signature
	s << (Q_UINT8) 0;
	s << (Q_UINT8) 0xff;
	s << (Q_UINT8) 0x58;
	s << (Q_UINT8) 4;
	s << (Q_UINT8) t.first()->b[0].time1;
	// Time signature denominator
	Q_UINT8 denom  = 1;
	switch (t.first()->b[0].time2) {
	case 1:	 denom = 0; break;
	case 2:	 denom = 1; break;
	case 4:	 denom = 2; break;
	case 8:	 denom = 3; break;
	case 16: denom = 4; break;
	case 32: denom = 5; break;
	}
	s << denom;
	s << (Q_UINT8) 24;
	s << (Q_UINT8) 8;

	// Tempo
	s << (Q_UINT8) 0;
	s << (Q_UINT8) 0xff;
	s << (Q_UINT8) 0x51;
	s << (Q_UINT8) 3;
	writeTempo(&s, 60000000 / tempo);

	// End of track marker
	s << (Q_UINT8) 0;
	s << (Q_UINT8) MIDI_META;
	s << (Q_UINT8) META_END_TRACK;
	s << (Q_UINT8) 0;

	curpos = f.at();
	f.at(sizepos);
	s << (Q_UINT32) (curpos - sizepos - 4);
	f.at(curpos);

    // TRACK DATA

	MidiList ml;                          // Sorted list of events

    QListIterator<TabTrack> it(t);
	for (; it.current(); ++it) {		  // For every track
		TabTrack *trk = it.current();

		s.writeRawBytes("MTrk", 4);		  // Track header
		sizepos = f.at();				  // Remember where to write track size
		s << (Q_UINT32) 0;				  // Store 0 as size temporarily

		s << (Q_UINT8) 0;				  // Track name
		s << (Q_UINT8) MIDI_META;
		s << (Q_UINT8) META_SEQUENCE_NAME;
		writeVarLen(&s, trk->name.length());
		s.writeRawBytes(trk->name, trk->name.length());

		// TRACK EVENTS

		// Bank & patch select
		s << (Q_UINT8) 0;				// No delta time
		s << (Q_UINT8) (MIDI_CONTROL_CHANGE | (trk->channel - 1));
		s << (Q_UINT8) 0;				// Controller 0 - high bank byte
		s << (Q_UINT8) (trk->bank >> 8);
		s << (Q_UINT8) 1;				// Slight delay
		s << (Q_UINT8) 32;				// Controller 32 - low bank byte
		s << (Q_UINT8) (trk->bank & 0xff);
		s << (Q_UINT8) 1;				// Slight delay
		s << (Q_UINT8) (MIDI_PROGRAM_CHANGE | (trk->channel - 1));
		s << (Q_UINT8) trk->patch;

		ml.clear();

		MidiData::getMidiList((TabTrack *) trk, ml);

		MidiEvent *e;
		Q_UINT8 lastevent = 0;
		long laststamp = 0;

		for (e = ml.first(); e != 0; e = ml.next()) {
			writeVarLen(&s, e->timestamp - laststamp);
			if (lastevent != e->type)
				s << e->type;
			s << e->data1;
			s << e->data2;

			lastevent = e->type;
			laststamp = e->timestamp;
		}

/*			if (trk->c[x].flags & FLAG_ARC) {
				s << (Q_UINT8) 'L';		// Continue of previous event
				s << (Q_UINT8) 2;		// Size of event
				s << dot2len(trk->c[x].l, trk->c[x].flags & FLAG_DOT); // Duration
			} else {
				s << (Q_UINT8) 'T';		// Tab column events
				s << (Q_UINT8) tcsize;	// Size of event
				needfx = FALSE;
				for (int i=0;i<trk->string;i++) {
					s << (Q_INT8) trk->c[x].a[i];
					if (trk->c[x].e[i])
						needfx = TRUE;
				}
				s << dot2len(trk->c[x].l, trk->c[x].flags & FLAG_DOT); // Duration
				if (needfx) {
					s << (Q_UINT8) 'E'; // Effect event
					s << (Q_UINT8) trk->string; // Size of event
					for (int i=0;i<trk->string;i++)
						s << (Q_UINT8) trk->c[x].e[i];
				}
				}
	    }
*/

		s << (Q_UINT8) 0;               // End of track marker
		s << (Q_UINT8) MIDI_META;
		s << (Q_UINT8) META_END_TRACK;
		s << (Q_UINT8) 0;

		curpos = f.at();
		f.at(sizepos);
		s << (Q_UINT32) (curpos - sizepos - 4);
		f.at(curpos);
	}

    f.close();

    return TRUE;
}

//////////////////////////////////////////////////////////////////////
// ASCII TAB loading/saving stuff

#define twidth          70

// Quick & easy centered text writing function
void TabSong::writeCentered(QTextStream *s, QString l)
{
    for (int i=0;i<(twidth-(int) l.length())/2;i++)
	(*s) << ' ';
    (*s) << l << '\n';
}

bool TabSong::load_from_tab(QString fileName)
{
    return FALSE;
}

bool TabSong::save_to_tab(QString fileName)
{
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

    int n = 1;

    QString lin[MAX_STRINGS];
    QString tmp;

    for (;it.current();++it) {          // For every track
		TabTrack *trk = it.current();

		s << "Track " << n << ": " << trk->name << "\n\n";

		// GREYFIX - channel, bank, patch, string, frets data

		int minstart = 1;
		for (int i = 0; i < trk->string; i++)
			if (note_name(trk->tune[i] % 12).length() > 1)
				minstart = 2;

		for (int i = 0; i < trk->string; i++) {
			lin[i] = note_name(trk->tune[i] % 12);
			if ((lin[i].length() == 1) && (minstart > 1))
				lin[i] = lin[i]+' ';
			lin[i] = lin[i] + " |-";
		}

		bool lng = FALSE;
		uint bar = 1;

		for (uint x = 0; x < trk->c.size(); x++) {
			if (bar + 1 < trk->b.size()) {  // This bar's not last
				if (trk->b[bar+1].start == x)
					bar++;              // Time for next bar
			}

			if (trk->b[bar].start == x)   // Add a bar
				for (int i = 0; i < trk->string; i++)
					lin[i] = lin[i] + "|";

			lng = FALSE;

			for (int i = 0; i < trk->string; i++)
				if (trk->c[x].a[i] >= 10)
					lng = TRUE;

			for (int i = 0; i < trk->string; i++) {
				if (trk->c[x].a[i] == -1) {
					if (lng)
						lin[i] = lin[i] + "--";
					else
						lin[i] = lin[i] + '-';
				} else {
					tmp.setNum(trk->c[x].a[i]);
					if ((lng) && (trk->c[x].a[i] < 10))
						tmp = '-' + tmp;
					lin[i] = lin[i] + tmp;
				}
				for (uint j = 0; j < (trk->c[x].l / 48); j++)
					lin[i] = lin[i] + '-';
			}

			if (lin[0].length() > twidth) {
				for (int i = trk->string-1; i >= 0; i--)
					s << lin[i] << '\n';
				s << '\n';
				for (int i = 0; i < trk->string; i++) {
					lin[i] = note_name(trk->tune[i] % 12);
					if ((lin[i].length() == 1) && (minstart > 1))
						lin[i] = lin[i] + ' ';
					lin[i] = lin[i] + " |-";
				}
			}
		}

		for (int i = trk->string-1; i >= 0; i--)
			s << lin[i] << '\n';
		s << '\n';

		n++;   // Numerical track counter
    }

    f.close();

    return TRUE;
}


//////////////////////////////////////////////////////////////////////
//
// MusiXTeX/kgtabs.tex export by alinx
//
// MusiXTeX is required to use exported files.
// Download it at ftp://ftp.dante.de/tex-archive/macros/musixtex/taupin
//             or http://www.gmd.de/Misc/Music
//

QString TabSong::tab(bool chord, int string, int fret)
{
	QString tmp, st, fr;

	st.setNum(string);
	fr.setNum(fret);

	if (chord)
		tmp="\\chotab";
	else
		tmp="\\tab";

	tmp += st;
	tmp += "{";
	tmp += fr;
	tmp += "}";

	return tmp;
}

QString TabSong::cleanString(QString str)  // insert control sequence
{
	QString tmp, toc;

	for (uint i=0; i < str.length(); i++){
		toc = str.mid(i, 1);
		if ((toc == "<") || (toc == ">"))
			tmp = tmp + "$" + toc + "$";
		else
			tmp = tmp + toc;
	}
	return tmp;
}

QString TabSong::getNote(QString note, int duration, bool dot)
{
	return "";
}

bool TabSong::save_to_tex_tab(QString fileName)
{
	QFile f(fileName);
    if (!f.open(IO_WriteOnly))
		return FALSE;

	QTextStream s(&f);

	QString nn[MAX_STRINGS];
	QString tmp;
	bool flatnote;

	QString bar, notes, tsize, showstr;

	bar = "\\bar";
	bar += "\n";
	notes = "\\Notes";
	showstr = "\\showstrings";

	switch (globalTabSize){
	case 0: tsize = "\\smalltabsize";
		break;
	case 1: tsize = "\\normaltabsize";
		break;
	case 2: tsize = "\\largetabsize";
		break;
	case 3: tsize = "\\Largetabsize";
		break;
	default: tsize = "\\largetabsize";
		break;
	}
	tsize += "\n";

	QListIterator<TabTrack>it(t);
	TabTrack *trk=it.current();

	// Stuff if globalShowStr=TRUE

	flatnote = FALSE;
	for (int i = 0; i < trk->string; i++) {
		nn[i] = note_name(trk->tune[i] % 12);
		if ((nn[i].contains("#", FALSE) == 1) && (nn[i].length() == 2)) {
			nn[i] = nn[i].left(1) + "$\\sharp$";
			flatnote = TRUE;
		}
		if ((nn[i].contains("b", FALSE) == 1) && (nn[i].length() == 2)) {
			nn[i] = nn[i].left(1) + "$\\flat$";
			flatnote = TRUE;
		}
	}

	tmp = "\\othermention{%";
	tmp += "\n";
	tmp += "\\noindent Tuning:\\\\";
	tmp += "\n";

	if (trk->string == 4){
		tmp += "\\tuning{1}{" + nn[3];
		tmp += "} \\quad \\tuning{3}{" + nn[1] + "} \\quad \\\\";
		tmp += "\n";
		tmp += "\\tuning{2}{" + nn[2];
		tmp += "} \\quad \\tuning{4}{" + nn[0] + "}";
		tmp += "\n";
	}

	if (trk->string == 5){
		tmp += "\\tuning{1}{" + nn[4];
		tmp += "} \\quad \\tuning{4}{" + nn[1] + "} \\quad \\\\";
		tmp += "\n";
		tmp += "\\tuning{2}{" + nn[3];
		tmp += "} \\quad \\tuning{5}{" + nn[0] + "} \\quad \\\\";
		tmp += "\n";
		tmp += "\\tuning{3}{" + nn[2] + "}";
		tmp += "\n";
	}

	if (trk->string == 6){
		tmp += "\\tuning{1}{" + nn[5];
		tmp += "} \\quad \\tuning{4}{" + nn[2] + "} \\quad \\\\";
		tmp += "\n";
		tmp += "\\tuning{2}{" + nn[4];
		tmp += "} \\quad \\tuning{5}{" + nn[1]+"} \\quad \\\\";
		tmp += "\n";
		tmp += "\\tuning{3}{" + nn[3];
		tmp += "} \\quad \\tuning{6}{" + nn[0]+"}";
		tmp += "\n";
	}

	if (trk->string >= 7){
		s << "Sorry, but MusiXTeX/kgtabs.tex has only 6 tablines" << "\n";
		s << "\\end" << "\n";
		f.close();
		return FALSE;
	}

	tmp += "}";
	tmp += "\n";

	if (trk->string < 4)
		tmp = "";

	for (int i = (trk->string - 1); i >= 0; i--){
		showstr += " ";
		showstr += note_name(trk->tune[i] % 12);
	}

	switch (trk->string){
	case 1: showstr += " X X X X X";         // now it is a defined control sequence
		break;
	case 2: showstr += " X X X X";
		break;
	case 3: showstr += " X X X";
		break;
	case 4: showstr += " X X";
		break;
	case 5: showstr += " X";
		break;
	default: break;
	}

	showstr += "\n";

	// TeX-File INFO-HEADER

	s << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << "\n";
	s << "%" << "\n";
	s << "% This MusiXTex File was created with KGuitar " << VERSION << "\n";
	s << "% $Id$" << "\n";
    s << "%" << "\n";
	s << "% You can download the latest version at:" << "\n";
	s << "%      http://kguitar.sourceforge.net" << "\n";
	s << "%" << "\n";
	s << "% MusiXTeX is required to use this file." << "\n";
	s << "% You can download it at one of the following sites:" << "\n";
	s << "%" << "\n";
	s << "%      ftp://ftp.dante.de/tex-archive/macros/musixtex/taupin/" << "\n";
    s << "%      http://www.gmd.de/Misc/Music/" << "\n";
	s << "%" << "\n";
	s << "% IMPORTANT: This file should not be used with LaTeX!" << "\n";
	s << "%" << "\n";
	s << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << "\n";

	// TeX-File HEADER
	s << "\\input musixtex" << "\n";
	s << "\\input musixsty" << "\n";
	s << "\\input kgtabs.tex" << "\n";

	// SONG HEADER

	if (!globalShowPageNumb)
		s << "\\nopagenumbers" << "\n";

	s << "\\fulltitle{" << cleanString(title) << "}";
	s << "\n";
	s << "\\subtitle{\\svtpoint\\bf Author: " << cleanString(author) << "}" << "\n";
	s << "\\author{Transcribed by: " << cleanString(transcriber);
	s << "\\\\%" << "\n";
	s << "        Tempo: " << tempo << "}";
	s << "\n";

	if (globalShowStr)
		s << tmp;

	s << "\\maketitle" << "\n";
	s << "\n";
	s << "\\settab1" << "\n";

	if (!globalShowBarNumb)
		s << "\\nobarnumbers" << "\n";

	s << "\\let\\extractline\\leftline" << "\n";
	s << "\n";

	// TRACK DATA
	int n = 1;       // Trackcounter
	int cho;         // more than one string in this column
	int width;
	uint trksize;
	uint bbar;       // who are bars?

	for (; it.current(); ++it) { // For every track
		TabTrack *trk = it.current();

		s << "Track " << n << ": " << trk->name;
		s << "\n";
		s << "\\generalmeter{\\meterfrac{" << trk->b[0].time1;
		s << "}{" << trk->b[0].time2 << "}}";
		s << "\n" << "\n"; // the 2nd LF is very important!!
		s << tsize;

		if (globalShowStr && (!flatnote))
			s << showstr;

		s << "\\startextract" << "\n";

		// make here the tabs

		cho = 0;
		width = 0;
		bbar = 1;
		trksize = trk->c.size();

		QString tmpline;

		for (uint j = 0; j < trksize; j++) { // for every column (j)
			tmpline = notes;

			if ((bbar + 1) < trk->b.size()) { // looking for bars
				if (trk->b[bbar + 1].start == j)  bbar++;
			}
			if (trk->b[bbar].start == j)  s << bar;

			for (int x = 0; x < trk->string; x++) // test how much tabs in this column
				if (trk->c[j].a[x]>=0)  cho++;

			for (int x = 0; x < trk->string; x++) {
				if ((trk->c[j].a[x] >= 0) && (cho == 1))
					s << notes << tab(FALSE, trk->string - x, trk->c[j].a[x]);
				if ((trk->c[j].a[x] >= 0) && (cho > 1))
					tmpline += tab(TRUE, trk->string - x, trk->c[j].a[x]);
			}

			if (cho > 1)
				s << tmpline;
			if (cho > 0) width++;                  // if tab is set
			if (((j + 1) == trksize) && (cho > 0)) // Last time?
				s << "\\sk\\en";
			else {
				if (cho > 0)  s << "\\en" << "\n";
				if ((cho > 0) && (width >= 26)){       // we need a LF in tab
					s << "\\endextract" << "\n";
					if (globalShowStr && (!flatnote))
						s << showstr;
					s << "\\startextract" << "\n";
					width = 0;
				}
			}
			cho = 0;
		}                        //end of (j)

		s << "\n";
		s << "\\endextract" << "\n";

		n++;
	}
	// End of TeX-File
	s << "\\end";

	f.close();
	return TRUE;
}

bool TabSong::save_to_tex_notes(QString fileName)
{
	return FALSE; //ALINXFIX: disabled

	QFile f(fileName);
    if (!f.open(IO_WriteOnly))
		return FALSE;

	QTextStream s(&f);

	QListIterator<TabTrack> it(t);

	// TeX-File INFO-HEADER
	s << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << "\n";
	s << "%" << "\n";
	s << "% This MusiXTex File was created with KGuitar " << VERSION << "\n";
    s << "% $Id$ " << "\n";
	s << "%" << "\n";
	s << "% You can download the latest version at:" << "\n";
	s << "%          http://kguitar.sourceforge.net" << "\n";
	s << "%" << "\n" << "%" << "\n";
	s << "% MusiXTeX is required to use this file." << "\n";
	s << "% This stuff you can download at:" << "\n" << "%" << "\n";
	s << "%       ftp.dante.de/tex-archive/macros/musixtex/taupin" << "\n";
	s << "%" << "\n";
	s << "%    or http://www.gmd.de/Misc/Music" << "\n";
	s << "%" << "\n" << "%" << "\n";
	s << "% IMPORTANT: Note that this file should not be used with LaTeX" << "\n";
	s << "%" << "\n";
	s << "% MusiXTeX runs as a three pass system. That means: for best ";
	s << "results" << "\n";
	s << "% you have to run: TeX => musixflx => TeX" << "\n" << "%" << "\n";
	s << "% For example:" << "\n";
	s << "%     $ tex foobar.tex " << "\n";
	s << "%     $ musixflx foobar.mx1" << "\n";
	s << "%     $ tex foobar.tex" << "\n";
	s << "%" << "\n";
	s << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << "\n";

	// TeX-File HEADER
	s << "\\input musixtex" << "\n" << "\n";

	// SONG HEADER
	if (!globalShowPageNumb)
		s << "\\nopagenumbers" << "\n";
	if (!globalShowBarNumb)
		s << "\\nobarnumbers" << "\n";
	s << "\n";

	// TRACK DATA
	int n = 1;       // Trackcounter

	for (; it.current(); ++it) { // For every track
		TabTrack *trk = it.current();
		s << "\\generalmeter{\\meterfrac{" << trk->b[0].time1;
		s << "}{" << trk->b[0].time2 << "}}";
		s << "\n" << "\n";
		s << "\\startpiece" << "\n";

		// make here the notes

		s << "\\endpiece" << "\n" << "\n";
		n++;                      // next Track
	}
	// End of TeX-File
	s << "\\end" << "\n";

	f.close();
	return TRUE;
}
