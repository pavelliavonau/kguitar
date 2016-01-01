#include "trackdrag.h"
#include "data/tabtrack.h"

#include <kdebug.h>

#include <QBuffer>
#include <QMimeData>


QString TrackDrag::TRACK_MIME_TYPE = "application/x-kguitar-snippet";

QByteArray TrackDrag::encode(TabTrack *trk)
{
	if (trk == NULL) {
		kDebug() << "TrackDrag::setTrack() >>>>>> trk == NULL" << endl;
		return QByteArray();  // ALINXFIX: Write in buffer "NULLTRACK"
	}

	// Save to buffer
	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly);

	QDataStream s(&buffer);

	//ALINXFIX: Move this stuff to share it with TabSong::save_to_kg
	//          this stuff is the same as "save_to_kg"
	//************Start***********
	bool needfx = FALSE;				// Should we write FX event after tab?
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
	s << (quint8) 2;				// 2 byte event length
	s << (quint8) trk->bars()[0].time1; // Time signature itself
	s << (quint8) trk->bars()[0].time2;

	for (int x = 0; x < trk->c.size(); x++) {
		if (bar+1 < (uint)trk->bars().size()) {	// This bar's not last
			if (trk->bars()[bar+1].start == x)
				bar++;				// Time for next bar
		}

		if ((bar < (uint)trk->bars().size()) && (trk->bars()[bar].start == x)) {
			s << (quint8) 'B';     // New bar event
			s << (quint8) 0;
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
			if (trk->c[x].flags) {
				s << (quint8) 'F'; // Flag event
				s << (quint8) 1;   // Size of event
				s << (quint8) trk->c[x].flags;
			}
		}
	}

	s << (quint8) 'X';				// End of track marker
	s << (quint8) 0;				// Length of end track event

	buffer.close();

	return buffer.buffer();
}

bool TrackDrag::canDecode(const QMimeData *e)
{
	return e->hasFormat(TRACK_MIME_TYPE);
}

bool TrackDrag::decode(const QMimeData *e, TabTrack *&trk)
{
	trk = NULL;

	if (!canDecode(e)) {
		kDebug() << "TrackDrag::decode(...) >> can't decode QMimeSource!!" << endl;
		return FALSE;
	}

	QByteArray b = e->data(TRACK_MIME_TYPE);

	if (!b.size()) //No data
		return FALSE;

	QBuffer buffer(&b);
	buffer.open(QIODevice::ReadOnly);

	QDataStream s(&buffer);

	//ALINXFIX: Move this stuff to share it with TabSong::save_to_kg
	//          this stuff is the same as "save_to_kg"
	//************Start***********
	quint16 i16;
	quint8 channel, patch, string, frets, tm, event, elength;
	qint8 cn;
	QString tn;

	s >> tm; // Track properties (Track mode)
	s >> tn; // Track name
	s >> channel;
	s >> i16; // Bank
	s >> patch;
	s >> string;
	s >> frets;

	if (string > MAX_STRINGS)
		return FALSE;

	TabTrack *newtrk = new TabTrack((TabTrack::TrackMode) tm,tn,channel,i16,patch,string,frets);

	for (int j = 0; j < string; j++) {
		s >> cn;
		newtrk->tune[j] = cn;
	}

	bool finished = FALSE;

	int x = 0, bar = 1;
	// uchar tcsize=newtrk->string+2;
	newtrk->c.resize(1);
	newtrk->bars().resize(1);
	newtrk->bars()[0].start = 0;
	newtrk->bars()[0].time1 = 4;
	newtrk->bars()[0].time2 = 4;

	kDebug() << "TrackDrag::decode >> reading events" << endl;;
	do {
		s >> event;
		s >> elength;

		switch (event) {
		case 'B':                   // Tab bar
			bar++;
			newtrk->bars().resize(bar);
			newtrk->bars()[bar-1].start=x;
			newtrk->bars()[bar-1].time1=newtrk->bars()[bar-2].time1;
			newtrk->bars()[bar-1].time2=newtrk->bars()[bar-2].time2;
			break;
		case 'T':                   // Tab column
			x++;
			newtrk->c.resize(x);
			for (int k = 0; k < string; k++) {
				s >> cn;
				newtrk->c[x-1].a[k] = cn;
				newtrk->c[x-1].e[k] = 0;
			}
			s >> i16;
			newtrk->c[x-1].flags = 0;
			newtrk->c[x-1].setFullDuration(i16);
			break;
		case 'E':                   // Effects of prev column
			if (x == 0) {			// Ignore if there were no tab cols
				kDebug() << "TrackDrag::decode >> Warning: FX column with no tab columns, ignoring..." << endl;
				break;
			}
			for (int k = 0; k < string; k++) {
				s >> cn;
				newtrk->c[x-1].e[k] = cn;
			}
			break;
		case 'F':                   // Flag of prev column
			if (x == 0) {			// Ignore if there were no tab cols
				kDebug() << "TrackDrag::decode >> Warning: flag with no tab columns, ignoring..." << endl;
				break;
			}
			s >> cn; newtrk->c[x-1].flags = cn;
			break;
		case 'L':					// Continuation of previous column
			x++;
			newtrk->c.resize(x);
			for (int k = 0; k < string; k++)
				newtrk->c[x-1].a[k] = -1;
			s >> i16;
			newtrk->c[x-1].flags = FLAG_ARC;
			newtrk->c[x-1].setFullDuration(i16);
			break;
		case 'S':                   // New time signature
			s >> cn; newtrk->bars()[bar-1].time1 = cn;
			s >> cn; newtrk->bars()[bar-1].time2 = cn;
			break;
		case 'X':					// End of track
			finished = TRUE;
			break;
		default:
			kDebug() << "TrackDrag::decode >> Warning: unknown event " << event << " Skipping..." << endl;
			for (int k = 0; k < elength; k++)
				s >> cn;
			break;
		}
	} while ((!finished) && (!s.atEnd()));

	newtrk->x = 0;
	newtrk->xb = 0;
	newtrk->y = 0;


	//************End***********

	buffer.close();
	trk = newtrk;
	return TRUE;
}
