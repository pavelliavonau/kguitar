#ifndef SETTRACK_H
#define SETTRACK_H

#include <qtabdialog.h>
#include "global.h"

class QLineEdit;
class KIntNumInput;
class QComboBox;
class SetTabFret;

class SetTrack: public QTabDialog {
    Q_OBJECT
public:
    SetTrack(QWidget *parent = 0, const char *name = 0);

    QLineEdit *title;
    KIntNumInput *channel, *bank, *patch;
	QComboBox *mode;
    SetTabFret *fret;
};

#endif
