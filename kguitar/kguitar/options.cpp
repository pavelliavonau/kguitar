#include "options.h"
#include "globaloptions.h"

#include "optionsmusictheory.h"
#include "optionsmelodyeditor.h"

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
	resize(530, 300);

	QFrame *mtPage = addPage(i18n("Music Theory"), 0,
							 DesktopIcon("lookandfeel", KIcon::SizeMedium));
	QVBoxLayout *mtLayout = new QVBoxLayout(mtPage);
	mt = new OptionsMusicTheory();
	mtLayout->addWidget(mt);

	// Setup Tabs
	setupMusixtexTab();

#ifdef WITH_TSE3
	sch = _sch;
	setupMidiTab();
#endif

	setupPrintingTab();

	me = new OptionsMelodyEditor(addPage(i18n("Melody Editor"), 0,
										 DesktopIcon("melodyeditor", KIcon::SizeMedium)));

	texSizeGroup->setButton(globalTabSize);
	showbarnumb->setChecked(globalShowBarNumb);
	showstr->setChecked(globalShowStr);
	showpagenumb->setChecked(globalShowPageNumb);
	texExpGroup->setButton(globalTexExpMode);

	prStyGroup->setButton(globalPrSty);

	connect(this, SIGNAL(defaultClicked()), SLOT(defaultBtnClicked()));
	connect(this, SIGNAL(okClicked()), SLOT(applyBtnClicked()));
	connect(this, SIGNAL(applyClicked()), SLOT(applyBtnClicked()));
}

void Options::setupMusixtexTab()
{
	QFrame *tex = addPage(i18n("MusiXTeX Export"), 0,
						  DesktopIcon("musixtex", KIcon::SizeMedium));

	texLyGroup = new QButtonGroup(i18n("MusiXTeX Layout"), tex);
	texLyGroup->setMinimumSize(175, 75);
	showbarnumb = new QCheckBox(i18n("Show Bar Number"), texLyGroup);
	showbarnumb->setGeometry(10, 35, 150, 20);
	showstr = new QCheckBox(i18n("Show Tuning"), texLyGroup);
	showstr->setGeometry(10, 60, 150, 20);
	showpagenumb = new QCheckBox(i18n("Show Page Number"), texLyGroup);
	showpagenumb->setGeometry(10, 85, 150, 20);

	QVBoxLayout *texvb1 = new QVBoxLayout(texLyGroup, 15, 10);
	texvb1->addSpacing(5);
	texvb1->addWidget(showbarnumb);
	texvb1->addWidget(showstr);
	texvb1->addWidget(showpagenumb);
	texvb1->activate();

	texExpGroup = new QButtonGroup(i18n("Export as..."), tex);
	texExpGroup->setMinimumSize(175, 75);
	expmode[0] = new QRadioButton(i18n("Tabulature"), texExpGroup);
	expmode[1] = new QRadioButton(i18n("Notes"), texExpGroup);

	QVBoxLayout *texvb2 = new QVBoxLayout(texExpGroup, 15, 10);
    texvb2->addSpacing(5); // Cosmetic space
	texvb2->addWidget(expmode[0]);
	texvb2->addWidget(expmode[1]);
	texvb2->activate();

	texSizeGroup = new QButtonGroup(i18n("Tab Size"), tex);
	texSizeGroup->setMinimumSize(175, 130);
	tabsize[0] = new QRadioButton(i18n("Smallest"), texSizeGroup);
	tabsize[1] = new QRadioButton(i18n("Small"), texSizeGroup);
	tabsize[2] = new QRadioButton(i18n("Normal"), texSizeGroup);
	tabsize[3] = new QRadioButton(i18n("Big"), texSizeGroup);

    QVBoxLayout *texvb3 = new QVBoxLayout(texSizeGroup, 15, 10);
    texvb3->addSpacing(5); // Cosmetic space
    for (int i = 0; i < 4; i++)
	    texvb3->addWidget(tabsize[i]);
    texvb3->activate();

	QHBoxLayout *vbtex = new QHBoxLayout(tex, 15, 10);
	vbtex->addWidget(texLyGroup);
	vbtex->addWidget(texSizeGroup);
	vbtex->addWidget(texExpGroup);
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

void Options::setupPrintingTab()
{
	QFrame *prn = addPage(i18n("Printing"), 0,
						  DesktopIcon("printmgr", KIcon::SizeMedium));

    // Printing style group

	prStyGroup = new QButtonGroup(i18n("Style"), prn);
	prStyGroup->setMinimumSize(150, 110);
	prsty[0] = new QRadioButton(i18n("Tabulature"), prStyGroup);
	prsty[1] = new QRadioButton(i18n("Notes"), prStyGroup);
	prsty[2] = new QRadioButton(i18n("Tabulature (full) and notes"), prStyGroup);
	prsty[3] = new QRadioButton(i18n("Tabulature (minimum) and notes (not implemented)"), prStyGroup);

	QVBoxLayout *vb1 = new QVBoxLayout(prStyGroup, 15, 10);
	vb1->addSpacing(5); // Cosmetic space
	for (int i = 0; i < 4; i++)
		vb1->addWidget(prsty[i]);
	vb1->activate();

	QHBoxLayout *vbcd = new QHBoxLayout(prn, 15, 10);
	vbcd->addWidget(prStyGroup);
	vbcd->activate();
}

void Options::fillMidiBox()
{
#ifdef WITH_TSE3
	std::vector<int> portNums;
	if (!sch)
		return;
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

// Saves options back from dialog to memory
void Options::applyBtnClicked()
{
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

	if (prsty[0]->isChecked())  globalPrSty = 0;
	if (prsty[1]->isChecked())  globalPrSty = 1;
	if (prsty[2]->isChecked())  globalPrSty = 2;
	if (prsty[3]->isChecked())  globalPrSty = 3;

	mt->applyBtnClicked();
	me->applyBtnClicked();
}

void Options::defaultBtnClicked()
{
	texSizeGroup->setButton(2);
	showbarnumb->setChecked(TRUE);
	showstr->setChecked(TRUE);
	showpagenumb->setChecked(TRUE);
	texExpGroup->setButton(0);

	prStyGroup->setButton(0);

	mt->defaultBtnClicked();
	me->defaultBtnClicked();
}
