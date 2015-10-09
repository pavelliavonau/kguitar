#ifndef OPTIONSMUSICTHEORY_H
#define OPTIONSMUSICTHEORY_H

#include "optionspage.h"
#include "global.h"

class QGroupBox;
class QButtonGroup;

class OptionsMusicTheory: public OptionsPage {
	Q_OBJECT
public:
	OptionsMusicTheory(KSharedConfigPtr& conf, QWidget *parent = 0);
	virtual void applyBtnClicked() override;
	virtual void defaultBtnClicked() override;

private slots:
	bool jazzWarning();

private:
	QGroupBox *maj7Group, *flatGroup, *noteNameGroup;
	QButtonGroup *maj7, *flat, *noteName;
};

#endif
