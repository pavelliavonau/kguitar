#include "timesig.h"

#include <klocale.h>

#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <QFormLayout>

SetTimeSig::SetTimeSig(int t1, int t2, QWidget *parent)
	: KDialog(parent)
{
	setCaption(i18n("Time signature"));
	setModal(true);
	setButtons(Ok | Cancel);

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	m_time1 = new QSpinBox(page);
	m_time1->setMinimum(1);
	m_time1->setMaximum(32);
	m_time1->setValue(t1);

	m_time2 = new QComboBox(page);
	m_time2->setInsertPolicy(QComboBox::NoInsertion);
	m_time2->addItem("1");
	m_time2->addItem("2");
	m_time2->addItem("4");
	m_time2->addItem("8");
	m_time2->addItem("16");
	m_time2->addItem("32");

	switch (t2) {
	case 1:	 m_time2->setCurrentIndex(0); break;
	case 2:	 m_time2->setCurrentIndex(1); break;
	case 4:	 m_time2->setCurrentIndex(2); break;
	case 8:	 m_time2->setCurrentIndex(3); break;
	case 16: m_time2->setCurrentIndex(4); break;
	case 32: m_time2->setCurrentIndex(5); break;
	}

	toend = new QCheckBox(i18n("Apply till the &end"),this);

	QFormLayout *l = new QFormLayout(page);
	l->addRow(i18n("&Beats per measure:"), m_time1);
	l->addRow(i18n("Beat &value:"), m_time2);
	l->addRow(toend);
	page->setLayout(l);
}

int SetTimeSig::time1()
{
	return m_time1->value();
}

int SetTimeSig::time2()
{
	return ((QString) m_time2->currentText()).toUInt();
}
