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
    flat[0] = new QRadioButton(i18n("-/+ symbols"),flatgr);
    flat[1] = new QRadioButton(i18n("b/# symbols"),flatgr);

    QVBoxLayout *vb2 = new QVBoxLayout(flatgr,15,10);
    vb2->addSpacing(5); // Cosmetic space
    vb2->addWidget(flat[0]);
    vb2->addWidget(flat[1]);
    vb2->activate();

    addTab(cd,i18n("&Chords"));


    ///////////////////////////////////////////////////////////////////
    // MusiXTeX Settings Tab  - alinx
    ///////////////////////////////////////////////////////////////////

    QWidget *tex = new QWidget(this);

    texlygr = new QButtonGroup(i18n("MusiXTeX Layout"),tex);
    texlygr->setGeometry(10,10,175,130);
    showbarnumb = new QCheckBox(i18n("Show Barnumber"),texlygr);
    showbarnumb->setGeometry(10,35,150,20);
    showstr = new QCheckBox(i18n("Show Tuning"),texlygr);
    showstr->setGeometry(10,60,150,20);
    showpagenumb = new QCheckBox(i18n("Show Pagenumber"),texlygr);
    showpagenumb->setGeometry(10,85,150,20);

    texsizegr = new QButtonGroup(i18n("Tab Size"),tex);
    texsizegr->setGeometry(200,10,175,130);
    tabsize[0] = new QRadioButton(i18n("smallest"),texsizegr);
    tabsize[1] = new QRadioButton(i18n("small"),texsizegr);
    tabsize[2] = new QRadioButton(i18n("normal"),texsizegr);
    tabsize[3] = new QRadioButton(i18n("big"),texsizegr);

    QVBoxLayout *texvb1 = new QVBoxLayout(texsizegr,15,10);
    texvb1->addSpacing(5); // Cosmetic space
    for (int i=0;i<4;i++)
	    texvb1->addWidget(tabsize[i]);
    texvb1->activate();

    addTab(tex, i18n("MusiXTeX Export"));


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
    setCaption(i18n("General options"));
}
