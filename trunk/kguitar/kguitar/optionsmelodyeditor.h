#ifndef OPTIONSMELODYEDITOR_H
#define OPTIONSMELODYEDITOR_H

#include <qwidget.h>
#include "global.h"
#include "config.h"

class QButtonGroup;
class QRadioButton;
class QComboBox;
class QCheckBox;

class OptionsMelodyEditor: public QWidget {
	Q_OBJECT
public:
	OptionsMelodyEditor(QWidget *parent = 0, const char *name = 0);
	void applyBtnClicked();
	void defaultBtnClicked();

	QButtonGroup *inlayGroup, *woodGroup;
	QRadioButton *inlay[5], *wood[4];
	QComboBox *mouseAction[3];
	QCheckBox *mouseAdvance[3];
};

#endif
