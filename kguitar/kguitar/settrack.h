#ifndef SETTRACK_H
#define SETTRACK_H

#include <qdialog.h>
#include "global.h"

class QLineEdit;
class KIntNumInput;
class SetTabFret;

class SetTrack: public QDialog
{
    Q_OBJECT
public:
    SetTrack(QWidget *parent=0, const char *name=0);

    QLineEdit *title;
    KIntNumInput *channel,*bank,*patch;
    SetTabFret *fret;
private:
};

#endif
