#ifndef OPTIONSPRINTING_H
#define OPTIONSPRINTING_H

#include "optionspage.h"
#include "global.h"

class QButtonGroup;
class QRadioButton;

class OptionsPrinting: public OptionsPage {
	Q_OBJECT
public:
	OptionsPrinting(QWidget *parent = 0, const char *name = 0);
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

    QButtonGroup *prStyGroup;
    QRadioButton *prsty[4];
};

#endif
