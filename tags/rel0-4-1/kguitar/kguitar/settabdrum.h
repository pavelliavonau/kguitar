#ifndef SETTABDRUM_H
#define SETTABDRUM_H

#include <qwidget.h>
#include "global.h"

#include <qspinbox.h>
#include <qlineedit.h>

class SetTabDrum: public QWidget {
    Q_OBJECT
public:
    SetTabDrum(QWidget *parent=0, const char *name=0);

    void setDrums(int n) { dr->setValue(n); };
    void setTune(uchar x, uchar n) { tuner[x]->setValue(n); };
    int drums() { return dr->value(); };
    uchar tune(uchar x) { return tuner[x]->value(); };

public slots:
    void stringChanged(int n);

private:
    virtual void resizeEvent(QResizeEvent *e);
    void reposTuners();

    QSpinBox *dr;
    QSpinBox *tuner[MAX_STRINGS];
	QLineEdit *tname[MAX_STRINGS];
    int oldst;
};

#endif
