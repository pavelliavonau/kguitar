#include "optionsmelodyeditor.h"
#include "settings.h"

#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QVBoxLayout>

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>

OptionsMelodyEditor::OptionsMelodyEditor(KSharedConfigPtr &conf, QWidget *parent)
	: OptionsPage(conf, parent)
{
	// GREYFIX!!!
	int globalMelodyEditorWood;
	int globalMelodyEditorAction[3];
	bool globalMelodyEditorAdvance[3];

	KConfigGroup g = config->group("MelodyEditor");
	globalMelodyEditorWood = g.readEntry("Wood",  2);
	globalMelodyEditorAction[0] = g.readEntry("Action0", 1);
	globalMelodyEditorAdvance[0] = g.readEntry("Advance0", false);
	globalMelodyEditorAction[1] = g.readEntry("Action1", 3);
	globalMelodyEditorAdvance[1] = g.readEntry("Advance1", true);
	globalMelodyEditorAction[2] = g.readEntry("Action2", 1);
	globalMelodyEditorAdvance[2] = g.readEntry("Advance2", true);

	QVBoxLayout *l = new QVBoxLayout(this);

	QGroupBox *designGroup = new QGroupBox(i18n("Design"), this);
	QHBoxLayout *l_design = new QHBoxLayout;
	QGroupBox *inlayGroup = new QGroupBox(i18n("Inlays"), designGroup);
	QVBoxLayout *l_inlay = new QVBoxLayout;

	inlay[0] = new QRadioButton(i18n("None"), inlayGroup);
	inlay[1] = new QRadioButton(i18n("Center dots"), inlayGroup);
	inlay[2] = new QRadioButton(i18n("Side dots"), inlayGroup);
	inlay[3] = new QRadioButton(i18n("Blocks"), inlayGroup);
	inlay[4] = new QRadioButton(i18n("Trapezoid"), inlayGroup);
	inlay[5] = new QRadioButton(i18n("Shark fin"), inlayGroup);

	for (int i = 0; i < 6; i++)
		l_inlay->addWidget(inlay[i]);
	l_inlay->addStretch(1);
	inlayGroup->setLayout(l_inlay);

	inlay[Settings::melodyEditorInlay()]->setChecked(true);

	QGroupBox *woodGroup = new QGroupBox(i18n("Texture"), designGroup);
	QVBoxLayout *l_wood = new QVBoxLayout;
	wood[0] = new QRadioButton(i18n("Schematic"), woodGroup);
	wood[1] = new QRadioButton(i18n("Maple"), woodGroup);
	wood[2] = new QRadioButton(i18n("Rosewood"), woodGroup);
	wood[3] = new QRadioButton(i18n("Ebony"), woodGroup);

	for (int i = 0; i < 4; i++)
		l_wood->addWidget(wood[i]);
	l_wood->addStretch(1);
	woodGroup->setLayout(l_wood);

	wood[globalMelodyEditorWood]->setChecked(true);

	l_design->addWidget(woodGroup);
	l_design->addWidget(inlayGroup);
	designGroup->setLayout(l_design);
	l->addWidget(designGroup);

	QGroupBox *actionsGroup = new QGroupBox(i18n("Mouse button actions"), this);
	QFormLayout *l_actions = new QFormLayout;

	QStringList labels;
	labels << i18n("Left:") << i18n("Middle:") << i18n("Right:");

	int i = 0;
	foreach (QString label, labels) {
		mouseAction[i] = new QComboBox(actionsGroup);
		mouseAction[i]->addItem(i18n("No action"));
		mouseAction[i]->addItem(i18n("Set note"));
		mouseAction[i]->addItem(i18n("Set 02 power chord"));
		mouseAction[i]->addItem(i18n("Set 022 power chord"));
		mouseAction[i]->addItem(i18n("Set 00 power chord"));
		mouseAction[i]->addItem(i18n("Set 0022 power chord"));
		mouseAction[i]->addItem(i18n("Delete note"));
		mouseAction[i]->setCurrentIndex(globalMelodyEditorAction[i]);

		mouseAdvance[i] = new QCheckBox(i18n("Advance to next column"), actionsGroup);
		mouseAdvance[i]->setChecked(globalMelodyEditorAdvance[i]);

		l_actions->addRow(label, mouseAction[i]);
		l_actions->addRow(mouseAdvance[i]);

		i++;
	}

	actionsGroup->setLayout(l_actions);
	l->addWidget(actionsGroup);
}

void OptionsMelodyEditor::defaultBtnClicked()
{
	inlay[1]->setChecked(true);
	wood[2]->setChecked(true);

	mouseAction[0]->setCurrentIndex(1);
	mouseAction[1]->setCurrentIndex(3);
	mouseAction[2]->setCurrentIndex(1);

	mouseAdvance[0]->setChecked(false);
	mouseAdvance[1]->setChecked(true);
	mouseAdvance[2]->setChecked(true);
}

void OptionsMelodyEditor::applyBtnClicked()
{
	KConfigGroup g = config->group("MelodyEditor");

	int inlaySel = -1;
	for (int i = 0; i < 6; i++) {
		if (inlay[i]->isChecked()) {
			inlaySel = i;
			break;
		}
	}

	int woodSel = -1;
	for (int i = 0; i < 4; i++) {
		if (wood[i]->isChecked()) {
			woodSel = i;
			break;
		}
	}

	g.writeEntry("Inlay", inlaySel);
	g.writeEntry("Wood", woodSel);
	g.writeEntry("Action0", mouseAction[0]->currentIndex());
	g.writeEntry("Advance0", mouseAdvance[0]->isChecked());
	g.writeEntry("Action1", mouseAction[1]->currentIndex());
	g.writeEntry("Advance1", mouseAdvance[1]->isChecked());
	g.writeEntry("Action2", mouseAction[2]->currentIndex());
	g.writeEntry("Advance2", mouseAdvance[2]->isChecked());
}
