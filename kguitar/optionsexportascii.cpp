#include "optionsexportascii.h"

#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

OptionsExportAscii::OptionsExportAscii(KSharedConfigPtr &conf, QWidget *parent)
	: OptionsPage(conf, parent)
{
	QVBoxLayout *vbox = new QVBoxLayout;
	// Create option widgets

	durationGroup = new QGroupBox(i18n("&Duration Display"), this);
	duration = new QButtonGroup(this);
	QRadioButton* button = new QRadioButton(i18n("Fixed one blank"), durationGroup);
	duration->addButton(button, 0);
	vbox->addWidget(button);

	button = new QRadioButton(i18n("One blank") + " = 1/4", durationGroup);
	duration->addButton(button, 1);
	vbox->addWidget(button);

	button = new QRadioButton(i18n("One blank") + " = 1/8", durationGroup);
	duration->addButton(button, 2);
	vbox->addWidget(button);

	button = new QRadioButton(i18n("One blank") + " = 1/16", durationGroup);
	duration->addButton(button, 3);
	vbox->addWidget(button);

	button = new QRadioButton(i18n("One blank") + " = 1/32", durationGroup);
	duration->addButton(button, 4);
	vbox->addWidget(button);

	durationGroup->setLayout(vbox);

	pageWidth = new QSpinBox(this);
	pageWidth->setRange(1, 1024 * 1024);
	QLabel *pageWidth_l = new QLabel(i18n("Page &width:"), this);
	pageWidth_l->setBuddy(pageWidth);

	always = new QCheckBox(i18n("Always show this dialog on export"), this);

	// Set widget layout

	QVBoxLayout *box = new QVBoxLayout(this);
	box->addWidget(durationGroup);

	QHBoxLayout *pageWidthBox = new QHBoxLayout();
	pageWidthBox->addWidget(pageWidth_l);
	pageWidthBox->addWidget(pageWidth);
	pageWidthBox->addStretch(1);

	box->addLayout(pageWidthBox);

	box->addStretch(1);
	box->addWidget(always);
	box->activate();

	// Fill in current config
	KConfigGroup g = config->group("ASCII");
	duration->button(g.readEntry("DurationDisplay", 3))->setChecked(true);
	pageWidth->setValue(g.readEntry("PageWidth", 72));
	always->setChecked(g.readEntry("AlwaysShow", TRUE));
}

void OptionsExportAscii::defaultBtnClicked()
{
	duration->button(3)->setChecked(true);
	pageWidth->setValue(72);
	always->setChecked(TRUE);
}

void OptionsExportAscii::applyBtnClicked()
{
	KConfigGroup g = config->group("ASCII");
	g.writeEntry("DurationDisplay", duration->id(duration->checkedButton()));
	g.writeEntry("PageWidth", pageWidth->value());
	g.writeEntry("AlwaysShow", always->isChecked());
}
