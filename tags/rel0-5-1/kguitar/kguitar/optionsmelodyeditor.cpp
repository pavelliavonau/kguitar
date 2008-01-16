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
	inlay[5] = new QRadioButton(i18n("Shark fin"), inlayGroup);

	inlayGroup->setButton(Settings::melodyEditorInlay());

	woodGroup = new QVButtonGroup(i18n("Texture"), designGroup, "texturegroup");
	wood[0] = new QRadioButton(i18n("Schematic"), woodGroup);
	wood[1] = new QRadioButton(i18n("Maple"), woodGroup);
	wood[2] = new QRadioButton(i18n("Rosewood"), woodGroup);
	wood[3] = new QRadioButton(i18n("Ebony"), woodGroup);

	woodGroup->setButton(globalMelodyEditorWood);

	l->addWidget(designGroup);

	QGroupBox *actionsGroup = new QGroupBox(3, Horizontal, i18n("Mouse button actions"), this, "actionsbox");

	QStringList labels;
	labels << i18n("Left:") << i18n("Middle:") << i18n("Right:");

	for (int i = 0; i < 3; i++) {
		(void) new QLabel(labels[i], actionsGroup);

		mouseAction[i] = new QComboBox(FALSE, actionsGroup);
		mouseAction[i]->insertItem(i18n("No action"));
		mouseAction[i]->insertItem(i18n("Set note"));
		mouseAction[i]->insertItem(i18n("Set 02 power chord"));
		mouseAction[i]->insertItem(i18n("Set 022 power chord"));
		mouseAction[i]->insertItem(i18n("Set 00 power chord"));
		mouseAction[i]->insertItem(i18n("Set 0022 power chord"));
		mouseAction[i]->insertItem(i18n("Delete note"));

		mouseAction[i]->setCurrentItem(globalMelodyEditorAction[i]);

		mouseAdvance[i] = new QCheckBox(i18n("Advance to next column"), actionsGroup);

		mouseAdvance[i]->setChecked(globalMelodyEditorAdvance[i]);
	}

	l->addWidget(actionsGroup);
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
