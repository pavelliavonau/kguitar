#include "settrack.h"
#include "settabfret.h"

#include <klocale.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <knuminput.h>
#include <qcombobox.h>
#include <qlabel.h>

SetTrack::SetTrack(QWidget *parent = 0, const char *name = 0)
	: QTabDialog(parent, name, TRUE)
{
    //////////////////////////////////////////////////////////////////
    // GENERAL CONTROLS TAB
    //////////////////////////////////////////////////////////////////

	QWidget *gen = new QWidget(this);

    QGridLayout *g = new QGridLayout(gen, 5, 2, 10);

    title = new QLineEdit(gen);
    channel = new KIntNumInput(gen);
    bank = new KIntNumInput(gen);
    patch = new KIntNumInput(gen);
	mode = new QComboBox(FALSE, gen);
	mode->insertItem(i18n("Fretted instrument"));
	mode->insertItem(i18n("Drum track"));

    QLabel *title_l = new QLabel(title, i18n("&Track name:"), gen);
    QLabel *channel_l = new QLabel(bank, i18n("&Channel:"), gen);
    QLabel *bank_l = new QLabel(bank, i18n("&Bank:"), gen);
    QLabel *patch_l = new QLabel(patch, i18n("&Patch:"), gen);
    QLabel *mode_l = new QLabel(mode, i18n("&Mode:"), gen);

    g->addWidget(title_l, 0, 0);
    g->addWidget(title, 0, 1);
    g->addWidget(channel_l, 1, 0);
    g->addWidget(channel, 1, 1);
    g->addWidget(bank_l, 2, 0);
    g->addWidget(bank, 2, 1);
    g->addWidget(patch_l, 3, 0);
    g->addWidget(patch, 3, 1);
    g->addWidget(mode_l, 4, 0);
    g->addWidget(mode, 4, 1);

	for (int i = 0; i < 4; i++)
		g->addRowSpacing(i, 20);

    g->addColSpacing(0, 80);
    g->setColStretch(1, 1);

	g->activate();

	addTab(gen, i18n("&General"));

    //////////////////////////////////////////////////////////////////
    // TAB MODE SPECIFIC WIDGET
    //////////////////////////////////////////////////////////////////

    fret = new SetTabFret(this);
	addTab(fret, i18n("&Mode-specific"));

	setOkButton(i18n("OK"));
	setCancelButton(i18n("Cancel"));

    setCaption(i18n("Track properties"));
}
