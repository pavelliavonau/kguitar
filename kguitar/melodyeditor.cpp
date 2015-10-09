#include "melodyeditor.h"
#include "fretboard.h"
#include "trackview.h"
#include "data/tabtrack.h"
#include "options.h"
#include "optionsmelodyeditor.h"
#include "settings.h"

#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <klocale.h>
#include <qapplication.h>
#include <KDialog>
#include <KVBox>

MelodyEditor::MelodyEditor(TrackView *_tv, QWidget *parent)
	: QWidget(parent)
// 			 WType_TopLevel | WStyle_Customize |
// 	         WStyle_StaysOnTop | WStyle_NormalBorder |
// 	         WStyle_Title | WStyle_MinMax | WStyle_SysMenu)
{
	tv = _tv;

	fb = new Fretboard(tv->trk(), this);

	tonic = new QComboBox(this);
	for (int i = 0; i < 12; i++)
		tonic->addItem(Settings::noteName(i));

	mode = new QComboBox(this);
	mode->addItem(i18n("<no mode>"));
	mode->addItem(i18n("Pentatonic"));
	mode->addItem(i18n("Natural Major"));
	mode->addItem(i18n("Natural Minor"));
	mode->addItem(i18n("Harmonic Major"));
	mode->addItem(i18n("Harmonic Minor"));
	mode->addItem(i18n("Melodic Major"));
	mode->addItem(i18n("Melodic Minor"));
	mode->addItem(i18n("Mixolydian"));
	mode->addItem(i18n("Lydian"));
	mode->addItem(i18n("Dorian"));
	mode->addItem(i18n("Phrygian"));
	mode->addItem(i18n("Locrian"));

	options = new QPushButton(i18n("Options..."), this);

	QLabel *tonic_l = new QLabel(i18n("&Tonic:"), this);
	tonic_l->setBuddy(tonic);

	QLabel *mode_l = new QLabel(i18n("&Mode:"), this);
	mode_l->setBuddy(mode);

	// Full layout
	QBoxLayout *l = new QVBoxLayout(this);

	// Settings box
	QBoxLayout *lsettings = new QHBoxLayout();
	lsettings->setSpacing(5);
	lsettings->addWidget(tonic_l);
	lsettings->addWidget(tonic);
	lsettings->addWidget(mode_l);
	lsettings->addWidget(mode);
	lsettings->addStretch(1);
	lsettings->addWidget(options);

	l->addLayout(lsettings);

	// Fretboard box
	l->addWidget(fb);

	connect(fb, SIGNAL(buttonPress(int, int, Qt::ButtonState)),
	        tv, SLOT(melodyEditorPress(int, int, Qt::ButtonState)));
	connect(fb, SIGNAL(buttonRelease(Qt::ButtonState)), tv, SLOT(melodyEditorRelease(Qt::ButtonState)));
	connect(tv, SIGNAL(trackChanged(TabTrack *)), fb, SLOT(setTrack(TabTrack *)));
	connect(tv, SIGNAL(columnChanged()), fb, SLOT(update()));
	connect(options, SIGNAL(clicked()), SLOT(optionsDialog()));
	connect(tonic, SIGNAL(highlighted(int)), fb, SLOT(setTonic(int)));
	connect(mode, SIGNAL(highlighted(int)), fb, SLOT(setMode(int)));

// 	installEventFilter(this);

	setWindowTitle(i18n("Melody Editor"));
}

void MelodyEditor::drawBackground()
{
	fb->drawBackground();
}

void MelodyEditor::optionsDialog()
{
	KDialog opDialog;
	opDialog.setCaption(i18n("Melody Constructor"));
	opDialog.setModal(true);
	opDialog.setButtons(KDialog::Help|KDialog::Default|KDialog::Ok|
            KDialog::Apply|KDialog::Cancel);
	KVBox *box = new KVBox(&opDialog);
	opDialog.setMainWidget(box);
	OptionsMelodyEditor op(Settings::config, (QFrame *) box);
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
