#ifndef OPTIONSMUSICTHEORY_H
#define OPTIONSMUSICTHEORY_H

#include <qwidget.h>
#include "global.h"
#include "config.h"

class QButtonGroup;
class QRadioButton;

class OptionsMusicTheory: public QWidget {
	Q_OBJECT
public:
	OptionsMusicTheory(QWidget *parent = 0, const char *name = 0);
	void applyBtnClicked();
	void defaultBtnClicked();
	
	QButtonGroup *maj7Group, *flatGroup;
	QRadioButton *maj7[3], *flat[2];
};

#endif
