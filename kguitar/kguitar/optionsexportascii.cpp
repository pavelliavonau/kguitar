#include "optionsexportascii.h"

#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include <klocale.h>
#include <kconfig.h>

OptionsExportAscii::OptionsExportAscii(KConfig *conf, QWidget *parent, const char *name)
	: OptionsPage(parent, name)
{
	config = conf;

	// Create option widgets

	durationGroup = new QVButtonGroup(i18n("Duration Display"), this);
	duration[0] = new QRadioButton(i18n("Fixed one blank"), durationGroup);
	duration[1] = new QRadioButton(i18n("One blank") + " = 1/4", durationGroup);
	duration[2] = new QRadioButton(i18n("One blank") + " = 1/8", durationGroup);
	duration[3] = new QRadioButton(i18n("One blank") + " = 1/16", durationGroup);
	duration[4] = new QRadioButton(i18n("One blank") + " = 1/32", durationGroup);

	always = new QCheckBox(i18n("Always show this dialog on export"), this);

	// Set widget layout

    QVBoxLayout *box = new QVBoxLayout(this);
	box->addWidget(durationGroup);
	box->addStretch(1);
	box->addWidget(always);
	box->activate();

	// Fill in current config

	config->setGroup("Export ASCII");
	durationGroup->setButton(config->readNumEntry("DurationDisplay", 3));
	always->setChecked(config->readBoolEntry("AlwaysShow", TRUE));
}

void OptionsExportAscii::defaultBtnClicked()
{
	durationGroup->setButton(3);
	always->setChecked(TRUE);
}

void OptionsExportAscii::applyBtnClicked()
{
	config->setGroup("Export ASCII");
	config->writeEntry("DurationDisplay",
	                                durationGroup->id(durationGroup->selected()));
	config->writeEntry("AlwaysShow", always->isChecked());
}
