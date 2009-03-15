#include "setsong.h"

#include <klocale.h>

#include <QLineEdit>
#include <QTextEdit>
#include <QFormLayout>

SetSong::SetSong(QMap<QString, QString> info, int tempo_, bool ro, QWidget *parent)
	: KDialog(parent)
{
	setCaption(i18n("Song properties"));
	setModal(true);
	setButtons(Ok | Cancel);

	QWidget *page1 = new QWidget(this);
	setMainWidget(page1);

	title = new QLineEdit(page1);
	author = new QLineEdit(page1);
	transcriber = new QLineEdit(page1);
	comments = new QTextEdit(page1);
	m_tempo = new KIntNumInput(page1);

	QFormLayout *l = new QFormLayout(page1);
	l->addRow(i18n("&Title:"), title);
	l->addRow(i18n("&Artist:"), author);
	l->addRow(i18n("&Transcriber:"), transcriber);
	l->addRow(i18n("&Comments:"), comments);
	l->addRow(i18n("T&empo:"), m_tempo);

	page1->setLayout(l);

	title->setText(info["TITLE"]);
 	title->setReadOnly(ro);
	author->setText(info["ARTIST"]);
 	author->setReadOnly(ro);
	transcriber->setText(info["TRANSCRIBER"]);
 	transcriber->setReadOnly(ro);
	comments->setPlainText(info["COMMENTS"]);
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
	m_info["COMMENTS"] = comments->toPlainText();
	return m_info;
}
