#include "frmmain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FrmMain w;
    w.showFullScreen();
    #ifdef QT_DEBUG
        w.showNormal();
    #endif

    return a.exec();
}
