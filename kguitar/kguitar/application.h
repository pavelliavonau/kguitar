#ifndef APPLICATION_H
#define APPLICATION_H

#include <kmainwindow.h>

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

private slots:
    void fileNew();
    void fileOpen();
    void recentLoad(int _id);
    void openBrowser();
    void fileSave();
    void fileSaveAs();
	void fileImportMid();
    void fileExportMid();
    void fileExportTab();
    void fileExportTexTab();
	void fileExportTexNotes();
    void filePrint();
    void fileClose();
    void fileQuit();
    void insertChord();
    void songProperties();
    void trackProperties();
    void options();

    void updateStatusBar();

    void setMainTB()  { globalShowMainTB = !globalShowMainTB; updateTbMenu(); };
    void setEditTB()  { globalShowEditTB = !globalShowEditTB; updateTbMenu(); };

    void setUSsharp() { globalNoteNames = 0; updateMenu(); };
    void setUSflats() { globalNoteNames = 1; updateMenu(); };
    void setUSmixed() { globalNoteNames = 2; updateMenu(); };

    void setEUsharp() { globalNoteNames = 3; updateMenu(); };
    void setEUflats() { globalNoteNames = 4; updateMenu(); };
    void setEUmixed() { globalNoteNames = 5; updateMenu(); };

    void setJZsharp() { if (jazzWarning()) { globalNoteNames = 6; updateMenu(); } };
    void setJZflats() { if (jazzWarning()) { globalNoteNames = 7; updateMenu(); } };
    void setJZmixed() { if (jazzWarning()) { globalNoteNames = 8; updateMenu(); } };

private:
    void updateMenu();
    void updateTbMenu();
    bool jazzWarning();

    QPrinter *printer;
//    TrackView *tv;      //ALINX: set disabled because I need it public for file browser
    KToolBar *fileTools;
    QPopupMenu *nnMenu, *tbMenu, *recMenu;
    int ni[9], tb[2];
	QStrList recentFiles;

    // Status bar labels
    QLabel *s_bar;

	void readOptions();
	void saveOptions();

protected:
    virtual void closeEvent(QCloseEvent*);
};


#endif
