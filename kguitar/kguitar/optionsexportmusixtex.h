#ifndef OPTIONSEXPORTMUSIXTEX_H
#define OPTIONSEXPORTMUSIXTEX_H

#include "optionspage.h"
#include "global.h"

class QButtonGroup;
class QCheckBox;
class QRadioButton;

class OptionsExportMusixtex: public OptionsPage {
	Q_OBJECT
public:
	OptionsExportMusixtex(QWidget *parent = 0, const char *name = 0);
	virtual void applyBtnClicked();
	virtual void defaultBtnClicked();

	QButtonGroup *texLyGroup, *texSizeGroup, *texExpGroup;
	QCheckBox *showbarnumb, *showstr, *showpagenumb;
	QRadioButton *tabsize[4], *expmode[2];
};

#endif
