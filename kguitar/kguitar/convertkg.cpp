#include "convertkg.h"
#include "settings.h"

#include <kconfig.h>
#include <qfile.h>
#include <qdatastream.h>

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
// et='S' - new time signature: 2 or 3 bytes - time1:time2 + (optional) key

ConvertKg::ConvertKg(TabSong *song): ConvertBase(song) {}

bool ConvertKg::save(QString fileName)
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
	s << song->title;
	s << song->author;
	s << song->transcriber;
	s << song->comments;
	s << song->tempo;

	// TRACK DATA
	s << song->t.count();				// Number of tracks

	bool needfx = FALSE;				// Should we write FX event after tab?

	QListIterator<TabTrack> it(song->t);
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
		s << (Q_UINT8) 3;				// 3 byte event length
		s << (Q_UINT8) trk->b[0].time1; // Time signature itself
		s << (Q_UINT8) trk->b[0].time2;
		s << (Q_INT8) trk->b[0].keysig;

		for (uint x = 0; x < trk->c.size(); x++) {
			if (bar+1 < trk->b.size()) {	// This bar's not last
				if ((uint)trk->b[bar+1].start == x)
					bar++;				// Time for next bar
			}

			if ((bar < (uint)trk->b.size()) && ((uint)trk->b[bar].start == x)) {
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

bool ConvertKg::load(QString fileName)
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
	s >> song->title;
	s >> song->author;
	s >> song->transcriber;
	s >> song->comments;
	s >> song->tempo;

	if (song->tempo < 0) {
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

	song->t.clear();

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

		song->t.append(new TabTrack((TabTrack::TrackMode) tm, tn, channel, i16, patch, string, frets));

		kdDebug() << "Appended a track..." << endl;;

		for (int j = 0; j < string; j++) {
			s >> cn;
			song->t.current()->tune[j] = cn;
		}

		kdDebug() << "Read the tuning..." << endl;;

		bool finished = FALSE;

		int x = 0, bar = 1;
		TabTrack *ct = song->t.current();
// uchar tcsize=ct->string+2;
		ct->c.resize(1);
		ct->b.resize(1);
		ct->b[0].start = 0;
		ct->b[0].time1 = 4;
		ct->b[0].time2 = 4;

		kdDebug() << "reading events" << endl;;
		do {
			s >> event;
			s >> elength;

			switch (event) {
			case 'B':                   // Tab bar
				bar++;
				ct->b.resize(bar);
				ct->b[bar-1].start=x;
				ct->b[bar-1].time1=ct->b[bar-2].time1;
				ct->b[bar-1].time2=ct->b[bar-2].time2;
				ct->b[bar-1].keysig=ct->b[bar-2].keysig;
				break;
			case 'T':                   // Tab column
				x++;
				ct->c.resize(x);
				for (int k = 0; k < string; k++) {
					s >> cn;
					ct->c[x-1].a[k] = cn;
					ct->c[x-1].e[k] = 0;
				}
				s >> i16;
				ct->c[x-1].flags = 0;
				ct->c[x-1].setFullDuration(i16);
				break;
			case 'E':                   // Effects of prev column
				if (x == 0) {			// Ignore if there were no tab cols
					kdDebug() << "Warning: FX column with no tab columns, ignoring..." << endl;
					break;
				}
				for (int k = 0; k < string; k++) {
					s >> cn;
					ct->c[x-1].e[k] = cn;
				}
				break;
			case 'F':                   // Flag of prev column
				if (x == 0) {			// Ignore if there were no tab cols
					kdDebug() << "Warning: flag with no tab columns, ignoring..." << endl;
					break;
				}
				s >> cn; ct->c[x-1].flags = cn;
				break;
			case 'L':					// Continuation of previous column
				x++;
				ct->c.resize(x);
				for (int k = 0; k < string; k++)
					ct->c[x-1].a[k] = -1;
				s >> i16;
				ct->c[x-1].flags = FLAG_ARC;
				ct->c[x-1].setFullDuration(i16);
				break;
			case 'S':                   // New time signature
				s >> cn; ct->b[bar-1].time1 = cn;
				s >> cn; ct->b[bar-1].time2 = cn;
				if (elength == 3) {
					s >> cn; ct->b[bar-1].keysig = cn;
				}
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

		ct->x = 0;
		ct->xb = 0;
		ct->y = 0;
	}

	f.close();

	return TRUE;
}
