#ifndef CHORDEDITOR_H
#define CHORDEDITOR_H

#include "config.h"

#include "global.h"
#include "fingering.h"

#include <qcombobox.h>
#include <qdialog.h>
//Added by qt3to4:
#include <QLabel>

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

#define STEPSIZE     40

class QLineEdit;
class QPushButton;
class QGroupBox;
class QRadioButton;
class QComboBox;
class QLabel;
class QListWidget;
class FingerList;
class TabTrack;

class ChordEditor: public QDialog {
	Q_OBJECT
public:
	ChordEditor(TabTrack *p, QWidget *parent = 0);
#ifdef WITH_TSE3
	ChordEditor(TSE3::MidiScheduler *_scheduler, TabTrack *p, QWidget *parent = 0);
#endif

	void initChordSelector(TabTrack *p);

	int  app(int x) { return fng->app(x); }
	void setApp(int x, int fret) { fng->setApp(x, fret); }
	int  scheme() { return strum_scheme; }

	Fingering *fng;
	QListWidget *chords;

public slots:
	void detectChord();
	void setStep3(int);
	void setHighSteps(int);
	void setStepsFromChord();
	void findSelection();
	void findChords();
	void askStrum();
	void playMidi();

	void analyzeChordName();
	void quickInsert();

private:
	bool calculateNotesFromSteps(int *, int &);

	TabTrack *parm;

	QLineEdit *chordName;
	QListWidget *tonic, *step3, *stephigh;
	QComboBox *st[7], *inv, *bassnote;
	QLabel *cnote[7];
        QGroupBox *complexity;
	QRadioButton *complexer[3];
	QPushButton *play;
	FingerList *fnglist;

	int strum_scheme;

#ifdef WITH_TSE3
	TSE3::MidiScheduler *scheduler;
#endif
};

#endif
