#ifndef OPTIONS_H
#define OPTIONS_H

#include <kdialogbase.h>
#include "global.h"

class QButtonGroup;
class QRadioButton;
class QListView;
class QListBox;
class QSlider;
class QCheckBox;
class OptionsPage;

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

#define OPTIONS_PAGES_NUM 5

class Options: public KDialogBase {
	Q_OBJECT
public:
	Options(
#ifdef WITH_TSE3
			TSE3::MidiScheduler *sch,
#endif
			QWidget *parent = 0, char *name = 0,//##
			bool modal = TRUE);

    OptionsPage *optWidget[OPTIONS_PAGES_NUM];

protected slots:
	void applyBtnClicked();
	void defaultBtnClicked();
};

#endif
