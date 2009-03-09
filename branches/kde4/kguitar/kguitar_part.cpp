#include "global.h"

#include "kguitar_part.h"

#include "songprint.h"
#include "songview.h"
#include "trackview.h"
#include "tracklist.h"
#include "tabsong.h"
#include "setsong.h"
#include "options.h"
#include "melodyeditor.h"
#include "trackdrag.h"
#include "settings.h"

#include "convertkg.h"
#include "convertascii.h"
#include "convertxml.h"
#include "convertmidi.h"
#include "converttse3.h"
#include "converttex.h"
#include "convertgtp.h"

#include "optionsexportascii.h"
#include "optionsexportmusixtex.h"

// KDE system things
#include <kparts/genericfactory.h>

#include <kapp.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kaccel.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kkeydialog.h>
#include <kdebug.h>
#include <kprinter.h>

#include <qwidget.h>

#include <qpixmap.h>
#include <qnamespace.h>
#include <qstatusbar.h>
#include <qclipboard.h>

#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qfileinfo.h>
//Added by qt3to4:
#include <Q3Frame>

typedef KParts::GenericFactory<KGuitarPart> KGuitarPartFactory;
K_EXPORT_COMPONENT_FACTORY(libkguitarpart, KGuitarPartFactory)

// Global variables - real declarations

QString drum_abbr[128];

KGuitarPart::KGuitarPart(QWidget *parentWidget,
                         const char * /*widgetName*/, QObject *parent, const char *name,
                         const QStringList & /*args*/)
	: KParts::ReadWritePart(parent, name)
{
	Settings::config = KGuitarPartFactory::instance()->config();

	cmdHist = new K3CommandHistory();

	setInstance(KGuitarPartFactory::instance());

	// Custom main widget
	sv = new SongView(this, cmdHist, parentWidget);

	// notify the part that this is our internal widget
	setWidget(sv);

	setupActions();
	setupAccels();

	// SET UP RESPONSES FOR VARIOUS TRACK CHANGES

	connect(sv->tv, SIGNAL(trackChanged(TabTrack *)), SLOT(updateToolbars(TabTrack *)));
	connect(QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(clipboardDataChanged()));
	connect(sv->tv, SIGNAL(barChanged()), SLOT(updateStatusBar()));

	setXMLFile("kguitar_part.rc");

	setReadWrite(true);
	setModified(false);

	// READ CONFIGS
	readOptions();

	readMidiNames();
}

KGuitarPart::~KGuitarPart()
{
	saveOptions();
	delete cmdHist;
}

void KGuitarPart::setReadWrite(bool rw)
{
	sv->setReadOnly(!rw);
	if (rw)	{
		connect(sv, SIGNAL(songChanged()), this, SLOT(setModified()));
	} else {
		disconnect(sv, SIGNAL(songChanged()), this, SLOT(setModified()));
	}

	ReadWritePart::setReadWrite(rw);
}

void KGuitarPart::setModified(bool modified)
{
	// enable or disable Save action based on modified
	KAction *save = actionCollection()->action(KStdAction::stdName(KStdAction::Save));
	if (!save)
		return;

	save->setEnabled(modified);

	// in any event, we want our parent to do it's thing
	ReadWritePart::setModified(modified);
}

KAboutData *KGuitarPart::createAboutData()
{
	KAboutData *aboutData = new KAboutData("kguitar", "KGuitarPart", VERSION);
	aboutData->addAuthor(i18n("KGuitar development team"), 0, 0);
	return aboutData;
}

// Reimplemented method from KParts to open file m_file
bool KGuitarPart::openFile()
{
	QFileInfo fi(m_file);

	if (!fi.isFile()) {
		KMessageBox::sorry(0, i18n("No file specified, please select a file."));
		return FALSE;
	}
	if (!fi.isReadable()) {
		KMessageBox::sorry(0, i18n("You have no permission to read this file."));
		return FALSE;
	}

	bool success = FALSE;

	QString ext = fi.extension();
	ext = ext.lower();

	ConvertBase *converter = converterForExtension(ext, sv->song());

	try {
		if (converter)  success = converter->load(m_file);
	} catch (QString msg) {
		kdDebug() << "Converter failed with message \"" << msg << "\"\n";
		KMessageBox::sorry(0, msg, i18n("Loading failed"));

		sv->song()->t.clear();
		sv->song()->t.append(new TabTrack(TabTrack::FretTab, i18n("Guitar"), 1, 0, 25, 6, 24));
		sv->refreshView();
		cmdHist->clear();

		return FALSE;
	}

	if (success) {
		sv->refreshView();
		cmdHist->clear();
	} else {
		setWinCaption(i18n("Unnamed"));
		KMessageBox::sorry(0, i18n("Can't load or import song!"
		                           "It may be a damaged/wrong file format or, "
		                           "if you're trying experimental importers "
		                           "it may be a flaw with the import code."));
	}

	return success;
}

bool KGuitarPart::exportOptionsDialog(QString ext)
{
	OptionsPage *op;
	KDialogBase opDialog(0, 0, TRUE, i18n("Additional Export Options"),
	                     KDialogBase::Help|KDialogBase::Default|
	                     KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok);

	Q3VBox *box = opDialog.makeVBoxMainWidget();

	if (ext == "tab") {
		op = new OptionsExportAscii(Settings::config, (Q3Frame *) box);
	} else if (ext == "tex") {
		op = new OptionsExportMusixtex(Settings::config, (Q3Frame *) box);
	} else {
		return TRUE;
	}

	// Skip the dialog if a user has set the appropriate option
	if (!Settings::config->readBoolEntry("AlwaysShow", TRUE)) {
		delete op;
		return TRUE;
	}

	connect(&opDialog, SIGNAL(defaultClicked()), op, SLOT(defaultBtnClicked()));
	connect(&opDialog, SIGNAL(okClicked()), op, SLOT(applyBtnClicked()));

	bool res = (opDialog.exec() == QDialog::Accepted);
	delete op;
	return res;
}

ConvertBase* KGuitarPart::converterForExtension(QString ext, TabSong *song)
{
	ConvertBase *converter = NULL;

	if (ext == "kg")   converter = new ConvertKg(song);
	if (ext == "tab")  converter = new ConvertAscii(song);
#ifdef WITH_TSE3
	if (ext == "mid")  converter = new ConvertMidi(song);
	if (ext == "tse3")  converter = new ConvertTse3(song);
#endif
	if (ext == "gtp" || ext == "gp3" || ext == "gp4" || ext == "gp5")  converter = new ConvertGtp(song);
	if (ext == "xml")  converter = new ConvertXml(song);
	if (ext == "tex")  converter = new ConvertTex(song);
	if (converter) {
		return converter;
	} else {
		throw i18n("Unable to handle file type \"%1\"").arg(ext);
	}
}

// Reimplemented method from KParts to current song to file m_file
bool KGuitarPart::saveFile()
{
	// if we aren't read-write, return immediately
	if (isReadWrite() == false)
		return false;

	// GREYFIX: some sort of dirty hack - workaround the KDE default
	// save, not saveAs without file name
	if (m_file.isEmpty()) {
		fileSaveAs();
		return false;
	}

	QFileInfo *fi = new QFileInfo(m_file);
	QString ext = fi->extension().lower();

	bool success = FALSE;

	try {
		if (exportOptionsDialog(ext)) {
			ConvertBase *converter = converterForExtension(ext, sv->song());
			if (converter)  success = converter->save(m_file);
		} else {
			return FALSE;
		}
	} catch (QString msg) {
		kdDebug() << "Converter failed with message \"" << msg << "\"\n";
		KMessageBox::sorry(0, msg, i18n("Loading failed"));

		sv->song()->t.clear();
		sv->song()->t.append(new TabTrack(TabTrack::FretTab, i18n("Guitar"), 1, 0, 25, 6, 24));
		sv->refreshView();
		cmdHist->clear();

		return FALSE;
	}

	if (success) {
		setWinCaption(m_file);
		cmdHist->clear();
	} else {
		KMessageBox::sorry(0, i18n("Can't save song in %1 format").arg(ext));
	}

	return success;
}

void KGuitarPart::fileSaveAs()
{
	QString filter =
		"*.kg|" + i18n("KGuitar files") + " (*.kg)\n"
		"*.tab|" + i18n("ASCII files") + " (*.tab)\n"
		"*.mid|" + i18n("MIDI files") + " (*.mid)\n"
		"*.tse3|" + i18n("TSE3MDL files") + " (*.tse3)\n"
		"*.gp4|" + i18n("Guitar Pro 4 files") + " (*.gp4)\n"
		"*.gp3|" + i18n("Guitar Pro 3 files") + " (*.gp3)\n"
		"*.xml|" + i18n("MusicXML files") + " (*.xml)\n"
		"*.tex|" + i18n("MusiXTeX") + " (*.tex)\n"
		"*|" + i18n("All files");
	QString file_name = KFileDialog::getSaveFileName(QString::null, filter);

	if (file_name.isEmpty() == false)
		saveAs(file_name);
}

// Updates possibility of actions, depending on freshly selected
// track. For drum track, lots of actions are unavailable.
void KGuitarPart::updateToolbars(TabTrack *)
{
	switch (sv->tv->trk()->trackMode()) {
	case TabTrack::DrumTab:
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
		sv->midiScheduler(),
#endif
		KGuitarPartFactory::instance()->config());
	op.exec();
	sv->me->drawBackground();
}

void KGuitarPart::readOptions()
{
	KConfig *config = KGuitarPartFactory::instance()->config();

// 	config->setGroup("MusiXTeX");
// 	globalTabSize = config->readNumEntry("TabSize", 2);
// 	globalShowBarNumb = config->readBoolEntry("ShowBarNumb", TRUE);
// 	globalShowStr = config->readBoolEntry("ShowStr", TRUE);
// 	globalShowPageNumb = config->readBoolEntry("ShowPageNumb", TRUE);
// 	globalTexExpMode = config->readNumEntry("TexExpMode", 0);

 	viewMelodyEditorAct->setChecked(config->readBoolEntry("Visible", TRUE));
	viewMelodyEditor();
//	viewScoreAct->setChecked(TRUE);		// LVIFIX: read value from config, enable only if feta fonts found
	viewScoreAct->setChecked(FALSE);	// LVIFIX: enable before commit
	viewScore();
}

void KGuitarPart::saveOptions()
{
 	Settings::config->setGroup("MelodyEditor");
	Settings::config->writeEntry("Visible", viewMelodyEditorAct->isChecked());
	Settings::config->sync();
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

// KGuitarBrowserExtension::KGuitarBrowserExtension(KGuitarPart *parent)
// 	: KParts::BrowserExtension(parent, "KGuitarBrowserExtension")
// {
// }

// void KGuitarBrowserExtension::print()
// {
// 	((KGuitarPart *)parent())->filePrint();
// }

void KGuitarPart::viewMelodyEditor()
{
	if (viewMelodyEditorAct->isChecked())
		sv->me->show();
	else
		sv->me->hide();
}

void KGuitarPart::viewScore()
{
	if (viewScoreAct->isChecked() /* && sv->sp->fFeta LVIFIX: enable after drawing code merge */)
		sv->tv->viewScore(true);
	else
		sv->tv->viewScore(false);
}

void KGuitarPart::setupActions()
{
	// SET UP STANDARD ACTIONS
	KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
	KStdAction::save(this, SLOT(save()), actionCollection());

	(void) KStdAction::print(this, SLOT(filePrint()), actionCollection());
	(void) KStdAction::preferences(this, SLOT(options()), actionCollection());

	(void) new KAction(i18n("P&roperties..."), 0, sv, SLOT(songProperties()),
	                   actionCollection(), "song_properties");

	// EDIT ACTIONS
	(void) KStdAction::undo(cmdHist, SLOT(undo()), actionCollection());
	(void) KStdAction::redo(cmdHist, SLOT(redo()), actionCollection());
	(void) KStdAction::cut(sv, SLOT(slotCut()), actionCollection());
	(void) KStdAction::copy(sv, SLOT(slotCopy()), actionCollection());
	(void) KStdAction::paste(sv, SLOT(slotPaste()), actionCollection());
	(void) KStdAction::selectAll(sv, SLOT(slotSelectAll()), actionCollection());

	// VIEW ACTIONS
	(void) KStdAction::zoomIn(sv->tv, SLOT(zoomIn()), actionCollection());
	(void) KStdAction::zoomOut(sv->tv, SLOT(zoomOut()), actionCollection());
	(void) KStdAction::zoom(sv->tv, SLOT(zoomLevelDialog()), actionCollection());
	viewMelodyEditorAct = new KToggleAction(i18n("Show Melody Editor"), "melodyeditor",
	                                        SHIFT + Key_M, this, SLOT(viewMelodyEditor()),
	                                        actionCollection(), "view_melodyEditor");
	viewScoreAct = new KToggleAction(i18n("Show Score"), "score", SHIFT + Key_S,
	                                 this, SLOT(viewScore()),
	                                 actionCollection(), "view_score");

	// TRACK ACTIONS
	trkNewAct = new KAction(i18n("&New..."), 0, sv, SLOT(trackNew()),
	                        actionCollection(), "track_new");
	trkDeleteAct = new KAction(i18n("&Delete"), 0, sv, SLOT(trackDelete()),
	                           actionCollection(), "track_delete");
	trkBassLineAct = new KAction(i18n("&Generate Bass Line"), 0, sv, SLOT(trackBassLine()),
	                             actionCollection(), "track_bassline");
	trkPropAct = new KAction(i18n("P&roperties..."), 0, sv, SLOT(trackProperties()),
	                         actionCollection(), "track_properties");
	rhythmerAct = new KAction(i18n("&Rhythm..."), "rhythmer", SHIFT + Key_R,
	                          sv->tv, SLOT(rhythmer()), actionCollection(), "rhythmer");
	insChordAct = new KAction(i18n("&Chord..."), "chord", SHIFT + Key_C,
	                          sv->tv, SLOT(insertChord()), actionCollection(), "insert_chord");

	saveOptionAct = new KAction(i18n("&Save Options"), 0, this,
	                            SLOT(saveOptions()), actionCollection(), "save_options");

	arrTrkAct = new KAction(i18n("&Arrange Track"), SHIFT + Key_A, sv->tv,
	                        SLOT(arrangeTracks()), actionCollection(), "arrange_trk");

	// LESS THAN TRIVIAL MOVING STUFF ACTIONS
	(void) new KAction(i18n("Transpose up"), 0, CTRL + Key_Up,
					   sv->tv, SLOT(transposeUp()), actionCollection(), "transpose_up");
	(void) new KAction(i18n("Transpose down"), 0, CTRL + Key_Down,
					   sv->tv, SLOT(transposeDown()), actionCollection(), "transpose_down");

	// SET UP DURATION
	(void) new KAction(i18n("Whole"), "note1", CTRL + Key_1,
	                   sv->tv, SLOT(setLength1()), actionCollection(), "set_len1");
	(void) new KAction("1/2", "note2", CTRL + Key_2,
	                   sv->tv, SLOT(setLength2()), actionCollection(), "set_len2");
	(void) new KAction("1/4", "note4", CTRL + Key_3,
	                   sv->tv, SLOT(setLength4()), actionCollection(), "set_len4");
	(void) new KAction("1/8", "note8", CTRL + Key_4,
	                   sv->tv, SLOT(setLength8()), actionCollection(), "set_len8");
	(void) new KAction("1/16", "note16", CTRL + Key_5,
	                   sv->tv, SLOT(setLength16()), actionCollection(), "set_len16");
	(void) new KAction("1/32", "note32", CTRL + Key_6,
	                   sv->tv, SLOT(setLength32()), actionCollection(), "set_len32");
	(void) new KAction(i18n("Dotted note"), "dotted_note", Key_Period,
	                   sv->tv, SLOT(dotNote()), actionCollection(), "dotted_note");
	(void) new KAction(i18n("Triplet note"), "triplet", Key_T,
	                   sv->tv, SLOT(tripletNote()), actionCollection(), "triplet");
	(void) new KAction(i18n("More duration"), 0, Key_Equal,
	                   sv->tv, SLOT(keyPlus()), actionCollection(), "more_duration");
	(void) new KAction(i18n("Less duration"), 0, Key_Minus,
	                   sv->tv, SLOT(keyMinus()), actionCollection(), "less_duration");

	// SET UP EFFECTS
	keySigAct = new KAction(i18n("Key signature"), "keysig", SHIFT + Key_K,
	                        sv->tv, SLOT(keySig()), actionCollection(), "key_sig");
	timeSigAct = new KAction(i18n("Time signature"), "timesig", SHIFT + Key_T,
	                         sv->tv, SLOT(timeSig()), actionCollection(), "time_sig");
	arcAct = new KAction(i18n("Link with previous column"), "arc", Key_L,
	                     sv->tv, SLOT(linkPrev()), actionCollection(), "link_prev");
	legatoAct = new KAction(i18n("Legato (hammer on/pull off)"), "fx_legato", Key_P,
	                        sv->tv, SLOT(addLegato()), actionCollection(), "fx_legato");
	slideAct = new KAction(i18n("Slide"), "fx_slide", Key_S,
	                       sv->tv, SLOT(addSlide()), actionCollection(), "fx_slide");
	letRingAct = new KAction(i18n("Let Ring"), "fx_let_ring", Key_I,
	                         sv->tv, SLOT(addLetRing()), actionCollection(), "fx_let_ring");
	natHarmAct = new KAction(i18n("Natural harmonic"), "fx_harmonic", Key_H,
	                         sv->tv, SLOT(addHarmonic()), actionCollection(), "fx_nat_harm"); 
	artHarmAct = new KAction(i18n("Artificial harmonic"), "fx_harmonic", Key_R,
	                         sv->tv, SLOT(addArtHarm()), actionCollection(), "fx_art_harm");
	palmMuteAct = new KAction(i18n("Palm muting"), "fx_palmmute", Key_M,
	                          sv->tv, SLOT(palmMute()), actionCollection(), "fx_palmmute");
	(void) new KAction(i18n("Dead note"), 0, Key_X,
	                   sv->tv, SLOT(deadNote()), actionCollection(), "deadnote");

	// SET UP 'Note Names'

	// SET UP MIDI-PLAY
	midiPlaySongAct = new KAction(i18n("&Play / stop"), "1rightarrow", Key_Space,
	                              sv, SLOT(playSong()), actionCollection(), "midi_playsong");
	midiStopPlayAct = new KAction(i18n("&Stop"), "player_stop", CTRL + SHIFT + Key_P,
	                              sv, SLOT(stopPlay()), actionCollection(), "midi_stopplay");
#ifndef WITH_TSE3
	midiPlaySongAct->setEnabled(FALSE);
	midiStopPlayAct->setEnabled(FALSE);
#endif
}

void KGuitarPart::setupAccels()
{
	// SET UP ACCEL...
	mainAccel = new KAccel(sv->tv);

	// ...FOR CURSOR
	mainAccel->insert("prev_column", i18n("Move cursor left"), QString::null,
	                  Key_Left, sv->tv, SLOT(keyLeft()));
	mainAccel->insert("next_column", i18n("Move cursor right"), QString::null,
	                  Key_Right, sv->tv, SLOT(keyRight()));
	mainAccel->insert("start_bar", i18n("Move cursor to the beginning of bar"), QString::null,
	                  Key_Home, sv->tv, SLOT(keyHome()));
	mainAccel->insert("end_bar", i18n("Move cursor to the end of bar"), QString::null,
	                  Key_End, sv->tv, SLOT(keyEnd()));
	mainAccel->insert("prev_bar", i18n("Move cursor to the previous bar"), QString::null,
	                  CTRL + Key_Left, sv->tv, SLOT(keyLeftBar()));
	mainAccel->insert("next_bar", i18n("Move cursor to the next bar"), QString::null,
	                  CTRL + Key_Right, sv->tv, SLOT(keyRightBar()));
	mainAccel->insert("start_track", i18n("Move cursor to the beginning of track"), QString::null,
	                  CTRL + Key_Home, sv->tv, SLOT(keyCtrlHome()));
	mainAccel->insert("end_track", i18n("Move cursor to the end of track"), QString::null,
	                  CTRL + Key_End, sv->tv, SLOT(keyCtrlEnd()));
	mainAccel->insert("select_prev_column", i18n("Move and select left"), QString::null,
	                  SHIFT + Key_Left, sv->tv, SLOT(selectLeft()));
	mainAccel->insert("select_next_column", i18n("Move and select right"), QString::null,
	                  SHIFT + Key_Right, sv->tv, SLOT(selectRight()));
	mainAccel->insert("key_up", i18n("Move cursor up"), QString::null,
	                  Key_Up, sv->tv, SLOT(moveUp()));
	mainAccel->insert("key_down", i18n("Move cursor down"), QString::null,
	                  Key_Down, sv->tv, SLOT(moveDown()));

	// ...FOR OTHER KEYS
	mainAccel->insert("key_del", i18n("Delete note"), QString::null,
	                  Key_Delete, sv->tv, SLOT(deleteNote()));
	mainAccel->insert("key_CtrlDel", i18n("Delete column"), QString::null,
	                  CTRL + Key_Delete, sv->tv, SLOT(deleteColumn()));
	mainAccel->insert("key_ins", i18n("Insert column"), QString::null,
	                  Key_Insert, sv->tv, SLOT(insertColumn()));

	// ...FOR KEY '0' - '9'
	mainAccel->insert("key_1", i18n("Key 1"), QString::null, Key_1, sv->tv, SLOT(key1()));
	mainAccel->insert("key_2", i18n("Key 2"), QString::null, Key_2, sv->tv, SLOT(key2()));
	mainAccel->insert("key_3", i18n("Key 3"), QString::null, Key_3, sv->tv, SLOT(key3()));
	mainAccel->insert("key_4", i18n("Key 4"), QString::null, Key_4, sv->tv, SLOT(key4()));
	mainAccel->insert("key_5", i18n("Key 5"), QString::null, Key_5, sv->tv, SLOT(key5()));
	mainAccel->insert("key_6", i18n("Key 6"), QString::null, Key_6, sv->tv, SLOT(key6()));
	mainAccel->insert("key_7", i18n("Key 7"), QString::null, Key_7, sv->tv, SLOT(key7()));
	mainAccel->insert("key_8", i18n("Key 8"), QString::null, Key_8, sv->tv, SLOT(key8()));
	mainAccel->insert("key_9", i18n("Key 9"), QString::null, Key_9, sv->tv, SLOT(key9()));
	mainAccel->insert("key_0", i18n("Key 0"), QString::null, Key_0, sv->tv, SLOT(key0()));
}

void KGuitarPart::clipboardDataChanged()
{
	KAction *paste = actionCollection()->action(KStdAction::stdName(KStdAction::Paste));
	if (!paste)
		return;
	paste->setEnabled(TrackDrag::canDecode(QApplication::clipboard()->data()));
}

void KGuitarPart::updateStatusBar()
{
	QString tmp;
	tmp.setNum(sv->tv->trk()->xb + 1);
	tmp = i18n("Bar: ") + tmp;
	emit setStatusBarText(tmp);
}
