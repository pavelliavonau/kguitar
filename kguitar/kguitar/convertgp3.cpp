#include "convertgp3.h"
#include "settings.h"

#include <qfile.h>
#include <qdatastream.h>

ConvertGp3::ConvertGp3(TabSong *song): ConvertBase(song) {}

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

bool ConvertGp3::load(QString fileName)
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
	GET_STR(title);		song->title = QString::fromLocal8Bit(str);
	GET_STR(subtitle);
	GET_STR(artist);	song->author = QString::fromLocal8Bit(str);
	GET_STR(album);
	GET_STR(author);
	GET_STR(copyright);
	GET_STR(tabled_by);	song->transcriber = QString::fromLocal8Bit(str);
	GET_STR(instruction);
	char variente = *buf==1;	//FIXME What's the real meaning of this byte?
	if (variente)
		DUMMIES( 4 );
	GET_STR(notice);	song->comments = QString::fromLocal8Bit(str);
	if (variente)
		DUMMIES( 1 );
	song->tempo = *buf;

	//Find beginning of tracks
	DUMMIES( 780 );
	while (*buf<' ') DUMMIES(1);	//Find 1st character of the first track name
	DUMMIES(-1);			//Get back on the "lenght" bytes

	//Read track infos: name
	song->t.clear();
	int nbtrack = 0;
	int nbstring = 6;	//Shoud be read somewhere for each track

	do {
		GET_STR_NO_INCBUF;
		printf("Track %s\n",str);fflush(stdout);
		song->t.append( new TabTrack(TabTrack::FretTab, 0, 0, 0, 0, 6, 24) );	//First 24: 24 frets, cos don't know the exact number
		song->t.current()->name = QString::fromLocal8Bit(str);
		song->t.current()->b.resize(0);
		song->t.current()->c.resize(0);
		nbtrack++;
		DUMMIES( 98 );	//There are 98 bytes of description per track. Title (fixed max lenght)+ ???
	} while (IS_CHAR(buf[1]) && IS_CHAR(buf[2]));
	printf("%d track(s)\n\n", nbtrack);fflush(stdout);
	QListIterator<TabTrack> tracks(song->t);

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
						song->tempo = buf[7];
						#ifdef VERBOSE_NOTES
						sprintf(message,"New tempo = %d\n",tempo);	LOGS
						#endif
						DUMMIES(9+1);
						break;
					case 255:	//Track setting
						song->tempo = buf[7];
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
	free(buffer);
	return TRUE;
}

bool ConvertGp3::save(QString)
{
    // Saving to Guitar Pro 3 format here
    // From Sly: "In your dreams"
    return FALSE;
}
