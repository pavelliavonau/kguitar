#include "config.h"

#include "chord.h"
#include "fingers.h"
#include "fingerlist.h"
#include "chordlist.h"
#include "tabsong.h"
#include "strumming.h"
#include "globaloptions.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qlabel.h>
#include <qlayout.h>

#ifdef WITH_TSE3
#include <tse3/Song.h>
#include <tse3/PhraseEdit.h>
#include <tse3/Part.h>
#include <tse3/Track.h>
#include <tse3/Metronome.h>
#include <tse3/MidiScheduler.h>
#include <tse3/Transport.h>
#endif

QString notes_us1[12] = {"C",  "C#", "D",  "D#", "E",  "F",
						 "F#", "G",	 "G#", "A",	 "A#", "B"};
QString notes_us2[12] = {"C",  "Db", "D",  "Eb", "E",  "F",
						 "Gb", "G",	 "Ab", "A",	 "Bb", "B"};
QString notes_us3[12] = {"C",  "Db", "D",  "Eb", "E",  "F",
						 "F#", "G",	 "G#", "A",	 "Bb", "B"};

QString notes_eu1[12] = {"C",  "C#", "D",  "D#", "E",  "F",
						 "F#", "G",	 "G#", "A",	 "A#", "H"};
QString notes_eu2[12] = {"C",  "Db", "D",  "Eb", "E",  "F",
						 "Gb", "G",	 "Ab", "A",	 "Hb", "H"};
QString notes_eu3[12] = {"C",  "Db", "D",  "Eb", "E",  "F",
						 "F#", "G",	 "G#", "A",	 "Hb", "H"};

QString notes_jz1[12] = {"C",  "C#", "D",  "D#", "E",  "F",
						 "F#", "G",	 "G#", "A",	 "B" , "H"};
QString notes_jz2[12] = {"C",  "Db", "D",  "Eb", "E",  "F",
						 "Gb", "G",	 "Ab", "A",	 "B" , "H"};
QString notes_jz3[12] = {"C",  "Db", "D",  "Eb", "E",  "F",
						 "F#", "G",	 "G#", "A",	 "B" , "H"};

QString note_name(int num)
{
	if ((num < 0) || (num > 11))
		return i18n("Unknown");

	switch (globalNoteNames) {
	case 0: return notes_us1[num];
	case 1: return notes_us2[num];
	case 2: return notes_us3[num];

	case 3: return notes_eu1[num];
	case 4: return notes_eu2[num];
	case 5: return notes_eu3[num];

	case 6: return notes_jz1[num];
	case 7: return notes_jz2[num];
	case 8: return notes_jz3[num];
	}

	return i18n("Unknown");
}

//					   3  5	 7	9  11 13
int stemplate[][6] = {{-1,2, 0, 0, 0, 0 },   // C
                      {-1,2, 2, 0, 0, 0 },   // C7
                      {-1,2, 3, 0, 0, 0 },   // C7M
                      {-1,2, 1, 0, 0, 0 },   // C6
                      {-1,2, 2, 2, 0, 0 },   // C9
                      {-1,2, 2, 2, 2, 0 },   // C11
                      {-1,2, 2, 2, 2, 2 },   // C13
                      {3, 3, 0, 0, 0, 0 },   // Caug
                      {2, 1, 1, 0, 0, 0 },   // Cdim
                      {0, 2, 0, 0, 0, 0 }};  // C5

QString maj7name[] = {"7M", "maj7", "dom7"};
QString flat[] = {"-", "b"};
QString sharp[] = {"+", "#"};

ChordSelector::ChordSelector(TabTrack *p, QWidget *parent = 0, const char *name = 0):
	QDialog(parent, name, TRUE)
{
	initChordSelector(p);
}

#ifdef WITH_TSE3
ChordSelector::ChordSelector(TSE3::MidiScheduler *_scheduler, TabTrack *p, QWidget *parent = 0,
							 const char *name = 0): QDialog(parent, name, TRUE)
{
	kdDebug() << k_funcinfo << endl;

	initChordSelector(p);
	scheduler = _scheduler;

	if (scheduler) {
		play->setEnabled(TRUE);
		kdDebug() << "   Found MidiScheduler" << endl;
	} else kdDebug() << "   No MidiScheduler found" << endl;
}
#endif

void ChordSelector::initChordSelector(TabTrack *p)
{
	parm = p;
	strum_scheme = 0;

	chname = new QLineEdit(this);
	chname->setMinimumHeight(20);

	// CHORD SELECTOR FOR FINDER WIDGETS

	tonic = new QListBox(this);
	for (int i = 0; i < 12; i++)
		tonic->insertItem(note_name(i));
//	tonic->setFixedVisibleLines(12);
	tonic->setMinimumWidth(40);
	connect(tonic, SIGNAL(highlighted(int)), SLOT(findChords()));

	bassnote = new QComboBox(FALSE, this);
	for (int i = 0; i < 12; i++)
		bassnote->insertItem(note_name(i));
	bassnote->setMinimumSize(40, 20);

	step3 = new QListBox(this);
	step3->insertItem("M");
	step3->insertItem("m");
	step3->insertItem("sus2");
	step3->insertItem("sus4");
//	step3->setFixedVisibleLines(4);
	step3->setMinimumWidth(40);
	connect(step3, SIGNAL(highlighted(int)), SLOT(setStep3()));

	stephigh = new QListBox(this);
	stephigh->insertItem("");
	stephigh->insertItem("7");
	stephigh->insertItem(maj7name[globalMaj7]);
	stephigh->insertItem("6");
	stephigh->insertItem("9");
	stephigh->insertItem("11");
	stephigh->insertItem("13");
	stephigh->insertItem("aug");
	stephigh->insertItem("dim");
	stephigh->insertItem("5");
//	stephigh->setFixedVisibleLines(10);
	stephigh->setMinimumWidth(40);
	connect(stephigh, SIGNAL(highlighted(int)), SLOT(setHighSteps()));

	// st array holds values for each step:
	// st[0] - 3'	 st[1] - 5'	   st[2] - 7'
	// st[3] - 9'	 st[4] - 11'   st[5] - 13'

	QLabel *stlabel[7];
	QString tmp;
	for (int i = 0; i < 7; i++) {
		tmp.setNum(i * 2 + 1);
		tmp = tmp + "\'";
		stlabel[i] = new QLabel(tmp, this);
		stlabel[i]->setAlignment(AlignCenter);

		cnote[i] = new QLabel(this);
		cnote[i]->setAlignment(AlignCenter);

		if (i > 0) {
			st[i - 1] = new QComboBox(FALSE, this);
			st[i - 1]->insertItem("x");
			if ((i == 2) || (i >= 4)) {
				st[i - 1]->insertItem(flat[globalFlatPlus]);
				st[i - 1]->insertItem("0");
				st[i - 1]->insertItem(sharp[globalFlatPlus]);
			}
			connect(st[i - 1], SIGNAL(activated(int)), SLOT(findSelection()));
			connect(st[i - 1], SIGNAL(activated(int)), SLOT(findChords()));
		}
	}

	st[0]->insertItem("2");
	st[0]->insertItem(flat[globalFlatPlus]);
	st[0]->insertItem("3");
	st[0]->insertItem("4");

	st[2]->insertItem("6");
	st[2]->insertItem(flat[globalFlatPlus]);
	st[2]->insertItem("7");

	inv = new QComboBox(FALSE, this);
	inv->insertItem(i18n("Root"));
	inv->insertItem(i18n("Inv #1"));
	inv->insertItem(i18n("Inv #2"));
	inv->insertItem(i18n("Inv #3"));
	inv->insertItem(i18n("Inv #4"));
	inv->insertItem(i18n("Inv #5"));
	inv->insertItem(i18n("Inv #6"));
	connect(inv, SIGNAL(activated(int)), SLOT(findChords()));

	complexity = new QButtonGroup(this);
	complexity->setMinimumSize(90, 70);
	complexer[0] = new QRadioButton(i18n("Usual"), complexity);
	complexer[0]->setGeometry(5, 5, 80, 20);
	complexer[1] = new QRadioButton(i18n("Rare"), complexity);
	complexer[1]->setGeometry(5, 25, 80, 20);
	complexer[2] = new QRadioButton(i18n("All"), complexity);
	complexer[2]->setGeometry(5, 45, 80, 20);
	complexity->setButton(0);
	connect(complexity, SIGNAL(clicked(int)), SLOT(findChords()));

	// CHORD ANALYZER

	fng = new Fingering(p, this);
	fng->move(230, 10);
	connect(fng, SIGNAL(chordChange()), SLOT(detectChord()));

	chords = new ChordList(this);
	chords->setMinimumWidth(120);
	connect(chords, SIGNAL(highlighted(int)), SLOT(setStepsFromChord()));

	// CHORD FINDER OUTPUT

	fnglist = new FingerList(p,this);
	connect(fnglist,SIGNAL(chordSelected(const int *)),
	        fng,SLOT(setFingering(const int *)));

	// DIALOG BUTTONS

	QPushButton *ok, *cancel, *strumbut;

	ok = new QPushButton(i18n("OK"), this);
	ok->setMinimumSize(75, 30);
	connect(ok, SIGNAL(clicked()), SLOT(accept()));

	cancel = new QPushButton(i18n("Cancel"), this);
	cancel->setMinimumSize(75, 30);
	connect(cancel, SIGNAL(clicked()), SLOT(reject()));

	strumbut = new QPushButton(i18n("&Strum..."), this);
	strumbut->setMinimumSize(75, 30);
	connect(strumbut, SIGNAL(clicked()), SLOT(askStrum()));

	play = new QPushButton(i18n("&Play"), this);
	play->setMinimumSize(75, 30);
	connect(play, SIGNAL(clicked()), SLOT(playMidi()));
    play->setEnabled(FALSE);

	// LAYOUT MANAGEMENT

	// Main layout
	QBoxLayout *l = new QHBoxLayout(this, 10);

	// Chord finding & analyzing layout
	QBoxLayout *lchord = new QVBoxLayout();
	l->addLayout(lchord, 1);

	// Chord editing layout
	QBoxLayout *lchedit = new QHBoxLayout();
	lchord->addWidget(chname);
	lchord->addLayout(lchedit);
	lchord->addWidget(fnglist, 1);

	// Chord selection (template-based) layout
	QGridLayout *lselect = new QGridLayout(3, 3, 5);
	lchedit->addLayout(lselect);

	lselect->addMultiCellWidget(tonic, 0, 2, 0, 0);
	lselect->addColSpacing(0, 40);

	lselect->addWidget(step3, 0, 1);
	lselect->addWidget(complexity, 1, 1);
	lselect->addWidget(inv, 2, 1);

	lselect->addMultiCellWidget(stephigh, 0, 1, 2, 2);
	lselect->addWidget(bassnote, 2, 2);

	// Chord icon showing layout
	QBoxLayout *lshow = new QVBoxLayout();
	lchedit->addLayout(lshow);

	// Analyzing and showing chord layout
	QBoxLayout *lanalyze = new QHBoxLayout();
	lshow->addLayout(lanalyze);
	lanalyze->addWidget(fng);
	lanalyze->addWidget(chords);

	// Steps editor layout
	QGridLayout *lsteps = new QGridLayout(3, 7, 0);
	lshow->addLayout(lsteps);

	lsteps->addWidget(stlabel[0], 0, 0);
	lsteps->addWidget(cnote[0], 2, 0);

	lsteps->addRowSpacing(0, 15);
	lsteps->addRowSpacing(1, 20);
	lsteps->addRowSpacing(2, 15);
	lsteps->setColStretch(0, 1);

	for (int i = 1; i < 7; i++) {
		lsteps->addWidget(stlabel[i], 0, i);
		lsteps->addWidget(st[i - 1], 1, i);
		lsteps->addWidget(cnote[i], 2, i);
		lsteps->setColStretch(i, 1);
	}

	// Strumming and buttons stuff layout
	QBoxLayout *lstrum = new QVBoxLayout();
	l->addLayout(lstrum);
	lstrum->addStretch(1);
	lstrum->addWidget(play);
	lstrum->addWidget(strumbut);
	lstrum->addWidget(ok);
	lstrum->addWidget(cancel);

	l->activate();

	setCaption(i18n("Chord constructor"));
}

void ChordSelector::askStrum()
{
	Strumming strum(strum_scheme);

	if (strum.exec())
		strum_scheme = strum.scheme();
}

void ChordSelector::playMidi()
{
#ifdef WITH_TSE3
	if (!scheduler)
		return;

	TSE3::PhraseEdit phraseEdit;
	TSE3::Clock time = 0;
	int duration = TSE3::Clock::PPQN;

	phraseEdit.insert(TSE3::MidiEvent(TSE3::MidiCommand(TSE3::MidiCommand_ProgramChange, 0,
														0 /*port*/, parm->patch), 0)); //ALINXFIX: port is an option

	int note;

	// play every note
	for (int i = 0; i < parm->string; i++)
		if (fng->app(i) != -1) {
			note = fng->app(i) + parm->tune[i];

			phraseEdit.insert(TSE3::MidiEvent(TSE3::MidiCommand(TSE3::MidiCommand_NoteOn, 0, 0/*port*/,
																note, 96), time, 96, time + duration));
			time += duration;
		}

	// play chord
	for (int i = 0; i < parm->string; i++)
		if (fng->app(i) != -1) {
			note = fng->app(i) + parm->tune[i];

			phraseEdit.insert(TSE3::MidiEvent(TSE3::MidiCommand(TSE3::MidiCommand_NoteOn, 0, 0/*port*/,
																note, 96), time, 96, time + duration * 3));
		}


	time += duration;
	phraseEdit.insert(TSE3::MidiEvent(TSE3::MidiCommand(TSE3::MidiCommand_NoteOn, 0, 0/*port*/,
														0, 0), time, 0, time + duration));


	TSE3::Song    m_song(1);
	TSE3::Phrase *phrase = phraseEdit.createPhrase(m_song.phraseList());
	TSE3::Part   *part   = new TSE3::Part(0, phraseEdit.lastClock());
	part->setPhrase(phrase);
	m_song[0]->insert(part);

	TSE3::Metronome metronome;
	TSE3::Transport transport(&metronome, scheduler);

    // Play and wait for the end
	transport.play(&m_song, 0);
	while (transport.status() != TSE3::Transport::Resting)
		transport.poll();

#endif
}

// Try to detect some chord forms from a given applicature.
void ChordSelector::detectChord()
{
	bool cn[12];
	int i, j, numnotes, noteok, bassiest = 255, bass;
	QString name;
	int s3, s5, s7, s9, s11, s13;

	for (i = 0; i < 12; i++)
		cn[i] = FALSE;
	numnotes=0; // number of different notes in a chord

	for (i = 0; i < parm->string; i++) {
		j = fng->app(i);
		if (j != -1) {
			j = (j + parm->tune[i]) % 12;
			if (!cn[j]) {
				cn[j] = TRUE;
				numnotes++;
			}
		}
	}

//	chords->setAutoUpdate(FALSE);
	chords->clearSelection();
	chords->clear();

	for (i = 0; i < 12; i++)  if (cn[i]) {

		// Initializing
		s3 = -1; s5 = -1; s7 = -1; s9 = -1; s11 = -1; s13 = -1;
		noteok = numnotes - 1;

		// Detecting thirds
		if (cn[(i + 4) % 12]) {
			s3 = 4; noteok--;			// Major
		} else if (cn[(i + 3) % 12]) {
			s3 = 3; noteok--;			// Minor
		} else if (cn[(i + 5) % 12]) {
			s3 = 5; noteok--;			// Sus4
		} else if (cn[(i + 2) % 12]) {
			s3 = 2; noteok--;			// Sus2
		}

		// Detecting fifths
		if (cn[(i + 7) % 12]) {
			s5 = 7; noteok--;			// 5
		} else if (cn[(i+6) % 12]) {
			s5 = 6; noteok--;			// 5-
		} else if (cn[(i+8) % 12]) {
			s5 = 8; noteok--;			// 5+
		}

		// Detecting sevenths
		if (cn[(i + 10) % 12]) {
			s7 = 10;noteok--;			// 7
		} else if (cn[(i + 11) % 12]) {
			s7 = 11;noteok--;			// 7M
		} else if (cn[(i + 9) % 12]) {
			s7 = 9;noteok--;			// 6
		}

		// Detecting 9ths
		if ((cn[(i + 2) % 12]) && (s3 != 2)) {
			s9 = 2;noteok--;			// 9
		} else if ((cn[(i + 3) % 12]) && (s3 != 3)) {
			s9 = 3;noteok--;			// 9+
		} else if (cn[(i + 1) % 12]) {
			s9 = 1;noteok--;			// 9-
		}

		// Detecting 11ths
		if ((cn[(i+5)%12]) && (s3!=5)) {
			s11=5;noteok--;				  // 11
		} else if ((cn[(i+4)%12]) && (s3!=4)) {
			s11=4;noteok--;				  // 11-
		} else if ((cn[(i+6)%12]) && (s5!=6)) {
			s11=6;noteok--;				  // 11+
		}

		// Detecting 13ths
		if ((cn[(i+9)%12]) && (s7!=9)) {
			s13=9;noteok--;
		} else if ((cn[(i+8)%12]) && (s5!=8)) {
			s13=8;noteok--;
		} else if ((cn[(i+10)%12]) && (s7!=10)) {
			s13=10;noteok--;
		}

		if (noteok == 0) {
			ChordListItem *item = new ChordListItem(i, bass, s3, s5,
			                                        s7, s9, s11, s13);
			chords->inSort(item);
		}
	}

//	chords->setAutoUpdate(TRUE);
	chords->repaint();
}

void ChordSelector::setStep3()
{
	switch (step3->currentItem()) {
	case 0: st[0]->setCurrentItem(3); break;				// Major
	case 1: st[0]->setCurrentItem(2); break;				// Minor
	case 2: st[0]->setCurrentItem(1); break;				// Sus2
	case 3: st[0]->setCurrentItem(4); break;				// Sus4
	}

	findSelection();
	findChords();
}

void ChordSelector::setStepsFromChord()
{
	ChordListItem *it = chords->currentItemPointer();

	tonic->setCurrentItem(it->tonic());
	for (int i = 0; i < 6; i++)
		st[i]->setCurrentItem(it->step(i));

	findSelection();
	findChords();
}

void ChordSelector::setHighSteps()
{
	int j = stephigh->currentItem();

	if (j == -1)
		return;

	for (int i = 0; i < 6; i++)
		if (stemplate[j][i] != -1)
			st[i]->setCurrentItem(stemplate[j][i]);

	findSelection();
	findChords();
}

void ChordSelector::findSelection()
{
	bool ok = TRUE;

	switch (st[0]->currentItem()) {
	case 0: step3->clearSelection(); break;					// no3
	case 1: step3->setCurrentItem(2); break;				// Sus2
	case 2: step3->setCurrentItem(1); break;				// Minor
	case 3: step3->setCurrentItem(0); break;				// Major
	case 4: step3->setCurrentItem(3); break;				// Sus4
	}

	for (uint j = stephigh->count() - 1; j > 0; j--) {
		ok = TRUE;
		for (int i = 0; i < 6; i++) {
			if ((stemplate[j][i] != -1) &&
				(stemplate[j][i] != st[i]->currentItem())) {
				ok = FALSE;
				break;
			}
		}
		if (ok) {
			stephigh->setCurrentItem(j);
			break;
		}
	}
	if (!ok)
		stephigh->clearSelection();
}

void ChordSelector::findChords()
{
	int i, j, k = 0, min, max, bass = 0, muted = 0;
	int app[MAX_STRINGS];				// raw fingering itself
	int ind[MAX_STRINGS];				// indexes in hfret array

	//				    1  5  7   9  11 13
	int toneshift[6] = {0, 7, 10, 2, 5, 9};

	int fb[MAX_STRINGS][MAX_FRETS];	// array with an either -1 or number of note from a chord

	int hfret[MAX_STRINGS][MAX_FRETS];// numbers of frets to hold on every string
	int hnote[MAX_STRINGS][MAX_FRETS];// numbers of notes in a chord that make ^^

	bool needrecalc;					// needs recalculate max/min

	// CALCULATION OF REQUIRED NOTES FOR A CHORD FROM USER STEP INPUT

	int need[7],got[7];

	int t = tonic->currentItem();

	if (t == -1)						// no calculations without tonic
		return;

	int notenum = 1;
	need[0] = t;
	cnote[0]->setText(note_name(t));

	switch (st[0]->currentItem()) {
	case 1: need[1] = (t + 2) % 12; notenum++; break;	  // 2
	case 2: need[1] = (t + 3) % 12; notenum++; break;	  // 3-
	case 3: need[1] = (t + 4) % 12; notenum++; break;	  // 3+
	case 4: need[1] = (t + 5) % 12; notenum++; break;	  // 4
	}

	if (st[0]->currentItem()!=0) {
		cnote[1]->setText(note_name(need[1]));
	} else {
		cnote[1]->clear();
	}

	for (i = 1; i < 6; i++) {
		j = st[i]->currentItem();
		if (j) {
			need[notenum] = (t + toneshift[i] + (j - 2)) % 12;
			cnote[i + 1]->setText(note_name(need[notenum]));
			notenum++;
		} else {
			cnote[i + 1]->clear();
		}
	}

	// BEGIN THE CHORD FILLING SESSION
	fnglist->beginSession();

	// CHECKING IF NOTE NUMBER GREATER THAT AVAILABLE STRINGS

	// Ex: it's impossible to play 13th chords on 6 strings, but it's
	//	   possible on 7 string guitar. This way we optimize things a bit

	if (parm->string<notenum) {
		fnglist->endSession();
		return;
	}

	// CHECKING THE INVERSION NUMBER RANGE

	if (inv->currentItem()>=notenum)
		inv->setCurrentItem(0);

	int span = 3; // maximal fingerspan

	if (complexer[1]->isChecked())
		span = 4;
	if (complexer[2]->isChecked())
		span = 5;

	// PREPARING FOR FINGERING CALCULATION

	for (i = 0; i < parm->string; i++) {
		for (j = 0; j <= parm->frets; j++)
			fb[i][j] = -1;
		for (k=0;k<notenum;k++) {
			j=(need[k]-parm->tune[i]%12+12)%12;
			while (j<=parm->frets) {
				fb[i][j]=k;
				j+=12;
			}
		}
	}

	for (i = 0; i < parm->string; i++) {
		k=1;
		hfret[i][0] = -1;
		hnote[i][0] = -2;
		for (j = 0; j <= parm->frets; j++)
			if (fb[i][j] != -1) {
				hfret[i][k] = j;
				hnote[i][k] = fb[i][j];
				k++;
			}
		hnote[i][k] = -1;
	}

	// After all the previous funky calculations, we would have 2 arrays:
	// hfret[string][index] with numbers of frets where we can hold the string,
	//						(any other fret would make a chord unacceptable)
	// hnote[string][index] with numbers of notes in the chord that correspond
	//						to each hfret array's fret. -1 means end of string,
	//						-2 means muted string.

	for (i = 0; i < parm->string; i++)
		ind[i] = 0;

	min = -1; max = -1; needrecalc = FALSE;

	// MAIN FINGERING CALCULATION LOOP

	i = 0;
	do {
		// end of string not reached
		if (!( (hnote[i][ind[i]]==-1) || ( (!needrecalc) && (max-min>=span)))) {
			if (needrecalc) {
				min=parm->frets+1;max=0;
				for (j=0;j<parm->string;j++) {
					if (hfret[j][ind[j]]>0) {
						if (hfret[j][ind[j]]<min)  min=hfret[j][ind[j]];
						if (hfret[j][ind[j]]>max)  max=hfret[j][ind[j]];
					}
					if (max-min>=span)
						break;
				}
			}
			if (max-min<span) {
				for (k=0;k<notenum;k++)
					got[k]=0;
				k=0;bass=255;muted=0;
				for (j=0;j<parm->string;j++) {
					if (hfret[j][ind[j]]>=0) {
						if (parm->tune[j]+hfret[j][ind[j]]<bass)
							bass=parm->tune[j]+hfret[j][ind[j]];
						if (!got[hnote[j][ind[j]]]) {
							got[hnote[j][ind[j]]]=1;
							k++;
						}
					} else {
						muted++;
					}
				}
			}

			if ((k==notenum) && (max-min<span) && (bass%12==need[inv->currentItem()])) {
				for (j=0;j<parm->string;j++)
					app[j]=hfret[j][ind[j]];
				if (complexer[0]->isChecked()) {
					if ((muted==0) ||										// No muted strings
						((muted==1) && (app[0]==-1)) ||						// Last string muted
						((muted==2) && (app[0]==-1) && (app[1]==-1))) {		// Last and pre-last muted
						fnglist->addFingering(app);
					}
				} else {
					fnglist->addFingering(app);
				}
			}

			i=0;
		} else {						// end of string reached
			ind[i]=0;i++;
			needrecalc=TRUE;
			if (i>=parm->string)
				break;
		}

		if (hfret[i][ind[i]]>min) {
			ind[i]++;
			if (hfret[i][ind[i]]>max)
				max = hfret[i][ind[i]];
			needrecalc=FALSE;
		} else {
			ind[i]++;
			needrecalc=TRUE;
		}
	} while (TRUE);

	fnglist->endSession();
}

