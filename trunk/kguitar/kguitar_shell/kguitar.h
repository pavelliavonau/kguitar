#ifndef KGUITAR_H
#define KGUITAR_H

#include <kparts/mainwindow.h>

class KRecentFilesAction;
class KToggleAction;
class KAction;
class KActionCollection;
class KCommandHistory;

/**
 * This is the KGuitar application "Shell". It has a menubar, toolbar,
 * and statusbar but relies on the "Part" to do all the real work.
 *
 * @short KGuitar Application Shell
 */
class KGuitar: public KParts::MainWindow {
    Q_OBJECT
public:
    KGuitar();
    virtual ~KGuitar();

public slots:
    void saveURL(const KURL& url);
	void load(const KURL& url);

protected slots:
	void fileNew();
    void fileOpen();
    void slotToggleMainTB();
    void slotToggleEditTB();

    void optionsShowStatusbar();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();

protected:
    /**
     * This method is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfig *);

    /**
     * This method is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(KConfig *);

private:
	void setupActions();

	KParts::ReadWritePart *kgpart;
    KRecentFilesAction *openRecentAct;
    KToggleAction *showMainTBAct, *showEditTBAct, *showStatusbarAct;
	KCommandHistory *cmdHistory;
};

#endif
