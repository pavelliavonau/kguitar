#include "optionsmelodyeditor.h"
#include "globaloptions.h"

#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qhgroupbox.h>
#include <qlayout.h>

#include <klocale.h>
#include <qvbuttongroup.h>
#include <qgroupbox.h>

int globalMelodyEditorInlay;
int globalMelodyEditorWood;
int globalMelodyEditorAction[3];
bool globalMelodyEditorAdvance[3];

OptionsMelodyEditor::OptionsMelodyEditor(QWidget *parent, const char *name)
	: OptionsPage(parent, name)
{
    QVBoxLayout *l = new QVBoxLayout(this, 0, -1, "main");

	QHGroupBox *designGroup = new QHGroupBox(i18n("Design"), this, "designbox");

	inlayGroup = new QVButtonGroup(i18n("Inlays"), designGroup, "inlaygroup");
	inlay[0] = new QRadioButton(i18n("None"), inlayGroup);
	inlay[1] = new QRadioButton(i18n("Center dots"), inlayGroup);
	inlay[2] = new QRadioButton(i18n("Side dots"), inlayGroup);
	inlay[3] = new QRadioButton(i18n("Blocks"), inlayGroup);
	inlay[4] = new QRadioButton(i18n("Trapezoid"), inlayGroup);

	inlayGroup->setButton(globalMelodyEditorInlay);

	woodGroup = new QVButtonGroup(i18n("Texture"), designGroup, "texturegroup");
	wood[0] = new QRadioButton(i18n("Schematic"), woodGroup);
	wood[1] = new QRadioButton(i18n("Maple"), woodGroup);
	wood[2] = new QRadioButton(i18n("Rosewood"), woodGroup);
	wood[3] = new QRadioButton(i18n("Ebony"), woodGroup);

	woodGroup->setButton(globalMelodyEditorWood);

    l->addWidget(designGroup);

	QGridLayout *gl = new QGridLayout(this, 3, 3);

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
	for (int i = 0; i < 5; i++)
		if (inlay[i]->isChecked())  globalMelodyEditorInlay = i;
	for (int i = 0; i < 4; i++)
		if (wood[i]->isChecked())  globalMelodyEditorWood = i;
	for (int i = 0; i < 3; i++) {
		globalMelodyEditorAction[i] = mouseAction[i]->currentItem();
		globalMelodyEditorAdvance[i] = mouseAdvance[i]->isChecked();
	}
}
