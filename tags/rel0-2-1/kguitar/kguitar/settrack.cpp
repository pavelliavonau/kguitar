#include "settrack.h"
#include "settabfret.h"

#include <kapp.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <kintegerline.h>
#include <qlabel.h>
#include <qpushbutton.h>

SetTrack::SetTrack(QWidget *parent=0, const char *name=0): QDialog(parent,name,TRUE)
{
    QVBoxLayout *l = new QVBoxLayout(this,10);

    // GENERAL CONTROLS

    QGridLayout *g = new QGridLayout(4,2,10);
    l->addLayout(g);

    title = new QLineEdit(this);
    channel = new KIntegerLine(this);
    bank = new KIntegerLine(this);
    patch = new KIntegerLine(this);
    QLabel *title_l = new QLabel(title,i18n("&Track name:"),this);
    QLabel *channel_l = new QLabel(bank,i18n("&Channel:"),this);
    QLabel *bank_l = new QLabel(bank,i18n("&Bank:"),this);
    QLabel *patch_l = new QLabel(patch,i18n("&Patch:"),this);

    g->addWidget(title_l,0,0);
    g->addWidget(title,0,1);
    g->addWidget(channel_l,1,0);
    g->addWidget(channel,1,1);
    g->addWidget(bank_l,2,0);
    g->addWidget(bank,2,1);
    g->addWidget(patch_l,3,0);
    g->addWidget(patch,3,1);

    for (int i=0;i<4;i++) 
	g->addRowSpacing(i,20);

    g->addColSpacing(0,80);
    g->setColStretch(1,1);

    // TAB MODE SPECIFIC WIDGET

    fret = new SetTabFret(this);
    l->addWidget(fret,1);

    // DIALOG BUTTONS

    QHBoxLayout *butt = new QHBoxLayout();
    l->addLayout(butt);

    QPushButton *ok = new QPushButton(i18n("OK"),this);
    connect(ok,SIGNAL(clicked()),SLOT(accept()));
    QPushButton *cancel = new QPushButton(i18n("Cancel"),this);
    connect(cancel,SIGNAL(clicked()),SLOT(reject()));

    butt->addWidget(ok);
    butt->addWidget(cancel);
    butt->addStrut(30);
    
    l->activate();

    setCaption(i18n("Track properties"));
}
