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

    void setMainTB()  {global_showMainTB = !(global_showMainTB); updateTbMenu(); };
    void setEditTB()  {global_showEditTB = !(global_showEditTB); updateTbMenu(); };

    void setUSsharp() { global_notenames = 0; updateMenu(); };
    void setUSflats() { global_notenames = 1; updateMenu(); };
    void setUSmixed() { global_notenames = 2; updateMenu(); };

    void setEUsharp() { global_notenames = 3; updateMenu(); };
    void setEUflats() { global_notenames = 4; updateMenu(); };
    void setEUmixed() { global_notenames = 5; updateMenu(); };

    void setJZsharp() { if (jazzWarning()) { global_notenames = 6; updateMenu(); } };
    void setJZflats() { if (jazzWarning()) { global_notenames = 7; updateMenu(); } };
    void setJZmixed() { if (jazzWarning()) { global_notenames = 8; updateMenu(); } };

private:
    void updateMenu();
    void updateTbMenu();
    bool jazzWarning();

    QPrinter *printer;
    TrackView *tv;
    KToolBar *fileTools;
    QPopupMenu *nnMenu, *tbMenu;
    int ni[9], tb[2];

    // Status bar labels
    QLabel *s_bar;

	void readOptions();
	void saveOptions();
};


#endif
