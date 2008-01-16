#include "trackdrag.h"
#include "tabtrack.h"

#include <kdebug.h>

#include <qdragobject.h>
#include <qbuffer.h>
//#include <qdatastream.h>


TrackDrag::TrackDrag(TabTrack *trk, QWidget *dragSource, const char *name) :
	QStoredDrag("application/x-kguitar-snippet", dragSource, name)
{
	setTrack(trk);
}


TrackDrag::TrackDrag(QWidget *dragSource, const char *name) :
	QStoredDrag("application/x-kguitar-snippet", dragSource, name)
{
}

TrackDrag::~TrackDrag()
{
}

void TrackDrag::setTrack(TabTrack *trk)
{
	if (trk == NULL) {
		kdDebug() << "TrackDrag::setTrack() >>>>>> trk == NULL" << endl;
		return;  // ALINXFIX: Write in buffer "NULLTRACK"
	}

	// Save to buffer
	QBuffer buffer;
	buffer.open(IO_WriteOnly);

	QDataStream s(&buffer);

	//ALINXFIX: Move this stuff to share it with TabSong::save_to_kg
	//          this stuff is the same as "save_to_kg"
	//************Start***********
	bool needfx = FALSE;				// Should we write FX event after tab?
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
			if (trk->c[x].flags) {
				s << (Q_UINT8) 'F'; // Flag event
				s << (Q_UINT8) 1;   // Size of event
				s << (Q_UINT8) trk->c[x].flags;
			}
		}
	}

	s << (Q_UINT8) 'X';				// End of track marker
	s << (Q_UINT8) 0;				// Length of end track event

	buffer.close();

	setEncodedData(buffer.buffer());
}

bool TrackDrag::canDecode(const QMimeSource *e)
{
	return e->provides("application/x-kguitar-snippet");
}

bool TrackDrag::decode(const QMimeSource *e, TabTrack *&trk)
{
	trk = NULL;

	if (!canDecode(e)) {
		kdDebug() << "TrackDrag::decode(...) >> can't decode QMimeSource!!" << endl;
		return FALSE;
	}

	QByteArray b = e->encodedData("application/x-kguitar-snippet");

	if (!b.size()) //No data
		return FALSE;

	QBuffer buffer(b);
	buffer.open(IO_ReadOnly);

	QDataStream s(&buffer);

	//ALINXFIX: Move this stuff to share it with TabSong::save_to_kg
	//          this stuff is the same as "save_to_kg"
	//************Start***********
	Q_UINT16 i16;
	Q_UINT8 channel, patch, string, frets, tm, event, elength;
	Q_INT8 cn;
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
	newtrk->b.resize(1);
	newtrk->b[0].start = 0;
	newtrk->b[0].time1 = 4;
	newtrk->b[0].time2 = 4;

	kdDebug() << "TrackDrag::decode >> reading events" << endl;;
	do {
		s >> event;
		s >> elength;

		switch (event) {
		case 'B':                   // Tab bar
			bar++;
			newtrk->b.resize(bar);
			newtrk->b[bar-1].start=x;
			newtrk->b[bar-1].time1=newtrk->b[bar-2].time1;
			newtrk->b[bar-1].time2=newtrk->b[bar-2].time2;
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
				kdDebug() << "TrackDrag::decode >> Warning: FX column with no tab columns, ignoring..." << endl;
				break;
			}
			for (int k = 0; k < string; k++) {
				s >> cn;
				newtrk->c[x-1].e[k] = cn;
			}
			break;
		case 'F':                   // Flag of prev column
			if (x == 0) {			// Ignore if there were no tab cols
				kdDebug() << "TrackDrag::decode >> Warning: flag with no tab columns, ignoring..." << endl;
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
			s >> cn; newtrk->b[bar-1].time1 = cn;
			s >> cn; newtrk->b[bar-1].time2 = cn;
			break;
		case 'X':					// End of track
			finished = TRUE;
			break;
		default:
			kdDebug() << "TrackDrag::decode >> Warning: unknown event " << event << " Skipping..." << endl;
			for (int k = 0; k < elength; k++)
				s >> cn;
			break;
		}
	} while ((!finished) && (!s.eof()));

	newtrk->x = 0;
	newtrk->xb = 0;
	newtrk->y = 0;


	//************End***********

	buffer.close();
	trk = newtrk;
	return TRUE;
}
