#include "application.h"

#include "trackview.h"
#include "chord.h"
#include "tabsong.h"
#include "setsong.h"
#include "settrack.h"
#include "settabfret.h"
#include "options.h"
#include "filebrowser.h"

#include <qpopupmenu.h>

#include <kapp.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kaccel.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include <qpixmap.h>
#include <qkeycode.h>
#include <qstatusbar.h>
#include <qprinter.h>

#include <qlineedit.h>
#include <qmultilinedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

// Global variables - real declarations

// General
int globalMaj7;
int globalFlatPlus;
int globalNoteNames;

// MusiXTeX
int globalTabSize;
bool globalShowBarNumb;
bool globalShowStr;
bool globalShowPageNumb;

// Appearance
bool globalShowMainTB;
bool globalShowEditTB;
int globalMainTBPos;
int globalEditTBPos;

// ALSA
int globalAlsaClient;
int globalAlsaPort;

ApplicationWindow::ApplicationWindow(): KMainWindow()
{
// 	printer = new QPrinter;
// 	printer->setMinMax(1,10);

	// READ CONFIGS
	readOptions();

	// MAIN WIDGET
	tv = new TrackView(this);
	setCentralWidget(tv);
	tv->setFocus();

	// SET UP ACTIONS

	KAction *newAct = KStdAction::openNew(this, SLOT(fileNew()),
										  actionCollection());
	KAction *openAct = KStdAction::open(this, SLOT(fileOpen()),
										actionCollection());
	KRecentFilesAction *openRecentAct = KStdAction::openRecent(this, 0,
															   actionCollection());
	KAction *saveAct = KStdAction::save(this, SLOT(fileSave()),
										actionCollection());
	KAction *saveAsAct = KStdAction::saveAs(this, SLOT(fileSaveAs()),
											actionCollection());
	KAction *printAct = KStdAction::print(this, SLOT(filePrint()),
										  actionCollection());
	KAction *closeAct = KStdAction::close(this, SLOT(fileClose()),
										actionCollection());
	KAction *quitAct = KStdAction::quit(this, SLOT(fileQuit()),
										actionCollection());
	KAction *preferencesAct = KStdAction::preferences(this, SLOT(options()),
													  actionCollection());

	// SET UP EDITING TOOLBAR
	toolBar("Edit")->insertButton("chord.xpm", 1, SIGNAL(clicked()),
								  this,SLOT(insertChord()),TRUE,i18n("Insert chord"));
	toolBar("Edit")->insertButton("note1.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(setLength1()),TRUE,i18n("Whole"));
	toolBar("Edit")->insertButton("note2.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(setLength2()),TRUE,"1/2");
	toolBar("Edit")->insertButton("note4.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(setLength4()),TRUE,"1/4");
	toolBar("Edit")->insertButton("note8.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(setLength8()),TRUE,"1/8");
	toolBar("Edit")->insertButton("note16.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(setLength16()),TRUE,"1/16");
	toolBar("Edit")->insertButton("note32.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(setLength32()),TRUE,"1/32");
	toolBar("Edit")->insertButton("timesig.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(timeSig()),TRUE,i18n("Time signature"));
	toolBar("Edit")->insertButton("arc.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(linkPrev()),TRUE,i18n("Link with previous column"));
	toolBar("Edit")->insertButton("fx-legato.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(addLegato()),TRUE,i18n("Legato (hammer on/pull off)"));
	toolBar("Edit")->insertButton("fx-harmonic.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(addHarmonic()),TRUE,i18n("Natural harmonic"));
	toolBar("Edit")->insertButton("fx-harmonic.xpm", 1, SIGNAL(clicked()),
								  tv,SLOT(addArtHarm()),TRUE,i18n("Artificial harmonic"));
	
//	toolBar()->setBarPos(KToolBar::BarPosition(globalMainTBPos));
//	toolBar("Edit")->setBarPos(KToolBar::BarPosition(globalEditTBPos));

	// SET UP MAIN MENU AND TOOLBARS

	QPopupMenu *p = new QPopupMenu();
	newAct->plug(p); newAct->plug(toolBar());
	openAct->plug(p); openAct->plug(toolBar());
	openRecentAct->plug(p);
	p->insertSeparator();
	saveAsAct->plug(p);
	saveAct->plug(p); saveAct->plug(toolBar());
	p->insertSeparator();

// 	recMenu = new QPopupMenu();
// 	connect(recMenu, SIGNAL(activated(int)), SLOT(recentLoad(int)));
// 	p->insertItem(i18n("Open &Recent"), recMenu);
// 	recMenu->clear();
// 	for (uint i = 0; i < recentFiles.count(); i++)
// 		recMenu->insertItem(recentFiles.at(i));

	p->insertItem(i18n("Browser..."), this, SLOT(openBrowser()));
	p->insertSeparator();

// 	QPopupMenu *imp = new QPopupMenu();
// 	imp->insertItem(i18n("&MIDI file..."), this, SLOT(importMID()));
// 	p->insertItem(i18n("&Import"), imp);

	QPopupMenu *exp = new QPopupMenu();
	exp->insertItem(i18n("&MIDI file..."), this, SLOT(exportMID()));
	exp->insertItem(i18n("ASCII &tab..."), this, SLOT(exportTAB()));
	exp->insertItem(i18n("&MusiXTeX tab..."), this, SLOT(exportTEXTAB()));
	//exp->insertItem(i18n("Musi&XTeX notes..."), this, SLOT(exportTEXNOTES()));
	p->insertItem(i18n("&Export"), exp);

	p->insertSeparator();
	p->insertItem(i18n("P&roperties..."), this, SLOT(songProperties()));
	printAct->plug(p); printAct->plug(toolBar());
	p->insertSeparator();
	closeAct->plug(p);
	quitAct->plug(p);
	menuBar()->insertItem(i18n("&File"), p);

	p = new QPopupMenu();
	p->insertItem(i18n("&Properties..."), this, SLOT(trackProperties()));
	menuBar()->insertItem(i18n("&Track"), p);

	p = new QPopupMenu();
	p->insertItem(i18n("&Chord"),this,SLOT(insertChord()));
	menuBar()->insertItem(i18n("&Insert"),p);

	p = new QPopupMenu();

	tbMenu = new QPopupMenu();
	tb[0] = tbMenu->insertItem(i18n("Main Toolbar"), this, SLOT(setMainTB()));
	tb[1] = tbMenu->insertItem(i18n("Edit Toolbar"), this, SLOT(setEditTB()));

	tbMenu->setCheckable(TRUE);
	updateTbMenu();

	p->insertItem(i18n("Show Toolbars"), tbMenu);

	nnMenu = new QPopupMenu();
	ni[0] = nnMenu->insertItem(i18n("American, sharps"), this, SLOT(setUSsharp()));
	ni[1] = nnMenu->insertItem(i18n("American, flats"),	 this, SLOT(setUSflats()));
	ni[2] = nnMenu->insertItem(i18n("American, mixed"),	 this, SLOT(setUSmixed()));
	nnMenu->insertSeparator();
	ni[3] = nnMenu->insertItem(i18n("European, sharps"), this, SLOT(setEUsharp()));
	ni[4] = nnMenu->insertItem(i18n("European, flats"),	 this, SLOT(setEUflats()));
	ni[5] = nnMenu->insertItem(i18n("European, mixed"),	 this, SLOT(setEUmixed()));
	nnMenu->insertSeparator();
	ni[6] = nnMenu->insertItem(i18n("Jazz, sharps"),	 this, SLOT(setJZsharp()));
	ni[7] = nnMenu->insertItem(i18n("Jazz, flats"),		 this, SLOT(setJZflats()));
	ni[8] = nnMenu->insertItem(i18n("Jazz, mixed"),		 this, SLOT(setJZmixed()));

	nnMenu->setCheckable(TRUE);
	updateMenu();

	p->insertItem(i18n("&Note names"), nnMenu);

	preferencesAct->plug(p);

	menuBar()->insertItem(i18n("&Settings"), p);

	QString aboutmess = "KGuitar " VERSION "\n\n";
	aboutmess += DESCRIPTION;
	aboutmess += "\n(C) 2000 by KGuitar development team\n";

	p = helpMenu();
	
	menuBar()->insertSeparator();
	menuBar()->insertItem(i18n("&Help"), p);

	statusBar()->insertItem(QString(i18n("Bar: ")) + "1", 1);
	connect(tv, SIGNAL(statusBarChanged()), SLOT(updateStatusBar()));
}

ApplicationWindow::~ApplicationWindow()
{
	delete tv;
//	delete printer;
}

void ApplicationWindow::closeEvent(QCloseEvent *e)
{
	saveOptions();
	KMainWindow::closeEvent(e);
}

void ApplicationWindow::updateMenu()
{
	for (int i = 0; i < 9; i++)
		nnMenu->setItemChecked(ni[i], i == globalNoteNames);
	saveOptions();
}

void ApplicationWindow::updateTbMenu()
{
	tbMenu->setItemChecked(tb[0], globalShowMainTB);
	tbMenu->setItemChecked(tb[1], globalShowEditTB);
	saveOptions();

	if (globalShowMainTB)
		toolBar()->show();
	else
		toolBar()->hide();

	if (globalShowEditTB)
		toolBar("Edit")->show();
	else
		toolBar("Edit")->hide();
}

void ApplicationWindow::updateStatusBar()
{
	QString tmp;
	tmp.setNum(tv->trk()->xb + 1);
	tmp = i18n("Bar: ") + tmp;
	statusBar()->changeItem(tmp,1);
}

bool ApplicationWindow::jazzWarning()
{
	return KMessageBox::warningYesNo(this,
									 i18n("Jazz note names are very special and should be\n"
										  "used only if really know what you do. Usage of jazz\n"
										  "note names without a purpose would confuse or mislead\n"
										  "anyone reading the music who did not have a knowledge\n"
										  "of jazz note naming.\n\n"
										  "Are you sure you want to use jazz notes?"),
									 "KGuitar") == KMessageBox::Yes;
}

void ApplicationWindow::fileNew()
{
	ApplicationWindow *ed = new ApplicationWindow;
	ed->resize(400, 400);
	ed->show();
}

void ApplicationWindow::fileOpen()
{
	QString fn = KFileDialog::getOpenFileName(0, "*.kg", this);
	if (!fn.isEmpty()) {
		if (tv->sng()->load_from_kg(fn)) {
			setCaption(fn);
			tv->setCurt(tv->sng()->t.first());
			tv->sng()->t.first()->x = 0;
			tv->sng()->t.first()->y = 0;
			tv->sng()->filename = fn;
			tv->updateRows();
			addRecentFile(fn.latin1());
		}
	}
}

void ApplicationWindow::recentLoad(int _id)
{
	QString fn = recentFiles.at(_id);

	if (!fn.isEmpty()) {
		if (tv->sng()->load_from_kg(fn)) {
			setCaption(fn);
			tv->setCurt(tv->sng()->t.first());
			tv->sng()->t.first()->x = 0;
			tv->sng()->t.first()->y = 0;
			tv->sng()->filename = fn;
			tv->updateRows();
			addRecentFile(fn.latin1());
		} else {
			recentFiles.remove(_id);
			recMenu->clear();
			for (int i = 0; i < (int) recentFiles.count(); i++)
				recMenu->insertItem(recentFiles.at(i));
			KMessageBox::sorry(this, "KGuitar",
							   i18n("Can't load the song!"));
		}
	}
}

void ApplicationWindow::addRecentFile(const char *fn)
{
	if (recentFiles.find(fn) == -1) {
		if (recentFiles.count() == 5)
			recentFiles.remove(4);
		recentFiles.insert(0, fn);
		recMenu->clear();
		for (uint i = 0 ; i < recentFiles.count(); i++)
			recMenu->insertItem(recentFiles.at(i));
	}
}

void ApplicationWindow::openBrowser()
{
	FileBrowser *fb = new FileBrowser(this);
	fb->show();
	delete fb;
}

void ApplicationWindow::fileSave()
{
	QString fn = tv->sng()->filename;

	if (fn.isEmpty())
		fn = KFileDialog::getSaveFileName(0,"*.kg", this);

	if (!fn.isEmpty()) {		
		tv->arrangeBars();//gotemfix: arrange bars before saving
		tv->sng()->save_to_kg(fn);
		tv->sng()->filename = fn;
		setCaption(fn);
		addRecentFile(fn.latin1());
	}
}

void ApplicationWindow::fileSaveAs()
{
	QString fn = KFileDialog::getSaveFileName(0, "*.kg", this);
	if (!fn.isEmpty()) {
		tv->sng()->save_to_kg(fn);
		tv->sng()->filename = fn;
		setCaption(fn);
		addRecentFile(fn.latin1());
	}
}

void ApplicationWindow::fileImportMid()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.mid",this);
	if (!fn.isEmpty())
		tv->sng()->load_from_mid(fn);
}

void ApplicationWindow::fileExportMid()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.mid",this);
	if (!fn.isEmpty())
		tv->sng()->save_to_mid(fn);
}

void ApplicationWindow::fileExportTab()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.tab",this);
	if (!fn.isEmpty())
		tv->sng()->save_to_tab(fn);
}

void ApplicationWindow::fileExportTexTab()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.tex",this);
	if (!fn.isEmpty())
		tv->sng()->save_to_tex_tab(fn);
}

void ApplicationWindow::fileExportTexNotes()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.tex",this);
	if (!fn.isEmpty())
		tv->sng()->save_to_tex_notes(fn);
}

void ApplicationWindow::filePrint()
{
//	   const int MARGIN = 10;
//	   int pageNo = 1;

//	   if ( printer->setup(this) ) {		// printer dialog
//	statusBar()->message( "Printing..." );
//	QPainter p;
//	p.begin( printer );			// paint on printer
//	p.setFont( e->font() );
//	int yPos		= 0;			// y position for each line
//	QFontMetrics fm = p.fontMetrics();
//	QPaintDeviceMetrics metrics( printer ); // need width/height
//											 // of printer surface
//	for( int i = 0 ; i < e->numLines() ; i++ ) {
//		if ( MARGIN + yPos > metrics.height() - MARGIN ) {
//		QString msg;
//		msg.sprintf( "Printing (page %d)...", ++pageNo );
//		statusBar()->message( msg );
//		printer->newPage();		// no more room on this page
//		yPos = 0;			// back to top of page
//		}
//		p.drawText( MARGIN, MARGIN + yPos,
//			metrics.width(), fm.lineSpacing(),
//			ExpandTabs | DontClip,
//			e->textLine( i ) );
//		yPos = yPos + fm.lineSpacing();
//	}
//	p.end();				// send job to printer
//	statusBar()->message( "Printing completed", 2000 );
//	   } else {
//	statusBar()->message( "Printing aborted", 2000 );
//	   }

}

void ApplicationWindow::fileClose()
{
	close(TRUE); // close AND DELETE!
}

void ApplicationWindow::fileQuit()
{
	fileClose(); //ALINXFIX: exit(0) is impossible, because options will not be saved
}

void ApplicationWindow::insertChord()
{
	int a[MAX_STRINGS];

	ChordSelector cs(tv->trk());
	for (int i = 0; i < tv->trk()->string; i++)
		cs.setApp(i, tv->finger(i));

	// required to detect chord from tabulature
	cs.detectChord();

	if (cs.exec()) {
		for (int i = 0; i < tv->trk()->string; i++)
			a[i] = cs.app(i);
		tv->trk()->insertStrum(cs.scheme(), a);
	}
}

void ApplicationWindow::songProperties()
{
	SetSong *ss = new SetSong();
	ss->title->setText(tv->sng()->title);
	ss->author->setText(tv->sng()->author);
	ss->transcriber->setText(tv->sng()->transcriber);
	ss->comments->setText(tv->sng()->comments);

	if (ss->exec()) {
		tv->sng()->title = ss->title->text();
		tv->sng()->author = ss->author->text();
		tv->sng()->transcriber = ss->transcriber->text();
		tv->sng()->comments = ss->comments->text();
	}

	delete ss;
}

void ApplicationWindow::trackProperties()
{
	SetTrack *st = new SetTrack();

	st->title->setText(tv->trk()->name);
	st->channel->setValue(tv->trk()->channel);
	st->bank->setValue(tv->trk()->bank);
	st->patch->setValue(tv->trk()->patch);

	st->fret->setString(tv->trk()->string);
	st->fret->setFrets(tv->trk()->frets);
	for (int i = 0; i < tv->trk()->string; i++)
		st->fret->setTune(i, tv->trk()->tune[i]);

	if (st->exec()) {
		tv->trk()->name = st->title->text();
		tv->trk()->channel = st->channel->value();
		tv->trk()->bank = st->bank->value();
		tv->trk()->patch = st->patch->value();
		
		tv->trk()->string = st->fret->string();
		tv->trk()->frets = st->fret->frets();
		for (int i = 0; i < tv->trk()->string; i++)
			tv->trk()->tune[i] = st->fret->tune(i);
	}

	delete st;
}

void ApplicationWindow::options()
{
	Options *op = new Options();

	op->maj7gr->setButton(globalMaj7);
	op->flatgr->setButton(globalFlatPlus);

	op->texsizegr->setButton(globalTabSize);
	op->showbarnumb->setChecked(globalShowBarNumb);
	op->showstr->setChecked(globalShowStr);
	op->showpagenumb->setChecked(globalShowPageNumb);

	if (op->exec()) {
		if (op->maj7[0]->isChecked())  globalMaj7 = 0;
		if (op->maj7[1]->isChecked())  globalMaj7 = 1;
		if (op->maj7[2]->isChecked())  globalMaj7 = 2;

		if (op->flat[0]->isChecked())  globalFlatPlus = 0;
		if (op->flat[1]->isChecked())  globalFlatPlus = 1;

		for (int i = 0;i <= 3; i++)
			if (op->tabsize[i]->isChecked()) globalTabSize = i;

		globalShowBarNumb = op->showbarnumb->isChecked();
		globalShowStr = op->showstr->isChecked();
		globalShowPageNumb = op->showpagenumb->isChecked();
	}

	delete op;
	saveOptions();
}

void ApplicationWindow::readOptions()
{
	KConfig *config = kapp->config();

	config->setGroup("General");
	globalMaj7 = config->readNumEntry("Maj7", 0);
	globalFlatPlus = config->readNumEntry("FlatPlus", 0);
	globalNoteNames = config->readNumEntry("NoteNames", 0);

	config->setGroup("MusiXTeX");
	globalTabSize = config->readNumEntry("TabSize", 2);
	globalShowBarNumb = config->readBoolEntry("ShowBarNumb", TRUE);
	globalShowStr = config->readBoolEntry("ShowStr", TRUE);
	globalShowPageNumb = config->readBoolEntry("ShowPageNumb", TRUE);

	config->setGroup("Appearance");
	globalShowMainTB = config->readBoolEntry("ShowMainTB", TRUE);
	globalShowEditTB = config->readBoolEntry("ShowEditTB", TRUE);
	globalMainTBPos = config->readNumEntry("MainTBPos", 0);
	globalEditTBPos = config->readNumEntry("EditTBPos", 0);
	QSize size = config->readSizeEntry("Geometry");
	if (!size.isEmpty())
		resize(size);
	config->readListEntry("RecentFiles", recentFiles);

	config->setGroup("ALSA");
	globalAlsaClient = config->readNumEntry("client", 64);
	globalAlsaPort = config->readNumEntry("port", 0);
}

void ApplicationWindow::saveOptions()
{
	KConfig *config = kapp->config();

	config->setGroup("General");
	config->writeEntry("Maj7", globalMaj7);
	config->writeEntry("FlatPlus", globalFlatPlus);
	config->writeEntry("NoteNames", globalNoteNames);

	config->setGroup("MusiXTeX");
	config->writeEntry("TabSize", globalTabSize);
	config->writeEntry("ShowBarNumb", globalShowBarNumb);
	config->writeEntry("ShowStr", globalShowStr);
	config->writeEntry("ShowPageNumb", globalShowPageNumb);

	config->setGroup("Appearance");
	config->writeEntry("ShowMainTB", globalShowMainTB);
	config->writeEntry("ShowEditTB", globalShowEditTB);
	config->writeEntry("MainTBPos", toolBar()->barPos());
	config->writeEntry("EditTBPos", toolBar("Edit")->barPos());
	config->writeEntry("Geometry", size());
	config->writeEntry("RecentFiles", recentFiles);

	config->setGroup("ALSA");
	config->writeEntry("Client", globalAlsaClient);
	config->writeEntry("Port", globalAlsaPort);
}
