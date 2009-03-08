#include "optionspage.h"

OptionsPage::OptionsPage(KConfig *conf, QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	config = conf;
}
