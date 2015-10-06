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
OptionsMidi::OptionsMidi(TSE3::MidiScheduler *_sch, KSharedConfigPtr &conf, QWidget *parent)
	: OptionsPage(conf, parent)
{
	sch = _sch;
#else
OptionsMidi::OptionsMidi(KSharedConfigPtr &conf, QWidget *parent)
	: OptionsPage(conf, parent)
{
#endif

	// Create option widgets

	midiport = new QTableWidget(this);
//	midiport->setSorting(-1); // no text sorting
	midiport->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	midiport->setColumnCount(2);

	fillMidiBox();

	QLabel *midiport_l = new QLabel(i18n("MIDI output port"), midiport);

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
	midiport->setHorizontalHeaderLabels(QStringList() << QString(i18n("Port")) << QString(i18n("Info")));
	midiport->setRowCount(portNums.size());

	for (size_t i = 0; i < sch->numPorts(); i++) {
		QTableWidgetItem *port = new QTableWidgetItem(QString::number(portNums[i]));
		QTableWidgetItem *info = new QTableWidgetItem(sch->portName(portNums[i]));
		midiport->setItem(i, 0, port);
		midiport->setItem(i, 1, info);
		if (Settings::midiPort() == portNums[i])
			midiport->setCurrentItem(port);
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
