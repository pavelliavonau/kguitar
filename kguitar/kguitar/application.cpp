#include "application.h"

#include "trackview.h"
#include "chord.h"
#include "tabsong.h"
#include "setsong.h"
#include "settrack.h"
#include "settabfret.h"
#include "options.h"

#include <qpopupmenu.h>

#include <kapp.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kaccel.h>
#include <kmsgbox.h>

#include <qpixmap.h>
#include <qkeycode.h>
#include <qstatusbar.h>
#include <qprinter.h>

#include <qmultilinedit.h>
#include <kintegerline.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

// Global variables - real declarations

// General
int global_maj7;
int global_flatplus;
int global_notenames;

// MusiXTeX
int global_tabsize;
bool global_showbarnumb;
bool global_showstr;
bool global_showpagenumb;

// Appearance
bool global_showMainTB;       // Toolbars
bool global_showEditTB;
int global_mainTBPos;
int global_editTBPos;
int global_mainWinWidth;      // ApplicationWindow
int global_mainWinHeight;
int global_mainWinX;
int global_mainWinY;

ApplicationWindow::ApplicationWindow(): KTMainWindow()
{
	printer = new QPrinter;
	printer->setMinMax(1,10);

	// READ CONFIGS
	readOptions();

	// MAIN WIDGET
	tv = new TrackView(this);
	setView(tv);
	tv->setFocus();

	// SET UP MAIN TOOLBAR
	toolBar()->insertButton(Icon("filenew.xpm"),1,SIGNAL(clicked()),
							this,SLOT(newDoc()),TRUE,i18n("New document"));
	toolBar()->insertButton(Icon("fileopen.xpm"),1,SIGNAL(clicked()),
							this,SLOT(load()),TRUE,i18n("Open a file"));
	toolBar()->insertButton(Icon("filefloppy.xpm"),1,SIGNAL(clicked()),
							this,SLOT(save()),TRUE,i18n("Save a file"));
	toolBar()->insertButton(Icon("fileprint.xpm"),1,SIGNAL(clicked()),
							this,SLOT(print()),TRUE,i18n("Print"));
	toolBar()->insertSeparator();

	// SET UP EDITING TOOLBAR
	toolBar(1)->insertButton(Icon("chord.xpm"),1,SIGNAL(clicked()),
							this,SLOT(inschord()),TRUE,i18n("Insert chord"));
	toolBar(1)->insertButton(Icon("note1.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(setLength1()),TRUE,i18n("Whole"));
	toolBar(1)->insertButton(Icon("note2.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(setLength2()),TRUE,"1/2");
	toolBar(1)->insertButton(Icon("note4.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(setLength4()),TRUE,"1/4");
	toolBar(1)->insertButton(Icon("note8.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(setLength8()),TRUE,"1/8");
	toolBar(1)->insertButton(Icon("note16.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(setLength16()),TRUE,"1/16");
	toolBar(1)->insertButton(Icon("note32.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(setLength32()),TRUE,"1/32");
	toolBar(1)->insertButton(Icon("timesig.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(timeSig()),TRUE,i18n("Time signature"));
	toolBar(1)->insertButton(Icon("arc.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(linkPrev()),TRUE,i18n("Link with previous column"));
	toolBar(1)->insertButton(Icon("fx-legato.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(addLegato()),TRUE,i18n("Legato (hammer on/pull off)"));
	toolBar(1)->insertButton(Icon("fx-harmonic.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(addHarmonic()),TRUE,i18n("Natural harmonic"));
	toolBar(1)->insertButton(Icon("fx-harmonic.xpm"),1,SIGNAL(clicked()),
							tv,SLOT(addArtHarm()),TRUE,i18n("Artificial harmonic"));
	
	toolBar()->setBarPos(KToolBar::BarPosition(global_mainTBPos));
	toolBar(1)->setBarPos(KToolBar::BarPosition(global_editTBPos));

	// SET UP MAIN MENU

	QPopupMenu *p = new QPopupMenu();
	p->insertItem(i18n("&New"), this, SLOT(newDoc()));
	p->insertItem(i18n("&Open..."), this, SLOT(load()));

	recMenu = new QPopupMenu();
	connect(recMenu, SIGNAL(activated(int)), SLOT(recentLoad(int)));
	p->insertItem(i18n("Open &Recent"), recMenu);
	recMenu->clear();
	for (int i = 0; i < recentFiles.count(); i++)
		recMenu->insertItem(recentFiles.at(i));

	p->insertSeparator();
	p->insertItem(i18n("&Save"), this, SLOT(save()));
	p->insertItem(i18n("S&ave as..."), this, SLOT(saveAs()));

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
	p->insertItem(i18n("&Print..."), this, SLOT(print()));
	p->insertSeparator();
	p->insertItem(i18n("&Close"), this, SLOT(closeDoc()));
	p->insertItem(i18n("&Quit"), this, SLOT(appQuit())); //qApp, SLOT(quit()));
	menuBar()->insertItem(i18n("&File"), p);

	p = new QPopupMenu();
	p->insertItem(i18n("&Track..."),this,SLOT(trackProperties()));
	menuBar()->insertItem(i18n("&Edit"), p);

	p = new QPopupMenu();
	p->insertItem(i18n("&Chord"),this,SLOT(inschord()));
	menuBar()->insertItem(i18n("&Insert"),p);

	p = new QPopupMenu();
	p->insertItem(i18n("&General..."), this, SLOT(options()));

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

	menuBar()->insertItem(i18n("&Options"),p);

	QString aboutmess = "KGuitar " VERSION "\n\n";
	aboutmess = aboutmess + i18n("A stringed instrument tabulature editor");
	aboutmess = aboutmess + "\n(C) 2000 KGuitar development team\n";

	p = KApplication::getKApplication()->getHelpMenu(0,aboutmess);
	
	menuBar()->insertSeparator();
	menuBar()->insertItem(i18n("&Help"), p);

	statusBar()->insertItem(QString(i18n("Bar: ")) + "1", 1);
	connect(tv, SIGNAL(statusBarChanged()), SLOT(updateStatusBar()));
}

ApplicationWindow::~ApplicationWindow()
{
	delete tv;
	delete printer;
}

void ApplicationWindow::updateMenu()
{
	for (int i = 0; i < 9; i++)
		nnMenu->setItemChecked(ni[i], i == global_notenames);
	saveOptions();
}

void ApplicationWindow::updateTbMenu()
{
	tbMenu->setItemChecked(tb[0], global_showMainTB);
	tbMenu->setItemChecked(tb[1], global_showEditTB);
	saveOptions();

	if (global_showMainTB)
		toolBar()->show();
	else
		toolBar()->hide();

	if (global_showEditTB)
		toolBar(1)->show();
	else
		toolBar(1)->hide();

	int  w, h;
	w = width();
	h = height();
	w += 2;
	resize(w, h);
	w -=2;          // ALINXFIX: is there a better way?
	resize(w, h);
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
	return KMsgBox::yesNo(this, "KGuitar",
						  i18n("Jazz note names are very special and should be\n"
							   "used only if really know what you do. Usage of jazz\n"
							   "note names without a purpose would confuse or mislead\n"
							   "anyone reading the music who did not have a knowledge\n"
							   "of jazz note naming.\n\n"
							   "Are you sure you want to use jazz notes?"))==1;
}

void ApplicationWindow::newDoc()
{
	ApplicationWindow *ed = new ApplicationWindow;
	ed->resize(400, 400);
	ed->show();
}

void ApplicationWindow::load()
{
	QString fn = KFileDialog::getOpenFileName(0, "*.kg", this);
	if (!fn.isEmpty()) {
		if (tv->sng()->load_from_kg(fn)) {
			QString tmp = PACKAGE;
			tmp += " <" + fn + ">";
			setCaption(tmp);
			tv->setCurt(tv->sng()->t.first());
			tv->sng()->t.first()->x = 0;
			tv->sng()->t.first()->y = 0;
			tv->sng()->filename = fn;
			tv->updateRows();
			addRecentFile(fn);
		}
	}
}

void ApplicationWindow::recentLoad(int _id)
{
	QString fn = recentFiles.at(_id);

	if (!fn.isEmpty()) {
		if (tv->sng()->load_from_kg(fn)) {
			QString tmp = PACKAGE;
			tmp += " <" + fn + ">";
			setCaption(tmp);
			tv->setCurt(tv->sng()->t.first());
			tv->sng()->t.first()->x = 0;
			tv->sng()->t.first()->y = 0;
			tv->sng()->filename = fn;
			tv->updateRows();
			addRecentFile(fn);
		}
		else {
			recentFiles.remove(_id);
			recMenu->clear();
			for (int i = 0 ; i < (int) recentFiles.count(); i++)
				recMenu->insertItem(recentFiles.at(i));
		}
	}
}

void ApplicationWindow::addRecentFile(QString fn)
{
// copied from Quanta 1.0.4 (http://quanta.sourceforge.net)

	if (recentFiles.find(fn) == -1) {
		if (recentFiles.count() < 5) {
			recentFiles.insert(0, fn);
		}
		else {
			recentFiles.remove(4);
			recentFiles.insert(0, fn);
		}
		recMenu->clear();
		for (int i = 0 ; i < (int) recentFiles.count(); i++)
			recMenu->insertItem(recentFiles.at(i));
	}
}

void ApplicationWindow::save()
{
	QString fn = tv->sng()->filename;

	if (fn.isEmpty())
		fn = KFileDialog::getSaveFileName(0,"*.kg",this);

	if (!fn.isEmpty()) {		
		tv->arrangeBars();//gotemfix: arrange bars before saving
		tv->sng()->save_to_kg(fn);
		tv->sng()->filename = fn;
		QString tmp = PACKAGE;
		tmp += " <" + fn + ">";
		setCaption(tmp);
		addRecentFile(fn);
	}
}

void ApplicationWindow::saveAs()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.kg",this);
	if (!fn.isEmpty()) {
		tv->sng()->save_to_kg(fn);
		tv->sng()->filename = fn;
		QString tmp = PACKAGE;
		tmp += " <" + fn + ">";
		setCaption(tmp);
		addRecentFile(fn);
	}
}

void ApplicationWindow::importMID()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.mid",this);
	if (!fn.isEmpty())
		tv->sng()->load_from_mid(fn);
}

void ApplicationWindow::exportMID()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.mid",this);
	if (!fn.isEmpty())
		tv->sng()->save_to_mid(fn);
}

void ApplicationWindow::exportTAB()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.tab",this);
	if (!fn.isEmpty())
		tv->sng()->save_to_tab(fn);
}

void ApplicationWindow::exportTEXTAB()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.tex",this);
	if (!fn.isEmpty())
		tv->sng()->save_to_tex_tab(fn);
}

void ApplicationWindow::exportTEXNOTES()
{
	QString fn = KFileDialog::getSaveFileName(0,"*.tex",this);
	if (!fn.isEmpty())
		tv->sng()->save_to_tex_notes(fn);
}

void ApplicationWindow::print()
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

void ApplicationWindow::closeDoc()
{
	close(TRUE); // close AND DELETE!
}

void ApplicationWindow::appQuit()
{
	saveOptions();
	closeDoc(); //ALINXFIX: exit(0) is impossible, because options will not be saved
}

void ApplicationWindow::inschord()
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

	op->maj7gr->setButton(global_maj7);
	op->flatgr->setButton(global_flatplus);

	op->texsizegr->setButton(global_tabsize);
	op->showbarnumb->setChecked(global_showbarnumb);
	op->showstr->setChecked(global_showstr);
	op->showpagenumb->setChecked(global_showpagenumb);

	if (op->exec()) {
		if (op->maj7[0]->isChecked())  global_maj7 = 0;
		if (op->maj7[1]->isChecked())  global_maj7 = 1;
		if (op->maj7[2]->isChecked())  global_maj7 = 2;

		if (op->flat[0]->isChecked())  global_flatplus = 0;
		if (op->flat[1]->isChecked())  global_flatplus = 1;

		for (int i = 0;i <= 3; i++)
			if (op->tabsize[i]->isChecked()) global_tabsize = i;

		global_showbarnumb = op->showbarnumb->isChecked();
		global_showstr = op->showstr->isChecked();
		global_showpagenumb = op->showpagenumb->isChecked();
	}

	delete op;
	saveOptions();
}

void ApplicationWindow::readOptions()
{
	kapp->getConfig()->setGroup("General");
	global_maj7 = kapp->getConfig()->readNumEntry("maj7", 0);
	global_flatplus = kapp->getConfig()->readNumEntry("flatplus", 0);
	global_notenames = kapp->getConfig()->readNumEntry("notenames", 0);

	kapp->getConfig()->setGroup("MusiXTeX");
	global_tabsize = kapp->getConfig()->readNumEntry("tabsize", 2);
	global_showbarnumb = kapp->getConfig()->readBoolEntry("showbarnumb", TRUE);
	global_showstr = kapp->getConfig()->readBoolEntry("showstr", TRUE);
	global_showpagenumb = kapp->getConfig()->readBoolEntry("showpagenumb", TRUE);

	kapp->getConfig()->setGroup("Appearance");
	global_showMainTB = kapp->getConfig()->readBoolEntry("showMainTB", TRUE);
	global_showEditTB = kapp->getConfig()->readBoolEntry("showEditTB", TRUE);
	global_mainTBPos = kapp->getConfig()->readNumEntry("mainTBPos", 0);
	global_editTBPos = kapp->getConfig()->readNumEntry("editTBPos", 0);
	QSize size = kapp->getConfig()->readSizeEntry("geometry");
	if (!size.isEmpty())
		resize(size);
	kapp->getConfig()->readListEntry("recentFiles", recentFiles);
}

void ApplicationWindow::saveOptions()
{
	kapp->getConfig()->setGroup("General");
	kapp->getConfig()->writeEntry("maj7", global_maj7);
	kapp->getConfig()->writeEntry("flatplus", global_flatplus);
	kapp->getConfig()->writeEntry("notenames", global_notenames);

	kapp->getConfig()->setGroup("MusiXTeX");
	kapp->getConfig()->writeEntry("tabsize", global_tabsize);
	kapp->getConfig()->writeEntry("showbarnumb", global_showbarnumb);
	kapp->getConfig()->writeEntry("showstr", global_showstr);
	kapp->getConfig()->writeEntry("showpagenumb", global_showpagenumb);

	kapp->getConfig()->setGroup("Appearance");
	kapp->getConfig()->writeEntry("showMainTB", global_showMainTB);
	kapp->getConfig()->writeEntry("showEditTB", global_showEditTB);
	kapp->getConfig()->writeEntry("mainTBPos", toolBar()->barPos());
	kapp->getConfig()->writeEntry("editTBPos", toolBar(1)->barPos());
	kapp->getConfig()->writeEntry("geometry", size());
	kapp->getConfig()->writeEntry("recentFiles", recentFiles);
}
