#include "setsong.h"

#include <klocale.h>
#include <knuminput.h>

#include <qlineedit.h>
#include <qmultilinedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

SetSong::SetSong(QWidget *parent, const char *name): QDialog(parent, name, TRUE)
{
	title = new QLineEdit(this);
	author = new QLineEdit(this);
	transcriber = new QLineEdit(this);
	comments = new QMultiLineEdit(this);
	tempo = new KIntNumInput(this);

	QLabel *title_l = new QLabel(title, i18n("&Title:"), this);
	QLabel *author_l = new QLabel(author, i18n("&Author:"), this);
	QLabel *transcriber_l = new QLabel(transcriber, i18n("&Transcriber:"), this);
	QLabel *comments_l = new QLabel(comments, i18n("&Comments:"), this);
	QLabel *tempo_l = new QLabel(tempo, i18n("T&empo:"), this);

	QPushButton *ok = new QPushButton(i18n("OK"), this);
	connect(ok, SIGNAL(clicked()), SLOT(accept()));
	QPushButton *cancel = new QPushButton(i18n("Cancel"), this);
	connect(cancel, SIGNAL(clicked()), SLOT(reject()));

	QGridLayout *g = new QGridLayout(this, 6, 2, 10);
	g->addWidget(title_l, 0, 0);
	g->addWidget(title, 0, 1);
	g->addWidget(author_l, 1, 0);
	g->addWidget(author, 1, 1);
	g->addWidget(transcriber_l, 2, 0);
	g->addWidget(transcriber, 2, 1);
	g->addWidget(comments_l, 3, 0);
	g->addWidget(comments, 3, 1);
	g->addWidget(tempo_l, 4, 0);
	g->addWidget(tempo, 4, 1);

	QHBoxLayout *butt = new QHBoxLayout();
	g->addLayout(butt, 5, 1);
	butt->addWidget(ok);
	butt->addWidget(cancel);

	g->setRowStretch(3, 1);
	g->setColStretch(1, 1);
	g->addColSpacing(0, 90);
	for (int i = 0; i < 4; i++)
		g->addRowSpacing(i, 20);
	g->addRowSpacing(4, 30);

	g->activate();

	setMinimumSize(250, 170);
	resize(400, 300);
	setCaption(i18n("Song properties"));
}
