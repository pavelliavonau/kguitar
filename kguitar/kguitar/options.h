#ifndef OPTIONS_H
#define OPTIONS_H

#include <qtabdialog.h>
#include "global.h"
#include "qcheckbox.h"

class QButtonGroup;
class QRadioButton;

class Options: public QTabDialog
{
    Q_OBJECT
public:
    Options(QWidget *parent=0, const char *name=0);

    QButtonGroup *maj7gr,*flatgr, *texlygr, *texsizegr;
    QRadioButton *maj7[3],*flat[2], *tabsize[4];
    QCheckBox *showbarnumb, *showstr, *showpagenumb;

private:
};

#endif
