#include "application.h"
#include "chord.h"

#include <qpixmap.h>
#include <ktoolbar.h>
#include <kbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qkeycode.h>
#include <qmultilinedit.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <kapp.h>
#include <qaccel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>

#include "filesave.xpm"
#include "fileopen.xpm"
#include "fileprint.xpm"
#include "chord.xpm"

ApplicationWindow::ApplicationWindow(): KTMainWindow()
{
    printer = new QPrinter;
    printer->setMinMax( 1, 10 );
    QPixmap openIcon, saveIcon, printIcon, chordIcon;

    openIcon = QPixmap(fileopen);
    saveIcon = QPixmap(filesave);
    printIcon = QPixmap(fileprint);
    chordIcon = QPixmap(chord_xpm);

    toolBar()->insertButton(openIcon,1,SIGNAL(clicked()),this,SLOT(load()),TRUE,"Open file");
    toolBar()->insertButton(saveIcon,1,SIGNAL(clicked()),this,SLOT(save()),TRUE,"Save file");
    toolBar()->insertButton(printIcon,1,SIGNAL(clicked()),this,SLOT(print()),TRUE,"Print tabulature");
    toolBar()->insertSeparator();
    toolBar()->insertButton(chordIcon,1,SIGNAL(clicked()),this,SLOT(inschord()),TRUE,"Insert chord");
    
    QPopupMenu * file = new QPopupMenu();
    menuBar()->insertItem("&File", file);

    file->insertItem( "New", this, SLOT(newDoc()), CTRL+Key_N );
    file->insertItem( openIcon, "Open", this, SLOT(load()), CTRL+Key_O );
    file->insertItem( saveIcon, "Save", this, SLOT(save()), CTRL+Key_S );
    file->insertSeparator();
    file->insertItem( printIcon, "Print", this, SLOT(print()), CTRL+Key_P );
    file->insertSeparator();
    file->insertItem( "Close", this, SLOT(closeDoc()), CTRL+Key_W );
    file->insertItem( "Quit", qApp, SLOT(quit()), CTRL+Key_Q );

    insertMenu = new QPopupMenu();
    menuBar()->insertItem("&Insert",insertMenu);

    insertMenu->insertItem(chordIcon,"&Chord",this,SLOT(inschord()));

    controls = new QPopupMenu();
    menuBar()->insertItem( "&Controls", controls );

    mb = controls->insertItem( "Menu bar", this, SLOT(toggleMenuBar()), CTRL+Key_M);
    // Now an accelerator for when the menubar is invisible!
    //QAccel* a = new QAccel(this);
    //    a->connectItem( a->insertItem( CTRL+Key_M ), this, SLOT(toggleMenuBar()) );

    tb = controls->insertItem( "Tool bar", this, SLOT(toggleToolBar()), CTRL+Key_T);
    sb = controls->insertItem( "Status bar", this, SLOT(toggleStatusBar()), CTRL+Key_B);
    controls->setCheckable( TRUE );
    controls->setItemChecked( mb, TRUE );
    controls->setItemChecked( tb, TRUE );
    controls->setItemChecked( sb, TRUE );

//     menuBar()->insertItem("&Help",getHelpMenu(FALSE,"Something"));

//     e = new QMultiLineEdit( this, "editor" );
//     e->setFocus();

    cs = new ChordSelector();

    statusBar()->message( "Ready", 2000 );
}


/*! Destroys the object and frees any allocated resources.

*/

ApplicationWindow::~ApplicationWindow()
{
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
    QString fn = QFileDialog::getOpenFileName(0,0,this);
    if ( !fn.isEmpty() )
	load( fn );
    else
	statusBar()->message( "Loading aborted", 2000 );
}


void ApplicationWindow::load( const char *fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_ReadOnly ) )
	return;

    e->setAutoUpdate( FALSE );
    e->clear();

    QTextStream t(&f);
    while ( !t.eof() ) {
	QString s = t.readLine();
	e->append( s );
    }
    f.close();

    e->setAutoUpdate( TRUE );
    e->repaint();
    setCaption( fileName );
    QString s;
    s.sprintf( "Loaded document %s", fileName );
    statusBar()->message( s, 2000 );
}

void ApplicationWindow::save()
{
    statusBar()->message( "File->Save is not implemented" );
    QMessageBox::message( "Note", "Left as an exercise for the user." );
}

void ApplicationWindow::print()
{
    const int MARGIN = 10;
    int pageNo = 1;

    if ( printer->setup(this) ) {		// printer dialog
	statusBar()->message( "Printing..." );
	QPainter p;
	p.begin( printer );			// paint on printer
	p.setFont( e->font() );
	int yPos        = 0;			// y position for each line
	QFontMetrics fm = p.fontMetrics();
	QPaintDeviceMetrics metrics( printer ); // need width/height
	                                         // of printer surface
	for( int i = 0 ; i < e->numLines() ; i++ ) {
	    if ( MARGIN + yPos > metrics.height() - MARGIN ) {
		QString msg;
		msg.sprintf( "Printing (page %d)...", ++pageNo );
		statusBar()->message( msg );
		printer->newPage();		// no more room on this page
		yPos = 0;			// back to top of page
	    }
	    p.drawText( MARGIN, MARGIN + yPos,
			metrics.width(), fm.lineSpacing(),
			ExpandTabs | DontClip,
			e->textLine( i ) );
	    yPos = yPos + fm.lineSpacing();
	}
	p.end();				// send job to printer
	statusBar()->message( "Printing completed", 2000 );
    } else {
	statusBar()->message( "Printing aborted", 2000 );
    }

}

void ApplicationWindow::closeDoc()
{
    close( TRUE ); // close AND DELETE!
}

void ApplicationWindow::toggleMenuBar()
{
    if ( menuBar()->isVisible() ) {
	menuBar()->hide();
	controls->setItemChecked( mb, FALSE );
    } else {
	menuBar()->show();
	controls->setItemChecked( mb, TRUE );
    }
}

void ApplicationWindow::toggleToolBar()
{
    if ( toolBar()->isVisible() ) {
	toolBar()->hide();
	controls->setItemChecked( tb, FALSE );
    } else {
	toolBar()->show();
	controls->setItemChecked( tb, TRUE );
    }
}

void ApplicationWindow::toggleStatusBar()
{
    if ( statusBar()->isVisible() ) {
	statusBar()->hide();
	controls->setItemChecked( sb, FALSE );
    } else {
	statusBar()->show();
	controls->setItemChecked( sb, TRUE );
    }
}

void ApplicationWindow::inschord()
{
    cs->exec();
}
