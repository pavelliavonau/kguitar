#include "optionsmidi.h"
#include "settings.h"

#include <klocale.h>

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QFrame>
#include <kconfig.h>
#include <kconfiggroup.h>

#ifdef WITH_TSE3
OptionsMidi::OptionsMidi(TSE3::MidiScheduler *_sch, KSharedConfigPtr &conf, QWidget *parent, const char *name)
	: OptionsPage(conf, parent, name)
{
	sch = _sch;
#else
OptionsMidi::OptionsMidi(KSharedConfigPtr &conf, QWidget *parent, const char *name)
	: OptionsPage(conf, parent, name)
{
#endif

	// Create option widgets

	midiport = new QTableWidget(this);
//	midiport->setSorting(-1); // no text sorting
	midiport->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	midiport->insertColumn(0);
	midiport->insertColumn(1);
	midiport->setHorizontalHeaderLabels(QStringList() << QString(i18n("Port")) << QString(i18n("Info")));

	fillMidiBox();

	QLabel *midiport_l = new QLabel(i18n("MIDI &output port"), midiport);

	QPushButton *midirefresh = new QPushButton(i18n("&Refresh"), this);
	connect(midirefresh, SIGNAL(clicked()), SLOT(fillMidiBox()));

	// Set widget layout

	QVBoxLayout *midivb = new QVBoxLayout(this);
	midivb->addWidget(midiport_l);
	midivb->addWidget(midiport, 1);
	midivb->addWidget(midirefresh);
	midivb->activate();
}

void OptionsMidi::fillMidiBox()
{
#ifdef WITH_TSE3
	std::vector<int> portNums;
	if (!sch)
		return;

	kdDebug() << "OptionsMidi::fillMidiBox: starting to fill";

	sch->portNumbers(portNums);

	midiport->clear();

	QTableWidgetItem *lastItem = NULL;

	for (size_t i = 0; i < sch->numPorts(); i++) {
		lastItem = new QTableWidgetItem(
			midiport, lastItem, QString::number(portNums[i]),
			sch->portName(portNums[i])
		);
		if (Settings::midiPort() == portNums[i])
			midiport->setCurrentItem(lastItem);
	}
#endif
}

void OptionsMidi::defaultBtnClicked()
{
}

void OptionsMidi::applyBtnClicked()
{
	if (midiport->currentItem())
		config->group("MIDI").writeEntry("Port", midiport->currentItem()->text().toInt());
}
