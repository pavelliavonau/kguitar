#ifndef APPLICATION_H
#define APPLICATION_H

#include <ktmainwindow.h>

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
    void print();
    void closeDoc();
    void inschord();
    void songProperties();
    void trackProperties();

    void toggleMenuBar();
    void toggleStatusBar();
    void toggleToolBar();

private:
    QPrinter *printer;
    TrackView *tv;
    KToolBar *fileTools;
    ChordSelector *cs;
    int mb, tb, sb;
};


#endif
