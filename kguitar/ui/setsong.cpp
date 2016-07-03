#include "setsong.h"
#include <KI18n/KLocalizedString>
#include <QDialogButtonBox>
#include <QVBoxLayout>

SetSong::SetSong(QMap<QString, QString> info, int _tempo, bool ro, QWidget *parent):
	QDialog(parent)
{
	setWindowTitle(i18n("Song Properties"));
	setModal(true);

	QWidget *mainWidget = new QWidget(this);
	setupUi(mainWidget);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
	                                                 | QDialogButtonBox::Cancel, this);

	QVBoxLayout* l = new QVBoxLayout();
	l->addWidget(mainWidget);
	l->addWidget(buttonBox);
	this->setLayout(l);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

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
