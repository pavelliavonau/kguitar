#include "optionsprinting.h"
#include "settings.h"

#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3HBoxLayout>

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>

OptionsPrinting::OptionsPrinting(KSharedConfigPtr &conf, QWidget *parent, const char *name)
	: OptionsPage(conf, parent, name)
{
	// Create option widgets

	styleGroup = new Q3VButtonGroup(i18n("Style"), this);
	style[0] = new QRadioButton(i18n("Tabulature"), styleGroup);
	style[1] = new QRadioButton(i18n("Notes"), styleGroup);
	style[2] = new QRadioButton(i18n("Tabulature (full) and notes"), styleGroup);
	style[3] = new QRadioButton(i18n("Tabulature (minimum) and notes (not implemented)"), styleGroup);

	// Set widget layout

    Q3HBoxLayout *box = new Q3HBoxLayout(this);
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
	config->group("Printing").writeEntry("Style", styleGroup->id(styleGroup->selected()));
}
