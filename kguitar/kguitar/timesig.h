#ifndef TIMESIG_H
#define TIMESIG_H

#include <qdialog.h>
#include "global.h"

class QSpinBox;
class QComboBox;

class SetTimeSig: public QDialog
{
    Q_OBJECT
public:
    SetTimeSig(QWidget *parent=0, const char *name=0);
    QSpinBox *time1;
    QComboBox *time2;    
};

#endif
