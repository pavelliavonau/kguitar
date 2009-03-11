#include "radiustuner.h"

#include <qpainter.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>

RadiusTuner::RadiusTuner(QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	val = new NoteSpinBox(this);
	connect(val, SIGNAL(valueChanged(int)), SLOT(update()));
	connect(val, SIGNAL(valueChanged(int)), SLOT(emitValueChanged()));
}

void RadiusTuner::resizeEvent(QResizeEvent *)
{
	val->setGeometry(0, height() - 20, width(), 20);
}

void RadiusTuner::emitValueChanged()
{
	emit valueChanged(val->value());
}

void RadiusTuner::paintEvent(QPaintEvent *)
{
	QPainter paint(this);

	int maxd = QMIN(width(), height()-20);
	int v = val->value()-12;
	if (v < 0)  v = 0;
	if (v > 103)  v = 103;
	int d = (103 - v) * maxd / 103;

	paint.setBrush(Qt::SolidPattern);
	paint.drawEllipse((width() - d) / 2, (height() - 20 - d) / 2, d, d);
}
