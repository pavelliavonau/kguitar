#ifndef OPTIONSPAGE_H
#define OPTIONSPAGE_H

#include "global.h"
#include <qwidget.h>

class KConfig;

class OptionsPage: public QWidget {
	Q_OBJECT
public:
	OptionsPage(KConfig *conf, QWidget *parent = 0, const char *name = 0);

public slots:
	virtual void applyBtnClicked() = 0;
	virtual void defaultBtnClicked() = 0;

protected:
	KConfig *config;
};

#endif
