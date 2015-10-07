#ifndef OPTIONSEXPORTMUSIXTEX_H
#define OPTIONSEXPORTMUSIXTEX_H

#include "optionspage.h"
#include "global.h"

class QGroupBox;
class QCheckBox;
class QButtonGroup;

class OptionsExportMusixtex: public OptionsPage {
	Q_OBJECT
public:
	OptionsExportMusixtex(KSharedConfigPtr &conf, QWidget *parent = 0);
	virtual void applyBtnClicked() override;
	virtual void defaultBtnClicked() override;

private:
	QGroupBox *tabSizeGroup, *exportModeGroup;
	QCheckBox *showBarNumber, *showStr, *showPageNumber;
	QButtonGroup *tabSize, *exportMode;
	QCheckBox *always;
};

#endif
