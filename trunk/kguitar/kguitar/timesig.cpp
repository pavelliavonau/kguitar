#include "timesig.h"

#include <kapp.h>

#include <qspinbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

SetTimeSig::SetTimeSig(QWidget *parent=0, const char *name=0):
    QDialog(parent, name, TRUE)
{
    time1 = new QSpinBox(1,32,1,this);
    time2 = new QComboBox(TRUE,this);
    time2->setInsertionPolicy(QComboBox::NoInsertion);
    time2->insertItem("1");
    time2->insertItem("2");
    time2->insertItem("4");
    time2->insertItem("8");
    time2->insertItem("16");
    time2->insertItem("32");

    QLabel *time1_l = new QLabel(time1,i18n("&Beats per measure:"),this);
    QLabel *time2_l = new QLabel(time2,i18n("Beat &value:"),this);

    QPushButton *ok = new QPushButton(i18n("OK"),this);
    connect(ok,SIGNAL(clicked()),SLOT(accept()));
    QPushButton *cancel = new QPushButton(i18n("Cancel"),this);
    connect(cancel,SIGNAL(clicked()),SLOT(reject()));

    QVBoxLayout *l = new QVBoxLayout(this,10);

    QGridLayout *g = new QGridLayout(2,2,5);
    l->addLayout(g,1);
    g->addWidget(time1_l,0,0);
    g->addWidget(time1,0,1);
    g->addWidget(time2_l,1,0);
    g->addWidget(time2,1,1);
    g->setColStretch(0,2);
    g->setColStretch(1,1);
    g->addColSpacing(0,150);
    g->addColSpacing(1,50);
    g->addRowSpacing(0,25); g->addRowSpacing(1,25);

    QHBoxLayout *b = new QHBoxLayout(10);
    l->addLayout(b);
    b->addWidget(ok);
    b->addWidget(cancel);
    b->addStrut(30);

    l->activate();

    resize(0,0);
    setCaption(i18n("Time signature"));
}
