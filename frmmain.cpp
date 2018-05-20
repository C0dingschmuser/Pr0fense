#include "frmmain.h"
#include "ui_frmmain.h"

FrmMain::FrmMain(QOpenGLWidget *parent) :
    QOpenGLWidget(parent),
    ui(new Ui::FrmMain)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_AcceptTouchEvents,true);
    music = new QMediaPlayer();
    playlist_menu = new QMediaPlaylist();
    playlist_game = new QMediaPlaylist();
    playlist_menu->setPlaybackMode(QMediaPlaylist::Loop);
    playlist_game->setPlaybackMode(QMediaPlaylist::Loop);
    account.ip = serverIP;
    account.accountState = ACCOUNT_INACTIVE;
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
    shop = new Shop();
    connect(shop,SIGNAL(buyMap(int)),this,SLOT(on_mapBuy(int)));
    connect(shop,SIGNAL(error_buy(int)),this,SLOT(on_buyError(int)));
    connect(shop,SIGNAL(buyShekel(QString)),this,SLOT(on_purchaseShekel(QString)));
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
    workerThread = new QThread();
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
    for(uint i=0;i<2;i++) {
        QPixmap p(":/data/images/path/path"+QString::number(i)+".png");
        pathTextures.push_back(p);
    }
    for(uint i = 0; i < 4; i++) {
        QPixmap p(":/data/images/towers/minigun/minigun" + QString::number(i+1));
        minigunTowerPx.push_back(p);
    }
    loadUserData();
    loadLitteSprueche();
    initPhysics();
    shop->lvlPreviews = lvlPreviews;
    shop->lvlprices = lvlPrices;
    shop->mainlvls = mainLevels;
    shop->lockPx = lockPx;
    shop->f = f;
    music->setPlaylist(playlist_menu);
    //TODO
    //Waves allg
#ifdef QT_NO_DEBUG
    if(!accepted) {
        QMessageBox msgBox;
        msgBox.setText(
                     "<b>BITTE LESEN</b><br>"
                     "<b>Pr0fense befindet sich in der Testphase</b><br>"
                     "- Bugs/Feedback bitte über Discord oder @05 melden<br>"
                     "<b>Pr0fense ist kostenlos spielbar aber durch In-App käufe kann dein<br>"
                     "Fortschritt beschleunigt werden.<br>"
                     "Tippe auf 'Okay', um unsere Datenschutzrichtlinie und die<br>"
                     "Nutzungsbedingungen zu akzeptieren.</b>");
        msgBox.addButton(tr("Okay"), QMessageBox::YesRole);
        msgBox.addButton(tr("Datenschutzrichtlinie"), QMessageBox::NoRole);
        msgBox.addButton(tr("Nutzungsbedingungen"), QMessageBox::NoRole);
        int reply = msgBox.exec();
        while(reply) { //Datenschutz / Nutzungsbedingungen
            QString linkS;
            if(reply == 1) {
                linkS = "https://pastebin.com/CJvvFEQr";
            } else if(reply == 2){
                linkS = "https://pastebin.com/BQcicxpA";
            }
            QUrl link(linkS);
            QDesktopServices::openUrl(link);
            reply = msgBox.exec();
        }
        accepted = true;
    }
#endif
    QFuture<void> future = QtConcurrent::run(this, &FrmMain::getStatus);
    if(account.username != "") {
        account.accountState = ACCOUNT_PENDING;
        QFuture<void> future2 = QtConcurrent::run(&account, &Account::checkAccount);
    }
    //getStatus();
    active = STATE_MAINMENU;
    t_draw->start(16); //NUR zeichnen & input
    t_main->start(10); //turmschüsse & so
    t_wave->start(100); //gegnerspawn
    t_waveSpeed->start(5); //gegnerbewegung
    t_projectile->start(10); //projektile von türmen
    t_projectile_full->start(50); //update von ziel
    t_menuAn->start(100); //blussis im hauptmenü
    t_physics->start(5); //box2d physik
    t_physics->moveToThread(workerThread);
    t_animation->moveToThread(workerThread);
    t_main->moveToThread(workerThread);
    t_shake->moveToThread(workerThread);
    t_wave->moveToThread(workerThread);
    t_waveSpeed->moveToThread(workerThread);
    t_projectile->moveToThread(workerThread);
    t_projectile_full->moveToThread(workerThread);
    t_menuAn->moveToThread(workerThread);
    QThread::currentThread()->setPriority(QThread::HighPriority);
    workerThread->start();
    workerThread->setPriority(QThread::NormalPriority);
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

void FrmMain::initPhysics()
{
    b2Vec2 gravity(0,0);
    world = new b2World(gravity);
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

void FrmMain::resetTimers()
{
    QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,100));
    QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,10));
    QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,10));
    QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100));
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

void FrmMain::createBounds()
{
    b2BodyDef def;
    def.type = b2_staticBody;
    def.position.Set(mapwidth/2,-2);
    def.angle = 0;
    wall1 = world->CreateBody(&def);
    b2PolygonShape boxShape;
    boxShape.SetAsBox(mapwidth/2,1);
    b2FixtureDef fixDef;
    fixDef.shape = &boxShape;
    fixDef.filter.categoryBits = BOUNDARY;
    fixDef.filter.maskBits = NORMAL_ENEMY | FLYING_ENEMY;
    wall1->CreateFixture(&fixDef);

    //2

    def.position.Set(-2,mapheight/2);
    wall2 = world->CreateBody(&def);
    boxShape.SetAsBox(1,mapheight/2);
    fixDef.shape = &boxShape;
    wall2->CreateFixture(&fixDef);

    //3

    def.position.Set(mapwidth/2,mapheight);
    wall3 = world->CreateBody(&def);
    boxShape.SetAsBox(mapwidth/2,1);
    fixDef.shape = &boxShape;
    wall3->CreateFixture(&fixDef);

    //4

    def.position.Set(mapwidth,mapheight/2);
    wall4 = world->CreateBody(&def);
    boxShape.SetAsBox(1,mapheight/2);
    fixDef.shape = &boxShape;
    wall4->CreateFixture(&fixDef);
}

void FrmMain::on_tdraw()
{
    update();
}

void FrmMain::on_tmain()
{
    if(exit) this->close();
    if(newPost && active == STATE_MAINMENU) {
        newPost = false;
        newPostInfo();
    }
    if(active == STATE_MAINMENU || active == STATE_EDITOR) return;

    if(towerMenuAnimating >= 0) {
        double anSpeed = 50;
        if(!towerMenuAnimatingDir) { //öffnen
            towerMenuRectAnimation.moveTo(
                        towerMenuRectAnimation.x() + anSpeed,
                        towerMenuRectAnimation.y());
            if(towerMenuRectAnimation.x() >= towerMenuRect.x()) {
                towerMenu = towerMenuAnimating;
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
                        case TOWER_SNIPER: //sniper
                            rect = QRectF(tCenter.x()-5,tCenter.y()-5,10,10);
                            color = QColor(47,237,27,100);
                            break;
                        case TOWER_FLAK: //flak
                            switch(towers[i]->side) {
                            case 0:
                                rect = QRectF(tCenter.x()-25,tCenter.y()-5,10,10);
                                towers[i]->side++;
                                break;
                            default:
                                rect = QRectF(tCenter.x()+15,tCenter.y()-5,10,10);
                                towers[i]->side = 0;
                                break;
                            }
                            color = QColor(247,210,100);
                            break;
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
                    } else if(towers[i]->type == TOWER_BENIS &&
                              Engine::isEqual(ownAngle, targetAngle, 2)) {//benistower
                        this->benis += towers[i]->dmg;
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
                            projectiles.push_back(p);
                            //--
                            towers[i]->shotsFiredSinceReload++;
                            if(towers[i]->shotsFiredSinceReload<10) {
                                towers[i]->currentReload = 50;
                            } else {
                                towers[i]->shotsFiredSinceReload = 0;
                                towers[i]->currentReload = towers[i]->reload;
                                towers[i]->target = nullptr;
                            }
                        }
                    } else if(towers[i]->type == TOWER_LASER &&
                              Engine::isEqual(ownAngle, targetAngle, 2)) { //Lasertower
                        if(!towers[i]->isShooting) {
                            towers[i]->isShooting = true;
                        }
                        if(towers[i]->shotsFiredSinceReload) {
                            double dmg = towers[i]->getDmg(towers[i]->target->type);
                            towers[i]->target->preHealth -= dmg / 10.0;
                            if(towers[i]->target->preHealth < 0 ) towers[i]->target->preHealth = 0;
                            towers[i]->target->reduceHealth(dmg / 10.0);
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
                        projectiles.push_back(p);
                        towers[i]->shotsFiredSinceReload++;
                        if(towers[i]->shotsFiredSinceReload > 1 &&
                                towers[i]->shotsFiredSinceReload < 3) {
                            towers[i]->currentReload = towers[i]->reload * 0.6;
                        } else if(towers[i]->shotsFiredSinceReload > 2 &&
                                  towers[i]->shotsFiredSinceReload < 5) {
                            towers[i]->currentReload = towers[i]->reload * 0.4;
                        } else {
                            towers[i]->currentReload = towers[i]->reload * 0.1;
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
                if(towers[i]->currentReload>0) towers[i]->currentReload -= 10;
            }
        }
        for(uint i=0;i<enemys.size();i++) {
            if(enemys[i]->type == ENEMY_GEBANNT) {//wenn fliegend dann rotor drehen
                enemys[i]->animation += 5;
                if(enemys[i]->animation>360) enemys[i]->animation = 0;
            }
            if(enemys[i]->health <= 0.01) {//wenn tot dann löschen
                benis += enemys[i]->price;
                QPoint point = enemys[i]->rect().center();
                int angle = -90;
                Projectile *p = pool->getProjectile();
                p->init3(QString::number(enemys[i]->price),
                         QPoint(point.x()-25,point.y()),
                         angle,
                         qCos(qDegreesToRadians((double)angle)),
                         qSin(qDegreesToRadians((double)angle)),
                         0.01,
                         0.5);
                p->hasShekelImage = true;
                projectiles.push_back(p);
                delEnemy(i);
            }
        }
        //mutex.unlock();
        if(DEBUG) {
            tmainMS = timerM.nsecsElapsed()/NANO_TO_MILLI;
        }
    } catch(std::exception e) {

    }
}

void FrmMain::on_tprojectile()
{
    if(active == STATE_EDITOR || (gamePaused && backMenu) || (gamePaused && loaded) || moveAn == ANIMATION_BACK) return;
    if(DEBUG) {
        timerP.restart();
    }
    try {
        //mutex.lock();
        for(uint i=0;i<projectiles.size();i++) {
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
                            tmpTarget->reduceHealth(projectiles[i]->dmg);
                            if(tmpTarget->soonBanned) {
                                tmpTarget->soonBanned++;
                                if(tmpTarget->soonBanned >= 11) tmpTarget->health = 0;
                            }
                            if(projectiles[i]->stun) {
                                tmpTarget->setStun(projectiles[i]->stun);
                            }
                            projectiles[i]->del = 1;
                            break;
                        }
                    }
                } else if(projectiles[i]->type < 5){ //text + blus + giftgas + fade proj
                    if(projectiles[i]->type == 3) { //Giftprojektil
                        for(uint a = 0;a < enemys.size(); a++) {
                            if(projectiles[i]->rect.intersects(enemys[a]->rectF())) {
                                if(projectiles[i]->opacity >= 0.7 && projectiles[i]->targetDefinitions.size()) {
                                    double dmg = projectiles[i]->poisonDmg * (projectiles[i]->getTargetEfficiency(enemys[a]->type) / 100.0);
                                    if(dmg > 0) {
                                        enemys[a]->reduceHealth(enemys[a]->maxHealth * (0.00075 * dmg));
                                        enemys[a]->preHealth -= enemys[a]->maxHealth * (0.00075 * dmg);
                                        if(enemys[a]->preHealth < 0) enemys[a]->preHealth = 0;
                                    }
                                }
                            }
                        }
                    }
                    if(projectiles[i]->opacity <= 0 && projectiles[i]->opacityDecAm) {
                        projectiles[i]->del = 3;
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
            if(projectiles[i]->del) {
                projectiles[i]->free();
                projectiles.erase(projectiles.begin()+i);
            }
        }
        //mutex.unlock();
    } catch(std::exception e) {

    }
    if(DEBUG) {
        tprojectileMS = timerP.nsecsElapsed()/NANO_TO_MILLI;
    }
}

void FrmMain::on_tprojectilefull()
{
    if(active == STATE_EDITOR || gamePaused) return;
    if(DEBUG) {
        timerPF.restart();
    }
    try {
        for(uint i=0;i<projectiles.size();i++) {
            if(!projectiles[i]->del) {
                //mutex.lock();
                projectiles[i]->update(true);
                //mutex.unlock();
            }
        }
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
    } catch(std::exception e) {

    }
    if(DEBUG) {
        tprojectileFMS = timerPF.nsecsElapsed()/NANO_TO_MILLI;
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
        QRect knot = tiles[paths[chosenpath].path[0]]->rect();
        int width = 0;
        double health = 0;
        int price = 0;
        int type = 0;
        double speed = 0;
        double diff = 1;
        uint16_t ownCategoryBits = NORMAL_ENEMY, collidingCategoryBits = NORMAL_ENEMY | BOUNDARY;
        if(currentWave.waveCount > 20) {
            diff = 1.05 * (currentWave.waveCount / 20);
        }
        bool flak = false;
        if(!shop->towerLocks[6] && !shop->towerPrices[6]) flak = true;

        type = currentWave.generateEnemyType();
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
            break;
        case ENEMY_GEBANNT:
            health = 25*diff;
            speed = 1.75;
            price = 5;
            ownCategoryBits = FLYING_ENEMY;
            collidingCategoryBits = FLYING_ENEMY | BOUNDARY;
            break;
        }
        width = getEnemySizeByType(type);
        QRectF r;
        std::vector <int> enemysAtSpawn;
        for(uint i=0;i<enemys.size();i++) {
            if(enemys[i]->cpos<2) enemysAtSpawn.push_back(i);
        }
        double size = 6400;
        for(uint i=0;i<enemysAtSpawn.size();i++) {
            double es = getEnemySizeByType(enemys[enemysAtSpawn[i]]->type)/2;
            size -= M_PI*(es*es);
        }
        if(size<M_PI*((width/2)*(width/2))) return;
        if(width>=40) {
            r = QRectF(knot.x()+((80-width)/2)+width/2,
                 knot.y()+((80-width)/2)+width/2,
                 width,width);
        } else {
            double x = (double)Engine::random(knot.x(),(knot.x()+knot.width())-width);
            double y = (double)Engine::random(knot.y(),(knot.y()+knot.height())-width);
            r = QRectF(x,y,width,width);
        }
        if(playingSpeed && !enemys.size() && !currentWave.enemysPerWave) {
            if(playingSpeed == 2) {
                QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,50));
                QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,10));
                QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,10));
                QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100));
            }
            playingSpeed = 0;
            gamePaused = true;
            shop->shekel += currentWave.calcShekel();
            currentWave.waveCount++;
            currentWave.updateEnemysPerWave(false, mapwidth/80);
            currentWave.resetWave();
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
        } else {
            return;
        }
        currentWave.addEnemy(type);
        Enemy *e = pool->getEnemy();//new Enemy(knot,0);
        e->init(r,speed,type,0,health,chosenpath,price);
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
    } catch(std::exception e) {

    }
}

void FrmMain::on_twaveSpeed()
{
    if(gamePaused) return;
    try {
        if((active == STATE_PLAYING || active == STATE_EDITOR) && mapLoaded) {
            if(DEBUG) {
                timerWS.restart();
            }
            for(uint i=0;i<enemys.size();i++) {
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
                        if(enemys[i]->stunned || enemys[i]->blocked) {
                            if(enemys[i]->speed >= enemys[i]->ospeed*0.5) {
                                enemys[i]->speed -= enemys[i]->ospeed*0.0075;
                            } else if(enemys[i]->blocked) {
                                enemys[i]->blocked = false;
                                enemys[i]->dir = enemys[i]->newdir;
                            }
                        } else {
                            if(enemys[i]->speed < enemys[i]->ospeed) {
                                enemys[i]->speed += enemys[i]->ospeed*0.0075;
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
                        if(!enemys[i]->rect().intersects(tiles[paths[enemys[i]->path].path[next-1]]->rect())) {
                            enemys[i]->cpos++;
                        }

                    } else {
                        //gegner löschen weil base erreicht
    #ifdef QT_NO_DEBUG //camera shake bei release
                        shaking = 75;
                        if(!t_shake->isActive()) QMetaObject::invokeMethod(t_shake,"start",Q_ARG(int,5));
    #endif
                        delEnemy(i);
                        basehp--;
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
    } catch(std::exception e) {

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
                    loadMap("map"+QString::number(subLevelSelected-4));
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
                //active = 2;
                break;
            case ANIMATION_BACK: //zurück
                if(active != STATE_EDITOR && !error) {
                    changePlaylist(0);
                    saveGameSave();
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
    if(shakeX==shakeIntensity) {
        shakeX = -shakeIntensity;
    } else {
        shakeX = shakeIntensity;
    }
    if(shakeY==shakeIntensity) {
        shakeY = -shakeIntensity;
    } else {
        shakeY = shakeIntensity;
    }
    shaking--;
    if(!shaking) {
        shakeX = 0;
        shakeY = 0;
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
    if(!gamePaused && active == STATE_PLAYING) {
        //qDebug()<<world->GetBodyCount();
        //mutex.lock();
        if(playingSpeed==1) {
            world->Step(0.05,2,4);
        } else if(playingSpeed==2){
            world->Step(0.1,2,4);
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
}

void FrmMain::on_mediaStateChanged(QMediaPlayer::MediaStatus status)
{
    if(status == QMediaPlayer::MediaStatus::LoadedMedia ||
            status == QMediaPlayer::MediaStatus::BufferedMedia) {
        if(music->state()!=QMediaPlayer::PlayingState) {
            music->setVolume(25);
            if(soundEnabled) {
                music->play();
            }
        }
    }
}

void FrmMain::on_appStateChanged(Qt::ApplicationState state)
{
    if(active == STATE_SUSPENDED) return;
    if((state==Qt::ApplicationInactive||
            state==Qt::ApplicationSuspended||
            state==Qt::ApplicationHidden)) {
        suspended = true;
        QMetaObject::invokeMethod(t_draw,"stop");
        if(soundEnabled) QMetaObject::invokeMethod(music,"pause");
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
        if(account.username != "") {
            account.accountState = ACCOUNT_PENDING;
            QFuture<void> future2 = QtConcurrent::run(&account, &Account::checkAccount);
        }
        suspended = false;
        if(soundEnabled) QMetaObject::invokeMethod(music,"play");
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
    }
}

void FrmMain::on_mapBuy(int subSelected)
{
    lvlPrices[subSelected] = 0;
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

void FrmMain::buyMinusTower()
{
    benis -= minusTowerCost;
    Tower *t = new Tower();
    t->type = TOWER_MINUS;
    t->dmg = 10;
    t->stun = 0;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 1000;
    t->currentReload = t->reload;
    t->range = 150;
    t->pspeed = 2.5;
    t->name = "Minustower";
    t->tnum = towerMenu;
    t->angleSpeed = 1.25;
    t->price = minusTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[towerMenu]->t = t;
    towerMenuSelected = 0;
}

void FrmMain::buyFavTower()
{
    benis -= herzTowerCost;
    Tower *t = new Tower();
    t->type = TOWER_HERZ;
    t->dmg = 0;
    t->stun = 2000;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 3000;
    t->currentReload = t->reload;
    t->range = 150;
    t->pspeed = 2.5;
    t->projectilePxID = 1; //Herz statt -
    t->name = "Favoritentower";
    t->tnum = towerMenu;
    t->angleSpeed = 1;
    t->price = herzTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[towerMenu]->t = t;
    towerMenuSelected = 0;
}

void FrmMain::buyRepostTower()
{
    benis -= repostTowerCost;
    Tower *t = new Tower();
    t->type = TOWER_REPOST;
    t->dmg = 0;
    t->stun = 0;
    t->repost = 3000;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 5000;
    t->currentReload = t->reload;
    t->range = 125;
    t->hasProjectileSpeed = false;
    t->hasTurnSpeed = false;
    t->pspeed = 2.5;
    t->projectilePxID = 1;
    t->name = "Reposttower";
    t->tnum = towerMenu;
    t->price = repostTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[towerMenu]->t = t;
    towerMenuSelected = 0;
}

void FrmMain::buyBenisTower()
{
    benis -= benisTowerCost;
    Tower *t = new Tower();
    t->type = TOWER_BENIS;
    t->dmg = 10;
    t->stun = 0;
    t->repost = 0;
    t->hasProjectileSpeed = false;
    t->hasTurnSpeed = false;
    t->hasRange = false;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 10000;
    t->currentReload = t->reload;
    t->range = 0;
    t->name = "Benistower";
    t->tnum = towerMenu;
    t->price = benisTowerCost;
    towers.push_back(t);
    tiles[towerMenu]->t = t;
    towerMenuSelected = 0;
}

void FrmMain::buyBanTower()
{
    benis -= benisTowerCost;
    Tower *t = new Tower();
    t->type = TOWER_BAN;
    t->dmg = 0;
    t->stun = 0;
    t->repost = 0;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 20000;
    t->currentReload = t->reload;
    //t->reloadMax = 30000;
    t->hasDamage = false;
    t->hasRange = false;
    t->hasTurnSpeed = false;
    t->range = 0;
    t->name = "Bantower";
    t->tnum = towerMenu;
    t->projectilePxID = 2;
    t->pspeed = 2.5;
    t->price = banTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[towerMenu]->t = t;
    towerMenuSelected = 0;
}

void FrmMain::buySniperTower()
{
    benis -= sniperTowerCost;
    Tower *t = new Tower();
    t->type = TOWER_SNIPER;
    t->dmg = 40;
    t->stun = 0;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 4000;
    t->currentReload = t->reload;
    t->range = 225;
    t->pspeed = 3.5;
    t->name = "Snipertower";
    t->tnum = towerMenu;
    t->angleSpeed = 0.9;
    t->price = sniperTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[towerMenu]->t = t;
    towerMenuSelected = 0;
}

void FrmMain::buyFlakTower()
{
    benis -= flakTowerCost;
    Tower *t = new Tower();
    t->type = TOWER_FLAK;
    t->dmg = 10;
    t->stun = 0;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 400;
    t->currentReload = t->reload;
    t->range = 150;
    t->pspeed = 2.5;
    t->name = "Flaktower";
    t->tnum = towerMenu;
    t->angleSpeed = 2;
    t->price = flakTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[towerMenu]->t = t;
    towerMenuSelected = 0;
}

void FrmMain::buyLaserTower()
{
    benis -= laserTowerCost;
    Tower *t = new Tower();
    t->type = TOWER_LASER;
    t->dmg = 20;
    t->stun = 0;
    t->pos = tiles[towerMenu]->pos;
    t->hasProjectileSpeed = false;
    t->reload = 3500;
    t->currentReload = t->reload;
    t->range = 175;
    t->pspeed = 2;
    t->name = "Lasertower";
    t->tnum = towerMenu;
    t->angleSpeed = 1.5;
    t->price = laserTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[towerMenu]->t = t;
    towerMenuSelected = 0;
}

void FrmMain::buyPoisonTower()
{
    benis -= poisonTowerCost;
    Tower *t = new Tower();
    t->type = TOWER_POISON;
    t->dmg = 1;
    t->stun = 0;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 4000;
    t->currentReload = t->reload;
    t->hasProjectileSpeed = false;
    t->range = 125;
    t->pspeed = 0;
    t->name = "Gifttower";
    t->tnum = towerMenu;
    t->angleSpeed = 1.5;
    t->price = poisonTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[towerMenu]->t = t;
    towerMenuSelected = 0;
}

void FrmMain::buyMinigunTower()
{
    benis -= minigunTowerCost;
    Tower *t = new Tower();
    t->type = TOWER_MINIGUN;
    t->dmg = 0.5;
    t->stun = 0;
    t->animationMax = 3;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 500;
    t->currentReload = t->reload;
    t->pspeed = 2.25;
    t->range = 150;
    t->name = "Miniguntower";
    t->tnum = towerMenu;
    t->angleSpeed = 1.5;
    t->price = minigunTowerCost;
    addTowerTargets(t);
    towers.push_back(t);
    tiles[towerMenu]->t = t;
    towerMenuSelected = 0;
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
#ifdef QT_DEBUG
    benis = 10000;
#else
    benis = 100;
#endif
    upgradeCost = 0;
    towerMenu = -1;
    if(mapwidth) {
        currentWave.resetWave(true);
#ifdef QT_DEBUG
        currentWave.waveCount = 100;
#endif
        currentWave.updateEnemysPerWave(true,mapwidth/80);
    }
    internalWaveCount = 0;
    mName = "";
    loaded = false;
    playingSpeed = 0; //noch auf 0 ändern
    isGameOver = false;
    error = false;
    basehp = 75;
    ownBaseCount = 0;
    enemyBaseCount = 0;
    gamePaused = false;
    for(uint i=0;i<tiles.size();i++) {
        delete tiles[i];
    }
    tiles.resize(0);
    delAllEnemys(1);
    for(uint i=0;i<towers.size();i++) {
        delete towers[i];
    }
    towers.resize(0);
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

void FrmMain::error_save(QFile &file)
{
    file.close();
    file.remove();
    fdir = 0;
    changePlaylist(0);
    error = true;
    hasPlayingSave = false;
    moveAn = ANIMATION_BACK;
    active = STATE_MAINMENU;
    QMessageBox::warning(this,"Fehler","Fehler beim laden!");
    return;
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
        size = 40;
        break;
    case ENEMY_MOD:
        size = 40;
        break;
    case ENEMY_ADMIN:
        size = 50;
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
    QTcpSocket *server = new QTcpSocket();
    server->connectToHost(serverIP,38910);
    server->waitForConnected(1500);
    if(server->state()==QTcpSocket::ConnectedState) {
        QString data = ".0#~";
        server->write(data.toUtf8());
        server->waitForBytesWritten(1500);
        server->waitForReadyRead(1500);
        QString response = server->readAll();
        if(response.at(0)=="."&&response.contains("~")) { //Antwort OK
            response.remove(0,1);
            QStringList split = response.split("#");
            if(version.toDouble()<split[1].toDouble()) { //Outdated
                outdated = true;
            }
            if(newestPost!=split[0]) { //Neuer Post
                newestPost = split[0];
                newPost = true;
            }
        }
    } else qDebug()<<"Connection failed. [getStatus]";
    server->close();
    server->deleteLater();
}

void FrmMain::newPostInfo()
{
    QUrl link("http://www.pr0gramm.com/new/"+newestPost);
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
    if(paket == "shekel_05") {
        QMessageBox::information(this,"Info", "Falls du wirklich dieses Paket kaufen möchtest, wende dich bitte an "
                                              "@FireDiver oder per E-Mail an firediver98@gmail.com");
        return;
    }
    QInAppProduct *product = store->registeredProduct(paket);
    if(!product) return;
    product->purchase();
    return;
#else
    QMessageBox::information(this, "Info", "Für Käufe in der Windows-Version wende dich bitte an @FireDiver"
                                           " oder per E-Mail an firediver98@gmail.com");
#endif
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
        if(identifier == QStringLiteral("shekel_supersmall")) {
            shop->shekel += shop->shekelPacks[0];
        } else if(identifier == QStringLiteral("shekel_small")) {
            shop->shekel += shop->shekelPacks[1];
        } else if(identifier == QStringLiteral("shekel_normal")) {
            shop->shekel += shop->shekelPacks[2];
        } else if(identifier == QStringLiteral("shekel_big")) {
            shop->shekel += shop->shekelPacks[3];
        } else if(identifier == QStringLiteral("shekel_large")) {
            shop->shekel += shop->shekelPacks[4];
        } else if(identifier == QStringLiteral("shekel_shekel")) {
            shop->shekel += shop->shekelPacks[5];
        } else if(identifier == QStringLiteral("shekel_ultra")) {
            shop->shekel += shop->shekelPacks[6];
        }
        transaction->finalize();
        QFuture<void> future2 = QtConcurrent::run(&account, &Account::registerPurchase, transaction->orderId(), identifier);
    } else if(transaction->status() == QInAppTransaction::PurchaseFailed) {
        if(transaction->failureReason()==QInAppTransaction::ErrorOccurred) {
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
    painter.setBrush(grau);
    painter.drawRect(0,0,mapwidth,mapheight);
    for(uint i=0;i<tiles.size();i++) {
        //if(tiles[i]->pos.intersects(viewRect)) {
            QPixmap p;
            switch(tiles[i]->type) {
            case 1: //weg
                p = pathTextures[Engine::random(0,2)];
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
            if((tiles[i]->type!=6&&!custom)||(custom&&tiles[i]->type!=6)) {
                painter.drawPixmap(tiles[i]->rect(),p);
            }
        //}
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
    painter.setBrush(grau);
    painter.drawRect(0,0,width,height);
    for(int i=0;i<split.size();i++) {
        int type = split[i].toInt();
        QPixmap p;
        switch(type) {
        case 1: //weg
            p = pathTextures[Engine::random(0,2)];
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
        if(type!=6) {
            painter.drawPixmap(rects[i],p);
        }
    }
    return map;
}

void FrmMain::drawIngame(QPainter &painter)
{
    painter.setPen(Qt::NoPen);
    if(mapLoaded) painter.drawPixmap(0,0,mapwidth,mapheight,mapPx);
    if(active == STATE_EDITOR) {
        drawEditor(painter);
    }
    for(uint i=0;i<enemys.size();i++) {
        painter.setOpacity(enemys[i]->opacity);
        painter.setBrush(Qt::red);
        switch(enemys[i]->type) {
        case ENEMY_FLIESE: //fliesentischbesitzer
            painter.setBrush(fliese);
            break;
        case ENEMY_NEUSCHWUCHTEL: //neuschwuchtel
            painter.setBrush(neuschwuchtel);
            break;
        case ENEMY_SCHWUCHTEL: //schwuchtel
            painter.setBrush(schwuchtel);
            break;
        case ENEMY_SPENDER: //edlerSpender
            painter.setBrush(edlerSpender);
            break;
        case ENEMY_ALTSCHWUCHTEL: //altschwuchtel
            painter.setBrush(altschwuchtel);
            break;
        case ENEMY_MOD: //mods
            painter.setBrush(mod);
            break;
        case ENEMY_ADMIN: //admins
            painter.setBrush(admin);
            break;
        case ENEMY_LEGENDE: //lebende legende
            painter.setBrush(legende);
            break;
        case ENEMY_GEBANNT: //gebannte
            painter.setBrush(gebannt);
            break;
        }
        if(enemys[i]->type != ENEMY_LEGENDE) {
            painter.drawEllipse(enemys[i]->rect());
        } else {
            painter.drawPolygon(Engine::getRauteFromRect(enemys[i]->rectF()));
        }
        if(enemys[i]->type == ENEMY_GEBANNT) { //rotor
            QRectF t = enemys[i]->rectF();
            painter.save();
            painter.translate(t.center());
            painter.rotate(enemys[i]->animation);
            painter.translate(-t.center().x(),-t.center().y());
            painter.drawPixmap(enemys[i]->rectF(),rotorPx,QRectF(0,0,23,23));
            painter.restore();
        }
        painter.setOpacity(fade);
        if(enemys[i]->repost) {
            painter.setOpacity(((double)enemys[i]->repost/enemys[i]->maxRepost)*fade);
            painter.drawPixmap(enemys[i]->rect(),repostMark);
            painter.setOpacity(fade);
        }
        if(enemys[i]->stunned&&!enemys[i]->soonBanned) {
            painter.setOpacity(((double)enemys[i]->stunned/enemys[i]->maxStun)*fade);
            painter.drawPixmap(enemys[i]->rect(),herzPx);
            painter.setOpacity(fade);
        } else if(enemys[i]->stunned&&enemys[i]->soonBanned) {
            painter.setOpacity((1-((double)enemys[i]->health/enemys[i]->maxHealth))*fade);
            painter.setBrush(gebannt);
            painter.drawEllipse(enemys[i]->rect());
            painter.setOpacity(fade);
        }
    }
    for(uint i=0;i<towers.size();i++) {
        if(towers[i]->pos.intersects(viewRect)) {
            drawTower(towers[i]->rect(),painter,towers[i]);
        }
    }
    for(uint i=0;i<towers.size();i++) {
        if(towers[i]->pos.intersects(viewRect)) {
            if(towers[i]->type == TOWER_LASER && towers[i]->isShooting && towers[i]->target != nullptr) {
                //Zeichnet Laser vom Laserturm
                painter.setPen(QPen(QBrush(QColor(51,127,210)),5));
                QPointF p(towers[i]->pos.center().x(),towers[i]->pos.center().y()-36);
                QTransform transform = QTransform().translate(p.x(),p.y()+36).rotate(towers[i]->angle).translate(-p.x(),-(p.y()+36));
                p = transform.map(p);
                painter.drawLine(p,towers[i]->target->rectF().center());
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(51,127,210));
                painter.drawEllipse(towers[i]->target->rectF().center(),5,5);
            }
        }
    }
    for(uint i=0;i<enemys.size();i++) {
        QRectF pos = enemys[i]->rectF();
        int health = enemys[i]->health;
        int maxHealth = enemys[i]->maxHealth;
        if(health<maxHealth) {
            if(health>maxHealth*0.8) {
                painter.setBrush(QColor(34,177,76));
            } else if(health>maxHealth*0.6) {
                painter.setBrush(QColor(181,230,29));
            } else if(health>maxHealth*0.4) {
                painter.setBrush(QColor(255,242,0));
            } else if(health>maxHealth*0.2) {
                painter.setBrush(QColor(223,89,0));
            } else if(health>0) {
                painter.setBrush(QColor(237,28,36));
            }
            int width = pos.width()*((double)enemys[i]->health/enemys[i]->maxHealth);
            if(width <= 0) width = 1;
            painter.drawRect(pos.x(),pos.y()+pos.height(),width,10);
        }
    }
    if(towerMenu > -1 || (towerMenuAnimating > -1 && !towerMenuAnimatingDir)) {
        int num = towerMenu;
        if(towerMenuAnimating > -1) {
            num = towerMenuAnimating;
        }
        painter.setOpacity(0.5*fade);
        painter.drawPixmap(tiles[num]->pos,auswahlTile,QRectF(0,0,80,80));
        painter.setOpacity(fade);
        if(tiles[num]->t != nullptr) {//wenn tower steht range anzeigen
            if(tiles[num]->t->hasRange) {
                int range = tiles[num]->t->range;
                painter.setBrush(Qt::NoBrush);
                QPen pen(Qt::red,2);
                painter.setPen(pen);
                painter.drawEllipse(tiles[num]->pos.center(),range,range);
                painter.setPen(Qt::NoPen);
            }
        }
    }
    for(uint i=0;i<projectiles.size();i++) {
        if(projectiles[i] != nullptr) { //nullptr abfrage
            switch(projectiles[i]->type) {
            case 0:
                painter.setOpacity(1);
            case 4:
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
                    painter.setBrush(gebannt);
                    painter.setPen(Qt::NoPen);
                    painter.drawEllipse(projectiles[i]->rect);
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
                painter.setPen(Qt::white);
                changeSize(painter,18,true);
                painter.drawText(projectiles[i]->rect.x(),projectiles[i]->rect.y(),
                                 projectiles[i]->text);
                if(projectiles[i]->hasShekelImage) {
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
            }
        }
    }
    if(DEBUG) {
        changeSize(painter,32,true);
        painter.setPen(Qt::blue);
        for(uint i=0;i<paths.size();i++) {
            Path tmpPath = paths[i];
            for(uint a=0;a<tmpPath.path.size();a++) {
                painter.drawText(tiles[tmpPath.path[a]]->rect(),QString::number(i));
            }
        }
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
    }
}

void FrmMain::drawHUD(QPainter &painter)
{
    if(active == STATE_PLAYING) {
        drawTowerMenu(painter);
        painter.drawPixmap(0,0,100,100,pr0coinPx);
        painter.drawPixmap(5,75,90,90,baseHerzPx);
        painter.drawPixmap(1800,0,100,100,wavesPx);
        painter.setPen(Qt::white);
        changeSize(painter,56,true);
        painter.drawText(85,70,QString::number(benis));
        painter.drawText(85,140,QString::number(basehp));
        painter.drawText(QRect(1600,20,200,60),Qt::AlignRight,QString::number(currentWave.waveCount));
        switch(playingSpeed) {
        case 0:
            painter.drawPixmap(btnPlayingSpeedRect,btnStopPx);
            painter.drawText(1570,1000,"Start");
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

    } else if(active == STATE_EDITOR) {
        drawEditorMenu(painter);
    }
}

void FrmMain::drawTowerMenu(QPainter &painter)
{
    if(towerMenu > -1 || towerMenuAnimating > -1) {
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
            QString o5 = QString::number(tmpTower->angleSpeed,'f',2);
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
                                                                   "Feuerrate:\n"
                                                                   "Projektilgeschwindigkeit:\n"
                                                                   "Drehgeschwindigkeit:");
            painter.drawText(QRect(offX+450,230,140,300),Qt::AlignRight,o2+"\n"+
                             o4+"\n"+
                             QString::number(tmpTower->reload/1000.0,'f',2)+"s"+o3+"\n"+
                             o6+"\n"+
                             o5);
            //
            painter.setPen(Qt::NoPen);
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

            if(towerMenuSelected != 5) {
                painter.setOpacity(0.6 * fade);
            } else painter.setOpacity(fade);
            tmp = btnSellRect;
            tmp.moveTo(offX + tmp.x(), tmp.y());
            painter.drawPixmap(tmp,btnSell);

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
                upgradeCost = (tmpTower->price * upgradeHighConst) * (tmpTower->dmglvl+1);
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
                upgradeCost = (tmpTower->price * upgradeLowConst) * (tmpTower->rangelvl+1);
                maxlvl = tmpTower->rangelvl;
                painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                changeSize(painter,38,false);
                painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                     "Erhöht die Reichweite");
                break;
            case 3: //feuerrate
                upgradeCost = (tmpTower->price * upgradeHighConst) * (tmpTower->ratelvl+1);
                maxlvl = tmpTower->ratelvl;
                painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                changeSize(painter,38,false);
                painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                     "Erhöht die Feuerrate");
                break;
            case 4: //speed
                upgradeCost = (tmpTower->price * upgradeLowConst) * (tmpTower->speedlvl+1);
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
                upgradeCost = (tmpTower->price * upgradeHighConst) * (tmpTower->turnlvl+1);
                maxlvl = tmpTower->turnlvl;
                painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"Info:");
                changeSize(painter,38,false);
                painter.drawText(QRect(offX + 25,785,550,200),Qt::AlignLeft,"\n"
                                                                     "Erhöht die Rotations-\n"
                                                                     "geschwindigkeit");
            }

            if(towerMenuSelected > 0 &&
                    towerMenuSelected < 7 &&
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

            painter.setPen(Qt::NoPen);
        } else { //Baumenü
            if(towerMenuAnimating > -1) {
                painter.translate(towerMenuRectAnimation.x(), 0);
            }

            drawTower(minusTowerRect,painter,nullptr,-1);
            drawTower(favTowerRect,painter,nullptr,-2);

            if(!shop->towerLocks[2]) {
                drawTower(repostTowerRect,painter,nullptr,-3);
                if(shop->towerPrices[2]) painter.drawPixmap(repostTowerRect, shop->lock_quadratPx);
            }

            if(!shop->towerLocks[3]) {
                drawTower(benisTowerRect,painter,nullptr,-4);
                if(shop->towerPrices[3]) painter.drawPixmap(benisTowerRect, shop->lock_quadratPx);
            }

            if(!shop->towerLocks[4]) {
                drawTower(banTowerRect,painter,nullptr,-5);
                if(shop->towerPrices[4]) painter.drawPixmap(banTowerRect, shop->lock_quadratPx);
            }

            if(!shop->towerLocks[5]) {
                drawTower(sniperTowerRect,painter,nullptr,-6);
                if(shop->towerPrices[5]) painter.drawPixmap(sniperTowerRect, shop->lock_quadratPx);
            }

            if(!shop->towerLocks[6]) {
                drawTower(flakTowerRect,painter,nullptr,-7);
                if(shop->towerPrices[6]) painter.drawPixmap(flakTowerRect, shop->lock_quadratPx);
            }

            if(!shop->towerLocks[7]) {
                drawTower(laserTowerRect,painter,nullptr,-8);
                if(shop->towerPrices[7]) painter.drawPixmap(laserTowerRect, shop->lock_quadratPx);
            }

            if(!shop->towerLocks[8]) {
                drawTower(poisonTowerRect,painter,nullptr,-9);
                if(shop->towerPrices[8]) painter.drawPixmap(poisonTowerRect, shop->lock_quadratPx);
            }

            if(!shop->towerLocks[9]) {
                drawTower(minigunTowerRect,painter,nullptr,-10);
                if(shop->towerPrices[9]) painter.drawPixmap(minigunTowerRect, shop->lock_quadratPx);
            }

            if(towerMenuAnimating > -1) {
                painter.translate(-towerMenuRectAnimation.x(), 0);
            }
        }
    }
}

void FrmMain::drawTower(QRect pos, QPainter &painter, Tower *tmpTower, int info)
{
    //pixelsize = 36
    double offX = 0;
    if(towerMenuAnimating > -1 && tmpTower == nullptr) {
        offX = towerMenuRectAnimation.x();
        //pos.moveTo(offX + pos.x(), pos.y());
    }
    offX = 0;

    bool menu=true;
    QRectF posF;
    painter.drawPixmap(pos,towerGroundPx);
    Tower *t = tmpTower;
    int tnum = info;
    if(t!=nullptr) {
        posF = t->pos;
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
    switch(tnum) {
    case TOWER_MINUS: //minustower
        if(info && towerMenuSelected==tnum) { //towerinfo
            if(benis>=minusTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(minusTowerCost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
        painter.drawPixmap(pos,towerBasePx);
        painter.save();
        if(t) {
            painter.translate(pos.center());
            painter.rotate(t->angle);
            painter.translate(-pos.center().x(),-pos.center().y());
        }
        painter.drawPixmap(pos,minusTowerPx);
        painter.restore();
        painter.setOpacity(fade);
        break;
    case TOWER_HERZ: //favoritentower
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=herzTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(herzTowerCost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
        painter.drawPixmap(pos,towerBasePx);
        painter.save();
        if(t) {
            painter.translate(pos.center());
            painter.rotate(t->angle);
            painter.translate(-pos.center().x(),-pos.center().y());
        }
        painter.drawPixmap(pos,favTowerPx);
        painter.restore();
        painter.setOpacity(fade);
        break;
    case TOWER_REPOST: //reposttower
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=repostTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(repostTowerCost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.36,pos.height()*0.36);
        painter.drawPixmap(pos,towerRepostBasePx);
        break;
    case TOWER_BENIS: //benistower
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=benisTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(benisTowerCost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.36,pos.height()*0.36);
        painter.drawPixmap(pos,towerBasePx);
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
            painter.drawPixmap(posF,pr0coinPx,QRectF(0,0,512,512));
        } else {
            painter.translate(-pos.center().x(),-pos.center().y());
            painter.drawPixmap(pos,pr0coinPx);
        }
        painter.restore();
        break;
    case TOWER_BAN: //bantower
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=banTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(banTowerCost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(gebannt);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.36,pos.height()*0.36);
        painter.drawPixmap(pos,towerBasePx);
        break;
    case TOWER_SNIPER: //snipertower
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=sniperTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(sniperTowerCost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
        painter.drawPixmap(pos,towerBasePx);
        painter.save();
        if(t) {
            painter.translate(pos.center());
            painter.rotate(t->angle);
            painter.translate(-pos.center().x(),-pos.center().y());
        }
        painter.drawPixmap(pos,sniperPx);
        painter.restore();
        painter.setOpacity(fade);
        break;
    case TOWER_FLAK: //flaktower
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=flakTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(flakTowerCost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
        painter.drawPixmap(pos,towerBasePx);
        painter.save();
        if(t) {
            painter.translate(pos.center());
            painter.rotate(t->angle);
            painter.translate(-pos.center().x(),-pos.center().y());
        }
        painter.drawPixmap(pos,flakTowerPx);
        painter.restore();
        painter.setOpacity(fade);
        break;
    case TOWER_LASER: //lasertower
        if(info && towerMenuSelected == tnum) { //towerinfo
            if(benis >= laserTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(laserTowerCost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
        painter.drawPixmap(pos,towerBasePx);
        painter.save();
        if(t) {
            painter.translate(pos.center());
            painter.rotate(t->angle);
            painter.translate(-pos.center().x(),-pos.center().y());
        }
        painter.drawPixmap(pos,laserTowerPx);
        painter.restore();
        painter.setOpacity(fade);
        break;
    case TOWER_POISON: //Gifttower
        if(info && towerMenuSelected == tnum) { //towerinfo
            if(benis >= poisonTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(poisonTowerCost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
        painter.drawPixmap(pos,towerBasePx);
        painter.save();
        if(t) {
            painter.translate(pos.center());
            painter.rotate(t->angle);
            painter.translate(-pos.center().x(),-pos.center().y());
        }
        painter.drawPixmap(pos,poisonTowerPx);
        painter.restore();
        painter.setOpacity(fade);
        break;
    case TOWER_MINIGUN:
        if(info && towerMenuSelected == tnum) { //towerinfo
            if(benis >= minigunTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(costPos,QString::number(minigunTowerCost));
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
        painter.drawPixmap(pos,towerBasePx);
        painter.save();
        if(t) {
            painter.translate(pos.center());
            painter.rotate(t->angle);
            painter.translate(-pos.center().x(),-pos.center().y());
            painter.drawPixmap(pos,minigunTowerPx[t->animation]);
        } else  painter.drawPixmap(pos,minigunTowerPx[1]);
        painter.restore();
        painter.setOpacity(fade);
        break;
    }
    changeSize(painter,38,true);
    painter.setPen(Qt::NoPen);
    if(towerMenuSelected == tnum && menu) {
        pos.adjust(-10,-10,10,10);
        painter.drawPixmap(pos,auswahlPx);
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
            for(uint i = 0;i <  mainLevels;i++) {
                if(!lvlPrices[i]) {
                    if(i+4!=subLevelSelected) painter.setOpacity(0.6*fade);
                    painter.drawPixmap(mainLvlRects[i],lvlPreviews[i]);
                    painter.setOpacity(fade);
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
    }
    painter.drawPixmap(shekelRect, shop->shekelMoney);
    switch(account.accountState) {
    case ACCOUNT_PENDING:
        painter.drawPixmap(accountRect, account_success_pendingPx);
        break;
    case ACCOUNT_ACTIVE:
    case ACCOUNT_VERIFIED:
        painter.drawPixmap(accountRect, account_successPx);
        break;
    default:
        painter.drawPixmap(accountRect, account_failPx);
        break;
    }
    painter.setPen(edlerSpender);
    changeSize(painter,48,true);
    painter.drawText(shekelRect.right() + 15, shekelRect.y() + 42, QString::number(shop->shekel));
    painter.setPen(Qt::white);
    changeSize(painter,32,true);
    painter.drawText(5,1070,version);
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
    painter.setOpacity(fade);
    if(!mapPlaying) {
        painter.drawPixmap(mPlayRect,mPlayPx);
    } else {
        painter.drawPixmap(mPlayRect,mStopPx);
    }
    if(!paths.size()||mCompileError) painter.setOpacity(0.5*fade);
    painter.drawPixmap(mSaveRect,mSavePx);
    painter.setOpacity(fade);
}

void FrmMain::drawBackMenu(QPainter &painter)
{
    painter.setBrush(grau);
    painter.setOpacity(0.5);
    painter.drawRect(0, 0, 1920, 1080);
    painter.setOpacity(1);

    painter.drawPixmap(btnContinueRect,btnContinue);
    painter.drawPixmap(btnBackRect,btnBack);
} //51

void FrmMain::towerMenuClicked(QRect pos)
{
    Tower *tmpTower = tiles[towerMenu]->t;
    if(tmpTower!=nullptr) {//turm steht, upgrade
        if(pos.intersects(btnDmgRect) && tmpTower->hasDamage) { //dmg upgrade
            if(tmpTower->dmglvl < 3) {
                if(towerMenuSelected == 1 && benis >= upgradeCost) { //kaufen
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
                        tmpTower->dmg += 15;
                        break;
                    case TOWER_FLAK: //flak
                        tmpTower->dmg += 5;
                        break;
                    case TOWER_POISON: //poison
                    case TOWER_MINIGUN:
                        tmpTower->dmg += 1;
                        break;
                    }
                    benis -= upgradeCost;
                    tmpTower->dmglvl++;
                } else towerMenuSelected = 1;
            }
        } else if(pos.intersects(btnRangeRect) && tmpTower->hasRange) { //range upgrade
            if(tmpTower->rangelvl < 3) {
                if(towerMenuSelected == 2 && benis >= upgradeCost) { //kaufen
                    tmpTower->range += 10;
                    benis -= upgradeCost;
                    tmpTower->rangelvl++;
                } else towerMenuSelected = 2;
            }
        } else if(pos.intersects(btnFirerateRect) && tmpTower->hasFirerate) { //feuerrate upgrade
            if(tmpTower->ratelvl < 3) {
                if(towerMenuSelected == 3 && benis >= upgradeCost) { //kaufen
                    tmpTower->reload *= 0.85;
                    benis -= upgradeCost;
                    tmpTower->ratelvl++;
                } else towerMenuSelected = 3;
            }
        } else if(pos.intersects(btnSpeedRect) && tmpTower->hasProjectileSpeed) { //kugelgeschwindigkeit upgrade
            if(tmpTower->speedlvl < 3) {
                if(towerMenuSelected == 4 && benis >= upgradeCost) { //kaufen
                    tmpTower->pspeed += 1;
                    benis -= upgradeCost;
                    tmpTower->speedlvl++;
                } else towerMenuSelected = 4;
            }
        } else if(pos.intersects(btnSellRect)) { //tower verkaufen
            if(towerMenuSelected == 5) {
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
                benis += add / 2;
                delTower(tmpTower);
            } else towerMenuSelected = 5;
        } else if(pos.intersects(btnTurnRect)) {
            if(tmpTower->turnlvl < 3) {
                if(towerMenuSelected == 6 && benis >= upgradeCost) {
                    switch(tmpTower->type) {
                    default:
                        tmpTower->angleSpeed *= 1.2;
                    }
                    benis -= upgradeCost;
                    tmpTower->turnlvl++;
                } else towerMenuSelected = 6;
            }
        }
    } else { //noch kein turm, turmauswahl
        if(pos.intersects(minusTowerRect)) { //Minustower
            if(towerMenuSelected == TOWER_MINUS && benis >= minusTowerCost) { //kaufen
                buyMinusTower();
            } else {
                towerMenuSelected = 1;
            }
        } else if(pos.intersects(favTowerRect)) { //Favoritentower
            if(towerMenuSelected == TOWER_HERZ && benis >= herzTowerCost) { //kaufen
                buyFavTower();
            } else {
                towerMenuSelected = 2;
            }
        } else if(pos.intersects(repostTowerRect) && !shop->towerPrices[TOWER_REPOST-1]) { //Reposttower
            if(towerMenuSelected == TOWER_REPOST && benis >= repostTowerCost) {
                buyRepostTower();
            } else {
                towerMenuSelected = 3;
            }
        } else if(pos.intersects(benisTowerRect) && !shop->towerPrices[TOWER_BENIS-1]) { //Benistower
            if(towerMenuSelected == TOWER_BENIS && benis >= benisTowerCost) {
                buyBenisTower();
            } else {
                towerMenuSelected = 4;
            }
        } else if(pos.intersects(banTowerRect) && !shop->towerPrices[TOWER_BAN-1]) { //Bantower
            if(towerMenuSelected == TOWER_BAN && benis >= banTowerCost) {
                buyBanTower();
            } else {
                towerMenuSelected = 5;
            }
        } else if(pos.intersects(sniperTowerRect) && !shop->towerPrices[TOWER_SNIPER-1]) { //Snipertower
            if(towerMenuSelected == TOWER_SNIPER && benis >= sniperTowerCost) {
                buySniperTower();
            } else {
                towerMenuSelected = 6;
            }
        } else if(pos.intersects(flakTowerRect) && !shop->towerPrices[TOWER_FLAK-1]) { //Flaktower
            if(towerMenuSelected == TOWER_FLAK && benis >= flakTowerCost) {
                buyFlakTower();
            } else {
                towerMenuSelected = 7;
            }
        } else if(pos.intersects(laserTowerRect) && !shop->towerPrices[TOWER_LASER-1]) {
            if(towerMenuSelected == TOWER_LASER && benis >= laserTowerCost) {
                buyLaserTower();
            } else {
                towerMenuSelected = 8;
            }
        } else if(pos.intersects(poisonTowerRect) && !shop->towerPrices[TOWER_POISON-1]) {
            if(towerMenuSelected == TOWER_POISON && benis >= poisonTowerCost) {
                buyPoisonTower();
            } else {
                towerMenuSelected = 9;
            }
        } else if(pos.intersects(minigunTowerRect) && !shop->towerPrices[TOWER_MINIGUN-1]) {
            if(towerMenuSelected == TOWER_MINIGUN && benis >= minigunTowerCost) {
                buyMinigunTower();
            } else {
                towerMenuSelected = 10;
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
    } else {
        for(uint i = 0; i < mainLevels; i++) {
            if(pos.intersects(mainLvlRects[i]) && !lvlPrices[i]) {
                subLevelSelected = i+4;
            }
        }
    }
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
                mWidth = 64;
                mHeight = 36;
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
    if(pos.intersects(mPathRect)) {
        editorSelected = 1;
    } else if(pos.intersects(mTurmTileRect)) {
        editorSelected = 2;
    } else if(pos.intersects(mEmptyTileRect)) {
        editorSelected = 3;
    } else if(pos.intersects(mBaseRect)) {
        editorSelected = 5;
    } else if(pos.intersects(mEnemyBaseRect)) {
        editorSelected = 4;
    } else if(pos.intersects(mClearRect)) {
        editorSelected = 6;
    } else if(pos.intersects(mPlayRect)) {
        if(!mapPlaying) {
            mCompileError = createPath();
            switch (mCompileError) {
            case -1:
                QMessageBox::information(this,"Fehler","Kein Start gefunden!");
                break;
            case -2:
                QMessageBox::information(this,"Fehler","Kein Ziel gefunden!");
                break;
            default: //alles ok
                bool ok=false;
                for(uint i=0;i<tiles.size();i++) {
                    if(tiles[i]->type==2) {
                        ok = true;
                        break;
                    }
                }
                if(ok) {
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
    } else { //kein Menü -> tilecheck
        if(mapPlaying) return;
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

void FrmMain::gameOver()
{
    isGameOver = true;
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
    if(file.exists()) hasPlayingSave = true;
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
}

void FrmMain::loadMap(QString name, int custom, int width, int height, QString path)
{
    loaded = true;
    QString map;
    switch (custom) {
    case 0: //standard
    {
        mName = name;
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
                createTileBodies.push_back(t);
                tiles.push_back(t);
            }
        }
    } else { //userlevel
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
                createTileBodies.push_back(t);
                tiles.push_back(t);
            }
        }
        this->paths = levels[subLevelSelected-1].paths;
    }
    viewRect = QRect(0,0,mapwidth,mapheight);
    mapPx = createMapImage(custom);
#ifdef QT_DEBUG
    QFile file;
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/img2.png");
    file.setFileName(path);
    file.open(QIODevice::WriteOnly);
    mapPx.save(&file);
    file.close();
#endif
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
        if(mapversion != version) {
            if(!mapversion.contains(".")) {
                error_save(file);
                return;
            }
            if(mapversion.toDouble() < 1.0) {
                error_save(file);
                return;
            }
            //Mapversion ungleich, evtl konvertierung
        }
        //Map laden
        switch(split[1].toInt()) {
        case 0: //standardlevel
            loadMap(split[2]);
            break;
        case 1: //userlevel
            subLevelSelected = split[2].toInt();
            loadMap("",2);
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
            t->saveNum = data[21].toInt();
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
            p->saveNum = data[23].toInt();
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
        currentWave.waveCount = waveSplit[9].toInt();
        currentWave.maxEnemysPerWave = waveSplit[10].toInt();
        currentWave.enemysPerWave = waveSplit[11].toInt();
        currentWave.waveTime = waveSplit[12].toInt();
        //------
        //Allgemeines Zeugs laden
        this->benis = split[7].toULongLong();
        this->waveCount = split[8].toInt();
        this->internalWaveCount = split[9].toInt();
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
    out << version << "#";
    if(mName=="") t = 1;
    out << QString::number(t) << "#";
    if(t) {
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
    out << "#" << QString::number(this->benis) << "#" << QString::number(waveCount) << "#";
    out << QString::number(internalWaveCount) << "#";
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

void FrmMain::saveOptions()
{
    QFile file;
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    path.append("/options.dat");
    file.setFileName(path);
    file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    QTextStream out(&file);
    unsigned long long a = shop->shekel * 3;
    out << QString::number(soundEnabled) << "#"
        << newestPost << "#"
        << QString::number(accepted) << "#"
        << QString::number( a, 16 ).toUpper();
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
                newestPost = split[1];
            }
            if(split.size() > 2) {
                accepted = split[2].toInt();
            }
            if(split.size() > 3) {
                unsigned long long a = split[3].toULongLong(nullptr,16);
                shop->shekel = a / 3;
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
            QStringList split = data.split("#");
            for(int i = 0; i < split.size(); i++) {
                lvlPrices.push_back(split[i].toInt());
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
}

int FrmMain::createPath()
{
    paths.resize(0);
    begin = -1;
    std::vector <int> starts;
    int maxX = mapwidth/80;
    for(uint i=0;i<tiles.size();i++) {
        if(tiles[i]->type==4) { //spawn
            starts.push_back(i);
        }
    }
    if(!starts.size()) {
        return -1; //kein start
    }
    for(uint i=0;i<starts.size();i++) {
        Path tmpPath;
        int begin = starts[i];
        tmpPath.path.push_back(begin);
        bool end=false;
        int last=-1;
        int current = begin;
        int tSize = tiles.size();
        while(!end) {
            int next;
            int ok = 0;
            next = current+1; //nach rechts
            if(next>-1&&next!=last&&next<tSize&&
                    ((next/maxX)==(current/maxX))) {
                if(tiles[next]->type==1) {
                    ok = 1;
                } else if(tiles[next]->type==5) {
                    ok = 2;
                }
            }
            if(ok) { //nach rechts bestätigt
                tmpPath.path.push_back(next);
                last = current;
                current = next;
                if(ok==2) {
                    end = true;
                }
            } else {
                next = current+maxX; //nach unten
                if(next>-1&&next!=last&&next<tSize) {
                    if(tiles[next]->type==1) {
                        ok = 1;
                    } else if(tiles[next]->type==5) {
                        ok = 2;
                    }
                }
                if(ok) { //nach unten bestätigt
                    tmpPath.path.push_back(next);
                    last = current;
                    current = next;
                    if(ok==2) {
                        end = true;
                    }
                } else {
                    next = current-1;
                    if(next>-1&&next!=last&&next<tSize&&
                            ((next/maxX)==(current/maxX))) {
                        if(tiles[next]->type==1) {
                            ok = 1;
                        } else if(tiles[next]->type==5) {
                            ok = 2;
                        }
                    }
                    if(ok) { //nach links bestätigt
                        tmpPath.path.push_back(next);
                        last = current;
                        current = next;
                        if(ok==2) {
                            end = true;
                        }
                    } else {
                        next = current-maxX;
                        if(next>-1&&next!=last&&next<tSize) {
                            if(tiles[next]->type==1) {
                                ok = 1;
                            } else if(tiles[next]->type==5) {
                                ok = 2;
                            }
                        }
                        if(ok) {
                            tmpPath.path.push_back(next);
                            last = current;
                            current = next;
                            if(ok==2) {
                                end=true;
                            }
                        } else {
                            end=true;
                        }
                    }
                }
            }
            if(tmpPath.path.size()>tiles.size()) {
                return -2; //kein weg gefunden
            }
            if(paths.size()) {
                for(uint a=0;a<paths.size();a++) {
                    Path p = paths[a];
                    bool take=false;
                    for(uint b=0;b<p.path.size();b++) {
                        if(take) {
                            tmpPath.path.push_back(p.path[b]);
                        }
                        if(p.path[b]==tmpPath.path.back()) {
                            //pfad bereits vorhanden -> übernehmen
                            take = true;
                        }
                    }
                    if(take) {
                        end = true;
                        break;
                    }
                }
            }
        }
        if(tiles[tmpPath.path[tmpPath.path.size()-1]]->type!=5) {
            return -2; //start = ziel
        }
        paths.push_back(tmpPath);
    }
    return 0; //alles ok
    //path.erase(path.begin()+path.size()-1);
}

void FrmMain::createAccount()
{
    QGuiApplication::inputMethod()->show();
    QString name = QInputDialog::getText(this, "Benutzername", "Benutzername: (3-20 Zeichen lang)(Darf nur Zahlen & Buchstaben enthalten!)");
    bool ok = true;
    if(name.size() < 3 || name.size() > 20) ok=false;
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
                    if(!password[i].isLetter()&&!password[i].isDigit()) {
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
                    accountCreationError();
                }
            } else if(!ok){ //Name verwendet
                QMessageBox::information(this,"Fehler","Name bereits verwendet! Versuche es nochmal!");
            } else if(ok == -1) { //server offline
                QMessageBox::information(this,"Fehler","Server nicht erreichbar! Bitte versuche es später noch einmal!");
            }
        } else {
            accountCreationError();
        }
    } else accountCreationError();
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

void FrmMain::accountCreationError()
{
    QMessageBox::information(this, "Info", "<b>Irgendwas ist schiefgelaufen!</b><br>"
                                           "Klicke auf das Accountsymbol,<br>"
                                           "um es erneut zu probieren!");
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
}

void FrmMain::touchEvent(QTouchEvent *e)
{
    if((gamePaused && backMenu) || isGameOver || active == STATE_MAINMENU) return;
    if(!zooming) zoomScale = scaleX;
    QList<QTouchEvent::TouchPoint> touchPoints = e->touchPoints();
    if(zooming&&e->type()==QEvent::TouchEnd&&e->touchPoints().count()==1) {
        if(zooming>0) zooming--;
    }
    if(touchPoints.count()==2) {
        const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
        const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
        if(zooming&&touchPoint0.state()==Qt::TouchPointReleased) {
            if(zooming>0) zooming--;
        }
        if(zooming&&touchPoint1.state()==Qt::TouchPointReleased) {
            if(zooming>0) zooming--;
        }
        double l1 = QLineF(touchPoint0.pos(),touchPoint1.pos()).length();
        double l2 = QLineF(touchPoint0.startPos(),touchPoint1.startPos()).length();
        double currentScaleFactor = l1/l2;
        if(e->touchPointStates()==Qt::TouchPointPressed||e->touchPointStates()==Qt::TouchPointMoved) {
            if(!zooming) zooming = 2;
            mid = viewRect.center();
            if(currentScaleFactor>1) {
                currentScaleFactor = 1.05;
            } else if(currentScaleFactor<1) {
                currentScaleFactor = 0.95;
            }
            double scale = (double)1/currentScaleFactor;
            int nw = viewRect.width()*scale;
            int nh = viewRect.height()*scale;
            int ntx,nty;
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
}

void FrmMain::mouseMoveEvent(QMouseEvent *e)
{
    if(active == STATE_MAINMENU || (gamePaused&&backMenu) || isGameOver) return;
    int x = e->pos().x();
    int y = e->pos().y();
    if(towerMenu != -1) {
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
    if(gamePaused) {
        if(mRectRaw.intersects(btnPlayingSpeedRect)&&!playingSpeed) {
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
    switch(active) {
    case STATE_MAINMENU: //hauptmenü
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
        } else if(mRectRaw.intersects(btnSoundRect)) { //sound
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
                MsgBox::createMsgBox(box, "Du hast aktuell noch <b>keinen</b> Account erstellt!<br>"
                                          "Ein Account bietet dir folgende Vorteile:<br>"
                                          "- Statistiken aktivieren<br>"
                                          "Bald noch vieles mehr!<br>"
                                          "Möchtest du einen Account erstellen?");
                int ret = box.exec();
                if(ret == 0) { //Ja geklickt
                    createAccount();
                }
            } else if(account.accountState == ACCOUNT_PENDING) {
                QMessageBox::information(this,"Info", "Warte auf Antwort vom Server\n"
                                                      "Falls du mit dem Internet verbunden bist\n"
                                                      "starte die App neu.");
            }
        }
        break;
    case STATE_PLAYING: //ingame
        if(mRectRaw.intersects(btnPlayingSpeedRect)&&!playingSpeedChanged) {
            //qDebug()<<enemys.size();
            switch(playingSpeed) {
            case 1: //normal
                do {
                    QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,25));
                    QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,5));
                    QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,5));
                    QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,50));
                } while(!checkTimers());
                playingSpeed = 2;
                break;
            case 2: //schnell
                do {
                    QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,50));
                    QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,10));
                    QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,10));
                    QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100));
                } while(!checkTimers());
                playingSpeed = 1;
                break;
            }
        } else if(towerMenu==-1 || !mRectRaw.intersects(towerMenuRect) &&
                  !QRect(mPos.x()/scaleFHDX, mPos.y()/scaleFHDY, 1, 1).intersects(towerMenuRect) &&
                  towerMenuAnimating == -1) {
            if(!mRectRaw.intersects(towerMenuRect) && towerMenu>-1) {
                towerMenuSelected = 0;
                towerMenu = -1;
                for(uint i=0;i<tiles.size();i++) {
                    if(mRect.intersects(tiles[i]->rect())) {
                        if(tiles[i]->type==2) {//turm
                            towerMenu = i;
                        }
                    }
                }
                if(towerMenu == -1) { //verschwinden animieren
                    towerMenuAnimating = 0;
                    towerMenuAnimatingDir = 1;
                }
            } else {
                for(uint i=0;i<tiles.size();i++) {
                    if(mRect.intersects(tiles[i]->rect())) {
                        if(tiles[i]->type==2) {//turm
                            towerMenuAnimating = i;
                            towerMenuAnimatingDir = 0;
                            //towerMenu = i;
                            towerMenuRectAnimation = towerMenuRect;
                            towerMenuRectAnimation.moveTo(-600,towerMenuRectAnimation.y());
                            towerMenuSelected = 0;
                            break;
                        }
                    }
                }
            }
        } else {
            towerMenuClicked(mRectRaw);
        }
        break;
    case STATE_EDITOR: //Mapeditor
        editorClicked(mRectRaw,mRect);
        break;
    }
}

void FrmMain::keyPressEvent(QKeyEvent *e)
{
    if(moveAn != ANIMATION_SUSPENDED) return;
    if(e->key()==Qt::Key_Escape||e->key()==Qt::Key_Back) {
        if((active == STATE_PLAYING && !isGameOver) || active == STATE_EDITOR) { //Im Spiel oder Editor
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
}

void FrmMain::paintEvent(QPaintEvent *e)
{
    if(!key) return;
    mutex.lock();
    try {
        Q_UNUSED(e)
        if(DEBUG) {
            timerD.restart();
        }
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        //painter.setRenderHint(QPainter::SmoothPixmapTransform);
        scaleX = double(this->geometry().width()/double(viewRect.width()));
        scaleY = double(this->geometry().height()/double(viewRect.height()));
        scaleFHDX = double(this->geometry().width()/double(1920));
        scaleFHDY = double(this->geometry().height()/double(1080));
        if(active == STATE_PLAYING || active == STATE_EDITOR) painter.save();
        painter.scale(scaleX,scaleY);
        painter.setBrush(grau);
        painter.translate(transX-shakeX,transY-shakeY);
        painter.drawRect(viewRect);
        switch(active) {
        case STATE_MAINMENU: //menu
            //iwas blockiert den thread bei active = 1 für paar sec
            if(moveAn > 0 && (moveAn < ANIMATION_BACK || moveAn == ANIMATION_CONTINUE) && fdir) {
                drawIngame(painter);
                changeSize(painter,128,true);
                painter.setPen(Qt::white);
                painter.drawText(420,400,"Lade...");
            } else {
                drawMainMenu(painter);
            }
            if(outdated) {
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
        if(backMenu) {
            painter.setOpacity(1);
            drawBackMenu(painter);
        }
        if(blendRectOpacity) {
            painter.setBrush(grau);
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
            tdrawMS = timerD.nsecsElapsed()/NANO_TO_MILLI;
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
        }
        mutex.unlock();
    } catch(std::exception e) {

    }
}

void FrmMain::wheelEvent(QWheelEvent *e)
{
    if(active == STATE_MAINMENU || (gamePaused&&backMenu) || isGameOver) return;
    int angle = e->angleDelta().y();
    double currentScaleFactor;
    if(angle>0) {//nach oben
        currentScaleFactor = 1.2;
    } else if(angle<0) { //nach unten
        currentScaleFactor = 0.8;
    }
    mid = viewRect.center();
    double scale = (double)1/currentScaleFactor;
    int nw = viewRect.width()*scale;
    int nh = viewRect.height()*scale;
    int ntx,nty;
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
    transX = ntx;
    transY = nty;
    viewRect.moveTo(-ntx,-nty);
    viewRect.setWidth(nw);
    viewRect.setHeight(nh);
}

void FrmMain::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
    t_draw->stop();
    QMetaObject::invokeMethod(t_main,"stop");
    QMetaObject::invokeMethod(t_projectile,"stop");
    workerThread->quit();
}
