#include "config.h"

#include "chordeditor.h"
#include "fingering.h"
#include "fingerlist.h"
#include "settings.h"
#include "strumming.h"
#include "settings.h"
#include "chordanalyzer.h"
#include "data/tabtrack.h"
#include "chordlistitem.h"

#include <KLocalizedString>
#include <kmessagebox.h>

#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qkeysequence.h>
#include <QGroupBox>
#include <QListWidget>

#ifdef WITH_TSE3
#include <tse3/Song.h>
#include <tse3/PhraseEdit.h>
#include <tse3/Part.h>
#include <tse3/Track.h>
#include <tse3/Metronome.h>
#include <tse3/MidiScheduler.h>
#include <tse3/Transport.h>
#include <QApplication>
#endif

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

ChordEditor::ChordEditor(TabTrack *p, QWidget *parent)
	: QDialog(parent)
{
	setModal(true);
	initChordSelector(p);
}

#ifdef WITH_TSE3
ChordEditor::ChordEditor(TSE3::MidiScheduler *_scheduler, TabTrack *p, QWidget *parent)
	: QDialog(parent)
{
	setModal(true);
	qDebug() << Q_FUNC_INFO;

	initChordSelector(p);
	scheduler = _scheduler;

	if (scheduler) {
		play->setEnabled(TRUE);
		qDebug() << "   Found MidiScheduler";
	} else {
		qDebug() << "   No MidiScheduler found";
	}
}
#endif

void ChordEditor::initChordSelector(TabTrack *p)
{
	parm = p;
	strum_scheme = 0;

	// CHORD NAMING ANALYZER

	chordName = new QLineEdit(this);

	// CHORD SELECTOR FOR FINDER WIDGETS

	tonic = new QListWidget(this);
	for (int i = 0; i < 12; i++)
		tonic->addItem(Settings::noteName(i));
//     tonic->setHScrollBarMode(QScrollView::AlwaysOff);
//     tonic->setVScrollBarMode(QScrollView::AlwaysOff);
// 	tonic->setRowMode(12);
//	tonic->setMinimumHeight(tonic->item(0) * 12 + 4);
// 	tonic->setMinimumWidth(tonic->maxItemWidth());
	connect(tonic, SIGNAL(currentRowChanged(int)), SLOT(findChords()));

	bassnote = new QComboBox(this);
	for (int i = 0; i < 12; i++)
		bassnote->addItem(Settings::noteName(i));

	step3 = new QListWidget(this);
	step3->addItem("M");
	step3->addItem("m");
	step3->addItem("sus2");
	step3->addItem("sus4");
//	step3->setFixedVisibleLines(4);
	connect(step3, SIGNAL(currentRowChanged(int)), SLOT(setStep3(int)));

	stephigh = new QListWidget(this);
	stephigh->addItem("");
	stephigh->addItem("7");
	stephigh->addItem(Settings::maj7Name());
	stephigh->addItem("6");
	stephigh->addItem("9");
	stephigh->addItem("11");
	stephigh->addItem("13");
	stephigh->addItem("aug");
	stephigh->addItem("dim");
	stephigh->addItem("5");
//	stephigh->setFixedVisibleLines(10);
//	stephigh->setMinimumWidth(40);
	connect(stephigh, SIGNAL(currentRowChanged(int)), SLOT(setHighSteps(int)));

	// st array holds values for each step:
    // st[1] - 1'
    // st[1] - 3'
    // st[2] - 5'
    // st[3] - 7'
	// st[4] - 9'
    // st[5] - 11'
    // st[6] - 13'

	QLabel *stlabel[7];
	QString tmp;
	for (int i = 0; i < 7; i++) {
		tmp.setNum(i * 2 + 1);
		tmp = tmp + "\'";
		stlabel[i] = new QLabel(tmp, this);
		stlabel[i]->setAlignment(Qt::AlignCenter);

		cnote[i] = new QLabel(this);
		cnote[i]->setAlignment(Qt::AlignCenter);

		st[i] = new QComboBox(this);
		if (i > 0)
			st[i]->addItem("x");
		if ((i == 2) || (i >= 4)) {
			st[i]->addItem(Settings::flatName());
			st[i]->addItem("0");
			st[i]->addItem(Settings::sharpName());
		}
		if (i > 0)  {
			connect(st[i], SIGNAL(activated(int)), SLOT(findSelection()));
			connect(st[i], SIGNAL(activated(int)), SLOT(findChords()));
		} else {
			st[i]->addItem("0");
			st[i]->setEnabled(FALSE);
		}
	}

	st[1]->addItem("2");
	st[1]->addItem(Settings::flatName());
	st[1]->addItem("3");
	st[1]->addItem("4");

	st[3]->addItem("6");
	st[3]->addItem(Settings::flatName());
	st[3]->addItem("7");

	inv = new QComboBox(this);
	inv->addItem(i18n("Root"));
	inv->addItem(i18n("Inv #1"));
	inv->addItem(i18n("Inv #2"));
	inv->addItem(i18n("Inv #3"));
	inv->addItem(i18n("Inv #4"));
	inv->addItem(i18n("Inv #5"));
	inv->addItem(i18n("Inv #6"));
	connect(inv, SIGNAL(activated(int)), SLOT(findChords()));

	complexity = new QGroupBox(this);
	complexer[0] = new QRadioButton(i18n("Usual"), complexity);
	complexer[1] = new QRadioButton(i18n("Rare"), complexity);
	complexer[2] = new QRadioButton(i18n("All"), complexity);
	complexer[0]->setChecked(true);
	connect(complexity, &QGroupBox::clicked, this, &ChordEditor::findChords);

	// CHORD ANALYZER

	fng = new Fingering(p, this);
	connect(fng, SIGNAL(chordChange()), SLOT(detectChord()));

	chords = new QListWidget(this);
	chords->setSortingEnabled(true);
	connect(chords, SIGNAL(itemSelectionChanged()), SLOT(setStepsFromChord()));

	// CHORD FINDER OUTPUT

	fnglist = new FingerList(p, this);
	connect(fnglist, SIGNAL(chordSelected(const int *)),
	        fng, SLOT(setFingering(const int *)));

	// DIALOG BUTTONS

	QPushButton *ok, *cancel, *strumbut, *chordNameAnalyze, *chordNameQuickInsert;

	chordNameAnalyze = new QPushButton(i18n("&Analyze"), this);
	connect(chordNameAnalyze, SIGNAL(clicked()), SLOT(analyzeChordName()));
	chordNameAnalyze->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Return));

	chordNameQuickInsert = new QPushButton(i18n("&Quick Insert"), this);
	connect(chordNameQuickInsert, SIGNAL(clicked()), SLOT(quickInsert()));
	chordNameQuickInsert->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));

	strumbut = new QPushButton(i18n("&Strum..."), this);
	connect(strumbut, SIGNAL(clicked()), SLOT(askStrum()));

	play = new QPushButton(i18n("&Play"), this);
	connect(play, SIGNAL(clicked()), SLOT(playMidi()));
    play->setEnabled(FALSE);

	ok = new QPushButton(i18n("OK"), this);
	connect(ok, SIGNAL(clicked()), SLOT(accept()));
	ok->setDefault(TRUE);

	cancel = new QPushButton(i18n("Cancel"), this);
	connect(cancel, SIGNAL(clicked()), SLOT(reject()));

	// LAYOUT MANAGEMENT

	// Main layout
	QBoxLayout *l = new QHBoxLayout(this);

	// Chord finding & analyzing layout
	QBoxLayout *lchord = new QVBoxLayout();
	l->addLayout(lchord, 1);

	// Chord editing layout
	QBoxLayout *lchedit = new QHBoxLayout();
	lchord->addWidget(chordName);
	lchord->addLayout(lchedit);
	lchord->addWidget(fnglist, 1);

	// Chord selection (template-based) layout
	QGridLayout *lselect = new QGridLayout();
	lchedit->addLayout(lselect);

	lselect->addWidget(tonic, 0, 0, 3, 1);

	lselect->addWidget(step3, 0, 1);
	lselect->addWidget(inv, 2, 1);

	lselect->addWidget(stephigh, 0, 2, 2, 1);
	lselect->addWidget(bassnote, 2, 2);

	// Complexity selection
	QBoxLayout *lcomplexity = new QVBoxLayout();
	for (int i = 0; i < 3; i++)
		lcomplexity->addWidget(complexer[i]);
	complexity->setLayout(lcomplexity);
	lselect->addWidget(complexity, 1, 1);

	// Chord icon showing layout
	QBoxLayout *lshow = new QVBoxLayout();
	lchedit->addLayout(lshow);

	// Analyzing and showing chord layout
	QBoxLayout *lanalyze = new QHBoxLayout();
	lshow->addLayout(lanalyze);
	lanalyze->addWidget(fng);
	lanalyze->addWidget(chords);

	// Steps editor layout
	QGridLayout *lsteps = new QGridLayout();
	lshow->addLayout(lsteps);

	lsteps->setColumnStretch(0, 1);

	for (int i = 0; i < 7; i++) {
		lsteps->addWidget(stlabel[i], 0, i);
		lsteps->addWidget(st[i], 1, i);
		lsteps->addWidget(cnote[i], 2, i);
		lsteps->setColumnStretch(i, 1);
	}

	// Strumming and buttons stuff layout
	QBoxLayout *lstrum = new QVBoxLayout();
	l->addLayout(lstrum);
    lstrum->addWidget(chordNameAnalyze);
    lstrum->addWidget(chordNameQuickInsert);
	lstrum->addStretch(1);
	lstrum->addWidget(play);
	lstrum->addWidget(strumbut);
	lstrum->addWidget(ok);
	lstrum->addWidget(cancel);

	l->activate();

	setWindowTitle(i18n("Chord Constructor"));
}

void ChordEditor::askStrum()
{
	Strumming strum(strum_scheme);

	if (strum.exec())
		strum_scheme = strum.scheme();
}

void ChordEditor::playMidi()
{
#ifdef WITH_TSE3
	if (!scheduler)
		return;

	TSE3::PhraseEdit phraseEdit;
	TSE3::Clock time = 0;
	int duration = TSE3::Clock::PPQN;

	phraseEdit.insert(
		TSE3::MidiEvent(
			TSE3::MidiCommand(
				TSE3::MidiCommand_ProgramChange,
				0, Settings::midiPort(), parm->patch),
			0)
		);

	int note;

	// play every note
	for (int i = 0; i < parm->string; i++)
		if (fng->app(i) != -1) {
			note = fng->app(i) + parm->tune[i];

			phraseEdit.insert(
				TSE3::MidiEvent(
					TSE3::MidiCommand(
						TSE3::MidiCommand_NoteOn,
						0, Settings::midiPort(), note, 96),
					time, 96, time + duration)
				);
			time += duration;
		}

	// play chord
	for (int i = 0; i < parm->string; i++)
		if (fng->app(i) != -1) {
			note = fng->app(i) + parm->tune[i];

			phraseEdit.insert(
				TSE3::MidiEvent(
					TSE3::MidiCommand(
						TSE3::MidiCommand_NoteOn,
						0, Settings::midiPort(), note, 96),
					time, 96, time + duration * 3)
				);
		}


	time += duration;
	phraseEdit.insert(
		TSE3::MidiEvent(
			TSE3::MidiCommand(
				TSE3::MidiCommand_NoteOn,
				0, Settings::midiPort(), 0, 0),
			time, 0, time + duration)
		);

	TSE3::Song   tsong(1);
	TSE3::Phrase *phrase = phraseEdit.createPhrase(tsong.phraseList());
	TSE3::Part   *part   = new TSE3::Part(0, phraseEdit.lastClock());
	part->setPhrase(phrase);
	tsong[0]->insert(part);

	TSE3::Metronome metronome;
	TSE3::Transport transport(&metronome, scheduler);

    // Play and wait for the end
	transport.play(&tsong, 0);
	do {
		qApp->processEvents();
		transport.poll();
	} while (transport.status() != TSE3::Transport::Resting);
#endif
}

// Try to detect some chord forms from a given applicature.
void ChordEditor::detectChord()
{
	bool cn[12];
	int i, j, numnotes, noteok, /*bassiest = 255,*/ bass = 0;
	//QString name;
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

		if (noteok == 0)
			chords->addItem(new ChordListItem(
				i, bass, s3, s5, s7, s9, s11, s13
			));
	}

//	chords->setAutoUpdate(TRUE);
	chords->sortItems();
	chords->repaint();
}

void ChordEditor::setStep3(int n)
{
	switch (n) {
	case 0: st[1]->setCurrentIndex(3); break;				// Major
	case 1: st[1]->setCurrentIndex(2); break;				// Minor
	case 2: st[1]->setCurrentIndex(1); break;				// Sus2
	case 3: st[1]->setCurrentIndex(4); break;				// Sus4
	}

	findSelection();
	findChords();
}

void ChordEditor::setStepsFromChord()
{
	ChordListItem *it = (ChordListItem *) chords->currentItem();

	tonic->setCurrentRow(it->tonic());
	for (int i = 0; i < 6; i++)
		st[i + 1]->setCurrentIndex(it->step(i));

	findSelection();
	findChords();
}

void ChordEditor::setHighSteps(int j)
{
	if (j == -1)
		return;

	for (int i = 0; i < 6; i++)
		if (stemplate[j][i] != -1)
			st[i + 1]->setCurrentIndex(stemplate[j][i]);

	findSelection();
	findChords();
}

// Analyses st[] combobox array and find out steps templating
// listboxes selections from it
void ChordEditor::findSelection()
{
	bool ok = TRUE;

	switch (st[1]->currentIndex()) {
	case 0: step3->clearSelection(); break;           // no3
	case 1: step3->setCurrentRow(2); break;           // Sus2
	case 2: step3->setCurrentRow(1); break;           // Minor
	case 3: step3->setCurrentRow(0); break;           // Major
	case 4: step3->setCurrentRow(3); break;           // Sus4
	}

	for (int j = stephigh->count() - 1; j >= 0; j--) {
		ok = TRUE;
		for (int i = 0; i < 6; i++) {
			if ((stemplate[j][i] != -1) &&
				(stemplate[j][i] != st[i + 1]->currentIndex())) {
				ok = FALSE;
				break;
			}
		}
		if (ok) {
			stephigh->setCurrentRow(j);
			break;
		}
	}
	if (!ok)
		stephigh->clearSelection();
}

// Calculate absolute notes from tonic + step user input
bool ChordEditor::calculateNotesFromSteps(int need[], int &notenum)
{
	//                  1  5  7   9  11 13
	int toneshift[6] = {0, 7, 10, 2, 5, 9};

	int t = tonic->currentRow();

	if (t == -1)                        // no calculations without tonic
		return FALSE;

	notenum = 1;
	need[0] = t;
	cnote[0]->setText(Settings::noteName(t));

	switch (st[1]->currentIndex()) {
	case 1: need[1] = (t + 2) % 12; notenum++; break; // 2
	case 2: need[1] = (t + 3) % 12; notenum++; break; // 3-
	case 3: need[1] = (t + 4) % 12; notenum++; break; // 3+
	case 4: need[1] = (t + 5) % 12; notenum++; break; // 4
	}

	if (st[1]->currentIndex()!=0) {
		cnote[1]->setText(Settings::noteName(need[1]));
	} else {
		cnote[1]->clear();
	}

	for (int i = 1; i < 6; i++) {
		int j = st[i + 1]->currentIndex();
		if (j) {
			need[notenum] = (t + toneshift[i] + (j - 2)) % 12;
			cnote[i + 1]->setText(Settings::noteName(need[notenum]));
			notenum++;
		} else {
			cnote[i + 1]->clear();
		}
	}

    return TRUE;
}

// Most complex and longest method that does the real calculation of
// chords. Translates steps settings input to the list of chords and
// adds them to displayer.
void ChordEditor::findChords()
{
	int i, j, k = 0, min, max, bass = 0, muted = 0;
	int app[MAX_STRINGS];				// raw fingering itself
	int ind[MAX_STRINGS];				// indexes in hfret array

	int fb[MAX_STRINGS][MAX_FRETS];	// array with an either -1 or number of note from a chord

	int hfret[MAX_STRINGS][MAX_FRETS];// numbers of frets to hold on every string
	int hnote[MAX_STRINGS][MAX_FRETS];// numbers of notes in a chord that make ^^

	bool needrecalc;                    // needs recalculate max/min

	// CALCULATION OF REQUIRED NOTES FOR A CHORD FROM USER STEP INPUT
	int need[7], got[7];
	int notenum;

	if (!calculateNotesFromSteps(need, notenum))
		return;

	// BEGIN THE CHORD FILLING SESSION
	fnglist->beginSession();

	// CHECKING IF NOTE NUMBER GREATER THAT AVAILABLE STRINGS

	// Ex: it's impossible to play 13th chords on 6 strings, but it's
	//	   possible on 7 string guitar. This way we optimize things a bit

	if (parm->string < notenum) {
		fnglist->endSession();
		return;
	}

	// CHECKING THE INVERSION NUMBER RANGE

	if (inv->currentIndex() >= notenum)
		inv->setCurrentIndex(0);

	int span = 3; // maximal fingerspan

	if (complexer[1]->isChecked())
		span = 4;
	if (complexer[2]->isChecked())
		span = 5;

	// PREPARING FOR FINGERING CALCULATION

	for (i = 0; i < parm->string; i++) {
		for (j = 0; j <= parm->frets; j++)
			fb[i][j] = -1;
		for (k = 0; k < notenum; k++) {
			j = (need[k] - parm->tune[i] % 12 + 12) % 12;
			while (j <= parm->frets) {
				fb[i][j] = k;
				j += 12;
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

			if ((k==notenum) && (max-min<span) && (bass%12==need[inv->currentIndex()])) {
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

// Try to decipher text-written chord name and set steps accordingly
void ChordEditor::analyzeChordName()
{
	ChordAnalyzer ca(chordName->text());
	if (ca.analyze()) {
		tonic->setCurrentRow(ca.tonic);
		for (int i = 0; i < 6; i++)
			st[i + 1]->setCurrentIndex(ca.step[i]);
		findSelection();
		findChords();
	} else {
		KMessageBox::error(this, ca.msg, i18n("Unable to understand chord name"));
	}
}

// Analyze chord by text-written name, automatically select best one
// and insert it into track
void ChordEditor::quickInsert()
{
	analyzeChordName();
	if (fnglist->count() > 0)  {
		fnglist->selectFirst();
		accept();
	}
}
