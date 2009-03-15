#ifndef SETTRACK_H
#define SETTRACK_H

#include <kpagedialog.h>
#include "global.h"
#include "tabtrack.h"

class QLineEdit;
class KIntNumInput;
class QComboBox;
class SetTabFret;
class SetTabDrum;
class SetTabMidi;
class TabTrack;

class SetTrack: public KPageDialog {
    Q_OBJECT
public:
    SetTrack(TabTrack *trk, QWidget *parent = 0);

    QLineEdit *title;
    KIntNumInput *channel, *bank, *patch;
	QComboBox *mode;
	KPageWidgetItem *modeSpecPage;
    QWidget *modespec;
    TabTrack *track;

private:
    void selectFret();
    void selectDrum();

public slots:
    void selectTrackMode(int sel);
};

#endif
