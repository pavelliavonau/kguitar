#include "kguitar_part.h"

#include "songview.h"
#include "trackview.h"
#include "tracklist.h"
#include "tabsong.h"
#include "setsong.h"
#include "options.h"
#include "melodyeditor.h"
#include "trackdrag.h"
#include "settings.h"

#include "convertascii.h"

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
#include <qkeycode.h>
#include <qstatusbar.h>
#include <qclipboard.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qfileinfo.h>

typedef KParts::GenericFactory<KGuitarPart> KGuitarPartFactory;
K_EXPORT_COMPONENT_FACTORY(libkguitarpart, KGuitarPartFactory);

// Global variables - real declarations

QString drum_abbr[128];

// bool isBrowserView;

// extern "C" {
// 	void *init_libkguitar() { return new KGuitarPartFactory; }
// };

// KInstance *KGuitarPartFactory::s_instance = 0L;

// KGuitarPartFactory::KGuitarPartFactory()
// {
// }

// KGuitarPartFactory::~KGuitarPartFactory()
// {
// 	if (s_instance)
// 		delete s_instance;
// 	s_instance = 0;
// }

// KParts::Part *KGuitarPartFactory::createPartObject(QWidget *parentWidget, const char *widgetName,
//                                                QObject *parent, const char *name, const char *className,
//                                                const QStringList &)
// {
// 	bool bBrowserView = (strcmp(className, "Browser/View") == 0);
// 	KParts::Part *obj = new KGuitarPart(bBrowserView, 0, parentWidget, widgetName,
// 	                                    parent, name);
// 	return obj;
// }

// KInstance *KGuitarPartFactory::instance()
// {
// 	if (!s_instance)
// 		s_instance = new KInstance("kguitar");
// 	return s_instance;
// }

//------------------------------------------------------------------------

// GREYFIX: old code:
// KGuitarPart::KGuitarPart(bool bBrowserView, KCommandHistory *_cmdHist, QWidget *parentWidget,
// 						 const char * /*widgetName*/, QObject *parent, const char *name,
// 						 const QStringList & /*args*/)

KGuitarPart::KGuitarPart(QWidget *parentWidget,
						 const char * /*widgetName*/, QObject *parent, const char *name,
						 const QStringList & /*args*/)
	: KParts::ReadWritePart(parent, name)
{
	Settings::config = KGuitarPartFactory::instance()->config();

//	p = parentWidget;
// 	isBrowserView = bBrowserView;
// 	cmdHist = _cmdHist;

// 	if (!cmdHist) {
		// We have no global KCommandHistory e.g. Part is called by Konqueror
		// so we create one
		cmdHist = new KCommandHistory();
// 	}

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

// 	m_extension = new KGuitarBrowserExtension(this);

// 	if (bBrowserView) {
// 		mainAccel->setEnabled(FALSE);
// 		sngPropAct->setText(i18n("Song Properties..."));
// 		trkPropAct->setText(i18n("Track Properties..."));
// 		setXMLFile("kguitar_konq.rc");
// 	} else {
	setXMLFile("kguitar_part.rc");
// 	}

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
		connect(sv, SIGNAL(songChanged()),
		        this, SLOT(setModified()));
	} else {
		disconnect(sv, SIGNAL(songChanged()),
		           this, SLOT(setModified()));
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
	KAboutData *aboutData = new KAboutData("kguitar", I18N_NOOP("KGuitarPart"), VERSION);
    aboutData->addAuthor("KGuitar development team", 0, 0);
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

	if (ext == "kg")
		success = sv->song()->loadFromKg(m_file);
	if (ext == "tab") {
		ConvertAscii converter(sv->song());
		success = converter.load(m_file);
	}
#ifdef WITH_TSE3
	if (ext == "mid")
		success = sv->song()->loadFromMid(m_file);
#endif
	if (ext == "gtp")
		success = sv->song()->loadFromGtp(m_file);
	if (ext == "gp3")
		success = sv->song()->loadFromGp3(m_file);
	if (ext == "xml")
		success = sv->song()->loadFromXml(m_file);

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
	// Skip dialog if user set appopriate option
	if (!Settings::config->readBoolEntry("AlwaysShow", TRUE))
		return TRUE;

	KDialogBase opDialog(0, 0, TRUE, i18n("Additional Export Options"),
	                     KDialogBase::Help|KDialogBase::Default|
						 KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok);


    QVBox *box = opDialog.makeVBoxMainWidget();

	OptionsPage *op;

	if (ext == "tab") {
		op = new OptionsExportAscii(Settings::config, (QFrame *) box);
	} else if (ext == "tex") {
		op = new OptionsExportMusixtex(Settings::config, (QFrame *) box);
	} else {
		kdWarning() << "Weird exportOptionsDialog() call! Wrong extension " << ext << endl;
		return FALSE;
	}

	connect(&opDialog, SIGNAL(defaultClicked()), op, SLOT(defaultBtnClicked()));
	connect(&opDialog, SIGNAL(okClicked()), op, SLOT(applyBtnClicked()));

	bool res = (opDialog.exec() == QDialog::Accepted);
	delete op;
	return res;
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

	if (ext == "kg") {
		sv->tv->arrangeBars(); // GREYFIX !
		success = sv->song()->saveToKg(m_file);
	}
	if (ext == "tab") {
		Settings::config->setGroup("ASCII");
		if (exportOptionsDialog(ext)) {
			ConvertAscii converter(sv->song());
			success = converter.save(m_file);
		} else {
			return FALSE;
		}
	}
#ifdef WITH_TSE3
	if (ext == "mid")
		success = sv->song()->saveToMid(m_file);
	if (ext == "tse3")
		success = sv->song()->saveToTse3(m_file);
#endif
	if (ext == "gtp")
		success = sv->song()->saveToGtp(m_file);
	if (ext == "gp3")
		success = sv->song()->saveToGp3(m_file);
	if (ext == "tex") {
		Settings::config->setGroup("MusiXTeX");
		if (exportOptionsDialog(ext)) {
			switch (Settings::texExportMode()) {
			case 0: success = sv->song()->saveToTexTab(m_file); break;
			case 1: success = sv->song()->saveToTexNotes(m_file); break;
			}
		} else {
			return FALSE;
		}
	}
	if (ext == "xml")
		success = sv->song()->saveToXml(m_file);

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
		"*.gtp|" + i18n("Guitar Pro files") + " (*.gtp)\n"
		"*.gp3|" + i18n("Guitar Pro 3 files") + " (*.gp3)\n"
		"*.xml|" + i18n("MusicXML files") + " (*.xml)\n"
		"*.tex|" + i18n("MusiXTeX") + " (*.tex)\n"
		"*|" + i18n("All files");
	QString file_name = KFileDialog::getSaveFileName(QString::null, filter);

	if (file_name.isEmpty() == false)
		saveAs(file_name);
}

// void KGuitarPart::fileSaveAs()
// {
// 	KFileDialog::getSaveFileName dlg(0,
// 	                "*.kg|" + i18n("KGuitar files") + " (*.kg)\n"
// 	                "*.tab|" + i18n("ASCII files") + " (*.tab)\n"
// 	                "*.mid|" + i18n("MIDI files") + " (*.mid)\n"
// 	                "*.tse3|" + i18n("TSE3MDL files") + " (*.tse3)\n"
// 	                "*.gtp|" + i18n("Guitar Pro files") + " (*.gtp)\n"
// 	                "*.gp3|" + i18n("Guitar Pro 3 files") + " (*.gp3)\n"
// 	                "*.xml|" + i18n("MusicXML files") + " (*.xml)\n"
// 	                "*.tex|" + i18n("MusiXTeX") + " (*.tex)\n"
// 	                "*|" + i18n("All files"), this, 0, TRUE);
// 	dlg.setCaption(i18n("Save as..."));

// 	if (dlg.exec() == QDialog::Accepted) {
// 		QString filter = dlg.currentFilter();
// 		QString fn = dlg.selectedFile();

// 		QFileInfo *fi = new QFileInfo(fn);
// 		if (fi->exists())
// 			if (KMessageBox::warningYesNo(this, i18n("This file exists! "
// 													 "Do you overwrite this file?")) == KMessageBox::No)
// 				return;
// 		if (fi->exists() && !fi->isWritable()) {
// 			KMessageBox::sorry(this, i18n("You have no permission to write this file!"));
// 			return;
// 		}

// 		if (filter == "*") {
// 			filter = fi->extension();
// 			filter = filter.lower();
// 			if (!((filter == "kg") || (filter == "mid") || (filter == "gtp") || (filter == "gp3") ||
// 				  (filter == "tex") || (filter == "tab") || (filter == "xml") || (filter == "tse3"))) {
// 				KMessageBox::sorry(this, i18n("Please select a filter or add an extension."));
// 				return;
// 			}
// 			filter = "*." + filter;
// 		}

// 		if ((filter == "*.kg") || (filter == "*.tab") || (filter == "*.mid") ||
// 			(filter == "*.gtp") || (filter == "*.gp3") || (filter == "*.tex") || (filter == "*.xml") || (filter = "*.tse3")) {
// 			KURL url = KURL(fn);
// 			saveURL(url);
// 		} else {
// 			KMessageBox::sorry(this, i18n("Unknown format: %1").arg(filter));
// 		}
// 	}
// }

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

void KGuitarPart::setupActions()
{
	// SET UP STANDARD ACTIONS
	KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
	KStdAction::save(this, SLOT(save()), actionCollection());

	(void) KStdAction::print(this, SLOT(filePrint()), actionCollection());

	preferencesAct = KStdAction::preferences(this, SLOT(options()),
	                                         actionCollection(), "pref_options");

	sngPropAct = new KAction(i18n("P&roperties..."), 0, sv, SLOT(songProperties()),
	                         actionCollection(), "song_properties");

	// EDIT CUT-N-PASTE ACTIONS
	(void) KStdAction::cut(sv, SLOT(slotCut()), actionCollection());
	(void) KStdAction::copy(sv, SLOT(slotCopy()), actionCollection());
	pasteAct = KStdAction::paste(sv, SLOT(slotPaste()), actionCollection());
	(void) KStdAction::selectAll(sv, SLOT(slotSelectAll()), actionCollection());

	// VIEW ACTIONS
	zoomInAct = new KAction(i18n("Zoom in"), "zoom_in", KAccel::stringToKey("Ctrl+="),
	                        sv->tv, SLOT(zoomIn()), actionCollection(), "zoom_in");
	zoomOutAct = new KAction(i18n("Zoom out"), "zoom_out", KAccel::stringToKey("Ctrl+-"),
	                         sv->tv, SLOT(zoomOut()), actionCollection(), "zoom_out");
	zoomLevelAct = new KAction(i18n("Zoom to..."), 0, sv->tv, SLOT(zoomLevelDialog()),
	                         actionCollection(), "zoom_level");
	viewMelodyEditorAct = new KToggleAction(i18n("Show Melody Editor"), "melodyeditor",
	                                        KAccel::stringToKey("Shift+M"),
	                                        this, SLOT(viewMelodyEditor()),
	                                        actionCollection(), "view_melodyEditor");

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
}

void KGuitarPart::setupAccels()
{
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
}

void KGuitarPart::clipboardDataChanged()
{
	pasteAct->setEnabled(TrackDrag::canDecode(QApplication::clipboard()->data()));
}

void KGuitarPart::updateStatusBar()
{
	QString tmp;
	tmp.setNum(sv->tv->trk()->xb + 1);
	tmp = i18n("Bar: ") + tmp;
	emit setStatusBarText(tmp);
}
