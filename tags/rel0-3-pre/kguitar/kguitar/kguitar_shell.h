#ifndef __KGUITARSHELL_H__
#define __KGUITARSHELL_H__

#include <kparts/mainwindow.h>

class KRecentFilesAction;
class ScrollBox;
class KGuitarPart;
class KToggleAction;
class KAction;
class KActionCollection;
class FileBrowser;

class KGuitarShell : public KParts::MainWindow
{
    Q_OBJECT
public:
    KGuitarShell();
    virtual ~KGuitarShell();

public slots:
    bool openURL(const KURL& url);
    void saveURL(const KURL& url);
 
protected slots:
    void slotFileOpen();
    void slotFileSave();
    void slotFileSaveAs();
    void slotToggleMainTB();
    void slotToggleEditTB();
    void slotShowStatusBar();
    void slotSetCaption(const QString& caption);
    void slotQuit();
    void slotConfigTB();
    void updateStatusBar();
    void openBrowser();
    void openBrowserURL(const KURL& url);

protected:
    void readSettings();
    void writeSettings();

private:
    KGuitarPart *m_kgpart;
    FileBrowser *fb;
	KAction *browserAct;
    KRecentFilesAction *openRecentAct;
    KToggleAction *showMainTBAct, *showEditTBAct, *showStatusbarAct;
};

#endif
