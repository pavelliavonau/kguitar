#ifndef CHORD_H
#define CHORD_H

#include <qdialog.h>
#include "global.h"
#include "globaloptions.h"

#include "fingers.h"
#include <qcombobox.h>

#define STEPSIZE     40

class QLineEdit;
class QListBox;
class QButtonGroup;
class QRadioButton;
class QComboBox;
class QLabel;
class ChordList;
class FingerList;
class TabTrack;
class Strumming;

//##class DeviceManager;

class ChordSelector: public QDialog {
    Q_OBJECT
public:
    ChordSelector(/*DeviceManager *_dm,*/ TabTrack *p, QWidget *parent = 0, //##
				  const char *name = 0);
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

private:
    TabTrack *parm;

    QLineEdit *chname;
    QListBox *tonic, *step3, *stephigh;
    QComboBox *st[6], *inv, *bassnote;
    QLabel *cnote[7];
    QButtonGroup *complexity;
    QRadioButton *complexer[3];
    FingerList *fnglist;

	int strum_scheme;
//##	DeviceManager *dm;
};

#endif
