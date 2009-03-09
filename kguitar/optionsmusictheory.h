#ifndef OPTIONSMUSICTHEORY_H
#define OPTIONSMUSICTHEORY_H

#include "optionspage.h"
#include "global.h"

class Q3ButtonGroup;
class QRadioButton;

class OptionsMusicTheory: public OptionsPage {
	Q_OBJECT
public:
	OptionsMusicTheory(KConfig *conf, QWidget *parent = 0, const char *name = 0);
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

private slots:
	bool jazzWarning();

private:
	Q3ButtonGroup *maj7Group, *flatGroup, *noteNameGroup;
	QRadioButton *maj7[3], *flat[2], *noteName[9];
};

#endif
