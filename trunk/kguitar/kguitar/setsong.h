#ifndef SETSONG_H
#define SETSONG_H

#include <kdialogbase.h>
#include "global.h"

#include <qmap.h>
#include <qstring.h>
#include <knuminput.h>

class QLineEdit;
class QMultiLineEdit;
class KIntNumInput;

/**
 * Song properties dialog.
 *
 * Provides song properties dialog that maintains all metainformation
 * about song and, for now, holds first tempo value.
 */
class SetSong: public KDialogBase {
	Q_OBJECT
public:
	SetSong(QMap<QString, QString> info, int tempo_, bool ro, QWidget *parent=0, const char *name=0);
	QMap<QString, QString> info();
	int tempo() { return m_tempo->value(); }

private:
	QLineEdit *title, *author, *transcriber;
	QMultiLineEdit *comments;
	KIntNumInput *m_tempo;

	QMap<QString, QString> m_info;
};

#endif
