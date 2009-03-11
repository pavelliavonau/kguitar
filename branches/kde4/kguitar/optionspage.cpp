#include "optionspage.h"

OptionsPage::OptionsPage(KSharedConfigPtr &conf, QWidget *parent, const char *name)
	: QWidget(parent, name)
{
	config = conf.data();
}
