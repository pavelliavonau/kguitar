#include "settabfret.h"

// External files with tuning library
#include "tunings.h"
#include "radiustuner.h"

#include <kapp.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>

SetTabFret::SetTabFret(QWidget *parent=0, const char *name=0): QGroupBox(parent,name)
{
    l = new QVBoxLayout(this,5);

    // Controls

    QGridLayout *g = new QGridLayout(2,2,10);
    l->addLayout(g);

    lib = new QComboBox(FALSE,this);
    QLabel *lib_l = new QLabel(i18n("Tuning:"),this);
    connect(lib,SIGNAL(highlighted(int)),SLOT(setLibTuning(int)));

    for (int i=0;lib_tuning[i].strings;i++)
	lib->insertItem(lib_tuning[i].name);

    st = new QSpinBox(1,MAX_STRINGS,1,this);
    QLabel *st_l = new QLabel(i18n("Strings:"),this);
    st_l->setGeometry(5,30,50,20);
    st->setGeometry(60,30,40,20);
    connect(st,SIGNAL(valueChanged(int)),SLOT(stringChanged(int)));

    g->addWidget(lib_l,0,0);
    g->addWidget(lib,0,1);
    g->addWidget(st_l,1,0);
    g->addWidget(st,1,1);
    g->addRowSpacing(0,20);
    g->addRowSpacing(1,20);

    // Tuners

    QHBoxLayout *rt = new QHBoxLayout();
    l->addLayout(rt,1);

    for (int i=0;i<MAX_STRINGS;i++) {
	tuner[i] = new RadiusTuner(this);
	rt->addWidget(tuner[i]);
    }

    l->activate();
}

void SetTabFret::setLibTuning(int n)
{
    st->setValue(lib_tuning[n].strings);
    for (int i=0;i<lib_tuning[n].strings;i++)
	tuner[i]->setValue(lib_tuning[n].shift[i]);
}

void SetTabFret::stringChanged(int n)
{
    for (int i=0;i<n;i++)
	if (!tuner[i])
	    tuner[i] = new RadiusTuner(this);
//	tuner[i]->show();
    for (int i=n;i<MAX_STRINGS;i++)
	delete tuner[i];
//    tuner[i]->hide();
    l->activate();
}
