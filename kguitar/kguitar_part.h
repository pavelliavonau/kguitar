#ifndef KGUITARPART_H
#define KGUITARPART_H

#include <kparts/readwritepart.h>

#include "songview.h"

#include "global.h"

class KToggleAction;
class QUndoStack;
class K4AboutData;
class ConvertBase;

/**
 * KGuitar KPart - main class of application, uses everything else.
 *
 * Deals mostly with basic KDE GUI and configuration stuff. Real
 * editor is composed in SongView, that, in turn, is composed from
 * several song editing widgets.
 */
class KGuitarPart: public KParts::ReadWritePart {
	Q_OBJECT
public:
	KGuitarPart(QWidget *parentWidget, QObject *parent, const QVariantList &);
	virtual ~KGuitarPart();

	/**
	 * A shell will use this to inform this Part if it should act
	 * read-only
	*/
 	virtual void setReadWrite(bool rw);

	/**
	 * Reimplemented to disable and enable Save action
	 */
	virtual void setModified(bool modified);

	/**
	 * Provide "About..." credits data, required by KPart
	 */
	//static K4AboutData *createAboutData();

public slots:
	void filePrint();
	void viewMelodyEditor();
	void viewScore();
	void updateStatusBar();

protected slots:
	void fileSaveAs();
	void clipboardDataChanged();

	void options();
	void saveOptions();

	/**
	 * Updates possibility of actions, depending on freshly selected
	 * track. For drum track, lots of actions are unavailable.
	 */
	void updateToolbars(QModelIndex, QModelIndex);

private:
	void setupActions();
	void setupAction(QString text, const char *icon,
	                 QKeySequence key, QWidget *target, const char *slot, const char *name);
	void setupAction(QAction *&act, QString text, const char *icon,
	                 QKeySequence key, QWidget *target, const char *slot, const char *name);
	void setupKey(const char *name, QString text, QKeySequence key, QWidget *target, const char *slot);

	void updateMenu();
	void setWinCaption(const QString& caption);
	void readOptions();
	void readMidiNames();

	bool exportOptionsDialog(QString ext);

	/**
	 * Instantiate appropriate converter for file, determining type
	 * by extension.
	 */
	ConvertBase* converterForExtension(QString ext, TabSong *song);

	/**
	 * Main widget - the song view. Accessible from other places with
	 * widget() from KPart.
	 */
	SongView *sv;

	// LVIFIX: do we need a printer variable, e.g. to remember settings ?
	// It would then have to be passed down to SongPrint via SongView.
	// KPrinter *printer;

	QAction *confTBAct,
		*trkNewAct, *trkDeleteAct, *trkBassLineAct, *trkPropAct, *insChordAct,
		*keySigAct, *timeSigAct, *arcAct, *legatoAct, *natHarmAct, *artHarmAct,
		*palmMuteAct, *slideAct, *letRingAct, *saveOptionAct, *confKeyAct,
		*arrTrkAct, *midiPlaySongAct, *midiStopPlayAct, *rhythmerAct;
	KToggleAction *showMainTBAct, *showEditTBAct, *viewMelodyEditorAct, *viewScoreAct;

	QUndoStack* cmdHist;

protected:
	virtual bool openFile();
	virtual bool saveFile();
	QAction *save;
	QAction *paste;
};

#endif
