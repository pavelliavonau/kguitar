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
#include <kaction.h>
#include <kstdaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kaccel.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kurl.h>
#include <kkeydialog.h>
#include <kdebug.h>

#include <qpixmap.h>
#include <qkeycode.h>
#include <qstatusbar.h>
#include <qprinter.h>

#include <qlineedit.h>
#include <qmultilinedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qfileinfo.h>

#include <libkmid/deviceman.h>

#include <stdio.h>

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
int globalTexExpMode;

// ALSA
int globalAlsaClient;
int globalAlsaPort;

bool globalHaveMidi;


extern "C" {
	void *init_libkguitar()
	{
		return new KGuitarFactory;
	}
};

KInstance *KGuitarFactory::s_instance = 0L;

KGuitarFactory::KGuitarFactory()
{
}

KGuitarFactory::~KGuitarFactory()
{
	if (s_instance)
		delete s_instance;
	s_instance = 0;
}

KParts::Part *KGuitarFactory::createPart(QWidget *parentWidget, const char *widgetName,
									 QObject *parent, const char *name, const char *className,
									 const QStringList &)
{
	bool bBrowserView = (strcmp(className, "Browser/View") == 0);
	KParts::Part *obj = new KGuitarPart(bBrowserView, parentWidget, widgetName,
										parent, name);
	emit objectCreated(obj);
	return obj;
}

KInstance *KGuitarFactory::instance()
{
	if (!s_instance)
		s_instance = new KInstance("kguitar");
	return s_instance;
}

//------------------------------------------------------------------------

KGuitarPart::KGuitarPart(bool bBrowserView, QWidget *parentWidget,
						 const char *, QObject *parent, const char *name)
	: KParts::ReadWritePart(parent, name)
{
// 	printer = new QPrinter;
// 	printer->setMinMax(1,10);

	kdDebug() << "KGuitarPart::KGuitarPart()" << endl;

	p = parentWidget;
	isBrowserView = bBrowserView;

	setInstance(KGuitarFactory::instance());

	// MAIN WIDGET
	tv = new TrackView(parentWidget);
	tv->setFocusPolicy(QWidget::StrongFocus);
	setWidget(tv);
	tv->setFocus();

	// SET UP STANDARD ACTIONS
	newAct = KStdAction::openNew(this, SLOT(fileNew()),
								 actionCollection(), "file_new");

	preferencesAct = KStdAction::preferences(this, SLOT(options()),
											 actionCollection(), "pref_options");
	confTBAct = KStdAction::configureToolbars(this, SLOT(slotConfigToolBars()),
											  actionCollection(), "config_toolbars");
	confKeyAct = KStdAction::keyBindings(this, SLOT(configKeys()),
										 actionCollection(), "config_keys");

	sngPropAct = new KAction(i18n("P&roperties..."), 0, this, SLOT(songProperties()),
							 actionCollection(), "song_properties");

	trkPropAct = new KAction(i18n("&Properties..."), 0, this, SLOT(trackProperties()),
							 actionCollection(), "track_properties");
	insChordAct = new KAction(i18n("&Chord..."), "chord",  KAccel::stringToKey("Shift+C"),
							  this, SLOT(insertChord()), actionCollection(), "insert_chord");


	saveOptionAct = new KAction(i18n("&Save Options"), 0, this,
								SLOT(saveOptions()), actionCollection(), "save_options");

	arrTrkAct = new KAction(i18n("&Arrange Track"), KAccel::stringToKey("Shift+A"), tv,
							SLOT(arrangeTracks()), actionCollection(), "arrange_trk");

	// SET UP DURATION
	len1Act = new KAction(i18n("Whole"), "note1", KAccel::stringToKey("Ctrl+1"),
						  tv, SLOT(setLength1()), actionCollection(), "set_len1");
	len2Act = new KAction(i18n("1/2"), "note2", KAccel::stringToKey("Ctrl+2"),
						  tv, SLOT(setLength2()), actionCollection(), "set_len2");
	len4Act = new KAction(i18n("1/4"), "note4", KAccel::stringToKey("Ctrl+3"),
						  tv, SLOT(setLength4()), actionCollection(), "set_len4");
	len8Act = new KAction(i18n("1/8"), "note8", KAccel::stringToKey("Ctrl+4"),
						  tv, SLOT(setLength8()), actionCollection(), "set_len8");
	len16Act = new KAction(i18n("1/16"), "note16", KAccel::stringToKey("Ctrl+5"),
						   tv, SLOT(setLength16()), actionCollection(), "set_len16");
	len32Act = new KAction(i18n("1/32"), "note32", KAccel::stringToKey("Ctrl+6"),
						   tv, SLOT(setLength32()), actionCollection(), "set_len32");

	// SET UP EFFECTS
	timeSigAct = new KAction(i18n("Time signature"), "timesig",
							 KAccel::stringToKey("Shift+T"), tv, SLOT(timeSig()),
							 actionCollection(), "time_sig");
	arcAct = new KAction(i18n("Link with previous column"), "arc",
						 KAccel::stringToKey("L"), tv, SLOT(linkPrev()),
						 actionCollection(), "link_prev");
	legatoAct = new KAction(i18n("Legato (hammer on/pull off)"), "fx_legato",
							KAccel::stringToKey("P"), tv, SLOT(addLegato()),
							actionCollection(), "fx_legato");
	natHarmAct = new KAction(i18n("Natural harmonic"), "fx_harmonic",
							 KAccel::stringToKey("H"), tv, SLOT(addHarmonic()),
							 actionCollection(), "fx_nat_harm");
	artHarmAct = new KAction(i18n("Artificial harmonic"), "fx_harmonic",
							 KAccel::stringToKey("R"), tv, SLOT(addArtHarm()),
							 actionCollection(), "fx_art_harm");

	// SET UP 'Note Names'
	usSharpAct = new KToggleAction(i18n("American, sharps"), 0, this,
								   SLOT(setUSsharp()), actionCollection(), "us_sharp");
	usFlatAct = new KToggleAction(i18n("American, flats"), 0, this,
								  SLOT(setUSflats()), actionCollection(), "us_flat");
	usMixAct = new KToggleAction(i18n("American, mixed"), 0, this,
								 SLOT(setUSmixed()), actionCollection(), "us_mix");
	euSharpAct = new KToggleAction(i18n("European, sharps"), 0, this,
								   SLOT(setEUsharp()), actionCollection(), "eu_sharp");
	euFlatAct = new KToggleAction(i18n("European, flats"), 0, this,
								  SLOT(setEUflats()), actionCollection(), "eu_flat");
	euMixAct = new KToggleAction(i18n("European, mixed"), 0, this,
								 SLOT(setEUmixed()), actionCollection(), "eu_mix");
	jazzSharpAct = new KToggleAction(i18n("Jazz, sharps"), 0, this,
									 SLOT(setJZsharp()), actionCollection(), "jazz_sharp");
	jazzFlatAct = new KToggleAction(i18n("Jazz, flats"), 0, this,
									SLOT(setJZflats()), actionCollection(), "jazz_flat");
	jazzMixAct = new KToggleAction(i18n("Jazz, mixed"), 0, this,
								   SLOT(setJZmixed()), actionCollection(), "jazz_mix");

	// SET UP ACCEL...
	mainAccel = new KAccel(tv);

	// ...FOR CURSOR
	mainAccel->insertItem(i18n("Move cursor right"), "key_right", "Right");
	mainAccel->connectItem("key_right", tv, SLOT(keyRight()));
	mainAccel->insertItem(i18n("Move cursor left"), "key_left", "Left");
	mainAccel->connectItem("key_left", tv, SLOT(keyLeft()));
	mainAccel->insertItem(i18n("Move cursor up"), "key_up", "Up");
	mainAccel->connectItem("key_up", tv, SLOT(keyUp()));
	mainAccel->insertItem(i18n("Move cursor up"), "key_CtrlUp", "Ctrl+Up");
	mainAccel->connectItem("key_CtrlUp", tv, SLOT(keyCtrlUp()));
	mainAccel->insertItem(i18n("Move cursor down"), "key_down", "Down");
	mainAccel->connectItem("key_down", tv, SLOT(keyDown()));
	mainAccel->insertItem(i18n("Move cursor down"), "key_CtrlDown", "Ctrl+Down");
	mainAccel->connectItem("key_CtrlDown", tv, SLOT(keyCtrlDown()));

    // ...FOR OTHER KEYS
	mainAccel->insertItem(i18n("Dead note"), "key_x", "X");
	mainAccel->connectItem("key_x", tv, SLOT(deadNote()));
	mainAccel->insertItem(i18n("Delete"), "key_del", "Delete");
	mainAccel->connectItem("key_del", tv, SLOT(keyDelete()));
	mainAccel->insertItem(i18n("Ctrl+Delete"), "key_CtrlDel", "Ctrl+Delete");
	mainAccel->connectItem("key_CtrlDel", tv, SLOT(keyCtrlDelete()));
	mainAccel->insertItem(i18n("Insert"), "key_ins", "Insert");
	mainAccel->connectItem("key_ins", tv, SLOT(keyInsert()));
	mainAccel->insertItem(i18n("Palm muting"), "key_m", "M");
	mainAccel->connectItem("key_m", tv, SLOT(keyM()));
	mainAccel->insertItem(i18n("Doted note"), "key_period", "Period");
	mainAccel->connectItem("key_period", tv, SLOT(keyPeriod()));
	mainAccel->insertItem(i18n("Key plus"), "key_plus", "Plus");
	mainAccel->connectItem("key_plus", tv, SLOT(keyPlus()));
	mainAccel->insertItem(i18n("Key minus"), "key_minus", "Minus");
	mainAccel->connectItem("key_minus", tv, SLOT(keyMinus()));

    // ...FOR KEY '0' - '9'
	mainAccel->insertItem(i18n("Key 1"), "key_1", "1");
	mainAccel->connectItem("key_1", tv, SLOT(key1()));
	mainAccel->insertItem(i18n("Key 2"), "key_2", "2");
	mainAccel->connectItem("key_2", tv, SLOT(key2()));
	mainAccel->insertItem(i18n("Key 3"), "key_3", "3");
	mainAccel->connectItem("key_3", tv, SLOT(key3()));
	mainAccel->insertItem(i18n("Key 4"), "key_4", "4");
	mainAccel->connectItem("key_4", tv, SLOT(key4()));
	mainAccel->insertItem(i18n("Key 5"), "key_5", "5");
	mainAccel->connectItem("key_5", tv, SLOT(key5()));
	mainAccel->insertItem(i18n("Key 6"), "key_6", "6");
	mainAccel->connectItem("key_6", tv, SLOT(key6()));
	mainAccel->insertItem(i18n("Key 7"), "key_7", "7");
	mainAccel->connectItem("key_7", tv, SLOT(key7()));
	mainAccel->insertItem(i18n("Key 8"), "key_8", "8");
	mainAccel->connectItem("key_8", tv, SLOT(key8()));
	mainAccel->insertItem(i18n("Key 9"), "key_9", "9");
	mainAccel->connectItem("key_9", tv, SLOT(key9()));
	mainAccel->insertItem(i18n("Key 0"), "key_0", "0");
	mainAccel->connectItem("key_0", tv, SLOT(key0()));

	m_extension = new KGuitarBrowserExtension(this);

	if (bBrowserView) {
		mainAccel->setEnabled(FALSE);
		sngPropAct->setText(i18n("Song Properties..."));
		trkPropAct->setText(i18n("Track Properties..."));
		setXMLFile("kguitar_konq.rc");
	} else
		setXMLFile("kguitar_part.rc");

	// READ CONFIGS
	readOptions();

	updateMenu();
}

KGuitarPart::~KGuitarPart()
{
	kdDebug() << "KGuitarPart::~KGuitarPart()" << endl;

//	delete printer;
	saveOptions();
}

void KGuitarPart::updateMenu()
{
	usSharpAct->setChecked(globalNoteNames == 0);
	usFlatAct->setChecked(globalNoteNames == 1);
	usMixAct->setChecked(globalNoteNames == 2);
	euSharpAct->setChecked(globalNoteNames == 3);
	euFlatAct->setChecked(globalNoteNames == 4);
	euMixAct->setChecked(globalNoteNames == 5);
	jazzSharpAct->setChecked(globalNoteNames == 6);
	jazzFlatAct->setChecked(globalNoteNames == 7);
	jazzMixAct->setChecked(globalNoteNames == 8);
}

bool KGuitarPart::jazzWarning()
{
	return KMessageBox::warningYesNo(p,
									 i18n("Jazz note names are very special and should be\n"
										  "used only if really know what you do. Usage of jazz\n"
										  "note names without a purpose would confuse or mislead\n"
										  "anyone reading the music who did not have a knowledge\n"
										  "of jazz note naming.\n\n"
										  "Are you sure you want to use jazz notes?")) == KMessageBox::Yes;
}

void KGuitarPart::fileNew()
{
// 	KGuitarPart *ed = new KGuitarPart;
// 	ed->resize(400, 400);
// 	ed->show();
}

bool KGuitarPart::saveFile()   // KParts
{
	bool ret = fileSave(m_file);
	if (!ret)
		setWinCaption(i18n("Unnamed"));
	return ret;
}

bool KGuitarPart::openFile()   // KParts
{
	bool ret = slotOpenFile(m_file);
	if (!ret)
		setWinCaption(i18n("Unnamed"));
	return ret;
}

bool KGuitarPart::slotOpenFile(QString fn)
{
	bool ret = FALSE;
	if (!fn.isEmpty()) {
		QFileInfo *fi = new QFileInfo(fn);

		if (!fi->isFile()) {
			KMessageBox::sorry(p, i18n("Please select a file."));
			return FALSE;
		}
		if (!fi->isReadable()) {
			KMessageBox::sorry(p, i18n("You have no permission to read this file."));
			return FALSE;
		}

		QString ext = fi->extension();
		ext = ext.upper();

		if (ext == "KG") {
			if (tv->sng()->load_from_kg(fn)) {
				setWinCaption(fn);
				tv->setCurt(tv->sng()->t.first());
				tv->sng()->t.first()->x = 0;
				tv->sng()->t.first()->y = 0;
				tv->sng()->filename = fn;
				tv->updateRows();
				ret = TRUE;
			} else {
				KMessageBox::sorry(p, i18n("Can't load the song!"));
				return FALSE;
			}
		}

		if (ext == "TAB") {
			if (tv->sng()->load_from_tab(fn)) {
				tv->sng()->filename = "";
				setWinCaption(i18n("Unnamed"));
				ret = TRUE;
			} else {
				KMessageBox::sorry(p, i18n("Can't load the song!"));
				return FALSE;
			}
		}

		if (ext == "MID") {
			if (tv->sng()->load_from_mid(fn)) {
				tv->sng()->filename = "";
				setWinCaption(i18n("Unnamed"));
				ret = TRUE;
			} else {
				KMessageBox::sorry(p, i18n("Can't load the song!"));
				return FALSE;
			}
		}

		if (ext == "GTP") {
			if (tv->sng()->load_from_gtp(fn)) {
				tv->sng()->filename = "";
				setWinCaption(i18n("Unnamed"));
				ret = TRUE;
			} else {
				KMessageBox::sorry(p, i18n("This feature is at this time not implemented.\n"
										   "If you would help to implement this feature\n"
										   "please write an email to: greycat@users.sourceforge.net"));
				return FALSE;
			}
		}
	}
	return ret;
}

bool KGuitarPart::fileSave(QString fn)
{
	bool ret = FALSE;

	QFileInfo *fi = new QFileInfo(fn);
	QString ext = fi->extension();
	ext = ext.upper();

	if (ext == "KG") {
		tv->arrangeBars();
		if (tv->sng()->save_to_kg(fn)) {
			tv->sng()->filename = fn;
			setWinCaption(fn);
			ret = TRUE;
		} else {
			KMessageBox::sorry(p, i18n("Can't save the song!"));
			return FALSE;
		}
	}
	if (ext == "TAB") {
		if (tv->sng()->save_to_tab(fn)) {
			ret = TRUE;
		} else {
			KMessageBox::sorry(p, i18n("Can't export the song!"));
			return FALSE;
		}
	}
	if (ext == "MID") {
		if (tv->sng()->save_to_mid(fn)) {
			ret = TRUE;
		} else {
			KMessageBox::sorry(p, i18n("Can't export the song!"));
			return FALSE;
		}
	}
	if (ext == "GTP") {
		if (tv->sng()->save_to_gtp(fn)) {
			ret = TRUE;
		} else {
			KMessageBox::sorry(p, i18n("Can't export the song!"));
			return FALSE;
		}
	}
	if (ext == "TEX") {
		switch (globalTexExpMode) {
		case 0: ret = tv->sng()->save_to_tex_tab(fn); break;
		case 1: ret = tv->sng()->save_to_tex_notes(fn); break;
		default: ret = FALSE; break;
		}
		if (!ret) {
			KMessageBox::sorry(p, i18n("Can't export the song!"));
			return FALSE;
		}
	}
	return ret;
}

void KGuitarPart::filePrint()
{

	KMessageBox::sorry(p, i18n("Printing is at this time not implemented. If you would\n"
							   "help to implement this feature in KGuitar please write\n"
							   "an email to: greycat@users.sourceforge.net"));

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

void KGuitarPart::insertChord()
{
	int a[MAX_STRINGS];

	ChordSelector cs(tv->devMan(), tv->trk());
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

void KGuitarPart::songProperties()
{
	SetSong *ss = new SetSong();
	ss->title->setText(tv->sng()->title);
	ss->title->setReadOnly(isBrowserView);
	ss->author->setText(tv->sng()->author);
	ss->author->setReadOnly(isBrowserView);
	ss->transcriber->setText(tv->sng()->transcriber);
	ss->transcriber->setReadOnly(isBrowserView);
	ss->comments->setText(tv->sng()->comments);
	ss->comments->setReadOnly(isBrowserView);

	if (ss->exec()) {
		tv->sng()->title = ss->title->text();
		tv->sng()->author = ss->author->text();
		tv->sng()->transcriber = ss->transcriber->text();
		tv->sng()->comments = ss->comments->text();
	}

	delete ss;
}

void KGuitarPart::trackProperties()
{
	SetTrack *st = new SetTrack();

	st->title->setText(tv->trk()->name);
	st->title->setReadOnly(isBrowserView);
	st->channel->setValue(tv->trk()->channel);
	st->channel->setDisabled(isBrowserView);
	st->bank->setValue(tv->trk()->bank);
	st->bank->setDisabled(isBrowserView);
	st->patch->setValue(tv->trk()->patch);
	st->patch->setDisabled(isBrowserView);
	st->mode->setDisabled(isBrowserView);

	st->fret->setString(tv->trk()->string);
	st->fret->setFrets(tv->trk()->frets);
	for (int i = 0; i < tv->trk()->string; i++)
		st->fret->setTune(i, tv->trk()->tune[i]);
	st->fret->setDisabled(isBrowserView);

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

void KGuitarPart::options()
{
	Options *op = new Options(tv->devMan());

	op->maj7gr->setButton(globalMaj7);
	op->flatgr->setButton(globalFlatPlus);

	op->texsizegr->setButton(globalTabSize);
	op->showbarnumb->setChecked(globalShowBarNumb);
	op->showstr->setChecked(globalShowStr);
	op->showpagenumb->setChecked(globalShowPageNumb);
	op->texexpgr->setButton(globalTexExpMode);

	op->exec();

	delete op;
}

void KGuitarPart::slotConfigToolBars()
{
	emit configToolBars();
}

void KGuitarPart::configKeys()
{
	KKeyDialog::configureKeys(actionCollection(), "kguitar_part.rc");
}

void KGuitarPart::readOptions()
{
	KConfig *config = KGuitarFactory::instance()->config();

	config->setGroup("General");
	globalMaj7 = config->readNumEntry("Maj7", 0);
	globalFlatPlus = config->readNumEntry("FlatPlus", 0);
	globalNoteNames = config->readNumEntry("NoteNames", 0);

	config->setGroup("MusiXTeX");
	globalTabSize = config->readNumEntry("TabSize", 2);
	globalShowBarNumb = config->readBoolEntry("ShowBarNumb", TRUE);
	globalShowStr = config->readBoolEntry("ShowStr", TRUE);
	globalShowPageNumb = config->readBoolEntry("ShowPageNumb", TRUE);
	globalTexExpMode = config->readNumEntry("TexExpMode", 0);

// 	config->setGroup("ALSA");
// 	globalAlsaClient = config->readNumEntry("client", 64);
// 	globalAlsaPort = config->readNumEntry("port", 0);

	config->setGroup("MIDI");
	int d = config->readNumEntry("Device", 0);
	tv->devMan()->setDefaultDevice(d);
}

void KGuitarPart::saveOptions()
{
	kdDebug() << "KGuitarPart::saveOptions()" << endl;

	if (isBrowserView) {
		kdDebug() << "Nothing to save if loaded in Konqueror" << endl;
		return;
	}

	KConfig *config = KGuitarFactory::instance()->config();

	config->setGroup("General");
	config->writeEntry("Maj7", globalMaj7);
	config->writeEntry("FlatPlus", globalFlatPlus);
	config->writeEntry("NoteNames", globalNoteNames);

	config->setGroup("MusiXTeX");
	config->writeEntry("TabSize", globalTabSize);
	config->writeEntry("ShowBarNumb", globalShowBarNumb);
	config->writeEntry("ShowStr", globalShowStr);
	config->writeEntry("ShowPageNumb", globalShowPageNumb);
	config->writeEntry("TexExpMode", globalTexExpMode);

// 	config->setGroup("ALSA");
// 	config->writeEntry("Client", globalAlsaClient);
// 	config->writeEntry("Port", globalAlsaPort);

	config->setGroup("MIDI");
	config->writeEntry("Device", tv->devMan()->defaultDevice());

	config->sync();

	kdDebug() << "KGuitarPart::saveOptions() => all things saved..." << endl;
}

void KGuitarPart::setWinCaption(const QString& caption)
{
	emit setWindowCaption(caption);
}


//-------------------------------------------------------------------


KGuitarBrowserExtension::KGuitarBrowserExtension(KGuitarPart *parent)
	: KParts::BrowserExtension(parent, "KGuitarBrowserExtension")
{
}

void KGuitarBrowserExtension::print()
{
	((KGuitarPart *)parent())->filePrint();
}
