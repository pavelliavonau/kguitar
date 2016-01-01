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
	if (!f.open(QIODevice::WriteOnly))
		return FALSE;

	QDataStream s(&f);

	// HEADER SIGNATURE
	s.writeRawData("KG\0", 3);

	// VERSION SIGNATURE
	s << (quint8) 2;

	// HEADER SONG DATA
	s << song->info["TITLE"];
	s << song->info["ARTIST"];
	s << song->info["TRANSCRIBER"];
	s << song->info["COMMENTS"];
	s << song->tempo;

	// TRACK DATA
	s << song->rowCount();				// Number of tracks

	bool needfx = FALSE;				// Should we write FX event after tab?

	// For every track
	//foreach (TabTrack *trk, song->t) {
	for(int row = 0; row < song->rowCount(); row++) {
		auto trk = song->index(row, 0).data(TabSong::TrackPtrRole).value<TabTrack*>();
		s << (quint8) trk->trackMode();// Track properties
		s << trk->name;
		s << (quint8) trk->channel;
		s << (quint16) trk->bank;
		s << (quint8) trk->patch;
		s << (quint8) trk->string;
		s << (quint8) trk->frets;
		for (int i = 0; i<trk->string; i++)
			s << (quint8) trk->tune[i];

		// TRACK EVENTS

		quint8 tcsize = trk->string+2;
		uint bar = 1;

		s << (quint8) 'S';				// Time signature event
		s << (quint8) 3;				// 3 byte event length
		s << (quint8) trk->bars()[0].time1; // Time signature itself
		s << (quint8) trk->bars()[0].time2;
		s << (qint8) trk->bars()[0].keysig;

		for (uint x = 0; x < trk->c.size(); x++) {
			if (bar+1 < trk->bars().size()) {	// This bar's not last
				if ((uint)trk->bars()[bar+1].start == x)
					bar++;				// Time for next bar
			}

			if ((bar < (uint)trk->bars().size()) && ((uint)trk->bars()[bar].start == x)) {
				s << (quint8) 'B';     // New bar event
				s << (quint8) 0;
				if ((trk->bars()[bar].time1 != trk->bars()[bar - 1].time1) ||
					(trk->bars()[bar].time2 != trk->bars()[bar - 1].time2)) {
					s << (quint8) 'S'; // New signature
					s << (quint8) 2;
					s << (quint8) trk->bars()[bar].time1;
					s << (quint8) trk->bars()[bar].time2;
				}
			}

			if (trk->c[x].flags & FLAG_ARC) {
				s << (quint8) 'L';		// Continue of previous event
				s << (quint8) 2;		// Size of event
				s << trk->c[x].fullDuration(); // Duration
			} else {
				s << (quint8) 'T';		// Tab column events
				s << (quint8) tcsize;	// Size of event
				needfx = FALSE;
				for (int i = 0;i < trk->string; i++) {
					s << (qint8) trk->c[x].a[i];
					if (trk->c[x].e[i])
						needfx = TRUE;
				}
				s << trk->c[x].fullDuration(); // Duration
				if (needfx) {
					s << (quint8) 'E'; // Effect event
					s << (quint8) trk->string; // Size of event
					for (int i = 0; i < trk->string; i++)
						s << (quint8) trk->c[x].e[i];
				}
				if (trk->c[x].effectFlags()) {
					s << (quint8) 'F'; // Flag event
					s << (quint8) 1;   // Size of event
					s << (quint8) trk->c[x].effectFlags();
				}
			}
		}

		s << (quint8) 'X';				// End of track marker
		s << (quint8) 0;				// Length of end track event
	}

	f.close();

	return TRUE;
}

bool ConvertKg::load(QString fileName)
{
	QFile f(fileName);
	if (!f.open(QIODevice::ReadOnly))
		return FALSE;

	QDataStream s(&f);

	// HEADER SIGNATURE
	char hdr[4];
	s.readRawData(hdr, 3); // should be KG\0 header
	if (!((hdr[0] == 'K') && (hdr[1] == 'G') && (hdr[2] == 0)))
		return FALSE;

	// FILE VERSION NUMBER
	quint8 ver;
	s >> ver; // version 2 files are unicode KDE2 files
	if ((ver < 1) || (ver > 2))
		return FALSE;

	// HEADER SONG DATA
	s >> song->info["TITLE"];
	s >> song->info["ARTIST"];
	s >> song->info["TRANSCRIBER"];
	s >> song->info["COMMENTS"];
	s >> song->tempo;

	if (song->tempo < 0) {
		kDebug() << "Bad tempo" << endl;
		return FALSE;
	}

	kDebug() << "Read headers..." << endl;

	// TRACK DATA
	int cnt;
	s >> cnt; // Track count

	if (cnt <= 0) {
		kDebug() << "Bad track count" << endl;
		return FALSE;
	}

	song->removeRows(0, song->rowCount());

	kDebug() << "Going to read " << cnt << " track(s)..." << endl;

	quint16 i16;
	quint8 channel, patch, string, frets, tm, event, elength;
	qint8 cn;
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

		kDebug() << "Read a track of " << string << " strings," << endl;
		kDebug() << "       bank = " << i16 << ", patch = " << patch << " ..." << endl;

		TabTrack *trk = new TabTrack((TabTrack::TrackMode) tm, tn, channel, i16, patch, string, frets);
		int count = song->rowCount();
		song->insertRow(count);
		auto index = song->index(count, 0);
		song->setData(index, QVariant::fromValue(trk), TabSong::TrackPtrRole);

		kDebug() << "Appended a track..." << endl;;

		for (int j = 0; j < string; j++) {
			s >> cn;
			trk->tune[j] = cn;
		}

		kDebug() << "Read the tuning..." << endl;;

		bool finished = FALSE;

		int x = 0, bar = 1;
		TabTrack *ct = trk;
// uchar tcsize=ct->string+2;
		ct->c.resize(1);
		ct->bars().resize(1);
		ct->bars()[0].start = 0;
		ct->bars()[0].time1 = 4;
		ct->bars()[0].time2 = 4;

		kDebug() << "reading events" << endl;;
		do {
			s >> event;
			s >> elength;

			switch (event) {
			case 'B':                   // Tab bar
				bar++;
				ct->bars().resize(bar);
				ct->bars()[bar-1].start=x;
				ct->bars()[bar-1].time1=ct->bars()[bar-2].time1;
				ct->bars()[bar-1].time2=ct->bars()[bar-2].time2;
				ct->bars()[bar-1].keysig=ct->bars()[bar-2].keysig;
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
					kDebug() << "Warning: FX column with no tab columns, ignoring..." << endl;
					break;
				}
				for (int k = 0; k < string; k++) {
					s >> cn;
					ct->c[x-1].e[k] = cn;
				}
				break;
			case 'F':                   // Flag of prev column
				if (x == 0) {			// Ignore if there were no tab cols
					kDebug() << "Warning: flag with no tab columns, ignoring..." << endl;
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
				s >> cn; ct->bars()[bar-1].time1 = cn;
				s >> cn; ct->bars()[bar-1].time2 = cn;
				if (elength == 3) {
					s >> cn; ct->bars()[bar-1].keysig = cn;
				}
				break;
			case 'X':					// End of track
				finished = TRUE;
				break;
			default:
				kDebug() << "Warning: unknown event " << event << " Skipping..." << endl;
				for (int k = 0; k < elength; k++)
					s >> cn;
				break;
			}
		} while ((!finished) && (!s.atEnd()));

		ct->x = 0;
		ct->xb = 0;
		ct->y = 0;
	}

	f.close();

	return TRUE;
}
