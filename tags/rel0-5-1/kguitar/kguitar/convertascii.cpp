#include "convertascii.h"
#include "settings.h"

#include <kconfig.h>
#include <qfile.h>
#include <qtextstream.h>

ConvertAscii::ConvertAscii(TabSong *song): ConvertBase(song)
{
	Settings::config->setGroup("ASCII");
	durMode = Settings::config->readNumEntry("DurationDisplay", 3);
	pageWidth = Settings::config->readNumEntry("PageWidth", 72);

	// Clever expression to determine minimal duration to put one
	// blank for (i.e. we put only one blank for this duration and any
	// durations less than this).
	oneBlankDuration = (durMode > 0) ? (120 >> (durMode - 1)) : 0;
}

bool ConvertAscii::save(QString fileName)
{
	// Initialize output stream
	QFile f(fileName);
	if (!f.open(IO_WriteOnly))
		return FALSE;
	QTextStream s(&f);
	stream = &s;

	// Print out header
	writeHeader();

	// Print out track data
	QListIterator<TabTrack> it(song->t);
	for (int n = 1; it.current(); ++it) {
		TabTrack *trk = it.current();
		writeTrack(trk, n);
		n++; // Numerical track counter
	}

	f.close();

	return TRUE;
}

bool ConvertAscii::load(QString)
{
	// GREYFIX: todo loading from ASCII tabs
	return FALSE;
}

void ConvertAscii::writeHeader()
{
	writeCentered(song->info["TITLE"]);
	(*stream) << endl;
	writeCentered("Author: " + song->info["ARTIST"]);
	writeCentered("Transcribed by: " + song->info["TRANSCRIBER"]);
	// GREYFIX - comments?
	(*stream) << "Tempo: " << song->tempo << endl << endl;
}

void ConvertAscii::writeTrack(TabTrack *trk, int n)
{
	QString tmp;

	startTrack(trk, n);
	startRow(trk);

	uint bar = 0;

	for (uint x = 0; x < trk->c.size(); x++) {

		// If this bar's not last
		if (bar + 1 < trk->b.size()) {
			if ((uint) trk->b[bar+1].start == x) {
				// Time for next bar
				bar++;
				flushBar(trk);
			}
		}

		addColumn(trk, &(trk->c[x]));
	}

	flushBar(trk);
	flushRow(trk);
}

void ConvertAscii::startTrack(TabTrack *trk, int n)
{
	(*stream) << "Track " << n << ": " << trk->name << endl << endl;
	// GREYFIX - channel, bank, patch, string, frets data

	minstart = 1;
	for (int i = 0; i < trk->string; i++)
		if (Settings::noteName(trk->tune[i] % 12).length() > 1)
			minstart = 2;
}

void ConvertAscii::startRow(TabTrack *trk)
{
	for (int i = 0; i < trk->string; i++) {
		if (trk->trackMode() == TabTrack::FretTab) {
			row[i] = Settings::noteName(trk->tune[i] % 12);
			while (row[i].length() < minstart)
				row[i] += ' ';
		} else {
			row[i] = drum_abbr[trk->tune[i]];
		}
		row[i] += "|-";
	}
	rowBars = 0;
}

void ConvertAscii::writeCentered(QString l)
{
	for (int i = 0; i < (pageWidth-(int) l.length()) / 2; i++)
		(*stream) << ' ';
	(*stream) << l << endl;
}

void ConvertAscii::addColumn(TabTrack *trk, TabColumn *col)
{
	bool lng = FALSE;

	// Check if column contains any 'long' (2-digit) values
	if (trk->trackMode() == TabTrack::DrumTab) {
		for (int i = 0; i < trk->string; i++)
			if (col->a[i] >= 10)
				lng = TRUE;
	}

	// Determine spaces for duration
	int spaces = col->l / oneBlankDuration;
	if (spaces < 1)  spaces = 1;

	// Render column
	for (int i = 0; i < trk->string; i++) {

		// Digits
		switch (col->a[i]) {
		case NULL_NOTE:
			bar[i] += lng ? "--" : "-";
			break;
		case DEAD_NOTE:
			bar[i] += lng ? "-X" : "X";
			break;
		default:
			if (trk->trackMode() == TabTrack::DrumTab) {
				bar[i] += "o";
			} else {
				if ((lng) && (col->a[i] < 10))
					bar[i] += '-';
				bar[i] += QString::number(col->a[i]);
			}
			break;
		}

		// Space for duration
		for (int j = 0; j < spaces; j++)
			bar[i] += '-';
	}
}

void ConvertAscii::flushBar(TabTrack *trk)
{
	// Close bar with vertical pipe symbol
	for (int i = 0; i < trk->string; i++)
		bar[i] += '|';

	// If we won't overfill page width or if we have no bars yet, add
	// bar[] to row[]
	if (rowBars == 0 || (row[0].length() + bar[0].length() <= pageWidth)) {
		for (int i = 0; i < trk->string; i++) {
			row[i] += bar[i];
			bar[i] = "";
		}
		rowBars++;
	}

	// If we have to flush row, do it
	if (row[0].length() + bar[0].length() >= pageWidth) {
		flushRow(trk);
		startRow(trk);
	}

	// If we still have bar to flush, do it now
	if (bar[0].length() > 0) {
		for (int i = 0; i < trk->string; i++) {
			row[i] += bar[i];
			bar[i] = "";
		}
		rowBars++;
	}
}

void ConvertAscii::flushRow(TabTrack *trk)
{
	if (rowBars > 0) {
		for (int i = trk->string - 1; i >= 0; i--)
			(*stream) << row[i] << endl;

		(*stream) << endl;
	}
}
