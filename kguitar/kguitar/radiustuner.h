#ifndef RADIUSTUNER_H
#define RADIUSTUNER_H

#include <qwidget.h>
#include "global.h"

#include "notespinbox.h"

#define RADTUNER_W   47
#define RADTUNER_H   50

class RadiusTuner: public QWidget
{
    Q_OBJECT
public:
    RadiusTuner(QWidget *parent=0, const char *name=0);
    void setValue(uchar x) { val->setValue(x); };
    uchar value() { return val->value(); };

signals:
    void valueChanged(int);

private slots:
    void emitVC();

private:
    NoteSpinBox *val;
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *p);
};

#endif
