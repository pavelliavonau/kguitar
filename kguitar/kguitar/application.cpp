#include "application.h"
#include "global.h"
#include "trackview.h"
#include "chord.h"
#include "track.h"
#include "setsong.h"

#include <qpopupmenu.h>

#include <kapp.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kaccel.h>

#include <qpixmap.h>
#include <qkeycode.h>
#include <qmultilinedit.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>


ApplicationWindow::ApplicationWindow(): KTMainWindow()
{
    printer = new QPrinter;
    printer->setMinMax( 1, 10 );

    toolBar()->insertButton(Icon("filenew.xpm"),1,SIGNAL(clicked()),this,SLOT(newDoc()),TRUE,i18n("New"));
    toolBar()->insertButton(Icon("fileopen.xpm"),1,SIGNAL(clicked()),this,SLOT(load()),TRUE,i18n("Open file"));
    toolBar()->insertButton(Icon("filefloppy.xpm"),1,SIGNAL(clicked()),this,SLOT(save()),TRUE,i18n("Save file"));
    toolBar()->insertButton(Icon("fileprint.xpm"),1,SIGNAL(clicked()),this,SLOT(print()),TRUE,i18n("Print"));
    toolBar()->insertSeparator();
    toolBar()->insertButton(Icon("chord.xpm"),1,SIGNAL(clicked()),this,SLOT(inschord()),TRUE,"Insert chord");
    
    QPopupMenu *p = new QPopupMenu();
    p->insertItem(i18n("&New"), this, SLOT(newDoc()));
    p->insertItem(i18n("&Open..."), this, SLOT(load()));
    p->insertItem(i18n("&Save"), this, SLOT(save()));
    p->insertItem(i18n("S&ave as..."));
    p->insertSeparator();
    p->insertItem(i18n("P&roperties..."), this, SLOT(songProperties()));
    p->insertItem(i18n("&Print..."), this, SLOT(print()));
    p->insertSeparator();
    p->insertItem(i18n("&Close"), this, SLOT(closeDoc()));
    p->insertItem(i18n("&Quit"), qApp, SLOT(quit()));
    menuBar()->insertItem(i18n("&File"), p);

    p = new QPopupMenu();
    p->insertItem(i18n("&Chord"),this,SLOT(inschord()));
    menuBar()->insertItem(i18n("&Insert"),p);

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

    p = KApplication::getKApplication()->getHelpMenu(0,i18n("KGuitar " VERSION "\n\n"
							    "A stringed instrument tabulature editor\n"
							    "(C) 2000 Mikhail Yakshin AKA GreyCat\n"));
    menuBar()->insertSeparator();
    menuBar()->insertItem(i18n("&Help"), p);

    // MAIN WIDGET

    tv = new TrackView(this);
    setView(tv);
    tv->setFocus();

    cs = new ChordSelector();

    statusBar()->message( "Ready", 2000 );
}

ApplicationWindow::~ApplicationWindow()
{
    delete tv;
    delete printer;
}

void ApplicationWindow::newDoc()
{
    ApplicationWindow *ed = new ApplicationWindow;
    ed->resize( 400, 400 );
    ed->show();
}

void ApplicationWindow::load()
{
    QString fn = KFileDialog::getOpenFileName(0,"*.kg",this);
    if (!fn.isEmpty()) {
	if (tv->sng()->load_from_kg(fn)) {
	    setCaption(fn);
	    tv->setCurt(tv->sng()->t.first());
	    tv->sng()->t.first()->x=0;
	    tv->sng()->t.first()->y=0;
	}
    }
}

void ApplicationWindow::save()
{
    QString fn = KFileDialog::getSaveFileName(0,"*.kg",this);
    if (!fn.isEmpty())
	tv->sng()->save_to_kg(fn);
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

void ApplicationWindow::toggleMenuBar()
{
//     if ( menuBar()->isVisible() ) {
// 	menuBar()->hide();
// 	controls->setItemChecked( mb, FALSE );
//     } else {
// 	menuBar()->show();
// 	controls->setItemChecked( mb, TRUE );
//     }
}

void ApplicationWindow::toggleToolBar()
{
//     if ( toolBar()->isVisible() ) {
// 	toolBar()->hide();
// 	controls->setItemChecked( tb, FALSE );
//     } else {
// 	toolBar()->show();
// 	controls->setItemChecked( tb, TRUE );
//     }
}

void ApplicationWindow::toggleStatusBar()
{
//     if ( statusBar()->isVisible() ) {
// 	statusBar()->hide();
// 	controls->setItemChecked( sb, FALSE );
//     } else {
// 	statusBar()->show();
// 	controls->setItemChecked( sb, TRUE );
//     }
}

void ApplicationWindow::inschord()
{
    if (cs->exec()) {
	for (int i=0;i<tv->trk()->string();i++)
	    tv->setFinger(i,cs->app(i));
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
