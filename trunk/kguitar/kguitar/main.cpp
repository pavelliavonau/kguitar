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
						 "(C) 2000 by KGuitar Development Team");
	
	aboutData.addAuthor("Mikhail Yakshin AKA GreyCat", 0, "GreyCat@users.sourceforge.net");
	
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
