#include "options.h"
#include "globaloptions.h"

#include "optionspage.h"
#include "optionsmusictheory.h"
#include "optionsmelodyeditor.h"
#include "optionsexportmusixtex.h"
#include "optionsmidi.h"
#include "optionsprinting.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

#ifdef WITH_TSE3
#include <qframe.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#endif

Options::Options(
#ifdef WITH_TSE3
				 TSE3::MidiScheduler *sch,
#endif
				 QWidget *parent, char *name, bool modal)
	: KDialogBase(IconList, i18n("Preferences"), Help|Default|Ok|Apply|Cancel,
	              Ok, parent, name, modal, TRUE)
{
	resize(530, 300);

    QFrame *optPage[OPTIONS_PAGES_NUM];

	optPage[0] = addPage(i18n("Music Theory"), 0, DesktopIcon("lookandfeel", KIcon::SizeMedium));
	optPage[1] = addPage(i18n("Melody Constructor"), 0, DesktopIcon("melodyeditor", KIcon::SizeMedium));
	optPage[2] = addPage(i18n("MusiXTeX Export"), 0, DesktopIcon("musixtex", KIcon::SizeMedium));
#ifdef WITH_TSE3
	optPage[3] = addPage(i18n("MIDI"), 0, DesktopIcon("kcmmidi", KIcon::SizeMedium));
#endif
	optPage[4] = addPage(i18n("Printing"), 0, DesktopIcon("printmgr", KIcon::SizeMedium));

	optWidget[0] = new OptionsMusicTheory(optPage[0]);
	optWidget[1] = new OptionsMelodyEditor(optPage[1]);
	optWidget[2] = new OptionsExportMusixtex(optPage[2]);
#ifdef WITH_TSE3
	optWidget[3] = new OptionsMidi(sch, optPage[3]);
#endif
	optWidget[4] = new OptionsPrinting(optPage[4]);

	// Special weird layout stuff to pack everything
	for (int i = 0; i < OPTIONS_PAGES_NUM; i++) {
		if (optWidget[i])  {
			QVBoxLayout *l = new QVBoxLayout(optPage[i]);
			l->addWidget(optWidget[i]);
		}
	}

	connect(this, SIGNAL(defaultClicked()), SLOT(defaultBtnClicked()));
	connect(this, SIGNAL(okClicked()), SLOT(applyBtnClicked()));
	connect(this, SIGNAL(applyClicked()), SLOT(applyBtnClicked()));
}

// Saves options back from dialog to memory
void Options::applyBtnClicked()
{
	for (int i = 0; i < OPTIONS_PAGES_NUM; i++)
		if (optWidget[i])
			optWidget[i]->applyBtnClicked();
}

void Options::defaultBtnClicked()
{
	for (int i = 0; i < OPTIONS_PAGES_NUM; i++)
		if (optWidget[i])
			optWidget[i]->defaultBtnClicked();
}
