#include "options.h"

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
#include <kconfigdialog.h>

#include <qlayout.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3Frame>

Options::Options(
#ifdef WITH_TSE3
                 TSE3::MidiScheduler *sch,
#endif
                 KSharedConfigPtr &config, QWidget *parent)
	: KPageDialog(parent)
{
	setCaption(i18n("Configure"));
	setButtons(Help|Default|Ok|Apply|Cancel);
	setFaceType(KPageDialog::Tree);
	KPageWidgetItem *optPage[OPTIONS_PAGES_NUM];

	optWidget[0] = new OptionsMusicTheory(config);
	optWidget[1] = new OptionsMelodyEditor(config);
	optWidget[2] = new OptionsExportMusixtex(config);
#ifdef WITH_TSE3
	optWidget[3] = new OptionsMidi(sch, config);
#else
	optWidget[3] = NULL;
#endif
	optWidget[4] = new OptionsPrinting(config);
	optWidget[5] = new OptionsExportAscii(config);

	optPage[0] = addPage(optWidget[0], i18n("Music Theory"));
//	, 0, SmallIcon("lookandfeel"));
	optPage[1] = addPage(optWidget[1], i18n("Melody Constructor"));
//	, 0, SmallIcon("melodyeditor"));
//	optPage[2] = addPage(optWidget[2], QStringList::split('/', i18n("Export") + "/" + i18n("MusiXTeX")));
	optPage[2] = addPage(optWidget[2], i18n("Export") + "/" + i18n("MusiXTeX"));
//    0, SmallIcon("ascii"));
//	,
//	                     0, SmallIcon("musixtex"));
#ifdef WITH_TSE3
	optPage[3] = addPage(optWidget[3], i18n("MIDI Devices"));
//, SmallIcon("kcmmidi"));
#endif
	optPage[4] = addPage(optWidget[4], i18n("Printing"));
//	, 0, SmallIcon("printmgr"));
//	optPage[5] = addPage(optWidget[5], QStringList::split('/', i18n("Export") + "/" + i18n("ASCII")));
	optPage[5] = addPage(optWidget[5], i18n("Export") + "/" + i18n("ASCII"));
//	                     0, SmallIcon("ascii"));

	// Special weird layout stuff to pack everything
	// GREYTODO: delete if not needed
//	for (int i = 0; i < OPTIONS_PAGES_NUM; i++) {
//		if (optWidget[i]) {
//			Q3VBoxLayout *l = new Q3VBoxLayout(optPage[i]);
//			l->addWidget(optWidget[i]);
//		}
//	}

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
