#ifndef CHORD_H
#define CHORD_H

#include "config.h"

#include "global.h"
#include "fingers.h"

#include <qcombobox.h>
#include <qdialog.h>

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

#define STEPSIZE     40

class QLineEdit;
class QListBox;
class QPushButton;
class QButtonGroup;
class QRadioButton;
class QComboBox;
class QLabel;
class ChordList;
class FingerList;
class TabTrack;
class Strumming;


class ChordSelector: public QDialog {
    Q_OBJECT
public:
	ChordSelector(TabTrack *p, QWidget *parent = 0, const char *name = 0);
#ifdef WITH_TSE3
    ChordSelector(TSE3::MidiScheduler *_scheduler, TabTrack *p, QWidget *parent = 0,
				  const char *name = 0);
#endif

	void initChordSelector(TabTrack *p);

	int  app(int x) { return fng->app(x); }
    void setApp(int x, int fret) { fng->setApp(x, fret); }
	int  scheme() { return strum_scheme; }

    Fingering *fng;
    ChordList *chords;

public slots:
    void detectChord();
    void setStep3();
    void setHighSteps();
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
    QListBox *tonic, *step3, *stephigh;
    QComboBox *st[7], *inv, *bassnote;
    QLabel *cnote[7];
    QButtonGroup *complexity;
    QRadioButton *complexer[3];
	QPushButton *play;
    FingerList *fnglist;

	int strum_scheme;

#ifdef WITH_TSE3
	TSE3::MidiScheduler *scheduler;
#endif
};

#endif
