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

#include <qfileinfo.h>
#include <qclipboard.h>

#include "kguitar_shell.h"
#include "application.h"
#include "filebrowser.h"
#include "trackview.h"
#include "trackdrag.h"
#include "tabsong.h"
#include "melodyeditor.h"

KGuitarShell::KGuitarShell()
{
	// Undo / Redo
	cmdHistory = new KCommandHistory(actionCollection());

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
	m_kgpart = new KGuitarPart(FALSE, cmdHistory, this, "kguitarpart", this, "kguitarpart");
#endif  // if 0

	//File
	(void) KStdAction::open(this, SLOT(slotFileOpen()), actionCollection(), "file_open");
	(void) KStdAction::save(this, SLOT(slotFileSave()), actionCollection(), "file_save");
	(void) KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection(), "file_saveAs");

	openRecentAct = KStdAction::openRecent(this, SLOT(openURL(const KURL&)), actionCollection(),
	                                       "file_openRecent");

	(void) KStdAction::print(m_kgpart, SLOT(filePrint()), actionCollection(), "file_print");
	(void) KStdAction::quit(this, SLOT(slotQuit()), actionCollection(), "file_quit");

	browserAct = new KAction(i18n("Browser..."), KAccel::stringToKey("Shift+B"), this,
	                         SLOT(openBrowser()), actionCollection(), "open_browser");

	// Cut-n-Paste
	(void) KStdAction::cut(m_kgpart->sv, SLOT(slotCut()), actionCollection(), "edit_cut");
	(void) KStdAction::copy(m_kgpart->sv, SLOT(slotCopy()), actionCollection(), "edit_copy");
	pasteAct = KStdAction::paste(m_kgpart->sv, SLOT(slotPaste()), actionCollection(), "edit_paste");
	(void) KStdAction::selectAll(m_kgpart->sv, SLOT(slotSelectAll()), actionCollection(), "edit_selectAll");

	showMainTBAct = KStdAction::showToolbar(this, SLOT(slotToggleMainTB()),
	                                        actionCollection(), "tog_mainTB");
	showMainTBAct->setText(i18n("Main Toolbar"));

	showEditTBAct = KStdAction::showToolbar(this, SLOT(slotToggleEditTB()),
	                                        actionCollection(), "tog_editTB");
	showEditTBAct->setText(i18n("Edit Toolbar"));


	showStatusbarAct = KStdAction::showStatusbar(this, SLOT(slotShowStatusBar()),
	                                             actionCollection(), "tog_statusbar");

	showMelodyEditorAct = new KToggleAction(i18n("Show Melody Editor"),
	                                        KAccel::stringToKey("Shift+M"),
	                                        this, SLOT(slotShowMelodyEditor()),
	                                        actionCollection(), "view_melodyEditor");

	connect(m_kgpart, SIGNAL(configToolBars()), SLOT(slotConfigTB()));
	connect(m_kgpart, SIGNAL(setWindowCaption(const QString&)), SLOT(slotSetCaption(const QString&)));
	connect(QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(slotClipboardDataChanged()));

	setXMLFile("kguitarui.rc");

	// Set the main widget
	setCentralWidget(m_kgpart->widget());

	// Integrate its GUI
	createGUI(m_kgpart);

	readSettings();
	slotClipboardDataChanged();

	openRecentAct->setMaxItems(5);

	setCaption(i18n("Unnamed"));

	toolBar("mainToolBar")->setText(i18n("Main Toolbar"));
	toolBar("editToolBar")->setText(i18n("Edit Toolbar"));

	statusBar()->insertItem(QString(i18n("Bar: ")) + "1", 1);
	connect(m_kgpart->sv->tv, SIGNAL(statusBarChanged()), SLOT(updateStatusBar()));
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

// Call KPart's URL opening and maintain recent files list
bool KGuitarShell::openURL(const KURL& url)
{
	bool ret = m_kgpart->openURL(url);
	if (ret)
		openRecentAct->addURL(url);
	return ret;
}

// Call KPart's saving URL and maintain recent files list
void KGuitarShell::saveURL(const KURL& url)
{
	if (m_kgpart->saveAs(url))
		openRecentAct->addURL(url);
}

void KGuitarShell::slotFileOpen()
{
	openURL(KFileDialog::getOpenURL(0,
	                                "*.kg *.gtp *.gp3 *.mid *.tab *.xml|" + i18n("All music files") + "\n"
	                                "*.kg|" + i18n("KGuitar files") + " (*.kg)\n"
	                                "*.tab|" + i18n("ASCII files") + " (*.tab)\n"
	                                "*.mid|" + i18n("MIDI files") + " (*.mid)\n"
	                                "*.gtp|" + i18n("Guitar Pro files") + " (*.gtp)\n"
	                                "*.gp3|" + i18n("Guitar Pro 3 files") + " (*.gp3)\n"
	                                "*.xml|" + i18n("MusicXML files") + " (*.xml)\n"
	                                "*|" + i18n("All files"), this));
}

void KGuitarShell::slotFileSaveAs()
{
	KFileDialog dlg(0,
	                "*.kg|" + i18n("KGuitar files") + " (*.kg)\n"
	                "*.tab|" + i18n("ASCII files") + " (*.tab)\n"
	                "*.mid|" + i18n("MIDI files") + " (*.mid)\n"
	                "*.tse3|" + i18n("TSE3MDL files") + " (*.tse3)\n"
	                "*.gtp|" + i18n("Guitar Pro files") + " (*.gtp)\n"
	                "*.gp3|" + i18n("Guitar Pro 3 files") + " (*.gp3)\n"
	                "*.xml|" + i18n("MusicXML files") + " (*.xml)\n"
	                "*.tex|" + i18n("MusiXTeX") + " (*.tex)\n")
	                "*|" + i18n("All files"), this, 0, TRUE);
	dlg.setCaption(i18n("Save as..."));

	if (dlg.exec() == QDialog::Accepted) {
		QString filter = dlg.currentFilter();
		QString fn = dlg.selectedFile();

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
			if (!((filter == "kg") || (filter == "mid") || (filter == "gtp") || (filter == "gp3") ||
				  (filter == "tex") || (filter == "tab") || (filter == "xml") || (filter == "tse3"))) {
				KMessageBox::sorry(this, i18n("Please select a filter or add an extension."));
				return;
			}
			filter = "*." + filter;
		}

		if ((filter == "*.kg") || (filter == "*.tab") || (filter == "*.mid") ||
			(filter == "*.gtp") || (filter == "*.gp3") || (filter == "*.tex") || (filter == "*.xml") || (filter = "*.tse3")) {
			KURL url = KURL(fn);
			saveURL(url);
		} else {
			KMessageBox::sorry(this, i18n("Unknown format: %1").arg(filter));
		}
	}
}

void KGuitarShell::slotFileSave()
{
	slotFileSaveAs();
//	m_kgpart->save();
//  	QString fn = m_kgpart->m_file;
//  	if (!fn.isEmpty()) {
//  		KURL url = KURL(fn);
//  		saveURL(url);
//  	} else {
//  		slotFileSaveAs();
//  	}
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
			m_kgpart->sv->tv->updateRows();
			m_kgpart->sv->repaint();
			fb->tlabel->setText(m_kgpart->sv->sng()->title);
			fb->alabel->setText(m_kgpart->sv->sng()->author);
			fb->tslabel->setText(m_kgpart->sv->sng()->transcriber);
		}
}

void KGuitarShell::slotShowStatusBar()
{
	if (showStatusbarAct->isChecked())
		statusBar()->show();
	else
		statusBar()->hide();
}

void KGuitarShell::slotShowMelodyEditor()
{
	if (showMelodyEditorAct->isChecked())
		m_kgpart->sv->me->show();
	else
		m_kgpart->sv->me->show();
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
	tmp.setNum(m_kgpart->sv->tv->trk()->xb + 1);
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

	showMelodyEditorAct->setChecked(config->readBoolEntry("ShowMelodyEditor", TRUE));
	slotShowMelodyEditor();

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
	config->writeEntry("ShowMelodyEditor", showMelodyEditorAct->isChecked());
	config->writeEntry("ShowStatusBar", showStatusbarAct->isChecked());
	config->writeEntry("Geometry", size());

	openRecentAct->saveEntries(config);

	config->sync();
}

void KGuitarShell::slotClipboardDataChanged()
{
	pasteAct->setEnabled(TrackDrag::canDecode(QApplication::clipboard()->data()));
}
