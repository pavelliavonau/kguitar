#include "optionsmusictheory.h"

#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3HBoxLayout>

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>

OptionsMusicTheory::OptionsMusicTheory(KSharedConfigPtr &conf, QWidget *parent, const char *name)
	: OptionsPage(conf, parent, name)
{
	// Create option widgets

	// Dominant 7th name selection group

	maj7Group = new Q3VButtonGroup(i18n("Dominant 7th"), this);
	maj7[0] = new QRadioButton("7M", maj7Group);
	maj7[1] = new QRadioButton("maj7", maj7Group);
	maj7[2] = new QRadioButton("dom7", maj7Group);

	// Chord step alterations selection group

	flatGroup = new Q3VButtonGroup(i18n("Alterations"), this);
	flat[0] = new QRadioButton(i18n("-/+ symbols"), flatGroup);
	flat[1] = new QRadioButton(i18n("b/# symbols"), flatGroup);

	// Note naming conventions

	noteNameGroup = new Q3VButtonGroup(i18n("Note naming"), this);
	noteName[0] = new QRadioButton(i18n("American, sharps"), noteNameGroup);
	noteName[1] = new QRadioButton(i18n("American, flats"), noteNameGroup);
	noteName[2] = new QRadioButton(i18n("American, mixed"), noteNameGroup);
	noteName[3] = new QRadioButton(i18n("European, sharps"), noteNameGroup);
	noteName[4] = new QRadioButton(i18n("European, flats"), noteNameGroup);
	noteName[5] = new QRadioButton(i18n("European, mixed"), noteNameGroup);
	noteName[6] = new QRadioButton(i18n("Jazz, sharps"), noteNameGroup);
	noteName[7] = new QRadioButton(i18n("Jazz, flats"), noteNameGroup);
	noteName[8] = new QRadioButton(i18n("Jazz, mixed"), noteNameGroup);

	connect(noteName[6], SIGNAL(clicked()), this, SLOT(jazzWarning()));
	connect(noteName[7], SIGNAL(clicked()), this, SLOT(jazzWarning()));
	connect(noteName[8], SIGNAL(clicked()), this, SLOT(jazzWarning()));

	// Set widget layout

	Q3HBoxLayout *box = new Q3HBoxLayout(this);

	Q3VBoxLayout *chordbox = new Q3VBoxLayout(box);
	chordbox->addWidget(maj7Group);
	chordbox->addWidget(flatGroup);

	box->addWidget(noteNameGroup);

	// Fill in current config

	KConfigGroup g = config->group("General");
	maj7Group->setButton(g.readEntry("Maj7", 0));
	flatGroup->setButton(g.readEntry("FlatPlus", 0));
	noteNameGroup->setButton(g.readEntry("NoteNames", 2));
}

void OptionsMusicTheory::defaultBtnClicked()
{
	maj7Group->setButton(0);
	flatGroup->setButton(0);
	noteNameGroup->setButton(2);
}

void OptionsMusicTheory::applyBtnClicked()
{
	KConfigGroup g = config->group("General");
	g.writeEntry("Maj7", maj7Group->id(maj7Group->selected()));
	g.writeEntry("FlatPlus", flatGroup->id(flatGroup->selected()));
	g.writeEntry("NoteNames", noteNameGroup->id(noteNameGroup->selected()));
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
