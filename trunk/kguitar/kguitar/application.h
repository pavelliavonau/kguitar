#ifndef APPLICATION_H
#define APPLICATION_H

#include <ktmainwindow.h>

#define VERSION "0.0.2"

class QMultiLineEdit;
class KToolBar;
class QPopupMenu;

class ChordSelector;

class ApplicationWindow: public KTMainWindow
{
    Q_OBJECT
public:
    ApplicationWindow();
    ~ApplicationWindow();
    
private slots:
    void newDoc();
    void load();
    void load( const char *fileName );
    void save();
    void print();
    void closeDoc();
    void inschord();

    void toggleMenuBar();
    void toggleStatusBar();
    void toggleToolBar();

private:
    QPrinter *printer;
    QMultiLineEdit *e;
    KToolBar *fileTools;
    ChordSelector *cs;
    int mb, tb, sb;
};


#endif
