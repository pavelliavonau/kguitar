#ifndef STRUMMING_H
#define STRUMMING_H

#include <qdialog.h>
//Added by qt3to4:
#include <QLabel>
#include "global.h"

class QComboBox;
class QLabel;

class Strumming: public QDialog {
	Q_OBJECT
public:
	Strumming(int default_scheme, QWidget *parent=0);
	int scheme();

private slots:
	void updateComment(int n);

private:
	QComboBox *pattern;
	QLabel *comment;
};

#endif
