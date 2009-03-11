#ifndef OPTIONSMELODYEDITOR_H
#define OPTIONSMELODYEDITOR_H

#include "optionspage.h"
#include "global.h"
#include <ksharedconfig.h>

class Q3VButtonGroup;
class QRadioButton;
class QComboBox;
class QCheckBox;

class OptionsMelodyEditor: public OptionsPage {
	Q_OBJECT
public:
	OptionsMelodyEditor(KSharedConfigPtr &conf, QWidget *parent = 0, const char *name = 0);

public slots:
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

private:
	Q3VButtonGroup *inlayGroup, *woodGroup;
	QRadioButton *inlay[6], *wood[4];
	QComboBox *mouseAction[3];
	QCheckBox *mouseAdvance[3];
};

#endif
