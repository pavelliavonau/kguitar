#ifndef APPLICATION_H
#define APPLICATION_H

#include <kmainwindow.h>
#include <kaction.h>

#include "global.h"
#include "globaloptions.h"

#define DESCRIPTION	I18N_NOOP("A stringed instrument tabulature editor")

class KToolBar;
class QPopupMenu;
class QLabel;
class ChordSelector;
class TrackView;

class ApplicationWindow: public KMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
    ~ApplicationWindow();
    TrackView *tv;                    //ALINX: I need it public for file browser
    void addRecentFile(const char *fn);
    void loadFile(KURL _url);

private slots:
    void fileNew();
    void fileOpen();
    void recentLoad(const KURL &_url);
    void openFile(QString fn);
    void openBrowser();
    void fileSave();
    void fileSaveAs();
    void filePrint();
    void fileClose();
    void fileQuit();
    void insertChord();
    void songProperties();
    void trackProperties();
    void options();
    void saveOptions();
    void configToolBars();
    void configKeys();

    void updateStatusBar();

    void setMainTB()  { globalShowMainTB = !globalShowMainTB; updateTbMenu(); };
    void setEditTB()  { globalShowEditTB = !globalShowEditTB; updateTbMenu(); };

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
    void updateTbMenu();
    bool jazzWarning();

    QPrinter *printer;
//    TrackView *tv;      //ALINX: set disabled because I need it public for file browser

    KAction *newAct, *openAct, *saveAct, *saveAsAct, *printAct, *closeAct,
            *quitAct, *preferencesAct, *confTBAct, *browserAct, *sngPropAct,
            *trkPropAct, *insChordAct, *len1Act, *len2Act, *len4Act, *len8Act, 
            *len16Act, *len32Act, *timeSigAct, *arcAct, *legatoAct, *natHarmAct, 
            *artHarmAct, *saveOptionAct, *confKeyAct, *arrTrkAct;
    KToggleAction *showMainTBAct, *showEditTBAct, *usSharpAct, *usFlatAct, 
                  *usMixAct, *euSharpAct, *euFlatAct, *euMixAct, *jazzSharpAct, 
                  *jazzFlatAct, *jazzMixAct;
    KRecentFilesAction *openRecentAct;
	KAccel *mainAccel;

    // Status bar labels
    QLabel *s_bar;

	void readOptions();

};


#endif