#include "optionsexportascii.h"

#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include <klocale.h>
#include <kconfig.h>

OptionsExportAscii::OptionsExportAscii(KConfig *conf, QWidget *parent, const char *name)
	: OptionsPage(conf, parent, name)
{
	// Create option widgets

	durationGroup = new QVButtonGroup(i18n("&Duration Display"), this);
	duration[0] = new QRadioButton(i18n("Fixed one blank"), durationGroup);
	duration[1] = new QRadioButton(i18n("One blank") + " = 1/4", durationGroup);
	duration[2] = new QRadioButton(i18n("One blank") + " = 1/8", durationGroup);
	duration[3] = new QRadioButton(i18n("One blank") + " = 1/16", durationGroup);
	duration[4] = new QRadioButton(i18n("One blank") + " = 1/32", durationGroup);

	pageWidth = new QSpinBox(1, 1024 * 1024, 1, this);
	QLabel *pageWidth_l = new QLabel(pageWidth, i18n("Page &width:"), this);

	always = new QCheckBox(i18n("Always show this dialog on export"), this);

	// Set widget layout

    QVBoxLayout *box = new QVBoxLayout(this);
	box->addWidget(durationGroup);

	QHBoxLayout *pageWidthBox = new QHBoxLayout(box);
	pageWidthBox->addWidget(pageWidth_l);
	pageWidthBox->addWidget(pageWidth);
	pageWidthBox->addStretch(1);

	box->addStretch(1);
	box->addWidget(always);
	box->activate();

	// Fill in current config

	config->setGroup("ASCII");
	durationGroup->setButton(config->readNumEntry("DurationDisplay", 3));
	pageWidth->setValue(config->readNumEntry("PageWidth", 72));
	always->setChecked(config->readBoolEntry("AlwaysShow", TRUE));
}

void OptionsExportAscii::defaultBtnClicked()
{
	durationGroup->setButton(3);
	pageWidth->setValue(72);
	always->setChecked(TRUE);
}

void OptionsExportAscii::applyBtnClicked()
{
	config->setGroup("ASCII");
	config->writeEntry("DurationDisplay", durationGroup->id(durationGroup->selected()));
	config->writeEntry("PageWidth", pageWidth->value());
	config->writeEntry("AlwaysShow", always->isChecked());
}
