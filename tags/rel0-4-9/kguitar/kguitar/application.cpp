#include "application.h"

#include "songview.h"
#include "trackview.h"
#include "tracklist.h"
#include "tabsong.h"
#include "setsong.h"
#include "options.h"
#include "melodyeditor.h"

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
#include <kurl.h>
#include <kkeydialog.h>
#include <kdebug.h>
#include <kprinter.h>

#include <qwidget.h>
#include <qcursor.h>

#include <qpixmap.h>
#include <qkeycode.h>
#include <qstatusbar.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qfileinfo.h>

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

// MIDI
int globalMidiPort;
bool globalHaveMidi;

// Printing
int globalPrSty;

QString drum_abbr[128];

bool isBrowserView;

extern "C" {
	void *init_libkguitar() { return new KGuitarFactory; }
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

KParts::Part *KGuitarFactory::createPartObject(QWidget *parentWidget, const char *widgetName,
                                               QObject *parent, const char *name, const char *className,
                                               const QStringList &)
{
	bool bBrowserView = (strcmp(className, "Browser/View") == 0);
	KParts::Part *obj = new KGuitarPart(bBrowserView, 0, parentWidget, widgetName,
	                                    parent, name);
	return obj;
}

KInstance *KGuitarFactory::instance()
{
	if (!s_instance)
		s_instance = new KInstance("kguitar");
	return s_instance;
}

//------------------------------------------------------------------------

KGuitarPart::KGuitarPart(bool bBrowserView, KCommandHistory *_cmdHist, QWidget *parentWidget,
						 const char *, QObject *parent, const char *name)
	: KParts::ReadWritePart(parent, name)
{
// 	printer = new QPrinter;
// 	printer->setMinMax(1,10);

	kdDebug() << "KGuitarPart::KGuitarPart() " << endl;

	p = parentWidget;
	isBrowserView = bBrowserView;
	cmdHist = _cmdHist;

	if (!cmdHist) {
		// We have no global KCommandHistory e.g. Part is called by Konqueror
		// so we create one
		cmdHist = new KCommandHistory();
	}

	setInstance(KGuitarFactory::instance());

	// MAIN WIDGET
	sv = new SongView(this, cmdHist, parentWidget);
	setWidget(sv);
	sv->setFocus();

	// SET UP STANDARD ACTIONS
	newAct = KStdAction::openNew(this, SLOT(fileNew()),
	                             actionCollection(), "file_new");

	preferencesAct = KStdAction::preferences(this, SLOT(options()),
	                                         actionCollection(), "pref_options");
	confTBAct = KStdAction::configureToolbars(this, SLOT(slotConfigToolBars()),
	                                          actionCollection(), "config_toolbars");
	confKeyAct = KStdAction::keyBindings(this, SLOT(configKeys()),
	                                     actionCollection(), "config_keys");

	sngPropAct = new KAction(i18n("P&roperties..."), 0, sv, SLOT(songProperties()),
	                         actionCollection(), "song_properties");

	// VIEW ACTIONS
	zoomInAct = new KAction(i18n("Zoom in"), "zoom_in", KAccel::stringToKey("Ctrl+="),
	                        sv->tv, SLOT(zoomIn()), actionCollection(), "zoom_in");
	zoomOutAct = new KAction(i18n("Zoom out"), "zoom_out", KAccel::stringToKey("Ctrl+-"),
	                         sv->tv, SLOT(zoomOut()), actionCollection(), "zoom_out");
	zoomLevelAct = new KAction(i18n("Zoom to..."), 0, sv->tv, SLOT(zoomLevelDialog()),
	                         actionCollection(), "zoom_level");

	// TRACK ACTIONS
	trkNewAct = new KAction(i18n("&New..."), 0, sv, SLOT(trackNew()),
	                        actionCollection(), "track_new");
	trkDeleteAct = new KAction(i18n("&Delete"), 0, sv, SLOT(trackDelete()),
	                           actionCollection(), "track_delete");
	trkBassLineAct = new KAction(i18n("&Generate Bass Line"), 0, sv, SLOT(trackBassLine()),
	                             actionCollection(), "track_bassline");
	trkPropAct = new KAction(i18n("P&roperties..."), 0, sv, SLOT(trackProperties()),
	                         actionCollection(), "track_properties");
	rhythmerAct = new KAction(i18n("&Rhythm..."), "rhythmer", KAccel::stringToKey("Shift+R"),
	                          sv->tv, SLOT(rhythmer()), actionCollection(), "rhythmer");
	insChordAct = new KAction(i18n("&Chord..."), "chord", KAccel::stringToKey("Shift+C"),
	                          sv->tv, SLOT(insertChord()), actionCollection(), "insert_chord");

	saveOptionAct = new KAction(i18n("&Save Options"), 0, this,
	                            SLOT(saveOptions()), actionCollection(), "save_options");

	arrTrkAct = new KAction(i18n("&Arrange Track"), KAccel::stringToKey("Shift+A"), sv->tv,
	                        SLOT(arrangeTracks()), actionCollection(), "arrange_trk");

	// SET UP DURATION
	len1Act = new KAction(i18n("Whole"), "note1", KAccel::stringToKey("Ctrl+1"),
						  sv->tv, SLOT(setLength1()), actionCollection(), "set_len1");
	len2Act = new KAction("1/2", "note2", KAccel::stringToKey("Ctrl+2"),
						  sv->tv, SLOT(setLength2()), actionCollection(), "set_len2");
	len4Act = new KAction("1/4", "note4", KAccel::stringToKey("Ctrl+3"),
						  sv->tv, SLOT(setLength4()), actionCollection(), "set_len4");
	len8Act = new KAction("1/8", "note8", KAccel::stringToKey("Ctrl+4"),
						  sv->tv, SLOT(setLength8()), actionCollection(), "set_len8");
	len16Act = new KAction("1/16", "note16", KAccel::stringToKey("Ctrl+5"),
						   sv->tv, SLOT(setLength16()), actionCollection(), "set_len16");
	len32Act = new KAction("1/32", "note32", KAccel::stringToKey("Ctrl+6"),
						   sv->tv, SLOT(setLength32()), actionCollection(), "set_len32");

	// SET UP EFFECTS
	keySigAct = new KAction(i18n("Key signature"), "keysig",
							 KAccel::stringToKey("Shift+K"), sv->tv, SLOT(keySig()),
							 actionCollection(), "key_sig");
	timeSigAct = new KAction(i18n("Time signature"), "timesig",
							 KAccel::stringToKey("Shift+T"), sv->tv, SLOT(timeSig()),
							 actionCollection(), "time_sig");
	arcAct = new KAction(i18n("Link with previous column"), "arc",
						 KAccel::stringToKey("L"), sv->tv, SLOT(linkPrev()),
						 actionCollection(), "link_prev");
	legatoAct = new KAction(i18n("Legato (hammer on/pull off)"), "fx_legato",
							KAccel::stringToKey("P"), sv->tv, SLOT(addLegato()),
							actionCollection(), "fx_legato");
	slideAct = new KAction(i18n("Slide"), "fx_slide",
						   KAccel::stringToKey("S"), sv->tv, SLOT(addSlide()),
						   actionCollection(), "fx_slide");
	letRingAct = new KAction(i18n("Let Ring"), "fx_let_ring",
							 KAccel::stringToKey("I"), sv->tv, SLOT(addLetRing()),
							 actionCollection(), "fx_let_ring");
	natHarmAct = new KAction(i18n("Natural harmonic"), "fx_harmonic",
							 KAccel::stringToKey("H"), sv->tv, SLOT(addHarmonic()),
							 actionCollection(), "fx_nat_harm");
	artHarmAct = new KAction(i18n("Artificial harmonic"), "fx_harmonic",
							 KAccel::stringToKey("R"), sv->tv, SLOT(addArtHarm()),
							 actionCollection(), "fx_art_harm");
	palmMuteAct = new KAction(i18n("Palm muting"), "fx_palmmute",
							 KAccel::stringToKey("M"), sv->tv, SLOT(palmMute()),
							 actionCollection(), "fx_palmmute");

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

    // SET UP MIDI-PLAY
	midiPlaySongAct = new KAction(i18n("&Play / stop"), "1rightarrow",
								   KAccel::stringToKey("Space"), sv, SLOT(playSong()),
								   actionCollection(), "midi_playsong");
	midiStopPlayAct = new KAction(i18n("&Stop"), "player_stop",
								  KAccel::stringToKey("Ctrl+Shift+P"), sv, SLOT(stopPlay()),
								  actionCollection(), "midi_stopplay");
#ifndef WITH_TSE3
	midiPlaySongAct->setEnabled(FALSE);
	midiStopPlayAct->setEnabled(FALSE);
#endif

	// SET UP ACCEL...
	mainAccel = new KAccel(sv->tv);

	// ...FOR CURSOR
	mainAccel->insertItem(i18n("Move cursor left"), "key_left", "Left");
	mainAccel->connectItem("key_left", sv->tv, SLOT(keyLeft()));
	mainAccel->insertItem(i18n("Move cursor right"), "key_right", "Right");
	mainAccel->connectItem("key_right", sv->tv, SLOT(keyRight()));
	mainAccel->insertItem(i18n("Move cursor to the beginning of bar"), "key_home", "Home");
	mainAccel->connectItem("key_home", sv->tv, SLOT(keyHome()));
	mainAccel->insertItem(i18n("Move cursor to the end of bar"), "key_end", "End");
	mainAccel->connectItem("key_end", sv->tv, SLOT(keyEnd()));
	mainAccel->insertItem(i18n("Move cursor to the beginning of track"), "key_CtrlHome", "Ctrl+Home");
	mainAccel->connectItem("key_CtrlHome", sv->tv, SLOT(keyCtrlHome()));
	mainAccel->insertItem(i18n("Move cursor to the end of track"), "key_CtrlEnd", "Ctrl+End");
	mainAccel->connectItem("key_CtrlEnd", sv->tv, SLOT(keyCtrlEnd()));
	mainAccel->insertItem(i18n("Move and select left"), "key_ShiftLeft", "Shift+Left");
	mainAccel->connectItem("key_ShiftLeft", sv->tv, SLOT(selectLeft()));
	mainAccel->insertItem(i18n("Move and select right"), "key_ShiftRight", "Shift+Right");
	mainAccel->connectItem("key_ShiftRight", sv->tv, SLOT(selectRight()));

	mainAccel->insertItem(i18n("Move cursor up"), "key_up", "Up");
	mainAccel->connectItem("key_up", sv->tv, SLOT(moveUp()));
	mainAccel->insertItem(i18n("Move cursor down"), "key_down", "Down");
	mainAccel->connectItem("key_down", sv->tv, SLOT(moveDown()));
	mainAccel->insertItem(i18n("Transpose up"), "key_CtrlUp", "Ctrl+Up");
	mainAccel->connectItem("key_CtrlUp", sv->tv, SLOT(transposeUp()));
	mainAccel->insertItem(i18n("Transpose down"), "key_CtrlDown", "Ctrl+Down");
	mainAccel->connectItem("key_CtrlDown", sv->tv, SLOT(transposeDown()));

    // ...FOR OTHER KEYS
	mainAccel->insertItem(i18n("Dead note"), "key_x", "X");
	mainAccel->connectItem("key_x", sv->tv, SLOT(deadNote()));
	mainAccel->insertItem(i18n("Delete note"), "key_del", "Delete");
	mainAccel->connectItem("key_del", sv->tv, SLOT(deleteNote()));
	mainAccel->insertItem(i18n("Delete column"), "key_CtrlDel", "Ctrl+Delete");
	mainAccel->connectItem("key_CtrlDel", sv->tv, SLOT(deleteColumn()));
	mainAccel->insertItem(i18n("Insert column"), "key_ins", "Insert");
	mainAccel->connectItem("key_ins", sv->tv, SLOT(insertColumn()));
	mainAccel->insertItem(i18n("Dotted note"), "key_period", "Period");
	mainAccel->connectItem("key_period", sv->tv, SLOT(dotNote()));
	mainAccel->insertItem(i18n("Triplet note"), "key_t", "T");
	mainAccel->connectItem("key_t", sv->tv, SLOT(tripletNote()));
	mainAccel->insertItem(i18n("More duration"), "key_equal", "Equal");
	mainAccel->connectItem("key_equal", sv->tv, SLOT(keyPlus()));
	mainAccel->insertItem(i18n("Less duration"), "key_minus", "Minus");
	mainAccel->connectItem("key_minus", sv->tv, SLOT(keyMinus()));

    // ...FOR KEY '0' - '9'
	mainAccel->insertItem(i18n("Key 1"), "key_1", "1");
	mainAccel->connectItem("key_1", sv->tv, SLOT(key1()));
	mainAccel->insertItem(i18n("Key 2"), "key_2", "2");
	mainAccel->connectItem("key_2", sv->tv, SLOT(key2()));
	mainAccel->insertItem(i18n("Key 3"), "key_3", "3");
	mainAccel->connectItem("key_3", sv->tv, SLOT(key3()));
	mainAccel->insertItem(i18n("Key 4"), "key_4", "4");
	mainAccel->connectItem("key_4", sv->tv, SLOT(key4()));
	mainAccel->insertItem(i18n("Key 5"), "key_5", "5");
	mainAccel->connectItem("key_5", sv->tv, SLOT(key5()));
	mainAccel->insertItem(i18n("Key 6"), "key_6", "6");
	mainAccel->connectItem("key_6", sv->tv, SLOT(key6()));
	mainAccel->insertItem(i18n("Key 7"), "key_7", "7");
	mainAccel->connectItem("key_7", sv->tv, SLOT(key7()));
	mainAccel->insertItem(i18n("Key 8"), "key_8", "8");
	mainAccel->connectItem("key_8", sv->tv, SLOT(key8()));
	mainAccel->insertItem(i18n("Key 9"), "key_9", "9");
	mainAccel->connectItem("key_9", sv->tv, SLOT(key9()));
	mainAccel->insertItem(i18n("Key 0"), "key_0", "0");
	mainAccel->connectItem("key_0", sv->tv, SLOT(key0()));

	// SET UP RESPONSES FOR VARIOUS TRACK CHANGES

	connect(sv->tv, SIGNAL(trackChanged(TabTrack *)), SLOT(updateToolbars(TabTrack *)));

	m_extension = new KGuitarBrowserExtension(this);

	if (bBrowserView) {
		mainAccel->setEnabled(FALSE);
		sngPropAct->setText(i18n("Song Properties..."));
		trkPropAct->setText(i18n("Track Properties..."));
		setXMLFile("kguitar_konq.rc");
	} else {
		setXMLFile("kguitar_part.rc");
	}

	// READ CONFIGS
	readOptions();

	readMidiNames();

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

// Updates possibility of actions, depending on freshly selected
// track. For drum track, lots of actions are unavailable.
void KGuitarPart::updateToolbars(TabTrack *)
{
	switch (sv->tv->trk()->trackMode() == DrumTab) {
	case DrumTab:
		insChordAct->setEnabled(FALSE);
		natHarmAct->setEnabled(FALSE);
		artHarmAct->setEnabled(FALSE);
		break;
	default:
		insChordAct->setEnabled(TRUE);
		natHarmAct->setEnabled(TRUE);
		artHarmAct->setEnabled(TRUE);
	}
}

void KGuitarPart::fileNew()
{
// 	KGuitarPart *ed = new KGuitarPart;
// 	ed->resize(400, 400);
// 	ed->show();
}

// Reimplemented method from KParts to current song to file m_file
bool KGuitarPart::saveFile()
{
	QFileInfo *fi = new QFileInfo(m_file);
	QString ext = fi->extension().lower();

	bool success = FALSE;

	if (ext == "kg") {
		sv->tv->arrangeBars(); // GREYFIX !
		success = sv->sng()->save_to_kg(m_file);
	}
	if (ext == "tab")
		success = sv->sng()->save_to_tab(m_file);
#ifdef WITH_TSE3
	if (ext == "mid")
		success = sv->sng()->save_to_mid(m_file);
	if (ext == "tse3")
		success = sv->sng()->save_to_tse3(m_file);
#endif
	if (ext == "gtp")
		success = sv->sng()->save_to_gtp(m_file);
	if (ext == "gp3")
		success = sv->sng()->save_to_gp3(m_file);
	if (ext == "tex") {
		switch (globalTexExpMode) {
		case 0: success = sv->sng()->save_to_tex_tab(m_file); break;
		case 1: success = sv->sng()->save_to_tex_notes(m_file); break;
		default: success = FALSE; break;
		}
	}
	if (ext == "xml")
		success = sv->sng()->save_to_xml(m_file);

	if (success) {
		setWinCaption(m_file);
		cmdHist->clear();
	} else {
		KMessageBox::sorry(p, i18n("Can't save song in %1 format").arg(ext));
	}

	return success;
}

// Reimplemented method from KParts to open file m_file
bool KGuitarPart::openFile()
{
// 	if (m_file.isEmpty()) {
// 		KMessageBox::sorry(p, i18n("
// 		return FALSE;
// 	}

	QFileInfo *fi = new QFileInfo(m_file);

	if (!fi->isFile()) {
		KMessageBox::sorry(p, i18n("No file specified, please select a file."));
		return FALSE;
	}
	if (!fi->isReadable()) {
		KMessageBox::sorry(p, i18n("You have no permission to read this file."));
		return FALSE;
	}

	bool success = FALSE;

	QString ext = fi->extension();
	ext = ext.lower();

	if (ext == "kg")
		success = sv->sng()->load_from_kg(m_file);
	if (ext == "tab")
		success = sv->sng()->load_from_tab(m_file);
#ifdef WITH_TSE3
	if (ext == "mid")
		success = sv->sng()->load_from_mid(m_file);
#endif
	if (ext == "gtp")
		success = sv->sng()->load_from_gtp(m_file);
	if (ext == "gp3")
		success = sv->sng()->load_from_gp3(m_file);
	if (ext == "xml")
		success = sv->sng()->load_from_xml(m_file);

	if (success) {
		setWinCaption(m_file);
		sv->refreshView();
		cmdHist->clear();
	} else {
		setWinCaption(i18n("Unnamed"));
		KMessageBox::sorry(p, i18n("Can't load or import song!"
								   "It may be a damaged/wrong file format or, "
								   "if you're trying experimental importers "
								   "it may be a flaw with the import code."));
	}

	return success;
}

void KGuitarPart::filePrint()
{
// LVIFIX: enable status message
//  slotStatusMsg(i18n("Printing..."));

	KPrinter printer(true, QPrinter::HighResolution);
	if (printer.setup())
		sv->print(&printer);

//  slotStatusMsg(i18n("Ready."));
}

void KGuitarPart::options()
{
	Options op(
#ifdef WITH_TSE3
		sv->midiScheduler()
#endif
		);
	op.exec();
	sv->me->drawBackground();
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

 	config->setGroup("TSE3");
 	globalMidiPort = config->readNumEntry("Port", 64);

	config->setGroup("Printing");
	globalPrSty = config->readNumEntry("PrSty", 0);

	config->setGroup("MelodyEditor");
	globalMelodyEditorInlay = config->readNumEntry("Inlay", 1);
	globalMelodyEditorWood = config->readNumEntry("Wood",  2);
	globalMelodyEditorAction[0] = config->readNumEntry("Action0", 1);
	globalMelodyEditorAdvance[0] = config->readBoolEntry("Advance0", FALSE);
	globalMelodyEditorAction[1] = config->readNumEntry("Action1", 3);
	globalMelodyEditorAdvance[1] = config->readBoolEntry("Advance1", TRUE);
	globalMelodyEditorAction[2] = config->readNumEntry("Action2", 1);
	globalMelodyEditorAdvance[2] = config->readBoolEntry("Advance2", TRUE);
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

 	config->setGroup("TSE3");
 	config->writeEntry("Port", globalMidiPort);

	config->setGroup("Printing");
	config->writeEntry("PrSty", globalPrSty);

	config->setGroup("MelodyEditor");
	config->writeEntry("Inlay", globalMelodyEditorInlay);
	config->writeEntry("Wood", globalMelodyEditorWood);
	config->writeEntry("Action0", globalMelodyEditorAction[0]);
	config->writeEntry("Advance0", globalMelodyEditorAdvance[0]);
	config->writeEntry("Action1", globalMelodyEditorAction[1]);
	config->writeEntry("Advance1", globalMelodyEditorAdvance[1]);
	config->writeEntry("Action2", globalMelodyEditorAction[2]);
	config->writeEntry("Advance2", globalMelodyEditorAdvance[2]);

	config->sync();

	kdDebug() << "KGuitarPart::saveOptions() => all things saved..." << endl;
}

void KGuitarPart::readMidiNames()
{
	drum_abbr[35] = QString("BD1");
	drum_abbr[36] = QString("BD2");
	drum_abbr[38] = QString("SD1");
	drum_abbr[40] = QString("SD2");

	drum_abbr[39] = QString("HCL"); // Hand clap

	drum_abbr[42] = QString("CHH");
	drum_abbr[44] = QString("PHH");
	drum_abbr[46] = QString("OHH");

	drum_abbr[49] = QString("CR1"); // Crash cymbal
	drum_abbr[57] = QString("CR2");

	drum_abbr[51] = QString("RI1"); // Ride cymbal
	drum_abbr[59] = QString("RI2");

	drum_abbr[54] = QString("TBR"); // Tamborine
	drum_abbr[55] = QString("SPL"); // Splash cymbal

	drum_abbr[41] = QString("TL2");
	drum_abbr[43] = QString("TL1");
	drum_abbr[45] = QString("TM2");
	drum_abbr[47] = QString("TM1");
	drum_abbr[48] = QString("TH2");
	drum_abbr[50] = QString("TH1");
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
