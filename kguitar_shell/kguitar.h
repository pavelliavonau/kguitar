#ifndef KGUITAR_H
#define KGUITAR_H

#include <kparts/mainwindow.h>
#include <QUrl>

class KRecentFilesAction;
class KToggleAction;
class KActionCollection;
namespace KParts
{
class ReadWritePart;
}
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
	void saveURL(const QUrl& url);
	void load(const QUrl &url);

protected slots:
	void fileNew();
	void fileOpen();
	void slotToggleMainTB();
	void slotToggleEditTB();

	void optionsConfigureKeys();
	void applyNewToolbarConfig();

protected:
	/**
	 * This method is called when it is time for the app to save its
	 * properties for session management purposes.
	 */
	void saveProperties(KConfigGroup &);

	/**
	 * This method is called when this app is restored.  The KConfig
	 * object points to the session management config file that was saved
	 * with @ref saveProperties
	 */
	void readProperties(const KConfigGroup &);

private:
	void setupActions();

	KParts::ReadWritePart *kgpart;
	KRecentFilesAction *openRecentAct;
	KToggleAction *showMainTBAct, *showEditTBAct, *showStatusbarAct;
};

#endif
