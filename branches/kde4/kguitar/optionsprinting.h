#ifndef OPTIONSPRINTING_H
#define OPTIONSPRINTING_H

#include "optionspage.h"
#include "global.h"

class Q3ButtonGroup;
class QRadioButton;

class OptionsPrinting: public OptionsPage {
	Q_OBJECT
public:
	OptionsPrinting(KSharedConfigPtr &conf, QWidget *parent = 0, const char *name = 0);
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

private:
    Q3ButtonGroup *styleGroup;
    QRadioButton *style[4];
};

#endif
