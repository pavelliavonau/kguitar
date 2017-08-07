#include "strumming.h"

#include "strum.h"

#include <KLocalizedString>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>

Strumming::Strumming(int default_scheme, QWidget *parent)
	: QDialog(parent)
{
	setModal(true);
	QVBoxLayout *l = new QVBoxLayout(this);

	QGridLayout *g = new QGridLayout;
	l->addLayout(g);

	// STRUMMING OPTIONS CONTROLS

	pattern = new QComboBox(this);
	for (int i = 0; lib_strum[i].len[0]; i++)
		pattern->addItem(i18n(lib_strum[i].name.toUtf8()));
	pattern->setCurrentIndex(default_scheme);
	connect(pattern, SIGNAL(highlighted(int)), SLOT(updateComment(int)));

	QLabel *pattern_l = new QLabel(i18n("Strum &pattern:"), this);
	pattern_l->setBuddy(pattern);

	g->addWidget(pattern_l, 0, 0);
	g->addWidget(pattern, 0, 1);

	g->setColumnStretch(1, 1);

	// COMMENT BOX

	comment = new QLabel(this);
	comment->setFrameStyle(QFrame::Box | QFrame::Sunken);
	comment->setAlignment(Qt::AlignJustify);
	comment->setWordWrap(true);
	comment->setMinimumSize(150, 85);
	updateComment(0);
	l->addWidget(comment);

	// DIALOG BUTTONS

	QHBoxLayout *butt = new QHBoxLayout();
	l->addLayout(butt);

	QPushButton *ok = new QPushButton(i18n("OK"), this);
	connect(ok,SIGNAL(clicked()),SLOT(accept()));
	QPushButton *cancel = new QPushButton(i18n("Cancel"), this);
	connect(cancel, SIGNAL(clicked()), SLOT(reject()));

	butt->addWidget(ok);
	butt->addWidget(cancel);
	butt->addStrut(30);

	l->activate();

	setWindowTitle(i18n("Strumming pattern"));
	resize(0, 0);
}

int Strumming::scheme()
{
	return pattern->currentIndex();
}

void Strumming::updateComment(int n)
{
	comment->setText(i18n(lib_strum[n].description.toUtf8()));
}
