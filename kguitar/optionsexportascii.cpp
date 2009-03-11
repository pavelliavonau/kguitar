#include "optionsexportascii.h"

#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3HBoxLayout>

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>

OptionsExportAscii::OptionsExportAscii(KSharedConfigPtr &conf, QWidget *parent, const char *name)
	: OptionsPage(conf, parent, name)
{
	// Create option widgets

	durationGroup = new Q3VButtonGroup(i18n("&Duration Display"), this);
	duration[0] = new QRadioButton(i18n("Fixed one blank"), durationGroup);
	duration[1] = new QRadioButton(i18n("One blank") + " = 1/4", durationGroup);
	duration[2] = new QRadioButton(i18n("One blank") + " = 1/8", durationGroup);
	duration[3] = new QRadioButton(i18n("One blank") + " = 1/16", durationGroup);
	duration[4] = new QRadioButton(i18n("One blank") + " = 1/32", durationGroup);

	pageWidth = new QSpinBox(1, 1024 * 1024, 1, this);
	QLabel *pageWidth_l = new QLabel(pageWidth, i18n("Page &width:"), this);

	always = new QCheckBox(i18n("Always show this dialog on export"), this);

	// Set widget layout

	Q3VBoxLayout *box = new Q3VBoxLayout(this);
	box->addWidget(durationGroup);

	Q3HBoxLayout *pageWidthBox = new Q3HBoxLayout(box);
	pageWidthBox->addWidget(pageWidth_l);
	pageWidthBox->addWidget(pageWidth);
	pageWidthBox->addStretch(1);

	box->addStretch(1);
	box->addWidget(always);
	box->activate();

	// Fill in current config
	KConfigGroup g = config->group("ASCII");
	durationGroup->setButton(g.readEntry("DurationDisplay", 3));
	pageWidth->setValue(g.readEntry("PageWidth", 72));
	always->setChecked(g.readEntry("AlwaysShow", TRUE));
}

void OptionsExportAscii::defaultBtnClicked()
{
	durationGroup->setButton(3);
	pageWidth->setValue(72);
	always->setChecked(TRUE);
}

void OptionsExportAscii::applyBtnClicked()
{
	KConfigGroup g = config->group("ASCII");
	g.writeEntry("DurationDisplay", durationGroup->id(durationGroup->selected()));
	g.writeEntry("PageWidth", pageWidth->value());
	g.writeEntry("AlwaysShow", always->isChecked());
}
