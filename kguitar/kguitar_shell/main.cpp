#include "kguitar.h"
#include "config.h"

#include <qdir.h>

#include <kapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static KCmdLineOptions options[] = {
	{ "+[URL]", I18N_NOOP("Document to open."), 0 },
	KCmdLineLastOption
};

#ifdef WITH_TSE3
static const char *DESCRIPTION = I18N_NOOP("A stringed instrument tabulature editor (with MIDI support via TSE3)");
#else
static const char *DESCRIPTION = I18N_NOOP("A stringed instrument tabulature editor");
#endif

int main(int argc, char **argv)
{
	KAboutData about("kguitar", "KGuitar",
	                 VERSION, DESCRIPTION, KAboutData::License_GPL,
	                 "(C) 2000-2003 by KGuitar Development Team", 0,
	                 "http://kguitar.sourceforge.net");

	about.addAuthor("Mikhail Yakshin AKA GreyCat", I18N_NOOP("Maintainer and main coder"),
						"greycat@users.sourceforge.net");
	about.addAuthor("Alex Brand AKA alinx", 0, "alinx@users.sourceforge.net");
	about.addAuthor("Leon Vinken", 0, "lvinken@users.sourceforge.net");
	about.addAuthor("Matt Malone", 0, "marlboro@users.sourceforge.net");
	about.addAuthor("Sylvain Vignaud", 0, "tfpsly@users.sourceforge.net");
	about.addCredit("Stephan Borchert", 0, "sborchert@users.sourceforge.net");
	about.addCredit("Juan Pablo Sousa Bravo AKA gotem", 0, "gotem@users.sourceforge.net");
	about.addCredit("Wilane Ousmane", 0, "wilane@users.sourceforge.net");
	about.addCredit("Richard G. Roberto", 0, "robertor@users.sourceforge.net");
	about.addCredit("Riccardo Vitelli AKA feac", 0, "feac@users.sourceforge.net");
	about.addCredit(0, I18N_NOOP("Special Thanks to Ronald Gelten who\n"
	                "allowed us to make changes to tabdefs.tex"), 0);

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	// see if we are starting with session management
	if (app.isRestored()) {
		RESTORE(KGuitar)
	} else {
        // no session.. just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

		if (args->count() == 0)  {
			KGuitar *widget = new KGuitar;
			widget->show();
		} else {
			for (int i = 0; i < args->count(); i++) {
				KGuitar *widget = new KGuitar;
				widget->show();
				widget->load(args->url(i));
			}
		}
		args->clear();
	}

	return app.exec();
}
