#ifndef OPTIONSPAGE_H
#define OPTIONSPAGE_H

#include "global.h"
#include <qwidget.h>

class OptionsPage: public QWidget {
    Q_OBJECT
public:
    OptionsPage(QWidget *parent = 0, const char *name = 0);
    virtual void applyBtnClicked() = 0;
    virtual void defaultBtnClicked() = 0;
};

#endif
