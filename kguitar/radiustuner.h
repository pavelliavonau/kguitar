#ifndef RADIUSTUNER_H
#define RADIUSTUNER_H

#include <qwidget.h>
#include "global.h"

#include "notespinbox.h"

#define RADTUNER_W   47
#define RADTUNER_H   50

/**
 * Spinbox with a visual diagram of string thickness.
 *
 * Used widely in tunings to visually demonstrate thickness of tuned
 * string. When used in large quantinites, makes a good demonstration
 * of how the strings are positioned on the instrument, where's the
 * thickest (=bassiest) string, where's the thinnest (=trebliest)
 * string. Some instruments are tuned in non ascending/descending
 * order, but provide alternating string pitches.
 *
 * Interface is pretty simple and is almost transparent wrapper to
 * underlying NoteSpinBox.
 *
 * @see NoteSpinBox
 */
class RadiusTuner: public QWidget {
	Q_OBJECT
public:
	RadiusTuner(QWidget *parent=0, const char *name=0);
	void setValue(uchar x) { val->setValue(x); };
	uchar value() { return val->value(); };

signals:
	void valueChanged(int);

private slots:
	void emitValueChanged();

private:
	NoteSpinBox *val;
	virtual void resizeEvent(QResizeEvent *e);
	virtual void paintEvent(QPaintEvent *p);
};

#endif
