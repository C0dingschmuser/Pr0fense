#include <QApplication>
#include <QMessageBox>

class Pr0fenseApp : public QApplication {
public:
    Pr0fenseApp( int &argc, char *argv[] ) :
        QApplication( argc, argv)
    {};
    virtual ~Pr0fenseApp() {};
    bool notify( QObject *receiver, QEvent *e) {
        bool result = true;
        try {
            if( receiver && e )
                result = QApplication::notify( receiver, e );
            }
        catch ( int param ) {
            QMessageBox::warning( NULL, "exception occured",
                                QString("exception %1 occured").arg(param) );
        }
        catch ( ... ) {
            QMessageBox::warning( NULL, "unknown exception",
                                QString("unknown exception occured ") );
        }
        return result;
    }
};

#include "frmmain.h"

int main(int argc, char *argv[])
{
    Pr0fenseApp app(argc, argv);
    FrmMain w;
    w.showFullScreen();
    #ifdef QT_DEBUG
        w.showNormal();
    #endif

    return app.exec();
}
