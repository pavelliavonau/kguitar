#include "convertgtp.h"

#include <qfile.h>
#include <qdatastream.h>

ConvertGtp::ConvertGtp(TabSong *song): ConvertBase(song) {}

// Reads Delphi string in GPro format. Delphi string looks pretty much like:
// <max-length> <real-length> <real-length * bytes - string data>

void ConvertGtp::readDelphiString(char *c)
{
	Q_UINT8 maxl, l;
	(*stream) >> maxl;
	(*stream) >> l;
	if (maxl == 0)
		maxl = l;
	else
		maxl--;
	stream->readRawBytes(c, maxl);
	c[l] = 0;
}

int ConvertGtp::readDelphiInteger()
{
	Q_UINT8 x;
	int r;
	(*stream) >> x; r = x;
	(*stream) >> x; r += x << 8;
	(*stream) >> x; r += x << 16;
	(*stream) >> x; r += x << 24;
	return r;
}

bool ConvertGtp::load(QString fileName)
{
	QFile f(fileName);
	if (!f.open(IO_ReadOnly))
		return FALSE;

	QDataStream s(&f);
	stream = &s;

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

	readDelphiString(c);                       // Song title
	song->title = QString::fromLocal8Bit(c);

	readDelphiString(c);                       // Author
	song->author = QString::fromLocal8Bit(c);

	readDelphiString(c);                       // Comments
	song->comments = QString::fromLocal8Bit(c);

	song->transcriber = QString("KGuitar");

	song->tempo = readDelphiInteger();       // Tempo

	readDelphiInteger();                     // GREYFIX - Triplet feel
	readDelphiInteger();                     // Unknown 4 bytes

	// TUNING INFORMATION

	song->t.clear();

	for (int i = 0; i < 8; i++) {
		num = readDelphiInteger();           // Number of strings
		song->t.append(new TabTrack(TabTrack::FretTab, 0, 0, 0, 0, num, 24));
		for (int j = 0; j < num; j++)
			song->t.current()->tune[j] = readDelphiInteger();
	}

	int maxbar = readDelphiInteger();        // Quantity of bars

	// TRACK ATTRIBUTES

	QListIterator<TabTrack> it(song->t);
	for (; it.current(); ++it) {
		TabTrack *trk = it.current();
		trk->patch = readDelphiInteger();    // MIDI Patch
		trk->frets = readDelphiInteger();    // Frets
		readDelphiString(c);                   // Track name
		trk->name = QString::fromLocal8Bit(c);
		s >> num;                              // Flags - GREYFIX!
		readDelphiInteger();                 // Slider: Volume (0-0x7F) - GREYFIX!
		readDelphiInteger();                 // Slider: Pan
		readDelphiInteger();                 // Slider: Chorus
		readDelphiInteger();                 // Slider: Reverb
		readDelphiInteger();                 // Capo
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

			int thisbar = readDelphiInteger();// Number of tab columns
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
				trk->c[x].setFullDuration(readDelphiInteger() / 4);

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
					readDelphiString(c);
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

bool ConvertGtp::save(QString)
{
    return FALSE;
}
