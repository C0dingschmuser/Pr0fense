#include "frmmain.h"
#include "ui_frmmain.h"

FrmMain::FrmMain(QOpenGLWidget *parent) :
    QOpenGLWidget(parent),
    ui(new Ui::FrmMain)
{
    ui->setupUi(this);
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    setAttribute(Qt::WA_AcceptTouchEvents,true);
    //QCoreApplication::setOrganizationName("Bruh Games");
    //QCoreApplication::setApplicationName("Pr0fense");

    music = new QMediaPlayer();
    playlist_menu = new QMediaPlaylist();
    playlist_game = new QMediaPlaylist();
    playlist_menu->setPlaybackMode(QMediaPlaylist::Loop);
    playlist_game->setPlaybackMode(QMediaPlaylist::Loop);
    account.ip = serverIP;
    account.accountState = ACCOUNT_INACTIVE;

    qsrand(static_cast<quint64>(QTime::currentTime().msecsSinceStartOfDay()));

    QString end = "mp3";
#ifdef Q_OS_ANDROID
    store = new QInAppStore(this);
    connect(store,SIGNAL(transactionReady(QInAppTransaction*)),this,SLOT(handleTransaction(QInAppTransaction*)));
    connect(store,SIGNAL(productRegistered(QInAppProduct*)),this,SLOT(productRegistered(QInAppProduct*)));
    connect(store,SIGNAL(productUnknown(QInAppProduct::ProductType,QString)),this,SLOT(productUnknown(QInAppProduct::ProductType,QString)));
    store->registerProduct(QInAppProduct::Consumable,QStringLiteral("shekel_supersmall"));
    store->registerProduct(QInAppProduct::Consumable,QStringLiteral("shekel_small"));
    store->registerProduct(QInAppProduct::Consumable,QStringLiteral("shekel_normal"));
    store->registerProduct(QInAppProduct::Consumable,QStringLiteral("shekel_big"));
    store->registerProduct(QInAppProduct::Consumable,QStringLiteral("shekel_large"));
    store->registerProduct(QInAppProduct::Consumable,QStringLiteral("shekel_shekel"));
    store->registerProduct(QInAppProduct::Consumable,QStringLiteral("shekel_ultra"));
    store->registerProduct(QInAppProduct::Unlockable,QStringLiteral("speed_superfast"));
    store->registerProduct(QInAppProduct::Unlockable,QStringLiteral("shekel_super"));
    end = "ogg";
    qputenv("QT_USE_ANDROID_NATIVE_DIALOGS", "0");
#endif
    playlist_menu->addMedia(QUrl("qrc:/data/music/track1."+end));
    playlist_game->addMedia(QUrl("qrc:/data/music/track2."+end));
    playlist_menu->addMedia(QUrl("qrc:/data/music/track4."+end));
    playlist_game->addMedia(QUrl("qrc:/data/music/track3."+end));
    //fehler: wenn mapeditor ausgewählt und weiterspielen dann grid
    connect(music,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT(on_mediaStateChanged(QMediaPlayer::MediaStatus)));
    connect(qApp,SIGNAL(applicationStateChanged(Qt::ApplicationState)),this,SLOT(on_appStateChanged(Qt::ApplicationState)));
    pool = new ObjectPool(&mutex);
    dailyReward = new DailyReward();
    connect(dailyReward,SIGNAL(addBonus(int)),this,SLOT(on_addBonus(int)));

    settings = new Settings();
    connect(settings, SIGNAL(openPage(int)),this,SLOT(on_openPage(int)));
    connect(settings, SIGNAL(energieSparenChanged(bool)), this, SLOT(on_energieSparenChanged(bool)));
    connect(settings, SIGNAL(zoomChanged()), this, SLOT(on_zoomChanged()));

    shop = new Shop();
    connect(shop,SIGNAL(buyMap(int,bool)),this,SLOT(on_mapBuy(int,bool)));
    connect(shop,SIGNAL(error_buy(int)),this,SLOT(on_buyError(int)));
    connect(shop,SIGNAL(buyShekel(QString)),this,SLOT(on_purchaseShekel(QString)));
    connect(shop,SIGNAL(buyItem(int)),this,SLOT(on_buyItem(int)));
    connect(shop,SIGNAL(setShekel(unsigned long long,bool)),this,SLOT(on_setShekel(unsigned long long,bool)));

    manager = new QNetworkAccessManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(on_status(QNetworkReply*)));

    statistics = new Statistics();

    server = new QTcpSocket();
    t_draw = new QTimer();
    t_animation = new QTimer();
    t_main = new QTimer();
    t_projectile = new QTimer();
    t_projectile_full = new QTimer();
    t_wave = new QTimer();
    t_waveSpeed = new QTimer();
    t_shake = new QTimer();
    t_menuAn = new QTimer();
    t_physics = new QTimer();
    t_idle = new QTimer();
    t_grasAn = new QTimer();
    t_towerTarget = new QTimer();
    workerThread = new QThread();
    projectileThread = new QThread();
    physicsThread = new QThread();
    connect(t_draw,SIGNAL(timeout()),this,SLOT(on_tdraw()));
    connect(t_animation,SIGNAL(timeout()),this,SLOT(on_tanimation()));
    connect(t_main,SIGNAL(timeout()),this,SLOT(on_tmain()));
    connect(t_wave,SIGNAL(timeout()),this,SLOT(on_twave()));
    connect(t_waveSpeed,SIGNAL(timeout()),this,SLOT(on_twaveSpeed()));
    connect(t_projectile,SIGNAL(timeout()),this,SLOT(on_tprojectile()));
    connect(t_projectile_full,SIGNAL(timeout()),this,SLOT(on_tprojectilefull()));
    connect(t_shake,SIGNAL(timeout()),this,SLOT(on_tshake()));
    connect(t_menuAn,SIGNAL(timeout()),this,SLOT(on_tmenuAn()));
    connect(t_physics,SIGNAL(timeout()),this,SLOT(on_tphysics()));
    connect(t_idle,SIGNAL(timeout()),this,SLOT(on_tidle()));
    connect(t_grasAn,SIGNAL(timeout()),this,SLOT(on_tgrasAn()));
    connect(t_towerTarget,SIGNAL(timeout()),this,SLOT(on_ttowerTarget()));

    for(uint i = 1;i < 12;i++) {
        QPixmap p(":/data/images/path/path"+QString::number(i)+".png");
        pathTextures.push_back(p);
    }
    for(uint i = 0; i < 4; i++) {
        QPixmap p(":/data/images/towers/minigun/minigun" + QString::number(i+1));
        minigunTowerPx.push_back(p);
    }

    //AStar astar;
    astar.setHeuristics(manhattan);
    /*astar.setMatrix(4,4);
    astar.setStart(0,0);
    astar.setEnd(3,0);
    astar.setWay(1,0);
    astar.setWay(2,0);
    astar.setExpandCost(1,1,0);
    astar.setExpandCost(1,2,0);
    list<pair<UINT, UINT> > path;
    path = astar.getPath();
    list<pair<UINT, UINT> >::iterator pathIt;
    for (pathIt = path.begin(); pathIt != path.end(); pathIt++) {
        qDebug()<<pathIt->first<<pathIt->second;
    }
    astar.destroyMatrix();*/
    loadGraphics();
    loadUserData();
    loadLitteSprueche();

    initPhysics();
    if(accepted) gameLoading = false;
    shop->lvlPreviews = lvlPreviews;
    shop->lvlprices = lvlPrices;
    shop->mainlvls = mainLevels;
    shop->f = f;
    music->setPlaylist(playlist_menu);
    //getStatus();
    active = STATE_MAINMENU;
    t_idle->moveToThread(workerThread);
    t_physics->moveToThread(physicsThread);
    t_animation->moveToThread(workerThread);
    t_main->moveToThread(workerThread);
    t_shake->moveToThread(workerThread);
    t_wave->moveToThread(workerThread);
    t_waveSpeed->moveToThread(workerThread);
    t_projectile->moveToThread(projectileThread);
    t_projectile_full->moveToThread(projectileThread);
    t_menuAn->moveToThread(workerThread);
    t_grasAn->moveToThread(workerThread);
    t_towerTarget->moveToThread(workerThread);
    QThread::currentThread()->setPriority(QThread::HighPriority);
    on_energieSparenChanged(settings->energieSparen);
    workerThread->start(QThread::LowPriority);
    projectileThread->start(QThread::LowPriority);
    physicsThread->start(QThread::LowPriority);
    QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,10)); //turmschüsse & so
    QMetaObject::invokeMethod(t_idle,"start",Q_ARG(int,1000));
    QMetaObject::invokeMethod(t_physics,"start",Q_ARG(int,10)); //box2d physik
    QMetaObject::invokeMethod(t_grasAn,"start",Q_ARG(int,500));
    QMetaObject::invokeMethod(t_menuAn,"start",Q_ARG(int,100)); //blussis im hauptmenü
    QMetaObject::invokeMethod(t_towerTarget,"start",Q_ARG(int,50));
    QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,10)); //projektile von türmen
    QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,50)); //update von ziel
    QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100)); //gegnerspawn
    QMetaObject::invokeMethod(t_waveSpeed,"start",Q_ARG(int,5)); //gegnerbewegung

    if(account.username != "") {
        account.accountState = ACCOUNT_PENDING;
    }
    getStatus();
}

FrmMain::~FrmMain()
{
    if(active == STATE_PLAYING) {
        saveGameSave();
    }
    saveMaps();
    saveOptions();
    //QMetaObject::invokeMethod(t_draw,"stop");
    QMetaObject::invokeMethod(t_main,"stop");
    QMetaObject::invokeMethod(t_projectile,"stop");
    QMetaObject::invokeMethod(t_projectile_full,"stop");
    QMetaObject::invokeMethod(t_wave,"stop");
    QMetaObject::invokeMethod(t_waveSpeed,"stop");
    QMetaObject::invokeMethod(t_physics,"stop");
    QMetaObject::invokeMethod(t_menuAn,"stop");
    //workerThread->terminate();
    delete ui;
    delete world;
    delete pool;
}

bool FrmMain::isLoading()
{
    bool ok = false;
    for(uint i = 0; i < tiles.size(); i++) {
        if(tiles[i]->opacity < 1) {
            ok = true;
            break;
        }
    }
    return ok;
}

bool FrmMain::pathsContains(int num)
{
    bool ok = false;
    for(uint i = 0; i < paths.size(); i++) {
        if(paths[i].contains(num)) {
            ok = true;
            break;
        }
    }
    return ok;
}

void FrmMain::startRandomTile()
{
    bool isAvaiable = false;
    for(uint i = 0; i < tiles.size(); i++) {
        if(!tiles[i]->opacity) {
            isAvaiable = true;
            break;
        }
    }
    if(isAvaiable) {
        int found = -1;
        while(found == -1) {
            int pos = Engine::random(0,tiles.size());
            if(!tiles[pos]->opacity) found = pos;
        }
        tiles[found]->opacity = 0.1;
    }
}

void FrmMain::initPhysics()
{
    b2Vec2 gravity(0,0);
    world = new b2World(gravity);
}

void FrmMain::restore(int amount)
{
    setShekel(shop->shekel + amount);
}

void FrmMain::addTowerTargets(Tower *t)
{
    switch(t->type) {
    case TOWER_MINUS:
        t->addTargetDef(ENEMY_FLIESE,100);
        t->addTargetDef(ENEMY_NEUSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SPENDER,100);
        t->addTargetDef(ENEMY_ALTSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_MOD,100);
        t->addTargetDef(ENEMY_ADMIN,100);
        t->addTargetDef(ENEMY_LEGENDE,100);
        break;
    case TOWER_HERZ:
        t->addTargetDef(ENEMY_FLIESE,100);
        t->addTargetDef(ENEMY_NEUSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SPENDER,100);
        t->addTargetDef(ENEMY_ALTSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_MOD,100);
        t->addTargetDef(ENEMY_ADMIN,100);
        t->addTargetDef(ENEMY_LEGENDE,100);
        break;
    case TOWER_REPOST:
        t->addTargetDef(ENEMY_FLIESE,100);
        t->addTargetDef(ENEMY_NEUSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SPENDER,100);
        t->addTargetDef(ENEMY_ALTSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_MOD,100);
        t->addTargetDef(ENEMY_ADMIN,100);
        t->addTargetDef(ENEMY_LEGENDE,100);
        t->addTargetDef(ENEMY_GEBANNT,100);
        break;
    case TOWER_BAN:
        t->addTargetDef(ENEMY_FLIESE,100);
        t->addTargetDef(ENEMY_NEUSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SPENDER,100);
        t->addTargetDef(ENEMY_ALTSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_MOD,100);
        t->addTargetDef(ENEMY_ADMIN,100);
        t->addTargetDef(ENEMY_LEGENDE,100);
        t->addTargetDef(ENEMY_GEBANNT,100);
        break;
    case TOWER_SNIPER:
        t->addTargetDef(ENEMY_FLIESE,100);
        t->addTargetDef(ENEMY_NEUSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SPENDER,100);
        t->addTargetDef(ENEMY_ALTSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_MOD,100);
        t->addTargetDef(ENEMY_ADMIN,100);
        t->addTargetDef(ENEMY_LEGENDE,100);
        break;
    case TOWER_FLAK:
        t->addTargetDef(ENEMY_GEBANNT,100);
        break;
    case TOWER_LASER:
        t->addTargetDef(ENEMY_FLIESE,100);
        t->addTargetDef(ENEMY_NEUSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SPENDER,100);
        t->addTargetDef(ENEMY_ALTSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_MOD,100);
        t->addTargetDef(ENEMY_ADMIN,100);
        t->addTargetDef(ENEMY_LEGENDE,100);
        t->addTargetDef(ENEMY_GEBANNT,50);
        break;
    case TOWER_POISON:
        t->addTargetDef(ENEMY_FLIESE,100);
        t->addTargetDef(ENEMY_NEUSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SPENDER,100);
        t->addTargetDef(ENEMY_ALTSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_MOD,100);
        t->addTargetDef(ENEMY_ADMIN,100);
        t->addTargetDef(ENEMY_LEGENDE,100);
        t->addTargetDef(ENEMY_GEBANNT,50);
        break;
    case TOWER_MINIGUN:
        t->addTargetDef(ENEMY_FLIESE,100);
        t->addTargetDef(ENEMY_NEUSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SCHWUCHTEL,100);
        t->addTargetDef(ENEMY_SPENDER,100);
        t->addTargetDef(ENEMY_ALTSCHWUCHTEL,100);
        t->addTargetDef(ENEMY_MOD,100);
        t->addTargetDef(ENEMY_ADMIN,100);
        t->addTargetDef(ENEMY_LEGENDE,100);
        t->addTargetDef(ENEMY_GEBANNT,50);
        break;
    }
}

void FrmMain::upgradeDmg(Tower *tmpTower)
{
    switch(tmpTower->type) {
    case TOWER_MINUS: //minus
    case TOWER_BENIS: //benis
    case TOWER_LASER: //laser
        tmpTower->dmg += 10;
        break;
    case TOWER_HERZ: //fav
        tmpTower->stun += 1000;
        break;
    case TOWER_REPOST: //repost
        tmpTower->repost += 1000;
        break;
    case TOWER_SNIPER: //sniper
        tmpTower->dmg *= 2;
        break;
    case TOWER_FLAK: //flak
        tmpTower->dmg += 5;
        break;
    case TOWER_POISON: //poison
    case TOWER_MINIGUN:
        tmpTower->dmg += 0.25;
        break;
    }
}

void FrmMain::setShekel(unsigned long long amount, bool save, bool loading)
{
    shekelLocked = true;
    QString s = QString::number(amount + 11);
    shop->shekelSave = s;
    if(amount > shop->shekel && !loading) {
        statistics->earnedShekel += (amount-shop->shekel);
    }
    shop->shekel = amount;
    shekelLocked = false;
    if(save) {
        saveOptions();
    }
}

void FrmMain::setBenis(unsigned long long amount, bool loading)
{
    benisLocked = true;
    QString s = QString::number(amount + 11);
    benisSave = s;
    if(amount > benis && !loading) {
        statistics->earnedBenis += (amount-benis);
    }
    benis = amount;
    benisLocked = false;
}

void FrmMain::setBaseHp(int amount)
{
    basehplocked = true;
    QString s = QString::number(amount + 11);
    basehpSave = s;
    basehp = amount;
    basehplocked = false;
}

void FrmMain::setWaveCount(int amount)
{
    waveCountLocked = true;
    QString s = QString::number(amount + 11);
    waveCountSave = s;
    waveCount = amount;
    currentWave.waveCount = amount;
    if((unsigned long long)amount > statistics->highestWave) {
        statistics->highestWave = amount;
    }
    waveCountLocked = false;
}

void FrmMain::shake(int duration, double shakeIX, double shakeIY)
{
    shaking = duration;
    shakeIntensityX = shakeIX;
    shakeIntensityY = shakeIY;
    if(!t_shake->isActive()) QMetaObject::invokeMethod(t_shake,"start",Q_ARG(int,5));
}

void FrmMain::resetTimers()
{
    QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,100));
    QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,10));
    QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,10));
    QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100));
    QMetaObject::invokeMethod(t_grasAn,"start",Q_ARG(int,500));
}

void FrmMain::checkPush(b2Body *delBody)
{
    if(delBody==nullptr) return;
    bool ok=true;
    for(uint i=0;i<delBodies.size();i++) {
        if(delBodies[i]==delBody) {
            ok = false;
            break;
        }
    }
    if(ok) {
        delBodies.push_back(delBody);
    }
}

void FrmMain::checkPush(Tile *t)
{
    if(t == nullptr) return;
    bool ok = true;
    for(uint i = 0; i < createTileBodies.size(); i++) {
        if(createTileBodies[i] == t) {
            ok = false;
            break;
        }
    }
    if(ok) {
        createTileBodies.push_back(t);
    }
}

void FrmMain::getCollidingBits(int type, uint16_t &ownCategoryBits, uint16_t &collidingCategoryBits)
{
    ownCategoryBits = NORMAL_ENEMY;
    collidingCategoryBits = NORMAL_ENEMY | BOUNDARY;
    if(type == ENEMY_GEBANNT) {
        ownCategoryBits = FLYING_ENEMY;
        collidingCategoryBits = FLYING_ENEMY | BOUNDARY;
    }
}

void FrmMain::createDeathProjectiles(QRect rect, int circle)
{
    double oSpeed = 0.01;
    double maxSpeed = 0.025;
    if(circle == 1 || circle == 2) {
        int pxan = 3;
        for(uint i = 0; i < 6; i++) {
            double speed = maxSpeed;
            int angle;
            switch(i+1) {
            case 2:
            case 3:
                angle = 180;
                break;
            case 4:
                angle = 90;
                break;
            case 1:
            case 5:
                angle = 0;
                break;
            case 6:
                angle = 270;
                break;
            }
            double vx = 0, vy = 0;
            angle -= 90;
            vx = qCos(qDegreesToRadians((double)angle));
            vy = qSin(qDegreesToRadians((double)angle));
            if(i+1 == 1 || i+1 == 2) {
                speed *= 0.5;
            }
            Projectile *p = pool->getProjectile();
            p->init(rect,
                    0,
                    0,
                    nullptr,
                    vx,
                    vy,
                    speed,
                    0,
                    0,
                    pxan);
            p->opacity = 1;
            p->opacityDecAm = oSpeed;
            p->type = 4;
            projectiles.push_back(p);
            pxan++;
        }
    } else {
        int pxan = 15;
        for(uint i = 0; i < 6; i++) {
            double speed = maxSpeed;
            int angle = 270;
            switch(i) {
            case 1:
                angle = 225;
                break;
            case 2:
                angle = 135;
                break;
            case 3:
                angle = 0;
                break;
            case 4:
                angle = 90;
                break;
            case 5:
                angle = 315;
                break;
            }
            double vx = 0, vy = 0;
            angle -= 90;
            vx = qCos(qDegreesToRadians((double)angle));
            vy = qSin(qDegreesToRadians((double)angle));
            if(i == 5) {
                speed /= 2;
            }
            Projectile *p = pool->getProjectile();
            p->init(rect,
                    0,
                    0,
                    nullptr,
                    vx,
                    vy,
                    speed,
                    0,
                    0,
                    pxan);
            p->opacity = 1;
            p->opacityDecAm = oSpeed;
            p->type = 4;
            projectiles.push_back(p);
            pxan++;
        }
    }
}

void FrmMain::createBounds()
{
    do {
        QMetaObject::invokeMethod(t_physics,"stop");
    } while(t_physics->isActive());

    b2BodyDef def;
    def.type = b2_staticBody;
    def.position.Set(Engine::numToBox2D(mapwidth/2),Engine::numToBox2D(-2));
    def.angle = 0;
    wall1 = world->CreateBody(&def);
    b2PolygonShape boxShape;
    boxShape.SetAsBox(Engine::numToBox2D(mapwidth/2),Engine::numToBox2D(1));
    b2FixtureDef fixDef;
    fixDef.shape = &boxShape;
    fixDef.filter.categoryBits = BOUNDARY;
    fixDef.filter.maskBits = NORMAL_ENEMY | FLYING_ENEMY;
    wall1->CreateFixture(&fixDef);

    //2

    def.position.Set(Engine::numToBox2D(-2),Engine::numToBox2D(mapheight/2));
    wall2 = world->CreateBody(&def);
    boxShape.SetAsBox(Engine::numToBox2D(1),Engine::numToBox2D(mapheight/2));
    fixDef.shape = &boxShape;
    wall2->CreateFixture(&fixDef);

    //3

    def.position.Set(Engine::numToBox2D(mapwidth/2),Engine::numToBox2D(mapheight));
    wall3 = world->CreateBody(&def);
    boxShape.SetAsBox(Engine::numToBox2D(mapwidth/2),Engine::numToBox2D(1));
    fixDef.shape = &boxShape;
    wall3->CreateFixture(&fixDef);

    //4

    def.position.Set(Engine::numToBox2D(mapwidth),Engine::numToBox2D(mapheight/2));
    wall4 = world->CreateBody(&def);
    boxShape.SetAsBox(Engine::numToBox2D(1),Engine::numToBox2D(mapheight/2));
    fixDef.shape = &boxShape;
    wall4->CreateFixture(&fixDef);

    QMetaObject::invokeMethod(t_physics,"start");
}

void FrmMain::on_tdraw()
{
    update();
}

void FrmMain::on_tmain()
{
    if(exit) this->close();

    if(dailyReward->active) {
        dailyReward->anY++;
        if(dailyReward->anY > 1793) {
            dailyReward->anY2++;
        }
        if(dailyReward->anY > 2513) {
            dailyReward->anY = 0;
            dailyReward->anY2 = -720;
        }
        return;
    }

    if(newPost && active == STATE_MAINMENU) {
        newPost = false;
        newPostInfo();
    }

    if(shekelCoinSize) {
        shekelCoinSize -= 0.2;
        if(shekelCoinSize <= 0) shekelCoinSize = 0;
    }

    if(active == STATE_MAINMENU || active == STATE_EDITOR) return;

    int num = -1;
    if(towerMenu > -1) {
        num = towerMenu;
    } else if(towerMenuAnimating > -1) {
        num = towerMenuAnimating;
    }

    if(spawnTiles) {
        for(int i = 0; i < 10; i++) {
            startRandomTile();
        }
        for(uint i = 0; i < tiles.size(); i++) {
            if(tiles[i]->opacity < 1 && tiles[i]->opacity > 0) {
                tiles[i]->opacity += 0.025;
                if(tiles[i]->opacity > 1) tiles[i]->opacity = 1;
            }
        }
        if(!isLoading()) spawnTiles = false;
    }

    if(towerMenuAnimating >= 0) {
        double anSpeed = 50;
        if(!towerMenuAnimatingDir) { //öffnen
            towerMenuRectAnimation.moveTo(
                        towerMenuRectAnimation.x() + anSpeed,
                        towerMenuRectAnimation.y());
            if(towerMenuRectAnimation.x() >= towerMenuRect.x()) {
                if(!towerMenuHidden) {
                    towerMenu = towerMenuAnimating;
                }
                towerMenuAnimating = -1;
            }
        } else { //schließen
            towerMenuRectAnimation.moveTo(
                        towerMenuRectAnimation.x() - anSpeed,
                        towerMenuRectAnimation.y());
            if(towerMenuRectAnimation.x() <= -600) {
                towerMenuRectAnimation = QRect(-600, towerMenuRectAnimation.y(),
                                           towerMenuRect.width(), towerMenuRect.height());
                towerMenuAnimating = -1;
            }
        }
    }

    if(gamePaused) return;
    if(DEBUG) {
        timerM.restart();
    }
    try {

        //mutex.lock();
        for(uint i=0;i<towers.size();i++) {
            if(!towers[i]->disabled) {
                double targetAngle = towers[i]->targetAngle;
                double ownAngle = towers[i]->angle;
                double angleSpeed = towers[i]->angleSpeed;
                if(!Engine::isEqual(ownAngle,targetAngle,1) &&
                        towers[i]->angleSpeed) {
                    bool turningDir = Engine::getTurnSide(ownAngle,targetAngle);
                    double aDiff = Engine::getAngleDifference(ownAngle,targetAngle);
                    if(towers[i]->target != nullptr) {
                        if(aDiff >= 150) {
                            QRect towerRect = towers[i]->rect();
                            QRect targetRect = towers[i]->target->rect();
                            switch(towers[i]->target->dir) {
                            case 1: //rechts
                                if(towerRect.center().y() < targetRect.center().y()) {
                                    //Tower liegt über Gegner
                                    turningDir = true;
                                } else {
                                    turningDir = false;
                                }
                                break;
                            case 3: //links
                                if(towerRect.center().y() > targetRect.center().y()) {
                                    //Tower liegt unter Gegner
                                    turningDir = false;
                                } else {
                                    turningDir = true;
                                }
                                break;
                            case 2: //oben
                                if(towerRect.center().x() < targetRect.center().x()) {
                                    //Tower liegt links von Gegner
                                    turningDir = true;
                                } else {
                                    turningDir = false;
                                }
                                break;
                            case 4: //unten
                                if(towerRect.center().x() > targetRect.center().x()) {
                                    //Tower liegt rechts von Gegner
                                    turningDir = false;
                                } else {
                                    turningDir = true;
                                }
                                break;
                            }
                        }
                    }
                    if(turningDir) { //rechtsrum
                        ownAngle += angleSpeed;
                        if(!Engine::getTurnSide(ownAngle,targetAngle) && aDiff < 2) {
                            ownAngle = targetAngle;
                        }
                    } else { //linksrum
                        ownAngle -= angleSpeed;
                        if(Engine::getTurnSide(ownAngle,targetAngle) && aDiff < 2) {
                            ownAngle = targetAngle;
                        }
                    }
                    if(ownAngle >= 360) ownAngle -= 360;
                    while(ownAngle < 0) {
                        ownAngle += 360;
                    }
                    towers[i]->angle = ownAngle;
                }
                QPointF tCenter = towers[i]->pos.center();
                //Check if shooting
                if(towers[i]->currentReload <= 0) {
                    Enemy *target = towers[i]->target;
                    if(target != nullptr || towers[i]->type == TOWER_REPOST ||
                            towers[i]->type == TOWER_BENIS ||
                            towers[i]->type == TOWER_BAN) {
                        bool shotFired = false;
                        if(Engine::isEqual(targetAngle,ownAngle,2) && (towers[i]->type < TOWER_REPOST ||
                                                                     towers[i]->type == TOWER_SNIPER ||
                                                                       towers[i]->type == TOWER_FLAK)) { //Minus, Herz, Sniper
                            Projectile *p = pool->getProjectile();
                            QPointF enemy = target->rect().center();
                            int angle = Engine::getAngle(tCenter,enemy);
                            QRectF rect;
                            QColor color = QColor(1,1,1,1);
                            switch(towers[i]->type) {
                            case TOWER_MINUS: //minus
                                rect = QRectF(tCenter.x()-7.5,tCenter.y()-7.5,15,15);
                                break;
                            case TOWER_SNIPER: {//sniper
                                rect = QRectF(tCenter.x() - 5,tCenter.y()-10,7,7);
                                QTransform transform =
                                        QTransform().translate(tCenter.x(),tCenter.y()).
                                        rotate(towers[i]->angle).
                                        translate(-tCenter.x(),-tCenter.y());
                                rect = transform.map(rect).boundingRect();
                                color = QColor(47,237,27,100);
                                break; }
                            case TOWER_FLAK: {//flak
                                switch(towers[i]->side) {
                                case 0:
                                    rect = QRectF(tCenter.x()-25,tCenter.y()-7,10,10);
                                    //rect.
                                    towers[i]->side++;
                                    break;
                                default:
                                    rect = QRectF(tCenter.x()+15,tCenter.y()-7,10,10);
                                    towers[i]->side = 0;
                                    break;
                                }
                                QTransform transform =
                                        QTransform().translate(tCenter.x(),tCenter.y()).
                                        rotate(towers[i]->angle).
                                        translate(-tCenter.x(),-tCenter.y());
                                rect = transform.map(rect).boundingRect();
                                color = QColor(247,210,100);
                                break; }
                            case TOWER_HERZ: //herz
                                rect = QRectF(tCenter.x()-10,tCenter.y()-10,20,20);
                                break;
                            }
                            ownAngle -= 90;
                            while(ownAngle<0) {
                                ownAngle += 360;
                            }
                            p->init(rect,
                                    angle,
                                    towers[i]->dmg * (towers[i]->getTargetEfficiency(target->type)/100.0),
                                    target,
                                    qCos(qDegreesToRadians(ownAngle)),
                                    qSin(qDegreesToRadians(ownAngle)),
                                    towers[i]->pspeed,
                                    towers[i]->stun,
                                    0,
                                    towers[i]->projectilePxID);
                            if(towers[i]->pos.intersects(viewRect)) {
                                //shake(5, -p->vx, -p->vy);
                            }
                            p->color = color;
                            QRectF adjusted = towers[i]->pos;
                            adjusted.adjust(10,10,-10,-10);
                            while(p->crect().intersects(adjusted)) {
                                //Updatet solange bis Projektil weg vom Turm
                                p->update();
                            }

                            target->preHealth -= p->dmg;
                            if(target->preHealth<0) target->preHealth = 0;
                            projectiles.push_back(p);
                            shotFired = true;
                        } else if(towers[i]->type == TOWER_REPOST){ //range attack -> alle in bereich betroffen
                            for(uint b=0;b<enemys.size();b++) {
                                if(enemys[b]->physicsInitialized&&!enemys[b]->soonBanned) {
                                    double distance = Engine::getDistance(tCenter,enemys[b]->rectF().center());
                                    if(distance<towers[i]->range) {
                                        enemys[b]->setRepost(towers[i]->repost);
                                        shotFired = true;
                                    }
                                }
                            }
                            if(shotFired) {
                                towers[i]->animation = 9;
                            }
                        } else if(towers[i]->type == TOWER_BENIS &&
                                  Engine::isEqual(ownAngle, targetAngle, 2)) {//benistower
                            setBenis(benis + towers[i]->dmg);
                            for(int a=0;a<10;a++) {
                                Projectile *p = pool->getProjectile();
                                int angle = Engine::random(0,361);
                                p->init2(QRectF(tCenter.x()-10,tCenter.y()-10,20,20),
                                         1,
                                         angle,
                                         qCos(qDegreesToRadians((double)angle)),
                                         qSin(qDegreesToRadians((double)angle)),
                                         0.01,
                                         0.35,
                                         2);
                                projectiles.push_back(p);
                                shotFired = true;
                            }
                        } else if(towers[i]->type == TOWER_BAN && enemys.size()) { //banntower
                            Enemy *target = towers[i]->target;
                            int ok=true;
                            if(target == nullptr || !target->physicsInitialized) {
                                ok = false;
                                uint remaining = enemys.size();
                                while (!ok) {
                                    int num;
                                    num = Engine::random(0,(int)enemys.size());
                                    if(enemys[num]->preHealth==enemys[num]->maxHealth) {
                                        target = enemys[num];
                                        ok = true;
                                        break;
                                    } else {
                                        remaining--;
                                        if(remaining<=0) break;
                                    }
                                }
                                if(ok>0) {
                                    target->stunned = 99999;
                                    target->soonBanned = true;
                                    target->body->SetLinearDamping(0.05);
                                    towers[i]->target = target;
                                    towers[i]->shotsFiredSinceReload = 0;
                                }
                            } else {
                                if(!towers[i]->target) ok = -2;
                                if(target->preHealth<=0) ok = false;
                                if(!ok) {
                                    towers[i]->target = nullptr;
                                    towers[i]->currentReload = towers[i]->reload;
                                    towers[i]->shotsFiredSinceReload = 0;
                                }
                            }
                            if(ok>0) {
                                //Schuss
                                towers[i]->sellingDisabled = true;
                                int dmg = target->maxHealth/10;
                                if(dmg<1) dmg = 1;
                                //target->soonBanned = 1;
                                //if(towers[i]->shotsFiredSinceReload==9) dmg = target->maxHealth;
                                target->preHealth -= dmg;
                                QPointF enemy = target->rect().center();
                                int angle = Engine::getAngle(tCenter,enemy);
                                if(towers[i]->shotsFiredSinceReload==9) {
                                    angle += 180;
                                } else {
                                    angle = Engine::random(0,360);
                                }
                                if(angle>360) angle -= 360;
                                Projectile *p = pool->getProjectile();
                                p->init(QRectF(tCenter.x()-10,tCenter.y()-10,20,20),
                                        angle,
                                        dmg,
                                        target,
                                        qCos(qDegreesToRadians((double)angle)),
                                        qSin(qDegreesToRadians((double)angle)),
                                        1.0,
                                        towers[i]->stun,
                                        0,
                                        towers[i]->projectilePxID);
                                if(towers[i]->pos.intersects(viewRect)) {
                                    //shake(5, -p->vx, -p->vy);
                                }
                                projectiles.push_back(p);
                                //--
                                towers[i]->shotsFiredSinceReload++;
                                if(towers[i]->shotsFiredSinceReload<10) {
                                    towers[i]->currentReload = 50;
                                } else {
                                    towers[i]->shotsFiredSinceReload = 0;
                                    towers[i]->currentReload = towers[i]->reload;
                                    towers[i]->target = nullptr;
                                    towers[i]->sellingDisabled = false;
                                }
                            }
                        } else if(towers[i]->type == TOWER_LASER &&
                                  Engine::isEqual(ownAngle, targetAngle, 2)) { //Lasertower
                            if(!towers[i]->isShooting) {
                                towers[i]->isShooting = true;
                            }
                            if(towers[i]->shotsFiredSinceReload) {
                                if(account.accountState == ACCOUNT_CHEATER) {
                                    handleCheater(towers[i]->target->rect());
                                } else {
                                    double dmg = towers[i]->getDmg(towers[i]->target->type);
                                    towers[i]->target->preHealth -= dmg / 10.0;
                                    if(towers[i]->target->preHealth < 0 ) towers[i]->target->preHealth = 0;
                                    towers[i]->target->reduceHealth(dmg / 10.0);
                                }
                            }
                            towers[i]->shotsFiredSinceReload++;
                            if(towers[i]->shotsFiredSinceReload > 10) {
                                towers[i]->shotsFiredSinceReload = 0;
                                towers[i]->currentReload = towers[i]->reload;
                                towers[i]->isShooting = false;
                                towers[i]->target = nullptr;
                            } else {
                                towers[i]->currentReload = 250;
                            }
                        } else if(towers[i]->type == TOWER_POISON &&
                                  Engine::isEqual(ownAngle, targetAngle, 2)) { //Gifttower
                            if(!towers[i]->isShooting) {
                                towers[i]->isShooting = true;
                            }
                            //gift
                            towers[i]->shotsFiredSinceReload++;
                            if(towers[i]->shotsFiredSinceReload > 50) {
                                towers[i]->currentReload = towers[i]->reload;
                                towers[i]->shotsFiredSinceReload = 0;
                            } else {
                                //Gift-Schuss
                                double angle = ownAngle - 90;
                                while(angle<0) {
                                    angle += 360;
                                }
                                angle -= 30;
                                QRectF rect = QRectF(towers[i]->pos.x()+30,towers[i]->pos.y()-10,20,20);
                                QTransform transform =
                                        QTransform().translate(tCenter.x(),tCenter.y()).
                                        rotate(towers[i]->angle).
                                        translate(-tCenter.x(),-tCenter.y());
                                rect = transform.map(rect).boundingRect();
                                for(uint a=0;a<6;a++) {
                                    Projectile *p = pool->getProjectile();
                                    p->init2(rect,
                                            3,
                                            angle,
                                            qCos(qDegreesToRadians((double)angle)),
                                            qSin(qDegreesToRadians((double)angle)),
                                            0.020,
                                            1.75,
                                            0,
                                            1);
                                    p->poisonDmg = towers[i]->dmg;
                                    p->targetDefinitions = towers[i]->targetTypes;
                                    projectiles.push_back(p);
                                    angle += 10;
                                    if(angle > 360) angle -= 360;
                                }
                                towers[i]->currentReload = 50;
                            }
                        } else if(towers[i]->type == TOWER_MINIGUN &&
                                  Engine::isEqual(ownAngle,targetAngle,2)) {
                            towers[i]->animation++;
                            if(towers[i]->animation > towers[i]->animationMax) towers[i]->animation = 0;
                            QRectF rect = QRectF(towers[i]->pos.x()+37.5,towers[i]->pos.y()-2.5,5,5);
                            QTransform transform =
                                    QTransform().translate(tCenter.x(),tCenter.y()).
                                    rotate(towers[i]->angle).
                                    translate(-tCenter.x(),-tCenter.y());
                            rect = transform.map(rect).boundingRect();
                            Projectile *p = pool->getProjectile();
                            double angle = ownAngle;
                            angle -= 90;
                            while(angle<0) {
                                angle += 360;
                            }
                            p->init(rect,
                                    angle,
                                    towers[i]->getDmg(target->type),
                                    target,
                                    qCos(qDegreesToRadians((double)angle)),
                                    qSin(qDegreesToRadians((double)angle)),
                                    towers[i]->pspeed,
                                    0,
                                    0,
                                    towers[i]->projectilePxID);
                            if(towers[i]->pos.intersects(viewRect)) {
                                //shake(5, -p->vx/2, -p->vy/2);
                            }
                            projectiles.push_back(p);
                            towers[i]->shotsFiredSinceReload++;
                            int max = 5;
                            if(towers[i]->shotsFiredSinceReload < max) {
                                towers[i]->currentReload = (max - towers[i]->shotsFiredSinceReload) * (0.1*towers[i]->reload);
                            } else {
                                towers[i]->currentReload = (0.1*towers[i]->reload);
                            }
                        }
                        if(towers[i]->type != TOWER_BAN &&
                                towers[i]->type != TOWER_LASER &&
                                towers[i]->type != TOWER_POISON &&
                                towers[i]->type != TOWER_MINIGUN && shotFired) {
                            towers[i]->currentReload = towers[i]->reload;
                        }
                    }
                } else {
                    if(towers[i]->currentReload>0 && !towers[i]->disabled) towers[i]->currentReload -= 10;
                }
            } else { //tower deaktiviert
                if(towers[i]->disabled < disabledTime && towers[i]->disabled) {
                    towers[i]->disabled -= 10;
                    if(towers[i]->disabled <= 0) {
                        towers[i]->disabled = 0;
                        towers[i]->sellingDisabled = false;
                        towers[i]->disabledFlagged = false;
                    }
                }
            }
        }
        for(uint i=0;i<enemys.size();i++) {
            if(enemys[i]->type == ENEMY_GEBANNT) {//wenn fliegend dann rotor drehen
                enemys[i]->animation += 5;
                if(enemys[i]->animation > 360) enemys[i]->animation = 0;
            }

            if(enemys[i]->type == ENEMY_LEGENDE) {
                if(enemys[i]->reload > 0) {
                    enemys[i]->reload -= 10;
                    if(enemys[i]->reload < 0) enemys[i]->reload = 0;
                } else {
                    int min = mapwidth*mapheight;
                    int num = -1;
                    for(uint a = 0; a < towers.size(); a++) {
                        if(!towers[a]->disabled && !towers[a]->disabledFlagged) {
                            int tmpDistance = Engine::getDistance(enemys[i]->rectF().center(), QPointF(towers[a]->rect().center()));
                            if(tmpDistance < min) {
                                min = tmpDistance;
                                num = a;
                            }
                        }
                    }
                    if(num > -1) {
                        Projectile *p = pool->getProjectile();
                        enemys[i]->reload = enemys[i]->maxReload;
                        int angle = Engine::getAngle(enemys[i]->rectF().center(),
                                                     QPointF(towers[num]->rect().center()));
                        QRectF targetRect = QRectF(towers[num]->rect().center().x()-5, towers[num]->rect().center().y()-5,
                                                   10,10);
                        towers[num]->disabledFlagged = true;
                        towers[num]->sellingDisabled = true;
                        p->rect = enemys[i]->rectF();
                        p->targetRect = targetRect;
                        p->rect.adjust(10,10,-10,-10);
                        p->opacity = 0;
                        p->opacityDecAm = 0;
                        p->vel = 2;
                        p->angle = angle;
                        p->vx = qCos(qDegreesToRadians((double)angle));
                        p->vy = qSin(qDegreesToRadians((double)angle));
                        p->pxID = Engine::random(0,blitzAnimation.size());
                        p->type = 8;
                        p->isUsed = true;
                        p->del = false;
                        //p->init_arc(enemys[i]->rectF(), 3, 1, 0.05, Qt::blue, 5);
                        projectiles.push_back(p);
                    }
                }
            }

            if(enemys[i]->health <= 0.01) {//wenn tot dann löschen
                //benis += enemys[i]->price;
                QPoint point = enemys[i]->rect().center();
                Projectile *p = pool->getProjectile();
                QPoint newPoint(((point.x()+transX)*scaleX)/scaleFHDX,
                        ((point.y()+transY)*scaleY)/scaleFHDY);
                int angle = Engine::getAngle(QPointF(newPoint), pr0coinRect.center());
                p->init3(QString::number(enemys[i]->price),
                         QPoint(newPoint.x(),newPoint.y()),
                         angle,
                         qCos(qDegreesToRadians((double)angle)),
                         qSin(qDegreesToRadians((double)angle)),
                         0,
                         10);
                p->dmg = enemys[i]->price;
                p->hasShekelImage = true;
                p->type = 5;
                p->rect.setWidth((enemys[i]->rect().width())*scaleX);
                projectiles.push_back(p);

                int circle = 1;
                if(enemys[i]->type == ENEMY_GEBANNT) circle = 2;
                if(enemys[i]->type == ENEMY_LEGENDE) circle = 3;
                createDeathProjectiles(enemys[i]->rect(), circle);
                statistics->kills[enemys[i]->type]++;
                delEnemy(i);
            }
        }
        for(uint i = 0; i < tiles.size(); i++) {
            if(tiles[i]->type == 7) {//Wand
                if(tiles[i]->wallTime > 0) {
                    tiles[i]->wallTime -= 10;
                } else {
                    bool del = true;
                    if(!pathsContains(i)) {
                        //Wenn i nicht in einem pfad
                        int maxX = mapwidth/80;
                        int size = tiles.size();
                        int left = i-1;
                        int right = i+1;
                        int up = i-maxX;
                        int down = i+maxX;
                        if((left/maxX == (int)i/maxX) && //selbe zeile
                                left > -1 && left < size) {
                            if(!pathsContains(left)) del = false; //wenn links pfad NICHT löschen
                        }

                        if((right/maxX == (int)i/maxX) &&
                                right > -1 && right < size) {
                            if(!pathsContains(right)) del = false;
                        }

                        if(up > -1 && up < size) {
                            if(!pathsContains(up)) del = false;
                        }

                        if(down > -1 && down < size) {
                            if(!pathsContains(down)) del = false;
                        }
                    }

                    if(del) {
                        b2Body *body = tiles[i]->body;
                        tiles[i]->body = nullptr;
                        tiles[i]->type = 1;
                        delBodies.push_back(body);
                    }
                }
            }
        }
        //mutex.unlock();
        if(DEBUG) {
            tmainMS = timerM.nsecsElapsed()/NANO_TO_MILLI;
        }
    } catch(std::exception& e) {
        qDebug()<<e.what();
    }
}

void FrmMain::on_tprojectile()
{
    //qDebug()<<projectiles.size();
    if(active == STATE_EDITOR || (gamePaused && backMenu) || (gamePaused && loaded) || moveAn == ANIMATION_BACK) return;
    if(DEBUG) {
        if(output) {
            qDebug()<<"projectile";
        }
        timerP.restart();
    }
    try {
        mutex.lock();
        for(uint i=0;i<projectiles.size();i++) {
            int del = calcProjectiles(i);
            if(del) {
                projectiles[i]->free();
                projectiles.erase(projectiles.begin()+i);
            }
        }
        mutex.unlock();
    } catch(std::exception& e) {
        qDebug()<<e.what();
    }
    if(DEBUG) {
        tprojectileMS = timerP.nsecsElapsed()/NANO_TO_MILLI;
        if(output) {
            qDebug()<<"projectile-end";
        }
    }
}

void FrmMain::on_tprojectilefull()
{
    if(active == STATE_EDITOR || gamePaused) return;
    if(DEBUG) {
        if(output) {
            qDebug()<<"projectile2";
        }
        timerPF.restart();
    }
    try {
        mutex.lock();
        for(uint i=0;i<projectiles.size();i++) {
            if(!projectiles[i]->del) {
                //mutex.lock();
                projectiles[i]->update(true);
                //mutex.unlock();
            }
        }
        mutex.unlock();
    } catch(std::exception& e) {
        qDebug()<<e.what();
    }
    if(DEBUG) {
        tprojectileFMS = timerPF.nsecsElapsed()/NANO_TO_MILLI;
        if(output) {
            qDebug()<<"projectile2-end";
        }
    }
}

void FrmMain::on_twave()
{
    try {

        if((enemys.size()&&mapPlaying)||!mapLoaded||
                gamePaused || active == STATE_MAINMENU || (active == STATE_EDITOR && !mapPlaying)||
                isGameOver||!playingSpeed) return;
        if(paths.size() > 1) {
            chosenpath = Engine::random(0,paths.size());
        }
        if(waveBreak) return;

        QRect knot = tiles[paths[chosenpath].path[0]]->rect();
        int width = 0;
        int pxID = Engine::random(0,normalEnemys.size());
        double health = 0;
        int price = 0;
        int type = 0;
        double speed = 0;
        double diff = 1;
        uint16_t ownCategoryBits, collidingCategoryBits;
        if(currentWave.waveCount > 20) {
            diff = 1.05 * (currentWave.waveCount / 20);
        }
        bool flak = false;
        if(!shop->towerLocks[6] && !shop->towerPrices[6]) flak = true;

        if(enemys.size() > (paths[chosenpath].path.size()*3)) return;

        if(enemys.size() > 125) return;

        type = currentWave.generateEnemyType(flak);
        getCollidingBits(type, ownCategoryBits, collidingCategoryBits);
        switch(type) {
        case ENEMY_FLIESE:
            health = 25*diff;
            speed = 1;
            price = 5;
            break;
        case ENEMY_NEUSCHWUCHTEL:
            health = 30*diff;
            speed = 0.80;
            price = 10;
            break;
        case ENEMY_SCHWUCHTEL:
            health = 40*diff;
            speed = 0.55;
            price = 15;
            break;
        case ENEMY_SPENDER:
            health = 50*diff;
            speed = 0.65;
            price = 20;
            break;
        case ENEMY_ALTSCHWUCHTEL:
            health = 70*diff;
            speed = 0.45;
            price = 20;
            break;
        case ENEMY_MOD:
            health = 100*diff;
            speed = 0.40;
            price = 30;
            break;
        case ENEMY_ADMIN:
            health = 100*diff;
            speed = 0.35;
            price = 30;
            break;
        case ENEMY_LEGENDE:
            health = 200*diff;
            speed = 0.3;
            price = 40;
            pxID = 0;
            break;
        case ENEMY_GEBANNT:
            health = 25*diff;
            speed = 1.75;
            price = 5;
            break;
        }
        width = getEnemySizeByType(type)*0.75;
        QRectF r;
        std::vector <int> enemysAtSpawn;
        for(uint i=0;i<enemys.size();i++) {
            if((enemys[i]->cpos < 2 && type != ENEMY_LEGENDE) ||
                    (enemys[i]->cpos < 3 && type == ENEMY_LEGENDE))
                enemysAtSpawn.push_back(i);
        }

        double size = 12800;
        for(uint i=0;i<enemysAtSpawn.size();i++) {
            double es = getEnemySizeByType(enemys[enemysAtSpawn[i]]->type);
            if((enemys[enemysAtSpawn[i]]->type == ENEMY_GEBANNT && type == ENEMY_GEBANNT) ||
                    (type != ENEMY_GEBANNT && enemys[enemysAtSpawn[i]]->type != ENEMY_GEBANNT)) {
                size -= es*es;
            }
        }
        if(size < 0) return;

        if(enemysAtSpawn.size() && type == ENEMY_LEGENDE) return;

        if(width /*>= 40*/) {
            r = QRectF(knot.x()+((80-width)/2)+width/2,
                 knot.y()+((80-width)/2)+width/2,
                 width,width);
        } else {
            double x = (double)Engine::random(knot.x(),(knot.x()+knot.width())-width);
            double y = (double)Engine::random(knot.y(),(knot.y()+knot.height())-width);
            r = QRectF(x,y,width,width);
        }
        r.moveTo(r.x()-r.width()/2, r.y()-r.height()/2);
        if(playingSpeed && !enemys.size() && !currentWave.enemysPerWave) {
            /*if(playingSpeed >= 2) {
                QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,50));
                QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,10));
                QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,10));
                QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100));
            }*/
            //playingSpeed = 0;
            //gamePaused = true;
            if(mapID != 2) {
                if(playingMode == MODE_NORMAL) {
                    if(!shop->lvlprices[mainLevels-1]) {
                        setShekel(shop->shekel + currentWave.calcShekel());
                    }
                } else if(playingMode == MODE_MAZE) {
                    //if(!shop->lvlprices[mainLevels-1]) {
                        setShekel(shop->shekel + currentWave.calcShekel());
                    //}
                }
            }
            waveBreak = 5000;
            setWaveCount(currentWave.waveCount + 1);
            currentWave.updateEnemysPerWave(false, mapwidth/80);
            currentWave.resetWave();
            if(currentWave.waveCount == 1) statistics->playedGames++;
            playedGames++;
            /*if(waveCount%10!=0&&waveCount) {
                internalWaveCount++;
            } else {
                internalWaveCount = 0;
            }
            if(waveCount%5==0&&waveCount) {
                chosenpath++;
                if(chosenpath > paths.size()-1) {
                    chosenpath = 0;
                }
            }
            if(waveCount==100) {
                benis = 0;
                waveCount = 0;
                internalWaveCount = 0;
            }*/
            return;
        } else if(playingSpeed && enemys.size()) {
            currentWave.waveTime += 100;
        }
        if(currentWave.enemysPerWave) {
            currentWave.enemysPerWave--;
            if(!currentWave.enemysPerWave) {
                canStartNextWave = true;
            }
        } else {
            return;
        }
        currentWave.addEnemy(type);
        Enemy *e = pool->getEnemy();//new Enemy(knot,0);
        e->init(r,speed,type,0,health,chosenpath,price);
        if(type == ENEMY_LEGENDE) {
            e->name = bossNames[Engine::random(0,bossNames.size())];
            e->maxReload = 5000;
            e->reload = e->maxReload;
        }
        e->calcWidth();
        e->imgID = pxID;
        e->ownCategoryBits = ownCategoryBits;
        e->collidingCategoryBits = collidingCategoryBits;
        int dir = 1;
        if(paths[chosenpath].path[0]-1==paths[chosenpath].path[1]) { //links
            dir = 3;
        } else if(paths[chosenpath].path[0]+(mapwidth/80)==paths[chosenpath].path[1]) { //unten
            dir = 2;
        } else if(paths[chosenpath].path[0]-(mapwidth/80)==paths[chosenpath].path[1]) { //oben
            dir = 4;
        }
        e->dir = dir;
        //mutex.lock();
        createBodies.push_back(e);
        //mutex.unlock();
    } catch(std::exception& e) {
        qDebug()<<e.what();
    }
}

void FrmMain::on_twaveSpeed()
{
    if(isZooming) {
        if(isZooming == 1) {//größer
            zoom(1+(zoomAmount*((double)zoomTime/maxZoomTime)));
        } else { //kleiner
            zoom(1-(zoomAmount*((double)zoomTime/maxZoomTime)));
        }
        zoomTime--;
        if(!zoomTime) {
            isZooming = false;
        }
    }

    if(active == STATE_PLAYING) {
        for(uint i = 0; i < towers.size(); i++) {
            Tower *t = towers[i];
            int range = t->tmpRange;
            int maxrange = t->range;
            if(t->tmpRangeDir == 0) {//größer
                if(range < maxrange) {
                    range += 5;
                    if(range >= maxrange) {
                        range = maxrange;
                        t->tmpRangeDir = 2;
                    }
                }
            } else if(t->tmpRangeDir == 1){ //kleiner
                if(range > 0) {
                    range -= 5;
                    if(range <= 0) {
                        range = 0;
                        t->tmpRangeDir = 2;
                    }
                }
            }
            t->tmpRange = range;
        }
    }

    if(gamePaused) return;

    try {
        if((active == STATE_PLAYING || active == STATE_EDITOR) && mapLoaded) {
            if(DEBUG) {
                timerWS.restart();
            }

            //evtl irgendwo / 0 teilen?
            for(uint i=0;i<enemys.size();i++) {
                enemys[i]->updateWidth(0.3 * playingSpeed);
                if(enemys[i]->flash) {
                    enemys[i]->flash -= 1*playingSpeed;
                    if(enemys[i]->flash < 0) enemys[i]->flash = 0;
                }
                int next = enemys[i]->cpos+1;
                //mutex.lock();
                if(enemys[i]->repost) {
                    enemys[i]->repost -= 5 * playingSpeed;
                    if(enemys[i]->repost < 0) enemys[i]->repost = 0;
                }
                if(enemys[i]->poison) {
                    enemys[i]->poison -= 5 * playingSpeed;
                    if(enemys[i]->poison < 0) enemys[i]->poison = 0;
                }
                if(enemys[i]->path>-1) {
                    if(next < (int)paths[enemys[i]->path].path.size()) {
                        QRect knot = tiles[paths[enemys[i]->path].path[next]]->rect();
                        knot.adjust(20,20,-20,-20);
                        if(paths[enemys[i]->path].path[next-1]+1==paths[enemys[i]->path].path[next]) { //rechts
                            enemys[i]->newdir = 1;
                        } else if(paths[enemys[i]->path].path[next-1]-1==paths[enemys[i]->path].path[next]) { //links
                            enemys[i]->newdir = 3;
                        } else if(paths[enemys[i]->path].path[next-1]+(mapwidth/80)==paths[enemys[i]->path].path[next]) { //unten
                            enemys[i]->newdir = 2;
                        } else if(paths[enemys[i]->path].path[next-1]-(mapwidth/80)==paths[enemys[i]->path].path[next]) { //oben
                            enemys[i]->newdir = 4;
                        }
                        if((enemys[i]->stunned && !enemys[i]->soonBanned) || enemys[i]->blocked ||
                                (enemys[i]->type != ENEMY_LEGENDE && getLegendenDistanz(i) <= (enemys[i]->pos.width()*8))) {
                            if((enemys[i]->speed > (0.5*enemys[i]->ospeed) && !enemys[i]->blocked) ||
                                    (enemys[i]->speed > (0.5*enemys[i]->ospeed) && enemys[i]->blocked)) {

                                enemys[i]->speed -= (enemys[i]->ospeed*0.0075)*playingSpeed;

                                if((enemys[i]->speed < (0.5*enemys[i]->ospeed) && !enemys[i]->blocked)) {
                                    enemys[i]->speed = 0.5*enemys[i]->ospeed;
                                } else if((enemys[i]->speed < (0.5*enemys[i]->ospeed) && enemys[i]->blocked)) {
                                    enemys[i]->speed = 0.5*enemys[i]->ospeed;
                                }
                            } else if(enemys[i]->blocked) {
                                enemys[i]->blocked = false;
                                enemys[i]->dir = enemys[i]->newdir;
                            }
                        } else if(enemys[i]->soonBanned) {
                            if(enemys[i]->speed > 0) {
                                enemys[i]->speed -= (enemys[i]->ospeed*0.0075)*playingSpeed;
                                if(enemys[i]->speed < 0) enemys[i]->speed = 0;
                            }
                        } else {
                            if(enemys[i]->speed < enemys[i]->ospeed) {
                                enemys[i]->speed += (enemys[i]->ospeed*0.0075)*playingSpeed;
                                if(enemys[i]->speed > enemys[i]->ospeed) enemys[i]->speed = enemys[i]->ospeed;
                            }
                        }
                        double speed = enemys[i]->speed;
                        //if(i-1<enemys.size()) speed = enemys[i-1]->speed;
                        double x=0,y=0;
                        if(enemys[i]->blocked) speed /= 2;
                        switch(enemys[i]->dir) {
                        case 1: //rechts
                            x = speed;
                            break;
                        case 3: //links
                            x = -speed;
                            break;
                        case 2: //oben
                            y = speed;
                            break;
                        case 4: //unten
                            y = -speed;
                            break;
                        }
                        if(enemys[i]->dir != enemys[i]->newdir) {
                            speed *= 2;
                            switch(enemys[i]->newdir) {
                            case 1:
                                x = speed;
                                break;
                            case 3:
                                x = -speed;
                                break;
                            case 2:
                                y = speed;
                                break;
                            case 4:
                                y = -speed;
                                break;
                            }
                        }
                        if(enemys[i]->newdir != enemys[i]->dir) {
                            enemys[i]->blocked = true;
                        }
                        enemys[i]->moveBy(x,y);
                        QRect oldRect = tiles[paths[enemys[i]->path].path[next-1]]->rect();
                        QRect adjustedRect = oldRect;
                        //adjustedRect.adjust(-5,-5,5,5);
                        if((!enemys[i]->rect().intersects(adjustedRect) && enemys[i]->type != ENEMY_LEGENDE) ||
                                (!enemys[i]->rect().intersects(oldRect) && enemys[i]->type == ENEMY_LEGENDE)) {
                            enemys[i]->cpos++;
                        }

                    } else {
                        //gegner löschen weil base erreicht
    #ifdef QT_NO_DEBUG //camera shake bei release
                        shake(75,2,2);
    #endif
                        delEnemy(i);
                        setBaseHp(basehp - 1);
                        statistics->enemysPassed++;
                        if(!basehp) {
                            gameOver();
                        }
                    }
                    if(enemys[i]->stunned) {
                        enemys[i]->stunned -= 5*playingSpeed;
                        if(enemys[i]->stunned<0) enemys[i]->stunned = 0;
                    }
                } else {
                    error_string("Fehler: Path",QString::number(enemys[i]->path),QString::number(paths.size()));
                }
                //mutex.unlock();
            }
            if(DEBUG) {
                twavespeedMS = timerWS.nsecsElapsed()/NANO_TO_MILLI;
            }
        }
    } catch(std::exception& e) {
        qDebug()<<e.what();
    }
}

void FrmMain::on_tanimation()
{
    if(!fdir) { //fade--
        if(blendRectOpacity<1) blendRectOpacity += 0.01;
        if(blendRectOpacity>=1) {
            blendRectOpacity = 1;
            switch(moveAn) {
            case ANIMATION_PLAY: //active
                changePlaylist(1);
                switch(subActiveSelected) {
                case 1: //levels
                {
                    reset();
                    if(playingMode == MODE_NORMAL) {
                        loadMap("map"+QString::number(subLevelSelected-4));
                    } else if(playingMode == MODE_MAZE) {
                        loadMap("maze/map"+QString::number(subLevelSelected-4));
                        waypoints.resize(0);
                        for(uint i = 0; i < tiles.size(); i++) {
                            if(tiles[i]->type >= TILE_WAYP) {
                                waypoints.push_back(i);
                            }
                        }
                        if(waypoints.size() > 1) {
                            std::sort(waypoints.begin(), waypoints.end());
                        }
                    }
                    createBounds();
                    gamePaused = true;
                    //active = 1;
                    break;
                }
                case 2: //eigene
                    reset(2);
                    loadMap("",2);
                    createBounds();
                    gamePaused = true;
                    //active = 1;
                    break;
                case 3: //community

                    break;
                }
                break;
            case ANIMATION_EDITOR: //mapeditor
                reset(3);
                loadMap(mName,1,mWidth,mHeight);
                //active = 2;
                break;
            case ANIMATION_EDITOR_OWN: //mapeditor-own
                reset(2);
                subLevelSelected = subActiveSelected;
                loadMap(mName,2,mWidth,mHeight);
                if(playingMode == MODE_MAZE) {
                    waypoints.resize(0);
                    for(uint i = 0; i < tiles.size(); i++) {
                        if(tiles[i]->type >= TILE_WAYP) {
                            waypoints.push_back(i);
                        }
                    }
                    if(waypoints.size() > 1) {
                        std::sort(waypoints.begin(), waypoints.end());
                    }
                }
                //active = 2;
                break;
            case ANIMATION_BACK: //zurück
                if(active != STATE_EDITOR && !error) {
                    changePlaylist(0);
                    if(!isGameOver) {
                        saveGameSave();
                    }
                }
                active = STATE_MAINMENU;
                reset();
                break;
            case ANIMATION_CONTINUE: //continue
                changePlaylist(1);
                reset();
                loadGameSave();
                createBounds();
                gamePaused = true;
                break;
            }
            fdir = 1;
        }
    } else if(fdir==1) { //fade++
        if(blendRectOpacity) blendRectOpacity -= 0.01;
        if(blendRectOpacity<=0) {
            if(moveAn == ANIMATION_PLAY || moveAn == ANIMATION_CONTINUE) {
                resetTimers();
                spawnTiles = true;
                active = STATE_PLAYING;
            } else if(moveAn == ANIMATION_EDITOR || moveAn == ANIMATION_EDITOR_OWN) {
                active = STATE_EDITOR;
            }
            moveAn = ANIMATION_SUSPENDED;
            blendRectOpacity = 0;
            fdir = -1;
            QMetaObject::invokeMethod(t_animation,"stop");
        }
    }
}

void FrmMain::on_tshake()
{
    if(!shaking) return;
    if(shakeX==shakeIntensityX) {
        shakeX = -shakeIntensityX;
    } else {
        shakeX = shakeIntensityX;
    }
    if(shakeY==shakeIntensityY) {
        shakeY = -shakeIntensityY;
    } else {
        shakeY = shakeIntensityY;
    }
    shaking--;
    if(!shaking) {
        shakeX = 0;
        shakeY = 0;
        shakeIntensityX = 0;
        shakeIntensityY = 0;
        QMetaObject::invokeMethod(t_shake,"stop");
    }
}

void FrmMain::on_tcoll()
{
    return;
    //entfernen
}

void FrmMain::on_tmenuAn()
{
    if(active == STATE_MAINMENU && !t_animation->isActive()) {
        if(playedGames >= 9 && dateVerified && lastRewardDate != currentDate) {
            dailyReward->active = true;
        }
        Projectile *p = pool->getProjectile();
        int angle = Engine::random(95,130);
        p->init2(QRectF(Engine::random(1550,1800),-80,40,40), //pos
                1, //type
                angle, //angle
                qCos(qDegreesToRadians((double)angle)), //vx
                qSin(qDegreesToRadians((double)angle)), //vy
                0.0005, //opacityDecAm
                1.0); //vel
        projectiles.push_back(p);
    }
}

void FrmMain::on_tphysics()
{
    if(physicsPaused) return;
    mutex.lock();
    if(!gamePaused && active == STATE_PLAYING) {
        //qDebug()<<world->GetBodyCount();
        //mutex.lock();
        if(playingSpeed == 1) {
            world->Step(0.0010,2,3);
        } else if(playingSpeed == 2){
            world->Step(0.0020,2,3);
        } else if(playingSpeed == 3) {
            world->Step(0.0040,2,3);
        }
        for(uint i=0;i<enemys.size();i++) {
            enemys[i]->updatePos();
        }
    }
    for(uint i=0;i<delBodies.size();i++) {
        if(!world->IsLocked()) {
            if(delBodies[i] != nullptr) {
                if(worldContains(delBodies[i])) {
                    world->DestroyBody(delBodies[i]);
                }
                delBodies[i] = nullptr;
            }
        }
    }
    delBodies.resize(0);
    for(uint i=0;i<createBodies.size();i++) {
        if(!world->IsLocked()) {
            createBodies[i]->initPhysics(world);
            enemys.push_back(createBodies[i]);
        }
    }
    createBodies.resize(0);
    for(uint i=0;i<createTileBodies.size();i++) {
        if(!world->IsLocked()) {
            createTileBodies[i]->initPhysics(world);
        }
    }
    createTileBodies.resize(0);
    mutex.unlock();
}

void FrmMain::on_tidle()
{
    if(playingSpeed && waveBreak) {
        if(waveBreak > 0) {
            waveBreak -= 1000;
            if(waveBreak < 0) waveBreak = 0;
        }
    }
    if(enemys.size() && playingSpeed && !gamePaused) {
        for(uint i = 0; i < enemys.size(); i++) {
            QRectF oldPos = enemys[i]->oldpos;
            QRectF pos = enemys[i]->pos;
            if(oldPos == pos && playingSpeed) {
                int cpos = enemys[i]->cpos;
                QRect knot = tiles[paths[enemys[i]->path].path[cpos]]->rect();
                double x = knot.x()+((80-pos.width())/2)+pos.width()/2;
                double y = knot.y()+((80-pos.width())/2)+pos.width()/2;
                enemys[i]->moveTo(x-pos.width()/2,y-pos.height()/2);
            }
            enemys[i]->oldpos = enemys[i]->pos;
        }
    }

    if((active == STATE_PLAYING || active == STATE_EDITOR) && appState == Qt::ApplicationActive) {
        statistics->timePlayed++;
    }

    if(!shekelLocked) {
        unsigned long long tmp = shop->shekelSave.toULongLong() - 11;
        if(tmp != shop->shekel) {
            qDebug()<<"chet";
            shop->shekel = tmp;
            saveOptions();
            this->close();
        }
    }
    if(!benisLocked && active == STATE_PLAYING) {
        unsigned long long tmp = benisSave.toULongLong() - 11;
        if(tmp != benis) {
            qDebug()<<"chet-benis";
            benis = tmp;
            this->close();
        }
    }
    if(!basehplocked && active == STATE_PLAYING) {
        int tmp = basehpSave.toInt() - 11;
        if(tmp != basehp) {
            qDebug()<<"chet-basehp";
            basehp = tmp;
            this->close();
        }
    }
    if(!waveCountLocked && active == STATE_PLAYING) {
        int tmp = waveCountSave.toInt() - 11;
        if(tmp != waveCount || tmp != currentWave.waveCount) {
            qDebug()<<"chet-wave";
            currentWave.waveCount = tmp;
            waveCount = tmp;
            this->close();
        }
    }
#ifdef Q_OS_WIN
    return;
#endif
    if(appState != Qt::ApplicationActive) {
        if(soundEnabled && musicLoaded) {
            if(music->state() != QMediaPlayer::PausedState) {
                QMetaObject::invokeMethod(music,"pause");
            }
        }
    }
}

void FrmMain::on_tgrasAn()
{
    if(active == STATE_PLAYING && mapLoaded) {
        if(settings->graphicQuality == GRAPHIC_MEDIUM) {
            if(viewRect.width() >= mapwidth/2) {
                return;
            }
        } else if(settings->graphicQuality == GRAPHIC_LOW) {
            //if(viewRect.width() >= mapwidth/3) {
                return;
            //}
        }
        for(uint i = 0; i < tiles.size(); i++) {
            if(tiles[i]->type == 6) {
                if(!tiles[i]->anDir) {
                    tiles[i]->animation++;
                    if(tiles[i]->animation == 3) tiles[i]->anDir = 1;
                } else {
                    tiles[i]->animation--;
                    if(!tiles[i]->animation) tiles[i]->anDir = 0;
                }
            }
        }

        for(uint i = 0; i < towers.size(); i++) {
            if(towers[i]->type == TOWER_REPOST &&
                    (!towers[i]->currentReload)) {
                if(towers[i]->animation == 9) {
                    towers[i]->animation = 0;
                } else {
                    towers[i]->animation++;
                    if(towers[i]->animation > 8) towers[i]->animation = 0;
                }
            }
        }

        towerBlitzAnNum++;
        if(towerBlitzAnNum > 3) {
            towerBlitzAnNum = 0;
        }
    }
}

void FrmMain::on_ttowerTarget()
{
    if(active == STATE_EDITOR || gamePaused || active == STATE_MAINMENU) return;
    for(uint i=0;i<towers.size();i++) {
        //Find target
        int pos = -1;
        double min = getHealth(true);
        QPointF tCenter = towers[i]->pos.center();
        bool done = false;
        if((towers[i]->hasProjectileSpeed ||
            towers[i]->type == TOWER_LASER ||
            towers[i]->type == TOWER_POISON) &&
                towers[i]->type != TOWER_BAN) {
            if(towers[i]->target != nullptr) {
                double distance = Engine::getDistance(tCenter,towers[i]->target->rectF().center());
                if((distance > towers[i]->range && towers[i]->type != TOWER_LASER && towers[i]->type != TOWER_POISON) ||
                        ((towers[i]->type == TOWER_LASER || towers[i]->type == TOWER_POISON) && !towers[i]->isShooting)) {
                    towers[i]->target = nullptr;
                    if(towers[i]->type == TOWER_MINIGUN) {
                        towers[i]->shotsFiredSinceReload = 0;
                        towers[i]->currentReload = towers[i]->reload;
                    }
                } else if(distance > towers[i]->range &&
                          (towers[i]->type == TOWER_LASER || towers[i]->type == TOWER_POISON) &&
                          towers[i]->isShooting) {
                    towers[i]->target = nullptr;
                    towers[i]->isShooting = false;
                    int diff = 5;
                    if(towers[i]->type == TOWER_POISON) {
                        diff = 50;
                    }
                    towers[i]->currentReload = (towers[i]->reload / diff) * towers[i]->shotsFiredSinceReload;
                    towers[i]->shotsFiredSinceReload = 0;
                }
            }
            double minDistance = mapwidth * mapheight;
            if(towers[i]->target == nullptr) {
                for(uint a=0;a < enemys.size() && !done;a++) {
                    if(enemys[a]->physicsInitialized) {
                        if(enemys[a]->preHealth && !enemys[a]->soonBanned) {
                            double health = enemys[a]->preHealth;
                            double distance = Engine::getDistance(tCenter,enemys[a]->rectF().center());
                            if(distance < towers[i]->range) {
                                switch(towers[i]->type) {
                                case TOWER_MINUS: //Minustower
                                case TOWER_SNIPER: //Snipertower
                                case TOWER_FLAK: //Flaktower
                                case TOWER_LASER: //Lasertower
                                case TOWER_POISON: //Gifttower
                                case TOWER_MINIGUN:
                                    if(towers[i]->isTargetValid(enemys[a]->type)) {
                                        if(enemys[a]->stunned) { //wenn betäubt focus
                                            pos = a;
                                            done = true;
                                            break;
                                        } else if(health < min ||
                                                  Engine::isEqual(health, min)) {
                                            pos = a;
                                            min = health;
                                            //done=true; //<-hinmachen falls nicht auf den mit niedrigsten hp
                                        }
                                    }
                                    break;
                                case TOWER_HERZ: //Favoritentower
                                case TOWER_REPOST: //Repostturm
                                    if(distance<minDistance) {
                                        pos = a;
                                        minDistance = distance;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            } else {
                double targetAngle = 90+Engine::getAngle(towers[i]->rect().center(),towers[i]->target->rect().center());
                if(targetAngle>360) targetAngle -= 360;
                towers[i]->targetAngle = targetAngle;
            }
        }
        if(pos>=0) {
            towers[i]->target = enemys[pos];
            double targetAngle = 90+Engine::getAngle(towers[i]->rect().center(),towers[i]->target->rect().center());
            if(targetAngle > 360) targetAngle -= 360;
            towers[i]->targetAngle = targetAngle;
        } else { //kein gegner gefunden, ziele auf nächsten gegner wenn möglich
            if(enemys.size() && towers[i]->target == nullptr) {
                int minDistance = mapwidth*mapheight;
                Enemy *target = nullptr;
                for(uint a = 0; a < enemys.size(); a++) {
                    switch(towers[i]->type) {
                    case TOWER_MINUS: //Minustower
                    case TOWER_HERZ: //Favtower
                    case TOWER_SNIPER: //Snipertower
                    case TOWER_FLAK: //Flaktower
                    case TOWER_LASER: //Lasertower
                    case TOWER_POISON: //Gifttower
                    case TOWER_MINIGUN:
                        if(towers[i]->isTargetValid(enemys[a]->type)) {
                            double tmpDistance = Engine::getDistance(enemys[a]->rect().center(),towers[i]->rect().center());
                            if(tmpDistance < minDistance) {
                                target = enemys[a];
                                minDistance = tmpDistance;
                            }
                        }
                        break;
                    }
                }
                if(target != nullptr) {
                    double targetAngle = 90+Engine::getAngle(towers[i]->rect().center(),target->rect().center());
                    if(targetAngle>360) targetAngle -= 360;
                    towers[i]->targetAngle = targetAngle;
                }
            }
        }
    }
}

void FrmMain::on_mediaStateChanged(QMediaPlayer::MediaStatus status)
{
    if(status == QMediaPlayer::MediaStatus::LoadedMedia ||
            status == QMediaPlayer::MediaStatus::BufferedMedia) {
        musicLoaded = true;
        int state = appState;
#ifdef Q_OS_WIN
        state = Qt::ApplicationActive;
#endif
        if(music->state() != QMediaPlayer::PlayingState && state == Qt::ApplicationActive) {
            music->setVolume(25);
            if(soundEnabled) {
                music->play();
            }
        }
    }
}

void FrmMain::on_appStateChanged(Qt::ApplicationState state)
{
#ifdef Q_OS_WIN
    return;
#endif
    this->appState = state;
    if(active == STATE_SUSPENDED) return;
    if((state==Qt::ApplicationInactive||
            state==Qt::ApplicationSuspended||
            state==Qt::ApplicationHidden)) {
        suspended = true;
        QMetaObject::invokeMethod(t_draw,"stop");
        if(soundEnabled && musicLoaded) music->pause();
        QMetaObject::invokeMethod(t_main,"stop");
        QMetaObject::invokeMethod(t_projectile,"stop");
        QMetaObject::invokeMethod(t_projectile_full,"stop");
        QMetaObject::invokeMethod(t_wave,"stop");
        QMetaObject::invokeMethod(t_waveSpeed,"stop");
        QMetaObject::invokeMethod(t_physics,"stop");
        QMetaObject::invokeMethod(t_menuAn,"stop");
        if((active == STATE_PLAYING && !isGameOver) || active == STATE_EDITOR) {
            if(active == STATE_PLAYING) {
                saveGameSave();
            }
            backMenu = true;
            gamePaused = true;
        }
        saveMaps();
        saveOptions();
    } else if(state == Qt::ApplicationActive && suspended) {
        suspended = false;
        if(soundEnabled && musicLoaded) music->play();
        do {
            QMetaObject::invokeMethod(t_draw,"start");
            QMetaObject::invokeMethod(t_main,"start");
            QMetaObject::invokeMethod(t_projectile,"start");
            QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,50));
            QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100));
            QMetaObject::invokeMethod(t_waveSpeed,"start");
            QMetaObject::invokeMethod(t_physics,"start");
            QMetaObject::invokeMethod(t_menuAn,"start");
        } while(!checkTimers(1));
        if(active == STATE_PLAYING && enemys.size()) {
            playingSpeed = 1;
            resetTimers();
        }
        dateVerified = false;
        getStatus();
    }
}

void FrmMain::on_mapBuy(int subSelected, bool mode)
{
    if(!mode) {
        lvlPrices[subSelected] = 0;
    }
    saveOptions();
}

void FrmMain::on_buyError(int id)
{
    switch(id) {
    case 1: //vorherige nicht gekauft
        QMessageBox::critical(this,"Fehler", "Du musst erst die vorherige Map kaufen!");
        break;
    case 2:
        QMessageBox::critical(this,"Fehler", "Du hast nicht genug Shekel!");
        break;
    }
}

void FrmMain::changeSize(QPainter &painter, int pixelSize, bool bold)
{
    f.setPixelSize(pixelSize);
    f.setBold(bold);
    painter.setFont(f);
}

void FrmMain::buyMinusTower(int tpos, bool intersects)
{
    //loop bei kaufen & checks + reset testen & einbauen
    if(playingMode == MODE_MAZE) {
        if(pathsContains(tpos)) intersects = true;
    }
    int cost = minusTowerCost;
    setBenis(benis - cost);
    Tower *t = new Tower();
    t->type = TOWER_MINUS;
    t->dmg = 10;
    t->stun = 0;
    t->pos = tiles[tpos]->pos;
    t->reload = 1000;
    t->currentReload = t->reload;
    t->range = 150;
    t->pspeed = 2.5;
    t->name = "Minustower";
    t->tnum = tpos;
    t->angleSpeed = 1.25;
    t->price = minusTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[tpos]->type = TILE_TOWER;
    tiles[tpos]->t = t;
    if(chosenTiles.size()) {
        towerMenu = chosenTiles[chosenTiles.size()-1];
        int num = chosenTiles[0];
        chosenTiles.erase(chosenTiles.begin());
        buyMinusTower(num, intersects);
    } else t->tmpRangeDir = 0;

    if(playingMode == MODE_MAZE) {
        if(intersects) {
            updatePath(tpos);
        }
    }

    towerMenuSelected = 0;
}

void FrmMain::buyFavTower(int tpos, bool intersects)
{
    if(playingMode == MODE_MAZE) {
        if(pathsContains(tpos)) intersects = true;
    }

    setBenis(benis - herzTowerCost);
    Tower *t = new Tower();
    t->type = TOWER_HERZ;
    t->dmg = 0;
    t->stun = 2000;
    t->pos = tiles[tpos]->pos;
    t->reload = 3000;
    t->currentReload = t->reload;
    t->range = 150;
    t->pspeed = 2.5;
    t->projectilePxID = 1; //Herz statt -
    t->name = "Favoritentower";
    t->tnum = tpos;
    t->angleSpeed = 1;
    t->price = herzTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[tpos]->t = t;
    if(chosenTiles.size()) {
        towerMenu = chosenTiles[chosenTiles.size()-1];
        int num = chosenTiles[0];
        chosenTiles.erase(chosenTiles.begin());
        buyFavTower(num);
    } else t->tmpRangeDir = 0;

    if(playingMode == MODE_MAZE) {
        if(intersects) {
            updatePath(tpos);
        }
    }

    towerMenuSelected = 0;
}

void FrmMain::buyRepostTower(int tpos, bool intersects)
{
    if(playingMode == MODE_MAZE) {
        if(pathsContains(tpos)) intersects = true;
    }

    setBenis(benis - repostTowerCost);
    Tower *t = new Tower();
    t->type = TOWER_REPOST;
    t->dmg = 0;
    t->stun = 0;
    t->repost = 3000;
    t->pos = tiles[tpos]->pos;
    t->reload = 5000;
    t->currentReload = t->reload;
    t->range = 125;
    t->hasProjectileSpeed = false;
    t->hasTurnSpeed = false;
    t->pspeed = 2.5;
    t->projectilePxID = 1;
    t->name = "Reposttower";
    t->tnum = tpos;
    t->price = repostTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[tpos]->t = t;
    if(chosenTiles.size()) {
        towerMenu = chosenTiles[chosenTiles.size()-1];
        int num = chosenTiles[0];
        chosenTiles.erase(chosenTiles.begin());
        buyRepostTower(num);
    } else t->tmpRangeDir = 0;

    if(playingMode == MODE_MAZE) {
        if(intersects) {
            updatePath(tpos);
        }
    }

    towerMenuSelected = 0;
}

void FrmMain::buyBenisTower(int tpos, bool intersects)
{
    if(playingMode == MODE_MAZE) {
        if(pathsContains(tpos)) intersects = true;
    }

    setBenis(benis - benisTowerCost);
    Tower *t = new Tower();
    t->type = TOWER_BENIS;
    t->dmg = 10;
    t->stun = 0;
    t->repost = 0;
    t->hasProjectileSpeed = false;
    t->hasTurnSpeed = false;
    t->hasRange = false;
    t->pos = tiles[tpos]->pos;
    t->reload = 10000;
    t->currentReload = t->reload;
    t->range = 0;
    t->name = "Benistower";
    t->tnum = tpos;
    t->price = benisTowerCost;
    towers.push_back(t);
    tiles[tpos]->t = t;
    if(chosenTiles.size()) {
        towerMenu = chosenTiles[chosenTiles.size()-1];
        int num = chosenTiles[0];
        chosenTiles.erase(chosenTiles.begin());
        buyBenisTower(num);
    } else t->tmpRangeDir = 0;

    if(playingMode == MODE_MAZE) {
        if(intersects) {
            updatePath(tpos);
        }
    }

    towerMenuSelected = 0;
}

void FrmMain::buyBanTower(int tpos, bool intersects)
{
    if(playingMode == MODE_MAZE) {
        if(pathsContains(tpos)) intersects = true;
    }

    setBenis(benis - banTowerCost);
    Tower *t = new Tower();
    t->type = TOWER_BAN;
    t->dmg = 0;
    t->stun = 0;
    t->repost = 0;
    t->pos = tiles[tpos]->pos;
    t->reload = 30000;
    t->currentReload = t->reload;
    //t->reloadMax = 30000;
    t->hasDamage = false;
    t->hasRange = false;
    t->hasTurnSpeed = false;
    t->range = 0;
    t->name = "Bantower";
    t->tnum = tpos;
    t->projectilePxID = 2;
    t->pspeed = 2.5;
    t->price = banTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[tpos]->t = t;
    if(chosenTiles.size()) {
        towerMenu = chosenTiles[chosenTiles.size()-1];
        int num = chosenTiles[0];
        chosenTiles.erase(chosenTiles.begin());
        buyBanTower(num);
    } else t->tmpRangeDir = 0;

    if(playingMode == MODE_MAZE) {
        if(intersects) {
            updatePath(tpos);
        }
    }

    towerMenuSelected = 0;
}

void FrmMain::buySniperTower(int tpos, bool intersects)
{
    if(playingMode == MODE_MAZE) {
        if(pathsContains(tpos)) intersects = true;
    }

    setBenis(benis - sniperTowerCost);
    Tower *t = new Tower();
    t->type = TOWER_SNIPER;
    t->dmg = 50;
    t->stun = 0;
    t->pos = tiles[tpos]->pos;
    t->reload = 4000;
    t->currentReload = t->reload;
    t->range = 225;
    t->pspeed = 3.75;
    t->name = "Snipertower";
    t->tnum = tpos;
    t->angleSpeed = 0.9;
    t->price = sniperTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[tpos]->t = t;
    if(chosenTiles.size()) {
        towerMenu = chosenTiles[chosenTiles.size()-1];
        int num = chosenTiles[0];
        chosenTiles.erase(chosenTiles.begin());
        buySniperTower(num);
    } else t->tmpRangeDir = 0;

    if(playingMode == MODE_MAZE) {
        if(intersects) {
            updatePath(tpos);
        }
    }

    towerMenuSelected = 0;
}

void FrmMain::buyFlakTower(int tpos, bool intersects)
{
    if(playingMode == MODE_MAZE) {
        if(pathsContains(tpos)) intersects = true;
    }

    setBenis(benis - flakTowerCost);
    Tower *t = new Tower();
    t->type = TOWER_FLAK;
    t->dmg = 10;
    t->stun = 0;
    t->pos = tiles[tpos]->pos;
    t->reload = 400;
    t->currentReload = t->reload;
    t->range = 150;
    t->pspeed = 2.5;
    t->name = "Flaktower";
    t->tnum = tpos;
    t->angleSpeed = 2;
    t->price = flakTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[tpos]->t = t;
    if(chosenTiles.size()) {
        towerMenu = chosenTiles[chosenTiles.size()-1];
        int num = chosenTiles[0];
        chosenTiles.erase(chosenTiles.begin());
        buyFlakTower(num);
    } else t->tmpRangeDir = 0;

    if(playingMode == MODE_MAZE) {
        if(intersects) {
            updatePath(tpos);
        }
    }

    towerMenuSelected = 0;
}

void FrmMain::buyLaserTower(int tpos, bool intersects)
{
    if(playingMode == MODE_MAZE) {
        if(pathsContains(tpos)) intersects = true;
    }

    setBenis(benis - laserTowerCost);
    Tower *t = new Tower();
    t->type = TOWER_LASER;
    t->dmg = 20;
    t->stun = 0;
    t->pos = tiles[tpos]->pos;
    t->hasProjectileSpeed = false;
    t->reload = 3500;
    t->currentReload = t->reload;
    t->range = 175;
    t->pspeed = 2;
    t->name = "Lasertower";
    t->tnum = tpos;
    t->angleSpeed = 1.5;
    t->price = laserTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[tpos]->t = t;
    if(chosenTiles.size()) {
        towerMenu = chosenTiles[chosenTiles.size()-1];
        int num = chosenTiles[0];
        chosenTiles.erase(chosenTiles.begin());
        buyLaserTower(num);
    } else t->tmpRangeDir = 0;

    if(playingMode == MODE_MAZE) {
        if(intersects) {
            updatePath(tpos);
        }
    }

    towerMenuSelected = 0;
}

void FrmMain::buyPoisonTower(int tpos, bool intersects)
{
    if(playingMode == MODE_MAZE) {
        if(pathsContains(tpos)) intersects = true;
    }

    setBenis(benis - poisonTowerCost);
    Tower *t = new Tower();
    t->type = TOWER_POISON;
    t->dmg = 0.25;
    t->stun = 0;
    t->pos = tiles[tpos]->pos;
    t->reload = 4000;
    t->currentReload = t->reload;
    t->hasProjectileSpeed = false;
    t->range = 125;
    t->pspeed = 0;
    t->name = "Gifttower";
    t->tnum = tpos;
    t->angleSpeed = 1.5;
    t->price = poisonTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[tpos]->t = t;
    if(chosenTiles.size()) {
        towerMenu = chosenTiles[chosenTiles.size()-1];
        int num = chosenTiles[0];
        chosenTiles.erase(chosenTiles.begin());
        buyPoisonTower(num);
    } else t->tmpRangeDir = 0;

    if(playingMode == MODE_MAZE) {
        if(intersects) {
            updatePath(tpos);
        }
    }

    towerMenuSelected = 0;
}

void FrmMain::buyMinigunTower(int tpos, bool intersects)
{
    if(playingMode == MODE_MAZE) {
        if(pathsContains(tpos)) intersects = true;
    }

    setBenis(benis - minigunTowerCost);
    Tower *t = new Tower();
    t->type = TOWER_MINIGUN;
    t->dmg = 0.5;
    t->stun = 0;
    t->animationMax = 3;
    t->pos = tiles[tpos]->pos;
    t->reload = 500;
    t->currentReload = t->reload;
    t->pspeed = 2.25;
    t->range = 150;
    t->name = "Miniguntower";
    t->tnum = tpos;
    t->angleSpeed = 1.5;
    t->price = minigunTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[tpos]->t = t;
    if(chosenTiles.size()) {
        towerMenu = chosenTiles[chosenTiles.size()-1];
        int num = chosenTiles[0];
        chosenTiles.erase(chosenTiles.begin());
        buyMinigunTower(num);
    } else t->tmpRangeDir = 0;

    if(playingMode == MODE_MAZE) {
        if(intersects) {
            updatePath(tpos);
        }
    }

    towerMenuSelected = 0;
}

void FrmMain::buyShekelAnimation(int amount, int price)
{
    for(int i = 0; i <= amount; i++) {
        QPoint newPoint = QPoint(Engine::random(100,1820), Engine::random(100, 980));
        Projectile *p = pool->getProjectile();
        int angle = Engine::getAngle(QPointF(newPoint), shekelRect.center());
        p->init3("lol",
                 QPoint(newPoint.x(),newPoint.y()),
                 angle,
                 qCos(qDegreesToRadians((double)angle)),
                 qSin(qDegreesToRadians((double)angle)),
                 0,
                 7.5);
        p->dmg = price;
        p->hasShekelImage = true;
        p->type = 7;
        p->rect.setWidth(50);
        projectiles.push_back(p);
    }
}

void FrmMain::placeWall(QPointF pos)
{
    if(active != STATE_PLAYING) return;
    if(shop->items[1].count < 1) return;
    int xpos = pos.x() / 80;
    int ypos = pos.y() / 80;
    int tpos = xpos + (ypos*mapwidth/80);
    if(tiles[tpos]->type == 1) {
        bool ok = true;
        for(uint i = 0; i < enemys.size(); i++) {
            int num = paths[enemys[i]->path].path[enemys[i]->cpos];
            if(tpos == num) {
                ok = false;
                break;
            }
        }
        if(ok) {
            mutex.lock();
            shop->items[1].count--;
            tiles[tpos]->type = 7;
            tiles[tpos]->wallTime = 5000;
            createTileBodies.push_back(tiles[tpos]);
            mutex.unlock();
        }
    }
}

void FrmMain::fixEnemies()
{
    //evtl zurück zum letzten wegpunkt?
    for(uint i = 0; i < enemys.size(); i++) {
        int cpos = enemys[i]->cpos;
        if(!pathsContains(cpos)) {
            if(tiles[cpos]->type != TILE_BASE &&
                    tiles[cpos]->type != TILE_SPAWN) {
                int newpos = paths[enemys[i]->path].path[0];
                QRect knot = tiles[newpos]->rect();
                QRectF pos = enemys[i]->rectF();
                double x = knot.x()+((80-pos.width())/2)+pos.width()/2;
                double y = knot.y()+((80-pos.width())/2)+pos.width()/2;
                enemys[i]->cpos = 0;
                enemys[i]->speed *= 2;
                enemys[i]->moveTo(x-pos.width()/2,y-pos.height()/2);
            }
        }
    }
}

void FrmMain::delEnemy(int pos)
{
    for(uint i=0;i<projectiles.size();i++) {
        if(!projectiles[i]->del) {
            if(projectiles[i]->target == enemys[pos]) {
                projectiles[i]->target = nullptr;
                projectiles[i]->type = 4;
                projectiles[i]->opacity = 1;
                projectiles[i]->opacityDecAm = 0.050;
                //projectiles[i]->del = 2;
            }
        }
    }
    for(uint i=0;i<towers.size();i++) {
        if(towers[i]->target == enemys[pos]) {
            towers[i]->target = nullptr;
            if(towers[i]->type == 8 && towers[i]->isShooting) {
                towers[i]->isShooting = false;
                towers[i]->currentReload = (towers[i]->reload / 5)*towers[i]->shotsFiredSinceReload;
                towers[i]->shotsFiredSinceReload = 0;
            }
        }
    }
    b2Body *b = enemys[pos]->body;
    enemys[pos]->free();
    checkPush(b);
    enemys.erase(enemys.begin()+pos);
}

void FrmMain::delTower(Tower *t)
{
    int n = t->tnum;
    for(uint i=0;i<towers.size();i++) {
        if(t==towers[i]) {
            towers.erase(towers.begin()+i);
            break;
        }
    }
    tiles[n]->t = nullptr;
    delete t;
    //mutex.unlock();
}

void FrmMain::reset(int custom)
{
    resetTimers();
    if(custom != 2) {
        mapSaved = 0;
        mapPlaying = 0;
    }
    if(custom!=3) {
        mWidth = 0;
        mHeight = 0;
    }
    viewRect = QRect(0,0,1920,1080);
    chosenpath = 0;
    transX = 0;
    transY = 0;
    editMode = 0;
#ifdef QT_DEBUG
    setBenis(10000, true);
#else
    setBenis(150, true);
#endif
    upgradeCost = 0;
    towerMenu = -1;
    tempPath.path.resize(0);
    if(mapwidth) {
        setWaveCount(0);
        currentWave.resetWave(true);
#ifdef QT_DEBUG
        setWaveCount(20);
#endif
        currentWave.updateEnemysPerWave(true,mapwidth/80);
    }
    canStartNextWave = false;
    internalWaveCount = 0;
    mName = "";
    loaded = false;
    playingSpeed = 0; //noch auf 0 ändern
    isGameOver = false;
    error = false;
    spawnTiles = false;
    setBaseHp(75);
    enemyWarning = false;
    beginFade = 500;
    waveBreak = 0;
    powerupSelected = -1;
    shekelCoinSize = 0;
    ownBaseCount = 0;
    enemyBaseCount = 0;
    chosenTiles.resize(0);
    gamePaused = false;
    for(uint i=0;i<tiles.size();i++) {
        tiles[i]->body = nullptr;
        delete tiles[i];
    }
    tiles.resize(0);
    delAllEnemys(1);
    std::vector <Tower*> tmpTowers = towers;
    towers.resize(0);
    for(uint i=0;i<towers.size();i++) {
        delete tmpTowers[i];
    }
    for(uint i=0;i<projectiles.size();i++) {
        projectiles[i]->free();
    }
    projectiles.resize(0);
    for(uint i=0;i<enemys.size();i++) {
        enemys[i]->free();
    }
    enemys.resize(0);
    paths.resize(0);
    //delAll = 1;
    do {
        QMetaObject::invokeMethod(t_physics,"stop");
    } while(t_physics->isActive());
    b2Body *list = world->GetBodyList();
    while(list) {
        b2Body *tmp = list->GetNext();
        world->DestroyBody(list);
        list = tmp;
    }
    QMetaObject::invokeMethod(t_physics,"start");
}

void FrmMain::delAllEnemys(int a)
{
    for(uint i=0;i<enemys.size();i++) {
        if(!a) checkPush(enemys[i]->body);
        enemys[i]->free();
    }
    enemys.resize(0);
}

void FrmMain::pauseGame()
{
    gamePaused = true;
}

void FrmMain::error_string(QString e1, QString e2, QString e3)
{
    QMessageBox::critical(this,"Fehler",e1+"\n"+e2+"\n"+e3);
    error = true;
    hasPlayingSave = false;
    moveAn = ANIMATION_BACK;
    active = STATE_MAINMENU;
    return;
}

void FrmMain::error_save(QFile &file, QString msg)
{
    file.close();
    file.remove();
    fdir = 0;
    changePlaylist(0);
    error = true;
    hasPlayingSave = false;
    moveAn = ANIMATION_BACK;
    active = STATE_MAINMENU;
    QMessageBox::warning(this,"Fehler",msg);
    return;
}

void FrmMain::error_main(QString e)
{
    closing = true;
    do {
        QMetaObject::invokeMethod(t_main,"stop");
        QMetaObject::invokeMethod(t_projectile,"stop");
        QMetaObject::invokeMethod(t_projectile_full,"stop");
        QMetaObject::invokeMethod(t_draw,"stop");
        QMetaObject::invokeMethod(t_wave,"stop");
        QMetaObject::invokeMethod(t_waveSpeed,"stop");
        QMetaObject::invokeMethod(t_physics,"stop");
        QMetaObject::invokeMethod(t_menuAn,"stop");
        QMetaObject::invokeMethod(t_animation,"stop");
        QMetaObject::invokeMethod(t_idle,"stop");
        QMetaObject::invokeMethod(t_shake,"stop");
        QMetaObject::invokeMethod(t_grasAn,"stop");
        QMetaObject::invokeMethod(t_towerTarget,"stop");
    } while(checkTimers(1));
    workerThread->quit();
    physicsThread->quit();
    projectileThread->quit();
    QMessageBox::information(this, "Fehler", "Fehler! Bitte an @FireDiver melden:\n"+
                                       e);
    this->close();
}

int FrmMain::updatePath(int pos)
{
    //todo
    //
    //- abfrage nach gegnern + fehlermeldung
    //- pfad bei turmverkauf
    int ret = 0;
    tiles[pos]->type = TILE_TOWER;
    mutex.lock();

    if(tempPath.path.size()) tempPath.path.resize(0);

    for(uint i = 0; i < tiles.size(); i++) {
        if(tiles[i]->body != nullptr) {
            if(tiles[i]->type != TILE_ART13) {
                b2Body *tmp = tiles[i]->body;
                tiles[i]->body = nullptr;
                delBodies.push_back(tmp);
            }
        }
    }

    ret = createMazePath();
    createPathBoundaries();
    fixEnemies();
    enemyWarning = false;
    if(ret) {
        mutex.unlock();
        return ret;
    }
    mapPx = createMapImage(0);
    mapPx_gras = createMapImage(2);

    mutex.unlock();
}

int FrmMain::calcProjectiles(int num)
{
    int i = num;
    if(i >= (int)projectiles.size()) return 0;
    if(!projectiles[i]->del) {
        projectiles[i]->update();
        if(!projectiles[i]->type) { //projektil
            Enemy *tmpTarget = projectiles[i]->target;
            if(projectiles[i]->rect.intersects(tmpTarget->centerRect())) {
                bool coll = true;

                /*if(tmpTarget->type == ENEMY_LEGENDE) { //Raute
                    QPolygonF poly = Engine::getRauteFromRect(tmpTarget->rectF());
                    coll = Engine::polygonIntersectsRect(projectiles[i]->rect, poly);
                } else { //Kreis
                    coll = Engine::intersectsCirclewithCircle(projectiles[i]->rect,tmpTarget->rectF());
                }*/

                if(coll) {
                    if(account.accountState == ACCOUNT_CHEATER) {
                        handleCheater(tmpTarget->rect());
                    } else {
                        tmpTarget->reduceHealth(projectiles[i]->dmg);
                        if(tmpTarget->soonBanned) {
                            tmpTarget->soonBanned++;
                            if(tmpTarget->soonBanned >= 11) tmpTarget->health = 0;
                        }
                        if(projectiles[i]->stun) {
                            tmpTarget->setStun(projectiles[i]->stun);
                        }
                    }
                    projectiles[i]->del = 1;
                }
            }
        } else if(projectiles[i]->type < 5){ //text + blus + giftgas + fade proj
            if(projectiles[i]->type == 3) { //Giftprojektil
                for(uint a = 0;a < enemys.size(); a++) {
                    if(projectiles[i]->rect.intersects(enemys[a]->rectF())) {
                        if(projectiles[i]->targetDefinitions.size()) {
                            double dmg = projectiles[i]->poisonDmg * (projectiles[i]->getTargetEfficiency(enemys[a]->type) / 100.0);
                            if(dmg > 0) {
                                if(account.accountState == ACCOUNT_CHEATER) {
                                    handleCheater(enemys[a]->rect());
                                } else {
                                    if(projectiles[i]->opacity >= 0.7) {
                                        dmg *= 1.0;
                                    } else if(projectiles[i]->opacity >= 0.5) {
                                        dmg *= 0.7;
                                    } else if(projectiles[i]->opacity >= 0.3) {
                                        dmg *= 0.4;
                                    } else if(projectiles[i]->opacity > 0) {
                                        dmg *= 0.1;
                                    }
                                    enemys[a]->reduceHealth(enemys[a]->maxHealth * (0.00025 * dmg));
                                    enemys[a]->preHealth -= enemys[a]->maxHealth * (0.00025 * dmg);
                                    if(enemys[a]->preHealth < 0) enemys[a]->preHealth = 0;
                                }
                            }
                        }
                    }
                }
            }
            if(projectiles[i]->opacity <= 0 && projectiles[i]->opacityDecAm) {
                projectiles[i]->del = 3;
            }
        } else if(projectiles[i]->type == 5 && projectiles[i]->rect.intersects(pr0coinRect)) { //Shekel symbol
            shekelCoinSize = 15;
            setBenis(benis + projectiles[i]->dmg);
            projectiles[i]->del = 5;
        } else if(projectiles[i]->type == 6) { //arc
            if(projectiles[i]->opacity <= 0 && projectiles[i]->opacityDecAm) {
                projectiles[i]->del = 6;
            }
        } else if(projectiles[i]->type == 7 && projectiles[i]->rect.intersects(shekelRect)) {//shekel kaufen
            shekelCoinSize = 15;
            setShekel(shop->shekel + projectiles[i]->dmg, false);
            projectiles[i]->del = 7;
        } else if(projectiles[i]->type == 8) {
            if(projectiles[i]->rect.intersects(QRectF(projectiles[i]->targetRect))) {
                int num = -1;
                for(uint a = 0; a < towers.size(); a++) {
                    if(projectiles[i]->targetRect.intersects(QRectF(towers[a]->rect()))) {
                        num = a;
                        break;
                    }
                }
                if(num > -1) {
                    towers[num]->disabled = disabledTime;
                }
                projectiles[i]->del = 8;
            }
        }
        if(!projectiles[i]->type &&
                !projectiles[i]->target->soonBanned) { //wenn kein effekt
            if(!projectiles[i]->rect.intersects(QRectF(0,0,mapwidth,mapheight))) {
                projectiles[i]->del = 2;
            }
        }
        if(projectiles[i]->type==1 && projectiles[i]->opacity < 0.5) {
            if(!projectiles[i]->rect.intersects(QRectF(0,0,1920,1080))) {
                projectiles[i]->del = 4;
            }
        }
    }
    return projectiles[i]->del;
}

int FrmMain::sendDataToServer(QString data)
{
    int code = -1;
    QTcpSocket *socket = new QTcpSocket();
    socket->connectToHost(serverIP,38900);
    socket->waitForConnected(500);
    if(socket->state()==QTcpSocket::ConnectedState) {
        QString sender = ".-3#" + data + "#~";
        socket->write(sender.toUtf8());
        socket->waitForBytesWritten(500);
        socket->waitForReadyRead(500);
        QString recvData = socket->readAll();
        code = recvData.toInt();
        socket->close();
    }
    socket->deleteLater();
    return code;
}

int FrmMain::getEnemySizeByType(int type)
{
    int size=0;
    switch(type) {
    case ENEMY_FLIESE:
        size = 20;
        break;
    case ENEMY_NEUSCHWUCHTEL:
        size = 25;
        break;
    case ENEMY_SCHWUCHTEL:
        size = 30;
        break;
    case ENEMY_SPENDER:
        size = 30;
        break;
    case ENEMY_ALTSCHWUCHTEL:
        size = 35;
        break;
    case ENEMY_MOD:
        size = 40;
        break;
    case ENEMY_ADMIN:
        size = 40;
        break;
    case ENEMY_LEGENDE:
        size = 75;
        break;
    case ENEMY_GEBANNT:
        size = 23;
        break;
    }
    return size;
}

int FrmMain::getPathId(int i)
{
    int ppos = -1;
    int path = -1;
    for(uint a = 0; a < paths.size(); a++) {
        if(paths[a].contains(i)) {
            path = a;
        }
    }

    if(path > -1) {
        for(uint a = 0; a < paths[path].path.size(); a++) {
            if(paths[path].path[a] == i) {
                ppos = a;
            }
        }
    } else return -1;

    if(ppos == -1) return -1;


    int size = tiles.size();
    int maxX = mapwidth/80;
    int left = i-1,
        right = i+1,
        up = i-maxX,
        down = i+maxX;
    int id = 0;
    bool leftOk = false;
    bool rightOk = false;
    bool upOk = false;
    bool downOk = false;

    if((left/maxX == i/maxX) && //selbe zeile
            left > -1 && left < size) {
        if(tiles[left]->type == 1 ||
                tiles[left]->type == 4 ||
                tiles[left]->type == 5 ||
                tiles[left]->type >= TILE_WAYP) {
            if(ppos - 1 > -1) {
                if(paths[path].path[ppos-1] == left) {
                    leftOk = true;
                }
            }
            if(ppos + 1 < (int)paths[path].path.size()) {
                if(paths[path].path[ppos+1] == left) {
                    leftOk = true;
                }
            }

        }
    }

    if((right/maxX == i/maxX) &&
            right > -1 && right < size) {
        if(tiles[right]->type == 1 ||
                tiles[right]->type == 4 ||
                tiles[right]->type == 5 ||
                tiles[right]->type >= TILE_WAYP) {
            if(ppos + 1 < (int)paths[path].path.size()) {
                if(paths[path].path[ppos+1] == right) {
                    rightOk = true;
                }
            }
            if(ppos - 1 > -1 ) {
                if(paths[path].path[ppos-1] == right) {
                    rightOk = true;
                }
            }
        }
    }

    if(up > -1 && up < size) {
        if(tiles[up]->type == 1 ||
                tiles[up]->type == 4 ||
                tiles[up]->type == 5 ||
                tiles[up]->type >= TILE_WAYP) {
            if(ppos + 1 < (int)paths[path].path.size()) {
                if(paths[path].path[ppos+1] == up) {
                    upOk = true;
                }
            }
            if(ppos - 1 > -1 ) {
                if(paths[path].path[ppos-1] == up) {
                    upOk = true;
                }
            }
        }
    }

    if(down > -1 && down < size) {
        if(tiles[down]->type == 1 ||
                tiles[down]->type == 4 ||
                tiles[down]->type == 5 ||
                tiles[down]->type >= TILE_WAYP) {
            if(ppos + 1 < (int)paths[path].path.size()) {
                if(paths[path].path[ppos+1] == down) {
                    downOk = true;
                }
            }
            if(ppos - 1 > -1 ) {
                if(paths[path].path[ppos-1] == down) {
                    downOk = true;
                }
            }
        }
    }

    //check ob in weg
    if(leftOk) {
        if(!pathsContains(left) &&
                tiles[left]->type != TILE_BASE &&
                tiles[left]->type != TILE_SPAWN) leftOk = false;
    }

    if(rightOk) {
        if(!pathsContains(right) &&
                tiles[right]->type != TILE_BASE &&
                tiles[right]->type != TILE_SPAWN) rightOk = false;
    }

    if(upOk) {
        if(!pathsContains(up) &&
                tiles[up]->type != TILE_BASE &&
                tiles[up]->type != TILE_SPAWN) upOk = false;
    }

    if(down) {
        if(!pathsContains(down) &&
                tiles[down]->type != TILE_BASE &&
                tiles[down]->type != TILE_SPAWN) downOk = false;
    }

    if(leftOk && rightOk && !upOk && !downOk) {
        id = 0;
    } else if(!leftOk && !rightOk && upOk && downOk) {
        id = 1;
    } else if(leftOk && !rightOk && upOk && !downOk) {
        id = 2;
    } else if(!leftOk && rightOk && upOk && !downOk) {
        id = 3;
    } else if(leftOk && rightOk && upOk && !downOk) {
        id = 4;
    } else if(!leftOk && rightOk && upOk && downOk) {
        id = 5;
    } else if(leftOk && !rightOk && upOk && downOk) {
        id = 6;
    } else if(leftOk && rightOk && upOk && downOk) {
        id = 7;
    } else if(!leftOk && rightOk && !upOk && downOk) {
        id = 8;
    } else if(leftOk && !rightOk && !upOk && downOk) {
        id = 9;
    } else if(leftOk && rightOk && !upOk && downOk) {
        id = 10;
    }
    return id;
}


double FrmMain::getHealth(bool max)
{
    double health = 0;
    for(uint i=0;i<enemys.size();i++) {
        if(enemys[i]->preHealth > health) {
            health = enemys[i]->preHealth;
        }
    }
    if(!max) {
        double min = health;
        for(uint i=0;i<enemys.size();i++) {
            if(enemys[i]->preHealth < min) {
                min = enemys[i]->preHealth;
            }
        }
        health = min;
    }
    return health;
}

double FrmMain::getLegendenDistanz(int ownPos)
{
    double distance = mapwidth*mapheight;
    double minDistance = mapwidth*mapheight;
    for(uint i = 0; i < enemys.size(); i++) {
        if((int)i != ownPos) {
            if(enemys[i]->type == ENEMY_LEGENDE) {
                double tmpD = Engine::getDistance(enemys[i]->pos.center(),
                                                  enemys[ownPos]->pos.center());
                if(tmpD < minDistance && enemys[i]->cpos >= enemys[ownPos]->cpos) {
                    minDistance = tmpD;
                    distance = tmpD;
                }
            }
        }
    }
    return distance;
}

bool FrmMain::checkTimers(int type)
{
    bool ok=true;
    if(!t_projectile_full->isActive()) ok = false;
    if(!t_projectile->isActive()) ok = false;
    if(!t_main->isActive()) ok = false;
    if(!t_wave->isActive()) ok = false;
    if(type == 1) {
        if(!t_draw->isActive()) ok = false;
        if(!t_waveSpeed->isActive()) ok = false;
        if(!t_physics->isActive()) ok = false;
        if(!t_menuAn->isActive()) ok = false;
        if(!t_grasAn->isActive()) ok = false;
        if(!t_towerTarget->isActive()) ok = false;
    }
    return ok;
}

bool FrmMain::worldContains(b2Body *b)
{
    b2Body *list = world->GetBodyList();
    bool ok = false;
    while(list && !ok) {
        if(list == b) {
            ok = true;
        }
        list = list->GetNext();
    }
    return ok;
}

QDate FrmMain::getServerDate()
{
    QDate date(2000,1,1);
    QTcpSocket *socket = new QTcpSocket();
    socket->connectToHost(serverIP,38900);
    socket->waitForConnected(1000);
    if(socket->state()==QTcpSocket::ConnectedState) {
        QString sender = ".-4#0#~";
        socket->write(sender.toUtf8());
        socket->waitForBytesWritten(1000);
        socket->waitForReadyRead(1000);
        QString recvData = socket->readAll();
        date = QDate::fromString(recvData,"yyyyMMdd");
        socket->close();
    }
    socket->deleteLater();
    return date;
}

void FrmMain::getStatus()
{
    QNetworkRequest request;
    request.setUrl(QUrl("http://flatterfogel.ddns.net/main.php?status=true&name="+account.username));
    nreply = manager->get(request);
    QTimer::singleShot(10000, this, SLOT(on_statustimeout()));
}

void FrmMain::newPostInfo()
{
    QUrl link("http://www.pr0gramm.com/new/"+QString::number(newestPost));
    QMessageBox box;
    MsgBox::createMsgBox(box, "Neuer Pr0fense-P0st auf pr0! Willst du ihn öffnen?");
    int result = box.exec();
    if(!result) {
        QDesktopServices::openUrl(link);
    }
}

void FrmMain::on_purchaseShekel(QString paket)
{
#ifdef Q_OS_ANDROID
    if(account.accountState != ACCOUNT_ACTIVE &&
            account.accountState != ACCOUNT_VERIFIED &&
            account.accountState != ACCOUNT_PENDING) {
        QMessageBox::critical(this,"Fehler", "Bitte erstelle zuerst einen Account im Hauptmrnü!");
        return;
    } else if(account.accountState == ACCOUNT_PENDING) {
        QMessageBox::critical(this,"Fehler","Bitte stelle sicher, dass du eine aktive Internetverbindung hast!");
        return;
    }

    QInAppProduct *product = store->registeredProduct(paket);
    if(!product) return;
    product->purchase();
    return;
#else
    Q_UNUSED(paket)
    QMessageBox::information(this, "Info", "Für Käufe in der Windows-Version wende dich bitte an @FireDiver"
                                           " oder per E-Mail an firediver98@gmail.com");
#endif
}

void FrmMain::on_buyItem(int pos)
{
    /*switch(pos) {
    case 0: //superschnell
        superfastUnlocked = true;
        break;
    }*/
    Q_UNUSED(pos)
    saveOptions();
}

void FrmMain::on_buyTower(int pos)
{
    Q_UNUSED(pos);
    saveOptions();
}

void FrmMain::on_openPage(int id)
{
    QString link;
    if(!id) {
        link = "https://pastebin.com/CJvvFEQr";
    } else if(id == 1) {
        link = "https://pastebin.com/BQcicxpA";
    }
    QUrl linkA(link);
    QDesktopServices::openUrl(linkA);
}

void FrmMain::on_setShekel(unsigned long long shekel, bool save)
{
    setShekel(shekel, save);
}

void FrmMain::on_addBonus(int amount)
{
    lastRewardDate = currentDate;
    saveOptions(shop->shekel + amount);
    if(amount <= 150) {
        buyShekelAnimation(amount, 1);
    } else {
        buyShekelAnimation(100, amount / 100);
    }
    dailyReward->active = false;
}

void FrmMain::on_graphicsChanged(int graphic)
{
    //noch nicht connected
    Q_UNUSED(graphic)
}

void FrmMain::on_energieSparenChanged(bool mode)
{
    int time1 = 5, time2 = 10;
#ifdef Q_OS_ANDROID
    time1 = 16;
    time2 = 33;
#endif
    if(!mode) {
        t_draw->start(time1);
    } else t_draw->start(time2);
}

void FrmMain::on_zoomChanged()
{
    saveOptions();
}

void FrmMain::on_status(QNetworkReply *reply)
{
    if (reply->error()) {
        qDebug() << reply->errorString();
        return;
    }

    QString response = reply->readAll();
    reply = nullptr;
    if(response.size()) {
        if(response.at(0)=="." && response.contains("~")) { //Antwort OK
            response.remove(0,1);
            QStringList split = response.split("#");
            if(version < split[1].toDouble()) { //Outdated
                outdated = true;
            }
            if(newestPost < split[0].toInt()) { //Neuer Post
                newestPost = split[0].toInt();
                newPost = true;
            }

            //Accountstate
            account.accountState = split[2].toInt();

            //restore shekel
            int am = split[3].toInt();
            if(am > 0) {
                restore(am);
            }

            //restore items
            int num = split[4].toInt();
            if(num > -1) {
                switch(num) {
                case 0:
                    shop->items[0].locked = false;
                    superfastUnlocked = true;
                    break;
                }
                saveOptions();
            }

            //Datumabfrage
            currentDate = QDate::fromString(split[5], "ddMMyyyy");
            if(lastRewardDate.daysTo(currentDate) > 1) {
                dailyReward->bonus = 0;
            }
            if(lastPlayedDate != currentDate) {
                playedGames = 0;
            }
            dateVerified = true;
        }
    }
}

void FrmMain::on_statustimeout()
{
    nreply->abort();
}

Level FrmMain::parseLvlString(QString lvl, int startPos)
{
    QString map = lvl;
    QStringList msplit = map.split("#");
    Level l;
    l.lvlName = msplit[0];
    l.width = msplit[1].toInt();
    l.height = msplit[2].toInt();
    l.lvlImage = createMapImage(msplit[4],l.width,l.height);
    QString path = msplit[3];
    QString rawPaths = path;
    QStringList rawSplit = rawPaths.split(";");
    for(int i=0;i<rawSplit[0].toInt();i++) {
        Path tmpPath;
        QString p = rawSplit[i+1];
        QStringList split = p.split(',');
        for(int i=0;i<split.size();i++) {
            tmpPath.path.push_back(split[i].toInt());
        }
        l.paths.push_back(tmpPath);
    }
    map = msplit[4];
    msplit = map.split(",");
    for(int a=0;a<msplit.size();a++) {
        l.map.push_back(msplit[a].toInt());
    }
    QRect rect;
    int y=1;
    int a=0;
    for(uint i=startPos;i<levels.size()+1;i++) {
        rect = QRect(500+(325*a),400+(193*y),300,168);
        a++;
        if(a==4) {
            a=0;
            y++;
        }
    }
    l.levelRect = rect;
    return l;
}
#ifdef Q_OS_ANDROID
void FrmMain::handleTransaction(QInAppTransaction *transaction)
{
    //QMessageBox::information(this,"a",transaction->orderId());
    if(transaction->status() == QInAppTransaction::PurchaseApproved) {
        //if(transaction->)
        QString identifier = transaction->product()->identifier();
        QString orderID = transaction->orderId();
        bool ok = false;
        if(orderID.contains("GPA")) {
            ok = true;
        } else {
            QStringList split = orderID.split(".");
            QString merchantID = split[0];
            if(merchantID.contains("7593275571574883854")) {
                ok = true;
            }
        }
        if(ok) {
            if(identifier.contains("shekel")) {
                int amount = 0;
                if(identifier == QStringLiteral("shekel_supersmall")) {
                    amount = shop->shekelPacks[0];
                } else if(identifier == QStringLiteral("shekel_small")) {
                    amount = shop->shekelPacks[1];
                } else if(identifier == QStringLiteral("shekel_normal")) {
                    amount = shop->shekelPacks[2];
                } else if(identifier == QStringLiteral("shekel_big")) {
                    amount = shop->shekelPacks[3];
                } else if(identifier == QStringLiteral("shekel_large")) {
                    amount = shop->shekelPacks[4];
                } else if(identifier == QStringLiteral("shekel_shekel")) {
                    amount = shop->shekelPacks[5];
                } else if(identifier == QStringLiteral("shekel_ultra")) {
                    amount = shop->shekelPacks[6];
                } else if(identifier == QStringLiteral("shekel_super")) {
                    amount = shop->shekelPacks[7];
                }
                saveOptions(shop->shekel + amount);
                if(amount > 100) {
                    buyShekelAnimation(100, amount/100);
                } else if(amount > 50) {
                    buyShekelAnimation(100, 1);
                } else {
                    buyShekelAnimation(50, 1);
                }
            } else { //items
                if(identifier == QStringLiteral("speed_superfast")) {
                    superfastUnlocked = true;
                    shop->items[0].locked = false;
                }
                saveOptions();
            }
            QFuture<void> future2 = QtConcurrent::run(&account, &Account::registerPurchase, orderID, identifier);
        } else {
            QMessageBox::information(this,"Fehler", "Irgendwas ist schiefgelaufen. Melde dich bitte bei @FireDiver bzw"
                                                    " firediver98@gmail.com");
        }
        transaction->finalize();
    } else if(transaction->status() == QInAppTransaction::PurchaseFailed) {
        if(transaction->failureReason() == QInAppTransaction::ErrorOccurred) {
            QMessageBox::information(this,"FEHLER","Fehler! Kauf fehlgeschlagen!");
        }
    }
}

void FrmMain::productRegistered(QInAppProduct *product)
{
    qDebug()<<"OK"<<product->identifier();
}

void FrmMain::productUnknown(QInAppProduct::ProductType ptype, QString id)
{
    Q_UNUSED(ptype)
    qDebug()<<"UNKNOWN"<<id;
}
#endif
QPixmap FrmMain::createMapImage(int custom)
{
    QPixmap map(mapwidth,mapheight);
    QPainter painter(&map);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Engine::getColor(-1));
    painter.drawRect(0,0,mapwidth,mapheight);
    for(uint i=0;i<tiles.size();i++) {
        //if(tiles[i]->pos.intersects(viewRect)) {
            QPixmap p;
            switch(tiles[i]->type) {
            case 2: //turmplatz
                if(playingMode != MODE_MAZE) {
                    p = turmtile;
                } else {
                    painter.setBrush(QColor(172,172,172));
                    painter.drawRect(tiles[i]->rect());
                }
                break;
            case 3: //leer
                p = emptytile;
                break;
            case 4: //enemybase
                p = enemybasePx;
                break;
            case 5: //ownbase
                p = ownbasePx;
                break;
            case 6: //maptile
                if(custom == 1) {
                    p = mapTile;
                } else if(custom == 2) {
                    p = grasAnimation[0];
                }
                break;
            }
            if(tiles[i]->type != TILE_PATH && tiles[i]->type < TILE_WAYP) {
                painter.drawPixmap(tiles[i]->rect(),p);
            } else {
                painter.setBrush(QColor(172,172,172));
                painter.drawRect(tiles[i]->rect());
            }

        //}
    }

    for(uint i = 0; i < paths.size(); i++) {
        for(uint a = 0; a < paths[i].path.size(); a++) {
            int num = paths[i].path[a];
            if(tiles[num]->type == TILE_PATH || tiles[num]->type >= TILE_WAYP) {
                QPixmap p;
                int id = getPathId(num);
                if(id > -1) {
                    p = pathTextures[id];
                    painter.drawPixmap(tiles[num]->rect(),p);
                } else {
                    painter.setBrush(QColor(172,172,172));
                    painter.drawRect(tiles[i]->rect());
                }
            }
        }
    }

    return map;
}

QPixmap FrmMain::createMapImage(QString data, int width, int height)
{
    //erstellt bild von map mit auflösung von 320*180;
    std::vector <QRect> rects;
    int maxX = width;
    int maxY = height;
    for(int u=0;u<maxY;u++) {
        for(int i=0;i<maxX;i++) {
            rects.push_back(QRect(0+(20*i),(20*u),20,20));
        }
    }
    width*=20;
    height*=20;
    QStringList split = data.split(",");
    QPixmap map(width,height);
    QPainter painter(&map);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Engine::getColor(-1));
    painter.drawRect(0,0,width,height);
    for(int i=0;i<split.size();i++) {
        int type = split[i].toInt();
        QPixmap p;
        switch(type) {
        case 2: //turmplatz
            if(playingMode != MODE_MAZE) {
                p = turmtile;
            } else {
                painter.setBrush(QColor(172,172,172));
                painter.drawRect(rects[i]);
            }
            break;
        case 3: //leer
            p = emptytile;
            break;
        case 4: //enemybase
            p = enemybasePx;
            break;
        case 5: //ownbase
            p = ownbasePx;
            break;
        case 6: //maptile
            p = grasAnimation[0];
            break;
        }
        if(type != TILE_PATH && type < TILE_WAYP) {
            painter.drawPixmap(rects[i],p);
        } else {
            if(type < TILE_WAYP) {
                painter.setBrush(QColor(172,172,172));
            } else {
                painter.setBrush(Qt::black);
            }
            painter.drawRect(rects[i]);
        }
    }
    return map;
}

void FrmMain::drawIngame(QPainter &painter)
{
    painter.setPen(Qt::NoPen);
    if(mapLoaded) {
        if(active != STATE_EDITOR) {
            if(!isLoading()) {
                if((settings->graphicQuality == GRAPHIC_MEDIUM && viewRect.width() < mapwidth/2) ||
                        settings->graphicQuality == GRAPHIC_HIGH) {
                    painter.drawPixmap(viewRect, mapPx, viewRect);
                    for(uint i = 0; i < tiles.size(); i++) {
                        if(tiles[i]->pos.intersects(viewRect)) {
                            if(tiles[i]->type == 6) {
                                painter.drawPixmap(tiles[i]->rect(), grasAnimation[tiles[i]->animation]);
                            }
                        }
                    }
                } else {
                    painter.drawPixmap(viewRect, mapPx_gras, viewRect);
                }
            } else {
                painter.setBrush(Engine::getColor(-1));
                painter.drawRect(0,0,mapwidth,mapheight);
                painter.drawPixmap(viewRect, mapPx, viewRect);
                for(uint i = 0; i < tiles.size(); i++) {
                    if(tiles[i]->pos.intersects(viewRect)) {
                        if(tiles[i]->type == 6) {
                            painter.drawPixmap(tiles[i]->rect(), grasAnimation[tiles[i]->animation]);
                        }
                    }
                }
            }
            for(uint a = 0; a < tiles.size(); a++) {
                if(tiles[a]->type == TILE_ART13) {
                    uint num = 0;
                    if(tiles[a]->wallTime > 0 && tiles[a]->wallTime < 5000) {
                        num = (5 - tiles[a]->wallTime / 1000)-1;
                    }
                    painter.drawPixmap(tiles[a]->rect(), wandAnimation[num]);
                }
            }
        }
        if(playingMode == MODE_MAZE) {
            changeSize(painter,64,true);
            for(uint i = 0; i < waypoints.size(); i++) {
                painter.setPen(Qt::white);
                painter.drawText(tiles[waypoints[i]]->rect(), Qt::AlignCenter, QString::number(i));
                painter.setPen(Qt::NoPen);
            }
        }
    }

    /*painter.setPen(Qt::white); //Tile nummern anzeigen
    changeSize(painter,8,true);
    for(uint i = 0; i < tiles.size(); i++) {
        painter.drawText(tiles[i]->rect(), Qt::AlignCenter, QString::number(i));
    }
    painter.setPen(Qt::NoPen);*/

    painter.setOpacity(0.5);
    if(playingMode == MODE_MAZE) {
        if(tempPath.path.size()) {
            for(uint i = 0; i < tempPath.path.size(); i++) {
                int num = (((double)i/tempPath.path.size())*255);
                painter.setBrush(QColor(num,0,0));
                painter.drawRect(tiles[tempPath.path[i]]->rect());
            }
        }
    }
    painter.setOpacity(1);

    if(active == STATE_EDITOR) {
        drawEditor(painter);
    }


    for(uint i=0;i<projectiles.size();i++) {
        if(projectiles[i] != nullptr) { //nullptr abfrage
            if((settings->graphicQuality == GRAPHIC_MEDIUM && viewRect.width() < mapwidth/2) ||
                    (settings->graphicQuality == GRAPHIC_LOW && viewRect.width() < mapwidth/3) ||
                    settings->graphicQuality == GRAPHIC_HIGH) {
                if(projectiles[i]->rect.intersects(viewRect)) {
                    switch(projectiles[i]->type) {
                    case 4: //todesprojektile
                        painter.setOpacity(projectiles[i]->opacity);
                        if(projectiles[i]->pxID < 9 && projectiles[i]->pxID > 2) {
                            painter.drawPixmap(projectiles[i]->rect, normalEnemyDeath[projectiles[i]->pxID-3], QRectF(0,0,130,130));
                        } else if(projectiles[i]->pxID < 21 && projectiles[i]->pxID > 14) {
                            painter.drawPixmap(projectiles[i]->rect, bossEnemyDeath[projectiles[i]->pxID-15], QRectF(0,0,156,156));
                        }
                        break;
                    }
                }
            }
        }
    }
    painter.setOpacity(1);
    if((mapwidth/80 > 32 && (settings->graphicQuality == GRAPHIC_MEDIUM && viewRect.width() >= mapwidth/2)) ||
                            (settings->graphicQuality == GRAPHIC_LOW && viewRect.width() >= mapwidth/3)) {
        painter.setBrush(Qt::red);
        for(uint i = 0; i < enemys.size(); i++) {
            if(enemys[i]->rectF().intersects(viewRect)) {
                painter.setBrush(Engine::getColor(enemys[i]->type));
                painter.drawRect(enemys[i]->rectF());
            }
        }
        /*std::vector <int> enemyUsed;
        for(uint i = 0; i < enemys.size(); i++) {
            if(!(std::find(enemyUsed.begin(), enemyUsed.end(), i) != enemyUsed.end())) {
                std::vector <int> enemysInRange;
                for(uint a = 0; a < enemys.size(); a++) {
                    if(Engine::isEqual(enemys[i]->rectF().center().x(),
                                       enemys[a]->rectF().center().x(),
                                       80) && Engine::isEqual(enemys[i]->rectF().center().y(),
                                                               enemys[a]->rectF().center().y(),
                                                               80)) {
                        enemysInRange.push_back(a);
                        enemyUsed.push_back(a);
                    }
                }
                if(enemysInRange.size()) {
                    int avgX = 0, avgY = 0, avgSize = 1;
                    for(uint b = 0; b < enemysInRange.size(); b++) {
                        avgX += enemys[enemysInRange[b]]->rectF().center().x();
                        avgY += enemys[enemysInRange[b]]->rectF().center().y();
                        avgSize += enemys[enemysInRange[b]]->rectF().width();
                    }
                    avgX /= enemysInRange.size();
                    avgY /= enemysInRange.size();
                    avgSize /= enemysInRange.size();
                    painter.setBrush(Qt::red);
                    painter.drawRect(avgX, avgY, avgSize*1.5, avgSize*1.5);
                }
            }
        }*/
    } else {
        for(uint i=0;i<enemys.size();i++) {
            if(enemys[i]->rectF().intersects(viewRect)) {
                painter.setOpacity(enemys[i]->opacity);
                if(enemys[i]->type != ENEMY_LEGENDE && enemys[i]->type != ENEMY_GEBANNT) {
                    painter.drawPixmap(enemys[i]->rectF(), normalEnemys[enemys[i]->imgID], QRectF(0,0,16,16));
                } else if(enemys[i]->type == ENEMY_GEBANNT) {
                    QRectF t = enemys[i]->rectF();
                    if(settings->graphicQuality == GRAPHIC_HIGH) {
                        painter.save();
                        painter.translate(t.center());
                        painter.rotate(enemys[i]->animation);
                        painter.translate(-t.center().x(),-t.center().y());
                        painter.drawPixmap(t, normalEnemys[enemys[i]->imgID], QRectF(0,0,16,16));
                        painter.setBrush(Engine::getColor(ENEMY_MOD));
                        painter.setOpacity(0.5*enemys[i]->opacity);
                        painter.drawRect(t);
                        painter.restore();
                    } else {
                        painter.drawPixmap(t, normalEnemys[enemys[i]->imgID], QRectF(0,0,16,16));
                        painter.setBrush(Engine::getColor(ENEMY_MOD));
                        int num = enemys[i]->animation;
                        if(num > 180) num -= 180;
                        painter.setOpacity(0.7*(((double)num/180)*enemys[i]->opacity));
                        painter.drawRect(t);
                    }
                } else if(enemys[i]->type == ENEMY_LEGENDE) {
                    painter.drawPixmap(enemys[i]->rectF(), bossPx, QRectF(0,0,156,156));
                }
                painter.setOpacity(0.5*enemys[i]->opacity);
                painter.setBrush(Qt::red);
                painter.setBrush(Engine::getColor(enemys[i]->type));
                if(enemys[i]->type != ENEMY_LEGENDE) {
                    QRectF rect = enemys[i]->rectF();
                    if(enemys[i]->type != ENEMY_GEBANNT) {
                        painter.drawRect(rect);
                    }
                } else {
                    painter.drawPolygon(Engine::getRauteFromRect(enemys[i]->rectF()));
                }
                painter.setOpacity(fade);
                if(enemys[i]->repost) {
                    painter.setOpacity(((double)enemys[i]->repost/enemys[i]->maxRepost)*fade);
                    painter.drawPixmap(enemys[i]->rect(),repostMark);
                    painter.setOpacity(fade);
                }
                if(enemys[i]->stunned && !enemys[i]->soonBanned) {
                    painter.setOpacity(((double)enemys[i]->stunned/enemys[i]->maxStun)*fade);
                    painter.drawPixmap(enemys[i]->rect(),herzPx);
                    painter.setOpacity(fade);
                } else if(enemys[i]->stunned && enemys[i]->soonBanned) {
                    painter.setOpacity((1-((double)enemys[i]->health/enemys[i]->maxHealth))*fade);
                    painter.setBrush(Engine::getColor(ENEMY_GEBANNT));
                    painter.setOpacity(fade);

                    if(enemys[i]->type != ENEMY_LEGENDE) {
                        QRectF rect = enemys[i]->rectF();
                        painter.drawRect(rect);
                    } else {
                        painter.drawPolygon(Engine::getRauteFromRect(enemys[i]->rectF()));
                    }
                }
                if(enemys[i]->flash) {
                    painter.setBrush(Qt::red);
                    painter.setOpacity(0.6);
                    if(enemys[i]->type != ENEMY_LEGENDE) {
                        painter.drawRect(enemys[i]->rectF());
                    } else {
                        painter.drawPolygon(Engine::getRauteFromRect(enemys[i]->rectF()));
                    }
                }
            }
        }
    }
    painter.setOpacity(1);
    for(uint i=0;i<towers.size();i++) {
        if(towers[i]->pos.intersects(viewRect)) {
            drawTower(towers[i]->rect(),painter,towers[i]);
        }
    }
    for(uint i=0;i<towers.size();i++) {
        if(towers[i]->pos.intersects(viewRect)) {
            if(towers[i]->type == TOWER_LASER && towers[i]->isShooting && towers[i]->target != nullptr) {
                if((settings->graphicQuality == GRAPHIC_MEDIUM && viewRect.width() < mapwidth/2) ||
                        (settings->graphicQuality == GRAPHIC_LOW && viewRect.width() < mapwidth/3) ||
                        settings->graphicQuality == GRAPHIC_HIGH) {
                    //Zeichnet Laser vom Laserturm
                    QPointF p(towers[i]->pos.center().x(),towers[i]->pos.center().y()-34);
                    QPointF p2(p.x() - 15, p.y() + 13);
                    QPointF p3(p.x() + 15, p.y() + 13);
                    QTransform transform = QTransform().translate(p.x(),p.y()+34).rotate(towers[i]->angle).translate(-p.x(),-(p.y()+34));
                    p = transform.map(p);
                    p2 = transform.map(p2);
                    p3 = transform.map(p3);
                    painter.setPen(QPen(QBrush(QColor(51,127,210)),2));
                    painter.drawLine(p2,p);
                    painter.drawLine(p3,p);
                    painter.setPen(QPen(QBrush(QColor(51,127,210)),4));
                    painter.drawLine(p,towers[i]->target->rectF().center());
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor(51,127,210));
                    painter.drawEllipse(towers[i]->target->rectF().center(),5,5);
                }
            }
        }
    }

    for(uint i = 0; i < towers.size(); i++) {
        if(towers[i]->pos.intersects(viewRect)) {
            if(towers[i]->disabled) {
                painter.drawPixmap(towers[i]->rect(), blitzAnimation[towerBlitzAnNum]);
            }
        }
    }

    if((settings->graphicQuality == GRAPHIC_MEDIUM && viewRect.width() < mapwidth/2) ||
                            (settings->graphicQuality == GRAPHIC_LOW && viewRect.width() < mapwidth/3) ||
            settings->graphicQuality == GRAPHIC_HIGH) {
        for(uint i=0;i<enemys.size();i++) {
            QRectF pos = enemys[i]->rectF();
            if(pos.intersects(viewRect)) {
                int health = enemys[i]->health;
                int maxHealth = enemys[i]->maxHealth;
                if(health < maxHealth && health >= 0) {
                    painter.setBrush(QColor(237,28,36));
                    if(health>maxHealth*0.8) {
                        painter.setBrush(QColor(34,177,76));
                    } else if(health>maxHealth*0.6) {
                        painter.setBrush(QColor(181,230,29));
                    } else if(health>maxHealth*0.4) {
                        painter.setBrush(QColor(255,242,0));
                    } else if(health>maxHealth*0.2) {
                        painter.setBrush(QColor(223,89,0));
                    } else if(health <= maxHealth*0.2) {
                        painter.setBrush(QColor(237,28,36));
                    }
                    enemys[i]->updateWidth(0);
                    int width = enemys[i]->healthWidth;//pos.width()*((double)enemys[i]->health/enemys[i]->maxHealth);
                    if(width <= 1) width = 1;
                    painter.drawRect(pos.x(),pos.y()+pos.height()+2,width,5);
                }
            }
        }
        painter.setPen(Qt::white);
        changeSize(painter,12,true);
        for(uint i = 0; i < enemys.size(); i++) {
            if(enemys[i]->type == ENEMY_LEGENDE) {
                QRectF rect = enemys[i]->rectF();
                rect.adjust(-50,0,50,0);
                painter.drawText(rect, Qt::AlignCenter, enemys[i]->name);
            }
        }
        painter.setPen(Qt::NoPen);
    }
    if(towerMenu > -1 || (towerMenuAnimating > -1 && !towerMenuAnimatingDir)) {
        int num = towerMenu;
        if(towerMenuAnimating > -1) {
            num = towerMenuAnimating;
        }
        painter.setOpacity(0.5*fade);
        painter.setBrush(QColor(91,185,28));
        //painter.drawPixmap(tiles[num]->pos,auswahlTile,QRectF(0,0,80,80));
        painter.drawRect(tiles[num]->pos);
        if(chosenTiles.size()) {
            for(uint i = 0; i < chosenTiles.size(); i++) {
                painter.drawRect(tiles[chosenTiles[i]]->pos);
            }
        }
        painter.setOpacity(fade);
        if(tiles[num]->t != nullptr && !chosenTiles.size()) { //upgrade blus anzeigen
            Tower *t = tiles[num]->t;
            if(!t->disabled) {
                if((t->dmglvl < 3 && t->hasDamage) ||
                        (t->speedlvl < 3 && t->hasProjectileSpeed) ||
                        (t->turnlvl < 3 && t->hasTurnSpeed) ||
                        (t->rangelvl < 3 && t->hasRange) ||
                        (t->ratelvl < 3 && t->hasFirerate)) {

                    if(towerMenuSelected == 7) {
                        painter.setOpacity(1);
                    } else painter.setOpacity(0.6);
                    painter.drawPixmap(tiles[num]->t->upgradePos, blus);
                    painter.setOpacity(1);
                }
            }
        }
    }

    for(uint i = 0; i < towers.size(); i++) {
        if(towers[i]->hasRange) {
            if(towers[i]->tmpRange > 0) {
                int range = towers[i]->tmpRange;
                painter.setBrush(Qt::NoBrush);
                QPen pen(Qt::red,2);
                painter.setPen(pen);
                painter.drawEllipse(towers[i]->pos.center(),range,range);
                painter.setPen(Qt::NoPen);
            }
        }
    }

    for(uint i=0;i<projectiles.size();i++) {
        if((settings->graphicQuality == GRAPHIC_MEDIUM && viewRect.width() < mapwidth*0.75) ||
                (settings->graphicQuality == GRAPHIC_LOW && viewRect.width() < mapwidth*0.5) ||
                settings->graphicQuality == GRAPHIC_HIGH) {
            if(projectiles[i] != nullptr) { //nullptr abfrage
                if(projectiles[i]->rect.intersects(viewRect)) {
                    switch(projectiles[i]->type) {
                    case 0:
                        painter.setOpacity(1);
                    //case 4:
                        if(projectiles[i]->type == 4) painter.setOpacity(projectiles[i]->opacity);
                        switch(projectiles[i]->pxID) {
                        case 0: //minus
                        {
                            painter.drawPixmap(projectiles[i]->rect,minus,QRectF(170,170,170,170));
                            QColor c = projectiles[i]->color;
                            if(c.alpha()>1) {
                                painter.setBrush(c);
                                painter.drawEllipse(projectiles[i]->rect);
                            }
                        }
                            break;
                        case 1: //herz
                            painter.drawPixmap(projectiles[i]->rect,herzPx,QRectF(88,112,335,307));
                            break;
                        case 2: //bann
                            painter.setBrush(Engine::getColor(ENEMY_GEBANNT));
                            painter.setPen(Qt::NoPen);
                            painter.drawEllipse(projectiles[i]->rect);
                            break;
                        case 3: //circle 1
                            painter.drawPixmap(projectiles[i]->rect, circleSplit1, QRectF(0,0,250,250));
                            break;
                        case 4: //circle 2
                            painter.drawPixmap(projectiles[i]->rect, circleSplit2, QRectF(0,0,250,250));
                            break;
                        case 5: //circle 3
                            painter.drawPixmap(projectiles[i]->rect, circleSplit3, QRectF(0,0,250,250));
                            break;
                        case 6: //raute 1
                            painter.drawPixmap(projectiles[i]->rect, rauteSplit1, QRectF(0,0,250,250));
                            break;
                        case 7: //raute 2
                            painter.drawPixmap(projectiles[i]->rect, rauteSplit2, QRectF(0,0,250,250));
                            break;
                        case 8: //raute 3
                            painter.drawPixmap(projectiles[i]->rect, rauteSplit3, QRectF(0,0,250,250));
                            break;
                        case 9: //raute 4
                            painter.drawPixmap(projectiles[i]->rect, rauteSplit4, QRectF(0,0,250,250));
                            break;
                        }
                        break;
                    case 1:
                        painter.setOpacity(projectiles[i]->opacity);
                        switch(projectiles[i]->pxID) {
                        case 1:
                            painter.drawPixmap(projectiles[i]->rect,blus_blurred,QRectF(0,0,66,67));
                            break;
                        case 2:
                            //114 114 284x284
                            painter.drawPixmap(projectiles[i]->rect,pr0coinPx,QRectF(114,114,284,284));
                            break;
                        }
                        painter.setOpacity(fade);
                        break;
                    case 2: //Shekel
                        painter.setOpacity(projectiles[i]->opacity);
                        painter.setPen(projectiles[i]->color);
                        changeSize(painter,18,true);
                        painter.drawText(projectiles[i]->rect.x(),projectiles[i]->rect.y(),
                                         projectiles[i]->text);
                        if(projectiles[i]->hasShekelImage) {
                            painter.setPen(Qt::NoPen);
                            QRectF r = projectiles[i]->rect;
                            painter.drawPixmap(r.x()-27,r.y()-22,30,30,pr0coinPx);
                        }
                        painter.setPen(Qt::NoPen);
                        painter.setOpacity(fade);
                        break;
                    case 3: //Giftgas
                        painter.setOpacity(projectiles[i]->opacity);
                        painter.drawPixmap(projectiles[i]->rect,poison_gas,QRectF(0,0,407,407));
                        painter.setOpacity(fade);
                        break;
                    case 6: //Arc
                        painter.setOpacity(projectiles[i]->opacity);
                        painter.setPen(QPen(projectiles[i]->color, projectiles[i]->angle));
                        painter.drawArc(projectiles[i]->rect, 0, 16*360);
                        painter.setPen(Qt::NoPen);
                        painter.setOpacity(fade);
                        break;
                    case 8: //blitz
                        painter.drawPixmap(projectiles[i]->rect, blitzAnimation[projectiles[i]->pxID], QRectF(0,0,512,512));
                        break;
                    }
                }
            }
        }
    }
    if(isLoading() && active != STATE_EDITOR) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(Engine::getColor(-1));
        for(uint i=0;i<tiles.size();i++) {
            if(tiles[i]->pos.intersects(viewRect) && tiles[i]->opacity < 1) {
                painter.setOpacity(1-tiles[i]->opacity);
                painter.drawRect(tiles[i]->rect());
                painter.setOpacity(1);
            }
        }
    }
    if(DEBUG) {
        QElapsedTimer debugElapsed;
        debugElapsed.start();
        changeSize(painter,32,true);
        painter.setPen(Qt::blue);
        for(uint i=0;i<enemys.size();i++) {
            painter.drawLine(tiles[paths[enemys[i]->path].path[enemys[i]->cpos]]->rect().center(),enemys[i]->rect().center());
        }
        for(uint i=0;i<towers.size();i++) {
            if(towers[i]->target!=nullptr) {
                QPointF center = towers[i]->pos.center();
                QPointF enemy = towers[i]->target->rectF().center();
                painter.setPen(QPen(Qt::red,3));
                painter.drawLine(center,enemy);
            }
        }
        debugns = debugElapsed.nsecsElapsed();
    }
}

void FrmMain::drawHUD(QPainter &painter)
{

    if(active == STATE_PLAYING) {

        QRect pcr = pr0coinRect.toRect();
        if(shekelCoinSize) {
            pcr.adjust(-shekelCoinSize,-shekelCoinSize,shekelCoinSize,shekelCoinSize);
        }
        painter.drawPixmap(pcr,pr0coinPx);
        painter.drawPixmap(5,75,90,90,baseHerzPx);
        painter.drawPixmap(1800,0,100,100,wavesPx);

        for(uint i = 0; i < projectiles.size(); i++) {
            if(!projectiles[i]->del) {
                if(projectiles[i]->type == 5) {
                    QRectF r = projectiles[i]->rect;
                    int width = r.width()*2;
                    painter.drawPixmap(r.x()-width/2,r.y()-width/2,width,width,pr0coinPx);
                }
            }
        }

        drawTowerMenu(painter);
        painter.setPen(Qt::white);
        changeSize(painter,56,true);
        painter.drawText(85,70,QString::number(benis));
        painter.drawText(85,140,QString::number(basehp));
        painter.drawText(QRect(1600,20,200,60),Qt::AlignRight,QString::number(currentWave.waveCount));
        if(!waveBreak) {
            switch(playingSpeed) {
            case 0:
                painter.drawPixmap(btnPlayingSpeedRect,btnStopPx);
                painter.drawText(1725,895,"Start");
                break;
            case 1:
                painter.drawPixmap(btnPlayingSpeedRect,btnNormalPx);
                break;
            case 2:
                painter.drawPixmap(btnPlayingSpeedRect,btnFastPx);
                break;
            case 3:
                painter.drawPixmap(btnPlayingSpeedRect,btnSuperfastPx);
                break;
            }
        } else {
            painter.drawPixmap(btnPlayingSpeedRect,waveBreakPx);
            changeSize(painter,64, true);
            painter.setPen(Qt::white);
            QRect r = btnPlayingSpeedRect;
            r.adjust(0,-25,0,0);
            painter.drawText(r, Qt::AlignCenter, QString::number(waveBreak/1000));
            changeSize(painter,48, true);
            painter.drawText(btnPlayingSpeedRect, Qt::AlignCenter | Qt::AlignBottom, "SKIP");
            painter.setPen(Qt::NoPen);
        }

        if(settings->zoomMode == 1) {
            painter.drawPixmap(btnZoomPlusRect, shekelPlusPx);
            painter.drawPixmap(btnZoomMinusRect, minusButtonPx);
        }

        if(towerMenu == -1) {
            for(uint i = 0; i < shop->items.size(); i++) {
                if(shop->items[i].hasCount) {
                    if(powerupSelected == (int)i) {
                        painter.setOpacity(1);
                    } else painter.setOpacity(0.6);

                    switch(i) {
                    case 1:
                        painter.drawPixmap(wandRect, wandAnimation[0]);
                    }

                    if(shop->items[i].count > 0) {
                        painter.setPen(Qt::white);
                    } else {
                        painter.setPen(Qt::red);
                    }

                    f.setPixelSize(24);

                    painter.drawText(50 + (100 * (i-1)), 1070, "x" + QString::number(shop->items[i].count));

                }
            }
        }

        painter.setOpacity(1);
        if(enemyWarning && playingMode == MODE_MAZE) {
            changeSize(painter,64,true);
            painter.setPen(Qt::red);
            painter.drawText(QRect(0,700,1920,300), Qt::AlignCenter, "Warnung: Gegner außerhalb vom neuen Pfad!");
        }

        painter.setPen(Qt::NoPen);
        /*if(playingSpeed && canStartNextWave) {
            painter.setOpacity(1);
        } else painter.setOpacity(0.6);
        painter.drawPixmap(btnNextWaveRect, wavesPx);
        painter.setOpacity(1);*/

    } else if(active == STATE_EDITOR) {
        drawEditorMenu(painter);
    }
}

void FrmMain::drawTowerMenu(QPainter &painter)
{
    if((towerMenu > -1 || towerMenuAnimating > -1) && (!towerMenuHidden || towerMenuRectAnimation.x() > -600)) {
        painter.setOpacity(0.6*fade);
        painter.setBrush(Qt::black);
        double offX = 0;
        int towerNum = towerMenu;
        if(towerMenuAnimating == -1) {
            painter.drawRect(towerMenuRect);
        } else {
            towerNum = towerMenuAnimating;
            offX = towerMenuRectAnimation.x();
            painter.drawRect(towerMenuRectAnimation);
        }
        if(towerMenuSelected) {
            painter.setOpacity(fade);
        }
        painter.setOpacity(fade);
        changeSize(painter,38,true);
        painter.setPen(Qt::white);

        if(!towerMenuHidden) {
            painter.drawPixmap(offX+towerMenuCloseRect.x(), towerMenuCloseRect.y(), 100, 100, towerMenuClosePx);
        }

        Tower *tmpTower = tiles[towerNum]->t;
        if(tmpTower != nullptr) {//UpgradeMenü
            changeSize(painter,46,true);
            painter.setPen(Qt::white);
            painter.drawText(QRect(offX+0,150,600,100),Qt::AlignCenter,tmpTower->name);
            changeSize(painter,36,true);
            QString o = "Schaden:";
            QString o2 = QString::number(tmpTower->dmg);
            QString o3;
            QString o4 = QString::number(tmpTower->range);
            QString o5 = QString::number(((360/tmpTower->angleSpeed)*10)/1000,'f',2)+"s";
            QString o6 = QString::number(tmpTower->pspeed*10);
            if(o2 == "0") o2 = "/";
            if(o4 == "0") o4 = "/";
            if(o5 == "0.00") o5 = "/";
            if(o6 == "0") o6 = "/";
            if(tmpTower->type==2) {
                o = "Betäubungsdauer:";
                o2 = QString::number(tmpTower->stun/1000.0,'f',2)+"s";
            } else if(tmpTower->type==3) {
                o = "Repostdauer:";
                o2 = QString::number(tmpTower->repost/1000.0,'f',2)+"s";
            } else if(tmpTower->type==4) {
                o = "Benisanzahl:";
            } else if(tmpTower->type==5) {
                o = "Schaden:";
                o2 = "∞";
                //o4 = o2;
                //o3 = " - " + QString::number(towers[tnum]->reloadMax/1000.0,'f',2) + "s";
            }
            painter.drawText(QRect(offX+10,230,450,300),Qt::AlignLeft,o+"\n"
                                                                   "Reichweite:\n"
                                                                   "Nachladezeit:\n"
                                                                   "Projektilgeschwindigkeit:\n"
                                                                   "Drehgeschwindigkeit:");
            painter.drawText(QRect(offX+450,230,140,300),Qt::AlignRight,o2+"\n"+
                             o4+"\n"+
                             QString::number(tmpTower->reload/1000.0,'f',2)+"s"+o3+"\n"+
                             o6+"\n"+
                             o5);
            //
            painter.setPen(Qt::NoPen);
            if(!tmpTower->disabled) {
                QRect tmp;
                if(tmpTower->hasDamage) {
                    if(towerMenuSelected != 1) {
                        painter.setOpacity(0.6 * fade);
                    }
                    tmp = btnDmgRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnDmg);
                    drawBar(painter,tmpTower,1);
                } else {
                    painter.setOpacity(0.6 * fade);
                    tmp = btnDmgRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnDmg_grau);
                }

                if(tmpTower->hasRange) {
                    if(towerMenuSelected != 2) {
                        painter.setOpacity(0.6 * fade);
                    } else painter.setOpacity(fade);
                    tmp = btnRangeRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnRange);
                    drawBar(painter,tmpTower,2);
                } else {
                    painter.setOpacity(0.6 * fade);
                    tmp = btnRangeRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnrange_grau);
                }

                if(tmpTower->hasFirerate) {
                    if(towerMenuSelected != 3) {
                        painter.setOpacity(0.6 * fade);
                    } else painter.setOpacity(fade);
                    tmp = btnFirerateRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnFirerate);
                    drawBar(painter,tmpTower,3);
                } else {
                    painter.setOpacity(0.6 * fade);
                    tmp = btnFirerateRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnFirerate_grau);
                }

                if(tmpTower->hasProjectileSpeed) {
                    if(towerMenuSelected != 4) {
                        painter.setOpacity(0.6 * fade);
                    } else painter.setOpacity(fade);
                    tmp = btnSpeedRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnSpeed);
                    drawBar(painter,tmpTower,4);
                } else {
                    painter.setOpacity(0.6 * fade);
                    tmp = btnSpeedRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnSpeed_grau);
                }

                if(!tmpTower->sellingDisabled) {
                    if(towerMenuSelected != 5) {
                        painter.setOpacity(0.6 * fade);
                    } else painter.setOpacity(fade);
                    tmp = btnSellRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnSell);
                } else {
                    painter.setOpacity(0.6 * fade);
                    tmp = btnSellRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnSell_grau);
                }

                if(tmpTower->hasTurnSpeed) {
                    if(towerMenuSelected != 6) {
                        painter.setOpacity(0.6 * fade);
                    } else painter.setOpacity(fade);
                    tmp = btnTurnRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnTurn);
                    drawBar(painter,tmpTower,6);
                } else {
                    painter.setOpacity(0.6 * fade);
                    tmp = btnTurnRect;
                    tmp.moveTo(offX + tmp.x(), tmp.y());
                    painter.drawPixmap(tmp,btnTurn_grau);
                }

                painter.setOpacity(fade);
                painter.setPen(Qt::white);
                changeSize(painter,38,true);
                int maxlvl = 1;
                switch(towerMenuSelected) {
                case 1: //dmg
                {
                    upgradeCost = tmpTower->calcFullUpgradeCost(upgradeHighConst,upgradeLowConst,1);
                    maxlvl = tmpTower->dmglvl;
                    switch(tmpTower->type) {
                    case TOWER_MINUS: //minus
                    case TOWER_SNIPER: //Sniper
                    case TOWER_LASER: //laser
                    case TOWER_POISON: //poison
                    case TOWER_MINIGUN: //minigun
                        painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                        changeSize(painter,38,false);
                        painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                             "Erhöht den Schaden");
                        break;
                    case TOWER_HERZ: //fav
                        painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                        changeSize(painter,38,false);
                        painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                             "Erhöht die Betäubungsdauer");
                        break;
                    case TOWER_REPOST: //repost
                        painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                        changeSize(painter,38,false);
                        painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                             "Erhöht die Repostdauer");
                        break;
                    case TOWER_BENIS: //benis
                        painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                        changeSize(painter,38,false);
                        painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                             "Erhöht die Benisanzahl");
                        break;
                    }
                    break;
                }
                case 2: //range
                    upgradeCost = tmpTower->calcFullUpgradeCost(upgradeHighConst,upgradeLowConst,2);
                    maxlvl = tmpTower->rangelvl;
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                    changeSize(painter,38,false);
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                         "Erhöht die Reichweite");
                    break;
                case 3: //feuerrate
                    upgradeCost = tmpTower->calcFullUpgradeCost(upgradeHighConst,upgradeLowConst,3);
                    maxlvl = tmpTower->ratelvl;
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                    changeSize(painter,38,false);
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                         "Verringert die Nachladezeit");
                    break;
                case 4: //speed
                    upgradeCost = tmpTower->calcFullUpgradeCost(upgradeHighConst,upgradeLowConst,4);
                    maxlvl = tmpTower->speedlvl;
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                    changeSize(painter,38,false);
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                         "Erhöht die Geschwindigkeit\n"
                                                                         "von Projektilen");
                    break;
                case 5: //sell
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                    changeSize(painter,38,false);
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                         "Tower verkaufen");
                    break;
                case 6: //turnspeed
                    upgradeCost = tmpTower->calcFullUpgradeCost(upgradeHighConst,upgradeLowConst,5);
                    maxlvl = tmpTower->turnlvl;
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                    changeSize(painter,38,false);
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                         "Erhöht die Rotations-\n"
                                                                         "geschwindigkeit");
                    break;
                case 7: //upgrade-full
                    upgradeCost = tmpTower->calcFullUpgradeCost(upgradeHighConst, upgradeLowConst);
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                    changeSize(painter,38,false);
                    painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                         "Maximales Turm-\n"
                                                                         "Upgrade");
                }

                if(towerMenuSelected > 0 &&
                        towerMenuSelected < 8 &&
                        towerMenuSelected != 5 &&
                        maxlvl < 3) {
                    painter.drawPixmap(QRect(offX + 0,710,100,100), pr0coinPx);
                    changeSize(painter,38,true);
                    if(benis >= upgradeCost) {
                        painter.setPen(Qt::green);
                    } else {
                        painter.setPen(Qt::red);
                    }
                    painter.drawText(QRect(offX + 80,742,100,100),QString::number(upgradeCost));
                }
            } else {
                if(tmpTower->disabled < disabledTime) {
                    painter.drawPixmap(QRect(offX+repairButtonRect.x(),
                                             repairButtonRect.y(),
                                             200,
                                             200), repairPx_grau);
                    painter.setPen(QPen(Qt::white, 15));
                    int startAngle = 90;
                    int stopAngle = -((1-(double)tmpTower->disabled/disabledTime)*360);
                    painter.drawArc(QRect(offX+repairButtonRect.x()-50,
                                          repairButtonRect.y()-50,
                                          200+100,
                                          200+100), 16*startAngle, 16*stopAngle);
                    painter.setPen(Qt::NoPen);
                } else {
                    painter.drawPixmap(QRect(offX+repairButtonRect.x(),
                                             repairButtonRect.y(),
                                             200,
                                             200), repairPx);
                }
            }

            painter.setPen(Qt::NoPen);
        } else { //Baumenü
            if(towerMenuAnimating > -1) {
                painter.translate(towerMenuRectAnimation.x(), 0);
            }

            drawTower(minusTowerRect,painter,nullptr,-1);
            drawTower(favTowerRect,painter,nullptr,-2);

            if(!shop->towerLocks[2]) {
                drawTower(repostTowerRect,painter,nullptr,-3);
                if(shop->towerPrices[2]) painter.drawPixmap(repostTowerRect, lock_quadratPx);
            }

            if(!shop->towerLocks[3]) {
                drawTower(benisTowerRect,painter,nullptr,-4);
                if(shop->towerPrices[3]) painter.drawPixmap(benisTowerRect, lock_quadratPx);
            }

            if(!shop->towerLocks[4]) {
                drawTower(banTowerRect,painter,nullptr,-5);
                if(shop->towerPrices[4]) painter.drawPixmap(banTowerRect, lock_quadratPx);
            }

            if(!shop->towerLocks[5]) {
                drawTower(sniperTowerRect,painter,nullptr,-6);
                if(shop->towerPrices[5]) painter.drawPixmap(sniperTowerRect, lock_quadratPx);
            }

            if(!shop->towerLocks[6]) {
                drawTower(flakTowerRect,painter,nullptr,-7);
                if(shop->towerPrices[6]) painter.drawPixmap(flakTowerRect, lock_quadratPx);
            }

            if(!shop->towerLocks[7]) {
                drawTower(laserTowerRect,painter,nullptr,-8);
                if(shop->towerPrices[7]) painter.drawPixmap(laserTowerRect, lock_quadratPx);
            }

            if(!shop->towerLocks[8]) {
                drawTower(poisonTowerRect,painter,nullptr,-9);
                if(shop->towerPrices[8]) painter.drawPixmap(poisonTowerRect, lock_quadratPx);
            }

            if(!shop->towerLocks[9]) {
                drawTower(minigunTowerRect,painter,nullptr,-10);
                if(shop->towerPrices[9]) painter.drawPixmap(minigunTowerRect, lock_quadratPx);
            }

            if(towerMenuAnimating > -1) {
                painter.translate(-towerMenuRectAnimation.x(), 0);
            }
        }
    } else if(towerMenu > -1 && towerMenuHidden) {
        painter.drawPixmap(towerMenuOpenRect, towerMenuOpenPx);
    }
}

void FrmMain::drawTower(QRect pos2, QPainter &painter, Tower *tmpTower, int info)
{
    //pixelsize = 36
    double offX = 0;
    if(towerMenuAnimating > -1 && tmpTower == nullptr) {
        offX = towerMenuRectAnimation.x();
        //pos.moveTo(offX + pos.x(), pos.y());
    }
    offX = 0;

    bool menu = true;
    QRectF posF;
    QRectF pos = pos2;
    //painter.drawPixmap(pos,towerGroundPx);
    Tower *t = tmpTower;
    int tnum = info;
    if(t!=nullptr) {
        posF = t->pos;
        pos = posF;
        tnum = t->type;
        menu=false;
    } else {
        tnum = abs(tnum);
    }
    painter.setPen(Qt::white);
    QRect infoRect = QRect(offX + 25,945,550,200);
    QRect costPxPos = QRect(offX + 0,870,100,100);
    QPoint costPos = QPoint(offX + 80,costPxPos.y()+65);
    if(info && towerMenuSelected == tnum) {
        painter.setPen(Qt::white);
        changeSize(painter,38,true);
        painter.drawText(infoRect,Qt::AlignLeft, Tower::getInfo(tnum, 0));
        changeSize(painter,38,false);
        painter.drawText(infoRect,Qt::AlignLeft, Tower::getInfo(tnum, 1));
    }
    uint cost = 0;
    switch(tnum) {
    case TOWER_MINUS: //minustower
        cost = minusTowerCost;
        if(chosenTiles.size()) cost += minusTowerCost*chosenTiles.size();
        if(info && towerMenuSelected==tnum) { //towerinfo
            if(benis >= cost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(cost));
        }
        painter.setPen(Qt::NoPen);
        if(t != nullptr) {
            painter.setBrush(Engine::getColor(ENEMY_NEUSCHWUCHTEL));
            painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
            painter.drawPixmap(pos,towerGroundPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.save();
            if(t) {
                painter.translate(pos.center());
                painter.rotate(t->angle);
                painter.translate(-pos.center().x(),-pos.center().y());
            }
            painter.drawPixmap(pos,minusTowerPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.restore();
        } else {
            painter.drawPixmap(minusTowerRect, towerTemplates[0]);
        }
        painter.setOpacity(fade);
        break;
    case TOWER_HERZ: //favoritentower
        cost = herzTowerCost;
        if(chosenTiles.size()) cost += herzTowerCost*chosenTiles.size();
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis >= cost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(cost));
        }
        painter.setPen(Qt::NoPen);
        if(t != nullptr) {
            painter.setBrush(Engine::getColor(ENEMY_NEUSCHWUCHTEL));
            painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
            painter.drawPixmap(pos,towerGroundPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.save();
            if(t) {
                painter.translate(pos.center());
                painter.rotate(t->angle);
                painter.translate(-pos.center().x(),-pos.center().y());
            }
            painter.drawPixmap(pos,favTowerPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.restore();
        } else {
            painter.drawPixmap(favTowerRect, towerTemplates[1]);
        }
        painter.setOpacity(fade);
        break;
    case TOWER_REPOST: {//reposttower
        cost = repostTowerCost;
        if(chosenTiles.size()) cost += repostTowerCost*chosenTiles.size();
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis >= cost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(cost));
        }
        int num = 0;
        if(t != nullptr) {
            num = t->animation;
        }
        painter.setPen(Qt::NoPen);
        //painter.setBrush(Engine::neuschwuchtel);
        //painter.drawEllipse((QPointF)pos.center(),pos.width()*0.36,pos.height()*0.36);
        painter.drawPixmap(pos,repostAnimation[num], QRectF(0,0,towerPxSize,towerPxSize));
        break; }
    case TOWER_BENIS: {//benistower
        cost = benisTowerCost;
        if(chosenTiles.size()) cost += benisTowerCost*chosenTiles.size();
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis >= cost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(cost));
        }
        painter.setPen(Qt::NoPen);
        if(t != nullptr) {
            painter.setBrush(Engine::getColor(ENEMY_NEUSCHWUCHTEL));
            painter.drawEllipse((QPointF)pos.center(),pos.width()*0.36,pos.height()*0.36);
            painter.drawPixmap(pos,towerGroundPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.save();
            if(t != nullptr) {
                painter.translate(posF.center());
            } else {
                painter.translate(pos.center());
            }
            if(t != nullptr) {
                int rotation = (1-(double)t->currentReload/t->reload)*360;
                painter.rotate(rotation);
            }
            painter.setBrush(Qt::red);
            painter.drawRect(-2,-2,4,4);
            if(t != nullptr) {
                painter.translate(-posF.center().x(),-posF.center().y());
                painter.drawPixmap(posF,pr0coinPx,QRectF(0,0,towerPxSize,towerPxSize));
            } else {
                painter.translate(-pos.center().x(),-pos.center().y());
                painter.drawPixmap(pos,pr0coinPx, QRectF(0,0,towerPxSize,towerPxSize));
            }
            painter.restore();
        } else {
            painter.drawPixmap(benisTowerRect, towerTemplates[3]);
        }
        break; }
    case TOWER_BAN: //bantower
        cost = banTowerCost;
        if(chosenTiles.size()) cost += banTowerCost*chosenTiles.size();
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis >= cost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(cost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(Engine::getColor(ENEMY_GEBANNT));
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.36,pos.height()*0.36);
        painter.drawPixmap(pos,towerGroundPx, QRectF(0,0,towerPxSize,towerPxSize));
        break;
    case TOWER_SNIPER: //snipertower
        cost = sniperTowerCost;
        if(chosenTiles.size()) cost += sniperTowerCost*chosenTiles.size();
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis >= cost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(cost));
        }
        painter.setPen(Qt::NoPen);
        if(t != nullptr) {
            painter.setBrush(Engine::getColor(ENEMY_NEUSCHWUCHTEL));
            painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
            painter.drawPixmap(pos,towerGroundPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.save();
            if(t) {
                painter.translate(pos.center());
                painter.rotate(t->angle);
                painter.translate(-pos.center().x(),-pos.center().y());
            }
            painter.drawPixmap(pos,sniperPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.restore();
        } else {
            painter.drawPixmap(sniperTowerRect, towerTemplates[5]);
        }
        painter.setOpacity(fade);
        break;
    case TOWER_FLAK: //flaktower
        cost = flakTowerCost;
        if(chosenTiles.size()) cost += flakTowerCost*chosenTiles.size();
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis >= cost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(cost));
        }
        painter.setPen(Qt::NoPen);
        if(t != nullptr) {
            painter.setBrush(Engine::getColor(ENEMY_NEUSCHWUCHTEL));
            painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
            painter.drawPixmap(pos,towerGroundPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.save();
            if(t) {
                painter.translate(pos.center());
                painter.rotate(t->angle);
                painter.translate(-pos.center().x(),-pos.center().y());
            }
            painter.drawPixmap(pos,flakTowerPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.restore();
        } else {
            painter.drawPixmap(flakTowerRect, towerTemplates[6]);
        }
        painter.setOpacity(fade);
        break;
    case TOWER_LASER: //lasertower
        cost = laserTowerCost;
        if(chosenTiles.size()) cost += laserTowerCost*chosenTiles.size();
        if(info && towerMenuSelected == tnum) { //towerinfo
            if(benis >= cost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(cost));
        }
        painter.setPen(Qt::NoPen);
        if(t != nullptr) {
            painter.setBrush(Engine::getColor(ENEMY_NEUSCHWUCHTEL));
            painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
            painter.drawPixmap(pos,towerGroundPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.save();
            if(t) {
                painter.translate(pos.center());
                painter.rotate(t->angle);
                painter.translate(-pos.center().x(),-pos.center().y());
            }
            painter.drawPixmap(pos,laserTowerPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.restore();
        } else {
            painter.drawPixmap(laserTowerRect, towerTemplates[7]);
        }
        painter.setOpacity(fade);
        break;
    case TOWER_POISON: //Gifttower
        cost = poisonTowerCost;
        if(chosenTiles.size()) cost += poisonTowerCost*chosenTiles.size();
        if(info && towerMenuSelected == tnum) { //towerinfo
            if(benis >= cost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(cost));
        }
        painter.setPen(Qt::NoPen);
        if(t != nullptr) {
            painter.setBrush(Engine::getColor(ENEMY_NEUSCHWUCHTEL));
            painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
            painter.drawPixmap(pos,towerGroundPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.save();
            if(t) {
                painter.translate(pos.center());
                painter.rotate(t->angle);
                painter.translate(-pos.center().x(),-pos.center().y());
            }
            painter.drawPixmap(pos,poisonTowerPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.restore();
        } else {
            painter.drawPixmap(poisonTowerRect, towerTemplates[8]);
        }
        painter.setOpacity(fade);
        break;
    case TOWER_MINIGUN:
        cost = minigunTowerCost;
        if(chosenTiles.size()) cost += minigunTowerCost*chosenTiles.size();
        if(info && towerMenuSelected == tnum) { //towerinfo
            if(benis >= cost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(cost));
        }
        painter.setPen(Qt::NoPen);
        if(t != nullptr) {
            painter.setBrush(Engine::getColor(ENEMY_NEUSCHWUCHTEL));
            painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
            painter.drawPixmap(pos,towerGroundPx, QRectF(0,0,towerPxSize,towerPxSize));
            painter.save();
            if(t) {
                painter.translate(pos.center());
                painter.rotate(t->angle);
                painter.translate(-pos.center().x(),-pos.center().y());
                painter.drawPixmap(pos,minigunTowerPx[t->animation], QRectF(0,0,512,512));
            } else  painter.drawPixmap(pos,minigunTowerPx[1], QRectF(0,0,towerPxSize,towerPxSize));
            painter.restore();
        } else {
            painter.drawPixmap(minigunTowerRect, towerTemplates[9]);
        }
        painter.setOpacity(fade);
        break;
    }
    changeSize(painter,38,true);
    painter.setPen(Qt::NoPen);
    if(towerMenuSelected == tnum && menu) {
        pos.adjust(-10,-10,10,10);
        painter.drawPixmap(pos2,auswahlPx);
        painter.drawPixmap(costPxPos,pr0coinPx);
    }
}

void FrmMain::drawBar(QPainter &painter, Tower *t, int num)
{
    int anOffx = 0;
    if(towerMenuAnimating > -1) {
        anOffx = towerMenuRectAnimation.x();
    }
    int offsetX = 95;
    int ofX2 = 62;
    int width = 41;
    switch(num) {
    case 1: //dmg
        for(int i=0;i<t->dmglvl;i++) {
            painter.drawPixmap(anOffx + (btnDmgRect.x()+offsetX+(i*ofX2)),btnDmgRect.y()+9,width,67,barPx);
        }
        break;
    case 2: //range
        for(int i=0;i<t->rangelvl;i++) {
            painter.drawPixmap(anOffx + (btnRangeRect.x()+offsetX+(i*ofX2)),btnRangeRect.y()+9,width,67,barPx);
        }
        break;
    case 3: //firerate
        for(int i=0;i<t->ratelvl;i++) {
            painter.drawPixmap(anOffx + (btnFirerateRect.x()+offsetX+(i*ofX2)),btnFirerateRect.y()+9,width,67,barPx);
        }
        break;
    case 4: //bspeed
        for(int i=0;i<t->speedlvl;i++) {
            painter.drawPixmap(anOffx + (btnSpeedRect.x()+offsetX+(i*ofX2)),btnSpeedRect.y()+9,width,67,barPx);
        }
        break;
    case 6: //turnspeed
        for(int i=0;i<t->turnlvl;i++) {
            painter.drawPixmap(anOffx + (btnTurnRect.x()+offsetX+(i*ofX2)),btnTurnRect.y()+9,width,67,barPx);
        }
        break;
    }
}

void FrmMain::drawMainMenu(QPainter &painter)
{
    painter.setPen(Qt::NoPen);
    painter.drawPixmap(0,0,1920,1080,menuPx);
    //Animationen
    for(uint i=0;i<projectiles.size();i++) {
        switch(projectiles[i]->type) {
        case 1:
            painter.setOpacity(fade*projectiles[i]->opacity);
            painter.drawPixmap(projectiles[i]->rect,blus_blurred,QRectF(0,0,66,67));
            painter.setOpacity(fade);
            break;
        case 2:
            painter.setOpacity(fade*projectiles[i]->opacity);
            painter.setPen(Qt::black);
            changeSize(painter,128,false);
            painter.drawText(projectiles[i]->rect.x(),projectiles[i]->rect.y(),
                             projectiles[i]->text);
            painter.setPen(Qt::NoPen);
            painter.setOpacity(fade);
            break;
        }
    }
    //
    painter.drawPixmap(0,0,1600,325,titlePx);
    if(hasPlayingSave) {
        painter.drawPixmap(btnContinueMenuRect,btnContinue);
    } else {
        painter.drawPixmap(btnContinueMenuRect,btnContinueGrey);
    }
    if(soundEnabled) {
        painter.drawPixmap(btnSoundRect,soundAnPx);
    } else {
        painter.drawPixmap(btnSoundRect,soundAusPx);
    }
    painter.drawPixmap(startRect,startPx);
    painter.drawPixmap(editRect,editPx);
    painter.drawPixmap(shopRect,shopPx);
    painter.drawPixmap(exitRect,exitPx);
    switch(subActive) {
    case 1: //play ausgewählt - levelanzeige
        //Standard
        if(subActiveSelected!=1) {
            painter.setOpacity(0.6*fade);
        }
        /*QRect(500,400 300x168)
        QRect(825,400 300x168)*/
        painter.drawPixmap(levels[0].levelRect,levelsPx);
        //Eigene
        if(subActiveSelected!=2) {
            painter.setOpacity(0.6*fade);
        } else {
            painter.setOpacity(fade);
        }
        painter.drawPixmap(levels[1].levelRect,ownMapsPx);
        //Community
        /*if(subActiveSelected!=3) {
            painter.setOpacity(0.6*fade);
        } else {
            painter.setOpacity(fade);
        }
        painter.drawPixmap(levels[2].levelRect,communityPx);*/
        //Play+Del Buttons
        if(!subLevelSelected) {
            painter.setOpacity(0.6*fade);
        } else {
            painter.setOpacity(fade);
        }
        painter.drawPixmap(menuDelRect,mPlayPx);
        painter.setOpacity(fade);
        //Levelanzeige
        switch(subActiveSelected) {
        case 1: //levels
            if(playingMode == MODE_NORMAL) {
                painter.drawPixmap(playingModeRect, playingModeNormal);
                for(uint i = 0;i <  mainLevels;i++) {
                    if(!lvlPrices[i]) {
                        if(i+4!=subLevelSelected) painter.setOpacity(0.6*fade);
                        painter.drawPixmap(mainLvlRects[i],lvlPreviews[i]);
                        painter.setOpacity(fade);
                    }
                }
            } else if(playingMode == MODE_MAZE) {
                painter.drawPixmap(playingModeRect, playingModeMaze);
                for(uint i = 0;i <  mazeLevels;i++) {
                    if(!shop->mazelvlPrices[i]) {
                        if(i+4!=subLevelSelected) painter.setOpacity(0.6*fade);
                        painter.drawPixmap(mainLvlRects[i],mazelvlPreviews[i]);
                        painter.setOpacity(fade);
                    }
                }
            }
            break;
        case 2: //eigene
            for(uint i=3;i<levels.size();i++) {
                if(i+1!=subLevelSelected) painter.setOpacity(0.6*fade);
                painter.drawPixmap(levels[i].levelRect,levels[i].lvlImage);
                painter.setOpacity(fade);
            }
            break;
        case 3: //community

            break;
        }
        break;
    case 2: //editor ausgewählt - levelanzeige
        if(subActiveSelected!=1) {
            painter.setOpacity(0.6*fade);
        }
        painter.drawPixmap(levels[0].levelRect,levels[0].lvlImage);
        painter.setOpacity(fade);
        if(subActiveSelected!=2) {
            painter.setOpacity(0.6*fade);
        }
        painter.drawPixmap(levels[1].levelRect,levels[1].lvlImage);
        painter.setOpacity(fade);
        if(!mediumUnlocked&&!largeUnlocked) {
            painter.drawPixmap(levels[1].levelRect,lockPx);
        }
        if(subActiveSelected!=3) {
            painter.setOpacity(0.6*fade);
        }
        painter.drawPixmap(levels[2].levelRect,levels[2].lvlImage);
        painter.setOpacity(fade);
        if(!largeUnlocked) {
            painter.drawPixmap(levels[2].levelRect,lockPx);
        }
        for(uint i=3;i<levels.size();i++) {
            if(i+1!=subActiveSelected) painter.setOpacity(0.6*fade);
            painter.drawPixmap(levels[i].levelRect,levels[i].lvlImage);
            painter.setOpacity(fade);
        }
        if(!subActiveSelected) painter.setOpacity(0.6*fade);
        painter.drawPixmap(menuPlayRect,mEditPx);
        if(subActiveSelected<4) painter.setOpacity(0.6*fade);
        painter.drawPixmap(menuDelRect,delPx);
        painter.setOpacity(fade);
        break;
    case 3: //shop
        shop->drawShop(painter);
        break;
    case 4: //einstellungen
        settings->drawSettings(painter);
        break;
    case 5: //statistiken & account
        statistics->drawStatistics(painter);
        break;
    }
    QRect pcr = shekelRect;
    if(shekelCoinSize) {
        pcr.adjust(-shekelCoinSize,-shekelCoinSize,shekelCoinSize,shekelCoinSize);
    }
    painter.drawPixmap(pcr, shekelMoney);
    painter.drawPixmap(shekelPlusRect, shekelPlusPx);

    switch(account.accountState) {
    case ACCOUNT_PENDING:
        painter.drawPixmap(accountRect, account_success_pendingPx);
        break;
    case ACCOUNT_ACTIVE:
    case ACCOUNT_VERIFIED:
    case ACCOUNT_CHEATER: //man will ja nicht dass er was merkt :D
        painter.drawPixmap(accountRect, account_successPx);
        break;
    default:
        painter.drawPixmap(accountRect, account_failPx);
        break;
    }
    painter.drawPixmap(statsRect, statsPx);
    painter.drawPixmap(questionRect, questionPx);
    painter.drawPixmap(settingsRect, settingsPx);
    painter.setPen(Engine::getColor(ENEMY_SPENDER));
    changeSize(painter,48,true);
    painter.drawText(shekelRect.right() + 15, shekelRect.y() + 42, QString::number(shop->shekel));
    painter.setPen(Qt::white);
    changeSize(painter,32,true);
    painter.drawText(5,1070,QString::number(version));
}

void FrmMain::drawEditor(QPainter &painter)
{
    for(uint i=0;i<tiles.size();i++) {
        if(tiles[i]->pos.intersects(viewRect)) {
            QPixmap p;
            switch(tiles[i]->type) {
            case 1: //weg
                p = pathTextures[0];
                break;
            case 2: //turmplatz
                p = turmtile;
                break;
            case 3: //leer
                p = emptytile;
                break;
            case 4: //enemybase
                p = enemybasePx;
                break;
            case 5: //ownbase
                p = ownbasePx;
                break;
            case 6: //maptile
                p = mapTile;
                break;
            }
            //if(tiles[i]->type!=6) {
                painter.drawPixmap(tiles[i]->rect(),p);
            //}
        }
    }
    //if(mCompileError) {
        painter.setOpacity(0.4*fade);
        for(uint i=0;i<paths.size();i++) {
            Path tmpPath = paths[i];
            for(uint a=0;a<tmpPath.path.size();a++) {
                painter.setBrush(Qt::red);
                painter.drawRect(tiles[tmpPath.path[a]]->pos);
            }
        }
        painter.setOpacity(fade);
    //}
}

void FrmMain::drawEditorMenu(QPainter &painter)
{
    if(editorSelected!=1) painter.setOpacity(0.6*fade);
    painter.drawPixmap(mPathRect,pathTextures[0]);
    if(editorSelected!=2) {
        painter.setOpacity(0.6*fade);
    } else {
        painter.setOpacity(1*fade);
    }
    painter.drawPixmap(mTurmTileRect,turmtile);
    if(editorSelected!=3) {
        painter.setOpacity(0.6*fade);
    } else {
        painter.setOpacity(1*fade);
    }
    painter.drawPixmap(mEmptyTileRect,emptytile);
    if(editorSelected!=5) {
        painter.setOpacity(0.6*fade);
    } else {
        painter.setOpacity(1*fade);
    }
    painter.drawPixmap(mBaseRect,ownbasePx);
    if(editorSelected!=4) {
        painter.setOpacity(0.6*fade);
    } else {
        painter.setOpacity(1*fade);
    }
    painter.drawPixmap(mEnemyBaseRect,enemybasePx);
    if(editorSelected!=6) {
        painter.setOpacity(0.6*fade);
    } else {
        painter.setOpacity(1*fade);
    }
    painter.drawPixmap(mClearRect,mapTile);

    if(playingMode == MODE_MAZE) {
        if(editorSelected!=7) {
            painter.setOpacity(0.6*fade);
        } else {
            painter.setOpacity(1*fade);
        }
        painter.drawPixmap(mWaypointRect,emptytile);
    }


    painter.setOpacity(fade);
    if(!mapPlaying) {
        painter.drawPixmap(mPlayRect,mPlayPx);
    } else {
        painter.drawPixmap(mPlayRect,mStopPx);
    }
    if(!paths.size()||mCompileError) painter.setOpacity(0.5*fade);
    painter.drawPixmap(mSaveRect,mSavePx);
    painter.setOpacity(fade);

    if(!editMode) {
        painter.drawPixmap(menuDelRect, movePx);
    } else {
        painter.drawPixmap(menuDelRect, mEditPx);
    }
}

void FrmMain::drawBackMenu(QPainter &painter)
{
    painter.setBrush(Engine::getColor(-1));
    painter.setOpacity(0.5);
    painter.drawRect(0, 0, 1920, 1080);
    painter.setOpacity(1);

    painter.drawPixmap(btnContinueRect,btnContinue);
    painter.drawPixmap(btnBackRect,btnBack);
} //51

void FrmMain::towerMenuClicked(QRect pos)
{
    if(towerMenu == -1 || towerMenuHidden || towerMenuAnimating > -1) return;
    Tower *tmpTower = tiles[towerMenu]->t;
    if(tmpTower != nullptr) { //turm steht, upgrade
        if(!tmpTower->disabled) {
            if(pos.intersects(btnDmgRect) && tmpTower->hasDamage) { //dmg upgrade
                if(tmpTower->dmglvl < 3) {
                    if(towerMenuSelected == 1 && benis >= upgradeCost) { //kaufen
                        upgradeDmg(tmpTower);
                        setBenis(benis - upgradeCost);
                        tmpTower->dmglvl++;
                        if(tmpTower->dmglvl == 3) towerMenuSelected = -1;
                    } else towerMenuSelected = 1;
                }
            } else if(pos.intersects(btnRangeRect) && tmpTower->hasRange) { //range upgrade
                if(tmpTower->rangelvl < 3) {
                    if(towerMenuSelected == 2 && benis >= upgradeCost) { //kaufen
                        tmpTower->range += 10;
                        setBenis(benis - upgradeCost);
                        tmpTower->rangelvl++;
                        if(tmpTower->rangelvl == 3) towerMenuSelected = -1;
                        tmpTower->tmpRangeDir = 0;
                    } else towerMenuSelected = 2;
                }
            } else if(pos.intersects(btnFirerateRect) && tmpTower->hasFirerate) { //feuerrate upgrade
                if(tmpTower->ratelvl < 3) {
                    if(towerMenuSelected == 3 && benis >= upgradeCost) { //kaufen
                        tmpTower->reload *= 0.85;
                        setBenis(benis - upgradeCost);
                        tmpTower->ratelvl++;
                        if(tmpTower->ratelvl == 3) towerMenuSelected = -1;
                    } else towerMenuSelected = 3;
                }
            } else if(pos.intersects(btnSpeedRect) && tmpTower->hasProjectileSpeed) { //kugelgeschwindigkeit upgrade
                if(tmpTower->speedlvl < 3) {
                    if(towerMenuSelected == 4 && benis >= upgradeCost) { //kaufen
                        tmpTower->pspeed += 1;
                        setBenis(benis - upgradeCost);
                        tmpTower->speedlvl++;
                        if(tmpTower->speedlvl == 3) towerMenuSelected = -1;
                    } else towerMenuSelected = 4;
                }
            } else if(pos.intersects(btnSellRect)) { //tower verkaufen
                if(towerMenuSelected == 5 && !tmpTower->sellingDisabled) {
                    uint add = (uint)tmpTower->price;
                    for(int a = 1 ; a < tmpTower->dmglvl; a++) {
                        add += (tmpTower->price * upgradeHighConst) * (a+1);
                    }
                    for(int a = 1 ; a < tmpTower->rangelvl; a++) {
                        add += (tmpTower->price * upgradeLowConst) * (a+1);
                    }
                    for(int a = 1 ; a < tmpTower->ratelvl; a++) {
                        add += (tmpTower->price * upgradeHighConst) * (a+1);
                    }
                    for(int a = 1 ; a < tmpTower->speedlvl; a++) {
                        add += (tmpTower->price * upgradeLowConst) * (a+1);
                    }
                    for(int a = 1 ; a < tmpTower->turnlvl; a++) {
                        add += (tmpTower->price * upgradeHighConst) * (a+1);
                    }
                    setBenis(benis + add/2);
                    delTower(tmpTower);
                    statistics->towersSold++;
                    towerMenuSelected = -1;
                } else towerMenuSelected = 5;
            } else if(pos.intersects(btnTurnRect)) {
                if(tmpTower->turnlvl < 3) {
                    if(towerMenuSelected == 6 && benis >= upgradeCost) {
                        switch(tmpTower->type) {
                        default:
                            tmpTower->angleSpeed *= 1.2;
                        }
                        setBenis(benis - upgradeCost);
                        tmpTower->turnlvl++;
                        if(tmpTower->turnlvl == 3) towerMenuSelected = -1;
                    } else towerMenuSelected = 6;
                }
            }
        } else {
            if(tmpTower->disabled >= disabledTime) {
                tmpTower->disabled -= 10;
            }
        }
    } else { //noch kein turm, turmauswahl
        bool buy = false;
        bool clicked = false;
        if(towerMenuSelected > -1) clicked = true;
        if(pos.intersects(minusTowerRect)) { //Minustower
            uint cost = minusTowerCost;
            if(chosenTiles.size()) {
                cost += minusTowerCost*chosenTiles.size();
            }
            if(towerMenuSelected == TOWER_MINUS && benis >= cost) { //kaufen
                buy = true;
                buyMinusTower(towerMenu);
            } else {
                towerMenuSelected = 1;
                clicked = true;
            }
        } else if(pos.intersects(favTowerRect)) { //Favoritentower
            uint cost = herzTowerCost;
            if(chosenTiles.size()) {
                cost += herzTowerCost*chosenTiles.size();
            }
            if(towerMenuSelected == TOWER_HERZ && benis >= cost) { //kaufen
                buy = true;
                buyFavTower(towerMenu);
            } else {
                towerMenuSelected = 2;
                clicked = true;
            }
        } else if(pos.intersects(repostTowerRect) && !shop->towerPrices[TOWER_REPOST-1]) { //Reposttower
            uint cost = repostTowerCost;
            if(chosenTiles.size()) {
                cost += repostTowerCost*chosenTiles.size();
            }
            if(towerMenuSelected == TOWER_REPOST && benis >= cost) {
                buy = true;
                buyRepostTower(towerMenu);
            } else {
                towerMenuSelected = 3;
                clicked = true;
            }
        } else if(pos.intersects(benisTowerRect) && !shop->towerPrices[TOWER_BENIS-1]) { //Benistower
            uint cost = benisTowerCost;
            if(chosenTiles.size()) {
                cost += benisTowerCost*chosenTiles.size();
            }
            if(towerMenuSelected == TOWER_BENIS && benis >= cost) {
                buy = true;
                buyBenisTower(towerMenu);
            } else {
                towerMenuSelected = 4;
                clicked = true;
            }
        } else if(pos.intersects(banTowerRect) && !shop->towerPrices[TOWER_BAN-1]) { //Bantower
            uint cost = banTowerCost;
            if(chosenTiles.size()) {
                cost += banTowerCost*chosenTiles.size();
            }
            if(towerMenuSelected == TOWER_BAN && benis >= cost) {
                buy = true;
                buyBanTower(towerMenu);
            } else {
                int tcount = 0;
                for(uint i = 0; i < towers.size(); i++) {
                    if(towers[i]->type == TOWER_BAN) tcount++;
                }
                if(tcount <= 9) {
                    towerMenuSelected = 5;
                } else {
                    QMessageBox::information(this, "Info", "Du kannst nicht mehr Banntower bauen!");
                }
                clicked = true;
            }
        } else if(pos.intersects(sniperTowerRect) && !shop->towerPrices[TOWER_SNIPER-1]) { //Snipertower
            uint cost = sniperTowerCost;
            if(chosenTiles.size()) {
                cost += sniperTowerCost*chosenTiles.size();
            }
            if(towerMenuSelected == TOWER_SNIPER && benis >= cost) {
                buy = true;
                buySniperTower(towerMenu);
            } else {
                towerMenuSelected = 6;
                clicked = true;
            }
        } else if(pos.intersects(flakTowerRect) && !shop->towerPrices[TOWER_FLAK-1]) { //Flaktower
            uint cost = flakTowerCost;
            if(chosenTiles.size()) {
                cost += flakTowerCost*chosenTiles.size();
            }
            if(towerMenuSelected == TOWER_FLAK && benis >= cost) {
                buy = true;
                buyFlakTower(towerMenu);
            } else {
                towerMenuSelected = 7;
                clicked = true;
            }
        } else if(pos.intersects(laserTowerRect) && !shop->towerPrices[TOWER_LASER-1]) {
            uint cost = laserTowerCost;
            if(chosenTiles.size()) {
                cost += laserTowerCost*chosenTiles.size();
            }
            if(towerMenuSelected == TOWER_LASER && benis >= cost) {
                buy = true;
                buyLaserTower(towerMenu);
            } else {
                towerMenuSelected = 8;
                clicked = true;
            }
        } else if(pos.intersects(poisonTowerRect) && !shop->towerPrices[TOWER_POISON-1]) {
            uint cost = poisonTowerCost;
            if(chosenTiles.size()) {
                cost += poisonTowerCost*chosenTiles.size();
            }
            if(towerMenuSelected == TOWER_POISON && benis >= poisonTowerCost) {
                buy = true;
                buyPoisonTower(towerMenu);
            } else {
                towerMenuSelected = 9;
                clicked = true;
            }
        } else if(pos.intersects(minigunTowerRect) && !shop->towerPrices[TOWER_MINIGUN-1]) {
            uint cost = minigunTowerCost;
            if(chosenTiles.size()) {
                cost += minigunTowerCost*chosenTiles.size();
            }
            if(towerMenuSelected == TOWER_MINIGUN && benis >= minigunTowerCost) {
                buy = true;
                buyMinigunTower(towerMenu);
            } else {
                towerMenuSelected = 10;
                clicked = true;
            }
        }
        if(buy) {
            towerMenuSelected = -1;
            shake(10, 0, 3);
            statistics->towersBuilt++;
            statistics->towersBuilt += chosenTiles.size();
            enemyWarning = false;
        } else if(!buy && clicked){
            if(towerMenuSelected > -1 && playingMode == MODE_MAZE) {
                mutex.lock();
                bool coll = false;
                if(pathsContains(towerMenu)) coll = true;
                for(uint i = 0; i < chosenTiles.size(); i++) {
                    if(pathsContains(chosenTiles[i])) {
                        coll = true;
                        break;
                    }
                }
                tempPath.path.resize(0);
                if(coll) {
                    std::vector <Tile> tempTiles;
                    for(uint i = 0; i < tiles.size(); i++) {
                        Tile tmp;
                        tmp.type = tiles[i]->type;
                        tempTiles.push_back(tmp);
                    }
                    tiles[towerMenu]->type = TILE_TOWER;
                    if(chosenTiles.size()) {
                        for(uint i = 0; i < chosenTiles.size(); i++) {
                            tiles[chosenTiles[i]]->type = TILE_TOWER;
                        }
                    }
                    int code = createMazePath(true);
                    for(uint i = 0; i < tiles.size(); i++) {
                        tiles[i]->type = tempTiles[i].type;
                    }

                    mutex.unlock();
                    if(code != 0) {
                        tempPath.path.resize(0);
                        towerMenuSelected = -1;
                        QMessageBox::information(this, "Info", "Bauen unmöglich.\n"
                                                               "Weg blockiert!");
                    } else {
                        bool ok = false;
                        for(uint i = 0; i < enemys.size(); i++) {
                            if(!tempPath.contains(enemys[i]->cpos)) {
                                ok = true;
                            }
                        }
                        if(ok) enemyWarning = true;
                    }
                } else {
                    mutex.unlock();
                }
            }
        }
    }
}

void FrmMain::editorMenuClicked(QRect pos)
{
    if(pos.intersects(levels[0].levelRect)) { //16x9
        subActiveSelected = 1;
    } else if(pos.intersects(levels[1].levelRect)) { //32x18
        subActiveSelected = 2;
    } else if(pos.intersects(levels[2].levelRect)) { //64x36
        subActiveSelected = 3;
    } else if(pos.intersects(menuPlayRect)) { //los
        playClicked();
    } else if(pos.intersects(menuDelRect)) {
        delClicked();
    } else if(pos.intersects(playingModeRect)) {
        //shop check einbauen
#ifdef QT_DEBUG
        if(playingMode == MODE_NORMAL) {
            playingMode = MODE_MAZE;
        } else playingMode = MODE_NORMAL;
#endif
    } else { //usermap auswahl
        for(uint i=3;i<levels.size();i++) {
            if(pos.intersects(levels[i].levelRect)) {
                subActiveSelected = i+1;
                break;
            }
        }
    }
}

void FrmMain::startMenuClicked(QRect pos)
{
    if(pos.intersects(levels[0].levelRect)) { //levels
        subActiveSelected = 1;
        subLevelSelected = 0;
    } else if(pos.intersects(levels[1].levelRect)) { //eigene maps
        subActiveSelected = 2;
        subLevelSelected = 0;
    } else if(pos.intersects(levels[2].levelRect)) { //communitymaps
        //subActiveSelected = 3;
    } else if(pos.intersects(menuDelRect)) {
        playClicked();
    } else if(pos.intersects(playingModeRect)) {
        if(playingMode == MODE_NORMAL) {
            playingMode = MODE_MAZE;
        } else {
            playingMode = MODE_NORMAL;
        }
    } else {
        bool clicked = false;
        switch(subActiveSelected) {
        case 1: //levels
            if(playingMode == MODE_NORMAL) {
                for(uint i = 0; i < mainLevels; i++) {
                    if(pos.intersects(mainLvlRects[i]) && !lvlPrices[i]) {
                        subLevelSelected = i+4;
                        clicked = true;
                        break;
                    }
                }
            } else if(playingMode == MODE_MAZE) {
                for(uint i = 0; i < mazeLevels; i++) {
                    if(pos.intersects(mainLvlRects[i]) && !shop->mazelvlPrices[i]) {
                        subLevelSelected = i+4;
                        clicked = true;
                        break;
                    }
                }
            }
            break;
        case 2: //eigene
            for(uint i=3;i<levels.size();i++) {
                if(pos.intersects(levels[i].levelRect)) {
                    subLevelSelected = i+1;
                    clicked = true;
                    break;
                }
            }
            break;
        }
        if(!clicked) subLevelSelected = 0;
    }
}

void FrmMain::handleCheater(QRect pos)
{
    QPoint point = pos.center();
    int angle = -90;
    Projectile *p = pool->getProjectile();
    p->init3("Oh neim!",
             QPoint(point.x()-25,point.y()),
             angle,
             qCos(qDegreesToRadians((double)angle)),
             qSin(qDegreesToRadians((double)angle)),
             0.01,
             0.5);
    p->color = QColor(Engine::random(0,255), Engine::random(0,255), Engine::random(0,255));
    p->hasShekelImage = false;
    projectiles.push_back(p);
}

void FrmMain::playClicked()
{
    switch(subActive) {
    case 1: //levels
        //nur zum testen
        if(subActiveSelected&&subLevelSelected) {
            moveAn = ANIMATION_PLAY;
            fdir = 0;
            QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
        }
        break;
    case 2: //eigene
        switch(subActiveSelected) {
        case 1: //16x9
            mWidth = 16;
            mHeight = 9;
            moveAn = ANIMATION_EDITOR;
            fdir = 0;
            QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
            break;
        case 2: //32x18
            if(!mediumUnlocked) { //kaufen
                //if(purchaseMediumMap()) QMessageBox::information(this,"Fehler","Fehler bei der Initialisierung!");
            } else {
                mWidth = 32;
                mHeight = 18;
                moveAn = ANIMATION_EDITOR;
                fdir = 0;
                QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
            }
            break;
        case 3: //64x36
            if(!largeUnlocked) {
                //if(purchaseLargeMap()) QMessageBox::information(this,"Fehler","Fehler bei der Initialisierung!");
            } else {
                mWidth = 64; //64
                mHeight = 36; //36
                moveAn = ANIMATION_EDITOR;
                fdir = 0;
                QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
            }
            break;
        default: //eigene
            if(subActiveSelected>3) {
                mapSaved = subActiveSelected-1;
                mWidth = levels[subActiveSelected-1].width;
                mHeight = levels[subActiveSelected-1].height;
                moveAn = ANIMATION_EDITOR_OWN;
                fdir = 0;
                QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
            }
            break;
        }
        break;
    }
}

void FrmMain::delClicked()
{
    if(subActiveSelected>3) {
        QMessageBox box;
        MsgBox::createMsgBox(box,"Map wirklich löschen?");
        int reply = box.exec();
        if(!reply) {
            if(levels.size()>subActiveSelected) {
                QRect r = levels[subActiveSelected-1].levelRect;
                levels[subActiveSelected].levelRect = r;
            }
            levels.erase(levels.begin()+(subActiveSelected-1));
            if(hasPlayingSave) {
                bool delfile = false;
                QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
                path.append("/save.dat");
                QFile file;
                file.setFileName(path);
                if(file.exists()) {
                    file.open(QIODevice::ReadOnly);
                    QTextStream in;
                    in.setDevice(&file);
                    QString data = in.readAll();
                    data.remove("\r\n");
                    QStringList split = data.split("#");
                    if(split.size() > 2) {
                        if(split[1].toInt() == 1) {
                            if(split[2].toUInt() == subActiveSelected) delfile = true;
                        }
                    }
                    file.close();
                }
                if(delfile) {
                    file.remove();
                    hasPlayingSave = false;
                }
            }
            subActiveSelected = 0;
        }
    }
}

void FrmMain::backMenuClicked(QRect pos)
{
    if(pos.intersects(btnContinueRect)) { //Weiterspielen
        backMenu = false;
        if(playingSpeed) {
            gamePaused = false;
        }
    } else if(pos.intersects(btnBackRect)) { //Zurück
        if((active == STATE_EDITOR && mapSaved) || active != STATE_EDITOR) {
            backMenu = false;
            moveAn = ANIMATION_BACK;
            fdir = 0;
            QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
        } else {
            QMessageBox box;
            MsgBox::createMsgBox(box,"Die Map wurde noch nicht gespeichert! Wirklich zurück?");
            int reply = box.exec();
            if(!reply) {
                backMenu = false;
                moveAn = ANIMATION_BACK;
                fdir = 0;
                QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
            }
        }
    }
}

void FrmMain::editorClicked(QRect pos, QRect aPos)
{
    isEditorMenuClicked = false;
    if(pos.intersects(mPathRect)) {
        editorSelected = 1;
        isEditorMenuClicked = true;
    } else if(pos.intersects(mTurmTileRect)) {
        editorSelected = 2;
        isEditorMenuClicked = true;
    } else if(pos.intersects(mEmptyTileRect)) {
        editorSelected = 3;
        isEditorMenuClicked = true;
    } else if(pos.intersects(mBaseRect)) {
        editorSelected = 5;
        isEditorMenuClicked = true;
    } else if(pos.intersects(mEnemyBaseRect)) {
        editorSelected = 4;
        isEditorMenuClicked = true;
    } else if(pos.intersects(mClearRect)) {
        editorSelected = 6;
        isEditorMenuClicked = true;
    } else if(pos.intersects(mWaypointRect)) {
        editorSelected = 7;
        isEditorMenuClicked = true;
    } else if(pos.intersects(mPlayRect)) {
        isEditorMenuClicked = true;
        if(!mapPlaying) {
            if(playingMode == MODE_NORMAL) {
                mCompileError = createPath();
            } else if(playingMode == MODE_MAZE) {
                if(!waypoints.size()) {
                    //waypoint array beim laden erstellen
                    mCompileError = -3;
                } else mCompileError = createMazePath();
            }
            switch (mCompileError) {
            case -1:
                QMessageBox::information(this, "Fehler", "Kein Start gefunden!");
                break;
            case -2:
                QMessageBox::information(this, "Fehler", "Kein Ziel gefunden!");
                break;
            case -3:
                QMessageBox::information(this, "Fehler", "Es muss mindestens 1 Wegpunkt existieren!");
                break;
            default: //alles ok
                bool ok=false;
                for(uint i=0;i<tiles.size();i++) {
                    if(tiles[i]->type==2) {
                        ok = true;
                        break;
                    }
                }
                if(ok || playingMode == MODE_MAZE) {
                    mapPlaying = true;
                } else {
                    QMessageBox::information(this,"Fehler","Es muss mindestens 1 Turmplatz vorhanden sein!");
                }
                break;
            }
        } else {
            delAllEnemys();
            mapPlaying = false;
            mCompileError = -1;
        }

    } else if(pos.intersects(mSaveRect)&&!mCompileError) {
        isEditorMenuClicked = true;
        Level l;
        QString name;
        if(!mapSaved) {
            QGuiApplication::inputMethod()->show();
            name = QInputDialog::getText(this,"Mapname","Mapname:(Mind. 3 Zeichen, nur Zahlen & Buchstaben!)");
            QString map = name+"#";
            bool ok=true;
            if(map.size()<3) ok=false;
            for(int i=0;i<name.size();i++) {
                if(!name[i].isLetter()&&!name[i].isDigit()) {
                    ok = false;
                }
            }
            if(!ok) {
                QMessageBox::information(this,"Fehler","Falsche Eingabe!");
                return;
            } else {
                for(uint i=0;i<levels.size();i++) {
                    if(levels[i].lvlName==name) { //name bereits vorhanden
                        QMessageBox::information(this,"Fehler","Name bereits verwendet!");
                        return;
                    }
                }
            }
        } else {
            l = levels[mapSaved];
            l.paths.clear();
            l.map.clear();
        }
        l.paths = paths;
        for(uint i=0;i<tiles.size();i++) {
            l.map.push_back(tiles[i]->type);
        }
        l.width = mapwidth/80;
        l.height = mapheight/80;
        l.lvlImage = createMapImage(l.getMapString(),l.width,l.height);
        //IMG speichern
#ifdef QT_DEBUG
        QFile file;
        QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        path.append("/img.png");
        file.setFileName(path);
        file.open(QIODevice::WriteOnly);
        QPixmap p = createMapImage(l.getMapString(),l.width,l.height);
        p.save(&file,"PNG");
        file.close();
#endif
        //
        if(!mapSaved) {
            for(uint i=3,y=1,a=0;i<levels.size()+1;i++) {
                if(i==levels.size()) {
                    QRect rect = QRect(500+(325*a),400+(193*y),300,168);
                    l.levelRect = rect;
                }
                a++;
                if(a==4) {
                    a=0;
                    y++;
                }
            }
            l.lvlName = name;
            mapSaved = levels.size();
            levels.push_back(l);
        } else {
            levels[mapSaved] = l;
        }
        saveMaps();
        QMessageBox::information(this,"Info","Speichern erfolgreich!");
    } else if(pos.intersects(menuDelRect)) {
        isEditorMenuClicked = true;
        if(!editMode) {
            editMode = 1;
        } else {
            editMode = 0;
        }
    } else { //kein Menü -> tilecheck
        if(mapPlaying || !editMode || isEditorMenuClicked) return;
        for(uint i=0;i<tiles.size();i++) {
            if(aPos.intersects(tiles[i]->rect())) {
                if(editorSelected == 5 && tiles[i]->type != editorSelected) {
                    if(!ownBaseCount) {
                        ownBaseCount++;
                        tiles[i]->type = editorSelected;
                    }
                } else if(editorSelected == 4 && tiles[i]->type != editorSelected) {
                    if((!enemyBaseCount && playingMode == MODE_MAZE) ||
                            playingMode == MODE_NORMAL) {
                        enemyBaseCount++;
                        tiles[i]->type = editorSelected;
                    }
                } else if(editorSelected == 7) {
                    if(tiles[i]->type >= TILE_WAYP) {
                        for(uint a = 0; a < waypoints.size(); a++) {
                            tiles[waypoints[a]]->type = TILE_GRAS;
                        }
                        waypoints.resize(0);
                    } else {
                        int count = 100 + waypoints.size();
                        tiles[i]->type = count;
                        waypoints.push_back(i);
                    }
                } else if(editorSelected) {
                    if(tiles[i]->type==5) ownBaseCount--;
                    if(tiles[i]->type==4) enemyBaseCount--;
                    tiles[i]->type = editorSelected;
                }
            }
        }
    }
}

void FrmMain::gameOver()
{
    isGameOver = true;
    hasPlayingSave = false;
    chosenSpruch = Engine::random(0,spruch.size());
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/save.dat");
    QFile file;
    file.setFileName(path);
    if(file.exists()) {
        file.remove();
    }
}

void FrmMain::loadUserData()
{
    loadOptions();
    loadLevels();
    QString base = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QString path = base;
    path.append("/save.dat");
    QFile file;
    file.setFileName(path);
    if(file.exists()) {
        bool ok = true;
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        QString data = in.readAll();
        QStringList split = data.split("#");
        if(split[10].toInt() <= 0) {
            ok = false;
        }
        file.close();
        if(ok) {
            hasPlayingSave = true;
        } else {
            file.remove();
            hasPlayingSave = false;
        }
    }
    path.replace("/save.dat","/maps.dat");
    file.setFileName(path);
    if(file.exists()) { //Lädt vom User erstellte Maps
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        QString data = in.readAll();
        if(data.size()) {
            data.remove("\r\n");
            QStringList split = data.split("|");
            for(int i=0;i<split.size()-1;i++) {
                levels.push_back(parseLvlString(split[i]));
            }
        }
        file.close();
    }
    userDataLoaded = true;
}

void FrmMain::loadMap(QString name, int custom, int width, int height, QString path, double mapversion)
{
    Q_UNUSED(mapversion)
    loaded = true;
    QString map;
    switch (custom) {
    case 0: //standard
    {
        mName = name;
        mapID = 1;
        QFile file;
        file.setFileName(path+name+".dat");
        QTextStream in;
        file.open(QIODevice::ReadOnly);
        in.setDevice(&file);
        QString data = in.readAll();
        file.close();
        data.remove("\r\n");
        QStringList split2 = data.split('#');
        mapwidth = split2[1].toInt()*80;
        mapheight = split2[2].toInt()*80;
        map = split2[4];
        data = split2[3];
        QStringList rawSplit = data.split(";");
        for(int i=0;i<rawSplit[0].toInt();i++) {
            Path tmpPath;
            QString p = rawSplit[i+1];
            QStringList split = p.split(',');
            for(int i=0;i<split.size();i++) {
                tmpPath.path.push_back(split[i].toInt());
            }
            paths.push_back(tmpPath);
        }
        break;
    }
    case 1: //mapeditor-preset
    {
        mapwidth = width*80;
        mapheight = height*80;
        viewRect = QRect(0,0,mapwidth,mapheight);
        for(int i=0;i<width*height;i++) {
            map += "6,";
        }
        map.remove(map.size()-1,1);
        break;
    }
    } //end switch
    if(custom!=2) {
        QStringList split = map.split(',');
        int maxX = mapwidth/80;
        int maxY = (mapheight)/80;
        for(int u=0;u<maxY;u++) {
            for(int i=0;i<maxX;i++) {
                Tile *t = new Tile();
                t->pos = QRect(0+(80*i),(80*u),80,80);
                if(custom!=1) {
                    t->type = split[tiles.size()].toInt();
                } else {
                    t->type = 6;
                }
                switch(t->type) {
                case 1:
                    t->ran = Engine::random(0,2);
                    break;
                }
                //createTileBodies.push_back(t);
                tiles.push_back(t);
            }
        }
    } else { //userlevel
        mapID = 2;
        width = levels[subLevelSelected-1].width;
        height = levels[subLevelSelected-1].height;
        mapwidth = width*80;
        mapheight = height*80;
        int maxX = width;
        int maxY = height;
        for(int u=0;u<maxY;u++) {
            for(int i=0;i<maxX;i++) {
                Tile *t = new Tile();
                t->pos = QRect(0+(80*i),(80*u),80,80);
                t->type = levels[subLevelSelected-1].map[tiles.size()];
                switch(t->type) {
                case 1:
                    t->ran = Engine::random(0,2);
                    break;
                case 4:
                    enemyBaseCount++;
                    break;
                case 5:
                    ownBaseCount++;
                    break;
                }
                //createTileBodies.push_back(t);
                tiles.push_back(t);
            }
        }
        this->paths = levels[subLevelSelected-1].paths;
    }
    viewRect = QRect(0,0,mapwidth,mapheight);
#ifdef QT_DEBUG
    QFile file;
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/img2.png");
    file.setFileName(path);
    file.open(QIODevice::WriteOnly);
    mapPx.save(&file);
    file.close();
#endif
    if(playingMode == MODE_MAZE && moveAn == ANIMATION_PLAY) {
        createMazePath();
    }

    if(moveAn != ANIMATION_CONTINUE) {
        createPathBoundaries();
        mapPx = createMapImage(custom);
        mapPx_gras = createMapImage(2);
    }
    mapLoaded = true;
}

void FrmMain::loadLevels()
{
    Level l;
    l.width = 16;
    l.height = 9;
    levels.push_back(l);
    l.width = 32;
    l.height = 18;
    levels.push_back(l);
    l.width = 64;
    l.height = 36;
    levels.push_back(l);
    for(int i=0;i<3;i++) {
        levels[i].levelRect = QRect(500+(325*i),400,300,168);
        levels[i].lvlImage = QPixmap(":/data/images/mapeditor/"+QString::number(i)+".png");
    }
    for(uint i=0;i<mainLevels;i++) {
        lvlPreviews.push_back(QPixmap(":/data/images/maps/map"+QString::number(i)+".png"));
    }
    for(uint i=0;i<mazeLevels;i++) {
        mazelvlPreviews.push_back(QPixmap(":/data/images/maps/maze/map"+QString::number(i)+".png"));
        shop->mazelvlPreviews = mazelvlPreviews;
    }
    int y=1;
    int a=0;
    for(uint i=0;i<mainLevels+1;i++) {
        QRect rect = QRect(500+(325*a),400+(193*y),300,168);
        a++;
        if(a==4) {
            a=0;
            y++;
        }
        mainLvlRects.push_back(rect);
    }
}

void FrmMain::loadGameSave()
{
    // maptyp#mapname#(turmtyp,pos,lvls);(...)#(enemypos,typ);(...)#
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/save.dat");
    QFile file;
    file.setFileName(path);
    if(file.exists()) {
        file.open(QIODevice::ReadOnly);
        QTextStream in;
        in.setDevice(&file);
        QString data = in.readAll();
        /*QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(data);
        clipboard->deleteLater();*/
        if(!data.size()) {
            error_save(file);
            return;
        }
        data.remove("\r\n");
        QStringList split = data.split("#");
        if(split.size()<9||split[2].size()<1) {
            error_save(file);
            return;
        }
        QString mapversion = split[0];
        if(mapversion.toDouble() != version) {
            if(!mapversion.contains(".")) {
                error_save(file);
                return;
            }
            if(mapversion.toDouble() < 1.0) {
                error_save(file);
                return;
            } else if((mapversion.toDouble() < 1.031 && !split[1].toInt())) {
                QString s = split[6];
                QStringList waveSplit = s.split(",");
                int wc = waveSplit[9].toInt();
                error_save(file, "Weil die Standardmaps geändert wurden sind alte Map-Saves inkompatibel!"
                                 " Als Entschädigung erhältst du Shekel abhängig davon bis zu welcher"
                                 " Welle du es geschafft hattest");
                int am = 0;
                for(int i = 0; i < wc; i += 10) {
                    if(i < 41) {
                        am += 5;
                    } else if(i < 51) {
                        am += 10;
                    } else if(i < 81) {
                        am += 15;
                    } else am += 20;
                }
                if(am) {
                    setShekel(shop->shekel + am);
                }
                return;
            }
            //Mapversion ungleich, evtl konvertierung
        }
        //Map laden
        switch(split[1].toInt()) {
        case 0: //standardlevel
            playingMode = MODE_NORMAL;
            loadMap(split[2], 0, 0, 0, ":/data/maps/", mapversion.toDouble());
            break;
        case 1: //userlevel
            playingMode = MODE_NORMAL;
            subLevelSelected = split[2].toInt();
            loadMap("",2);
            break;
        case 2: //mazelevel
            playingMode = MODE_MAZE;
            loadMap(split[2], 0, 0, 0, ":/data/maps/", mapversion.toDouble());
            break;
        }
        //Türme laden
        QString s = split[3];
        QStringList towerList = s.split(";");
        for(int i=0;i<towerList.size()-1;i++) {
            QString rawData = towerList[i];
            QStringList data = rawData.split(",");
            Tower *t = new Tower();
            t->type = data[0].toInt();
            switch(t->type) {
            case 2:
                t->projectilePxID = 1;
                break;
            case 3:
                t->hasProjectileSpeed = false;
                break;
            case 4:
                t->hasProjectileSpeed = false;
                t->hasRange = false;
                break;
            case 5:
                t->hasDamage = false;
                t->hasRange = false;
                break;
            }
            t->dmg = data[1].toDouble();
            t->stun = data[2].toDouble();
            t->tnum = data[3].toInt();
            t->pos = tiles[t->tnum]->pos;
            t->reload = data[4].toInt();
            t->range = data[5].toInt();
            t->pspeed = data[6].toDouble();
            t->dmglvl = data[7].toInt();
            t->ratelvl = data[8].toInt();
            t->speedlvl = data[9].toInt();
            t->rangelvl = data[10].toInt();
            t->repost = data[11].toInt();
            t->projectilePxID = data[12].toInt();
            t->angle = data[13].toDouble();
            t->angleSpeed = data[14].toDouble();
            t->targetAngle = data[15].toDouble();
            t->isShooting = data[16].toInt();
            t->turnlvl = data[17].toInt();
            t->price = data[18].toInt();
            t->animation = data[19].toInt();
            t->animationMax = data[20].toInt();
            if(mapversion.toDouble() >= 1.02) {
                t->disabled = data[21].toInt();
            }
            if(mapversion.toDouble() > 1.035) {
                t->disabledFlagged = data[22].toInt();
            }
            t->saveNum = data[data.size()-1].toInt();
            switch(t->type) {
            case TOWER_REPOST: //Reposttower
            case TOWER_BENIS: //Benistower
                t->hasProjectileSpeed = false;
                t->hasTurnSpeed = false;
                break;
            case TOWER_BAN: //Banntower
                t->hasDamage = false;
                t->hasRange = false;
                t->hasTurnSpeed = false;
                break;
            case TOWER_LASER: //Lasertwoer
            case TOWER_POISON: //Gifttower
                t->hasProjectileSpeed = false;
                break;
            }
            addTowerTargets(t);
            towers.push_back(t);
            tiles[data[3].toInt()]->type = TILE_TOWER;
            tiles[data[3].toInt()]->t = t;
        }
        //Gegner laden
        s = split[4];
        QStringList enemyList = s.split(";");
        for(int i=0;i<enemyList.size()-1;i++) {
            QString rawData = enemyList[i];
            QStringList data = rawData.split(",");
            Enemy *e = pool->getEnemy();
            e->cpos = data[0].toInt();
            e->dir = data[1].toInt();
            e->health = data[2].toDouble();
            e->maxHealth = data[3].toDouble();
            e->maxStun = data[4].toInt();
            e->ospeed = data[5].toDouble();
            e->pos = QRectF(data[6].toDouble(),data[7].toDouble(),data[8].toDouble(),data[9].toDouble());
            e->preHealth = data[10].toDouble();// später falls projektile
            e->speed = data[11].toDouble();
            e->stunned = data[12].toInt();
            e->repost = data[13].toInt();
            e->soonBanned = data[14].toInt();
            e->type = data[15].toInt();
            e->opacity = data[16].toDouble();
            e->path = data[17].toInt();
            e->price = data[18].toInt();
            e->animation = data[19].toInt();
            e->poison = data[20].toInt();
            e->maxPoison = data[21].toInt();
            e->poisonDmg = data[22].toInt();
            e->newdir = data[23].toInt();
            e->blocked = data[24].toInt();
            if(mapversion.toDouble() > 1.02) {
                e->reload = data[25].toInt();
                e->maxReload = data[26].toInt();
            }
            if(mapversion.toDouble() > 1.034) {
                e->imgID = data[27].toInt();
            } else {
                e->imgID = Engine::random(0,normalEnemys.size());
            }
            e->calcWidth();
            uint16_t ownBits;
            uint16_t collBits;
            getCollidingBits(e->type, ownBits, collBits);
            e->ownCategoryBits = ownBits;
            e->collidingCategoryBits = collBits;
            if(e->type == ENEMY_LEGENDE) {
                e->name = bossNames[Engine::random(0,bossNames.size())];
            }
            createBodies.push_back(e);
            //enemys.push_back(e);
        }
        //Turm Pointerzuweisung
        for(uint i=0;i<towers.size();i++) {
            if(towers[i]->saveNum>-1) {
                towers[i]->target = createBodies[towers[i]->saveNum];
            }
        }
        //------
        //Projektile
        s = split[5];
        QStringList projectileList = s.split(";");
        for(int i=0;i<projectileList.size()-1;i++) {
            QString rawData = projectileList[i];
            QStringList data = rawData.split(",");
            Projectile *p = pool->getProjectile();
            p->rect = QRectF(data[0].toDouble(),data[1].toDouble(),data[2].toDouble(),data[3].toDouble());
            p->angle = data[4].toInt();
            p->dmg = data[5].toInt();
            p->stun = data[6].toInt();
            p->repost = data[7].toInt();
            p->type = data[8].toInt();
            p->opacity = data[9].toDouble();
            p->opacityDecAm = data[10].toDouble();
            p->vx = data[11].toDouble();
            p->vy = data[12].toDouble();
            p->vel = data[13].toDouble();
            p->del = data[14].toInt();
            p->text = data[15];
            p->pxID = data[16].toInt();
            p->hasShekelImage = data[17].toInt();
            p->color = QColor(data[18].toInt(),data[19].toInt(),data[20].toInt(),data[21].toInt());
            p->poisonDmg = data[22].toInt();
            if(mapversion.toDouble() > 1.035) {
                p->targetRect = QRectF(data[23].toDouble(),data[24].toDouble(),data[25].toDouble(),data[26].toDouble());
            }
            p->saveNum = data[data.size()-1].toInt();
            projectiles.push_back(p);
        }
        //------
        //Pointerzuweisung projektile
        for(uint i=0;i<projectiles.size();i++) {
            if(projectiles[i]->saveNum>-1) {
                projectiles[i]->target = createBodies[projectiles[i]->saveNum];
            }
        }
        //------
        //Welle laden
        s = split[6];
        QStringList waveSplit = s.split(",");
        currentWave.fliesen = waveSplit[0].toInt();
        currentWave.neuschwuchteln = waveSplit[1].toInt();
        currentWave.schwuchteln = waveSplit[2].toInt();
        currentWave.spender = waveSplit[3].toInt();
        currentWave.altschwuchteln = waveSplit[4].toInt();
        currentWave.mods = waveSplit[5].toInt();
        currentWave.admins = waveSplit[6].toInt();
        currentWave.legenden = waveSplit[7].toInt();
        currentWave.gebannte = waveSplit[8].toInt();
        setWaveCount(waveSplit[9].toInt());
        currentWave.maxEnemysPerWave = waveSplit[10].toInt();
        currentWave.enemysPerWave = waveSplit[11].toInt();
        currentWave.waveTime = waveSplit[12].toInt();
        //------
        //Allgemeines Zeugs laden
        if(playingMode == MODE_MAZE) {
            createMazePath();
        } else {
            createPath();
        }
        createPathBoundaries();

        mapPx = createMapImage(0);
        mapPx_gras = createMapImage(2);

        setBenis(split[7].toULongLong(), true);
        this->internalWaveCount = split[9].toInt();
        int am = split[10].toInt();
        if(!am) am = 75;
        setBaseHp(am);
        file.close();
    }
}

void FrmMain::loadLitteSprueche()
{
    spruch.push_back("immer han du di pech");
    spruch.push_back("Merkel did nothing wrong");
    spruch.push_back("lel");
    spruch.push_back("du hanst verlierert");
    spruch.push_back("ggwp");
    spruch.push_back("Refugees welcome :D");
    spruch.push_back("DerpyDerp für Leb... Oh");
    spruch.push_back("#krebsiscancer");
    spruch.push_back("Bruh Games > EA");
    spruch.push_back("Danke Merkel");
    spruch.push_back("dm780 bester mod");
    spruch.push_back("würde nochmal");
    spruch.push_back("deus vult");
    spruch.push_back("¯\\_(ツ)_/¯");
    spruch.push_back("(~￣ ³￣)~");
    spruch.push_back("( ͡° ͜ʖ ͡°)");
    spruch.push_back("\\[T]/");
    spruch.push_back("Ach komm, das geht doch besser");
    spruch.push_back("Originaler Nichtskönner");
    spruch.push_back("Erwartung nicht erfüllt");
    spruch.push_back("ZOMG");
}

void FrmMain::saveGameSave()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/save.dat");
    QFile file;
    file.setFileName(path);
    file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    QTextStream out(&file);
    int t = 0;
    if(playingMode == MODE_MAZE) {
        t = 2;
    }
    out << QString::number(version) << "#";
    if(mName=="") {
        t = 1;
    }
    out << QString::number(t) << "#";
    if(t == 1) {
        out << QString::number(subLevelSelected) << "#";
    } else {
        out << mName << "#";
    }
    //Tower speichern
    for(uint i=0;i<towers.size();i++) {
        int saveNum = -1;
        if(towers[i]->target!=nullptr) {
            for(uint a=0;a<enemys.size();a++) {
                if(enemys[a]==towers[i]->target) {
                    saveNum = a;
                    break;
                }
            }
        }
        out << towers[i]->toString() << "," << QString::number(saveNum) << ";";
    }
    out << "#";
    //Gegner speichern
    for(uint i=0;i<enemys.size();i++) {
        out << enemys[i]->toString() << ";";
    }
    out << "#";
    for(uint i=0;i<projectiles.size();i++) {
        if(projectiles[i]->type != 2) { //wenn nicht text
            int saveNum = -1;
            for(uint a=0;a<enemys.size();a++) {
                if(enemys[a]==projectiles[i]->target) {
                    saveNum = a;
                    break;
                }
            }
            out << projectiles[i]->toString() << "," << QString::number(saveNum) << ";";
        }
    }
    out << "#" << currentWave.toString();
    out << "#" << QString::number(this->benis) << "#" << QString::number(waveCountSave.toInt()-11) << "#";
    out << QString::number(internalWaveCount) << "#" << QString::number(basehpSave.toInt()-11) << "#";
    file.close();
    hasPlayingSave = true;
}

void FrmMain::saveMaps()
{
    QFile file;
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/maps.dat");
    file.setFileName(path);
    file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    QTextStream out(&file);
    QString pathStrings;
    for(uint i = mainLevels - 1;i < levels.size();i++) {
        //FORMAT NAME#BREITE#HÖHE#PFADE#MAPDATA|
        pathStrings = "";
        QString data = levels[i].lvlName + "#" + QString::number(levels[i].width) + "#" + QString::number(levels[i].height) + "#";
        for(uint b=0;b<levels[i].paths.size();b++) {
            QString tp;
            for(uint a=0;a<levels[i].paths[b].path.size();a++) {
                tp.append(QString::number(levels[i].paths[b].path[a])+",");
            }
            tp.remove(tp.size()-1,1);
            pathStrings += tp + ";";
        }
        pathStrings.remove(pathStrings.size()-1,1);
        data.append(QString::number(levels[i].paths.size())+";"+pathStrings+"#");
        QString map;
        for(uint b=0;b<levels[i].map.size();b++) {
            map.append(QString::number(levels[i].map[b])+",");
        }
        map.remove(map.size()-1,1);
        data.append(map+"|");
        out << data;
    }
    file.close();
}

void FrmMain::saveOptions(unsigned long long customShekel, bool init)
{
    //QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Bruh Games", "Pr0fense");
    if(!userDataLoaded && !init) return;
    QFile file;
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/options.dat");
    file.setFileName(path);
    file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    QTextStream out(&file);
    unsigned long long a;
    if(!customShekel) {
        a = (shop->shekelSave.toULongLong() - 11) * 3;
    } else {
        a = customShekel * 3;
    }
    out << QString::number(soundEnabled) << "#"
        << QString::number(newestPost) << "#"
        << QString::number(accepted) << "#"
        << QString::number( a, 16 ).toUpper() << "#"
        << QString::number(popup) << "#"
        << QString::number(playedGames) << "#"
        << lastRewardDate.toString("ddMMyyyy") << "#"
        << QString::number(dailyReward->bonus) << "#"
        << lastPlayedDate.toString("ddMMyyyy") << "#"
        << QString::number(settings->graphicQuality) << "#"
        << QString::number(settings->energieSparen) << "#";
    for(uint i = 0; i < statistics->kills.size(); i++) {
        out << QString::number(statistics->kills[i], 16).toUpper() << "#";
    }
    out << QString::number(statistics->earnedBenis, 16).toUpper() << "#"
        << QString::number(statistics->earnedShekel, 16).toUpper() << "#"
        << QString::number(statistics->highestWave, 16).toUpper() << "#"
        << QString::number(statistics->enemysPassed, 16).toUpper() << "#"
        << QString::number(statistics->playedGames, 16).toUpper() << "#"
        << QString::number(statistics->towersBuilt, 16).toUpper() << "#"
        << QString::number(statistics->towersSold, 16).toUpper() << "#"
        << QString::number(statistics->timePlayed, 16).toUpper();
    file.close();
    path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/lvls.dat");
    file.setFileName(path);
    file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    QString data;
    out.setDevice(&file);
    for(uint i = 0; i < lvlPrices.size(); i++) {
        data = data + QString::number(lvlPrices[i]) + "#";
    }
    data.remove(data.size()-1, 1);
    data += '|';
    for(uint i = 0; i < shop->mazelvlPrices.size(); i++) {
        data = data + QString::number(shop->mazelvlPrices[i]) + "#";
    }
    data.remove(data.size()-1, 1);
    out << data;
    file.close();

    path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/towers.dat");
    file.setFileName(path);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    data = "";
    out.setDevice(&file);
    for(uint i = 0; i < shop->towerPrices.size(); i++) {
        data = data + QString::number(shop->towerPrices[i]) + "," +
                QString::number(shop->towerLocks[i]) + "#";
    }
    data.remove(data.size()-1, 1);
    out << data;
    file.close();

    path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/items.dat");
    file.setFileName(path);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    data = "";
    out.setDevice(&file);
    for(uint i = 0; i < shop->items.size(); i++) {
        data = data +
                QString::number(shop->items[i].locked) + "," +
                QString::number(shop->items[i].count) + "#";
    }
    if(shop->items.size() > 1) data.remove(data.size()-1, 1);
    out << data;
    file.close();
}

void FrmMain::loadGraphics()
{
    update();
    shop->loadGraphics();
    settings->loadGraphics();
    statistics->loadGraphics();
    playingModeNormal = QPixmap(":/data/images/normal.png");
    playingModeMaze = QPixmap(":/data/images/maze.png");
    repairPx = QPixmap(":/data/images/repair.png");
    repairPx_grau = QPixmap(":/data/images/ui/deaktiviert/repair.png");
    bossPx = QPixmap(":/data/images/enemys/boss/Boss1.png");
    mapTile = QPixmap(":/data/images/maptile.png");
    turmtile = QPixmap(":/data/images/turmtile.png");
    emptytile = QPixmap(":/data/images/empty.png");
    enemybasePx = QPixmap(":/data/images/enemybase.png");
    ownbasePx = QPixmap(":/data/images/ownbase.png");
    towerGroundPx = QPixmap(":/data/images/towers/base.png");
    towerRepostBasePx = QPixmap(":/data/images/towers/repost.png");
    circleSplit1 = QPixmap(":/data/images/enemys/death/circle1.png");
    circleSplit2 = QPixmap(":/data/images/enemys/death/circle2.png");
    circleSplit3 = QPixmap(":/data/images/enemys/death/circle3.png");
    rauteSplit1 = QPixmap(":/data/images/enemys/death/raute1.png");
    rauteSplit2 = QPixmap(":/data/images/enemys/death/raute2.png");
    rauteSplit3 = QPixmap(":/data/images/enemys/death/raute3.png");
    rauteSplit4 = QPixmap(":/data/images/enemys/death/raute4.png");
    repostMark = QPixmap(":/data/images/towers/repost_mark.png");
    herzPx = QPixmap(":/data/images/towers/herz.png");
    minus = QPixmap(":/data/images/towers/minus.png");
    sniperPx = QPixmap(":/data/images/towers/sniper.png");
    minusTowerPx = QPixmap(":/data/images/towers/minusTower.png");
    favTowerPx = QPixmap(":/data/images/towers/favTower.png");
    flakTowerPx = QPixmap(":/data/images/towers/flak.png");
    laserTowerPx = QPixmap(":/data/images/towers/lasertower.png");
    poisonTowerPx = QPixmap(":/data/images/towers/poison.png");
    poison_gas = QPixmap(":/data/images/towers/poison_poison.png");
    blus = QPixmap(":/data/images/towers/blus.png");
    blus_blurred = QPixmap(":/data/images/towers/blus_blurred.png");
    buyPx = QPixmap(":/data/images/ui/kaufen.png");
    sellPx = QPixmap(":/data/images/ui/verkaufen.png");
    auswahlPx = QPixmap(":/data/images/ui/auswahl.png");
    auswahlTile = QPixmap(":/data/images/auswahltile.png");
    btnSpeed = QPixmap(":/data/images/ui/btnSpeed.png");
    btnSpeed_grau = QPixmap(":/data/images/ui/deaktiviert/btnSpeed.png");
    btnDmg = QPixmap(":/data/images/ui/btnDmg.png");
    btnDmg_grau = QPixmap(":/data/images/ui/deaktiviert/btnDmg.png");
    btnFirerate = QPixmap(":/data/images/ui/btnFirerate.png");
    btnFirerate_grau = QPixmap(":/data/images/ui/deaktiviert/btnFirerate.png");
    btnRange = QPixmap(":/data/images/ui/btnRange.png");
    btnrange_grau = QPixmap(":/data/images/ui/deaktiviert/btnRange.png");
    btnSell = QPixmap(":/data/images/ui/btnSell.png");
    btnSell_grau = QPixmap(":/data/images/ui/deaktiviert/btnSell.png");
    btnTurn = QPixmap(":/data/images/ui/btnTurn.png");
    btnTurn_grau = QPixmap(":/data/images/ui/deaktiviert/btnTurn.png");
    btnBack = QPixmap(":/data/images/ui/zurueck.png");
    btnContinue = QPixmap(":/data/images/ui/weiterspielen.png");
    btnContinueGrey = QPixmap(":/data/images/ui/deaktiviert/weiterspielen_grau.png");
    btnStopPx = QPixmap(":/data/images/ui/stop.png");
    btnStop2Px = QPixmap(":/data/images/ui/stop2.png");
    btnStop3Px = QPixmap(":/data/images/ui/stop3.png");
    btnNormalPx = QPixmap(":/data/images/ui/normal.png");
    btnFastPx = QPixmap(":/data/images/ui/fast.png");
    btnSuperfastPx = QPixmap(":/data/images/ui/superfast.png");
    soundAusPx = QPixmap(":/data/images/ui/sound0.png");
    soundAnPx = QPixmap(":/data/images/ui/sound1.png");
    barPx = QPixmap(":/data/images/ui/powerBar.png");
    menuPx = QPixmap(":/data/images/menu.png");
    titlePx = QPixmap(":/data/images/ui/title.png");
    startPx = QPixmap(":/data/images/ui/start.png");
    editPx = QPixmap(":/data/images/ui/editor.png");
    exitPx = QPixmap(":/data/images/ui/beenden.png");
    shopPx = QPixmap(":/data/images/ui/shop.png");
    account_failPx = QPixmap(":/data/images/ui/account-fail.png");
    account_successPx = QPixmap(":/data/images/ui/account_success.png");
    account_success_pendingPx = QPixmap(":/data/images/ui/account_waiting.png");
    enemyPx = QPixmap(":/data/images/enemy.png");
    pr0coinPx = QPixmap(":/data/images/towers/coin.png");
    baseHerzPx = QPixmap(":/data/images/ui/herz.png");
    wavesPx = QPixmap(":/data/images/ui/waves.png");
    rotorPx = QPixmap(":/data/images/rotor.png");
    shekelMoney = QPixmap(":/data/images/shekel.png");
    lock_quadratPx = QPixmap(":/data/images/schloss_quadrat.png");
    shekelPlusPx = QPixmap(":/data/images/PlusButton.png");
    minusButtonPx = QPixmap(":/data/images/minusButton.png");
    sternPx = QPixmap(":/data/images/stern.png");
    settingsPx = QPixmap(":/data/images/settings.png");
    questionPx = QPixmap(":/data/images/qmark.png");
    popupPx = QPixmap(":/data/images/ui/speed3_popup.png");
    statsPx = QPixmap(":/data/images/stats.png");
    waveBreakPx = QPixmap(":/data/images/ui/waveBreak.png");

    levelsPx = QPixmap(":/data/images/levels.png");
    ownMapsPx = QPixmap(":/data/images/eigene.png");
    communityPx = QPixmap(":/data/images/community.png");

    movePx = QPixmap(":/data/images/ui/move.png");
    delPx = QPixmap(":/data/images/ui/del.png");
    lockPx = QPixmap(":/data/images/schloss.png");
    mPlayPx = QPixmap(":/data/images/ui/mplay.png");
    mStopPx = QPixmap(":/data/images/ui/mstop.png");
    mSavePx = QPixmap(":/data/images/ui/msave.png");
    mEditPx = QPixmap(":/data/images/ui/edit.png");

    towerMenuOpenPx = QPixmap(":/data/images/ui/towermenu/show.png");
    towerMenuClosePx = QPixmap(":/data/images/ui/towermenu/hide.png");

    for(uint i = 0; i < 4; i++) {
        grasAnimation.push_back(QPixmap(":/data/images/Gras/gras"+QString::number(i)+".png"));
    }

    for(uint i = 0; i < 10; i++) {
        repostAnimation.push_back(QPixmap(":/data/images/towers/repost/repost"+QString::number(i)+".png"));
    }

    for(uint i = 0; i < 6; i++) {
        normalEnemyDeath.push_back(QPixmap(":/data/images/enemys/death/normal/Break"+QString::number(i+1)+".png"));
    }

    for(uint i = 0; i < 6; i++) {
        bossEnemyDeath.push_back(QPixmap(":/data/images/enemys/death/boss/BossBreak"+QString::number(i+1)+".png"));
    }

    for(uint i = 0; i < 4595; i++) {
        normalEnemys.push_back(QPixmap(":/data/images/enemys/normal/"+QString::number(i)+".jpg"));
    }

    for(uint i = 0; i < 4; i++) {
        blitzAnimation.push_back(QPixmap(":/data/images/effects/Blitz"+QString::number(i+1)+".png"));
    }

    for(uint i = 0; i < 5; i++) {
        wandAnimation.push_back(QPixmap(":/data/images/effects/art13/"+QString::number(i)+".png"));
    }

    for(uint i = 0; i < 10; i++) {
        towerTemplates.push_back(QPixmap(":/data/images/towers/template/tower"+QString::number(i+1)+".png"));
    }
}

void FrmMain::loadOptions()
{
    QFile file;
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#ifdef Q_OS_WIN
    if(!QDir(path).exists()) {
        QDir().mkdir(path);
    }
#endif
    file.setFileName(":/data/names.dat");
    file.open(QIODevice::ReadOnly);

    QTextStream in(&file);
    QString data = in.readAll();
    QStringList split = data.split("#");
    for(int i = 0; i < split.size(); i++) {
        bossNames.push_back(split[i]);
    }

    file.close();
    path.append("/options.dat");
    file.setFileName(path);
    if(file.exists()) {
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        QString data = in.readAll();
        data.remove("\r\n");
        if(data.size()) {
            QStringList split = data.split("#");
            soundEnabled = split[0].toInt();
            if(split.size() > 1) {
                newestPost = split[1].toInt();
            }
            if(split.size() > 2) {
                accepted = split[2].toInt();
            }
            if(split.size() > 3) {
                unsigned long long a = split[3].toULongLong(nullptr,16);
                setShekel(a / 3, true, true);
            }
            if(split.size() > 4) {
                popup = split[4].toInt();
            }
            if(split.size() > 5) {
                playedGames = split[5].toInt();
            }
            if(split.size() > 6) {
                lastRewardDate = QDate::fromString(split[6],"ddMMyyyy");
            }
            if(split.size() > 7) {
                dailyReward->bonus = split[7].toInt();
            }
            if(split.size() > 8) {
                lastPlayedDate = QDate::fromString(split[8],"ddMMyyyy");
            }
            if(split.size() > 9) {
                settings->graphicQuality = split[9].toInt();
            }
            if(split.size() > 10) {
                settings->energieSparen = split[10].toInt();
            }
            if(split.size() > 11) {
                for(uint i = 0; i < statistics->kills.size(); i++) {
                    statistics->kills[i] = split[11+i].toULongLong(nullptr,16);
                }
                statistics->earnedBenis = split[20].toULongLong(nullptr,16);
                statistics->earnedShekel = split[21].toULongLong(nullptr,16);
                statistics->highestWave = split[22].toULongLong(nullptr,16);
                statistics->enemysPassed = split[23].toULongLong(nullptr,16);
                statistics->playedGames = split[24].toULongLong(nullptr,16);
                statistics->towersBuilt = split[25].toULongLong(nullptr,16);
                statistics->towersSold = split[26].toULongLong(nullptr,16);
                statistics->timePlayed = split[27].toULongLong(nullptr,16);
            }
        }
        file.close();
    }
    path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/lvls.dat");
    file.setFileName(path);
    bool ok = false;
    if(file.exists()) {
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        QString data = in.readAll();
        data.remove("\r\n");
        if(data.size()) {
            if(data.contains('|')) {
                QStringList lvlsplit = data.split("|");
                QString normal = lvlsplit[0];
                QString maze = lvlsplit[1];

                QStringList normalSplit = normal.split("#");
                for(int i = 0; i < normalSplit.size(); i++) {
                    lvlPrices.push_back(normalSplit[i].toInt());
                }

                QStringList mazeSplit = maze.split("#");
                for(int i = 0; i < mazeSplit.size(); i++) {
                    shop->mazelvlPrices[i] = mazeSplit[i].toInt();
                }
            } else {
                QStringList split = data.split("#");
                for(int i = 0; i < split.size(); i++) {
                    lvlPrices.push_back(split[i].toInt());
                }
            }
            ok = true;
        }
        file.close();
    }
    if(!ok) {
        lvlPrices.push_back(0);
        lvlPrices.push_back(300);
        lvlPrices.push_back(500);
        lvlPrices.push_back(1000);
    }
    path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/account.dat");
    file.setFileName(path);
    if(file.exists()) {
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        QString data = in.readAll();
        data.remove("\r\n");
        if(data.size()) {
            QStringList split = data.split("#");
            account.username = split[0];
            account.password = split[1];
            account.accountState = split[2].toInt();
        }
        file.close();
    }
    path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/towers.dat");
    file.setFileName(path);
    ok = false;
    if(file.exists()) {
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        QString data = in.readAll();
        data.remove("\r\n");
        if(data.size()) {
            ok = true;
            QStringList split = data.split("#");
            for(int i = 0;i < split.size(); i++) {
                QString tmpS = split[i];
                QStringList split2 = tmpS.split(",");
                shop->towerPrices.push_back(split2[0].toInt());
                shop->towerLocks.push_back(split2[1].toInt());
            }
        }
        file.close();
    }
    if(!ok) {
        shop->towerPrices.push_back(0);
        shop->towerLocks.push_back(false);

        shop->towerPrices.push_back(0);
        shop->towerLocks.push_back(false);

        shop->towerPrices.push_back(400);
        shop->towerLocks.push_back(true);

        shop->towerPrices.push_back(500);
        shop->towerLocks.push_back(true);

        shop->towerPrices.push_back(750);
        shop->towerLocks.push_back(true);

        shop->towerPrices.push_back(1000);
        shop->towerLocks.push_back(true);

        shop->towerPrices.push_back(1000);
        shop->towerLocks.push_back(true);

        shop->towerPrices.push_back(800);
        shop->towerLocks.push_back(true);

        shop->towerPrices.push_back(1200);
        shop->towerLocks.push_back(true);

        shop->towerPrices.push_back(1100);
        shop->towerLocks.push_back(true);
    }
    loadItems();
}

void FrmMain::loadItems()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/items.dat");
    QFile file;
    file.setFileName(path);
    if(file.exists()) {
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        QString data = in.readAll();
        data.remove("\r\n");
        if(data.size()) {
            QStringList split = data.split("#");
            for(int i = 0;i < split.size(); i++) {
                QString itemData = split[i];
                if(itemData.contains(",")) {
                    QStringList itemSplit = itemData.split(",");
                    shop->items[i].locked = itemSplit[0].toInt();
                    shop->items[i].count = itemSplit[1].toInt();
                } else {
                    shop->items[i].locked = split[i].toInt();
                }
            }
        }
        file.close();
    }
    if(!shop->items[0].locked) superfastUnlocked = true;
}

void FrmMain::changePlaylist(int playlist)
{
    music->stop();
    switch(playlist) {
    case 0: //menu
        music->setPlaylist(playlist_menu);
        playlist_menu->setCurrentIndex(Engine::random(0,2));
        break;
    case 1:
        music->setPlaylist(playlist_game);
        playlist_game->setCurrentIndex(Engine::random(0,2));
        break;
    }
    music->setPosition(0);
}

int FrmMain::createPath()
{
    paths.resize(0);
    begin = -1;
    std::vector <int> starts;
    int maxX = mapwidth/80;
    int maxY = mapheight/80;

    for(uint i=0;i<tiles.size();i++) {
        if(tiles[i]->type==4) { //spawn
            starts.push_back(i);
        }
    }
    if(!starts.size()) {
        return -1; //kein start
    }

    int end = -1;
    for(uint a = 0; a < tiles.size(); a++) {
        if(tiles[a]->type == 5) {
            end = a;
            break;
        }
    }

    if(end == -1) return -2;

    for(uint i=0;i<starts.size();i++) {
        Path tmpPath;
        int beginX = tiles[starts[i]]->rect().x()/80;
        int beginY = tiles[starts[i]]->rect().y()/80;
        int endX = 0;
        int endY = 0;

        if(end > -1) {
            endX = tiles[end]->rect().x()/80;
            endY = tiles[end]->rect().y()/80;
            astar.destroyMatrix();
            astar.setMatrix(maxX, maxY);
            astar.setStart(beginX, beginY);
            astar.setEnd(endX, endY);

            for(uint a = 0; a < tiles.size(); a++) {
                int x = tiles[a]->rect().x()/80;
                int y = tiles[a]->rect().y()/80;

                switch(tiles[a]->type) {
                case TILE_SPAWN:
                case TILE_BASE:
                    break;
                case TILE_PATH: //weg
                case TILE_ART13: //art13
                    astar.setWay(x,y);
                    astar.setExpandCost(1,x,y);
                    break;
                default:
                    astar.setWall(x,y);
                    break;
                }
            }

            list<pair<UINT, UINT> > aStarPath;
            list<pair<UINT, UINT> >::iterator pathIt;

            aStarPath = astar.getPath();
            tmpPath.path.push_back(starts[i]);
            for(pathIt = aStarPath.begin(); pathIt != aStarPath.end(); pathIt++) {
                int tpos = pathIt->first + (pathIt->second * mapwidth/80);
                tmpPath.path.push_back(tpos);
            }

        }

        if(tiles[tmpPath.path[tmpPath.path.size()-1]]->type!=5) {
            return -2; //start = ziel od kein weg gefunden
        }

        paths.push_back(tmpPath);
    }
    return 0; //alles ok
    //path.erase(path.begin()+path.size()-1);
}

int FrmMain::createMazePath(bool tmp)
{
    if(!tmp) {
        paths.resize(0);
    }
    waypoints.resize(0);
    begin = -1;
    int start = -1;
    int maxX = mapwidth/80;
    int maxY = mapheight/80;

    for(uint i=0;i<tiles.size();i++) {
        if(tiles[i]->type==4) { //spawn
            start = i;
            break;
        }
    }
    if(start < 0) {
        return -1; //kein start
    }

    int end = -1;
    for(uint a = 0; a < tiles.size(); a++) {
        if(tiles[a]->type == 5) {
            end = a;
            break;
        }
    }

    int mend = end;
    if(end == -1) return -2;

    for(uint i = 0; i < tiles.size(); i++) {
        if(tiles[i]->type >= TILE_WAYP) {
            waypoints.push_back(i);
        }
    }
    end = waypoints[0];
    //sortiert wegpunkte von klein nach groß
    if(waypoints.size() > 1) {
        std::sort(waypoints.begin(), waypoints.end());
    }

    Path tmpPath;
    int beginX = tiles[start]->rect().x()/80;
    int beginY = tiles[start]->rect().y()/80;
    int endX = 0;
    int endY = 0;

    tmpPath.path.push_back(start);

    for(int i = -1; i < (int)waypoints.size(); i++) {
        if(i > -1) {
            start = waypoints[i];
            beginX = tiles[start]->rect().x()/80;
            beginY = tiles[start]->rect().y()/80;
        }

        if(i+1 < (int)waypoints.size()) {
            end = waypoints[i+1];
        } else {
            end = mend;
        }

        if(end > -1) {
            endX = tiles[end]->rect().x()/80;
            endY = tiles[end]->rect().y()/80;
            astar.destroyMatrix();
            astar.setMatrix(maxX, maxY);
            astar.setStart(beginX, beginY);
            astar.setEnd(endX, endY);

            for(uint a = 0; a < tiles.size(); a++) {
                int x = tiles[a]->rect().x()/80;
                int y = tiles[a]->rect().y()/80;

                switch(tiles[a]->type) {
                case TILE_SPAWN:
                case TILE_BASE:
                    break;
                case TILE_PATH: //weg
                case TILE_ART13: //art13
                    if(tmpPath.contains(a)) {
                        astar.setWall(x,y);
                    } else {
                        astar.setWay(x,y);
                        astar.setExpandCost(1,x,y);
                    }
                    break;
                default:
                    if(tiles[a]->type < TILE_WAYP) {
                        astar.setWall(x,y);
                    }
                    break;
                }
            }

            list<pair<UINT, UINT> > aStarPath;
            list<pair<UINT, UINT> >::iterator pathIt;

            aStarPath = astar.getPath();
            for(pathIt = aStarPath.begin(); pathIt != aStarPath.end(); pathIt++) {
                int tpos = pathIt->first + (pathIt->second * mapwidth/80);
                tmpPath.path.push_back(tpos);
            }

        }
        if(tiles[tmpPath.path[tmpPath.path.size()-1]]->type < TILE_WAYP && i+1 < (int)waypoints.size()) {
            return -2;
        }
    }

    if(tiles[tmpPath.path[tmpPath.path.size()-1]]->type!=5) {
        return -2; //start = ziel od kein weg gefunden
    }

    if(!tmp) {
        paths.push_back(tmpPath);
    } else {
        tempPath.path = tmpPath.path;
    }
    return 0; //alles ok
}

void FrmMain::zoom(double amount)
{
    double currentScaleFactor = amount;
    mid = viewRect.center();
    double scale = (double)1/currentScaleFactor;
    double nw = viewRect.width()*scale;
    double nh = viewRect.height()*scale;
    double ntx, nty;
    bool reset = false;
    if(active == STATE_PLAYING) {
        ntx = (mid.x()-nw/2)*-1;
        nty = (mid.y()-nh/2)*-1;
        bool ok = true;
        if((-ntx < 0) || (-ntx + nw > mapwidth) ||
                (-nty < 0) || (-nty + nh > mapheight)) {
            if(nh > mapheight) nh = mapheight;
            if(nw > mapwidth) nw = mapwidth;
            if(-ntx < 0) ntx = 0;
            if(-ntx + nw > mapwidth) ntx += (-ntx+nw) - mapwidth;
            if(-nty < 0) nty = 0;
            if(-nty + nh > mapheight) nty += (-nty+nh) - mapheight;
            ok = false;
            reset = true;
        } else if(nw<600||nh<337) {
            nw = 600;
            nh = 337;
            reset = true;
        }
        if(ok) {
            ntx = (mid.x()-nw/2)*-1;
            nty = (mid.y()-nh/2)*-1;
        }
    } else {
        if(nw>mapwidth*2||nh>mapheight*2) {
            nw = mapwidth*2;
            nh = mapheight*2;
            ntx = 960;
            nty = 540;
            reset = true;
        } else if(nw<600||nh<337) {
            nw = 600;
            nh = 337;
            reset = true;
        }
        ntx = (mid.x()-nw/2)*-1;
        nty = (mid.y()-nh/2)*-1;
    }
    transX = ntx;
    transY = nty;
    viewRect.moveTo(-ntx,-nty);
    viewRect.setWidth(nw);
    viewRect.setHeight(nh);
    if(reset) {
        isZooming = false;
    }
}

void FrmMain::createPathBoundaries()
{
    for(uint i = 0; i < paths.size(); i++) {
        Path currentPath = paths[i];
        for(uint a = 0; a < currentPath.path.size(); a++) {
            uint currentTile = (uint)currentPath.path[a];
            QRect tmp = tiles[currentTile]->rect();
            tmp.adjust(-2,-2,2,2);

            for(uint b = 0; b < tiles.size(); b++) {
                if(!pathsContains(b) && b != currentTile) {
                    if(tiles[b]->type != 4 &&
                            tiles[b]->type != 5) {
                        if(tiles[b]->rect().intersects(tmp)) {
                            checkPush(tiles[b]);
                        }
                    }
                }
            }
        }
    }
}

void FrmMain::loginAccount()
{
    QGuiApplication::inputMethod()->show();
    QString name = QInputDialog::getText(this, "Benutzername", "Benutzername: (2-20 Zeichen lang)(Darf nur Zahlen & Buchstaben enthalten!)");
    bool ok = true;
    if(name.size() < 2 || name.size() > 20) ok=false;
    for(int i = 0; i < name.size(); i++) {
        if(!name[i].isLetter()&&!name[i].isDigit()) {
            ok = false;
        }
    }
    if(ok) { //Name ok
        QGuiApplication::inputMethod()->show();
        QString password = QInputDialog::getText(this, "Passwort", "Passwort: (3-20 Zeichen lang)(Darf nur Zahlen & Buchstaben enthalten!)",
                                                 QLineEdit::Password);
        bool pwok = true;
        if(password.size() < 3 || password.size() > 20) pwok = false;
        for(int i = 0; i < password.size(); i++) {
            if(!password[i].isLetter()&&!password[i].isDigit()) {
                pwok = false;
            }
        }
        if(pwok) { //passwort ok
            QMessageBox::information(this,"Info","Es werden jetzt Benutzername & Passwort geprüft.<br>"
                                                 "Dies kann bis zu 10 Sekunden dauern!");
            int code = account.login(name, password);
            switch(code) {
            case -1: //server nicht erreichbar
                QMessageBox::critical(this,"Fehler", "Server nicht erreichbar!");
                break;
            case 0: //Falscher Username / Passwort
                QMessageBox::critical(this, "Fehler", "Falscher Benutzername oder Passwort!");
                break;
            case 1: //alles ok
                saveAccount();
                QMessageBox::information(this,"Info", "Login erfolgreich!");
                break;
            }
        }
    }
}

void FrmMain::createAccount()
{
    QGuiApplication::inputMethod()->show();
    QString name = QInputDialog::getText(this, "Benutzername", "Benutzername: (2-20 Zeichen lang)(Darf nur Zahlen & Buchstaben enthalten!)");
    bool ok = true;
    if(name.size() < 2 || name.size() > 20) ok=false;
    for(int i = 0; i < name.size(); i++) {
        if(!name[i].isLetter()&&!name[i].isDigit()) {
            ok = false;
        }
    }
    if(ok) {
        QMessageBox box;
        MsgBox::createMsgBox(box, "Ist '"+name+"' dein Benutzername?");
        int ret = box.exec();
        if(ret == 0) {//Ja
            QMessageBox::information(this,"Info", "Es wird jetzt geprüft, ob der Name bereits verwendet wurde.\n"
                                                  "Dies kann bis zu 10 Sekunden dauern.");
            int ok = account.checkName(name) ;
            if(ok == 1) { //Alles ok
                QGuiApplication::inputMethod()->show();
                QString password = QInputDialog::getText(this, "Passwort", "Der Name ist noch Frei.\n"
                                                                       "Passwort: (3-20 Zeichen lang)(Darf nur Zahlen & Buchstaben enthalten!)", QLineEdit::Password);
                bool pwok = true;
                if(password.size() < 3 || password.size() > 20) pwok = false;
                for(int i = 0; i < password.size(); i++) {
                    if(!password[i].isLetter() && !password[i].isDigit()) {
                        pwok = false;
                    }
                }
                if(pwok) {
                    QMessageBox::information(this,"Info","Der Account wird jetzt erstellt. \n"
                                                         "Dies kann bis zu 10 Sekunden dauern.");
                    int code = account.registerAccount(name, password);
                    if(code == 1) {
                        saveAccount();
                        QMessageBox::information(this,"Info","Registrierung erfolgreich!");
                    } else if(!code) {
                        accountCreationError();
                    } else if(code == -1) {
                        QMessageBox::information(this,"Fehler","Server nicht erreichbar! Bitte versuche es später noch einmal!");
                    }
                } else {
                    accountCreationError(1);
                }
            } else if(!ok){ //Name verwendet
                QMessageBox::information(this,"Fehler","Name bereits verwendet! Versuche es nochmal!");
            } else if(ok == -1) { //server offline
                QMessageBox::information(this,"Fehler","Server nicht erreichbar! Bitte versuche es später noch einmal!");
            }
        } else {
            accountCreationError();
        }
    } else accountCreationError(2);
}

void FrmMain::saveAccount()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/account.dat");
    QFile file;
    file.setFileName(path);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream out(&file);
    out << account.username << "#" << account.password << "#" << QString::number(account.accountState);
    file.close();
}

void FrmMain::accountCreationError(int id)
{
    QString text = "<b>ZOMG!</b><br>"
                   "Klicke auf das Accountsymbol,<br>"
                   "um es erneut zu probieren!";
    switch(id) {
    case 1:
        text = "<b>ZOMG!</b><br>"
                "Dein Passwort entspricht nicht den Anforderungen!";
        break;
    case 2:
        text = "<b>ZOMG!</b><br>"
                "Dein Benutzername entspricht nicht den Anforderungen!";
        break;
    }
    QMessageBox::information(this, "Info", text);
}

bool FrmMain::event(QEvent *e)
{
    switch(e->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        touchEvent(static_cast<QTouchEvent*>(e));
    default:
        try {
            return QOpenGLWidget::event(e);
        } catch(std::exception &e) {
            qDebug()<<"error";
        } catch(...) {
            qDebug()<<"error2";
        }
    }
    return true;
}

void FrmMain::touchEvent(QTouchEvent *e)
{
    if((gamePaused && backMenu) || isGameOver || active == STATE_MAINMENU || settings->zoomMode == 1) return;
    if(!zooming) zoomScale = scaleX;
    QList<QTouchEvent::TouchPoint> touchPoints = e->touchPoints();
    if(zooming && e->type() == QEvent::TouchEnd && e->touchPoints().count() == 1) {
        if(zooming>0) zooming--;
    }
    if(touchPoints.count() == 2) {
        const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
        const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
        if(zooming && touchPoint0.state() == Qt::TouchPointReleased) {
            if(zooming>0) zooming--;
        }
        if(zooming && touchPoint1.state() == Qt::TouchPointReleased) {
            if(zooming>0) zooming--;
        }
        //touchPoint0.pos()
        double l1 = QLineF(touchPoint0.pos(),touchPoint1.pos()).length();
        double l2 = QLineF(touchPoint0.startPos(),touchPoint1.startPos()).length();
        double currentScaleFactor = l1/l2;
        if(e->touchPointStates()==Qt::TouchPointPressed||e->touchPointStates()==Qt::TouchPointMoved) {
            if(!zooming) zooming = 2;
            mid = viewRect.center();
            if(currentScaleFactor>1) {
                currentScaleFactor = 1.03;
            } else if(currentScaleFactor<1) {
                currentScaleFactor = 0.97;
            }
            double scale = (double)1/currentScaleFactor;
            double nw = viewRect.width()*scale;
            double nh = viewRect.height()*scale;
            double ntx, nty;
            if(active == STATE_PLAYING) {
                ntx = (mid.x()-nw/2)*-1;
                nty = (mid.y()-nh/2)*-1;
                bool ok = true;
                if((-ntx < 0) || (-ntx + nw > mapwidth) ||
                        (-nty < 0) || (-nty + nh > mapheight)) {
                    if(nh > mapheight) nh = mapheight;
                    if(nw > mapwidth) nw = mapwidth;
                    if(-ntx < 0) ntx = 0;
                    if(-ntx + nw > mapwidth) ntx += (-ntx+nw) - mapwidth;
                    if(-nty < 0) nty = 0;
                    if(-nty + nh > mapheight) nty += (-nty+nh) - mapheight;
                    ok = false;
                } else if(nw<600||nh<337) {
                    nw = 600;
                    nh = 337;
                }
                if(ok) {
                    ntx = (mid.x()-nw/2)*-1;
                    nty = (mid.y()-nh/2)*-1;
                }
            } else {
                if(nw>mapwidth*2||nh>mapheight*2) {
                    nw = mapwidth*2;
                    nh = mapheight*2;
                    ntx = 960;
                    nty = 540;
                } else if(nw<600||nh<337) {
                    nw = 600;
                    nh = 337;
                }
                ntx = (mid.x()-nw/2)*-1;
                nty = (mid.y()-nh/2)*-1;
            }
            transX = ntx;
            transY = nty;
            viewRect.moveTo(-ntx,-nty);
            viewRect.setWidth(nw);
            viewRect.setHeight(nh);
            currentScaleFactor = 1;
        }
    }
}

void FrmMain::mousePressEvent(QMouseEvent *e)
{
    if(zooming) return;
    pressed = true;
    moved = 0;
    int x = e->pos().x();
    int y = e->pos().y();
    mPos = QPoint(x,y);
    x = (e->pos().x()/scaleX)-transX;
    y = (e->pos().y()/scaleY)-transY;
    int x2 = (e->pos().x()/scaleFHDX);
    int y2 = (e->pos().y()/scaleFHDY);
    QRect mRect(x,y,1,1);
    QRect mRectRaw(x2,y2,1,1);
    if(active == STATE_EDITOR) {
        editorClicked(mRectRaw,mRect);
    }
}

void FrmMain::mouseMoveEvent(QMouseEvent *e)
{
    if(active == STATE_MAINMENU || (gamePaused&&backMenu) || isGameOver) return;
    int x = e->pos().x();
    int y = e->pos().y();
    if(towerMenu > -1 && !towerMenuHidden) {
        if(QRect(mPos.x()/scaleFHDX,mPos.y()/scaleFHDY,1,1).intersects(towerMenuRect)) return;
    }
    QPoint nPos(x,y);
    if(zooming) {
        resetPos = true;
        return;
    } else if(resetPos) {
        resetPos = false;
        mPos = nPos;
        return;
    }
    double xdis = (mPos.x()-nPos.x())/scaleX;
    double ydis = (mPos.y()-nPos.y())/scaleY;
    moved++;
    mPos = nPos;
    if(pressed) {
        QRectF nV = viewRect;
        if(active == STATE_PLAYING) {
            bool okX = true, okY = true;
            nV.moveTo(viewRect.x()+xdis,viewRect.y()+ydis);
            if((nV.x() < 0) || (nV.x() + nV.width() > mapwidth)) {
                okX = false;
            }

            if((nV.y() < 0) || (nV.y() + nV.height() > mapheight)) {
                okY = false;
            }

            if(okX) {
                viewRect.moveTo(viewRect.x()+xdis, viewRect.y());
                transX += xdis*-1;
            }
            if(okY) {
                viewRect.moveTo(viewRect.x(), viewRect.y()+ydis);
                transY += ydis*-1;
            }
        } else if(active == STATE_EDITOR){
            if(!editMode) {
                QRectF nVO;
                nV.moveTo(viewRect.x()+xdis,viewRect.y()+ydis);
                nVO = nV;
                bool ok=false;
                nV.adjust(+200,+150,-150,-150);
                for(uint i=0;i<tiles.size();i++) {
                    if(tiles[i]->pos.intersects(nV)) {
                        ok = true;
                        break;
                    }
                }
                if(ok) {
                    viewRect = nVO;
                    transX += xdis*-1;
                    transY += ydis*-1;
                }
            } else {
                if(!mapPlaying && !isEditorMenuClicked) {
                    int x_t = (e->pos().x()/scaleX)-transX;
                    int y_t = (e->pos().y()/scaleY)-transY;
                    QRect aPos = QRect(x_t, y_t, 1, 1);
                    for(uint i=0;i<tiles.size();i++) {
                        if(aPos.intersects(tiles[i]->rect())) {
                            if(editorSelected==5) {
                                if(!ownBaseCount) {
                                    ownBaseCount++;
                                    tiles[i]->type = editorSelected;
                                }
                            } else if(editorSelected==4) {
                                //if(!enemyBaseCount) {
                                    enemyBaseCount++;
                                    tiles[i]->type = editorSelected;
                                //}
                            } else if(editorSelected) {
                                if(tiles[i]->type==5) ownBaseCount--;
                                if(tiles[i]->type==4) enemyBaseCount--;
                                tiles[i]->type = editorSelected;
                            }
                        }
                    }
                }
            }
        }
    }
}

void FrmMain::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    if(zooming||moved>5) return;
    pressed = false;
    bool playingSpeedChanged=false;
    int x = (e->pos().x()/scaleX)-transX;
    int y = (e->pos().y()/scaleY)-transY;
    int x2 = (e->pos().x()/scaleFHDX);
    int y2 = (e->pos().y()/scaleFHDY);
    QRect mRect(x,y,1,1);
    QRect mRectRaw(x2,y2,1,1);

    if(isGameOver) {
        moveAn = ANIMATION_BACK;
        fdir = 0;
        if(playingSpeed != 1) {
            playingSpeed = 1;
            resetTimers();
        }
        QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
        return;
    }

    if(gameLoading) {
        if(!accepted) {
            if(mRectRaw.intersects(okButtonRect)) {
                accepted = true;
                gameLoading = false;
            } else if(mRectRaw.intersects(dsButtonRect)) {
                QString linkS = "https://pastebin.com/CJvvFEQr";
                QUrl link(linkS);
                QDesktopServices::openUrl(link);
            } else if(mRectRaw.intersects(nbButtonRect)) {
                QString linkS = "https://pastebin.com/BQcicxpA";
                QUrl link(linkS);
                QDesktopServices::openUrl(link);
            }
        }
        return;
    }

    if(isLoading() && active == STATE_PLAYING) return;

    if(gamePaused) {
        if(mRectRaw.intersects(btnPlayingSpeedRect) && !playingSpeed && active == STATE_PLAYING && !backMenu) {
            gamePaused = false;
            playingSpeed = 1;
            if(loaded) loaded = false;
            playingSpeedChanged = true;
            return;
        } else if(backMenu){
            backMenuClicked(mRectRaw);
            return;
        }
    }

    if(dailyReward->active) {
        dailyReward->clicked(mRectRaw);
        return;
    }
    //int amount = 2800;
    switch(active) {
    case STATE_MAINMENU: //hauptmenü
        if(popup != 2 && shop->shekel > 100 && !dailyReward->active && lastRewardDate.year() == 2000) {
            popup = 2;
        }
        switch(subActive) {
        case 1: //spielen
            startMenuClicked(mRectRaw);
            break;
        case 2: //mapeditor
            editorMenuClicked(mRectRaw);
            break;
        case 3: //shop
            shop->shopClicked(mRectRaw);
            break;
        case 4: //settings
            settings->settingsClicked(mRectRaw);
            break;
        }
        if(mRectRaw.intersects(btnContinueMenuRect)) {
            if(hasPlayingSave) {
                moveAn = ANIMATION_CONTINUE;
                fdir = 0;
                QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
            }
        } else if(mRectRaw.intersects(startRect)) {//start
            if(subActive!=1) {
                subActive = 1;
                subActiveSelected = 0;
                subLevelSelected = 0;
            }
        } else if(mRectRaw.intersects(editRect)) {//mapeditor
            if(subActive!=2) {
                subActive = 2;
                subActiveSelected = 0;
                subLevelSelected = 0;
            }
            /**/
        } else if(mRectRaw.intersects(exitRect)) { //verlassen
            this->close();
        } else if(mRectRaw.intersects(btnSoundRect) && musicLoaded) { //sound
            if(soundEnabled) {
                soundEnabled = false;
                music->pause();
            } else {
                soundEnabled = true;
                music->play();
            }
        } else if(mRectRaw.intersects(shopRect)) { //shop
            shop->subMenu = 0;
            if(subActive != 3) {
                subActive = 3;
            }
        } else if(mRectRaw.intersects(accountRect)) { //account
            if(account.accountState == ACCOUNT_INACTIVE) {
                QMessageBox box;
                MsgBox::createMsgBox(box, "Du bist aktuell noch <b>nicht</b> angemeldet!<br>"
                                          "Ein Account bietet dir folgende Vorteile:<br>"
                                          "- Statistiken aktivieren<br>"
                                          "Bald noch vieles mehr!",
                                        "Registrieren",
                                        "Anmelden",
                                        "Abbrechen");
                int ret = box.exec();
                if(ret == 0) { //Ja geklickt
                    createAccount();
                } else if(ret == 1) {//login
                    loginAccount();
                }
            } else if(account.accountState == ACCOUNT_PENDING) {
                QMessageBox::information(this,"Info", "Warte auf Antwort vom Server\n"
                                                      "Falls du mit dem Internet verbunden bist\n"
                                                      "starte die App neu.");
            } else if(account.accountState == ACCOUNT_ACTIVE ||
                      account.accountState == ACCOUNT_VERIFIED ||
                      account.accountState == ACCOUNT_CHEATER) {
                QMessageBox box;
                MsgBox::createMsgBox(box, "Du bist aktuell mit <b>"+account.username+"</b> angemeldet",
                                     "Okay", "Abmelden");
                int code = box.exec();
                if(code == 1){ //Abmelden
                    QMessageBox box;
                    MsgBox::createMsgBox(box, "Wirklich abmelden?");
                    code = box.exec();
                    if(code == 0) {//Ja
                        if(account.accountState == ACCOUNT_CHEATER) {
                            QMessageBox::information(this,"Info", "Neim!");
                        } else {
                            account.username = "";
                            account.password = "";
                            account.accountState = ACCOUNT_INACTIVE;
                            saveAccount();
                            QMessageBox::information(this, "Info", "Erfolgreich abgemeldet!");
                        }
                    }
                }
            }
        } else if(mRectRaw.intersects(shekelPlusRect)) {
            subActive = 3;
            shop->subMenu = 3;
            shop->subSelected = -1;
        } else if(mRectRaw.intersects(settingsRect)) {
            subActive = 4;
        } else if(mRectRaw.intersects(questionRect)) {
            QString link = "https://github.com/FireDiver/Pr0fense/blob/master/README.md";
            QUrl url(link);
            QDesktopServices::openUrl(url);
        } else if(mRectRaw.intersects(statsRect)) {
            if(subActive != 5) {
                subActive = 5;
            }
        }
        break;
    case STATE_PLAYING: //ingame
        bool hudClicked = false;
        if(mRectRaw.intersects(towerMenuCloseRect) && towerMenu > -1 && !towerMenuHidden) {
            towerMenuHidden = true;
            towerMenuAnimating = towerMenu;
            towerMenuAnimatingDir = 1; //schließen
            hudClicked = true;
        } else if(mRectRaw.intersects(towerMenuOpenRect) && towerMenu > -1 && towerMenuHidden) {
            towerMenuHidden = false;
            towerMenuAnimating = towerMenu;
            towerMenuAnimatingDir = 0; //öffnen
            hudClicked = true;
        } else if(mRectRaw.intersects(wandRect) && shop->items[1].count > 0) {
            powerupSelected = 1;
            hudClicked = true;
        } else if(mRectRaw.intersects(btnZoomMinusRect) && settings->zoomMode == 1) {
            if(!isZooming) {
                maxZoomTime = 20;
                zoomTime = maxZoomTime;
            } else {
                zoomTime += maxZoomTime / 2;
            }
            if( isZooming != 2) { //nach unten
                isZooming = 2;
                zoomAmount = 0.02;
            }
            hudClicked = true;
        } else if(mRectRaw.intersects(btnZoomPlusRect) && settings->zoomMode == 1) {
            if(!isZooming) {
                maxZoomTime = 20;
                zoomTime = maxZoomTime;
            } else {
                zoomTime += maxZoomTime / 2;
            }
            if(isZooming != 1) {//nach oben
                isZooming = 1;
                zoomAmount = 0.02;
            }
            hudClicked = true;
        } else if(mRectRaw.intersects(btnPlayingSpeedRect) && !playingSpeedChanged) {
            //qDebug()<<enemys.size();
            if(!waveBreak) {
                switch(playingSpeed) {
                case 1: //normal
                    do {
                        QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,25));
                        QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,5));
                        QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,5));
                        QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,50));
                        QMetaObject::invokeMethod(t_grasAn,"start",Q_ARG(int,250));
                    } while(!checkTimers());
                    playingSpeed = 2;
                    break;
                case 2: //schnell
                    if(!superfastUnlocked) {
                        do {
                            QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,50));
                            QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,10));
                            QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,10));
                            QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100));
                            QMetaObject::invokeMethod(t_grasAn,"start",Q_ARG(int,500));
                        } while(!checkTimers());
                        playingSpeed = 1;
                    } else {
                        do {
                            QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,12));
                            QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,2));
                            QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,2));
                            QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,25));
                            QMetaObject::invokeMethod(t_grasAn,"start",Q_ARG(int,125));
                        } while(!checkTimers());
                        playingSpeed = 3;
                    }
                    break;
                case 3: //superschnell
                    do {
                        QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,50));
                        QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,10));
                        QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,10));
                        QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100));
                        QMetaObject::invokeMethod(t_grasAn,"start",Q_ARG(int,500));
                    } while(!checkTimers());
                    playingSpeed = 1;
                    break;
                }
            } else {
                if(waveBreak) waveBreak = 0;
            }
            hudClicked = true;
        }

        //6 max towermenuselected
        if(towerMenu > -1 && !towerMenuHidden) {
            //warum nicht in towermenuclicked()? -> wird erst später aufgerufen
            Tower *tmpTower = tiles[towerMenu]->t;
            if(tmpTower != nullptr) {
                if(mRect.intersects(tmpTower->upgradePos) && !mRectRaw.intersects(towerMenuRect) && !tmpTower->disabled) {
                    if(towerMenuSelected != 7) { //auswählen
                        Tower *t = tmpTower;
                        if((t->dmglvl < 3 && t->hasDamage) ||
                                (t->speedlvl < 3 && t->hasProjectileSpeed) ||
                                (t->turnlvl < 3 && t->hasTurnSpeed) ||
                                (t->rangelvl < 3 && t->hasRange) ||
                                (t->ratelvl < 3 && t->hasFirerate)) { //upgrade geht
                            towerMenuSelected = 7;
                            hudClicked = true;
                        }
                    } else if(towerMenuSelected == 7) { //kauf checken
                        if(upgradeCost <= benis) {
                            setBenis(benis - upgradeCost);

                            if(tmpTower->hasDamage) {
                                for(int i = tmpTower->dmglvl; i < 3; i++) {
                                    upgradeDmg(tmpTower);
                                }
                                tmpTower->dmglvl = 3;
                            }

                            if(tmpTower->hasRange) {
                                for(int i = tmpTower->rangelvl; i < 3; i++) {
                                    tmpTower->range += 10;
                                }
                                tmpTower->rangelvl = 3;
                            }

                            if(tmpTower->hasFirerate) {
                                for(int i = tmpTower->ratelvl; i < 3; i++) {
                                    tmpTower->reload *= 0.85;
                                }
                                tmpTower->ratelvl = 3;
                            }

                            if(tmpTower->hasProjectileSpeed) {
                                for(int i = tmpTower->speedlvl; i < 3; i++) {
                                    tmpTower->pspeed += 1;
                                }
                                tmpTower->speedlvl = 3;
                            }

                            if(tmpTower->hasTurnSpeed) {
                                for(int i = tmpTower->turnlvl; i < 3; i++) {
                                    tmpTower->angleSpeed *= 1.2;
                                }
                                tmpTower->turnlvl = 3;
                            }

                            towerMenuSelected = -1;
                        }
                        hudClicked = true;
                    }
                }
            }
        }

        if(!hudClicked && (towerMenu == -1 || (!mRectRaw.intersects(towerMenuRect) &&
                  !QRect(mPos.x()/scaleFHDX, mPos.y()/scaleFHDY, 1, 1).intersects(towerMenuRect) &&
                  towerMenuAnimating == -1) || (towerMenuHidden))) {

            if((!mRectRaw.intersects(towerMenuRect) || towerMenuHidden) && towerMenu >- 1) {
                towerMenuSelected = 0;
                bool ok = false;
                for(uint i = 0; i < tiles.size(); i++) {
                    if(mRect.intersects(tiles[i]->rect())) {
                        if(((tiles[i]->type == 2 && playingMode == MODE_NORMAL) ||
                            ((tiles[i]->type == 1 || tiles[i]->type == 2) && playingMode == MODE_MAZE)) &&
                                (int)i != towerMenu) {//turm
                            if(tiles[i]->t == nullptr &&
                                    tiles[towerMenu]->t == nullptr) {
                                ok = true;
                                if(!(std::find(chosenTiles.begin(), chosenTiles.end(), i) != chosenTiles.end())) {
                                    chosenTiles.push_back(i);
                                } else { //entfernen
                                    for(uint a = 0; a < chosenTiles.size(); a++) {
                                        if(chosenTiles[a] == (int)i) {
                                            chosenTiles.erase(chosenTiles.begin() + a);
                                            break;
                                        }
                                    }
                                }
                            } else {
                                //wenn turm dann range anzeigen
                                ok = true;
                                chosenTiles.resize(0);
                                if((int)i != towerMenu) {
                                    if(tiles[towerMenu]->t != nullptr) tiles[towerMenu]->t->tmpRangeDir = 1;
                                    if(tiles[i]->t != nullptr) tiles[i]->t->tmpRangeDir = 0;
                                }
                                towerMenu = i;
                            }
                        }
                    }
                }
                if(!ok) { //verschwinden animieren
                    if(!chosenTiles.size()) {
                        if(tiles[towerMenu]->t != nullptr) tiles[towerMenu]->t->tmpRangeDir = 1; //kleiner
                    }
                    chosenTiles.resize(0);
                    towerMenu = -1;
                    enemyWarning = false;
                    if(playingMode == MODE_MAZE) {
                        tempPath.path.resize(0);
                    }
                    if(!towerMenuHidden) {
                        towerMenuAnimating = 0;
                        towerMenuAnimatingDir = 1;
                    }
                }
            } else {
                for(uint i=0;i<tiles.size();i++) {
                    if(mRect.intersects(tiles[i]->rect())) {
                        if((tiles[i]->type == 2 && playingMode == MODE_NORMAL) ||
                                ((tiles[i]->type == 1 || tiles[i]->type == 2) && playingMode == MODE_MAZE)) {//turm
                            if(tiles[i]->t != nullptr) {
                                tiles[i]->t->tmpRangeDir = 0; //größer
                            }
                            if(!towerMenuHidden) {
                                towerMenuAnimating = i;
                                towerMenuAnimatingDir = 0;
                                //towerMenu = i;
                                towerMenuRectAnimation = towerMenuRect;
                                towerMenuRectAnimation.moveTo(-600,towerMenuRectAnimation.y());
                            } else {
                                towerMenu = i;
                            }
                            towerMenuSelected = 0;
                            break;
                        }
                    }
                }
            }
        } else {
            if(!towerMenuHidden) {
                towerMenuClicked(mRectRaw);
                hudClicked = true;
            }
        }

        if(!hudClicked && towerMenu == -1) {
            if(powerupSelected == 1) { //Wand
                placeWall(mRect.center());
                powerupSelected = -1;
            }
        }

        break;
    }
}

void FrmMain::keyPressEvent(QKeyEvent *e)
{
    if(moveAn != ANIMATION_SUSPENDED) return;
    if(e->key()==Qt::Key_Escape||e->key()==Qt::Key_Back) {
        if((active == STATE_PLAYING && !isGameOver && !isLoading()) || active == STATE_EDITOR) { //Im Spiel oder Editor
            if(!backMenu) {
                backMenu = true;
                gamePaused = true;
                mapPlaying = false;
            } else {
                backMenu = false;
                gamePaused = false;
            }
        } else if(active == STATE_MAINMENU) {
            if(subActive && !shop->subMenu) {
                subActive = 0;
            } else {
                if(shop->subMenu) {
                    shop->subMenu = 0;
                } else {
                    this->close();
                }
            }
        }
    }
    if(e->key() == Qt::Key_F11) {
        if(this->isMaximized()) {
            this->setWindowFlags(Qt::Widget);
            QScreen *screen = QGuiApplication::screenAt(QPoint(300,300));
            QRect r = screen->geometry();
            this->move(r.center().x()-960/2, r.center().y()-540/2);
            this->resize(960,540);
            this->showNormal();
        } else {
            this->setWindowFlag(Qt::FramelessWindowHint);
            this->showMaximized();
        }
    }
}

void FrmMain::paintEvent(QPaintEvent *e)
{
    if(!key || closing) return;
    mutex.lock();
    try {
        Q_UNUSED(e)
        if(DEBUG) {
            debugns = 0;
            if(output) {
                qDebug()<<"paint";
            }
            timerD.restart();
        }
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        //painter.setRenderHint(QPainter::SmoothPixmapTransform);
        //qDebug()<<"VIEW:"<<viewRect;
        scaleX = double(this->geometry().width()/double(viewRect.width()));
        scaleY = double(this->geometry().height()/double(viewRect.height()));
        scaleFHDX = double(this->geometry().width()/double(1920));
        scaleFHDY = double(this->geometry().height()/double(1080));
        if(active == STATE_PLAYING || active == STATE_EDITOR) painter.save();
        painter.scale(scaleX,scaleY);
        painter.setBrush(Engine::getColor(-1));
        painter.translate(transX-shakeX,transY-shakeY);
        painter.drawRect(viewRect);
        if(gameLoading) {
            changeSize(painter,46,true);
            if(!accepted) {
                painter.drawPixmap(QRect(704,0,512,512), pr0fensePx);
                painter.setPen(Qt::red);
                painter.drawText(QRect(0,150,1920,1080),
                                 Qt::AlignCenter,
                                 "🢃🢃🢃🢃🢃 BITTE LESEN 🢃🢃🢃🢃🢃\n"
                                 "Pr0fense ist kostenlos spielbar aber durch In-App käufe\n"
                                 "kann dein Fortschritt beschleunigt werden.\n\n"
                                 "Tippe auf 'Okay', um unsere Datenschutzrichtlinie (1)\n"
                                 "und die Nutzungsbedingungen (2) zu akzeptieren.");
                Engine::drawButton(painter, okButtonRect, "Okay");
                Engine::drawButton(painter, dsButtonRect, "1");
                Engine::drawButton(painter, nbButtonRect, "2");
            } else {
                painter.drawPixmap(QRect(704,284,512,512), pr0fensePx);
            }
            changeSize(painter,32,true);
            painter.setPen(Qt::white);
            painter.drawText(QRect(0,0,1920,1080),Qt::AlignBottom | Qt::AlignLeft,
                             "Bruh Games präsentiert: Pr0fense - ein pr0 Spiel");
            painter.drawText(QRect(0,0,1920,1080),Qt::AlignBottom | Qt::AlignRight,
                             "Built with Qt - Released under GNU GPL");
            mutex.unlock();
            return;
        }
        switch(active) {
        case STATE_MAINMENU: //menu
            //iwas blockiert den thread bei active = 1 für paar sec
            if(moveAn > 0 && (moveAn < ANIMATION_BACK || moveAn == ANIMATION_CONTINUE) && fdir) {
                drawIngame(painter);
            } else {
                drawMainMenu(painter);
                if(popup != 2 && shop->shekel > 100 && !dailyReward->active && lastRewardDate.year() == 2000) {
                    painter.setBrush(Engine::getColor(-1));
                    painter.drawRect(QRect(popupRect.x()-2, popupRect.y()-2, 1284, 724));
                    painter.drawPixmap(popupRect, popupPx);
                } else if(dailyReward->active) {
                    painter.setOpacity(0.4);
                    painter.setBrush(Engine::getColor(-1));
                    painter.drawRect(0,0,1920,1080);
                    painter.setOpacity(1);
                    dailyReward->drawDailyReward(painter);
                }
            }
            if(outdated && !moveAn) {
                changeSize(painter,32,true);
                painter.setPen(Qt::magenta);
                painter.drawText(QRect(0,1040,1920,40),Qt::AlignRight,"Update ist im Playstore und auf Github verfügbar!");
                painter.setPen(Qt::NoPen);
            }
            break;
        case STATE_PLAYING: //ingame
        case STATE_EDITOR: //mapeditor
            drawIngame(painter);
            painter.restore();
            painter.scale(scaleFHDX,scaleFHDY);
            if(!isGameOver) {
                drawHUD(painter);
            } else {
                painter.setOpacity(0.6*fade);
                painter.setBrush(Qt::black);
                painter.drawRect(0,0,1920,1080);
                painter.setOpacity(fade);
                painter.setPen(Qt::white);
                changeSize(painter,192,true);
                painter.drawText(QRect(0,0,1920,1080),Qt::AlignCenter,"Game Over");
                changeSize(painter,62,true);
                painter.drawText(QRect(0,250,1920,830),Qt::AlignCenter,spruch[chosenSpruch]);
            }
            break;
        }

        if(active == STATE_MAINMENU) {
            for(uint i = 0; i < projectiles.size(); i++) {
                if(!projectiles[i]->del) {
                    if(projectiles[i]->type == 7) {
                        QRectF r = projectiles[i]->rect;
                        int width = r.width()*2;
                        painter.drawPixmap(r.x()-width/2,r.y()-width/2,width,width/2,shekelMoney);
                    }
                }
            }

            if(moveAn > 0 && (moveAn < ANIMATION_BACK || moveAn == ANIMATION_CONTINUE) && fdir) {
                changeSize(painter,128,true);
                painter.setPen(Qt::white);
                painter.drawText(1100,800,"Lade...");
            }
        }

        if(backMenu) {
            painter.setOpacity(1);
            drawBackMenu(painter);
        }
        if(blendRectOpacity) {
            painter.setBrush(Engine::getColor(-1));
            painter.setOpacity(blendRectOpacity);
            if(viewRect.width()>1920) {
                painter.drawRect(-100,-100,viewRect.width()+200,viewRect.height()+200);
            } else {
                painter.drawRect(-100,-100,2100,1200);
            }
            painter.setOpacity(1);
        }
        changeSize(painter,32,true);
        painter.setPen(Qt::magenta);
        if(DEBUG) {
            tdrawMS = (timerD.nsecsElapsed()-debugns)/NANO_TO_MILLI;
            frameTimes.push_back(tdrawMS);
            painter.drawText(QRect(1350,40,350,500),"TMAIN:"+QString::number(tmainMS,'f')+"ms\n"+
                                    "TWAVES:"+QString::number(twavespeedMS,'f')+"ms\n"+
                                    "TPROJ:"+QString::number(tprojectileMS,'f')+"ms\n"+
                                    "TPROJF:"+QString::number(tprojectileFMS,'f')+"ms\n");
            changeSize(painter,28,true);
            if(frameTimes.size()>60) frameTimes.erase(frameTimes.begin());
            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::black);
            painter.setOpacity(0.4);
            painter.drawRect(1350,225,300,100);
            painter.setOpacity(1);
            painter.setPen(Qt::magenta);
            painter.drawText(1350,250,"FRAMETIME "+QString::number(tdrawMS,'f',1)+"ms");
            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::red);
            for(uint i=0;i<frameTimes.size();i++) {
                double ms = frameTimes[i];
                if(ms>50) ms = 50;
                double x = 1350+(5*i);
                double y = (ms/50)*100;
                painter.drawRect(x,325-y,5.0,2.5);
            }
            if(output) {
                qDebug()<<"paint-end";
            }
        }

    } catch(std::exception& e) {
        qDebug()<<e.what();
    }
    mutex.unlock();
}

void FrmMain::wheelEvent(QWheelEvent *e)
{
    if(active == STATE_MAINMENU || (gamePaused&&backMenu) || isGameOver) return;
    int angle = e->angleDelta().y();
    if(!isZooming) {
        maxZoomTime = 20;
        zoomTime = maxZoomTime;
    } else {
        zoomTime += maxZoomTime / 2;

    }
    if(angle>0 && isZooming != 1) {//nach oben
        isZooming = 1;
        zoomAmount = 0.02;
    } else if(angle<0 && isZooming != 2) { //nach unten
        isZooming = 2;
        zoomAmount = 0.02;
    }
}

void FrmMain::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
    closing = true;
    do {
        QMetaObject::invokeMethod(t_main,"stop");
        QMetaObject::invokeMethod(t_projectile,"stop");
        QMetaObject::invokeMethod(t_projectile_full,"stop");
        QMetaObject::invokeMethod(t_draw,"stop");
        QMetaObject::invokeMethod(t_wave,"stop");
        QMetaObject::invokeMethod(t_waveSpeed,"stop");
        QMetaObject::invokeMethod(t_physics,"stop");
        QMetaObject::invokeMethod(t_menuAn,"stop");
        QMetaObject::invokeMethod(t_animation,"stop");
        QMetaObject::invokeMethod(t_idle,"stop");
        QMetaObject::invokeMethod(t_shake,"stop");
        QMetaObject::invokeMethod(t_grasAn,"stop");
        QMetaObject::invokeMethod(t_towerTarget,"stop");
    } while(checkTimers(1));
    workerThread->quit();
    physicsThread->quit();
    projectileThread->quit();
}
