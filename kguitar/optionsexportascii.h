#ifndef OPTIONSEXPORTASCII_H
#define OPTIONSEXPORTASCII_H

#include "optionspage.h"
#include "global.h"

class QGroupBox;
class QButtonGroup;
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
	OptionsExportAscii(KSharedConfigPtr &config, QWidget *parent = 0);
	virtual void applyBtnClicked() override;
	virtual void defaultBtnClicked() override;

private:
	QGroupBox *durationGroup;
	QButtonGroup *duration;
	QSpinBox *pageWidth;
	QCheckBox *always;
};

#endif
