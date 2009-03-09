#include "optionsexportmusixtex.h"
#include "settings.h"

#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <klocale.h>
#include <kconfig.h>

OptionsExportMusixtex::OptionsExportMusixtex(KConfig *conf, QWidget *parent, const char *name)
	: OptionsPage(conf, parent, name)
{
	// Create option widgets

	Q3VButtonGroup *layoutGroup = new Q3VButtonGroup(i18n("MusiXTeX Layout"), this);
	showBarNumber  = new QCheckBox(i18n("Show Bar Number"), layoutGroup);
	showStr        = new QCheckBox(i18n("Show Tuning"), layoutGroup);
	showPageNumber = new QCheckBox(i18n("Show Page Number"), layoutGroup);

	exportModeGroup = new Q3VButtonGroup(i18n("Export as..."), this);
	exportMode[0] = new QRadioButton(i18n("Tabulature"), exportModeGroup);
	exportMode[1] = new QRadioButton(i18n("Notes"), exportModeGroup);

	tabSizeGroup = new Q3VButtonGroup(i18n("Tab Size"), this);
	tabSize[0] = new QRadioButton(i18n("Smallest"), tabSizeGroup);
	tabSize[1] = new QRadioButton(i18n("Small"), tabSizeGroup);
	tabSize[2] = new QRadioButton(i18n("Normal"), tabSizeGroup);
	tabSize[3] = new QRadioButton(i18n("Big"), tabSizeGroup);

	always = new QCheckBox(i18n("Always show this dialog on export"), this);

	// Set widget layout

	Q3VBoxLayout *box = new Q3VBoxLayout(this);
	box->addWidget(layoutGroup);
	box->addWidget(tabSizeGroup);
	box->addWidget(exportModeGroup);
	box->addStretch(1);
	box->addWidget(always);
	box->activate();

	// Fill in current config

	tabSizeGroup->setButton(Settings::texTabSize());
	showBarNumber->setChecked(Settings::texShowBarNumber());
	showStr->setChecked(Settings::texShowStr());
	showPageNumber->setChecked(Settings::texShowPageNumber());
	exportModeGroup->setButton(Settings::texExportMode());
	always->setChecked(config->readBoolEntry("AlwaysShow", TRUE));
}

void OptionsExportMusixtex::defaultBtnClicked()
{
	tabSizeGroup->setButton(2);
	showBarNumber->setChecked(TRUE);
	showStr->setChecked(TRUE);
	showPageNumber->setChecked(TRUE);
	exportModeGroup->setButton(0);
}

void OptionsExportMusixtex::applyBtnClicked()
{
	config->setGroup("MusiXTeX");
	config->writeEntry("TabSize", tabSizeGroup->id(tabSizeGroup->selected()));
	config->writeEntry("ShowBarNumber", showBarNumber->isChecked());
	config->writeEntry("ShowStr", showStr->isChecked());
	config->writeEntry("ShowPageNumber", showPageNumber->isChecked());
	config->writeEntry("ExportMode", exportModeGroup->id(exportModeGroup->selected()));
	config->writeEntry("AlwaysShow", always->isChecked());
}
