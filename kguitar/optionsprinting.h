#ifndef OPTIONSPRINTING_H
#define OPTIONSPRINTING_H

#include "optionspage.h"
#include "global.h"

class QButtonGroup;
class QGroupBox;

class OptionsPrinting: public OptionsPage {
	Q_OBJECT
public:
	OptionsPrinting(KSharedConfigPtr &conf, QWidget *parent = 0);
	virtual void applyBtnClicked() override;
	virtual void defaultBtnClicked() override;

private:
    QGroupBox *styleGroup;
    QButtonGroup *styleButtons;
};

#endif
