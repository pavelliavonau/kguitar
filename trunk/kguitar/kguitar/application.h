#ifndef APPLICATION_H
#define APPLICATION_H

#include <kio/job.h>
#include <kparts/browserextension.h>
#include <kparts/factory.h>

#include <kaction.h>

#include "trackview.h"
#include "tabsong.h"

#include "global.h"
#include "globaloptions.h"


class KToolBar;
class QPopupMenu;
class QLabel;
class ChordSelector;

class KGuitarBrowserExtension;

class KGuitarFactory : public KParts::Factory
{
    Q_OBJECT
public:
    KGuitarFactory();
    virtual ~KGuitarFactory();

    virtual KParts::Part *createPart( QWidget *parentWidget, const char *widgetName, 
            QObject *parent, const char *name, const char *classname, const QStringList &args );

    static KInstance *instance();

private:
    static KInstance *s_instance;
};


class KGuitarPart: public KParts::ReadWritePart
{
    Q_OBJECT
public:
    KGuitarPart(bool bBrowserView, QWidget *parentWidget, const char *widgetName, 
                QObject *parent, const char *name);
    virtual ~KGuitarPart();
    TrackView *tv;

public slots:
    void filePrint();

private slots:
    void fileNew();
    bool slotOpenFile(QString fn);
    bool fileSave(QString fn);
    void insertChord();
    void songProperties();
    void trackProperties();
    void options();
    void saveOptions();
    void slotConfigToolBars();
    void configKeys();

    void setUSsharp() { globalNoteNames = 0; updateMenu(); };
    void setUSflats() { globalNoteNames = 1; updateMenu(); };
    void setUSmixed() { globalNoteNames = 2; updateMenu(); };

    void setEUsharp() { globalNoteNames = 3; updateMenu(); };
    void setEUflats() { globalNoteNames = 4; updateMenu(); };
    void setEUmixed() { globalNoteNames = 5; updateMenu(); };

    void setJZsharp() { if (jazzWarning()) { globalNoteNames = 6; } updateMenu(); };
    void setJZflats() { if (jazzWarning()) { globalNoteNames = 7; } updateMenu(); };
    void setJZmixed() { if (jazzWarning()) { globalNoteNames = 8; } updateMenu(); };

private:
    void updateMenu();
    void setWinCaption(const QString& caption);
    bool jazzWarning();

    KGuitarBrowserExtension *m_extension;

    QPrinter *printer;

    KAction *newAct, *preferencesAct, *confTBAct, *browserAct, *sngPropAct,
            *trkPropAct, *insChordAct, *len1Act, *len2Act, *len4Act, *len8Act, 
            *len16Act, *len32Act, *timeSigAct, *arcAct, *legatoAct, *natHarmAct, 
            *artHarmAct, *saveOptionAct, *confKeyAct, *arrTrkAct;
    KToggleAction *showMainTBAct, *showEditTBAct, *usSharpAct, *usFlatAct, 
                  *usMixAct, *euSharpAct, *euFlatAct, *euMixAct, *jazzSharpAct, 
                  *jazzFlatAct, *jazzMixAct;
	KAccel *mainAccel;

    // Status bar labels
    QLabel *s_bar;

    //parentWidget
    QWidget *p; 

    //BrowserView
    bool isBrowserView;

	void readOptions();

protected:
    // reimplemented from ReadWritePart
    virtual bool openFile();    
    virtual bool saveFile();

signals:
    void configToolBars();

};

//--------------------------------------------------------------------------

class KGuitarBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT
    friend class KGuitarPart; // emits our signals
public:
    KGuitarBrowserExtension(KGuitarPart *parent);
    virtual ~KGuitarBrowserExtension() {}

public slots:
    // Automatically detected by konqueror
    void print();

};

#endif
