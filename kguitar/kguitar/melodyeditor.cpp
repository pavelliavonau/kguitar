#include "config.h"
#include "melodyeditor.h"
#include "fretboard.h"
#include "trackview.h"
#include "tabtrack.h"
#include "options.h"
#include "optionsmelodyeditor.h"

#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>
#include <qapplication.h>

MelodyEditor::MelodyEditor(TrackView *_tv, QWidget *parent, const char *name)
	:QWidget(parent, name)
// 			 WType_TopLevel | WStyle_Customize |
// 	         WStyle_StaysOnTop | WStyle_NormalBorder |
// 	         WStyle_Title | WStyle_MinMax | WStyle_SysMenu)
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

	connect(fb, SIGNAL(buttonPress(int, int, ButtonState)), tv, SLOT(melodyEditorPress(int, int, ButtonState)));
	connect(fb, SIGNAL(buttonRelease(ButtonState)), tv, SLOT(melodyEditorRelease(ButtonState)));
	connect(tv, SIGNAL(trackChanged(TabTrack *)), fb, SLOT(setTrack(TabTrack *)));
	connect(tv, SIGNAL(columnChanged()), fb, SLOT(update()));
	connect(options, SIGNAL(clicked()), SLOT(optionsDialog()));

// 	installEventFilter(this);

	setCaption(i18n("Melody Constructor"));
}

void MelodyEditor::drawBackground()
{
    fb->drawBackground();
}

void MelodyEditor::optionsDialog()
{
	KDialogBase opDialog(0, 0, TRUE, i18n("Melody Editor"),
	                     KDialogBase::Help|KDialogBase::Default|KDialogBase::Ok|
	                     KDialogBase::Apply|KDialogBase::Cancel, KDialogBase::Ok);
	OptionsMelodyEditor op;
	opDialog.setMainWidget(&op);
    op.show();
	connect(&opDialog, SIGNAL(defaultClicked()), &op, SLOT(defaultBtnClicked()));
	connect(&opDialog, SIGNAL(okClicked()), &op, SLOT(applyBtnClicked()));
	connect(&opDialog, SIGNAL(applyClicked()), &op, SLOT(applyBtnClicked()));
	opDialog.exec();
	drawBackground();
}
// Special event filter that translates all keypresses to main widget,
// i.e. TrackView
// bool MelodyEditor::eventFilter(QObject *o, QEvent *e)
// {
// 	if (e->type() == QEvent::KeyPress) {
// 		QKeyEvent *k = (QKeyEvent *) e;
// 		printf("Ate key press %d\n", k->key());
// 		QEvent ce(*e);
// 		QApplication::sendEvent(tv, &ce);
// 		return TRUE;
// 	} else {
// 		return FALSE;
// 	}
// }
