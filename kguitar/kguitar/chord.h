#ifndef CHORD_H
#define CHORD_H

#include <qdialog.h>
#include "global.h"

class QLineEdit;
class QListBox;
class QButtonGroup;
class QRadioButton;
class Fingering;
class FingerList;

class ChordSelector: public QDialog
{
    Q_OBJECT
public:
    ChordSelector(QWidget *parent=0, const char *name=0);
public slots:
    void detectChord();
    void findChords(); 
private:
    QLineEdit *chname; 
    QListBox *tonic,*step3; 
    QListBox *chords;
    QButtonGroup *complexity;
    QRadioButton *complexer[3];
    Fingering *fng;
    FingerList *fnglist;
};

#endif
