#ifndef SETTRACK_H
#define SETTRACK_H

#include <q3tabdialog.h>
#include "global.h"
#include "tabtrack.h"

class QLineEdit;
class KIntNumInput;
class QComboBox;
class SetTabFret;
class SetTabDrum;
class SetTabMidi;
class TabTrack;

class SetTrack: public Q3TabDialog {
    Q_OBJECT
public:
    SetTrack(TabTrack *trk, QWidget *parent = 0, const char *name = 0);

    QLineEdit *title;
    KIntNumInput *channel, *bank, *patch;
	QComboBox *mode;
    QWidget *modespec;
    TabTrack *track;

private:
    void selectFret();
    void selectDrum();

public slots:
    void selectTrackMode(int sel);
};

#endif
