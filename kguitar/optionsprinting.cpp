#include "optionsprinting.h"
#include "settings.h"

#include <QRadioButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QGroupBox>

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

OptionsPrinting::OptionsPrinting(KSharedConfigPtr &conf, QWidget *parent)
	: OptionsPage(conf, parent)
{
	// Create option widgets

	QVBoxLayout *box = new QVBoxLayout;
	styleButtons = new QButtonGroup(this);

	QRadioButton* button = new QRadioButton(i18n("Tabulature"));
	styleButtons->addButton(button, 0);
	box->addWidget(button);

	button = new QRadioButton(i18n("Notes"));
	styleButtons->addButton(button, 1);
	box->addWidget(button);

	button = new QRadioButton(i18n("Tabulature (full) and notes"));
	styleButtons->addButton(button, 2);
	box->addWidget(button);

	button = new QRadioButton(i18n("Tabulature (minimum) and notes (not implemented)"));
	styleButtons->addButton(button, 3);
	box->addWidget(button);

	box->addStretch();
	box->activate();

	styleGroup = new QGroupBox(i18n("Style"), this);
	styleGroup->setLayout(box);

	// Set widget layout
	box = new QVBoxLayout;
	box->addWidget(styleGroup);
	setLayout(box);

	// Fill in current config
	styleButtons->button(Settings::printingStyle())->setChecked(true);
}

void OptionsPrinting::defaultBtnClicked()
{
	styleButtons->button(0)->setChecked(true);
}

void OptionsPrinting::applyBtnClicked()
{
	config->group("Printing").writeEntry("Style", styleButtons->id(styleButtons->checkedButton()));
}
