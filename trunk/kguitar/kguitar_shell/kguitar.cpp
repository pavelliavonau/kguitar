#include "kguitar.h"

#include <kiconloader.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <kstdaction.h>
#include <kedittoolbar.h>
#include <kaccel.h>
#include <kdebug.h>
#include <kcommand.h>
#include <kstatusbar.h>
#include <kkeydialog.h>

#include <qfileinfo.h>

KGuitar::KGuitar(): KParts::MainWindow(0L, "KGuitar")
{
	// set the shell's ui resource file
	setXMLFile("kguitar_shell.rc");

	// setup shell-specific actions
	setupActions();

	// and a status bar
	statusBar()->message(QString(i18n("Bar: ")) + "1", 1);

	// this routine will find and load KGuitar KPart.
	KLibFactory *factory = KLibLoader::self()->factory("libkguitarpart");
	if (factory) {
		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
		kgpart = static_cast<KParts::ReadWritePart *>(
			factory->create(this, "kguitar_part",
			                "KParts::ReadWritePart")
			);
	} else {
		// if we couldn't find our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, i18n("Could not find KGuitar KPart! Check your installation!"));
		kapp->quit();
		// we return here, cause kapp->quit() only means "exit the
		// next time we enter the event loop...
		return;
	}

	// Alternative method of loading KGuitarPart when we do static linking
 	// kgpart = new KGuitarPart(this, "kguitarpart", this, "kguitarpart", NULL);

	setCentralWidget(kgpart->widget());
	createGUI(kgpart);

	// undo / redo
	cmdHistory = new KCommandHistory(actionCollection());

	setCaption(i18n("Unnamed"));

	toolBar("mainToolBar")->setText(i18n("Main Toolbar"));
	toolBar("editToolBar")->setText(i18n("Edit Toolbar"));

	setAutoSaveSettings();
}

KGuitar::~KGuitar()
{
}

void KGuitar::setupActions()
{
	(void) KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
	(void) KStdAction::open(this, SLOT(fileOpen()), actionCollection());

	openRecentAct = KStdAction::openRecent(this, SLOT(load(const KURL&)), actionCollection());
	openRecentAct->setMaxItems(5);

	(void) KStdAction::quit(kapp, SLOT(quit()), actionCollection(), "file_quit");

	setStandardToolBarMenuEnabled(TRUE);
	KStdAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());
	
	createStandardStatusBarAction();

	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

// Call KPart's URL opening and maintain recent files list
void KGuitar::load(const KURL& url)
{
	bool ret = kgpart->openURL(url);
	if (ret)
		openRecentAct->addURL(url);
}

// Call KPart's saving URL and maintain recent files list
void KGuitar::saveURL(const KURL& url)
{
	if (kgpart->saveAs(url))
		openRecentAct->addURL(url);
}

void KGuitar::fileNew()
{
	if (!kgpart->url().isEmpty() || kgpart->isModified())
		(new KGuitar)->show();
}

void KGuitar::fileOpen()
{
	KURL url =
		KFileDialog::getOpenURL(0,
		                        "*.kg *.gp4 *.gp3 *.mid *.tab *.xml|" + i18n("All music files") + "\n"
		                        "*.kg|" + i18n("KGuitar files") + " (*.kg)\n"
		                        "*.tab|" + i18n("ASCII files") + " (*.tab)\n"
		                        "*.mid|" + i18n("MIDI files") + " (*.mid)\n"
		                        "*.gp4|" + i18n("Guitar Pro 4 files") + " (*.gp4)\n"
		                        "*.gp3|" + i18n("Guitar Pro 3 files") + " (*.gp3)\n"
		                        "*.xml|" + i18n("MusicXML files") + " (*.xml)\n"
		                        "*|" + i18n("All files"), this);

	if (url.isEmpty() == false) {
		// if the document is not in its initial state, we open a new window
		if (kgpart->url().isEmpty() && !kgpart->isModified()) {
			// we open the file in this window...
			load(url);
		} else {
			// we open the file in a new window...
			KGuitar* newWin = new KGuitar;
			newWin->load(url);
			newWin->show();
		}
	}
}

void KGuitar::saveProperties(KConfig* /*config*/)
{
	// the 'config' object points to the session managed
	// config file.	 anything you write here will be available
	// later when this app is restored
}

void KGuitar::readProperties(KConfig* /*config*/)
{
	// the 'config' object points to the session managed
	// config file.	 this function is automatically called whenever
	// the app is being restored.  read in here whatever you wrote
	// in 'saveProperties'
}

void KGuitar::slotToggleMainTB()
{
	KToolBar *bar = toolBar("mainToolBar");

	if (bar!=0L) {
		if (showMainTBAct->isChecked())
			bar->show();
		else
			bar->hide();
	}
}

void KGuitar::slotToggleEditTB()
{
	KToolBar *bar = toolBar("editToolBar");

	if (bar!=0L) {
		if (showEditTBAct->isChecked())
			bar->show();
		else
			bar->hide();
	}
}

void KGuitar::optionsConfigureKeys()
{
	KKeyDialog dlg(TRUE, this);
	dlg.insert(actionCollection(), "kguitar_shell.rc");
	dlg.insert(kgpart->actionCollection(), "kguitar_part.rc");
	(void) dlg.configure(TRUE);
}

void KGuitar::optionsConfigureToolbars()
{
	saveMainWindowSettings(KGlobal::config(), autoSaveGroup());

	// use the standard toolbar editor
	KEditToolbar dlg(factory());
	connect(&dlg, SIGNAL(newToolbarConfig()),
	        this, SLOT(applyNewToolbarConfig()));
	dlg.exec();
}

void KGuitar::applyNewToolbarConfig()
{
	applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}
