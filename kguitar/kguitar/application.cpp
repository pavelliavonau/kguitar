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

int global_maj7;
int global_flatplus;
int global_notenames;

ApplicationWindow::ApplicationWindow(): KTMainWindow()
{
    printer = new QPrinter;
    printer->setMinMax(1,10);

    // READ CONFIGS
    // GREYFIX to read reading

    global_maj7=0;
    global_flatplus=0;
    global_notenames=0;

    // MAIN WIDGET

    tv = new TrackView(this);
    setView(tv);
    tv->setFocus();

    // SET UP TOOLBAR

    toolBar()->insertButton(Icon("filenew.xpm"),1,SIGNAL(clicked()),this,SLOT(newDoc()),TRUE,i18n("New document"));
    toolBar()->insertButton(Icon("fileopen.xpm"),1,SIGNAL(clicked()),this,SLOT(load()),TRUE,i18n("Open a file"));
    toolBar()->insertButton(Icon("filefloppy.xpm"),1,SIGNAL(clicked()),this,SLOT(save()),TRUE,i18n("Save a file"));
    toolBar()->insertButton(Icon("fileprint.xpm"),1,SIGNAL(clicked()),this,SLOT(print()),TRUE,i18n("Print"));
    toolBar()->insertSeparator();
    toolBar()->insertButton(Icon("chord.xpm"),1,SIGNAL(clicked()),
			    this,SLOT(inschord()),TRUE,i18n("Insert chord"));
    toolBar()->insertButton(Icon("note1.xpm"),1,SIGNAL(clicked()),
			    tv,SLOT(setLength1()),TRUE,i18n("Whole"));
    toolBar()->insertButton(Icon("note2.xpm"),1,SIGNAL(clicked()),
			    tv,SLOT(setLength2()),TRUE,"1/2");
    toolBar()->insertButton(Icon("note4.xpm"),1,SIGNAL(clicked()),
			    tv,SLOT(setLength4()),TRUE,"1/4");
    toolBar()->insertButton(Icon("note8.xpm"),1,SIGNAL(clicked()),
			    tv,SLOT(setLength8()),TRUE,"1/8");
    toolBar()->insertButton(Icon("note16.xpm"),1,SIGNAL(clicked()),
			    tv,SLOT(setLength16()),TRUE,"1/16");
    toolBar()->insertButton(Icon("note32.xpm"),1,SIGNAL(clicked()),
			    tv,SLOT(setLength32()),TRUE,"1/32");
    toolBar()->insertButton(Icon("timesig.xpm"),1,SIGNAL(clicked()),
			    tv,SLOT(timeSig()),TRUE,i18n("Time signature"));
    toolBar()->insertButton(Icon("arc.xpm"),1,SIGNAL(clicked()),
			    tv,SLOT(timeSig()),TRUE,i18n("Arc to previous note"));
    
    // SET UP MAIN MENU

    QPopupMenu *p = new QPopupMenu();
    p->insertItem(i18n("&New"), this, SLOT(newDoc()));
    p->insertItem(i18n("&Open..."), this, SLOT(load()));
    p->insertSeparator();
    p->insertItem(i18n("&Save"), this, SLOT(save()));
    p->insertItem(i18n("S&ave as..."), this, SLOT(saveAs()));

    QPopupMenu *exp = new QPopupMenu();
//    exp->insertItem(i18n("&MIDI file..."), this, SLOT(exportMID()));
    exp->insertItem(i18n("ASCII &tab..."), this, SLOT(exportTAB()));
    p->insertItem(i18n("&Export"), exp);

    p->insertSeparator();
    p->insertItem(i18n("P&roperties..."), this, SLOT(songProperties()));
    p->insertItem(i18n("&Print..."), this, SLOT(print()));
    p->insertSeparator();
    p->insertItem(i18n("&Close"), this, SLOT(closeDoc()));
    p->insertItem(i18n("&Quit"), qApp, SLOT(quit()));
    menuBar()->insertItem(i18n("&File"), p);

    p = new QPopupMenu();
    p->insertItem(i18n("&Track..."),this,SLOT(trackProperties()));
    menuBar()->insertItem(i18n("&Edit"), p);

    p = new QPopupMenu();
    p->insertItem(i18n("&Chord"),this,SLOT(inschord()));
    menuBar()->insertItem(i18n("&Insert"),p);

    p = new QPopupMenu();
    p->insertItem(i18n("&General..."), this, SLOT(options()));

    nnMenu = new QPopupMenu();
    ni[0] = nnMenu->insertItem(i18n("American, sharps"), this, SLOT(setUSsharp()));
    ni[1] = nnMenu->insertItem(i18n("American, flats"),  this, SLOT(setUSflats()));
    ni[2] = nnMenu->insertItem(i18n("American, mixed"),  this, SLOT(setUSmixed()));
    nnMenu->insertSeparator();
    ni[3] = nnMenu->insertItem(i18n("European, sharps"), this, SLOT(setEUsharp()));
    ni[4] = nnMenu->insertItem(i18n("European, flats"),  this, SLOT(setEUflats()));
    ni[5] = nnMenu->insertItem(i18n("European, mixed"),  this, SLOT(setEUmixed()));
    nnMenu->insertSeparator();
    ni[6] = nnMenu->insertItem(i18n("Jazz, sharps"),     this, SLOT(setJZsharp()));
    ni[7] = nnMenu->insertItem(i18n("Jazz, flats"),      this, SLOT(setJZflats()));
    ni[8] = nnMenu->insertItem(i18n("Jazz, mixed"),      this, SLOT(setJZmixed()));

    nnMenu->setCheckable(TRUE);
    updateMenu();

    p->insertItem(i18n("&Note names"), nnMenu);

    menuBar()->insertItem(i18n("&Options"),p);

//     controls = new QPopupMenu();
//     menuBar()->insertItem( "&Controls", controls );

//     mb = controls->insertItem( "Menu bar", this, SLOT(toggleMenuBar()), CTRL+Key_M);
//     // Now an accelerator for when the menubar is invisible!
//     //QAccel* a = new QAccel(this);
//     //    a->connectItem( a->insertItem( CTRL+Key_M ), this, SLOT(toggleMenuBar()) );

//     tb = controls->insertItem( "Tool bar", this, SLOT(toggleToolBar()), CTRL+Key_T);
//     sb = controls->insertItem( "Status bar", this, SLOT(toggleStatusBar()), CTRL+Key_B);
//     controls->setCheckable( TRUE );
//     controls->setItemChecked( mb, TRUE );
//     controls->setItemChecked( tb, TRUE );v
//     controls->setItemChecked( sb, TRUE );

    QString aboutmess = "KGuitar " VERSION "\n\n";
    aboutmess = aboutmess + i18n("A stringed instrument tabulature editor");
    aboutmess = aboutmess + "\n(C) 2000 Mikhail Yakshin AKA GreyCat\n";

    p = KApplication::getKApplication()->getHelpMenu(0,aboutmess);
    
    menuBar()->insertSeparator();
    menuBar()->insertItem(i18n("&Help"), p);

    statusBar()->message( "Ready", 2000 );
}

ApplicationWindow::~ApplicationWindow()
{
    delete tv;
    delete printer;
}

void ApplicationWindow::updateMenu()
{
    for (int i=0;i<9;i++)
	nnMenu->setItemChecked(ni[i], i==global_notenames);
}

bool ApplicationWindow::jazzWarning()
{
    return KMsgBox::yesNo(this, "KGuitar",
			  i18n("Jazz note names are very special and should be\n"
			       "used only if really know what you do. Usage of jazz\n"
			       "note names without a purpose would confuse the user\n"
			       "and could let the misunderstanding of music.\n\n"
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
    QString fn = KFileDialog::getOpenFileName(0,"*.kg",this);
    if (!fn.isEmpty()) {
	if (tv->sng()->load_from_kg(fn)) {
	    setCaption(fn);
	    tv->setCurt(tv->sng()->t.first());
//	    tv->sng()->t.first()->x=0;
	    tv->sng()->t.first()->y=0;
	    tv->sng()->filename=fn;
	    tv->updateRows();
	}
    }
}

void ApplicationWindow::save()
{
    QString fn = tv->sng()->filename;

    if (fn.isEmpty())
	fn = KFileDialog::getSaveFileName(0,"*.kg",this);

    if (!fn.isEmpty()) {
	tv->sng()->save_to_kg(tv->sng()->filename);
	tv->sng()->filename=fn;
    }
}

void ApplicationWindow::saveAs()
{
    QString fn = KFileDialog::getSaveFileName(0,"*.kg",this);
    if (!fn.isEmpty()) {
	tv->sng()->save_to_kg(fn);
	tv->sng()->filename=fn;
    }
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

void ApplicationWindow::print()
{
//     const int MARGIN = 10;
//     int pageNo = 1;

//     if ( printer->setup(this) ) {		// printer dialog
// 	statusBar()->message( "Printing..." );
// 	QPainter p;
// 	p.begin( printer );			// paint on printer
// 	p.setFont( e->font() );
// 	int yPos        = 0;			// y position for each line
// 	QFontMetrics fm = p.fontMetrics();
// 	QPaintDeviceMetrics metrics( printer ); // need width/height
// 	                                         // of printer surface
// 	for( int i = 0 ; i < e->numLines() ; i++ ) {
// 	    if ( MARGIN + yPos > metrics.height() - MARGIN ) {
// 		QString msg;
// 		msg.sprintf( "Printing (page %d)...", ++pageNo );
// 		statusBar()->message( msg );
// 		printer->newPage();		// no more room on this page
// 		yPos = 0;			// back to top of page
// 	    }
// 	    p.drawText( MARGIN, MARGIN + yPos,
// 			metrics.width(), fm.lineSpacing(),
// 			ExpandTabs | DontClip,
// 			e->textLine( i ) );
// 	    yPos = yPos + fm.lineSpacing();
// 	}
// 	p.end();				// send job to printer
// 	statusBar()->message( "Printing completed", 2000 );
//     } else {
// 	statusBar()->message( "Printing aborted", 2000 );
//     }

}

void ApplicationWindow::closeDoc()
{
    close( TRUE ); // close AND DELETE!
}

void ApplicationWindow::inschord()
{
    ChordSelector cs(tv->trk());
    for (int i=0;i<tv->trk()->string;i++)
	cs.setApp(i,tv->finger(i));

    if (cs.exec()) {
	for (int i=0;i<tv->trk()->string;i++)
	    tv->setFinger(i,cs.app(i));
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
    for (int i=0;i<tv->trk()->string;i++)
	st->fret->setTune(i,tv->trk()->tune[i]);

    if (st->exec()) {
	tv->trk()->name = st->title->text();
	tv->trk()->channel = st->channel->value();
	tv->trk()->bank = st->bank->value();
	tv->trk()->patch = st->patch->value();

	tv->trk()->string = st->fret->string();
	tv->trk()->frets = st->fret->frets();
	for (int i=0;i<tv->trk()->string;i++)
	    tv->trk()->tune[i] = st->fret->tune(i);
    }

    delete st;
}

void ApplicationWindow::options()
{
    Options *op = new Options();

    op->maj7gr->setButton(global_maj7);
    op->flatgr->setButton(global_flatplus);

    if (op->exec()) {
	if (op->maj7[0]->isChecked())  global_maj7=0;
	if (op->maj7[1]->isChecked())  global_maj7=1;
	if (op->maj7[2]->isChecked())  global_maj7=2;
	if (op->flat[0]->isChecked())  global_flatplus=0;
	if (op->flat[1]->isChecked())  global_flatplus=1;
    }

    delete op;
}
