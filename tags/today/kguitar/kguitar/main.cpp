#include <kapp.h>
#include "application.h"

int main(int argc, char **argv)
{
    KApplication a(argc, argv, "kguitar");
    ApplicationWindow *mw = new ApplicationWindow();
    a.setMainWidget(mw);
    mw->show();
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    return a.exec();
}
