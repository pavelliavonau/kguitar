#ifndef NOTESPINBOX_H
#define NOTESPINBOX_H

#include <qwidget.h>
#include "global.h"

#include <qvalidator.h>
#include <qspinbox.h>

class NoteValidator: public QValidator
{
    Q_OBJECT
public:
    NoteValidator(QWidget *parent, const char *name = 0):
		QValidator(parent, name) {};
    virtual State validate(QString &input, int &pos) const;
};

class NoteSpinBox: public QSpinBox
{
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
