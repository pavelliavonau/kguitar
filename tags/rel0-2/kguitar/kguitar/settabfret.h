#ifndef SETTABFRET_H
#define SETTABFRET_H

#include <qgroupbox.h>
#include "global.h"

#include <qspinbox.h>
#include <radiustuner.h>

class QComboBox;

class SetTabFret: public QGroupBox
{
    Q_OBJECT
public:
    SetTabFret(QWidget *parent=0, const char *name=0);

    void setString(int n) { st->setValue(n); };
    void setFrets(int n) { fr->setValue(n); };
    void setTune(uchar x, uchar n) { tuner[x]->setValue(n); };
    int string() { return st->value(); };
    int frets() { return fr->value(); };
    uchar tune(uchar x) { return tuner[x]->value(); };

public slots:
    void setLibTuning(int n);
    void stringChanged(int n);
    void tuneChanged();

private:
    virtual void resizeEvent(QResizeEvent *e);
    void reposTuners();

    QComboBox *lib;
    QSpinBox *st,*fr;
    RadiusTuner *tuner[MAX_STRINGS];
    int oldst;
};

#endif
