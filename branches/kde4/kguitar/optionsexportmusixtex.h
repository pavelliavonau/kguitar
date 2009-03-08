#ifndef OPTIONSEXPORTMUSIXTEX_H
#define OPTIONSEXPORTMUSIXTEX_H

#include "optionspage.h"
#include "global.h"

class QVButtonGroup;
class QCheckBox;
class QRadioButton;

class OptionsExportMusixtex: public OptionsPage {
	Q_OBJECT
public:
	OptionsExportMusixtex(KConfig *conf, QWidget *parent = 0, const char *name = 0);
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

private:
	QVButtonGroup *tabSizeGroup, *exportModeGroup;
	QCheckBox *showBarNumber, *showStr, *showPageNumber;
	QRadioButton *tabSize[4], *exportMode[2];
	QCheckBox *always;
};

#endif
