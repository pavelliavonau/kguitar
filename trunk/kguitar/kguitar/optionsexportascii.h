#ifndef OPTIONSEXPORTASCII_H
#define OPTIONSEXPORTASCII_H

#include "optionspage.h"
#include "global.h"

class KConfig;
class QVButtonGroup;
class QRadioButton;
class QCheckBox;

class OptionsExportAscii: public OptionsPage {
	Q_OBJECT
public:
	OptionsExportAscii(KConfig *conf, QWidget *parent = 0, const char *name = 0);
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

private:
	QVButtonGroup *durationGroup;
	QRadioButton *duration[5];
	QCheckBox *always;
};

#endif
