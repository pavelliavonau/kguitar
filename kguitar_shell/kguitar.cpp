#include "kguitar.h"
#include "kguitar.moc"

#include <kaction.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kshortcutsdialog.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kstandardaction.h>
#include <kstatusbar.h>
#include <kurl.h>
#include <klocale.h>
#include <krecentfilesaction.h>
#include <k3command.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>

#include <QApplication>

KGuitar::KGuitar(): KParts::MainWindow()
{
	// set the shell's ui resource file
	setXMLFile("kguitar_shell.rc");

	// setup shell-specific actions
	setupActions();

	// and a status bar
	statusBar()->showMessage(QString(i18n("Bar: ")) + "1");

	// this routine will find and load our Part.  it finds the Part by
	// name which is a bad idea usually.. but it's alright in this
	// case since our Part is made for this Shell
	KLibFactory *factory = KLibLoader::self()->factory("libkguitarpart");
	if (factory) {
		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
		kgpart = static_cast<KParts::ReadWritePart *>(factory->create(this, "KGuitarPart"));

		if (kgpart) {
			// tell the KParts::MainWindow that this is indeed the main widget
			setCentralWidget(kgpart->widget());

			// and integrate the part's GUI with the shell's
			setupGUI();
		}
	} else {
		// if we couldn't find our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, i18n("Could not find our Part!"));
		qApp->quit();
		// we return here, cause qApp->quit() only means "exit the
		// next time we enter the event loop...
		return;
	}

	// Alternative method of loading KGuitarPart when we do static linking
 	// kgpart = new KGuitarPart(this, "kguitarpart", this, "kguitarpart", NULL);

	// undo / redo
	cmdHistory = new K3CommandHistory(actionCollection());

	setCaption(i18n("Unnamed"));

//	toolBar("mainToolBar")->setText(i18n("Main Toolbar"));
//	toolBar("editToolBar")->setText(i18n("Edit Toolbar"));

	// apply the saved mainwindow settings, if any, and ask the mainwindow
	// to automatically save settings if changed: window size, toolbar
	// position, icon size, etc.
	setAutoSaveSettings();
}


KGuitar::~KGuitar()
{
}

void KGuitar::setupActions()
{
	KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
	KStandardAction::open(this, SLOT(fileOpen()), actionCollection());

	openRecentAct = KStandardAction::openRecent(this, SLOT(load(const KUrl&)), actionCollection());
	openRecentAct->setMaxItems(5);

	KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());

	setStandardToolBarMenuEnabled(TRUE);
	KStandardAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection());

	createStandardStatusBarAction();

	KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
}

// Call KPart's URL opening and maintain recent files list
void KGuitar::load(const KUrl& url)
{
	bool ret = kgpart->openUrl(url);
	if (ret)
		openRecentAct->addUrl(url);
}

// Call KPart's saving URL and maintain recent files list
void KGuitar::saveURL(const KUrl& url)
{
	if (kgpart->saveAs(url))
		openRecentAct->addUrl(url);
}

void KGuitar::fileNew()
{
	if (!kgpart->url().isEmpty() || kgpart->isModified())
		(new KGuitar)->show();
}

void KGuitar::fileOpen()
{
	KUrl url = KFileDialog::getOpenUrl(KUrl(),
		"*.kg *.gp4 *.gp3 *.mid *.tab *.xml|" + i18n("All music files") + "\n"
		"*.kg|" + i18n("KGuitar files") + " (*.kg)\n"
		"*.tab|" + i18n("ASCII files") + " (*.tab)\n"
		"*.mid|" + i18n("MIDI files") + " (*.mid)\n"
		"*.gp4|" + i18n("Guitar Pro 4 files") + " (*.gp4)\n"
		"*.gp3|" + i18n("Guitar Pro 3 files") + " (*.gp3)\n"
		"*.xml|" + i18n("MusicXML files") + " (*.xml)\n"
		"*|" + i18n("All files"),
		this
	);

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

void KGuitar::saveProperties(KConfigGroup & /*config*/)
{
	// the 'config' object points to the session managed
	// config file.	 anything you write here will be available
	// later when this app is restored
}

void KGuitar::readProperties(const KConfigGroup & /*config*/)
{
	// the 'config' object points to the session managed
	// config file.	 this function is automatically called whenever
	// the app is being restored.  read in here whatever you wrote
	// in 'saveProperties'
}

void KGuitar::slotToggleMainTB()
{
	QToolBar *bar = toolBar("mainToolBar");

	if (bar!=0L) {
		if (showMainTBAct->isChecked())
			bar->show();
		else
			bar->hide();
	}
}

void KGuitar::slotToggleEditTB()
{
	QToolBar *bar = toolBar("editToolBar");

	if (bar!=0L) {
		if (showEditTBAct->isChecked())
			bar->show();
		else
			bar->hide();
	}
}

void KGuitar::optionsConfigureKeys()
{
	KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);
	dlg.addCollection(actionCollection(), "kguitar_shell.rc");
	dlg.addCollection(kgpart->actionCollection(), "kguitar_part.rc");
	(void) dlg.configure(true);
}

void KGuitar::applyNewToolbarConfig()
{
	applyMainWindowSettings(autoSaveConfigGroup());
}
