#include "config.h"
#include "melodyeditor.h"
#include "fretboard.h"
#include "trackview.h"
#include "tabtrack.h"

#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>

MelodyEditor::MelodyEditor(TrackView *_tv, QWidget *parent, const char *name)
	:QWidget(parent, name)
{
	tv = _tv;

	fb = new Fretboard(tv->trk(), this);

	tonic = new QComboBox(FALSE, this);
	for (int i = 0; i < 12; i++)
		tonic->insertItem(note_name(i));

	mode = new QComboBox(FALSE, this);
	mode->insertItem(i18n("<no mode>"));
	mode->insertItem(i18n("Pentatonic"));
	mode->insertItem(i18n("Natural Major"));
	mode->insertItem(i18n("Natural Minor"));
	mode->insertItem(i18n("Melodic Minor"));
	mode->insertItem(i18n("Dorian"));
	mode->insertItem(i18n("Lydian"));
	mode->insertItem(i18n("Phrygian"));
	mode->insertItem(i18n("Locrian"));
	mode->insertItem(i18n("Mixolydian"));

	options = new QPushButton(i18n("Options..."), this);

	QLabel *tonic_l = new QLabel(tonic, i18n("&Tonic:"), this);
	QLabel *mode_l = new QLabel(mode, i18n("&Mode:"), this);

	// Full layout
	QBoxLayout *l = new QVBoxLayout(this);

	// Settings box
	QBoxLayout *lsettings = new QHBoxLayout(l, 5);
	lsettings->addWidget(tonic_l);
	lsettings->addWidget(tonic);
	lsettings->addWidget(mode_l);
	lsettings->addWidget(mode);
	lsettings->addStretch(1);
	lsettings->addWidget(options);

	// Fretboard box
	l->addWidget(fb);

	connect(fb, SIGNAL(buttonClicked(int, int, ButtonState)), tv, SLOT(setMelodyClick(int, int, ButtonState)));
	connect(tv, SIGNAL(trackChanged(TabTrack *)), fb, SLOT(setTrack(TabTrack *)));

	setCaption(i18n("Melody Constructor"));
}
