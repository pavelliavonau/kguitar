#ifndef SETTABFRET_H
#define SETTABFRET_H

#include <qgroupbox.h>
#include "global.h"

class QSpinBox;
class QComboBox;
class RadiusTuner;
class QVBoxLayout;
class QHBoxLayout;

class SetTabFret: public QGroupBox
{
    Q_OBJECT
public:
    SetTabFret(QWidget *parent=0, const char *name=0);

public slots:
    void setLibTuning(int n);
    void stringChanged(int n);

private:
    QComboBox *lib;
    QSpinBox *st;
    QVBoxLayout *l;
    RadiusTuner *tuner[MAX_STRINGS];
    int oldst;
};

#endif
