#include "config.h"

#include <qdir.h>

#include <kapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kurl.h>

#include "kguitar_shell.h"
#include "application.h"

static KCmdLineOptions options[] = {
	{ "+file", I18N_NOOP("File to open."), 0 },
	{ 0, 0, 0 }
};

#ifdef WITH_TSE3
static const char *DESCRIPTION = I18N_NOOP("A stringed instrument tabulature editor (with MIDI support via TSE3)");
#else
static const char *DESCRIPTION = I18N_NOOP("A stringed instrument tabulature editor");
#endif

int main(int argc, char **argv)
{
	KAboutData aboutData("kguitar", "KGuitar",
						 VERSION, DESCRIPTION, KAboutData::License_GPL,
						 "(C) 2000, 2001, 2002 by KGuitar Development Team", 0,
						 "http://kguitar.sourceforge.net");

	aboutData.addAuthor("Mikhail Yakshin AKA GreyCat", I18N_NOOP("Maintainer and main coder"),
						"greycat@users.sourceforge.net");
	aboutData.addAuthor("Alex Brand AKA alinx", 0, "alinx@users.sourceforge.net");
	aboutData.addCredit("Stephan Borchert", 0, "sborchert@users.sourceforge.net");
	aboutData.addCredit("Juan Pablo Sousa Bravo AKA gotem", 0, "gotem@users.sourceforge.net");
	aboutData.addCredit("Wilane Ousmane", 0, "wilane@users.sourceforge.net");
	aboutData.addCredit("Richard G. Roberto", 0, "robertor@users.sourceforge.net");
	aboutData.addCredit("Riccardo Vitelli AKA feac", 0, "feac@users.sourceforge.net");
    aboutData.addCredit(0, I18N_NOOP("Special Thanks to Ronald Gelten who\n"
                        "allowed us to make changes to tabdefs.tex"), 0);

	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication a;

	if (a.isRestored())
		RESTORE(KGuitarShell)
	else {
		KGuitarShell *shell = new KGuitarShell;
		if (argc > 1) {
			KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
			if (args->count()){
				KURL url(QDir::currentDirPath()+"/", args->arg(0));
				shell->openURL(url);
			}
			args->clear();
		}
		shell->show();
		return a.exec();
	}
}
