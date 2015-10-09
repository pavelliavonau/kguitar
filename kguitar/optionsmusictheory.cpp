#include "optionsmusictheory.h"

#include <QRadioButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>

OptionsMusicTheory::OptionsMusicTheory(KSharedConfigPtr &conf, QWidget *parent)
	: OptionsPage(conf, parent)
{

	QGridLayout* grid = new QGridLayout(this);
	// Create option widgets

	QVBoxLayout *box = new QVBoxLayout;

	// Dominant 7th name selection group

	maj7Group = new QGroupBox(i18n("Dominant 7th"), this);
	maj7 = new QButtonGroup(this);

	QRadioButton* button = new QRadioButton("7M", maj7Group);
	maj7->addButton(button, 0);
	box->addWidget(button);

	button = new QRadioButton("maj7", maj7Group);
	maj7->addButton(button, 1);
	box->addWidget(button);

	button = new QRadioButton("dom7", maj7Group);
	maj7->addButton(button, 2);
	box->addWidget(button);

	box->addStretch();
	maj7Group->setLayout(box);

	grid->addWidget(maj7Group, 0, 0);
	// Chord step alterations selection group

	box = new QVBoxLayout;

	flatGroup = new QGroupBox(i18n("Alterations"), this);
	flat = new QButtonGroup(this);

	button = new QRadioButton(i18n("-/+ symbols"), flatGroup);
	flat->addButton(button, 0);
	box->addWidget(button);

	button = new QRadioButton(i18n("b/# symbols"), flatGroup);
	flat->addButton(button, 1);
	box->addWidget(button);

	box->addStretch();
	flatGroup->setLayout(box);

	grid->addWidget(flatGroup, 1, 0);
	// Note naming conventions

	box = new QVBoxLayout;

	noteNameGroup = new QGroupBox(i18n("Note naming"), this);
	noteName = new QButtonGroup(this);

	button = new QRadioButton(i18n("American, sharps"), noteNameGroup);
	noteName->addButton(button, 0);
	box->addWidget(button);

	button = new QRadioButton(i18n("American, flats"), noteNameGroup);
	noteName->addButton(button, 1);
	box->addWidget(button);

	button = new QRadioButton(i18n("American, mixed"), noteNameGroup);
	noteName->addButton(button, 2);
	box->addWidget(button);

	button = new QRadioButton(i18n("European, sharps"), noteNameGroup);
	noteName->addButton(button, 3);
	box->addWidget(button);

	button = new QRadioButton(i18n("European, flats"), noteNameGroup);
	noteName->addButton(button, 4);
	box->addWidget(button);

	button = new QRadioButton(i18n("European, mixed"), noteNameGroup);
	noteName->addButton(button, 5);
	box->addWidget(button);

	button = new QRadioButton(i18n("Jazz, sharps"), noteNameGroup);
	noteName->addButton(button, 6);
	box->addWidget(button);

	button = new QRadioButton(i18n("Jazz, flats"), noteNameGroup);
	noteName->addButton(button, 7);
	box->addWidget(button);

	button = new QRadioButton(i18n("Jazz, mixed"), noteNameGroup);
	noteName->addButton(button, 8);
	box->addWidget(button);

	box->addStretch();
	noteNameGroup->setLayout(box);

	grid->addWidget(noteNameGroup, 0, 1, 2, 1);

	connect(noteName->button(6), SIGNAL(clicked()), this, SLOT(jazzWarning()));
	connect(noteName->button(7), SIGNAL(clicked()), this, SLOT(jazzWarning()));
	connect(noteName->button(8), SIGNAL(clicked()), this, SLOT(jazzWarning()));

	setLayout(grid);

	// Fill in current config
	KConfigGroup g = config->group("General");
	maj7->button(g.readEntry("Maj7", 0))->setChecked(true);
	flat->button(g.readEntry("FlatPlus", 0))->setChecked(true);
	noteName->button(g.readEntry("NoteNames", 2))->setChecked(true);
}

void OptionsMusicTheory::defaultBtnClicked()
{
	maj7->button(0)->setChecked(true);
	flat->button(0)->setChecked(true);
	noteName->button(2)->setChecked(true);
}

void OptionsMusicTheory::applyBtnClicked()
{
	KConfigGroup g = config->group("General");
	g.writeEntry("Maj7", maj7->id(maj7->checkedButton()));
	g.writeEntry("FlatPlus", flat->id(flat->checkedButton()));
	g.writeEntry("NoteNames", noteName->id(noteName->checkedButton()));
}

bool OptionsMusicTheory::jazzWarning()
{
	return KMessageBox::warningYesNo(this,
									 i18n("Jazz note names are very special and should be "
										  "used only if really know what you do. Usage of jazz "
										  "note names without a purpose would confuse or mislead "
										  "anyone reading the music who did not have a knowledge "
										  "of jazz note naming.\n\n"
										  "Are you sure you want to use jazz notes?")) == KMessageBox::Yes;
}
