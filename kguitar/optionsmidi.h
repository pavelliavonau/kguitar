#ifndef OPTIONSMIDI_H
#define OPTIONSMIDI_H

#include "optionspage.h"
#include "global.h"

class QTableWidget;

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

class OptionsMidi: public OptionsPage {
	Q_OBJECT
public:
#ifdef WITH_TSE3
	OptionsMidi(TSE3::MidiScheduler *, KSharedConfigPtr&, QWidget *parent = 0);
#else
	OptionsMidi(KSharedConfigPtr &, QWidget *parent = 0);
#endif
	virtual void applyBtnClicked() override;
	virtual void defaultBtnClicked() override;

protected slots:
	void fillMidiBox();

private:
	QTableWidget *midiPortsTableWidget;
#ifdef WITH_TSE3
	TSE3::MidiScheduler *sch;
#endif
};

#endif
