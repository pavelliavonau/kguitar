#include "options.h"
#include "globaloptions.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#ifdef WITH_TSE3
#include <qframe.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#endif

Options::Options(
#ifdef WITH_TSE3
				 TSE3::MidiScheduler *_sch,
#endif
				 QWidget *parent, char *name, bool modal)
	: KDialogBase(IconList, i18n("Preferences"), Help|Default|Ok|Apply|Cancel,
				  Ok, parent, name, modal, TRUE)
{
	// Setup Tabs
	setupTheoryTab();
	setupMusixtexTab();

#ifdef WITH_TSE3
	sch = _sch;
	setupMidiTab();
#endif

	resize(530, 300);
	connect(this, SIGNAL(defaultClicked()), SLOT(defaultBtnClicked()));
	connect(this, SIGNAL(okClicked()), SLOT(applyBtnClicked()));
	connect(this, SIGNAL(applyClicked()), SLOT(applyBtnClicked()));
}

void Options::setupTheoryTab()
{
	// ALINXFIX: find or make a better icon for this page !!!
	QFrame *cd = addPage(i18n("Music Theory"), 0,
						 DesktopIcon("looknfeel", KIcon::SizeMedium));

    // Dominant 7th name selection group

	maj7gr = new QButtonGroup(i18n("Dominant 7th"), cd);
	maj7gr->setMinimumSize(150, 110);
	maj7[0] = new QRadioButton("7M", maj7gr);
	maj7[1] = new QRadioButton("maj7", maj7gr);
	maj7[2] = new QRadioButton("dom7", maj7gr);

	QVBoxLayout *vb1 = new QVBoxLayout(maj7gr, 15, 10);
	vb1->addSpacing(5); // Cosmetic space
	for (int i = 0; i < 3; i++)
		vb1->addWidget(maj7[i]);
	vb1->activate();

    // Chord step alterations selection group

	flatgr = new QButtonGroup(i18n("Alterations"), cd);
	flatgr->setMinimumSize(150, 110);
	flat[0] = new QRadioButton(i18n("-/+ symbols"), flatgr);
	flat[1] = new QRadioButton(i18n("b/# symbols"), flatgr);

	QVBoxLayout *vb2 = new QVBoxLayout(flatgr, 15, 10);
	vb2->addSpacing(5); // Cosmetic space
	vb2->addWidget(flat[0]);
	vb2->addWidget(flat[1]);
	vb2->activate();

	QHBoxLayout *vbcd = new QHBoxLayout(cd, 15, 10);
	vbcd->addWidget(maj7gr);
	vbcd->addWidget(flatgr);
	vbcd->activate();
}

void Options::setupMusixtexTab()
{
	QFrame *tex = addPage(i18n("MusiXTeX Export"), 0,
						  DesktopIcon("musixtex", KIcon::SizeMedium));

	texlygr = new QButtonGroup(i18n("MusiXTeX Layout"), tex);
	texlygr->setMinimumSize(175, 75);
	showbarnumb = new QCheckBox(i18n("Show Barnumber"), texlygr);
	showbarnumb->setGeometry(10, 35, 150, 20);
	showstr = new QCheckBox(i18n("Show Tuning"), texlygr);
	showstr->setGeometry(10, 60, 150, 20);
	showpagenumb = new QCheckBox(i18n("Show Pagenumber"), texlygr);
	showpagenumb->setGeometry(10, 85, 150, 20);

	QVBoxLayout *texvb1 = new QVBoxLayout(texlygr, 15, 10);
	texvb1->addSpacing(5);
	texvb1->addWidget(showbarnumb);
	texvb1->addWidget(showstr);
	texvb1->addWidget(showpagenumb);
	texvb1->activate();

	texexpgr = new QButtonGroup(i18n("Export as..."), tex);
	texexpgr->setMinimumSize(175, 75);
	expmode[0] = new QRadioButton(i18n("Tabulature"), texexpgr);
	expmode[1] = new QRadioButton(i18n("Notes"), texexpgr);

	QVBoxLayout *texvb2 = new QVBoxLayout(texexpgr, 15, 10);
    texvb2->addSpacing(5); // Cosmetic space
	texvb2->addWidget(expmode[0]);
	texvb2->addWidget(expmode[1]);
	texvb2->activate();

	texsizegr = new QButtonGroup(i18n("Tab Size"), tex);
	texsizegr->setMinimumSize(175, 130);
	tabsize[0] = new QRadioButton(i18n("Smallest"), texsizegr);
	tabsize[1] = new QRadioButton(i18n("Small"), texsizegr);
	tabsize[2] = new QRadioButton(i18n("Normal"), texsizegr);
	tabsize[3] = new QRadioButton(i18n("Big"), texsizegr);

    QVBoxLayout *texvb3 = new QVBoxLayout(texsizegr, 15, 10);
    texvb3->addSpacing(5); // Cosmetic space
    for (int i = 0; i < 4; i++)
	    texvb3->addWidget(tabsize[i]);
    texvb3->activate();

	QHBoxLayout *vbtex = new QHBoxLayout(tex, 15, 10);
	vbtex->addWidget(texlygr);
	vbtex->addWidget(texsizegr);
	vbtex->addWidget(texexpgr);
	vbtex->activate();
}

#ifdef WITH_TSE3
void Options::setupMidiTab()
{
	QFrame *midi = addPage(i18n("MIDI"), QString::null, DesktopIcon("kcmmidi", KIcon::SizeMedium));

	midiport = new QListView(midi);
	midiport->setSorting(-1); // no text sorting
	midiport->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	midiport->addColumn(i18n("Port"));
	midiport->addColumn(i18n("Info"));
	midiport->setMinimumSize(350, 100);

	fillMidiBox();

	QLabel *midiport_l = new QLabel(midiport, i18n("MIDI &output port"), midi);
	midiport_l->setMinimumSize(100, 15);

	QPushButton *midirefresh = new QPushButton(i18n("&Refresh"), midi);
	connect(midirefresh, SIGNAL(clicked()), SLOT(fillMidiBox()));
	midirefresh->setMinimumSize(75, 30);

	QVBoxLayout *midivb = new QVBoxLayout(midi, 10, 5);
	midivb->addWidget(midiport_l);
	midivb->addWidget(midiport, 1);
	midivb->addWidget(midirefresh);
	midivb->activate();
}
#endif

void Options::fillMidiBox()
{
#ifdef WITH_TSE3
	std::vector<int> portNums;
	sch->portNumbers(portNums);

	midiport->clear();

	QListViewItem *lastItem = NULL;

	for (size_t i = 0; i < sch->numPorts(); i++) {
		lastItem = new QListViewItem(midiport,
									 lastItem,
									 QString::number(portNums[i]),
									 sch->portName(portNums[i]));
		if (globalMidiPort == portNums[i])
			midiport->setCurrentItem(lastItem);
	}
#endif
}

void Options::applyBtnClicked()
{
	if (maj7[0]->isChecked())  globalMaj7 = 0;
	if (maj7[1]->isChecked())  globalMaj7 = 1;
	if (maj7[2]->isChecked())  globalMaj7 = 2;

	if (flat[0]->isChecked())  globalFlatPlus = 0;
	if (flat[1]->isChecked())  globalFlatPlus = 1;

	for (int i = 0; i <= 3; i++)
		if (tabsize[i]->isChecked())  globalTabSize = i;

	globalShowBarNumb = showbarnumb->isChecked();
	globalShowStr = showstr->isChecked();
	globalShowPageNumb = showpagenumb->isChecked();

	if (expmode[0]->isChecked()) globalTexExpMode = 0;
	if (expmode[1]->isChecked()) globalTexExpMode = 1;

#ifdef WITH_TSE3
	if (midiport->currentItem())
		globalMidiPort = midiport->currentItem()->text(0).toInt();
#endif
}

void Options::defaultBtnClicked()
{
	maj7gr->setButton(0);
	flatgr->setButton(0);

	texsizegr->setButton(2);
	showbarnumb->setChecked(TRUE);
	showstr->setChecked(TRUE);
	showpagenumb->setChecked(TRUE);
	texexpgr->setButton(0);
}
