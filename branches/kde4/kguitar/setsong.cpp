#include "setsong.h"

#include <klocale.h>

#include <qlineedit.h>
#include <q3textedit.h>
#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3GridLayout>

SetSong::SetSong(QMap<QString, QString> info, int tempo_, bool ro, QWidget *parent, const char *name)
	: KDialogBase(parent, name, TRUE, i18n("Song properties"),
	              Ok | Cancel, Ok, TRUE)
{
	QWidget *page1 = new QWidget(this);
	setMainWidget(page1);
	
	title = new QLineEdit(page1);
	author = new QLineEdit(page1);
	transcriber = new QLineEdit(page1);
	comments = new Q3TextEdit(page1);
	m_tempo = new KIntNumInput(page1);

	QLabel *title_l = new QLabel(title, i18n("&Title:"), page1);
	QLabel *author_l = new QLabel(author, i18n("&Artist:"), page1);
	QLabel *transcriber_l = new QLabel(transcriber, i18n("&Transcriber:"), page1);
	QLabel *comments_l = new QLabel(comments, i18n("&Comments:"), page1);
	QLabel *tempo_l = new QLabel(m_tempo, i18n("T&empo:"), page1);

	Q3GridLayout *g = new Q3GridLayout(page1, 6, 2, 0, spacingHint());
	g->addWidget(title_l, 0, 0);
	g->addWidget(title, 0, 1);
	g->addWidget(author_l, 1, 0);
	g->addWidget(author, 1, 1);
	g->addWidget(transcriber_l, 2, 0);
	g->addWidget(transcriber, 2, 1);
	g->addWidget(comments_l, 3, 0);
	g->addWidget(comments, 3, 1);
	g->addWidget(tempo_l, 4, 0);
	g->addWidget(m_tempo, 4, 1);

	g->activate();

	title->setText(info["TITLE"]);
 	title->setReadOnly(ro);
	author->setText(info["ARTIST"]);
 	author->setReadOnly(ro);
	transcriber->setText(info["TRANSCRIBER"]);
 	transcriber->setReadOnly(ro);
	comments->setText(info["COMMENTS"]);
 	comments->setReadOnly(ro);
	m_tempo->setValue(tempo_);
	//	tempo->setReadOnly(ro); // GREYFIX - what the heck about KIntNumInput???

	m_info = info;
}

QMap<QString, QString> SetSong::info()
{
	m_info["TITLE"] = title->text();
	m_info["ARTIST"] = author->text();
	m_info["TRANSCRIBER"] = transcriber->text();
	m_info["COMMENTS"] = comments->text();
	return m_info;
}
