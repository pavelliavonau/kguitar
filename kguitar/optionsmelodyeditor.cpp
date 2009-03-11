#include "optionsmelodyeditor.h"
#include "settings.h"

#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <q3hgroupbox.h>
#include <q3vgroupbox.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <klocale.h>
#include <q3buttongroup.h>
#include <q3groupbox.h>
#include <kconfig.h>
#include <kconfiggroup.h>

OptionsMelodyEditor::OptionsMelodyEditor(KSharedConfigPtr &conf, QWidget *parent, const char *name)
	: OptionsPage(conf, parent, name)
{
	// GREYFIX!!!
	int globalMelodyEditorWood;
	int globalMelodyEditorAction[3];
	bool globalMelodyEditorAdvance[3];

	KConfigGroup g = config->group("MelodyEditor");
	globalMelodyEditorWood = g.readEntry("Wood",  2);
	globalMelodyEditorAction[0] = g.readEntry("Action0", 1);
	globalMelodyEditorAdvance[0] = g.readEntry("Advance0", FALSE);
	globalMelodyEditorAction[1] = g.readEntry("Action1", 3);
	globalMelodyEditorAdvance[1] = g.readEntry("Advance1", TRUE);
	globalMelodyEditorAction[2] = g.readEntry("Action2", 1);
	globalMelodyEditorAdvance[2] = g.readEntry("Advance2", TRUE);

	Q3VBoxLayout *l = new Q3VBoxLayout(this, 0, -1, "main");

	Q3HGroupBox *designGroup = new Q3HGroupBox(i18n("Design"), this, "designbox");

	inlayGroup = new Q3VButtonGroup(i18n("Inlays"), designGroup, "inlaygroup");
	inlay[0] = new QRadioButton(i18n("None"), inlayGroup);
	inlay[1] = new QRadioButton(i18n("Center dots"), inlayGroup);
	inlay[2] = new QRadioButton(i18n("Side dots"), inlayGroup);
	inlay[3] = new QRadioButton(i18n("Blocks"), inlayGroup);
	inlay[4] = new QRadioButton(i18n("Trapezoid"), inlayGroup);
	inlay[5] = new QRadioButton(i18n("Shark fin"), inlayGroup);

	inlayGroup->setButton(Settings::melodyEditorInlay());

	woodGroup = new Q3VButtonGroup(i18n("Texture"), designGroup, "texturegroup");
	wood[0] = new QRadioButton(i18n("Schematic"), woodGroup);
	wood[1] = new QRadioButton(i18n("Maple"), woodGroup);
	wood[2] = new QRadioButton(i18n("Rosewood"), woodGroup);
	wood[3] = new QRadioButton(i18n("Ebony"), woodGroup);

	woodGroup->setButton(globalMelodyEditorWood);

	l->addWidget(designGroup);

	Q3VGroupBox *actionsGroup = new Q3VGroupBox(i18n("Mouse button actions"), this, "actionsbox");

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
	KConfigGroup g = config->group("MelodyEditor");
	g.writeEntry("Inlay", inlayGroup->id(inlayGroup->selected()));
	g.writeEntry("Wood", woodGroup->id(woodGroup->selected()));
	g.writeEntry("Action0", mouseAction[0]->currentItem());
	g.writeEntry("Advance0", mouseAdvance[0]->isChecked());
	g.writeEntry("Action1", mouseAction[1]->currentItem());
	g.writeEntry("Advance1", mouseAdvance[1]->isChecked());
	g.writeEntry("Action2", mouseAction[2]->currentItem());
	g.writeEntry("Advance2", mouseAdvance[2]->isChecked());
}
