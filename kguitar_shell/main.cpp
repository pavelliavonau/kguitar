#include "config.h"
#include "kguitar.h"

#include <QApplication>
#include <QCommandLineParser>
#include <KLocalizedString>
#include <QDebug>
#include <KAboutData>

#ifdef WITH_TSE3
static const char description[] = I18N_NOOP("A stringed instrument tabulature editor (with MIDI support via TSE3)");
#else
static const char description[] = I18N_NOOP("A stringed instrument tabulature editor");
#endif

static const char version[] = VERSION;

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	KAboutData about(
		"kguitar", i18n("KGuitar"), version,
		i18n(description), KAboutLicense::GPL,
		i18n("(C) 2000-2009 by KGuitar Development Team"),
		QString(), "http://kguitar.sourceforge.net"
	);

	about.addAuthor(i18n("Mikhail Yakshin AKA GreyCat"), i18n("Maintainer and main coder"), "greycat@users.sourceforge.net");

	about.addAuthor(i18n("Alex Brand AKA alinx"), QString(), "alinx@users.sourceforge.net");
	about.addAuthor(i18n("Leon Vinken"), QString(), "lvinken@users.sourceforge.net");
	about.addAuthor(i18n("Matt Malone"), QString(), "marlboro@users.sourceforge.net");
	about.addAuthor(i18n("Sylvain Vignaud"), QString(), "tfpsly@users.sourceforge.net");
	about.addCredit(i18n("Stephan Borchert"), QString(), "sborchert@users.sourceforge.net");
	about.addCredit(i18n("Juan Pablo Sousa Bravo AKA gotem"), QString(), "gotem@users.sourceforge.net");
	about.addCredit(i18n("Wilane Ousmane"), QString(), "wilane@users.sourceforge.net");
	about.addCredit(i18n("Richard G. Roberto"), QString(), "robertor@users.sourceforge.net");
	about.addCredit(i18n("Riccardo Vitelli AKA feac"), QString(), "feac@users.sourceforge.net");
	about.addCredit(i18n("Ronald Gelten"), i18n("Special thanks for allowing us to make changes to tabdefs.tex"));

	KAboutData::setApplicationData(about);

	// TODO: check cmd line functionality
	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOptions({
        {"+[URL]",
            QCoreApplication::translate("main", "Document to open")},
        {"save-as",
            QCoreApplication::translate("main", "Save document to a file (possibly converting) and quit immediately.")},
    });


	parser.process(app);

	QString saveFile;

	// see if we are starting with session management
	if (app.isSessionRestored()) {
		RESTORE(KGuitar)
	} else {
		// no session.. just start up normally
		QStringList args = parser.positionalArguments();

		// handle conversion
		saveFile = parser.value("save-as");

		if (args.count() == 0)  {
			KGuitar *widget = new KGuitar;
			widget->show();
		} else {
			for (int i = 0; i < args.count(); i++) {
				KGuitar *widget = new KGuitar;

				if(QUrl(args.at(i)).isValid())
					widget->load(args.at(i));
				else
				{
					delete widget;
					continue;
				}

				if (!saveFile.isEmpty()) {
					qDebug() << "Saving as " << saveFile << "...\n";
					widget->saveURL(QUrl(saveFile.toUtf8()));
				} else {
					widget->show();
				}
			}
		}
	}

	// quit if called just for conversion
	if (!saveFile.isEmpty()) {
		return 0;
	} else {
		return app.exec();
	}
}
