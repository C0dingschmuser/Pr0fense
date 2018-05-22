#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QDesktopWidget>
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
#include <QFuture>
#include <QtMath>
#include <QInputDialog>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QtPurchasing>
#include <QTcpSocket>
#include <QDate>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QDir>
#include <QDesktopServices>
#include <QClipboard>
#include "Box2D/Box2D.h"
#include "tower.h"
#include "objectpool.h"
#include "tile.h"
#include "engine.h"
#include "level.h"
#include "wave.h"
#include "shop.h"
#include "path.h"
#include "msgbox.h"
#include "account.h"
#ifdef Q_OS_ANDROID
#include "lockhelper.h"
#endif

namespace Ui {
class FrmMain;
}

enum _entityCategory {
    BOUNDARY =      0x0001,
    NORMAL_ENEMY =  0x0002,
    FLYING_ENEMY =  0x0004,
};

//EnemyType Enums in enemy.h definiert

//TowerType Enums in tower.h definiert

enum _gameStates {
    STATE_MAINMENU =   0,
    STATE_PLAYING =    1,
    STATE_EDITOR =     2,
    STATE_SUSPENDED = -1,
};

enum _animationStates {
    ANIMATION_PLAY =       1,
    ANIMATION_EDITOR =     2,
    ANIMATION_EDITOR_OWN = 3,
    ANIMATION_BACK =       4,
    ANIMATION_CONTINUE =   5,
    ANIMATION_SUSPENDED = 0,
};

enum _accountStates {
    ACCOUNT_BANNED = -1,
    ACCOUNT_INACTIVE = 0,
    ACCOUNT_ACTIVE = 1,
    ACCOUNT_VERIFIED = 2,
    ACCOUNT_CHEATER = 3,
    ACCOUNT_PENDING = 4,
};

class FrmMain : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit FrmMain(QOpenGLWidget *parent = 0);
    ~FrmMain();

private:
    Ui::FrmMain *ui;
#ifdef Q_OS_ANDROID
    QInAppStore *store;
    KeepAwakeHelper helper; //verhindert, dass app in standby geht
#endif
    Shop *shop;
    const QString version = "1.02";
    QString newestPost = "2561200";
    Account account;
    bool newPost = false;
    b2World *world=nullptr;
    b2Body *wall1=nullptr,*wall2=nullptr,*wall3=nullptr,*wall4=nullptr;
    QMediaPlayer *music;
    QMediaPlaylist *playlist_game;
    QMediaPlaylist *playlist_menu;
    QTcpSocket *server;
    const QString serverIP = "flatterfogel.ddns.net";
    bool outdated = false;
    QMutex mutex;
    QDate latestTestingDate = QDate(2018,5,01);
    bool DEBUG = false;
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
    QTimer *t_menuAn;
    QTimer *t_physics;
    QTimer *t_idle;
    QThread *workerThread;
    double scaleX;
    double scaleY;
    double scaleFHDX;
    double scaleFHDY;
    ObjectPool *pool;
    std::vector <Tower*> towers;
    std::vector <Enemy*> enemys;
    std::vector <Projectile*> projectiles;
    std::vector <Tile*> tiles;
    Tile *chosenTile = NULL;
    std::vector <Path> paths;
    std::vector <QString> spruch;
    std::vector <QPixmap> pathTextures;
    std::vector <QPixmap> lvlPreviews;
    std::vector <QRect> mainLvlRects;
    std::vector <Level> levels;
    std::vector <uint> lvlPrices;
    std::vector <double> frameTimes;
    std::vector <b2Body*> delBodies;
    std::vector <Enemy*> createBodies;
    std::vector <Tile*> createTileBodies;
    std::vector <QPixmap> minigunTowerPx;
    QPixmap mapTile = QPixmap(":/data/images/maptile.png");
    QPixmap turmtile = QPixmap(":/data/images/turmtile.png");
    QPixmap emptytile = QPixmap(":/data/images/empty.png");
    QPixmap enemybasePx = QPixmap(":/data/images/enemybase.png");
    QPixmap ownbasePx = QPixmap(":/data/images/ownbase.png");
    QPixmap towerGroundPx = QPixmap(":/data/images/towers/base.png");
    QPixmap towerBasePx = QPixmap(":/data/images/towers/base2.png");
    QPixmap towerRepostBasePx = QPixmap(":/data/images/towers/repost.png");
    QPixmap circleSplit1 = QPixmap(":/data/images/enemys/death/circle1.png");
    QPixmap circleSplit2 = QPixmap(":/data/images/enemys/death/circle2.png");
    QPixmap circleSplit3 = QPixmap(":/data/images/enemys/death/circle3.png");
    QPixmap rauteSplit1 = QPixmap(":/data/images/enemys/death/raute1.png");
    QPixmap rauteSplit2 = QPixmap(":/data/images/enemys/death/raute2.png");
    QPixmap rauteSplit3 = QPixmap(":/data/images/enemys/death/raute3.png");
    QPixmap rauteSplit4 = QPixmap(":/data/images/enemys/death/raute4.png");
    QPixmap repostMark = QPixmap(":/data/images/towers/repost_mark.png");
    QPixmap herzPx = QPixmap(":/data/images/towers/herz.png");
    QPixmap minus = QPixmap(":/data/images/towers/minus.png");
    QPixmap sniperPx = QPixmap(":/data/images/towers/sniper.png");
    QPixmap minusTowerPx = QPixmap(":/data/images/towers/minusTower.png");
    QPixmap favTowerPx = QPixmap(":/data/images/towers/favTower.png");
    QPixmap flakTowerPx = QPixmap(":/data/images/towers/flak.png");
    QPixmap laserTowerPx = QPixmap(":/data/images/towers/lasertower.png");
    QPixmap poisonTowerPx = QPixmap(":/data/images/towers/poison.png");
    QPixmap poison_gas = QPixmap(":/data/images/towers/poison_poison.png");
    QPixmap blus = QPixmap(":/data/images/towers/blus.png");
    QPixmap blus_blurred = QPixmap(":/data/images/towers/blus_blurred.png");
    QPixmap buyPx = QPixmap(":/data/images/ui/kaufen.png");
    QPixmap sellPx = QPixmap(":/data/images/ui/verkaufen.png");
    QPixmap auswahlPx = QPixmap(":/data/images/ui/auswahl.png");
    QPixmap auswahlTile = QPixmap(":/data/images/auswahltile.png");
    QPixmap btnSpeed = QPixmap(":/data/images/ui/btnSpeed.png");
    QPixmap btnSpeed_grau = QPixmap(":/data/images/ui/deaktiviert/btnSpeed.png");
    QPixmap btnDmg = QPixmap(":/data/images/ui/btnDmg.png");
    QPixmap btnDmg_grau = QPixmap(":/data/images/ui/deaktiviert/btnDmg.png");
    QPixmap btnFirerate = QPixmap(":/data/images/ui/btnFirerate.png");
    QPixmap btnFirerate_grau = QPixmap(":/data/images/ui/deaktiviert/btnFirerate.png");
    QPixmap btnRange = QPixmap(":/data/images/ui/btnRange.png");
    QPixmap btnrange_grau = QPixmap(":/data/images/ui/deaktiviert/btnRange.png");
    QPixmap btnSell = QPixmap(":/data/images/ui/btnSell.png");
    QPixmap btnTurn = QPixmap(":/data/images/ui/btnTurn.png");
    QPixmap btnTurn_grau = QPixmap(":/data/images/ui/deaktiviert/btnTurn.png");
    QPixmap btnBack = QPixmap(":/data/images/ui/zurueck.png");
    QPixmap btnContinue = QPixmap(":/data/images/ui/weiterspielen.png");
    QPixmap btnContinueGrey = QPixmap(":/data/images/ui/deaktiviert/weiterspielen_grau.png");
    QPixmap btnStopPx = QPixmap(":/data/images/ui/stop.png");
    QPixmap btnNormalPx = QPixmap(":/data/images/ui/normal.png");
    QPixmap btnFastPx = QPixmap(":/data/images/ui/fast.png");
    QPixmap btnSuperfastPx = QPixmap(":/data/images/ui/superfast.png");
    QPixmap soundAusPx = QPixmap(":/data/images/ui/sound0.png");
    QPixmap soundAnPx = QPixmap(":/data/images/ui/sound1.png");
    QPixmap barPx = QPixmap(":/data/images/ui/powerBar.png");
    QPixmap menuPx = QPixmap(":/data/images/menu.png");
    QPixmap titlePx = QPixmap(":/data/images/ui/title.png");
    QPixmap startPx = QPixmap(":/data/images/ui/start.png");
    QPixmap editPx = QPixmap(":/data/images/ui/editor.png");
    QPixmap exitPx = QPixmap(":/data/images/ui/beenden.png");
    QPixmap shopPx = QPixmap(":/data/images/ui/shop.png");
    QPixmap account_failPx = QPixmap(":/data/images/ui/account-fail.png");
    QPixmap account_successPx = QPixmap(":/data/images/ui/account_success.png");
    QPixmap account_success_pendingPx = QPixmap(":/data/images/ui/account_waiting.png");
    QPixmap enemyPx = QPixmap(":/data/images/enemy.png");
    QPixmap pr0coinPx = QPixmap(":/data/images/towers/coin.png");
    QPixmap baseHerzPx = QPixmap(":/data/images/ui/herz.png");
    QPixmap wavesPx = QPixmap(":/data/images/ui/waves.png");
    QPixmap rotorPx = QPixmap(":/data/images/rotor.png");
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
    QPixmap mEditPx = QPixmap(":/data/images/ui/edit.png");
    QPixmap mapPx;
    //<Farben>
    QColor grau = QColor(22,22,24);
    QColor gebannt = QColor(68,68,68);
    QColor fliese = QColor(108,67,43);
    QColor neuschwuchtel = QColor(225,8,233);
    QColor schwuchtel = QColor(Qt::white);
    QColor edlerSpender = QColor(28,185,146);
    QColor altschwuchtel = QColor(91,185,28);
    QColor mod = QColor(0,143,255);
    QColor admin = QColor(255,153,0);
    QColor legende = QColor(28,185,146);
    //</Farben>
    //<Standard>
    double zoomScale = 0;
    const uint minusTowerCost = 50, herzTowerCost = 75, repostTowerCost = 125, benisTowerCost = 100,
        banTowerCost = 300, sniperTowerCost = 250, flakTowerCost = 100, laserTowerCost = 175,
        poisonTowerCost = 300, minigunTowerCost = 200;
    const double upgradeHighConst = 0.75, upgradeLowConst = 0.5;
    int shaking = 0;
    int shakeX = 0, shakeY=0, shakeIntensityX = 2, shakeIntensityY = 2;
    QFont f = QFont("Arial");
    /*Active-States
    0 - Hauptmenü
    1 - Ingame
    2 - Mapeditor*/
    int active = STATE_SUSPENDED;
    uint upgradeCost = 0;
    int subActive = 0;
    uint subActiveSelected = 0;
    uint subLevelSelected = 0;
    int mapwidth = 1920;
    int mapheight = 1080;
    QRectF viewRect = QRect(0,0,1920,1080);
    int moveAn = 0;
    double transX = 0;
    double transY = 0;
    double shekelCoinSize = 0;
    bool accepted = false;
    int begin = -1;
    QPoint mPos;
    QPointF mid;
    int zooming = 0;
    bool pressed = false;
    bool backMenu = false;
    bool physicsPaused = false;
    int moved = 0;
    //Währung
    unsigned long long benis = 0;
    int shekel = 0;
    //
    bool mapLoaded = false;
    bool resetPos = false;
    bool towerMenuAnimatingDir = 0;
    int towerMenuAnimating = -1;
    int towerMenu = -1;
    int towerMenuSelected = 0;
    int delAll = 0;
    double fade = 1;
    double blendRectOpacity = 0;
    int fdir = 0;
    QRectF target;
    QRectF pr0coinRect = QRectF(0,0,100,100);
    QRect towerMenuRect = QRect(0,150,600,930);
    QRect towerMenuRectAnimation = QRect(-600,150,500,930);
    QRect minusTowerRect = QRect(50,200,150,150);
    QRect favTowerRect = QRect(225,200,150,150);
    QRect repostTowerRect = QRect(400,200,150,150);
    QRect benisTowerRect = QRect(50,375,150,150);
    QRect banTowerRect = QRect(225,375,150,150);
    QRect sniperTowerRect = QRect(400,375,150,150);
    QRect flakTowerRect = QRect(50,550,150,150);
    QRect laserTowerRect = QRect(225,550,150,150);
    QRect poisonTowerRect = QRect(400,550,150,150);
    QRect minigunTowerRect = QRect(50,725,150,150);
    QRect btnDmgRect = QRect(10,450,285,85);
    QRect btnRangeRect = QRect(305,450,285,85);
    QRect btnFirerateRect = QRect(10,545,285,85);
    QRect btnSpeedRect = QRect(305,545,285,85);
    QRect btnSellRect = QRect(10,640,285,85);
    QRect btnTurnRect = QRect(305,640,285,85);
    //Hauptmenü
    QRect startRect = QRect(50,500,400,89);
    QRect editRect = QRect(50,600,400,89);
    QRect shopRect = QRect(50,700,400,89);
    QRect exitRect = QRect(50,800,400,89);
    QRect shekelRect = QRect(50, 330, 100, 50);
    QRect accountRect = QRect(50, 900, 100, 100);
    //283 x 413
    QRect btnContinueRect = QRect(677,413,566,127);
    QRect btnContinueMenuRect = QRect(50,400,400,89);
    QRect btnBackRect = QRect(677,540,566,127);
    QRect btnSoundRect = QRect(1720,30,150,130);
    //Mapeditor
    int mapSaved=0;
    int mWidth = 0;
    int mHeight = 0;
    QString mName = "";
    int chosenSpruch = 0;
    int enemyBaseCount = 0;
    int ownBaseCount = 0;
    int waveCount;
    int internalWaveCount;
    int chosenpath = 0;
    Wave currentWave;
    bool mapPlaying = false;
    bool mediumUnlocked = true;
    bool largeUnlocked = true;
    bool gamePaused = false;
    bool hasPlayingSave = false;
    bool isGameOver = false;
    bool key = true;
    bool exit = false;
    bool soundEnabled = true;
    bool superfastUnlocked = false;
    int editorSelected = 0;
    int playingSpeed = 1;
    int mCompileError = -1;
    QRect mPathRect = QRect(25,850,200,200);
    QRect mTurmTileRect = QRect(250,850,200,200);
    QRect mEmptyTileRect = QRect(475,850,200,200);
    QRect mBaseRect = QRect(925,850,200,200);
    QRect mEnemyBaseRect = QRect(700,850,200,200);
    QRect mClearRect = QRect(1150,850,200,200);
    QRect mPlayRect = QRect(25,25,200,200);
    QRect mSaveRect = QRect(1695,25,200,200);
    QRect menuPlayRect = QRect(1545,900,150,150);
    QRect menuDelRect = QRect(1720,900,150,150);
    QRect btnPlayingSpeedRect = menuDelRect;
    //</Standard>
    //DEBUG
    bool suspended = false;
    bool loaded = false;
    bool error=false;
    int appState = Qt::ApplicationActive;
    const double NANO_TO_MILLI = 1000000.0;
    const uint mainLevels = 4;
    double tmainMS = 0;
    double tdrawMS = 0;
    double twavespeedMS = 0;
    double tprojectileMS = 0;
    double tprojectileFMS = 0;
    //<BASE>
    int basehp = 75;
    //</BASE>
    void initPhysics();
    void restore(int amount);
    void addTowerTargets(Tower *t);
    void shake(int duration, int shakeIX, int shakeIY);
    void resetTimers();
    void checkPush(b2Body *delBody);
    void createDeathProjectiles(QRect rect, bool circle = true);
    void createBounds();
    void drawIngame(QPainter &painter);
    void drawHUD(QPainter &painter);
    void drawTowerMenu(QPainter &painter);
    void drawTower(QRect pos, QPainter &painter, Tower *tmpTower=nullptr, int info=false);
    void drawBar(QPainter &painter, Tower *t, int num);
    void drawMainMenu(QPainter &painter);
    void drawEditor(QPainter &painter);
    void drawEditorMenu(QPainter &painter);
    void drawBackMenu(QPainter &painter);
    void towerMenuClicked(QRect pos);
    void editorMenuClicked(QRect pos);
    void startMenuClicked(QRect pos);
    void handleCheater(QRect pos);
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
    void saveMaps();
    void saveOptions();
    void loadOptions();
    void loadItems();
    void changePlaylist(int playlist);
    int createPath();
    void loginAccount();
    void createAccount();
    void saveAccount();
    void accountCreationError();
    void changeSize(QPainter &painter, int pixelSize, bool bold = false);
    void buyMinusTower();
    void buyFavTower();
    void buyRepostTower();
    void buyBenisTower();
    void buyBanTower();
    void buySniperTower();
    void buyFlakTower();
    void buyLaserTower();
    void buyPoisonTower();
    void buyMinigunTower();
    void delEnemy(int pos);
    void delTower(Tower *t);
    void reset(int custom = 0);
    void delAllEnemys(int a=0);
    void pauseGame();
    void error_string(QString e1,QString e2,QString e3);
    void error_save(QFile &file);
    int sendDataToServer(QString data);
    int getEnemySizeByType(int type = 0);
    double getHealth(bool max = true);
    bool checkTimers(int type = 0);
    bool worldContains(b2Body *b);
    QDate getServerDate();
    void getStatus();
    void newPostInfo();
    Level parseLvlString(QString lvl, int startPos=3);
    QPixmap createMapImage(int custom=0);
    QPixmap createMapImage(QString data, int width, int height);

private slots:
#ifdef Q_OS_ANDROID
    void handleTransaction(QInAppTransaction *transaction);
    void productRegistered(QInAppProduct *product);
    void productUnknown(QInAppProduct::ProductType ptype,QString id);
#endif
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
    void on_tphysics();
    void on_tidle();
    void on_mediaStateChanged(QMediaPlayer::MediaStatus status);
    void on_appStateChanged(Qt::ApplicationState state);
    void on_mapBuy(int subSelected);
    void on_buyError(int id);
    void on_purchaseShekel(QString paket);
    void on_buyItem(int pos);

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
