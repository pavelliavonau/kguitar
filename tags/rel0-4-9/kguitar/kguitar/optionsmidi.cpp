#include "optionsmidi.h"
#include "globaloptions.h"

#include <klocale.h>

#include <qlayout.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qpushbutton.h>

OptionsMidi::OptionsMidi(TSE3::MidiScheduler *_sch, QWidget *parent, const char *name)
	: OptionsPage(parent, name)
{
    sch = _sch;

	midiport = new QListView(this);
	midiport->setSorting(-1); // no text sorting
	midiport->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	midiport->addColumn(i18n("Port"));
	midiport->addColumn(i18n("Info"));

	fillMidiBox();

	QLabel *midiport_l = new QLabel(midiport, i18n("MIDI &output port"), this);

	QPushButton *midirefresh = new QPushButton(i18n("&Refresh"), this);
	connect(midirefresh, SIGNAL(clicked()), SLOT(fillMidiBox()));

	QVBoxLayout *midivb = new QVBoxLayout(this, 10, 5);
	midivb->addWidget(midiport_l);
	midivb->addWidget(midiport, 1);
	midivb->addWidget(midirefresh);
	midivb->activate();
}

void OptionsMidi::fillMidiBox()
{
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
}

void OptionsMidi::defaultBtnClicked()
{
}

void OptionsMidi::applyBtnClicked()
{
	if (midiport->currentItem())
		globalMidiPort = midiport->currentItem()->text(0).toInt();
}
