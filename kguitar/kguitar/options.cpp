#include "options.h"
#include "globaloptions.h"

#include "optionspage.h"
#include "optionsmusictheory.h"
#include "optionsmelodyeditor.h"
#include "optionsexportmusixtex.h"
#include "optionsmidi.h"
#include "optionsprinting.h"
#include "optionsexportascii.h"

#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

Options::Options(
#ifdef WITH_TSE3
				 TSE3::MidiScheduler *sch,
#endif
                 KConfig *config, QWidget *parent, char *name, bool modal)
	: KDialogBase(TreeList, i18n("Preferences"), Help|Default|Ok|Apply|Cancel,
	              Ok, parent, name, modal, TRUE)
{
	resize(530, 300);

    QFrame *optPage[OPTIONS_PAGES_NUM];

	optPage[0] = addPage(i18n("Music Theory"), 0, SmallIcon("lookandfeel"));
	optPage[1] = addPage(i18n("Melody Constructor"), 0, SmallIcon("melodyeditor"));
	optPage[2] = addPage(QStringList::split('/', i18n("Export") + "/" + i18n("MusiXTeX")),
	                     0, SmallIcon("musixtex"));
#ifdef WITH_TSE3
	optPage[3] = addPage(i18n("MIDI Devices"), 0, SmallIcon("kcmmidi"));
#endif
	optPage[4] = addPage(i18n("Printing"), 0, SmallIcon("printmgr"));
	optPage[5] = addPage(QStringList::split('/', i18n("Export") + "/" + i18n("ASCII")),
	                     0, SmallIcon("ascii"));

	optWidget[0] = new OptionsMusicTheory(config, optPage[0]);
	optWidget[1] = new OptionsMelodyEditor(config, optPage[1]);
	optWidget[2] = new OptionsExportMusixtex(config, optPage[2]);
#ifdef WITH_TSE3
	optWidget[3] = new OptionsMidi(sch, config, optPage[3]);
#endif
	optWidget[4] = new OptionsPrinting(config, optPage[4]);
	optWidget[5] = new OptionsExportAscii(config, optPage[5]);

	// Special weird layout stuff to pack everything
	for (int i = 0; i < OPTIONS_PAGES_NUM; i++) {
		if (optWidget[i]) {
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
