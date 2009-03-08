#include "converttex.h"
#include "settings.h"

#include <kconfig.h>
#include <qfile.h>
#include <qtextstream.h>

//////////////////////////////////////////////////////////////////////
//
// MusiXTeX/kgtabs.tex export by alinx
//
// MusiXTeX is required to use exported files.
// Download it at ftp://ftp.dante.de/tex-archive/macros/musixtex/taupin
//             or http://www.gmd.de/Misc/Music
//

ConvertTex::ConvertTex(TabSong *song): ConvertBase(song)
{
	Settings::config->setGroup("MusiXTeX");
}

bool ConvertTex::save(QString fileName)
{
	bool success = FALSE;

	QFile f(fileName);
    if (!f.open(IO_WriteOnly))
		return FALSE;

	QTextStream s(&f);

	switch (Settings::texExportMode()) {
	case 0: success = saveToTab(s); break;
	case 1: success = saveToNotes(s); break;
	}

	f.close();
	return success;
}

bool ConvertTex::load(QString)
{
	// GREYFIX: todo loading from TEX tabs
	return FALSE;
}

bool ConvertTex::saveToTab(QTextStream &s)
{
	QString nn[MAX_STRINGS];
	QString tmp;
	bool flatnote;

	QString bar, notes, tsize, showstr;

	bar = "\\bar";
	bar += "\n";
	notes = "\\Notes";
	showstr = "\\showstrings";

	switch (Settings::texTabSize()){
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

	QListIterator<TabTrack> it(song->t);
	TabTrack *trk = it.current();

	// Stuff if globalShowStr=TRUE

	flatnote = FALSE;
	for (int i = 0; i < trk->string; i++) {
		nn[i] = Settings::noteName(trk->tune[i] % 12);
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
		return FALSE;
	}

	tmp += "}";
	tmp += "\n";

	if (trk->string < 4)
		tmp = "";

	for (int i = (trk->string - 1); i >= 0; i--){
		showstr += " ";
		showstr += Settings::noteName(trk->tune[i] % 12);
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
	s << "% $Id: converttex.cpp 882 2005-04-02 00:01:38Z greycat $" << "\n";
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

	if (!Settings::texShowPageNumber())
		s << "\\nopagenumbers" << "\n";

	s << "\\fulltitle{" << cleanString(song->info["TITLE"]) << "}";
	s << "\n";
	s << "\\subtitle{\\svtpoint\\bf Author: " << cleanString(song->info["AUTHOR"]) << "}" << "\n";
	s << "\\author{Transcribed by: " << cleanString(song->info["TRANSCRIBER"]);
	s << "\\\\%" << "\n";
	s << "        Tempo: " << song->tempo << "}";
	s << "\n";

	if (!Settings::texShowStr())
		s << tmp;

	s << "\\maketitle" << "\n";
	s << "\n";
	s << "\\settab1" << "\n";

	if (!Settings::texShowBarNumber())
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

		if (Settings::texShowStr() && (!flatnote))
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

			if ((bbar + 1) < (uint)trk->b.size()) { // looking for bars
				if ((uint)trk->b[bbar + 1].start == j)  bbar++;
			}
			if ((uint)trk->b[bbar].start == j)  s << bar;

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
					if (Settings::texShowStr() && (!flatnote))
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

	return TRUE;
}

bool ConvertTex::saveToNotes(QTextStream &s)
{ 
	return FALSE; //ALINXFIX: disabled

	QListIterator<TabTrack> it(song->t);

	// TeX-File INFO-HEADER
	s << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << "\n";
	s << "%" << "\n";
	s << "% This MusiXTex File was created with KGuitar " << VERSION << "\n";
    s << "% $Id: converttex.cpp 882 2005-04-02 00:01:38Z greycat $ " << "\n";
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
	if (!Settings::texShowPageNumber())
		s << "\\nopagenumbers" << "\n";
	if (!Settings::texShowBarNumber())
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

	return TRUE;
}

QString ConvertTex::cleanString(QString str)
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

QString ConvertTex::tab(bool chord, int string, int fret)
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

QString ConvertTex::getNote(QString note, int duration, bool dot)
{
	(void)duration;(void)dot;
	return "";
}
