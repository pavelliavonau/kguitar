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

#include <qfileinfo.h>

#include "kguitar_shell.h"
#include "application.h"
#include "filebrowser.h"

KGuitarShell::KGuitarShell()
{
    // We already link to libkguitar !
#if 0
    // Try to find libkguitar
	KLibFactory *factory = KLibLoader::self()->factory("libkguitar");
	if (factory) {
        // Create the part
		m_kgpart = (KGuitarPart *)factory->create(this, "kguitarpart", "KParts::ReadWritePart");
	}
	else {
		kdFatal() << "*** No libkguitar found !" << endl;
		KMessageBox::error(this, i18n("No libkguitar found !"));
		exit(1);
		return;
	}
#else
	m_kgpart = new KGuitarPart(FALSE, this, "kguitarpart", this, "kguitarpart");
#endif  // if 0


  //File
	KStdAction::open (this, SLOT(slotFileOpen()), actionCollection(), "file_open");
	KStdAction::save(this, SLOT(slotFileSave()), actionCollection(), "file_save");
	KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection(), "file_saveAs");

	openRecentAct = KStdAction::openRecent(this, SLOT(openURL(const KURL&)), actionCollection(), 
									"file_openRecent");

	KStdAction::print(m_kgpart, SLOT(filePrint()), actionCollection(), "file_print");
	KStdAction::quit(this, SLOT(slotQuit()), actionCollection(), "file_quit");

	browserAct = new KAction(i18n("Browser..."), KAccel::stringToKey("Shift+B"), this, 
							 SLOT(openBrowser()), actionCollection(), "open_browser");


	showMainTBAct = KStdAction::showToolbar(this, SLOT(slotToggleMainTB()), 
											actionCollection(), "tog_mainTB");
	showMainTBAct->setText(i18n("Main Toolbar"));

	showEditTBAct = KStdAction::showToolbar(this, SLOT(slotToggleEditTB()), 
											actionCollection(), "tog_editTB");
	showEditTBAct->setText(i18n("Edit Toolbar"));


	showStatusbarAct = KStdAction::showStatusbar(this, SLOT(slotShowStatusBar()), 
												 actionCollection(), "tog_statusbar");

	connect(m_kgpart, SIGNAL(configToolBars()), SLOT(slotConfigTB()));
	connect(m_kgpart, SIGNAL(setWindowCaption(const QString&)), SLOT(slotSetCaption(const QString&)));

	setXMLFile("kguitarui.rc");

    // Set the main widget
	setCentralWidget(m_kgpart->widget());

    // Integrate its GUI
	createGUI(m_kgpart);

	readSettings();

	openRecentAct->setMaxItems(5);

	setCaption(i18n("Unnamed"));

	toolBar("mainToolBar")->setText(i18n("Main Toolbar"));
	toolBar("editToolBar")->setText(i18n("Edit Toolbar"));

	statusBar()->insertItem(QString(i18n("Bar: ")) + "1", 1);
	connect(m_kgpart->tv, SIGNAL(statusBarChanged()), SLOT(updateStatusBar()));
}

KGuitarShell::~KGuitarShell()
{
	kdDebug() << "KGuitarShell::~KGuitarShell()" << endl;
	writeSettings();
	delete m_kgpart;
}

void KGuitarShell::slotQuit()
{
	kdDebug() << "KGuitarShell::slotQuit()" << endl;
	kapp->closeAllWindows();
}

void KGuitarShell::slotSetCaption(const QString& caption)
{
	setCaption(caption);
}

bool KGuitarShell::openURL(const KURL& url)
{
	bool ret = m_kgpart->openURL(url);
	if(ret)
		openRecentAct->addURL(url);
	return ret;
}

void KGuitarShell::saveURL(const KURL& url)
{
	if (m_kgpart->saveAs(url))
		openRecentAct->addURL(url);
}

void KGuitarShell::slotFileOpen()
{
	KURL url = KFileDialog::getOpenURL(0, i18n("*.kg|KGuitar files (*.kg)\n"
											   "*.tab|ASCII files (*.tab)\n"
											   "*.mid|MIDI files (*.mid)\n"
											   "*.gtp|Guitar Pro files (*.gtp)\n"
											   "*|All files"), this);
	if (!url.isEmpty())
		openURL(url);
}

void KGuitarShell::slotFileSaveAs()
{
	KFileDialog *dlg=new KFileDialog(0, i18n("*.kg|KGuitar files (*.kg)\n"
											 "*.tab|ASCII files (*.tab)\n"
											 "*.mid|MIDI files (*.mid)\n"
											 "*.gtp|Guitar Pro files (*.gtp)\n"
											 "*.tex|MusiXTeX (*.tex)\n"
											 "*|All files"), this, 0, TRUE);
	dlg->setCaption(i18n("Save as..."));

	dlg->show();

	QString filter = dlg->currentFilter();
	QString fn = dlg->selectedFile();

	if (!fn.isEmpty()) {
		QFileInfo *fi = new QFileInfo(fn);
		if (fi->exists())
			if (KMessageBox::warningYesNo(this, i18n("This file exists! "
													 "Do you overwrite this file?")) == KMessageBox::No)
				return;
		if (fi->exists() && !fi->isWritable()) {
			KMessageBox::sorry(this, i18n("You have no permission to write this file!"));
			return;
		}

		if (filter == "*") {
			filter = fi->extension();
			filter = filter.lower();
			if (!((filter == "kg") || (filter == "mid") || (filter == "gtp") || 
				(filter == "tex") || (filter == "tab"))) {
				KMessageBox::sorry(this, i18n("Please select a Filter or add an extension."));
				return;
			}
			filter = "*." + filter;
		}
		if ((filter == "*.kg") || (filter == "*.tab") || (filter == "*.mid") ||
			(filter == "*.gtp") || (filter == "*.tex")) {
			KURL url = KURL(fn);
			saveURL(url);
		}		
	}
}

void KGuitarShell::slotFileSave()
{
	QString fn = m_kgpart->tv->sng()->filename;

	if (!fn.isEmpty()) {
		KURL url = KURL(fn);
		saveURL(url);
	}
	else
		slotFileSaveAs();
}

void KGuitarShell::openBrowser()
{
	fb = new FileBrowser(this);
	connect(fb, SIGNAL(loadFile(const KURL&)), SLOT(openBrowserURL(const KURL&)));

	fb->show();

	disconnect(fb, SIGNAL(loadFile(const KURL&)));
}

void KGuitarShell::openBrowserURL(const KURL& url)
{
	if (openURL(url)) 
		if (fb != 0L) {
			m_kgpart->tv->updateRows();
			m_kgpart->tv->repaint();
			fb->tlabel->setText(m_kgpart->tv->sng()->title);
			fb->alabel->setText(m_kgpart->tv->sng()->author);
			fb->tslabel->setText(m_kgpart->tv->sng()->transcriber);
		}
}

void KGuitarShell::slotShowStatusBar()
{
	if (showStatusbarAct->isChecked())
		statusBar()->show();
	else
		statusBar()->hide();
}

void KGuitarShell::slotToggleMainTB()
{
	KToolBar *bar = toolBar("mainToolBar");

	if (bar!=0L) {
		if (showMainTBAct->isChecked())
			bar->show();
		else
			bar->hide();
	}
}

void KGuitarShell::slotToggleEditTB()
{
	KToolBar *bar = toolBar("editToolBar");

	if (bar!=0L) {
		if (showEditTBAct->isChecked())
			bar->show();
		else
			bar->hide();
	}
}

void KGuitarShell::slotConfigTB()
{
	KEditToolbar dlg(factory());

	if (dlg.exec())
		createGUI(m_kgpart);  //ALINXFIX: If toolbar is changed, the menubar is not correct.
                              //          I can't find an other way or is this a bug in KDE 2.0 ?
}

void KGuitarShell::updateStatusBar()
{
	QString tmp;
	tmp.setNum(m_kgpart->tv->trk()->xb + 1);
	tmp = i18n("Bar: ") + tmp;
	statusBar()->changeItem(tmp, 1);
}

void KGuitarShell::readSettings()
{
	KConfig *config = KGlobal::config();

	config->setGroup("Appearance");

	showMainTBAct->setChecked(config->readBoolEntry("ShowMainTB", TRUE));
	showEditTBAct->setChecked(config->readBoolEntry("ShowEditTB", TRUE));
	toolBar("mainToolBar")->applySettings(config, "MainToolBar");
	toolBar("editToolBar")->applySettings(config, "EditToolBar");

	showStatusbarAct->setChecked(config->readBoolEntry("ShowStatusBar", TRUE));
	slotShowStatusBar();

	QSize size = config->readSizeEntry("Geometry");
	if (!size.isEmpty())
		resize(size);

	openRecentAct->loadEntries(config);
}

void KGuitarShell::writeSettings()
{
	KConfig *config = KGlobal::config();

	config->setGroup("Appearance");

	config->writeEntry("ShowMainTB", showMainTBAct->isChecked());
	config->writeEntry("ShowEditTB", showEditTBAct->isChecked());
	toolBar("mainToolBar")->saveSettings(config, "MainToolBar");
	toolBar("editToolBar")->saveSettings(config, "EditToolBar");
	config->writeEntry("ShowStatusBar", showStatusbarAct->isChecked());
	config->writeEntry("Geometry", size());

	openRecentAct->saveEntries(config);

	config->sync();
}
