#include "optionspage.h"

OptionsPage::OptionsPage(KSharedConfigPtr &conf, QWidget *parent)
	: QWidget(parent)
{
	config = conf.data();
}
