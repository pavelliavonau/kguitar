#ifndef CHORD_H
#define CHORD_H

#include <qdialog.h>
#include "global.h"
#include "globaloptions.h"

#include "fingers.h"

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

class ChordSelector: public QDialog
{
    Q_OBJECT
public:
    ChordSelector(TabTrack *p, QWidget *parent=0, const char *name=0);
    int  app(int x) { return fng->app(x); }
    void setApp(int x, int fret) { fng->setApp(x,fret); }

public slots:
    void detectChord();
    void setStep3();
    void setHighSteps();
    void setStepsFromChord();
    void findSelection();
    void findChords(); 
private:
    TabTrack *parm;

    QLineEdit *chname; 
    QListBox *tonic,*step3,*stephigh;
    ChordList *chords;
    QComboBox *st[6],*inv;
    QLabel *cnote[7];
    QButtonGroup *complexity;
    QRadioButton *complexer[3];
    Fingering *fng;
    FingerList *fnglist;
};

#endif
