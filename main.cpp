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
            QMessageBox::warning( NULL, "Fehler",
                                QString("Fehler %1 ").arg(param) );
        }
        catch ( ... ) {
            QMessageBox::warning( NULL, "Fehler",
                                QString("Unbekannter Fehler aufgetreten ") );
        }
        return result;
    }
};

#include "frmmain.h"

int main(int argc, char *argv[])
{
    Pr0fenseApp app(argc, argv);
    FrmMain w;
#ifdef QT_NO_DEBUG
    w.setWindowFlags(Qt::FramelessWindowHint);
#endif

#ifdef Q_OS_ANDROID
    w.showFullScreen();
#else
    w.showMaximized();
#endif

#ifdef QT_DEBUG
        w.showNormal();
#endif

    return app.exec();
}
