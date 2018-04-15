#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QOpenGLWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QColor>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QPixmap>
#include <QTouchEvent>
#include <QtConcurrent/QtConcurrent>
#include <QtMath>
#include <QInputDialog>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QtPurchasing>
#include <QTcpSocket>
#include <QDate>
#include "tower.h"
#include "objectpool.h"
#include "tile.h"
#include "engine.h"
#include "level.h"
#ifdef Q_OS_ANDROID
#include "lockhelper.h"
#endif

namespace Ui {
class FrmMain;
}

class FrmMain : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit FrmMain(QOpenGLWidget *parent = 0);
    ~FrmMain();

private:
    Ui::FrmMain *ui;
    QInAppStore *store;
    QMutex mutex;
    QDate latestTestingDate = QDate(2018,4,17);
    bool DEBUG=false;
    QElapsedTimer timerD;
    QElapsedTimer timerM;
    QElapsedTimer timerP;
    QElapsedTimer timerPF;
    QElapsedTimer timerWS;
    QTimer *t_draw;
    QTimer *t_main;
    QTimer *t_projectile;
    QTimer *t_projectile_full;
    QTimer *t_wave;
    QTimer *t_waveSpeed;
    QTimer *t_animation;
    QTimer *t_shake;
    QTimer *t_coll;
    QTimer *t_menuAn;
    QThread *workerThread;
#ifdef Q_OS_ANDROID
    KeepAwakeHelper helper; //verhindert, dass app in standby geht
#endif
    double scaleX;
    double scaleY;
    double scaleFHDX;
    double scaleFHDY;
    ObjectPool *pool;
    std::vector <Tower*> towers;
    std::vector <Enemy*> enemys;
    std::vector <Projectile*> projectiles;
    std::vector <Tile*> tiles;
    std::vector <int> path;
    std::vector <QString> spruch;
    std::vector <QPixmap> pathTextures;
    std::vector <QPixmap> lvlPreviews;
    std::vector <QRect> mainLvlRects;
    std::vector <Level> levels;
    std::vector <double> frameTimes;
    QPixmap mapTile = QPixmap(":/data/images/maptile.png");
    QPixmap turmtile = QPixmap(":/data/images/turmtile.png");
    QPixmap emptytile = QPixmap(":/data/images/empty.png");
    QPixmap enemybasePx = QPixmap(":/data/images/enemybase.png");
    QPixmap ownbasePx = QPixmap(":/data/images/ownbase.png");
    QPixmap towerGroundPx = QPixmap(":/data/images/towers/base.png");
    QPixmap towerBasePx = QPixmap(":/data/images/towers/base2.png");
    QPixmap towerFavBasePx = QPixmap(":/data/images/towers/baseFav.png");
    QPixmap towerRepostBasePx = QPixmap(":/data/images/towers/repost.png");
    QPixmap repostMark = QPixmap(":/data/images/towers/repost_mark.png");
    QPixmap herzPx = QPixmap(":/data/images/towers/herz.png");
    QPixmap minus = QPixmap(":/data/images/towers/minus.png");
    QPixmap blus = QPixmap(":/data/images/towers/blus.png");
    QPixmap blus_blurred = QPixmap(":/data/images/towers/blus_blurred.png");
    QPixmap buyPx = QPixmap(":/data/images/ui/kaufen.png");
    QPixmap sellPx = QPixmap(":/data/images/ui/verkaufen.png");
    QPixmap auswahlPx = QPixmap(":/data/images/ui/auswahl.png");
    QPixmap auswahlTile = QPixmap(":/data/images/auswahltile.png");
    QPixmap btnSpeed = QPixmap(":/data/images/ui/btnSpeed.png");
    QPixmap btnDmg = QPixmap(":/data/images/ui/btnDmg.png");
    QPixmap btnFirerate = QPixmap(":/data/images/ui/btnFirerate.png");
    QPixmap btnRange = QPixmap(":/data/images/ui/btnRange.png");
    QPixmap btnSell = QPixmap(":/data/images/ui/btnSell.png");
    QPixmap btnBack = QPixmap(":/data/images/ui/zurueck.png");
    QPixmap btnContinue = QPixmap(":/data/images/ui/weiterspielen.png");
    QPixmap barPx = QPixmap(":/data/images/ui/powerBar.png");
    QPixmap menuPx = QPixmap(":/data/images/menu.png");
    QPixmap titlePx = QPixmap(":/data/images/ui/title.png");
    QPixmap startPx = QPixmap(":/data/images/ui/start.png");
    QPixmap editPx = QPixmap(":/data/images/ui/editor.png");
    QPixmap enemyPx = QPixmap(":/data/images/enemy.png");
    QPixmap pr0coinPx = QPixmap(":/data/images/towers/coin.png");
    QPixmap baseHerzPx = QPixmap(":/data/images/ui/herz.png");
    //Playmenu
    QPixmap levelsPx = QPixmap(":/data/images/levels.png");
    QPixmap ownMapsPx = QPixmap(":/data/images/eigene.png");
    QPixmap communityPx = QPixmap(":/data/images/community.png");
    //Mapeditor
    QPixmap delPx = QPixmap(":/data/images/ui/del.png");
    QPixmap lockPx = QPixmap(":/data/images/schloss.png");
    QPixmap mPlayPx = QPixmap(":/data/images/ui/mplay.png");
    QPixmap mStopPx = QPixmap(":/data/images/ui/mstop.png");
    QPixmap mSavePx = QPixmap(":/data/images/ui/msave.png");
    QPixmap mapPx;
    //<Farben>
    QColor grau = QColor(22,22,24);
    QColor neuschwuchtel = QColor(225,8,233);
    QColor schwuchtel = QColor(Qt::white);
    //</Farben>
    //<Standard>
    double zoomScale = 0;
    int minusTowerCost = 50, herzTowerCost = 40, repostTowerCost = 75, benisTowerCost = 100;
    int shaking = 0;
    int shakeX=0, shakeY=0, shakeIntensity=3;
    QFont f = QFont("Arial");
    /*Active-States
    0 - Hauptmenü
    1 - Ingame
    2 - Mapeditor*/
    int active = 0;
    int subActive = 0;
    uint subActiveSelected = 0;
    uint subLevelSelected = 0;
    int mapwidth = 1920;
    int mapheight = 1080;
    QRectF viewRect = QRect(0,0,1920,1080);
    int moveAn = 0;
    double transX = 0;
    double transY = 0;
    int begin = -1;
    QPoint mPos;
    QPointF mid;
    int zooming = 0;
    bool pressed = false;
    bool backMenu = false;
    int moved = 0;
    //Währung
    int benis = 0;
    int shekel = 0;
    //
    bool mapLoaded = false;
    bool resetPos = false;
    int towerMenu = -1;
    int towerMenuSelected = 0;
    double fade = 1;
    int fdir = 0;
    QRectF target;
    QRect towerMenuRect = QRect(0,150,600,800);
    QRect minusTowerRect = QRect(50,200,150,150);
    QRect favTowerRect = QRect(225,200,150,150);
    QRect repostTowerRect = QRect(400,200,150,150);
    QRect benisTowerRect = QRect(50,375,150,150);
    QRect btnDmgRect = QRect(10,450,285,85);
    QRect btnRangeRect = QRect(305,450,285,85);
    QRect btnFirerateRect = QRect(10,545,285,85);
    QRect btnSpeedRect = QRect(305,545,285,85);
    QRect btnSellRect = QRect(10,640,285,85);
    //Hauptmenü
    QRect startRect = QRect(50,500,400,89);
    QRect editRect = QRect(50,600,400,89);
    //283 x 413
    QRect btnContinueRect = QRect(677,413,566,127);
    QRect btnContinueMenuRect = QRect(50,400,400,89);
    QRect btnBackRect = QRect(677,540,566,127);
    //Mapeditor
    int mapSaved=0;
    int mWidth = 0;
    int mHeight = 0;
    QString mName = "";
    int chosenSpruch = 0;
    int enemyBaseCount = 0;
    int ownBaseCount = 0;
    bool mapPlaying = false;
    bool mediumUnlocked = true;
    bool largeUnlocked = true;
    bool gamePaused = false;
    bool hasPlayingSave = false;
    bool isGameOver = false;
    bool key = false;
    bool exit = false;
    int editorSelected = 0;
    int mCompileError = -1;
    QRect mPathRect = QRect(25,850,200,200);
    QRect mTurmTileRect = QRect(250,850,200,200);
    QRect mEmptyTileRect = QRect(475,850,200,200);
    QRect mBaseRect = QRect(925,850,200,200);
    QRect mEnemyBaseRect = QRect(700,850,200,200);
    QRect mClearRect = QRect(1150,850,200,200);
    QRect mPlayRect = QRect(25,25,200,200);
    QRect mSaveRect = QRect(1695,25,200,200);
    QRect menuPlayRect = QRect(1545,880,150,150);
    QRect menuDelRect = QRect(1720,880,150,150);
    //</Standard>
    //DEBUG
    const double NANO_TO_MILLI = 1000000.0;
    const uint mainLevels = 2;
    const QString serverIP = "flatterfogel.ddns.net";
    double tmainMS = 0;
    double tdrawMS = 0;
    double twavespeedMS = 0;
    double tprojectileMS = 0;
    double tprojectileFMS = 0;
    //<BASE>
    int basehp = 75;
    //</BASE>
    void drawIngame(QPainter &painter);
    void drawTowerMenu(QPainter &painter);
    void drawTower(QRect pos, QPainter &painter, int tnum=-1, bool info=false);
    void drawBar(QPainter &painter, Tower *t, int num);
    void drawMainMenu(QPainter &painter);
    void drawEditor(QPainter &painter);
    void drawEditorMenu(QPainter &painter);
    void drawBackMenu(QPainter &painter);
    void towerMenuClicked(QRect pos);
    void editorMenuClicked(QRect pos);
    void startMenuClicked(QRect pos);
    void playClicked();
    void delClicked();
    void backMenuClicked(QRect pos);
    void editorClicked(QRect pos, QRect aPos);
    void gameOver();
    void loadUserData();
    void loadMap(QString name, int custom=0, int width=0, int height=0, QString path=":/data/maps/");
    void loadLevels();
    void loadGameSave();
    void loadLitteSprueche();
    void saveGameSave();
    int createPath();
    void changeSize(QPainter &painter, int pixelSize, bool bold=false);
    void buyMinusTower(int pos);
    void buyFavTower();
    void buyRepostTower();
    void buyBenisTower();
    void delEnemy(int pos);
    void reset(int custom = 0);
    void delAllEnemys();
    void pauseGame();
    int sendDataToServer(QString data);
    QDate getServerDate();
    int purchaseMediumMap();
    int purchaseLargeMap();
    Level parseLvlString(QString lvl, int startPos=3);
    QPixmap createMapImage(int custom=0);
    QPixmap createMapImage(QString data, int width, int height);

private slots:
    void handleTransaction(QInAppTransaction *transaction);
    void productRegistered(QInAppProduct *product);
    void productUnknown(QInAppProduct::ProductType ptype,QString id);
    void on_tdraw();
    void on_tmain();
    void on_tprojectile();
    void on_tprojectilefull();
    void on_twave();
    void on_twaveSpeed();
    void on_tanimation();
    void on_tshake();
    void on_tcoll();
    void on_tmenuAn();

protected:
    bool event(QEvent *e) override;
    void touchEvent(QTouchEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *e);
    void closeEvent(QCloseEvent *e);
    void wheelEvent(QWheelEvent *e) override;
};

#endif // FRMMAIN_H
