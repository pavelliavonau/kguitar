#include "config.h"
#include "kguitar.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include "kguitar.h"

#include <Q3CString>

#ifdef WITH_TSE3
static const char description[] = I18N_NOOP("A stringed instrument tabulature editor (with MIDI support via TSE3)");
#else
static const char description[] = I18N_NOOP("A stringed instrument tabulature editor");
#endif

static const char version[] = VERSION;

int main(int argc, char **argv)
{
	KAboutData about(
		"kguitar", 0, ki18n("KGuitar"), version,
		ki18n(description), KAboutData::License_GPL,
		ki18n("(C) 2000-2009 by KGuitar Development Team"),
		KLocalizedString(), 0, "http://kguitar.sourceforge.net"
	);

	about.addAuthor(ki18n("Mikhail Yakshin AKA GreyCat"), ki18n("Maintainer and main coder"), "greycat@users.sourceforge.net");

	about.addAuthor(ki18n("Alex Brand AKA alinx"), KLocalizedString(), "alinx@users.sourceforge.net");
	about.addAuthor(ki18n("Leon Vinken"), KLocalizedString(), "lvinken@users.sourceforge.net");
	about.addAuthor(ki18n("Matt Malone"), KLocalizedString(), "marlboro@users.sourceforge.net");
	about.addAuthor(ki18n("Sylvain Vignaud"), KLocalizedString(), "tfpsly@users.sourceforge.net");
	about.addCredit(ki18n("Stephan Borchert"), KLocalizedString(), "sborchert@users.sourceforge.net");
	about.addCredit(ki18n("Juan Pablo Sousa Bravo AKA gotem"), KLocalizedString(), "gotem@users.sourceforge.net");
	about.addCredit(ki18n("Wilane Ousmane"), KLocalizedString(), "wilane@users.sourceforge.net");
	about.addCredit(ki18n("Richard G. Roberto"), KLocalizedString(), "robertor@users.sourceforge.net");
	about.addCredit(ki18n("Riccardo Vitelli AKA feac"), KLocalizedString(), "feac@users.sourceforge.net");
	about.addCredit(ki18n("Ronald Gelten"), ki18n("Special thanks for allowing us to make changes to tabdefs.tex"));

	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions options;
	options.add("+[URL]", ki18n("Document to open"));
	options.add("save-as <URL>", ki18n("Save document to a file (possibly converting) and quit immediately."));
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	QString saveFile = NULL;

	// see if we are starting with session management
	if (app.isSessionRestored()) {
		RESTORE(KGuitar)
	} else {
		// no session.. just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

		// handle conversion
		saveFile = args->getOption("save-as");

		if (args->count() == 0)  {
			KGuitar *widget = new KGuitar;
			widget->show();
		} else {
			for (int i = 0; i < args->count(); i++) {
				KGuitar *widget = new KGuitar;
				widget->load(args->url(i));

				if (saveFile != NULL) {
//					kdDebug() << "Saving as " << saveFile << "...\n";
					widget->saveURL(args->url(i));
				} else {
					widget->show();
				}
			}
		}
		args->clear();
	}

	// quit if called just for conversion
	if (saveFile != NULL) {
		return 0;
	} else {
		return app.exec();
	}
}
