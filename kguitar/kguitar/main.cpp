#include <kapp.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmainwindow.h>

#include "application.h"

static KCmdLineOptions options[] = {
	{ "+file", I18N_NOOP("File to open."), 0 },
	{ 0, 0, 0 }
};

int main(int argc, char **argv)
{
	KAboutData aboutData("kguitar", I18N_NOOP("KGuitar"),
						 VERSION, DESCRIPTION, KAboutData::License_GPL,
						 "(C) 2000 by KGuitar Development Team", 0,
						 "http://kguitar.sourceforge.net");

	aboutData.addAuthor("Mikhail Yakshin AKA GreyCat", I18N_NOOP("Maintainer and main coder"),
						"GreyCat@users.sourceforge.net");
	aboutData.addAuthor("Alex Brand AKA alinx", 0, "alinx@users.sourceforge.net");
	aboutData.addCredit("Stephan Borchert", 0, "sborchert@users.sourceforge.net");
	aboutData.addCredit("Harri Haataja AKA realblades", 0, "realblades@users.sourceforge.net");
	aboutData.addCredit("Matt Malone AKA Marlboro", 0, "Marlboro@users.sourceforge.net");
	aboutData.addCredit("Wilane Ousmane", 0, "wilane@users.sourceforge.net");
	aboutData.addCredit("Richard G. Roberto", 0, "robertor@users.sourceforge.net");

	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication a;

	if (a.isRestored())
		RESTORE(ApplicationWindow)
	else {
		ApplicationWindow *kguitar = new ApplicationWindow;
		if (argc > 1) {
			KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
//			kguitar->loadFile(KURL(QDir::currentDirPath()+"/", args->arg(0)));
// GREYFIX - do the proper loading
			args->clear();
		}
// 		a.setMainWidget(kguitar);
		kguitar->show();
// 		a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
		return a.exec();
	}
}
