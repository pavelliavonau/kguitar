#include "optionsmusictheory.h"
#include "globaloptions.h"

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>

#include <klocale.h>

OptionsMusicTheory::OptionsMusicTheory(QWidget *parent, const char *name)
	: OptionsPage(parent, name)
{
	// Dominant 7th name selection group

	maj7Group = new QButtonGroup(i18n("Dominant 7th"), this);
	maj7[0] = new QRadioButton("7M", maj7Group);
	maj7[1] = new QRadioButton("maj7", maj7Group);
	maj7[2] = new QRadioButton("dom7", maj7Group);

	QVBoxLayout *vb1 = new QVBoxLayout(maj7Group, 15, 10);
	vb1->addSpacing(5); // Cosmetic space
	for (int i = 0; i < 3; i++)
		vb1->addWidget(maj7[i]);
	vb1->activate();

    // Chord step alterations selection group

	flatGroup = new QButtonGroup(i18n("Alterations"), this);
	flatGroup->setMinimumSize(150, 110);
	flat[0] = new QRadioButton(i18n("-/+ symbols"), flatGroup);
	flat[1] = new QRadioButton(i18n("b/# symbols"), flatGroup);

	QVBoxLayout *vb2 = new QVBoxLayout(flatGroup, 15, 10);
	vb2->addSpacing(5); // Cosmetic space
	vb2->addWidget(flat[0]);
	vb2->addWidget(flat[1]);
	vb2->activate();

	QHBoxLayout *vbcd = new QHBoxLayout(this, 15, 10);
	vbcd->addWidget(maj7Group);
	vbcd->addWidget(flatGroup);
	vbcd->activate();
}

void OptionsMusicTheory::defaultBtnClicked()
{
	maj7Group->setButton(0);
	flatGroup->setButton(0);
}

void OptionsMusicTheory::applyBtnClicked()
{
	if (maj7[0]->isChecked())  globalMaj7 = 0;
	if (maj7[1]->isChecked())  globalMaj7 = 1;
	if (maj7[2]->isChecked())  globalMaj7 = 2;

	if (flat[0]->isChecked())  globalFlatPlus = 0;
	if (flat[1]->isChecked())  globalFlatPlus = 1;
}
