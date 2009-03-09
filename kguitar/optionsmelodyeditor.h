#ifndef OPTIONSMELODYEDITOR_H
#define OPTIONSMELODYEDITOR_H

#include "optionspage.h"
#include "global.h"

class Q3ButtonGroup;
class QRadioButton;
class QComboBox;
class QCheckBox;

class OptionsMelodyEditor: public OptionsPage {
	Q_OBJECT
public:
	OptionsMelodyEditor(KConfig *conf, QWidget *parent = 0, const char *name = 0);

public slots:
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

private:
	Q3ButtonGroup *inlayGroup, *woodGroup;
	QRadioButton *inlay[6], *wood[4];
	QComboBox *mouseAction[3];
	QCheckBox *mouseAdvance[3];
};

#endif
