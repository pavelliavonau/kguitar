#ifndef SETSONG_H
#define SETSONG_H

#include <qdialog.h>
#include "global.h"

class KLined;
class QMultiLineEdit;

class SetSong: public QDialog
{
    Q_OBJECT
public:
    SetSong(QWidget *parent=0, const char *name=0);

    KLined *title,*author,*transcriber;
    QMultiLineEdit *comments;
private:
};

#endif
