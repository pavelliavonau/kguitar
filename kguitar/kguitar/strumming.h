#ifndef STRUMMING_H
#define STRUMMING_H

#include <qdialog.h>
#include "global.h"

#include <qcombobox.h>

class Strumming: public QDialog
{
    Q_OBJECT
public:
    Strumming(int default_scheme, QWidget *parent=0, const char *name=0);
	int scheme();

private:
	QComboBox *pattern;
};

#endif
