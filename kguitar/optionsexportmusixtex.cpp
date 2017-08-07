#include "optionsexportmusixtex.h"
#include "settings.h"

#include <QRadioButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

OptionsExportMusixtex::OptionsExportMusixtex(KSharedConfigPtr &conf, QWidget *parent)
	: OptionsPage(conf, parent)
{
	QVBoxLayout *vbox = new QVBoxLayout;

	// Create option widgets

	QGroupBox *layoutGroup = new QGroupBox(i18n("MusiXTeX Layout"), this);
	showBarNumber  = new QCheckBox(i18n("Show Bar Number"), layoutGroup);
	showStr        = new QCheckBox(i18n("Show Tuning"), layoutGroup);
	showPageNumber = new QCheckBox(i18n("Show Page Number"), layoutGroup);

	vbox->addWidget(showBarNumber);
	vbox->addWidget(showStr);
	vbox->addWidget(showPageNumber);
	layoutGroup->setLayout(vbox);

	vbox = new QVBoxLayout;

	exportModeGroup = new QGroupBox(i18n("Export as..."), this);
	exportMode = new QButtonGroup(this);
	QRadioButton* button = new QRadioButton(i18n("Tabulature"), exportModeGroup);
	exportMode->addButton(button, 0);
	vbox->addWidget(button);

	button = new QRadioButton(i18n("Notes"), exportModeGroup);
	exportMode->addButton(button, 1);
	vbox->addWidget(button);
	vbox->addStretch();

	exportModeGroup->setLayout(vbox);

	vbox = new QVBoxLayout;

	tabSizeGroup = new QGroupBox(i18n("Tab Size"), this);
	tabSize = new QButtonGroup(this);
	button = new QRadioButton(i18n("Smallest"), tabSizeGroup);
	tabSize->addButton(button, 0);
	vbox->addWidget(button);
	button = new QRadioButton(i18n("Small"), tabSizeGroup);
	tabSize->addButton(button, 1);
	vbox->addWidget(button);
	button = new QRadioButton(i18n("Normal"), tabSizeGroup);
	tabSize->addButton(button, 2);
	vbox->addWidget(button);
	button = new QRadioButton(i18n("Big"), tabSizeGroup);
	tabSize->addButton(button, 3);
	vbox->addWidget(button);
	vbox->addStretch();

	tabSizeGroup->setLayout(vbox);

	always = new QCheckBox(i18n("Always show this dialog on export"), this);

	// Set widget layout

	QVBoxLayout *box = new QVBoxLayout(this);
	box->addWidget(layoutGroup);
	box->addWidget(tabSizeGroup);
	box->addWidget(exportModeGroup);
	box->addStretch(1);
	box->addWidget(always);
	box->activate();

	// Fill in current config

	tabSize->button(Settings::texTabSize())->setChecked(true);
	showBarNumber->setChecked(Settings::texShowBarNumber());
	showStr->setChecked(Settings::texShowStr());
	showPageNumber->setChecked(Settings::texShowPageNumber());
	exportMode->button(Settings::texExportMode())->setChecked(true);
	always->setChecked(config->group("MusiXTeX").readEntry("AlwaysShow", TRUE));
}

void OptionsExportMusixtex::defaultBtnClicked()
{
	tabSize->button(2)->setChecked(true);
	showBarNumber->setChecked(TRUE);
	showStr->setChecked(TRUE);
	showPageNumber->setChecked(TRUE);
	exportMode->button(0)->setChecked(true);
}

void OptionsExportMusixtex::applyBtnClicked()
{
	KConfigGroup g = config->group("MusiXTeX");
	g.writeEntry("TabSize", tabSize->id(tabSize->checkedButton()));
	g.writeEntry("ShowBarNumber", showBarNumber->isChecked());
	g.writeEntry("ShowStr", showStr->isChecked());
	g.writeEntry("ShowPageNumber", showPageNumber->isChecked());
	g.writeEntry("ExportMode", exportMode->id(exportMode->checkedButton()));
	g.writeEntry("AlwaysShow", always->isChecked());
}
