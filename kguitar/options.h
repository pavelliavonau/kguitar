#ifndef OPTIONS_H
#define OPTIONS_H

#include <kpagedialog.h>
#include "global.h"

class KConfig;
class Q3ButtonGroup;
class QRadioButton;
class Q3ListView;
class Q3ListBox;
class QSlider;
class QCheckBox;
class OptionsPage;

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

#define OPTIONS_PAGES_NUM 6

class Options: public KPageDialog {
	Q_OBJECT
public:
	Options(
#ifdef WITH_TSE3
			TSE3::MidiScheduler *sch,
#endif
			KConfig *config,
			QWidget *parent = 0, char *name = 0,//##
			bool modal = TRUE);

    OptionsPage *optWidget[OPTIONS_PAGES_NUM];

protected slots:
	void applyBtnClicked();
	void defaultBtnClicked();
};

#endif
