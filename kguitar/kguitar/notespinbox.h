#ifndef NOTESPINBOX_H
#define NOTESPINBOX_H

#include "global.h"
#include <qwidget.h>

#include <qvalidator.h>
#include <qspinbox.h>

/**
 * Validator that accepts only valid MIDI note description strings.
 *
 * These strings basically consist of some sort of note name (one
 * letter or one letter plus alteration symbol "#" or "b") and some
 * sort of digit to designate the octave number. This class accepts,
 * understands and automatically converts to current note naming
 * scheme almost everything, except jazz note naming.
 */
class NoteValidator: public QValidator {
	Q_OBJECT
public:
	NoteValidator(QWidget *parent, const char *name = 0)
		: QValidator(parent, name) {};
	virtual State validate(QString &input, int &pos) const;
};

/**
 * Special QSpinBox that accepts MIDI note names in various notations.
 *
 * This spinbox is able to parse note names entered from keyboard and
 * spinned by using up/down controls. Uses NoteValidator to parse
 * text and map it to MIDI note numbers.
 *
 * @see NoteValidator
 */
class NoteSpinBox: public QSpinBox {
	Q_OBJECT
public:
	NoteSpinBox(QWidget *parent=0, const char *name=0);
	~NoteSpinBox();
private:
	NoteValidator *nv;

	virtual QString mapValueToText(int v);
	virtual int mapTextToValue(bool *ok);
};

#endif
