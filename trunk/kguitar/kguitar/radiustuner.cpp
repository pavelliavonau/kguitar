#include "radiustuner.h"

#include <qpainter.h>

RadiusTuner::RadiusTuner(QWidget *parent=0, const char *name=0): QWidget(parent,name)
{
    val = new NoteSpinBox(this);
    connect(val,SIGNAL(valueChanged(int)),SLOT(update()));
    connect(val,SIGNAL(valueChanged(int)),SLOT(emitVC()));
}

void RadiusTuner::resizeEvent(QResizeEvent *e)
{
    val->setGeometry(0,height()-20,width(),20);
}

void RadiusTuner::emitVC()
{
    emit valueChanged(val->value());
}

void RadiusTuner::paintEvent(QPaintEvent *p)
{
    QPainter paint(this);

    // GREYFIX: Minimum operation. It's GCC-only. Unportable?
    int maxd = width() <? (height()-20);
    int d = (255-val->value())*maxd/255;

    paint.setBrush(SolidPattern);
    paint.drawEllipse((width()-d)/2,(height()-20-d)/2,d,d);
}
