#include "optionsprinting.h"
#include "settings.h"

#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>

#include <klocale.h>
#include <kconfig.h>

OptionsPrinting::OptionsPrinting(KConfig *conf, QWidget *parent, const char *name)
	: OptionsPage(conf, parent, name)
{
	// Create option widgets

	styleGroup = new QVButtonGroup(i18n("Style"), this);
	style[0] = new QRadioButton(i18n("Tabulature"), styleGroup);
	style[1] = new QRadioButton(i18n("Notes"), styleGroup);
	style[2] = new QRadioButton(i18n("Tabulature (full) and notes"), styleGroup);
	style[3] = new QRadioButton(i18n("Tabulature (minimum) and notes (not implemented)"), styleGroup);

	// Set widget layout

    QHBoxLayout *box = new QHBoxLayout(this);
	box->addWidget(styleGroup);
	box->activate();

	// Fill in current config

	styleGroup->setButton(Settings::printingStyle());
}

void OptionsPrinting::defaultBtnClicked()
{
	styleGroup->setButton(0);
}

void OptionsPrinting::applyBtnClicked()
{
	config->setGroup("Printing");
	config->writeEntry("Style", styleGroup->id(styleGroup->selected()));
}
