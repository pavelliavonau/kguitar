#ifndef KGUITARPART_H
#define KGUITARPART_H

#include <kparts/part.h>

// #include <kparts/browserextension.h>

#include <kaction.h>
#include <kcommand.h>

#include "songview.h"

#include "global.h"

class KToolBar;
class QLabel;

// class KGuitarBrowserExtension;

// class KGuitarFactory: public KParts::Factory {
// 	Q_OBJECT
// public:
// 	KGuitarFactory();
// 	virtual ~KGuitarFactory();

// 	virtual KParts::Part *createPartObject(QWidget *parentWidget, const char *widgetName,
// 	                                       QObject *parent, const char *name,
// 	                                       const char *classname, const QStringList &args);

// 	static KInstance *instance();

// private:
// 	static KInstance *s_instance;
// };

class KGuitarPart: public KParts::ReadWritePart {
	Q_OBJECT
public:
	KGuitarPart(QWidget *parentWidget,
	            const char * /*widgetName*/, QObject *parent, const char *name,
	            const QStringList & /*args*/);
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
    static KAboutData *createAboutData();

public slots:
	void filePrint();
	void viewMelodyEditor();
	void updateStatusBar();

protected slots:
    void fileSaveAs();
	void clipboardDataChanged();

	void options();
	void saveOptions();
	void updateToolbars(TabTrack *);

private:
	void setupActions();
	void setupAccels();

	void updateMenu();
	void setWinCaption(const QString& caption);
	void readOptions();
	void readMidiNames();

	bool exportOptionsDialog(QString ext);

	/**
	 * Main widget - the song view. Accessible from other places with
	 * widget() from KPart.
	 */
	SongView *sv;

// 	KGuitarBrowserExtension *m_extension;

	// LVIFIX: do we need a printer variable, e.g. to remember settings ?
	// It would then have to be passed down to SongPrint via SongView.
	// KPrinter *printer;

	KAction *preferencesAct, *confTBAct, *browserAct, *sngPropAct,
		*trkNewAct, *trkDeleteAct, *trkBassLineAct, *trkPropAct, *insChordAct,
		*len1Act, *len2Act, *len4Act, *len8Act, *len16Act, *len32Act,
		*keySigAct, *timeSigAct, *arcAct, *legatoAct, *natHarmAct, *artHarmAct,
		*palmMuteAct, *slideAct, *letRingAct, *saveOptionAct, *confKeyAct,
		*arrTrkAct, *midiPlaySongAct, *midiStopPlayAct, *rhythmerAct,
		*zoomInAct, *zoomOutAct, *zoomLevelAct, *pasteAct;
    KToggleAction *showMainTBAct, *showEditTBAct, *viewMelodyEditorAct;

	KAccel *mainAccel;

	KCommandHistory* cmdHist;

protected:
    // reimplemented from ReadWritePart
    virtual bool openFile();
    virtual bool saveFile();
};

//--------------------------------------------------------------------------

// class KGuitarBrowserExtension : public KParts::BrowserExtension {
//     Q_OBJECT
//     friend class KGuitarPart; // emits our signals

// public:
//     KGuitarBrowserExtension(KGuitarPart *parent);
//     virtual ~KGuitarBrowserExtension() {}

// public slots:
//     // Automatically detected by konqueror
//     void print();
// };

#endif
