#ifndef OPTIONSMIDI_H
#define OPTIONSMIDI_H

#include "optionspage.h"
#include "global.h"

class QListView;

#include <tse3/MidiScheduler.h>

class OptionsMidi: public OptionsPage {
	Q_OBJECT
public:
	OptionsMidi(TSE3::MidiScheduler *, KConfig *, QWidget *parent = 0, const char *name = 0);
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

protected slots:
	void fillMidiBox();

private:
	QListView *midiport;
	TSE3::MidiScheduler *sch;
};

#endif
