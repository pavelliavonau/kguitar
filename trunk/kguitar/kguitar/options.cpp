#include "options.h"

#include <kapp.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

Options::Options(QWidget *parent=0, const char *name=0): QTabDialog(parent,name,TRUE)
{
    //////////////////////////////////////////////////////////////////
    // CHORD DIALOG SETTINGS TAB
    //////////////////////////////////////////////////////////////////

    QWidget *cd = new QWidget(this);

    // Dominant 7th name selection group

    maj7gr = new QButtonGroup(i18n("Dominant 7th"),cd);
    maj7gr->setGeometry(10,10,150,110);
    maj7[0] = new QRadioButton("7M",maj7gr);
    maj7[1] = new QRadioButton("maj7",maj7gr);
    maj7[2] = new QRadioButton("dom7",maj7gr);

    QVBoxLayout *vb1 = new QVBoxLayout(maj7gr,15,10);
    vb1->addSpacing(5); // Cosmetic space
    for (int i=0;i<3;i++)
	vb1->addWidget(maj7[i]);
    vb1->activate();

    // Chord step alterations selection group

    flatgr = new QButtonGroup(i18n("Alterations"),cd);
    flatgr->setGeometry(170,10,150,110);
    flat[0] = new QRadioButton("-/+ symbols",flatgr);
    flat[1] = new QRadioButton("b/# symbols",flatgr);

    QVBoxLayout *vb2 = new QVBoxLayout(flatgr,15,10);
    vb2->addSpacing(5); // Cosmetic space
    vb2->addWidget(flat[0]);
    vb2->addWidget(flat[1]);
    vb2->activate();

    addTab(cd,i18n("&Chords"));

    //////////////////////////////////////////////////////////////////
    // SOME OTHER SETTINGS TAB
    //////////////////////////////////////////////////////////////////

    QWidget *so = new QWidget(this);
    
    addTab(so,i18n("Others"));
    
    //////////////////////////////////////////////////////////////////
    // REST OF TABDIALOG SETTINS
    //////////////////////////////////////////////////////////////////

    setOkButton(i18n("OK"));
    setCancelButton(i18n("Cancel"));

    resize(400,300);
    setCaption(i18n("Options"));
}
