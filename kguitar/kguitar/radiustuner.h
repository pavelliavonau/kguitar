#ifndef RADIUSTUNER_H
#define RADIUSTUNER_H

#include <qwidget.h>
#include "global.h"

#include <qspinbox.h>

#define RADTUNER_W   45
#define RADTUNER_H   50

class RadiusTuner: public QWidget
{
    Q_OBJECT
public:
    RadiusTuner(QWidget *parent=0, const char *name=0);
    void setValue(uchar x) { val->setValue(x); };
    uchar value() { return val->value(); };
private:
    QSpinBox *val;
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *p);
};

#endif
