#ifndef APPLICATION_H
#define APPLICATION_H

#include <ktmainwindow.h>

#include "global.h"
#include "globaloptions.h"

class KToolBar;
class QPopupMenu;
class QLabel;
class ChordSelector;
class TrackView;

class ApplicationWindow: public KTMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
    ~ApplicationWindow();

private slots:
    void newDoc();
    void load();
    void recentLoad(int _id);
    void save();
    void saveAs();
	void importMID();
    void exportMID();
    void exportTAB();
    void exportTEXTAB();
	void exportTEXNOTES();
    void print();
    void closeDoc();
    void appQuit();
    void inschord();
    void songProperties();
    void trackProperties();
    void options();

    void updateStatusBar();

    void setMainTB()  {globalShowMainTB = !(globalShowMainTB); updateTbMenu(); };
    void setEditTB()  {globalShowEditTB = !(globalShowEditTB); updateTbMenu(); };

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
    void addRecentFile(QString fn);
    bool jazzWarning();

    QPrinter *printer;
    TrackView *tv;
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
