#include "optionsmelodyeditor.h"
#include "settings.h"

#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qhgroupbox.h>
#include <qlayout.h>

#include <klocale.h>
#include <qvbuttongroup.h>
#include <qgroupbox.h>
#include <kconfig.h>

OptionsMelodyEditor::OptionsMelodyEditor(KConfig *conf, QWidget *parent, const char *name)
	: OptionsPage(conf, parent, name)
{
	// GREYFIX!!!
	int globalMelodyEditorWood;
	int globalMelodyEditorAction[3];
	bool globalMelodyEditorAdvance[3];

	config->setGroup("MelodyEditor");
	globalMelodyEditorWood = config->readNumEntry("Wood",  2);
	globalMelodyEditorAction[0] = config->readNumEntry("Action0", 1);
	globalMelodyEditorAdvance[0] = config->readBoolEntry("Advance0", FALSE);
	globalMelodyEditorAction[1] = config->readNumEntry("Action1", 3);
	globalMelodyEditorAdvance[1] = config->readBoolEntry("Advance1", TRUE);
	globalMelodyEditorAction[2] = config->readNumEntry("Action2", 1);
	globalMelodyEditorAdvance[2] = config->readBoolEntry("Advance2", TRUE);

	QVBoxLayout *l = new QVBoxLayout(this, 0, -1, "main");

	QHGroupBox *designGroup = new QHGroupBox(i18n("Design"), this, "designbox");

	inlayGroup = new QVButtonGroup(i18n("Inlays"), designGroup, "inlaygroup");
	inlay[0] = new QRadioButton(i18n("None"), inlayGroup);
	inlay[1] = new QRadioButton(i18n("Center dots"), inlayGroup);
	inlay[2] = new QRadioButton(i18n("Side dots"), inlayGroup);
	inlay[3] = new QRadioButton(i18n("Blocks"), inlayGroup);
	inlay[4] = new QRadioButton(i18n("Trapezoid"), inlayGroup);

	inlayGroup->setButton(Settings::melodyEditorInlay());

	woodGroup = new QVButtonGroup(i18n("Texture"), designGroup, "texturegroup");
	wood[0] = new QRadioButton(i18n("Schematic"), woodGroup);
	wood[1] = new QRadioButton(i18n("Maple"), woodGroup);
	wood[2] = new QRadioButton(i18n("Rosewood"), woodGroup);
	wood[3] = new QRadioButton(i18n("Ebony"), woodGroup);

	woodGroup->setButton(globalMelodyEditorWood);

	l->addWidget(designGroup);

	QGridLayout *gl = new QGridLayout(this, 3, 3, 5, 11);

	for (int i = 0; i < 3; i++) {
		mouseAction[i] = new QComboBox(FALSE, this);
		gl->addWidget(mouseAction[i], i, 1);
		mouseAction[i]->insertItem(i18n("No action"));
		mouseAction[i]->insertItem(i18n("Set note"));
		mouseAction[i]->insertItem(i18n("Set 02 power chord"));
		mouseAction[i]->insertItem(i18n("Set 022 power chord"));
		mouseAction[i]->insertItem(i18n("Set 00 power chord"));
		mouseAction[i]->insertItem(i18n("Set 0022 power chord"));
		mouseAction[i]->insertItem(i18n("Delete note"));

		mouseAction[i]->setCurrentItem(globalMelodyEditorAction[i]);

		mouseAdvance[i] = new QCheckBox(i18n("Advance to next column"), this);
		gl->addWidget(mouseAdvance[i], i, 2);

		mouseAdvance[i]->setChecked(globalMelodyEditorAdvance[i]);
	}

	gl->addWidget(new QLabel(mouseAction[0], i18n("Left button:"), this), 0, 0);
	gl->addWidget(new QLabel(mouseAction[1], i18n("Middle button:"), this), 1, 0);
	gl->addWidget(new QLabel(mouseAction[2], i18n("Right button:"), this), 2, 0);

	l->addLayout(gl);

	resize(300, 300);
}

void OptionsMelodyEditor::defaultBtnClicked()
{
	inlayGroup->setButton(1);
	woodGroup->setButton(2);

	mouseAction[0]->setCurrentItem(1);
	mouseAction[1]->setCurrentItem(3);
	mouseAction[2]->setCurrentItem(1);

	mouseAdvance[0]->setChecked(FALSE);
	mouseAdvance[1]->setChecked(TRUE);
	mouseAdvance[2]->setChecked(TRUE);
}

void OptionsMelodyEditor::applyBtnClicked()
{
	config->setGroup("MelodyEditor");
	config->writeEntry("Inlay", inlayGroup->id(inlayGroup->selected()));
	config->writeEntry("Wood", woodGroup->id(woodGroup->selected()));
	config->writeEntry("Action0", mouseAction[0]->currentItem());
	config->writeEntry("Advance0", mouseAdvance[0]->isChecked());
	config->writeEntry("Action1", mouseAction[1]->currentItem());
	config->writeEntry("Advance1", mouseAdvance[1]->isChecked());
	config->writeEntry("Action2", mouseAction[2]->currentItem());
	config->writeEntry("Advance2", mouseAdvance[2]->isChecked());
}
