#include "tabsong.h"
#include "application.h"

#include "musicxml.h"

#include <qxml.h>
#include <qfile.h>
#include <qdatastream.h>

#ifdef WITH_TSE3
#include <tse3/Track.h>
#include <tse3/Part.h>
#include <tse3/MidiFile.h>
#include <tse3/TSE3MDL.h>
#include <string>
#endif

//GREYFIX
#include <iostream.h>

TabSong::TabSong(QString _title, int _tempo)
{
	tempo = _tempo;
	title = _title;
	t.setAutoDelete(TRUE);
}

// Helper functions for duration conversion

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
				t.current()->c[x-1].flags = 0;
				t.current()->c[x-1].setFullDuration(i16);
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
				t.current()->c[x-1].flags = FLAG_ARC;
				t.current()->c[x-1].setFullDuration(i16);
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
	s.writeRawBytes("KG\0", 3);

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
				if ((trk->b[bar].time1 != trk->b[bar - 1].time1) ||
					(trk->b[bar].time2 != trk->b[bar - 1].time2)) {
					s << (Q_UINT8) 'S'; // New signature
					s << (Q_UINT8) 2;
					s << (Q_UINT8) trk->b[bar].time1;
					s << (Q_UINT8) trk->b[bar].time2;
				}
			}

			if (trk->c[x].flags & FLAG_ARC) {
				s << (Q_UINT8) 'L';		// Continue of previous event
				s << (Q_UINT8) 2;		// Size of event
				s << trk->c[x].fullDuration(); // Duration
			} else {
				s << (Q_UINT8) 'T';		// Tab column events
				s << (Q_UINT8) tcsize;	// Size of event
				needfx = FALSE;
				for (int i = 0;i < trk->string; i++) {
					s << (Q_INT8) trk->c[x].a[i];
					if (trk->c[x].e[i])
						needfx = TRUE;
				}
				s << trk->c[x].fullDuration(); // Duration
				if (needfx) {
					s << (Q_UINT8) 'E'; // Effect event
					s << (Q_UINT8) trk->string; // Size of event
					for (int i = 0; i < trk->string; i++)
						s << (Q_UINT8) trk->c[x].e[i];
				}
				if (trk->c[x].effectFlags()) {
					s << (Q_UINT8) 'F'; // Flag event
					s << (Q_UINT8) 1;   // Size of event
					s << (Q_UINT8) trk->c[x].effectFlags();
				}
			}
		}

		s << (Q_UINT8) 'X';				// End of track marker
		s << (Q_UINT8) 0;				// Length of end track event
	}

	f.close();

	return TRUE;
}

// Reads Delphi string in GPro format. Delphi string looks pretty much like:
// <max-length> <real-length> <real-length * bytes - string data>

void TabSong::readDelphiString(QDataStream *s, char *c)
{
	Q_UINT8 maxl, l;
	(*s) >> maxl;
	(*s) >> l;
	if (maxl == 0)
		maxl = l;
	else
		maxl--;
	s->readRawBytes(c, maxl);
	c[l] = 0;
}

int TabSong::readDelphiInteger(QDataStream *s)
{
	Q_UINT8 x;
	int r;
	(*s) >> x; r = x;
	(*s) >> x; r += x << 8;
	(*s) >> x; r += x << 16;
	(*s) >> x; r += x << 24;
	return r;
}

bool TabSong::load_from_gtp(QString fileName)
{
	QFile f(fileName);
	if (!f.open(IO_ReadOnly))
		return FALSE;

	QDataStream s(&f);

	Q_UINT8 num, num2, fx;
	char c[300];

	// SIGNATURE CHECK

	s >> num;
	s.readRawBytes(c, num);
	if (strncmp(c, "FICHIER GUITAR", 14) != 0)
		return FALSE;

	// SEVERAL UNKNOWN BYTES

	num2 = 30 - num;
	s.readRawBytes(c, num2);

	// SONG ATTRIBUTES

	readDelphiString(&s, c);                   // Song title
	title = QString::fromLocal8Bit(c);

	readDelphiString(&s, c);                   // Author
	author = QString::fromLocal8Bit(c);

	readDelphiString(&s, c);                   // Comments
	comments = QString::fromLocal8Bit(c);

	transcriber = QString("KGuitar");

	tempo = readDelphiInteger(&s);             // Tempo

	readDelphiInteger(&s);                     // GREYFIX - Triplet feel
	readDelphiInteger(&s);                     // Unknown 4 bytes

	// TUNING INFORMATION

	t.clear();

	for (int i = 0; i < 8; i++) {
		num = readDelphiInteger(&s);           // Number of strings
		t.append(new TabTrack(FretTab, 0, 0, 0, 0, num, 24));
		for (int j = 0; j < num; j++)
			t.current()->tune[j] = readDelphiInteger(&s);
	}

	int maxbar = readDelphiInteger(&s);        // Quantity of bars

	// TRACK ATTRIBUTES

	QListIterator<TabTrack> it(t);
	for (; it.current(); ++it) {
		TabTrack *trk = it.current();
		trk->patch = readDelphiInteger(&s);    // MIDI Patch
		trk->frets = readDelphiInteger(&s);    // Frets
		readDelphiString(&s, c);               // Track name
		trk->name = QString::fromLocal8Bit(c);
		s >> num;                              // Flags - GREYFIX!
		readDelphiInteger(&s);                 // Slider: Volume (0-0x7F) - GREYFIX!
		readDelphiInteger(&s);                 // Slider: Pan
		readDelphiInteger(&s);                 // Slider: Chorus
		readDelphiInteger(&s);                 // Slider: Reverb
		readDelphiInteger(&s);                 // Capo
	};

	s.readRawBytes(c, 10);                     // 10 unknown bytes

// GREYFIX
#define kdDebug()  cout

	//	for (
	it.toFirst();// it.current(); ++it) {
		TabTrack *trk = it.current();

		trk->b.resize(maxbar);
		trk->c.resize(0);

		int x = 0;

		for (int xb = 0; xb < maxbar; xb++) {
			s >> num; trk->b[xb].time1 = num;  // Time signature
			s >> num; trk->b[xb].time2 = num;
			trk->b[xb].start = x;

			kdDebug() << "==================== NEW BAR: " <<
				(int) trk->b[xb].time1 << ":" << (int) trk->b[xb].time2 << ", from " << x << endl;

			s.readRawBytes(c, 8);              // 8 unknown bytes

			int thisbar = readDelphiInteger(&s);// Number of tab columns
			trk->c.resize(trk->c.size() + thisbar);

			s.readRawBytes(c, 93);             // 93 unknown bytes

			for (int j = 0; j < thisbar; x++, j++) {

				if (s.atEnd())
					break;

				// Guitar Pro uses overmaxed system for MIDI event
				// timing. Thus, it sets 480 ticks to be equal to 1/4 of a
				// whole note. KGuitar has 120 ticks as 1/4, so we need to
				// divide duration by 4
				
				trk->c[x].flags = 0;
				trk->c[x].setFullDuration(readDelphiInteger(&s) / 4);

				s >> num;                      // 1 unknown byte
				s >> num;                      // type mask
// 				s >> num2;                     // 1 unknown byte
				
				kdDebug() << "Read column " << x << ": duration " << trk->c[x].l << " (type mask=" << (int) num << ")" << endl;
				
				// Here comes "type mask" of a column.
				
				switch (num) {
				case 0:
 					s >> num2;                      // 1 unknown byte
					break;
				case 2:
					readDelphiString(&s, c);
					kdDebug() << "Chord name [" << c << "]" << endl;
					printf("%d byte:\n", f.at());
					s >> num;                      // unknown byte - always 1?
					if (num != 1)
						printf("Byte after chord name is not 1!\n");
					s >> num;                      // chord diagram tonic (0-11) - if 12 then no diagram
					kdDebug() << "Got tonic " << (int) num << " after 1 after chord name ";
					if (num != 0xc) {
						kdDebug() << "=> trying to skip the diagram" << endl;
						s.readRawBytes(c, 63);     // unknown bytes - diagram data
					} else {
						kdDebug() << "=> assuming no chord diagram" << endl;
						s.readRawBytes(c, 7);      // unknown bytes - C0 + something - 8 bytes
					}
					break;
				case 8:
					kdDebug() << "Rest 8" << endl;
					for (int i = 0; i < MAX_STRINGS; i++) {
						trk->c[x].a[i] = -1;
						trk->c[x].e[i] = 0;
					}
  					s >> num;
					continue;
				case 16:
					kdDebug() << "(dotted note) ";
 					s >> num;                       // 1 unknown byte
					break;
				case 32:
					kdDebug() << "(triplet note) ";
					s >> num;
					s >> num;
					break;
				case 64:
					kdDebug() << "(linked beat) ";
					for (int i = 0; i < MAX_STRINGS; i++) {
						trk->c[x].a[i] = -1;
						trk->c[x].e[i] = 0;
					}
					trk->c[x].flags |= FLAG_ARC;
					s >> num;                       // 1 unknown byte
					continue;
				default:
					kdDebug() << "(unknown typemask) ";
					s >> num;
					break;
				}

				s >> num2;                     // mask of following fret numbers
				s >> fx;                       // effects
				s >> num;                      // 1 unknown byte

				kdDebug() << "Frets using mask " << (int) num2 << ", fx " << (int) fx << ": ";

				for (int i = 5; i >= 0; i--) {
					trk->c[x].e[i] = 0;
					if (num2 & (1 << i)) {
						s >> num;              // fret number
						trk->c[x].a[i] = (num == 100) ? DEAD_NOTE : num; // 100 = GTP's dead note
						kdDebug() << (int) num;
						s >> num;              // volume>? - GREYFIX
						if (fx & (1 << i)) {
							s >> num;
							switch (num) {
							case 1: kdDebug() << "h"; trk->c[x].e[i] = EFFECT_LEGATO; break;
							case 2: kdDebug() << "p"; trk->c[x].e[i] = EFFECT_LEGATO; break;
							case 3: kdDebug() << "su"; break;
							case 4: kdDebug() << "sd"; break;
							}
						}
					} else {
						trk->c[x].a[i] = -1;
						kdDebug() << "X";
					}
				}
				
				kdDebug() << endl;
			}
		}
		//    };

	printf("%d byte:\n", f.at());
	num2 = 0;
	while (!s.atEnd()) {
		s >> num;
		printf("%02x ", num);
		num2++;
		if (num2 == 8)
			printf(" ");
		if (num2 == 16) {
			printf("\n");
			num2 = 0;
		}
	}
	printf("\n");

	f.close();

    return TRUE;
}

bool TabSong::save_to_gtp(QString fileName)
{
    // Saving to Guitar Pro format here
    return FALSE;
}

bool TabSong::load_from_xml(QString fileName)
{
	MusicXMLParser handler(this);
	QFile xmlFile(fileName);
	QXmlInputSource source(xmlFile);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.parse(source);

    return TRUE;
}

bool TabSong::save_to_xml(QString fileName)
{
    QFile f(fileName);
    if (!f.open(IO_WriteOnly))
		return FALSE;

    QTextStream s(&f);

	MusicXMLWriter handler(this);
	handler.write(s);

    f.close();
    return TRUE;
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

#ifdef WITH_TSE3
bool TabSong::save_to_mid(QString filename)
{
	TSE3::MidiFileExport exp;
	exp.save((const char *) filename.local8Bit(), midiSong());
	// GREYFIX: pretty ugly unicode string to standard string hack
	return TRUE;
}

bool TabSong::save_to_tse3(QString filename)
{
	TSE3::TSE3MDL mdl("KGuitar", 2);
	mdl.save((const char *) filename.local8Bit(), midiSong());
	// GREYFIX: pretty ugly unicode string to standard string hack
	return TRUE;
}
#endif

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

#ifdef WITH_TSE3
// Assembles the whole TSE song from various tracks, generated with
// corresponding midiTrack() calls.
TSE3::Song *TabSong::midiSong()
{
	TSE3::Song *song = new TSE3::Song(0);

	QListIterator<TabTrack> it(t);
	for (; it.current(); ++it) {
		TSE3::Track *trk = new TSE3::Track();
		TSE3::PhraseEdit *trackData = it.current()->midiTrack();
		TSE3::Phrase *phrase = trackData->createPhrase(song->phraseList());
		TSE3::Part *part = new TSE3::Part(0, trackData->lastClock());
		part->setPhrase(phrase);
		trk->insert(part);
		song->insert(trk);
		delete trackData;
	}

	return song;
}
#endif