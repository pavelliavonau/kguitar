#include "setsong.h"

#include <kapp.h>
#include <qlineedit.h>
#include <qmultilinedit.h>
#include <qlabel.h>
#include <qpushbutton.h>

SetSong::SetSong(QWidget *parent=0, const char *name=0): QDialog(parent,name,TRUE)
{
    title = new QLineEdit(this);
    title->setGeometry(100,10,290,20);
    QLabel *title_l = new QLabel(title,i18n("&Title:"),this);
    title_l->setGeometry(10,10,90,20);

    author = new QLineEdit(this);
    author->setGeometry(100,40,290,20);
    QLabel *author_l = new QLabel(author,i18n("&Author:"),this);
    author_l->setGeometry(10,40,90,20);

    transcriber = new QLineEdit(this);
    transcriber->setGeometry(100,70,290,20);
    QLabel *transcriber_l = new QLabel(transcriber,i18n("&Transcriber:"),this);
    transcriber_l->setGeometry(10,70,90,20);

    comments = new QMultiLineEdit(this);
    comments->setGeometry(100,100,290,150);
    QLabel *comments_l = new QLabel(comments,i18n("&Comments:"),this);
    comments_l->setGeometry(10,100,90,20);

    QPushButton *ok = new QPushButton(i18n("OK"),this);
    ok->setGeometry(10,260,80,30);
    connect(ok,SIGNAL(clicked()),SLOT(accept()));
    QPushButton *cancel = new QPushButton(i18n("Cancel"),this);
    cancel->setGeometry(100,260,80,30);
    connect(cancel,SIGNAL(clicked()),SLOT(reject()));

    setCaption(i18n("Song properties"));
    setFixedSize(400,300);
}
