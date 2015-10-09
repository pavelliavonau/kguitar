#include "setsong.h"

SetSong::SetSong(QMap<QString, QString> info, int _tempo, bool ro, QWidget *parent):
	KDialog(parent)
{
	QWidget *mainWidget = new QWidget(this);
	setupUi(mainWidget);
	setMainWidget(mainWidget);

	setCaption(i18n("Song Properties"));
	setModal(true);
	setButtons(Ok | Cancel);

	title->setText(info["TITLE"]);
	title->setReadOnly(ro);
	artist->setText(info["ARTIST"]);
 	artist->setReadOnly(ro);
	transcriber->setText(info["TRANSCRIBER"]);
 	transcriber->setReadOnly(ro);
	comments->setPlainText(info["COMMENTS"]);
 	comments->setReadOnly(ro);
	tempoEdit->setValue(_tempo);

	m_info = info;
}

QMap<QString, QString> SetSong::info()
{
	m_info["TITLE"] = title->text();
	m_info["ARTIST"] = artist->text();
	m_info["TRANSCRIBER"] = transcriber->text();
	m_info["COMMENTS"] = comments->toPlainText();
	return m_info;
}
