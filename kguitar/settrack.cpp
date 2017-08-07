#include "settrack.h"
#include "settabfret.h"
#include "settabdrum.h"
#include "settabmidi.h"

#include <KLocalizedString>
#include <qlineedit.h>
#include <QFormLayout>
#include <qcombobox.h>
#include <QDialogButtonBox>

SetTrack::SetTrack(TabTrack *trk, QWidget *parent)
	: KPageDialog(parent)
{
	setWindowTitle(i18n("Track properties"));
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
	                                                 | QDialogButtonBox::Cancel, this);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	setButtonBox(buttonBox);
	setFaceType(Tabbed);

	//////////////////////////////////////////////////////////////////
	// GENERAL CONTROLS TAB
	//////////////////////////////////////////////////////////////////

	QWidget *gen = new QWidget(this);

	QFormLayout *l = new QFormLayout(gen);

	title = new QLineEdit(gen);
	channel = new QSpinBox(gen);
	bank = new QSpinBox(gen);
	patch = new QSpinBox(gen);
	mode = new QComboBox(gen);
	mode->addItem(i18n("Fretted instrument"));
	mode->addItem(i18n("Drum track"));

	l->addRow(i18n("&Track name:"), title);
	l->addRow(i18n("&Channel:"), channel);
	l->addRow(i18n("&Bank:"), bank);
	l->addRow(i18n("&Patch:"), patch);
	l->addRow(i18n("&Mode:"), mode);

	gen->setLayout(l);

	// Fill tab with information

	title->setText(trk->name);
	//	title->setReadOnly(isBrowserView);
	channel->setValue(trk->channel);
	//	channel->setDisabled(isBrowserView);
	bank->setValue(trk->bank);
	//	bank->setDisabled(isBrowserView);
	patch->setValue(trk->patch);
	//	patch->setDisabled(isBrowserView);
	mode->setCurrentIndex(trk->trackMode());
	//	mode->setDisabled(isBrowserView);
	connect(mode, SIGNAL(activated(int)), SLOT(selectTrackMode(int)));

	track = trk;

	addPage(gen, i18n("&General"));

	//////////////////////////////////////////////////////////////////
	// TAB MIDI SPECIFIC WIDGET
	//////////////////////////////////////////////////////////////////

	QWidget *tabmidiPage = new SetTabMidi(this);

	SetTabMidi *tabmidi = (SetTabMidi *) tabmidiPage;
	//ToDo: set values from track
	tabmidi->setVolume(0);
	tabmidi->setPan(0);
	tabmidi->setReverb(0);
	tabmidi->setChorus(0);
	tabmidi->setTranspose(0);

	addPage(tabmidiPage, i18n("MIDI &effects"));

	//////////////////////////////////////////////////////////////////
	// TAB MODE SPECIFIC WIDGET
	//////////////////////////////////////////////////////////////////

	modespec = new SetTabFret(this);
	modeSpecPage = addPage(modespec, i18n("&Mode-specific"));

	// Fill tab with information
	selectTrackMode(trk->trackMode());
}

void SetTrack::selectTrackMode(int sel)
{
	switch ((TabTrack::TrackMode) sel) {
	case TabTrack::FretTab: selectFret(); break;
	case TabTrack::DrumTab: selectDrum(); break;
	}
}

void SetTrack::selectFret()
{
	removePage(modeSpecPage);
	modespec = new SetTabFret(this);
	modeSpecPage = addPage(modespec, i18n("&Mode-specific"));
	SetTabFret *fret = (SetTabFret *) modespec;

	fret->setString(track->string);
	fret->setFrets(track->frets);
	for (int i = 0; i < track->string; i++)
		fret->setTune(i, track->tune[i]);
	//	fret->setDisabled(isBrowserView);
}

void SetTrack::selectDrum()
{
	removePage(modeSpecPage);
	modespec = new SetTabDrum(this);
	modeSpecPage = addPage(modespec, i18n("&Mode-specific"));
	SetTabDrum *drum = (SetTabDrum *) modespec;

	drum->setDrums(track->string);
	for (int i = 0; i < track->string; i++)
		drum->setTune(i, track->tune[i]);
}
