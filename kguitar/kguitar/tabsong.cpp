#include "tabsong.h"
#include "settings.h"

#include "musicxml.h"

#include <qxml.h>
#include <qfile.h>
#include <qdatastream.h>
#include <kconfig.h>

#ifdef WITH_TSE3
#include <tse3/Track.h>
#include <tse3/Part.h>
#include <tse3/TempoTrack.h>
#include <string>
#endif

TabSong::TabSong(QString _title, int _tempo)
{
	tempo = _tempo;
	title = _title;
	t.setAutoDelete(TRUE);
}

// Helper functions for duration conversion

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

uint TabSong::maxLen()
{
	uint res = 0;

	QListIterator<TabTrack> it(t);
	for (; it.current(); ++it)
		res = it.current()->b.size() > res ? it.current()->b.size() : res;

	return res;
}

void TabSong::arrangeBars()
{
	QListIterator<TabTrack> it(t);
	for (; it.current(); ++it) {		// For every track
		TabTrack *trk = it.current();
		trk->arrangeBars();
	}
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

bool TabSong::loadFromGtp(QString fileName)
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
		t.append(new TabTrack(TabTrack::FretTab, 0, 0, 0, 0, num, 24));
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

bool TabSong::saveToGtp(QString)
{
    // Saving to Guitar Pro format here
    return FALSE;
}

/* This dirty piece of code is made by Sylvain "Sly" Vignaud
 * Contact him for infos at:
 * vignsyl@iit.edu
 */

#define SLY_LOGS
//#define VERBOSE_NOTES

#ifdef SLY_LOGS
	#define LOGS	{printf(message);fprintf(logs,message);fflush(stdout);fflush(logs);}
	#define WHEREAMI(str)	{sprintf(message,"\t%s\t: Offset %ld = %d %d * %d * %d %d %d\n", str,totalsize-size, buf[-2],buf[-1],buf[0],buf[1],buf[2],buf[3]);LOGS}
#endif

#define GET_STR(name_for_info)	{		\
			unsigned long max;	\
			GET_LONG( max );	\
			unsigned char nb = *buf;\
			DUMMIES( 1 );		\
			if (max==0) max = nb;	\
			else max--;		\
			strncpy(str,(char*)buf,nb);	\
			str[nb] = 0;		\
			DUMMIES( max );		\
		}

#define GET_STR_NO_INCBUF	{		\
			int nb = *buf;		\
			strncpy(str,(char*)buf+1,nb);	\
			str[nb] = 0;		\
			}

#define GET_LONG(i)	{i = get_long(buf); DUMMIES( 4 );}
#define GET_CHAR(i)	i = *buf++, size--;
#define DUMMIES(i)	buf += i, size -= i

#define IS_CHAR(c)	(	( (c)>='a' && (c)<='z' ) || ( (c)>='A' && (c)<='Z' )	)
#define VALID_COMMAND	( (buf[0]==0 && buf[2]!=0) || buf[0]!=1 || buf[0]==32 || buf[0]!=34 || buf[0]==40 || buf[0]==64)

inline long get_long( unsigned char *buf) {
	long res = buf[0] | buf[1]<<8 | buf[2]<<16 | buf[3]<<24;
	return res;
}

inline int get_string(unsigned char *buf) {
	int strings = buf[0];
	int string = 8;
	while (strings) {
		strings >>= 1;
		string--;
	}
	return string;	//1-6
}

bool TabSong::loadFromGp3(QString fileName)
{
	unsigned char *buffer;
	unsigned char *buf;
	long size, totalsize;
	char str[10240];	//For temporary string storage while converting Delphi strings -> QStrings
	#ifdef SLY_LOGS
	FILE *logs;
	char message[10240];	(void)message;
	#endif

	printf("\nGP3 loader by Sylvain Vignaud - please email bugs at vignsyl@iit.edu\n");fflush(stdout);

	#ifdef SLY_LOGS
	logs = fopen("logs.txt","wt");
	if (!logs) {printf("Cannot create log file\n");fflush(stdout);return FALSE;}
	#endif

	//Open file
	QFile gp3file(fileName);
	if (!gp3file.open(IO_ReadOnly)) {
		#ifdef SLY_LOGS
		fclose( logs );
		#endif
		printf("Cannot open file\n");fflush(stdout);
		return FALSE;
	}

	//Read whole file in a buffer
	totalsize = size = gp3file.size();
	buffer = (unsigned char*)malloc(size);
	QDataStream fstream(&gp3file);
	fstream.readRawBytes( (char*)buffer,size );
	gp3file.close();

	//Check file signature
	#define GTP_ID3	"FICHIER GUITAR PRO"

	buf = buffer;
	if ( strncmp((char*)buf+1,GTP_ID3,strlen(GTP_ID3)) ) {
		printf("Not a Guitar Pro 3 file!\n[%s] != [%s]\n",GTP_ID3,(char*)buf+1);fflush(stdout);
		free(buffer);
		return FALSE;	//Not a GP3 file
	}

	//Now it's time to parse the whole thing

	//First the header: general infos
	DUMMIES( 31 );
	GET_STR(title);		title = QString::fromLocal8Bit(str);
	GET_STR(subtitle);
	GET_STR(artist);	author = QString::fromLocal8Bit(str);
	GET_STR(album);
	GET_STR(author);
	GET_STR(copyright);
	GET_STR(tabled_by);	transcriber = QString::fromLocal8Bit(str);
	GET_STR(instruction);
	char variente = *buf==1;	//FIXME What's the real meaning of this byte?
	if (variente)
		DUMMIES( 4 );
	GET_STR(notice);	comments = QString::fromLocal8Bit(str);
	if (variente)
		DUMMIES( 1 );
	tempo = *buf;

	//Find beginning of tracks
	DUMMIES( 780 );
	while (*buf<' ') DUMMIES(1);	//Find 1st character of the first track name
	DUMMIES(-1);			//Get back on the "lenght" bytes

	//Read track infos: name
	t.clear();
	int nbtrack = 0;
	int nbstring = 6;	//Shoud be read somewhere for each track

	do {
		GET_STR_NO_INCBUF;
		printf("Track %s\n",str);fflush(stdout);
		t.append( new TabTrack(TabTrack::FretTab, 0, 0, 0, 0, 6, 24) );	//First 24: 24 frets, cos don't know the exact number
		t.current()->name = QString::fromLocal8Bit(str);
		t.current()->b.resize(0);
		t.current()->c.resize(0);
		nbtrack++;
		DUMMIES( 98 );	//There are 98 bytes of description per track. Title (fixed max lenght)+ ???
	} while (IS_CHAR(buf[1]) && IS_CHAR(buf[2]));
	printf("%d track(s)\n\n", nbtrack);fflush(stdout);
	QListIterator<TabTrack> tracks(t);

/* Format GP3 : Bar1(Track1,Track2,...,TrackN) , Bar2(Track1,Track2,...,TrackN) , ... , BarN(Track1,Track2,...,TrackN)
A bar is:
	long nb notes in this bar
	Then:
		0 ? fetchs : positions utilisees. Nb fetchs (bits=1) = nb notes next
		1 ? fetchs : positions utilisees. Next notes = O       Nb fetchs (bits=1) = nb notes next
		Then:
			32 ? fetch = black o|
			34 ? fetch = black o
			40 ? fetch ? = |
			64 ? ? ? = rest

fetch = :
e|64
B|32
G|16
D|8
A|4
E|2
*/

	//Parse the tracks
	DUMMIES( -1 );
	unsigned long lgrbar;
	unsigned long tempstotal[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned long time[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned long numbar = 0;
	long size_tracks = size;
	char error = 0;

	do {
//		printf("New bar lgr %ld | time[numtrack] = %ld / %ld | buf = %d %d\n",lgrbar,time[numtrack],tempstotal,buf[0],buf[1]);
		#ifndef NOT_IN_KGUITAR
		tracks.toFirst();
		#endif
		for (int numtrack=0; numtrack<nbtrack; numtrack++) {
			#ifndef NOT_IN_KGUITAR
			TabTrack *track = tracks.current();++tracks;
			#endif
			if (buf[1]==100) {	//Effect ?
				DUMMIES(36);	//Seems to be a note that begin
			}	//and end on 2 different bars, or a changement of timing (ie: 4/4 -> 12/8)
			GET_LONG( lgrbar );
			if (lgrbar==0) break;
			#ifdef NOT_IN_KGUITAR
			track = partition->get_track(numtrack);
			#endif
			tempstotal[numtrack] += lgrbar;
			#ifdef VERBOSE_NOTES
			sprintf(message,"Track %d bar %ld lenght %ld  begin at %ld end at %ld\n",numtrack,numbar,lgrbar,time[numtrack],tempstotal[numtrack]);	LOGS
			#endif

			#ifndef NOT_IN_KGUITAR
			unsigned long begin_bar = track->c.size();
			track->c.resize( track->c.size()+lgrbar );
			unsigned long end_bar = track->c.size();
			//Defaut data (clear)
			for (unsigned long i=begin_bar; i<end_bar; i++) {
				for (int j=0; j<MAX_STRINGS; j++) {
					track->c[i].a[j] = -1;
					track->c[i].e[j] = 0;
				}
				track->c[i].flags = 0;
			}
			track->b.resize( numbar+1 );
			track->b[numbar].start = time[numtrack];
			track->b[numbar].time1 = 4;
			track->b[numbar].time2 = 4;
			#endif

			if (lgrbar>0)
			do {
				int length;
				int strings;
				char *comment = NULL;
				strings = 0;
				int note_length = 0;
				switch (*buf) {
					case 0:		//Note(s)
					case 1:		//Ronde
						if (buf[1]==100) {	//Effect - pull
							DUMMIES(39);
							while (*buf<100 || *buf>115)
								DUMMIES(-1);
							DUMMIES(5);
							continue;
						}
						else if (buf[1]==50) {	//Effect - Half-Pull
							DUMMIES( 36 );
							continue;
						}

						length = 120/(1+buf[1]);
						strings = buf[2];
//						if (strings==0)
//							WHEREAMI("BUG : strings=0");
						#ifdef VERBOSE_NOTES
						sprintf(message,"\tTick %ld strings=%d\n",time[numtrack],strings);	LOGS
						#endif
						note_length = 1;
						DUMMIES( 3 );
						break;
					case 32:	//Triplet
						#if 1
						DUMMIES(4);
						#else
//						WHEREAMI("Triplet");
						length = 30;
						DUMMIES(6);
						string = get_string(buf)-1;
						fetch = buf[3];
						#ifdef VERBOSE_NOTES
						sprintf(message,"\tTick %d\n\t\tTriplet string %d fetches %d", time[numtrack], string-1,fetch);	LOGS
						#endif
						#ifdef NOT_IN_KGUITAR
						track->add( string,fetch,time[numtrack]++,length );
						#else
						track->c[time[numtrack]].setFullDuration(length);
						track->c[time[numtrack]++].a[5-(string)] = fetch;
						#endif
						DUMMIES( 11 );
						string = get_string(buf)-1;
						fetch = buf[3];
						#ifdef VERBOSE_NOTES
						sprintf(message,"-%d",fetch);	LOGS
						#endif
						#ifdef NOT_IN_KGUITAR
						track->add( string,fetch,time[numtrack]++,length );
						#else
						track->c[time[numtrack]].setFullDuration(length);
						track->c[time[numtrack]++].a[5-(string)] = fetch;
						#endif
						DUMMIES( 11 );
						string = get_string(buf)-1;
						fetch = buf[3];
						#ifdef VERBOSE_NOTES
						sprintf(message,"-%d\n",fetch);	LOGS
						#endif
						#ifdef NOT_IN_KGUITAR
						track->add( string,fetch,time[numtrack]++,length );
						#else
						track->c[time[numtrack]].setFullDuration(length);
						track->c[time[numtrack]++].a[5-(string)] = fetch;
						#endif
						DUMMIES( 4 );
						#endif
						break;
					case 4:	//Text (song or comment) + note
						length = 120/(1+buf[1]);
						DUMMIES(2);
						GET_STR(comment);
						#ifdef VERBOSE_NOTES
						sprintf(message,"\tTick %ld strings=%d\n\t\t{%s}\n",time[numtrack],strings, comment);	LOGS
						#endif
						free(comment);
						comment = NULL;
						strings = buf[0];
						note_length = 1;
						DUMMIES(1);
						break;
					case 64:	//Rest	2/4
					case 65:	//Rest	4/4
						#ifdef VERBOSE_NOTES
						sprintf(message,"\tTick %ld\n\t\tRest\n", time[numtrack]);	LOGS
						#endif
						DUMMIES(4);
						#ifdef NOT_IN_KGUITAR
						track->add( 3,'S',time[numtrack],120 );
						#endif
						note_length = 1;
						break;
					case 68:	//New timing + rest + comment
						DUMMIES(3);
						GET_STR(comment);
						DUMMIES(1);
						#ifdef VERBOSE_NOTES
						sprintf(message,"\tComment\n\t\t{%s}\n", comment);	LOGS
						#endif
						free(comment);
						comment = NULL;
						#ifdef NOT_IN_KGUITAR
						track->add( 3,'S',time[numtrack],120 );
						#endif
						note_length = 1;
						break;
					case 20:	//Text without note
						DUMMIES(2);
						GET_STR(comment);
						#ifdef VERBOSE_NOTES
						sprintf(message,"\tComment\n\t\t{%s}\n",comment);	LOGS
						#endif
						free(comment);
						comment = NULL;
						break;
					case 16:	//Track setting after 2 bytes
						DUMMIES(2);
						tempo = buf[7];
						#ifdef VERBOSE_NOTES
						sprintf(message,"New tempo = %d\n",tempo);	LOGS
						#endif
						DUMMIES(9+1);
						break;
					case 255:	//Track setting
						tempo = buf[7];
						#ifdef VERBOSE_NOTES
						sprintf(message,"New tempo = %d\n",tempo);	LOGS
						#endif
						DUMMIES(9);
						break;
					case 2:		//Effect - Pull and back
						DUMMIES(54);
						break;
					case 8:		//Efect - vibrato
						if (buf[1]==254) {	//AH
							length = 120/(1+buf[3]);
							strings = buf[2];
							note_length = 1;
							DUMMIES(4);
						}
						else	DUMMIES(1);
						break;
					case 5:		//Effect - Slide + text + strings
						if (buf[1]==0) {
							DUMMIES(2);
							GET_STR(comment);
							strings = *buf;
							note_length = 1;
							length = 30;
							#ifdef VERBOSE_NOTES
							sprintf(message,"\tTick %ld strings=%d Slide up\n",time[numtrack],strings);	LOGS
							#endif
							DUMMIES(1);
							free(comment);
							comment = NULL;
						}
						else	DUMMIES(45);
						break;
					default:
						#ifdef SLY_LOG
						sprintf(message,"\n** Unknown command Track %d bar %ld (%d)\n\n", numtrack,numbar, *buf);	fflush(stdout);	LOGS
						#else
						printf("\n** Unknown command Track %d bar %ld (%d)\n\n", numtrack,numbar, *buf);	fflush(stdout);
						#endif
						error = 1;
						break;
				}
				int mask,string;
				for (mask=1<<6,string=0; string<nbstring && error==0; mask>>=1,string++) {
					if (strings&mask) {
						unsigned char fetch = 0;
						switch (*buf) {
							case 32:	//Fetch
							case 34:	//Ronde
								fetch = buf[2];
								#ifdef VERBOSE_NOTES
								sprintf(message,"\t\tString %d Fetch %2d Lenght %3d\n",string,fetch,length);	LOGS
								#endif
								#ifdef NOT_IN_KGUITAR
								track->add( string,fetch,time[numtrack],length );
								#else
								track->c[time[numtrack]].setFullDuration(length);
								track->c[time[numtrack]].a[5-(string)] = fetch;
								#endif
								DUMMIES(3);
								break;
							case 40:	//Dot note
							case 42:	//Pull (?)
							case 48:	//Vibrato
								fetch = buf[2];
								DUMMIES(4);
								#ifdef VERBOSE_NOTES
								sprintf(message,"\t\tString %d Fetch %2d Lenght %3d Dot\n",string,fetch,length);	LOGS
								#endif
								#ifdef NOT_IN_KGUITAR
								track->add( string,fetch,time[numtrack],length );
								#else
								track->c[time[numtrack]].setFullDuration(length);
								track->c[time[numtrack]].a[5-(string)] = fetch;
								#endif
								break;
							default:
								#ifdef SLY_LOGS
								sprintf(message,"\n** Unknown note Track %d bar %ld (%d)\n\n", numtrack,numbar, *buf);	fflush(stdout);	LOGS
								#else
								printf("\n** Unknown note Track %d bar %ld (%d)\n\n", numtrack,numbar, *buf);	fflush(stdout);
								#endif
								error = 1;
								break;
						}
					}
				}
				time[numtrack] += note_length;
			}	//Next track
			while (!error && time[numtrack]<tempstotal[numtrack] && size>=0);
			if ( error ) {
				WHEREAMI("Unknown at");
				break;
			}

			#ifdef VERBOSE_NOTES
WHEREAMI("fin bar");
//			sprintf(message, "Track %d bar %ld fini\n",numtrack,numbar);	LOGS
			#endif
			if (size<=0) break;
			while (*buf==0) DUMMIES( -1 );	//Out of sync after a triplet at the end of the bar

			#ifdef NOT_IN_KGUITAR
			if (partition->get_time()<time[numtrack])
				partition->set_time( time[numtrack] );
			for (int i=0; i<MAX_STRINGS; i++)
				track->add( i,'|',time[numtrack],0 );
			#endif
			if (time[numtrack]!=tempstotal[numtrack]) {
				sprintf(message, "BUG! time[numtrack]!=tempstotal[numtrack] : Track %d, Bar %ld : %ld != %ld (size=%ld)\n", numtrack, numbar, time[numtrack], tempstotal[numtrack], size);	LOGS
				error = 1;
				break;
			}
		}	//Next bar
		numbar++;
	} while (!error && lgrbar>0 && size>=0);
	if (size<0) size = 0;

	printf("Managed to parse %.2f percent of the track infos\n\n", 100.0f-100.0f*(float)size/(float)size_tracks);

	//Close the logs file
	#ifdef SLY_LOGS
	fclose(logs);
	#endif
	free( buffer );
	return TRUE;
}

bool TabSong::saveToGp3(QString)
{
    // Saving to Guitar Pro 3 format here
    // From Sly: "In your dreams"
    return FALSE;
}

/* End of Sly's dirty piece of code
 */

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
// Assembles the whole TSE song from various tracks, generated with
// corresponding midiTrack() calls.
TSE3::Song *TabSong::midiSong(bool tracking)
{
	TSE3::Song *song = new TSE3::Song(0);

	// Create tempo track
	TSE3::Event<TSE3::Tempo> tempoEvent(tempo, TSE3::Clock(0));
	song->tempoTrack()->insert(tempoEvent);

	// Create data tracks
	int tn = 0;
	QListIterator<TabTrack> it(t);
	for (; it.current(); ++it) {
		TSE3::PhraseEdit *trackData = it.current()->midiTrack(tracking, tn);
		TSE3::Phrase *phrase = trackData->createPhrase(song->phraseList());
		TSE3::Part *part = new TSE3::Part(0, trackData->lastClock() + 1); // GREYFIX: this may be why last event got clipped?
		part->setPhrase(phrase);
		TSE3::Track *trk = new TSE3::Track();
		trk->insert(part);
		song->insert(trk);
		delete trackData;
		tn++;
	}

	return song;
}
#endif
