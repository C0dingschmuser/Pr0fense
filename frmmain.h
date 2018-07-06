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
#include <QScreen>
#include <QClipboard>
#include <QSettings>
#include <QFutureWatcher>
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
#include "settings.h"
#include "dailyreward.h"
#include "statistics.h"
#ifdef Q_OS_ANDROID
#include "lockhelper.h"
#endif

namespace Ui {
class FrmMain;
}

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
    Statistics *statistics;
    constexpr static const double version = 1.037;
    const int disabledTime = 4000;
    int newestPost = 2607304;
    int connectionTries = 0;
    Account account;
    Settings *settings;
    DailyReward *dailyReward;
    QFutureWatcher <void> watcher;
    int futureID = 0;
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
    bool dateVerified = false;
    QDate currentDate = QDate::currentDate();
    QDate lastPlayedDate = QDate::currentDate();
    QDate lastRewardDate = QDate(2000,1,1);
    bool DEBUG = false;
    bool output = false;
    int debugns = 0;
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
    QTimer *t_grasAn;
    QTimer *t_towerTarget;
    QThread *workerThread;
    QThread *projectileThread;
    QThread *physicsThread;
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
    std::vector <QString> bossNames;
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
    std::vector <QPixmap> grasAnimation;
    std::vector <QPixmap> repostAnimation;
    std::vector <QPixmap> normalEnemyDeath;
    std::vector <QPixmap> bossEnemyDeath;
    std::vector <QPixmap> normalEnemys;
    std::vector <QPixmap> blitzAnimation;
    std::vector <QPixmap> wandAnimation;
    std::vector <int> chosenTiles;
    QPixmap pr0fensePx = QPixmap(":/data/images/logo.png");
    QPixmap repairPx;
    QPixmap repairPx_grau;
    QPixmap bossPx;
    QPixmap mapTile;
    QPixmap turmtile;
    QPixmap emptytile;
    QPixmap enemybasePx;
    QPixmap ownbasePx;
    QPixmap towerGroundPx;
    QPixmap towerRepostBasePx;
    QPixmap circleSplit1;
    QPixmap circleSplit2;
    QPixmap circleSplit3;
    QPixmap rauteSplit1;
    QPixmap rauteSplit2;
    QPixmap rauteSplit3;
    QPixmap rauteSplit4;
    QPixmap repostMark;
    QPixmap herzPx;
    QPixmap minus;
    QPixmap sniperPx;
    QPixmap minusTowerPx;
    QPixmap favTowerPx;
    QPixmap flakTowerPx;
    QPixmap laserTowerPx;
    QPixmap poisonTowerPx;
    QPixmap poison_gas;
    QPixmap blus;
    QPixmap blus_blurred;
    QPixmap buyPx;
    QPixmap sellPx;
    QPixmap auswahlPx;
    QPixmap auswahlTile;
    QPixmap btnSpeed;
    QPixmap btnSpeed_grau;
    QPixmap btnDmg;
    QPixmap btnDmg_grau;
    QPixmap btnFirerate;
    QPixmap btnFirerate_grau;
    QPixmap btnRange;
    QPixmap btnrange_grau;
    QPixmap btnSell;
    QPixmap btnSell_grau;
    QPixmap btnTurn;
    QPixmap btnTurn_grau;
    QPixmap btnBack;
    QPixmap btnContinue;
    QPixmap btnContinueGrey;
    QPixmap btnStopPx;
    QPixmap btnStop2Px;
    QPixmap btnStop3Px;
    QPixmap btnNormalPx;
    QPixmap btnFastPx;
    QPixmap btnSuperfastPx;
    QPixmap soundAusPx;
    QPixmap soundAnPx;
    QPixmap barPx;
    QPixmap menuPx;
    QPixmap titlePx;
    QPixmap startPx;
    QPixmap editPx;
    QPixmap exitPx;
    QPixmap shopPx;
    QPixmap account_failPx;
    QPixmap account_successPx;
    QPixmap account_success_pendingPx;
    QPixmap enemyPx;
    QPixmap pr0coinPx;
    QPixmap baseHerzPx;
    QPixmap wavesPx;
    QPixmap rotorPx;
    QPixmap shekelMoney;
    QPixmap lock_quadratPx;
    QPixmap shekelPlusPx;
    QPixmap sternPx;
    QPixmap settingsPx;
    QPixmap questionPx;
    QPixmap popupPx;
    QPixmap minusButtonPx;
    QPixmap statsPx;
    //Playmenu
    QPixmap levelsPx;
    QPixmap ownMapsPx;
    QPixmap communityPx;
    //Mapeditor
    QPixmap movePx;
    QPixmap delPx;
    QPixmap lockPx;
    QPixmap mPlayPx;
    QPixmap mStopPx;
    QPixmap mSavePx;
    QPixmap mEditPx;
    QPixmap mapPx;
    QPixmap mapPx_gras;
    //</Farben>
    //<Standard>
    double zoomScale = 0;
    const uint minusTowerCost = 50, herzTowerCost = 75, repostTowerCost = 150, benisTowerCost = 200,
        banTowerCost = 250, sniperTowerCost = 350, flakTowerCost = 100, laserTowerCost = 300,
        poisonTowerCost = 500, minigunTowerCost = 400;
    const double upgradeHighConst = 0.75, upgradeLowConst = 0.5;
    int shaking = 0;
    int playedGames = 0;
    int towerBlitzAnNum = 0;
    double shakeX = 0, shakeY=0, shakeIntensityX = 2, shakeIntensityY = 2;
    QFont f = QFont("Arial");
    /*Active-States
    0 - Hauptmenü
    1 - Ingame
    2 - Mapeditor*/
    bool userDataLoaded = false;
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
    int beginFade = 500;
    bool accepted = false;
    bool canStartNextWave = false;
    bool editMode = 0;
    bool cloudSaving = true;
    bool gameLoading = true;
    int begin = -1;
    QPointF mPos;
    QPointF mid;
    int zooming = 0;
    bool pressed = false;
    bool backMenu = false;
    bool physicsPaused = false;
    bool isLoading();
    void startRandomTile();
    bool spawnTiles = false;
    bool closing = false;
    int moved = 0;
    int popup = 0;
    //Währung
    unsigned long long benis = 0;
    QString benisSave = "11";
    int shekel = 0;
    //
    bool mapLoaded = false;
    bool resetPos = false;
    bool towerMenuAnimatingDir = 0;
    int towerMenuAnimating = -1;
    int towerMenu = -1;
    int towerMenuSelected = 0;
    int powerupSelected = -1;
    int delAll = 0;
    double fade = 1;
    double blendRectOpacity = 0;
    int fdir = 0;
    int towerPxSize = 512;
    QRectF target;
    QRect okButtonRect = QRect(585,900,200,100);
    QRect dsButtonRect = QRect(835,900,200,100);
    QRect nbButtonRect = QRect(1085,900,200,100);
    QRectF pr0coinRect = QRectF(0,0,100,100);
    QRect repairButtonRect = QRect(200,565,200,200);
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
    QRect btnZoomPlusRect = QRect(1825, 425, 75, 75);
    QRect btnZoomMinusRect = QRect(1825, 550, 75, 75);
    QRect wandRect = QRect(50, 930, 100, 100);
    //Hauptmenü
    QRect startRect = QRect(50,500,400,89);
    QRect editRect = QRect(50,600,400,89);
    QRect shopRect = QRect(50,700,400,89);
    QRect exitRect = QRect(50,800,400,89);
    QRect shekelRect = QRect(50, 330, 100, 50);
    QRect shekelPlusRect = QRect(400, 335, 50, 50);
    QRect accountRect = QRect(50, 900, 100, 100);
    QRect statsRect = QRect(150, 900, 100, 100);
    QRect questionRect = QRect(250, 900, 100, 100);
    QRect settingsRect = QRect(350, 900, 100, 100);
    QRect popupRect = QRect(350,200,1280,720);
    //283 x 413
    QRect btnContinueRect = QRect(677,413,566,127);
    QRect btnContinueMenuRect = QRect(50,400,400,89);
    QRect btnBackRect = QRect(677,540,566,127);
    QRect btnSoundRect = QRect(1720,30,150,130);
    //Mapeditor
    bool isEditorMenuClicked = false;
    int mapSaved=0;
    int mWidth = 0;
    int mHeight = 0;
    QString mName = "";
    int chosenSpruch = 0;
    int enemyBaseCount = 0;
    int ownBaseCount = 0;
    int waveCount;
    QString waveCountSave;
    int internalWaveCount;
    int chosenpath = 0;
    int mapID = 0;
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
    bool musicLoaded = false;
    bool shekelLocked = false;
    bool benisLocked = false;
    bool basehplocked = false;
    bool waveCountLocked = false;
    bool superfastUnlocked = false;
    int isZooming = false;
    int zoomTime = 0;
    int maxZoomTime = 0;
    double zoomAmount = 0;
    int editorSelected = 0;
    int playingSpeed = 1;
    int mCompileError = -1;
    QRect mPathRect = QRect(25,850,150,150);
    QRect mTurmTileRect = QRect(250,850,150,150);
    QRect mEmptyTileRect = QRect(475,850,150,150);
    QRect mBaseRect = QRect(925,850,150,150);
    QRect mEnemyBaseRect = QRect(700,850,150,150);
    QRect mClearRect = QRect(1150,850,150,150);
    QRect mPlayRect = QRect(25,25,150,150);
    QRect mSaveRect = QRect(1695,25,150,150);
    QRect menuPlayRect = QRect(1545,900,150,150);
    QRect menuDelRect = QRect(1720,900,150,150);
    QRect btnPlayingSpeedRect = menuDelRect;
    QRect btnNextWaveRect = QRect(menuDelRect.x() - 170, menuDelRect.y(), 150, 150);
    //</Standard>
    //DEBUG
    bool suspended = false;
    bool loaded = false;
    bool error=false;
    bool statusConn = false;
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
    QString basehpSave = "86";
    //</BASE>
    void initPhysics();
    void restore(int amount);
    void addTowerTargets(Tower *t);
    void upgradeDmg(Tower *tmpTower);
    void setShekel(unsigned long long amount, bool save = true, bool loading = false);
    void setBenis(unsigned long long amount, bool loading = false);
    void setBaseHp(int amount);
    void setWaveCount(int amount);
    void shake(int duration, double shakeIX, double shakeIY);
    void resetTimers();
    void checkPush(b2Body *delBody);
    void checkPush(Tile *t);
    void getCollidingBits(int type, uint16_t &ownCategoryBits, uint16_t &collidingCategoryBits);
    void createDeathProjectiles(QRect rect, int circle = true);
    void createBounds();
    void drawIngame(QPainter &painter);
    void drawHUD(QPainter &painter);
    void drawTowerMenu(QPainter &painter);
    void drawTower(QRect pos2, QPainter &painter, Tower *tmpTower=nullptr, int info=false);
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
    void loadMap(QString name, int custom=0, int width=0, int height=0, QString path=":/data/maps/", double mapversion = version);
    void loadLevels();
    void loadGameSave();
    void loadLitteSprueche();
    void saveGameSave();
    void saveMaps();
    void saveOptions(unsigned long long customShekel = 0, bool init = false);
    void loadGraphics();
    void loadOptions();
    void loadItems();
    void changePlaylist(int playlist);
    int createPath();
    void zoom(double amount);
    void createPathBoundaries();
    void loginAccount();
    void createAccount();
    void saveAccount();
    void accountCreationError();
    void changeSize(QPainter &painter, int pixelSize, bool bold = false);
    void buyMinusTower(int tpos);
    void buyFavTower(int tpos);
    void buyRepostTower(int tpos);
    void buyBenisTower(int tpos);
    void buyBanTower(int tpos);
    void buySniperTower(int tpos);
    void buyFlakTower(int tpos);
    void buyLaserTower(int tpos);
    void buyPoisonTower(int tpos);
    void buyMinigunTower(int tpos);
    void buyShekelAnimation(int amount, int price);
    void placeWall(QPointF pos);
    void delEnemy(int pos);
    void delTower(Tower *t);
    void reset(int custom = 0);
    void delAllEnemys(int a=0);
    void pauseGame();
    void error_string(QString e1,QString e2,QString e3);
    void error_save(QFile &file, QString msg = "Fehler beim laden!");
    int calcProjectiles(int num);
    int sendDataToServer(QString data);
    int getEnemySizeByType(int type = 0);
    int getPathId(int i);
    double getHealth(bool max = true);
    double getLegendenDistanz(int ownPos);
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
    void on_tgrasAn();
    void on_ttowerTarget();
    void on_mediaStateChanged(QMediaPlayer::MediaStatus status);
    void on_appStateChanged(Qt::ApplicationState state);
    void on_mapBuy(int subSelected);
    void on_buyError(int id);
    void on_purchaseShekel(QString paket);
    void on_buyItem(int pos);
    void on_buyTower(int pos);
    void on_openPage(int id);
    void on_setShekel(unsigned long long shekel, bool save = true);
    void on_futureFinished();
    void on_addBonus(int amount);
    void on_graphicsChanged(int graphic);
    void on_energieSparenChanged(bool mode);
    void on_zoomChanged();

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
