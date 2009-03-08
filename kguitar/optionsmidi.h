#ifndef OPTIONSMIDI_H
#define OPTIONSMIDI_H

#include "optionspage.h"
#include "global.h"

class QListView;

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

class OptionsMidi: public OptionsPage {
	Q_OBJECT
public:
#ifdef WITH_TSE3
	OptionsMidi(TSE3::MidiScheduler *, KConfig *, QWidget *parent = 0, const char *name = 0);
#else
	OptionsMidi(KConfig *, QWidget *parent = 0, const char *name = 0);
#endif
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

protected slots:
	void fillMidiBox();

private:
	QListView *midiport;
#ifdef WITH_TSE3
	TSE3::MidiScheduler *sch;
#endif
};

#endif
