#ifndef OPTIONSMUSICTHEORY_H
#define OPTIONSMUSICTHEORY_H

#include "optionspage.h"
#include "global.h"

class QButtonGroup;
class QRadioButton;

class OptionsMusicTheory: public OptionsPage {
	Q_OBJECT
public:
	OptionsMusicTheory(QWidget *parent = 0, const char *name = 0);
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

	QButtonGroup *maj7Group, *flatGroup;
	QRadioButton *maj7[3], *flat[2];
};

#endif
