#ifndef APPLICATION_H
#define APPLICATION_H

#include <ktmainwindow.h>

#include "global.h"
#include "globaloptions.h"

class KToolBar;
class QPopupMenu;

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
    void exportMID();
    void exportTAB();
    void print();
    void closeDoc();
    void inschord();
    void songProperties();
    void trackProperties();
    void options();

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
    bool jazzWarning();

    QPrinter *printer;
    TrackView *tv;
    KToolBar *fileTools;
    QPopupMenu *nnMenu;
    int ni[9];
};


#endif
