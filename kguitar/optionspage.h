#ifndef OPTIONSPAGE_H
#define OPTIONSPAGE_H

#include "global.h"
#include <qwidget.h>

class KConfig;

/**
 * Abstract base class for all options pages.
 *
 * Defines basic interfaces to store configuration object and virtual
 * methods to "store settings" and "reset to default".
 */
class OptionsPage: public QWidget {
	Q_OBJECT
public:
	OptionsPage(KConfig *conf, QWidget *parent = 0, const char *name = 0);

public slots:
	/**
	 * Standard button callback on "OK" and "Apply" buttons to store
	 * information from dialog to permanent storage in config object.
	 */
	virtual void applyBtnClicked() = 0;

	/**
	 * Standard button callback on "Default" button to reset all
	 * dialog settings to sane defaults. Doesn't do any thing to
	 * config object, touches only the dialog.
	 */
	virtual void defaultBtnClicked() = 0;

protected:
	/**
	 * Convenience storage to KConfig configuration object that
	 * represents application-wide configuration.
	 */
	KConfig *config;
};

#endif
