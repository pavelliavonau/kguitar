#include "convertgtp.h"

#include <klocale.h>
#include <qfile.h>
#include <qdatastream.h>

ConvertGtp::ConvertGtp(TabSong *song): ConvertBase(song)
{
	strongChecks = TRUE;
}

void ConvertGtp::skipBytes(int n)
{
	int x = stream->skipRawData(n);
	if (x != n)  throw QString("skipBytes: skip past EOF");
}

QString ConvertGtp::readDelphiString()
{
	QString str;
	Q_UINT8 l;
	char *c;

	int maxl = readDelphiInteger();
	if (stream->device()->atEnd())  throw QString("readDelphiString: EOF");
	(*stream) >> l;

	if (maxl != l + 1)  throw QString("readDelphiString: first word doesn't match second byte");

	c = (char *) malloc(l + 5);

	if (stream->device()->size() - stream->device()->pos() < l) {
		throw QString("readDelphiString: not enough bytes to read %1 byte string").arg(l);
	}

	if (c) {
		stream->readRawBytes(c, l);
		c[l] = 0;
		str = QString::fromLocal8Bit(c);
		free(c);
	}

	return str;
}

QString ConvertGtp::readPascalString(int maxlen)
{
	QString str;
	Q_UINT8 l;
	char *c;

	(*stream) >> l;

	c = (char *) malloc(l + 5);

	if (c) {
		stream->readRawBytes(c, l);
		c[l] = 0;
		str = QString::fromLocal8Bit(c);
		free(c);
	}

	// Skip garbage after pascal string end
	skipBytes(maxlen - l);

	return str;
}

QString ConvertGtp::readWordPascalString()
{
	QString str;
	char *c;

	int l = readDelphiInteger();

	c = (char *) malloc(l + 5);

	if (c) {
		stream->readRawBytes(c, l);
		c[l] = 0;
		str = QString::fromLocal8Bit(c);
		free(c);
	}

	return str;
}

int ConvertGtp::readDelphiInteger()
{
	Q_UINT8 x;
	int r;
	if (stream->device()->atEnd())  throw QString("readDelphiInteger: EOF");
	(*stream) >> x; r = x;
	if (stream->device()->atEnd())  throw QString("readDelphiInteger: EOF");
	(*stream) >> x; r += x << 8;
	if (stream->device()->atEnd())  throw QString("readDelphiInteger: EOF");
	(*stream) >> x; r += x << 16;
	if (stream->device()->atEnd())  throw QString("readDelphiInteger: EOF");
	(*stream) >> x; r += x << 24;
	return r;
}

void ConvertGtp::readChromaticGraph()
{
	Q_UINT8 num;
	int n;

	// GREYFIX: currently just skips over chromatic graph
	(*stream) >> num;                        // icon
	readDelphiInteger();                     // shown amplitude
	n = readDelphiInteger();                 // number of points
	for (int i = 0; i < n; i++) {
		readDelphiInteger();                 // time
		readDelphiInteger();                 // pitch
		(*stream) >> num;                    // vibrato
	}
}

void ConvertGtp::readChord()
{
	int x1, x2, x3, x4;
	Q_UINT8 num;
	QString text;
	char garbage[50];
	// GREYFIX: currently just skips over chord diagram

	// GREYFIX: chord diagram
	x1 = readDelphiInteger();
	if (x1 != 257)
		kdWarning() << "Chord INT1=" << x1 << ", not 257\n";
	x2 = readDelphiInteger();
	if (x2 != 0)
		kdWarning() << "Chord INT2=" << x2 << ", not 0\n";
	x3 = readDelphiInteger();
	kdDebug() << "Chord INT3: " << x3 << "\n"; // FF FF FF FF if there is diagram
	x4 = readDelphiInteger();
	if (x4 != 0)
		kdWarning() << "Chord INT4=" << x4 << ", not 0\n";
	(*stream) >> num;
	if (num != 0)
		kdWarning() << "Chord BYTE5=" << (int) num << ", not 0\n";
	text = readPascalString(25);
	kdDebug() << "Chord diagram: " << text << "\n";

	// Chord diagram parameters - for every string
	for (int i = 0; i < STRING_MAX_NUMBER; i++) {
		x1 = readDelphiInteger();
		kdDebug() << x1 << "\n";
	}

	// Unknown bytes
	stream->readRawBytes(garbage, 36);

	kdDebug() << "after chord, position: " << stream->device()->pos() << "\n";
}

void ConvertGtp::readSignature()
{
	currentStage = QString("readSignature");

	QString s = readPascalString(30);        // Format string
	kdDebug() << "GTP format: \"" << s << "\"\n";

	// Parse version string
	if (s == "FICHIER GUITARE PRO v1") {
		versionMajor = 1; versionMinor = 0;
	} else if (s == "FICHIER GUITARE PRO v1.01") {
		versionMajor = 1; versionMinor = 1;
	} else if (s == "FICHIER GUITARE PRO v1.02") {
		versionMajor = 1; versionMinor = 2;
	} else if (s == "FICHIER GUITARE PRO v1.03") {
		versionMajor = 1; versionMinor = 3;
	} else if (s == "FICHIER GUITARE PRO v1.04") {
		versionMajor = 1; versionMinor = 4;
	} else if (s == "FICHIER GUITAR PRO v2.20") {
		versionMajor = 2; versionMinor = 20;
	} else if (s == "FICHIER GUITAR PRO v2.21") {
		versionMajor = 2; versionMinor = 21;
	} else if (s == "FICHIER GUITAR PRO v3.00") {
		versionMajor = 3; versionMinor = 0;
	} else if (s == "FICHIER GUITAR PRO v4.00") {
		versionMajor = 4; versionMinor = 0;
	} else if (s == "FICHIER GUITAR PRO v4.06") {
		versionMajor = 4; versionMinor = 6;
	} else if (s == "FICHIER GUITAR PRO L4.06") {
		versionMajor = 4; versionMinor = 6;
	} else if (s == "FICHIER GUITAR PRO v5.00") {
		versionMajor = 5; versionMinor = 0;
	} else if (s == "FICHIER GUITAR PRO v5.10") {
		versionMajor = 5; versionMinor = 10;
	} else {
		throw i18n("Invalid file format: \"%1\"").arg(s);
	}
}

void ConvertGtp::readSongAttributes()
{
	QString s;

	Q_UINT8 num;

	currentStage = QString("readSongAttributes: song->info");

	song->info["TITLE"] = readDelphiString();
	song->info["SUBTITLE"] = readDelphiString();
	song->info["ARTIST"] = readDelphiString();
	song->info["ALBUM"] = readDelphiString();
	if (versionMajor >= 5)
		song->info["LYRICIST"] = readDelphiString();
	song->info["COMPOSER"] = readDelphiString();
	song->info["COPYRIGHT"] = readDelphiString();
	song->info["TRANSCRIBER"] = readDelphiString();
	song->info["INSTRUCTIONS"] = readDelphiString();

	// Notice lines
	currentStage = QString("readSongAttributes: notice lines");
	song->info["COMMENTS"] = "";
	int n = readDelphiInteger();
	for (int i = 0; i < n; i++)
		song->info["COMMENTS"] += readDelphiString() + "\n";

	if (versionMajor <= 4) {
		currentStage = QString("readSongAttributes: shuffle rhythm feel");
		(*stream) >> num;                // GREYFIX: Shuffle rhythm feel
	}

	if (versionMajor >= 4) {
		currentStage = QString("readSongAttributes: lyrics");
		// Lyrics
		if (versionMajor <= 4)
			readDelphiInteger();                 // GREYFIX: Lyric track number start
		for (int i = 0; i < LYRIC_LINES_MAX_NUMBER; i++) {
			readDelphiInteger();                 // GREYFIX: Start from bar
			readWordPascalString();              // GREYFIX: Lyric line
		}
	}

	if (versionMajor >= 5) {
		currentStage = QString("readSongAttributes: print page");
		// Print page
		skipBytes((versionMinor > 0) ? 52 : 33);
		for (int i = 0; i < 11; i++)
			readDelphiString();
	}

	currentStage = QString("readSongAttributes: tempo");
	song->tempo = readDelphiInteger();       // Tempo
	kdDebug() << "tempo: " << song->tempo << "\n";

	if (versionMajor > 5 || (versionMajor == 5 && versionMinor > 0))
		skipBytes(1);

	if (versionMajor >= 4) {
		(*stream) >> num;                // GREYFIX: key
		readDelphiInteger();             // GREYFIX: octave
	} else {
		readDelphiInteger();             // GREYFIX: key
	}
}

void ConvertGtp::readTrackDefaults()
{
	Q_UINT8 num, volume, pan, chorus, reverb, phase, tremolo;
	currentStage = QString("readTrackDefaults");

	for (int i = 0; i < TRACK_MAX_NUMBER * 2; i++) {
		trackPatch[i] = readDelphiInteger(); // MIDI Patch
		(*stream) >> volume;                 // GREYFIX: volume
		(*stream) >> pan;                    // GREYFIX: pan
		(*stream) >> chorus;                 // GREYFIX: chorus
		(*stream) >> reverb;                 // GREYFIX: reverb
		(*stream) >> phase;                  // GREYFIX: phase
		(*stream) >> tremolo;                // GREYFIX: tremolo
		kdDebug() << "=== TrackDefaults: " << i <<
			" (patch=" << trackPatch[i] <<
			" vol=" << (int) volume <<
			" p=" << (int) pan <<
			" c=" << (int) chorus <<
			" ph=" << (int) phase <<
			" tr=" << (int) tremolo << "\n";

		(*stream) >> num;                    // 2 byte padding: must be 00 00
		if (num != 0)  kdDebug() << QString("1 of 2 byte padding: there is %1, must be 0\n").arg(num);
		(*stream) >> num;
		if (num != 0)  kdDebug() << QString("2 of 2 byte padding: there is %1, must be 0\n").arg(num);
	}

	// Some weird padding, filled with FFs
	if (versionMajor >= 5)
		skipBytes(42);
}

void ConvertGtp::readBarProperties()
{
	Q_UINT8 bar_bitmask, num;

	int time1 = 4;
	int time2 = 4;
	int keysig = 0;

	bars.resize(numBars);

	currentStage = QString("readBarProperties");
	kdDebug() << "readBarProperties(): start\n";

	for (int i = 0; i < numBars; i++) {
		(*stream) >> bar_bitmask;                    // bar property bitmask
		if (bar_bitmask != 0)
			kdDebug() << "BAR #" << i << " - flags " << (int) bar_bitmask << "\n";
		// GREYFIX: new_time_numerator
		if (bar_bitmask & 0x01) {
			(*stream) >> num;
			time1 = num;
			kdDebug() << "new time1 signature: " << time1 << ":" << time2 << "\n";
		}
		// GREYFIX: new_time_denominator
		if (bar_bitmask & 0x02) {
			(*stream) >> num;
			time2 = num;
			kdDebug() << "new time2 signature: " << time1 << ":" << time2 << "\n";
		}

		// Since GP5, time signature changes also brings new
		// "beam eight notes by" array
		if ((versionMajor >= 5) && (bar_bitmask & 0x03)) {
			// "beam eight notes by" array, usually 2-2-2-2
			skipBytes(4);
		}

		// GREYFIX: begin repeat
		if (bar_bitmask & 0x04) {
			kdDebug() << "begin repeat\n";
		}
		// GREYFIX: number_of_repeats
		if (bar_bitmask & 0x08) {
			(*stream) >> num;
			kdDebug() << "end repeat " << (int) num << "x\n";
		}
		// GREYFIX: alternative_ending_to
		if (bar_bitmask & 0x10) {
			(*stream) >> num;
			kdDebug() << "alternative ending to " << (int) num << "\n";
		}
		// GREYFIX: new section
		if (bar_bitmask & 0x20) {
			QString text = readDelphiString();
			readDelphiInteger(); // color?
			kdDebug() << "new section: " << text << "\n";
		}
		if (bar_bitmask & 0x40) {
			(*stream) >> num;                // GREYFIX: alterations_number
			keysig = num;
			(*stream) >> num;                // GREYFIX: minor
			kdDebug() << "new key signature (" << keysig << ", " << num << ")\n";
		}
		// GREYFIX: double bar
		if (bar_bitmask & 0x80) {
			kdDebug() << "double bar\n";
		}

		bars[i].time1 = time1;
		bars[i].time2 = time1;
		bars[i].keysig = keysig;

		// GREYFIX: unknown bytes, should be 00 00 00
		if (versionMajor >= 5)
			skipBytes(3);
	}
	kdDebug() << "readBarProperties(): end\n";
}

void ConvertGtp::readTrackProperties()
{
	Q_UINT8 num;
	int strings, midiChannel2, capo, color;

	currentStage = QString("readTrackProperties");
	kdDebug() << "readTrackProperties(): start\n";

// 	if (versionMajor >= 5)
// 		skipBytes(3);

	for (int i = 0; i < numTracks; i++) {
		kdDebug() << "start track pos: " << stream->device()->pos() << "\n";

		(*stream) >> num;                    // GREYFIX: simulations bitmask
		kdDebug() << "Simulations: " << num << "\n";

// 		if (versionMajor >= 5)
// 			skipBytes(4);

		song->t.append(new TabTrack(TabTrack::FretTab, 0, 0, 0, 0, 6, 24));
		TabTrack *trk = song->t.at(i);

		trk->name = readPascalString(40);    // Track name
		kdDebug() << "Track: " << trk->name << "\n";

		// Tuning information

		kdDebug() << "pos: " << stream->device()->pos() << "\n";

		strings = readDelphiInteger();
		if (strings <= 0 || strings > STRING_MAX_NUMBER)  throw QString("Track %1: insane # of strings (%2)\n").arg(i).arg(strings);
		trk->string = strings;

		// Parse [0..string-1] with real string tune data in reverse order
		for (int j = trk->string - 1; j >= 0; j--) {
			trk->tune[j] = readDelphiInteger();
			if (trk->tune[j] > 127)  throw QString("Track %1: insane tuning on string %2 = %3\n").arg(i).arg(j).arg(trk->tune[j]);
		}

		// Throw out the other useless garbage in [string..MAX-1] range
		for (int j = trk->string; j < STRING_MAX_NUMBER; j++)
			readDelphiInteger();

		// GREYFIX: auto flag here?

		readDelphiInteger();                 // GREYFIX: MIDI port
		trk->channel = readDelphiInteger();  // MIDI channel 1
		midiChannel2 = readDelphiInteger();  // GREYFIX: MIDI channel 2
		trk->frets = readDelphiInteger();    // Frets
		capo = readDelphiInteger();          // GREYFIX: Capo
		color = readDelphiInteger();         // GREYFIX: Color

		if (versionMajor >= 5) {
			if (versionMajor > 5 || (versionMajor == 5 && versionMinor > 0)) {
				skipBytes(49);
				kdDebug() << "additional track string1: " << readDelphiString();
				kdDebug() << "additional track string2: " << readDelphiString();
			} else {
				skipBytes(41);
			}
		}

		kdDebug() <<
			"MIDI #" << trk->channel << "/" << (int) midiChannel2 << ", " <<
			(int) trk->string << " strings, " <<
			(int) trk->frets << " frets, capo " <<
			capo << "\n";

		if (trk->frets <= 0 || (strongChecks && trk->frets > 100))  throw QString("Track %1: insane number of frets (%2)\n").arg(i).arg(trk->frets);
		if (trk->channel > 16)  throw QString("Track %1: insane MIDI channel 1 (%2)\n").arg(i).arg(trk->channel);
		if (midiChannel2 < 0 || midiChannel2 > 16)  throw QString("Track %1: insane MIDI channel 2 (%2)\n").arg(i).arg(midiChannel2);

		// Fill remembered values from defaults
		trk->patch = trackPatch[i];

		// If it's GP5, create a full copy of this track: we
		// will use it to simulate "voice 2".
		if (versionMajor >= 5)
			song->t.append(new TabTrack(trk));
	}

	kdDebug() << "end all tracks pos: " << stream->device()->pos() << "\n";

	if (versionMajor >= 5) {
		skipBytes(1);
// 		if (versionMajor == 5 && versionMinor == 0)
// 			skipBytes(1);
	}

	kdDebug() << "readTrackProperties(): end\n";
}

void ConvertGtp::readTabs()
{
	currentStage = QString("readTabs");

	for (int tr = 0; tr < numTracks; tr++) {
		TabTrack *trk = song->t.at(tr);
		trk->b.resize(numBars);
		trk->c.resize(0);
	}

	for (int j = 0; j < numBars; j++) {
		for (int tr = 0; tr < numTracks; tr++) {
			TabTrack *trk;
			// Guitar Pro 5 includes support for "multiple
			// voices", namely 2. We will translate them
			// into 2 separate tracks (later we will
			// destroy empty tracks).
			if (versionMajor >= 5) {
				for (int voice = 0; voice < 2; voice++) {
					currentStage = QString("readTabs: track %1, bar %2, voice %3").arg(tr).arg(j).arg(voice);
					trk = song->t.at(tr * 2 + voice);
					kdDebug() << "TRACK " << tr << " (voice " << voice << "), BAR " << j << " (position: " << stream->device()->pos() << ")\n";
					readBar(trk, j);
				}
			} else {
				currentStage = QString("readTabs: track %1, bar %2").arg(tr).arg(j);
				trk = song->t.at(tr);
				kdDebug() << "TRACK " << tr << ", BAR " << j << " (position: " << stream->device()->pos() << ")\n";
				readBar(trk, j);
			}
		}
	}
}

void ConvertGtp::readBar(TabTrack *trk, int j)
{
	int x;
	int numBeats = readDelphiInteger();
	kdDebug() << "numBeats " << numBeats << " (position: " << stream->device()->pos() << ")\n";

	if (numBeats < 0 || (strongChecks && numBeats > 128))
		throw QString("insane number of beats: %1").arg(numBeats);
	
	x = trk->c.size();
	trk->c.resize(trk->c.size() + numBeats);
	trk->b[j].time1 = bars[j].time1;
	trk->b[j].time2 = bars[j].time2;
	trk->b[j].keysig = bars[j].keysig;
	trk->b[j].start = x;
	
	for (int k = 0; k < numBeats; k++) {
		readColumn(trk, x);
		x++;
	}
}

void ConvertGtp::readColumn(TabTrack *trk, int x)
{
	Q_UINT8 beat_bitmask, strings, num;
	Q_INT8 length, volume, pan, chorus, reverb, phase, tremolo;

	trk->c[x].flags = 0;

	if (num != 0)
		kdWarning() << "prefix != 0";

	(*stream) >> beat_bitmask;   // beat bitmask

	if (beat_bitmask & 0x01)     // dotted column
		trk->c[x].flags |= FLAG_DOT;

	if (beat_bitmask & 0x40) {
		(*stream) >> num;        // GREYFIX: pause_kind
	}

	// Guitar Pro 4 beat lengths are as following:
	// -2 = 1    => 480     3-l = 5  2^(3-l)*15
	// -1 = 1/2  => 240           4
	//  0 = 1/4  => 120           3
	//  1 = 1/8  => 60            2
	//  2 = 1/16 => 30 ... etc    1
	//  3 = 1/32 => 15            0

	(*stream) >> length;            // length
	kdDebug() << "beat_bitmask: " << (int) beat_bitmask << "; length: " << length << "\n";

	trk->c[x].l = (1 << (3 - length)) * 15;

	if (beat_bitmask & 0x20) {
		int tuple = readDelphiInteger();
		kdDebug() << "Tuple: " << tuple << "\n"; // GREYFIX: t for tuples
		if (!(tuple == 3 || (tuple >= 5 && tuple <= 7) || (tuple >= 9 && tuple <= 13)))  throw QString("Insane tuple t: %1").arg(tuple);
	}

	if (beat_bitmask & 0x02)     // Chord diagram
		readChord();

	if (beat_bitmask & 0x04) {
		kdDebug() << "Text: " << readDelphiString() << "\n"; // GREYFIX: text with a beat
	}

	// GREYFIX: column-wide effects
	if (beat_bitmask & 0x08)
		readColumnEffects(trk, x);

	if (beat_bitmask & 0x10) {     // mixer variations
		(*stream) >> num;          // GREYFIX: new MIDI patch
		(*stream) >> volume;       // GREYFIX: new
		(*stream) >> pan;          // GREYFIX: new
		(*stream) >> chorus;       // GREYFIX: new
		(*stream) >> reverb;       // GREYFIX: new
		(*stream) >> phase;        // GREYFIX: new
		(*stream) >> tremolo;      // GREYFIX: new
		int tempo = readDelphiInteger(); // GREYFIX: new tempo

		// GREYFIX: transitions
		if (volume != -1)   (*stream) >> num;
		if (pan != -1)      (*stream) >> num;
		if (chorus != -1)   (*stream) >> num;
		if (reverb != -1)   (*stream) >> num;
		if (tremolo != -1)  (*stream) >> num;
		if (tempo != -1)    (*stream) >> num;

		if (versionMajor >= 4) {
			(*stream) >> num;          // bitmask: what should be applied to all tracks
		}
	}

	(*stream) >> strings;          // used strings mask

	for (int y = STRING_MAX_NUMBER - 1; y >= 0; y--) {
		trk->c[x].e[y] = 0;
		trk->c[x].a[y] = NULL_NOTE;
		if (strings & (1 << (y + STRING_MAX_NUMBER - trk->string)))
			readNote(trk, x, y);
	}

	// Dump column
	QString tmp = "";
	for (int y = 0; y <= trk->string; y++) {
		if (trk->c[x].a[y] == NULL_NOTE) {
			tmp += ".";
		} else {
			tmp += '0' + trk->c[x].a[y];
		}
	}
	kdDebug() << "[" << tmp << "]\n";

	if (versionMajor >= 5) {
		(*stream) >> num;
		kdDebug() << "trailing byte: " << num << "\n";
		if (num == 7 || num == 8 || num == 10)
			skipBytes(1);
		skipBytes(1);
	}
}

void ConvertGtp::readColumnEffects(TabTrack *trk, int x)
{
	Q_UINT8 fx_bitmask1 = 0, fx_bitmask2 = 0, num;

	(*stream) >> fx_bitmask1;
	if (versionMajor >= 4) {
		(*stream) >> fx_bitmask2;
		kdDebug() << "column-wide fx: " << (int) fx_bitmask1 << "/" << (int) fx_bitmask2 << "\n";
	} else {
		kdDebug() << "column-wide fx: " << (int) fx_bitmask1 << "\n";
	}
	
	if (fx_bitmask1 & 0x20) {      // GREYFIX: string torture
		(*stream) >> num;
		switch (num) {
		case 0:                    // GREYFIX: tremolo bar
			if (versionMajor < 4)  readDelphiInteger();
			break;
		case 1:                    // GREYFIX: tapping
			if (versionMajor < 4)  readDelphiInteger(); // ?
			break;
		case 2:                    // GREYFIX: slapping
			if (versionMajor < 4)  readDelphiInteger(); // ?
			break;
		case 3:                    // GREYFIX: popping
			if (versionMajor < 4)  readDelphiInteger(); // ?
			break;
		default:
			throw QString("Unknown string torture effect: %1").arg(num);
		}
	}
	if (fx_bitmask1 & 0x04) {      // GP3 column-wide natural harmonic
		kdDebug() << "GP3 column-wide natural harmonic\n";
		for (int y = 0; y < trk->string; y++)
			trk->c[x].e[y] |= EFFECT_HARMONIC;
	}
	if (fx_bitmask1 & 0x08) {      // GP3 column-wide artificial harmonic
		kdDebug() << "GP3 column-wide artificial harmonic\n";
		for (int y = 0; y < trk->string; y++)
			trk->c[x].e[y] |= EFFECT_ARTHARM;
        }
	if (fx_bitmask2 & 0x04)
		readChromaticGraph();  // GREYFIX: tremolo graph
	if (fx_bitmask1 & 0x40) {
		(*stream) >> num;      // GREYFIX: down stroke length
		(*stream) >> num;      // GREYFIX: up stroke length
	}
	if (fx_bitmask2 & 0x02) {
		(*stream) >> num;      // GREYFIX: stroke pick direction
	}
	if (fx_bitmask1 & 0x01) {      // GREYFIX: GP3 column-wide vibrato
	}
	if (fx_bitmask1 & 0x02) {      // GREYFIX: GP3 column-wide wide vibrato (="tremolo" in GP3)
	}
}

void ConvertGtp::readNote(TabTrack *trk, int x, int y)
{
	Q_UINT8 note_bitmask, variant, num, mod_mask1, mod_mask2;

	(*stream) >> note_bitmask;               // note bitmask
	(*stream) >> variant;                    // variant

	if (note_bitmask) {
		kdDebug() << "note_bitmask: " << (int) note_bitmask << "\n";
	}

	if (note_bitmask & 0x01) {               // GREYFIX: note != beat
		(*stream) >> num;                    // length
		(*stream) >> num;                    // t
	}

	if (note_bitmask & 0x02) {};             // GREYFIX: note is dotted

	if (note_bitmask & 0x10) {               // GREYFIX: dynamic
		(*stream) >> num;
	}

	(*stream) >> num;                        // fret number
	trk->c[x].a[y] = num;

	if (variant == 2) {                      // link with previous beat
		trk->c[x].flags |= FLAG_ARC;
		for (uint i = 0; i < MAX_STRINGS; i++) {
			trk->c[x].a[i] = NULL_NOTE;
			trk->c[x].e[i] = 0;
		}
	}

	if (variant == 3)                        // dead notes
		trk->c[x].a[y] = DEAD_NOTE;

	if (note_bitmask & 0x80) {               // GREYFIX: fingering
		(*stream) >> num;
		(*stream) >> num;
	}

	if (note_bitmask & 0x08) {
		(*stream) >> mod_mask1;
		if (versionMajor >= 4) {
			(*stream) >> mod_mask2;
			kdDebug() << "note mod: mask1=" << mod_mask1 << " mask2=" << mod_mask2 << "\n";
		} else {
			kdDebug() << "note mod: mask1=" << mod_mask1 << "\n";
		}
		if (mod_mask1 & 0x01) {
			readChromaticGraph();            // GREYFIX: bend graph
		}
		if (mod_mask1 & 0x02)                // hammer on / pull off
			trk->c[x].e[y] |= EFFECT_LEGATO;
		if (mod_mask1 & 0x08)                // let ring
			trk->c[x].e[y] |= EFFECT_LETRING;
		if (mod_mask1 & 0x10) {              // GREYFIX: graces
			(*stream) >> num;                // GREYFIX: grace fret
			(*stream) >> num;                // GREYFIX: grace dynamic
			(*stream) >> num;                // GREYFIX: grace transition
			(*stream) >> num;                // GREYFIX: grace length
		}
		if (versionMajor >= 4) {
			if (mod_mask2 & 0x01)                // staccato - we do palm mute
				trk->c[x].flags |= FLAG_PM;
			if (mod_mask2 & 0x02)                // palm mute - we mute the whole column
				trk->c[x].flags |= FLAG_PM;
			if (mod_mask2 & 0x04) {              // GREYFIX: tremolo
				(*stream) >> num;                // GREYFIX: tremolo picking length
			}
			if (mod_mask2 & 0x08) {              // slide
				trk->c[x].e[y] |= EFFECT_SLIDE;
				(*stream) >> num;                // GREYFIX: slide kind
			}
			if (mod_mask2 & 0x10) {              // GREYFIX: harmonic
				(*stream) >> num;                // GREYFIX: harmonic kind
			}
			if (mod_mask2 & 0x20) {              // GREYFIX: trill
				(*stream) >> num;                // GREYFIX: trill fret
				(*stream) >> num;                // GREYFIX: trill length
			}
		}
	}
}

bool ConvertGtp::load(QString fileName)
{
	QFile f(fileName);
	if (!f.open(QIODevice::ReadOnly))
		throw i18n("Unable to open file for reading");

	QDataStream s(&f);
	stream = &s;

	try {
//		song = new TabSong();

		readSignature();
		song->t.clear();
		readSongAttributes();
	 	readTrackDefaults();

	 	numBars = readDelphiInteger();           // Number of bars
		if (numBars <= 0 || (strongChecks && numBars > 16384))  throw QString("Insane number of bars: %1").arg(numBars);
		kdDebug() << "Bars: " << numBars << "\n";

	 	numTracks = readDelphiInteger();         // Number of tracks
		if (numTracks <= 0 || (strongChecks && numTracks > 32))  throw QString("Insane number of tracks: %1").arg(numTracks);
		kdDebug() << "Tracks: " << numTracks << "\n";

	 	readBarProperties();
	 	readTrackProperties();
	 	readTabs();

		currentStage = QString("Exit code");
		if (!f.atEnd()) {
			int ex = readDelphiInteger();            // Exit code: 00 00 00 00
			if (ex != 0)
				kdWarning() << "File not ended with 00 00 00 00\n";
			if (!f.atEnd())
				kdWarning() << "File not ended - there's more data!\n";
		}
	} catch (QString msg) {
		throw
			i18n("Guitar Pro import error:") + QString("\n") +
			msg + QString("\n") +
			i18n("Stage: %1").arg(currentStage) + QString("\n") +
			i18n("File position: %1/%2").arg(f.at()).arg(f.size());
	}

	f.close();

	return song;
}

bool ConvertGtp::save(QString)
{
	throw i18n("Not implemented");
}