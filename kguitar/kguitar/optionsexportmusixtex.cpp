#include "optionsexportmusixtex.h"
#include "globaloptions.h"

#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include <klocale.h>

OptionsExportMusixtex::OptionsExportMusixtex(QWidget *parent, const char *name)
	: OptionsPage(parent, name)
{
	texLyGroup = new QVButtonGroup(i18n("MusiXTeX Layout"), this);
	showbarnumb = new QCheckBox(i18n("Show Bar Number"), texLyGroup);
	showstr = new QCheckBox(i18n("Show Tuning"), texLyGroup);
	showpagenumb = new QCheckBox(i18n("Show Page Number"), texLyGroup);

	texExpGroup = new QVButtonGroup(i18n("Export as..."), this);
	expmode[0] = new QRadioButton(i18n("Tabulature"), texExpGroup);
	expmode[1] = new QRadioButton(i18n("Notes"), texExpGroup);

	texSizeGroup = new QVButtonGroup(i18n("Tab Size"), this);
	tabsize[0] = new QRadioButton(i18n("Smallest"), texSizeGroup);
	tabsize[1] = new QRadioButton(i18n("Small"), texSizeGroup);
	tabsize[2] = new QRadioButton(i18n("Normal"), texSizeGroup);
	tabsize[3] = new QRadioButton(i18n("Big"), texSizeGroup);

	QHBoxLayout *vbtex = new QHBoxLayout(this, 15, 10);
	vbtex->addWidget(texLyGroup);
	vbtex->addWidget(texSizeGroup);
	vbtex->addWidget(texExpGroup);
	vbtex->activate();

	texSizeGroup->setButton(globalTabSize);
	showbarnumb->setChecked(globalShowBarNumb);
	showstr->setChecked(globalShowStr);
	showpagenumb->setChecked(globalShowPageNumb);
	texExpGroup->setButton(globalTexExpMode);
}

void OptionsExportMusixtex::defaultBtnClicked()
{
	texSizeGroup->setButton(2);
	showbarnumb->setChecked(TRUE);
	showstr->setChecked(TRUE);
	showpagenumb->setChecked(TRUE);
	texExpGroup->setButton(0);
}

void OptionsExportMusixtex::applyBtnClicked()
{
	for (int i = 0; i <= 3; i++)
		if (tabsize[i]->isChecked())  globalTabSize = i;

	globalShowBarNumb = showbarnumb->isChecked();
	globalShowStr = showstr->isChecked();
	globalShowPageNumb = showpagenumb->isChecked();

	if (expmode[0]->isChecked()) globalTexExpMode = 0;
	if (expmode[1]->isChecked()) globalTexExpMode = 1;
}
