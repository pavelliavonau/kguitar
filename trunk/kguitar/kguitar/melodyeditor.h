#ifndef MELODYEDITOR_H
#define MELODYEDITOR_H

#include <qwidget.h>
#include "global.h"

class TrackView;
class Fretboard;
class QComboBox;
class QPushButton;

class MelodyEditor: public QWidget {
    Q_OBJECT
public:
    MelodyEditor(TrackView *, QWidget *parent = 0, const char *name = 0);

private:
	Fretboard *fb;
	QComboBox *tonic, *mode;
	QPushButton *options;

	TrackView *tv;
	int fr[MAX_FRETS + 1];
};

#endif
