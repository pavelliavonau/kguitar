#ifndef OPTIONSEXPORTASCII_H
#define OPTIONSEXPORTASCII_H

#include "optionspage.h"
#include "global.h"

class KConfig;
class Q3VButtonGroup;
class QRadioButton;
class QSpinBox;
class QCheckBox;

/**
 * Options page for ASCII tabulature export setup.
 *
 * Allows to set duration display (number of spaces in ASCII
 * tabulature rendering) and page width.
 */
class OptionsExportAscii: public OptionsPage {
	Q_OBJECT
public:
	OptionsExportAscii(KSharedConfigPtr &config, QWidget *parent = 0, const char *name = 0);
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

private:
	Q3VButtonGroup *durationGroup;
	QRadioButton *duration[5];
	QSpinBox *pageWidth;
	QCheckBox *always;
};

#endif
