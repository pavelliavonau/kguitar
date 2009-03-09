#ifndef TIMESIG_H
#define TIMESIG_H

#include <kdialog.h>
#include "global.h"

class QSpinBox;
class QComboBox;
class QCheckBox;

class SetTimeSig: public KDialog {
	Q_OBJECT

public:
	SetTimeSig(int t1, int t2, QWidget *parent = 0, const char *name = 0);

	int time1();
	int time2();

	QCheckBox *toend;

protected:
	QSpinBox *m_time1;
	QComboBox *m_time2;
};

#endif
