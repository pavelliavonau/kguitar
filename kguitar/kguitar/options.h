#ifndef OPTIONS_H
#define OPTIONS_H

#include <kdialogbase.h>
#include "global.h"
#include "qcheckbox.h"

class QButtonGroup;
class QRadioButton;
class QListView;
class QListBox;
class QSlider;

class OptionsMusicTheory;
class OptionsMelodyEditor;

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

class Options: public KDialogBase {
	Q_OBJECT
public:
	Options(
#ifdef WITH_TSE3
			TSE3::MidiScheduler *_sch,
#endif
			QWidget *parent = 0, char *name = 0,//##
			bool modal = TRUE);

    QButtonGroup *texLyGroup, *texSizeGroup, *texExpGroup, *prStyGroup;
    QRadioButton *tabsize[4], *expmode[2], *prsty[4];
    QCheckBox *showbarnumb, *showstr, *showpagenumb;

	OptionsMusicTheory *mt;
	OptionsMelodyEditor *me;

protected:
	void setupMusixtexTab();
	void setupMidiTab();
	void setupPrintingTab();

protected slots:
	void fillMidiBox();
	void applyBtnClicked();
	void defaultBtnClicked();

private:
#ifdef WITH_TSE3
	QListView *midiport;
	TSE3::MidiScheduler *sch;
#endif
};

#endif
