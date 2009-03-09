#include "timesig.h"

#include <klocale.h>

#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3GridLayout>

SetTimeSig::SetTimeSig(int t1, int t2, QWidget *parent, const char *name)
	: KDialogBase(parent, name, TRUE, i18n("Time signature"),
	              Ok | Cancel, Ok, TRUE)
{
	QWidget *page = new QWidget(this);
	setMainWidget(page);

	m_time1 = new QSpinBox(1, 32, 1, page);
	m_time1->setValue(t1);

	m_time2 = new QComboBox(TRUE, page);
	m_time2->setInsertionPolicy(QComboBox::NoInsertion);
	m_time2->insertItem("1");
	m_time2->insertItem("2");
	m_time2->insertItem("4");
	m_time2->insertItem("8");
	m_time2->insertItem("16");
	m_time2->insertItem("32");

	switch (t2) {
	case 1:	 m_time2->setCurrentItem(0); break;
	case 2:	 m_time2->setCurrentItem(1); break;
	case 4:	 m_time2->setCurrentItem(2); break;
	case 8:	 m_time2->setCurrentItem(3); break;
	case 16: m_time2->setCurrentItem(4); break;
	case 32: m_time2->setCurrentItem(5); break;
	}

	QLabel *l_time1 = new QLabel(m_time1, i18n("&Beats per measure:"), page);
	QLabel *l_time2 = new QLabel(m_time2, i18n("Beat &value:"), page);

	toend = new QCheckBox(i18n("Apply till the &end"),this);
	
	Q3GridLayout *l = new Q3GridLayout(page, 3, 2, 0, spacingHint());
	l->addWidget(l_time1, 0, 0);
	l->addWidget(m_time1, 0, 1);
	l->addWidget(l_time2, 1, 0);
	l->addWidget(m_time2, 1, 1);
	l->addMultiCellWidget(toend, 2, 2, 0, 1);
	
	l->activate();
}

int SetTimeSig::time1()
{
	return m_time1->value();
}

int SetTimeSig::time2()
{
	return ((QString) m_time2->currentText()).toUInt();	
}
