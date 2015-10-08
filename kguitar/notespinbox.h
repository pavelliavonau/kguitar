#ifndef NOTESPINBOX_H
#define NOTESPINBOX_H

#include "global.h"

#include <QSpinBox>

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
	NoteSpinBox(QWidget *parent=0);
private:

	virtual QString mapValueToText(int v);
	virtual int mapTextToValue(bool *ok);

	// QAbstractSpinBox interface
protected:
	QValidator::State validate(QString &input, int &pos) const override;
};

#endif
