#include "optionsprinting.h"
#include "globaloptions.h"

#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>

#include <klocale.h>

OptionsPrinting::OptionsPrinting(QWidget *parent, const char *name)
	: OptionsPage(parent, name)
{

    // Printing style group

	prStyGroup = new QVButtonGroup(i18n("Style"), this);
	prsty[0] = new QRadioButton(i18n("Tabulature"), prStyGroup);
	prsty[1] = new QRadioButton(i18n("Notes"), prStyGroup);
	prsty[2] = new QRadioButton(i18n("Tabulature (full) and notes"), prStyGroup);
	prsty[3] = new QRadioButton(i18n("Tabulature (minimum) and notes (not implemented)"), prStyGroup);

    QHBoxLayout *vbcd = new QHBoxLayout(this);
	vbcd->addWidget(prStyGroup);
	vbcd->activate();

	prStyGroup->setButton(globalPrSty);
}

void OptionsPrinting::defaultBtnClicked()
{
	prStyGroup->setButton(0);
}

void OptionsPrinting::applyBtnClicked()
{
	if (prsty[0]->isChecked())  globalPrSty = 0;
	if (prsty[1]->isChecked())  globalPrSty = 1;
	if (prsty[2]->isChecked())  globalPrSty = 2;
	if (prsty[3]->isChecked())  globalPrSty = 3;
}
