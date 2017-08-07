#include "optionsmidi.h"
#include "settings.h"

#include <KLocalizedString>

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QFrame>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <QHeaderView>

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

	midiPortsTableWidget = new QTableWidget(this);
//	midiport->setSorting(-1); // no text sorting
	midiPortsTableWidget->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	midiPortsTableWidget->setColumnCount(2);
	midiPortsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	midiPortsTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	midiPortsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	midiPortsTableWidget->horizontalHeader()->setStretchLastSection(true);
	midiPortsTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	fillMidiBox();

	QLabel *midiport_l = new QLabel(i18n("MIDI output port"), midiPortsTableWidget);

	QPushButton *midirefresh = new QPushButton(i18n("&Refresh"), this);
	connect(midirefresh, SIGNAL(clicked()), SLOT(fillMidiBox()));

	// Set widget layout

	QVBoxLayout *midivb = new QVBoxLayout(this);
	midivb->addWidget(midiport_l);
	midivb->addWidget(midiPortsTableWidget, 1);
	midivb->addWidget(midirefresh);
	midivb->activate();
}

void OptionsMidi::fillMidiBox()
{
#ifdef WITH_TSE3
	std::vector<int> portNums;
	if (!sch)
		return;

	qDebug() << "OptionsMidi::fillMidiBox: starting to fill";

	sch->portNumbers(portNums);

	midiPortsTableWidget->clear();
	midiPortsTableWidget->setHorizontalHeaderLabels(QStringList() << QString(i18n("Port")) << QString(i18n("Info")));
	midiPortsTableWidget->setRowCount(portNums.size());

	for (size_t i = 0; i < sch->numPorts(); i++) {
		QTableWidgetItem *port = new QTableWidgetItem(QString::number(portNums[i]));
		QTableWidgetItem *info = new QTableWidgetItem(sch->portName(portNums[i]));
		midiPortsTableWidget->setItem(i, 0, port);
		midiPortsTableWidget->setItem(i, 1, info);
		if (Settings::midiPort() == portNums[i])
			midiPortsTableWidget->setCurrentItem(port);
	}
#endif
}

void OptionsMidi::defaultBtnClicked()
{
}

void OptionsMidi::applyBtnClicked()
{
  if (midiPortsTableWidget->selectionModel()->hasSelection())
    config->group("MIDI").writeEntry("Port", midiPortsTableWidget->item(midiPortsTableWidget->currentRow(), 0)->text().toInt());
}
