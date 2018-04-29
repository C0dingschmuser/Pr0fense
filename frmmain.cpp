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
#ifdef Q_OS_ANDROID
    store = new QInAppStore(this);
    connect(store,SIGNAL(transactionReady(QInAppTransaction*)),this,SLOT(handleTransaction(QInAppTransaction*)));
    connect(store,SIGNAL(productRegistered(QInAppProduct*)),this,SLOT(productRegistered(QInAppProduct*)));
    connect(store,SIGNAL(productUnknown(QInAppProduct::ProductType,QString)),this,SLOT(productUnknown(QInAppProduct::ProductType,QString)));
    store->registerProduct(QInAppProduct::Unlockable,QStringLiteral("mapeditor_medium"));
    store->registerProduct(QInAppProduct::Unlockable,QStringLiteral("mapeditor_large"));
#endif
    playlist_menu->addMedia(QUrl("qrc:/data/music/track1.mp3"));
    playlist_game->addMedia(QUrl("qrc:/data/music/track2.mp3"));
    playlist_menu->addMedia(QUrl("qrc:/data/music/track4.mp3"));
    playlist_game->addMedia(QUrl("qrc:/data/music/track3.mp3"));
    //fehler: wenn mapeditor ausgewählt und weiterspielen dann grid
    connect(music,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT(on_mediaStateChanged(QMediaPlayer::MediaStatus)));
    connect(qApp,SIGNAL(applicationStateChanged(Qt::ApplicationState)),this,SLOT(on_appStateChanged(Qt::ApplicationState)));
    pool = new ObjectPool(&mutex);
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
    loadUserData();
    loadLitteSprueche();
    initPhysics();
    music->setPlaylist(playlist_menu);
    //TODO
    //Waves allg
#ifdef QT_NO_DEBUG
    QMessageBox::information(this,
                             "INFO",
                             "<b>Pr0fense befindet sich in der Testphase</b><br>"
                             "- Bugs/Feedback bitte über Discord oder @05 melden<br>"
                             "- <b>Ja, ich weiß</b>, dass die Turm-Upgrades noch <b>gratis</b> sind<br>"
                             "- <b>Ja, ich weiß</b>, dass der Shop aktuell nicht geht<br>"
                             "- <b>Ja, ich weiß</b>, dass es aktuell noch Unbalanced ist<br>"
                             "- Sourcecode findet ihr auf <b>github.com/firediver/pr0fense</b><br>"
                             "- Zur Fehlererkennung und Diagnostik werden einige anonyme Daten an<br>"
                             "den Server geschickt, dazu gehören:<br>"
                             "-> Onlinezeiten<br>"
                             "-> Fehlermeldungen<br>"
                             "-> Firebase-Spezifische Daten");
#endif
    QFuture<void> future = QtConcurrent::run(this, &FrmMain::getStatus);
    //getStatus();
    t_draw->start(16); //NUR zeichnen & input
    t_main->start(10); //turmschüsse & so
    t_wave->start(100); //gegnerspawn
    t_waveSpeed->start(5); //gegnerbewegung
    t_projectile->start(10); //projektile von türmen
    t_projectile_full->start(100); //update von ziel
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
    if(active==1) {
        saveGameSave();
    }
    saveMaps();
    saveOptions();
    QMetaObject::invokeMethod(t_draw,"stop");
    QMetaObject::invokeMethod(t_main,"stop");
    QMetaObject::invokeMethod(t_projectile,"stop");
    QMetaObject::invokeMethod(t_projectile_full,"stop");
    QMetaObject::invokeMethod(t_wave,"stop");
    QMetaObject::invokeMethod(t_waveSpeed,"stop");
    QMetaObject::invokeMethod(t_physics,"stop");
    QMetaObject::invokeMethod(t_menuAn,"stop");
    workerThread->terminate();
    delete ui;
    delete world;
    delete pool;
}

void FrmMain::initPhysics()
{
    b2Vec2 gravity(0,0);
    world = new b2World(gravity);
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
    if(exit) qApp->exit();
    if(newPost&&!active) {
        newPost = false;
        newPostInfo();
    }
    if(active==0||active==2||gamePaused) return;

    if(DEBUG) {
        timerM.restart();
    }
    try {
        //mutex.lock();
        for(uint i=0;i<towers.size();i++) {
            if(tiles[towers[i]->tnum]->towernum!=(int)i) {
                delTower(i);
            } else {
                if(towers[i]->currentReload<=0) { //schuss
                    int pos = -1, min=1000;
                    QPointF tCenter = towers[i]->pos.center();
                    bool done = false;
                    if(towers[i]->hasProjectileSpeed) {
                        for(uint a=0;a<enemys.size()&&!done;a++) {
                            if(enemys[a]->physicsInitialized) {
                                if(enemys[a]->preHealth&&!enemys[a]->soonBanned) {
                                    double health = enemys[a]->preHealth;
                                    double distance = Engine::getDistance(tCenter,enemys[a]->rectF().center());
                                    if(distance<towers[i]->range) {
                                        switch(towers[i]->type) {
                                        case 1: //Minustower
                                            if(enemys[a]->stunned) { //wenn betäubt focus
                                                pos = a;
                                                done = true;
                                                break;
                                            } else if(health<min) {
                                                pos = a;
                                                min = health;
                                                //done=true; //<-hinmachen falls nicht auf den mit niedrigsten hp
                                            }
                                            break;
                                        case 2: //Favoritentower
                                        case 3: //Repostturm
                                            if(distance<min) {
                                                pos = a;
                                                min = distance;
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if(pos>=0||towers[i]->type==3||towers[i]->type==4||towers[i]->type==5) {
                        if(towers[i]->type<3) {
                            Projectile *p = pool->getProjectile();
                            QPointF enemy = enemys[pos]->rect().center();
                            int angle = Engine::getAngle(tCenter,enemy);
                            QRectF rect;
                            switch(towers[i]->type) {
                            case 1: //minus
                                rect = QRectF(tCenter.x()-10,tCenter.y()-10,20,20);
                                break;
                            case 2: //herz
                                rect = QRectF(tCenter.x()-20,tCenter.y()-20,40,40);
                                break;
                            }
                            p->init(rect,
                                    angle,
                                    towers[i]->dmg,
                                    enemys[pos],
                                    qCos(qDegreesToRadians((double)angle)),
                                    qSin(qDegreesToRadians((double)angle)),
                                    towers[i]->pspeed,
                                    towers[i]->stun,
                                    0,
                                    towers[i]->projectilePxID);
                            enemys[pos]->preHealth -= p->dmg;
                            if(enemys[pos]->preHealth<0) enemys[pos]->preHealth = 0;
                            projectiles.push_back(p);
                        } else if(towers[i]->type==3){ //range attack -> alle in bereich betroffen
                            //return;
                            for(uint b=0;b<enemys.size();b++) {
                                if(enemys[b]->physicsInitialized) {
                                    double distance = Engine::getDistance(tCenter,enemys[b]->rectF().center());
                                    if(distance<towers[i]->range) {
                                        enemys[b]->setRepost(towers[i]->repost);
                                    }
                                }
                            }
                        } else if(towers[i]->type==4) {//benistower
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
                            }
                        } else if(towers[i]->type==5&&enemys.size()) { //banntower
                            Enemy *target = towers[i]->target;
                            int ok=true;
                            if(!target||!target->physicsInitialized) {
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
                                    towers[i]->target = NULL;
                                    towers[i]->currentReload = towers[i]->reload;
                                    towers[i]->shotsFiredSinceReload = 0;
                                }
                            }
                            if(ok>0) {
                                //Schuss
                                int dmg = target->maxHealth/10;
                                if(dmg<1) dmg = 1;
                                if(towers[i]->shotsFiredSinceReload==9) dmg = target->maxHealth;
                                target->preHealth -= dmg;
                                QPointF enemy = target->rect().center();
                                int angle = Engine::getAngle(tCenter,enemy);
                                if(towers[i]->shotsFiredSinceReload==9) {
                                    angle += 180;
                                } else {
                                    angle = Engine::random(0,360);
                                }
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
                                    towers[i]->target = NULL;
                                }
                            }
                        }
                        if(towers[i]->type!=5) {
                            towers[i]->currentReload = towers[i]->reload;
                        }
                    }
                } else {
                    if(towers[i]->currentReload>0) towers[i]->currentReload -= 10;
                }
            }
        }
        for(uint i=0;i<enemys.size();i++) {
            if(enemys[i]->health<=0) {//wenn tot dann löschen
                benis += enemys[i]->price;
                QPoint point = enemys[i]->rect().center();
                int angle = -90;
                Projectile *p = pool->getProjectile();
                p->init3("+ "+QString::number(enemys[i]->price),
                         QPoint(point.x()-25,point.y()),
                         angle,
                         qCos(qDegreesToRadians((double)angle)),
                         qSin(qDegreesToRadians((double)angle)),
                         0.01,
                         0.5);
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
    if(active==2||(gamePaused&&backMenu)||(gamePaused&&loaded)||moveAn==4) return;
    if(DEBUG) {
        timerP.restart();
    }
    try {
        //mutex.lock();
        for(uint i=0;i<projectiles.size();i++) {
            if(!projectiles[i]->del) {
                projectiles[i]->update();
                if(!projectiles[i]->type) { //projektil
                    if(projectiles[i]->crect().intersects(projectiles[i]->target->rectF())) {
                        projectiles[i]->target->reduceHealth(projectiles[i]->dmg);
                        if(projectiles[i]->stun) {
                            projectiles[i]->target->setStun(projectiles[i]->stun);
                        }
                        projectiles[i]->del = 1;
                        break;
                    }
                } else if(projectiles[i]->type==2||projectiles[i]->type==1){ //text oder blus
                    if(projectiles[i]->opacity<=0&&projectiles[i]->opacityDecAm) projectiles[i]->del = 3;
                }
                if(!projectiles[i]->type&&
                        !projectiles[i]->target->soonBanned) { //wenn kein effekt
                    if(!projectiles[i]->rect.intersects(QRectF(0,0,mapwidth,mapheight))) {
                        projectiles[i]->del = 2;
                    }
                }
                if(projectiles[i]->type==1&&projectiles[i]->opacity<0.5) {
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
    /*warum nochmal? updatet vx und vy 'nur' alle
    200ms-> deutlich weniger rechenleistung wird verbraucht*/
    if(active==2||gamePaused) return;
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
                gamePaused||active==0||(active==2&&!mapPlaying)||
                isGameOver||!playingSpeed) return;
        QRect knot = tiles[paths[chosenpath].path[0]]->rect();
        int width;
        int health;
        int price;
        int type;
        double speed;
        double diff = 1;
        if(waveCount>10) {
            diff = 1.1*(waveCount/10);
        }
        if(currentWave.fliesen) {
            type = 0;
            health = 25*diff;
            speed = 1;
            price = 5;
        } else if(currentWave.neuschwuchteln) {
            type = 1;
            health = 30*diff;
            speed = 0.80;
            price = 10;
        } else if(currentWave.schwuchteln) {
            type = 2;
            health = 40*diff;
            speed = 0.55;
            price = 15;
        } else if(currentWave.spender) {
            type = 3;
            health = 50;
            speed = 0.65;
            price = 20;
        } else if(currentWave.altschwuchteln) {
            type = 4;
            health = 70*diff;
            speed = 0.45;
            price = 20;
        } else if(currentWave.mods) {
            type = 5;
            health = 100*diff;
            speed = 0.40;
            price = 30;
        } else if(currentWave.admins) {
            type = 6;
            health = 100*diff;
            speed = 0.35;
            price = 30;
        } else if(currentWave.legenden) {
            type = 7;
            health = 200*diff;
            speed = 0.3;
            price = 40;
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
        //if(width>50) {
            r = QRectF(knot.x()+((80-width)/2)+width/2,
                 knot.y()+((80-width)/2)+width/2,
                 width,width);
        //} else {
            //double x = (double)Engine::random(knot.x(),(knot.x()+knot.width())-width);
            //double y = (double)Engine::random(knot.y(),(knot.y()+knot.height())-width);
            //r = QRectF(x,y,width,width);
        //}
        if(playingSpeed&&!enemys.size()&&currentWave.empty) {
            playingSpeed = 0;
            gamePaused = true;
            waveCount++;
            if(waveCount%10!=0&&waveCount) {
                internalWaveCount++;
            } else {
                internalWaveCount = 0;
            }
            if(waveCount%5==0&&waveCount) {
                chosenpath++;
                if(chosenpath>paths.size()-1) {
                    chosenpath = 0;
                }
            }
            currentWave = waves[internalWaveCount];
            return;
        }
        if(currentWave.fliesen) {
            currentWave.fliesen--;
        } else if(currentWave.neuschwuchteln) {
            currentWave.neuschwuchteln--;
        } else if(currentWave.schwuchteln) {
            currentWave.schwuchteln--;
        } else if(currentWave.spender) {
            currentWave.spender--;
        } else if(currentWave.altschwuchteln) {
            currentWave.altschwuchteln--;
        } else if(currentWave.mods) {
            currentWave.mods--;
        } else if(currentWave.admins) {
            currentWave.admins--;
        } else if(currentWave.legenden) {
            currentWave.legenden--;
        } else {
            currentWave.empty = true;
            return;
        }
        Enemy *e = pool->getEnemy();//new Enemy(knot,0);
        e->init(r,speed,type,0,health,chosenpath,price);
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
        if((active==1||active==2)&&mapLoaded) {
            if(DEBUG) {
                timerWS.restart();
            }
            for(uint i=0;i<enemys.size();i++) {
                int next = enemys[i]->cpos+1;
                //mutex.lock();
                if(enemys[i]->repost) {
                    enemys[i]->repost -= 5*playingSpeed;
                    if(enemys[i]->repost<0) enemys[i]->repost = 0;
                }
                if(next<(int)paths[enemys[i]->path].path.size()) {
                    QRect knot = tiles[paths[enemys[i]->path].path[next]]->rect();
                    knot.adjust(20,20,-20,-20);
                    if(enemys[i]->stunned) {
                        if(enemys[i]->speed >= enemys[i]->ospeed*0.5) {
                            enemys[i]->speed -= enemys[i]->ospeed*0.1;
                        }
                    } else {
                        if(enemys[i]->speed < enemys[i]->ospeed) {
                            enemys[i]->speed += enemys[i]->ospeed*0.1;
                            if(enemys[i]->speed > enemys[i]->ospeed) enemys[i]->speed = enemys[i]->ospeed;
                        }
                    }
                    double speed = enemys[i]->speed;
                    //if(i-1<enemys.size()) speed = enemys[i-1]->speed;
                    double x=0,y=0;
                    if(paths[enemys[i]->path].path[next-1]+1==paths[enemys[i]->path].path[next]) { //rechts
                        x = speed;
                        enemys[i]->dir = 1;
                    } else if(paths[enemys[i]->path].path[next-1]-1==paths[enemys[i]->path].path[next]) { //links
                        x = -speed;
                        enemys[i]->dir = 3;
                    }
                    if(paths[enemys[i]->path].path[next-1]+(mapwidth/80)==paths[enemys[i]->path].path[next]) { //unten
                        y = speed;
                        enemys[i]->dir = 2;
                    } else if(paths[enemys[i]->path].path[next-1]-(mapwidth/80)==paths[enemys[i]->path].path[next]) { //oben
                        y = -speed;
                        enemys[i]->dir = 4;
                    }
                    enemys[i]->moveBy(x,y);
                    if(!enemys[i]->rect().intersects(tiles[paths[enemys[i]->path].path[next-1]]->rect())) {
                        enemys[i]->cpos++;
                    }
                    /*int matches = 0;
                    if(enemys[i]->rect().intersects(tiles[path[next]]->rect())) {
                        for(uint a=next-1;a<path.size();a++) {
                            if(enemys[i]->rect().intersects(tiles[path[a]]->rect())) {
                                matches++;
                            }
                        }
                        if(matches==1) {
                            enemys[i]->cpos++;
                        }
                    }*/

                } else {
                    //gegner löschen
/*#ifdef QT_NO_DEBUG //camera shake bei release
                    shaking = 100;
                    if(!t_shake->isActive()) QMetaObject::invokeMethod(t_shake,"start",Q_ARG(int,5));
#endif*/
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
            case 1: //active
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
            case 2: //mapeditor
                reset(3);
                loadMap(mName,1,mWidth,mHeight);
                //active = 2;
                break;
            case 3: //mapeditor-own
                reset(2);
                subLevelSelected = subActiveSelected;
                loadMap(mName,2,mWidth,mHeight);
                //active = 2;
                break;
            case 4: //zurück
                if(active!=2&&!error) {
                    changePlaylist(0);
                    saveGameSave();
                }
                active = 0;
                reset();
                break;
            case 5: //continue
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
            if(moveAn==1||moveAn==5) {
                active = 1;
            } else if(moveAn==2||moveAn==3) {
                active = 2;
            }
            moveAn = -1;
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
    if(active==0&&!t_animation->isActive()) {
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
    if(!gamePaused&&active==1) {
        //qDebug()<<world->GetBodyCount();
        //mutex.lock();
        if(playingSpeed==1) {
            world->Step(0.05,2,4);
        } else if(playingSpeed==2){
            world->Step(0.1,2,4);
        }
        for(uint i=0;i<enemys.size();i++) {
            //enemys[i]->updatePos();
        }
    }
    for(uint i=0;i<delBodies.size();i++) {
        if(!world->IsLocked()) {
            if(delBodies[i]!=nullptr) {
                world->DestroyBody(delBodies[i]);
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
        music->setVolume(25);
        if(soundEnabled) {
            music->play();
        }
    }
}

void FrmMain::on_appStateChanged(Qt::ApplicationState state)
{
    if((state==Qt::ApplicationInactive||
            state==Qt::ApplicationSuspended)&&!suspended) {
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
        if((active==1&&!isGameOver)||active==2) {
            if(active==1) {
                saveGameSave();
            }
            backMenu = true;
            gamePaused = true;
        }
        saveMaps();
        saveOptions();
    } else if(state==Qt::ApplicationActive&&suspended) {
        suspended = false;
        QMetaObject::invokeMethod(t_draw,"start");
        if(soundEnabled) QMetaObject::invokeMethod(music,"play");
        QMetaObject::invokeMethod(t_main,"start");
        QMetaObject::invokeMethod(t_projectile,"start");
        QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,100));
        QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100));
        QMetaObject::invokeMethod(t_waveSpeed,"start");
        QMetaObject::invokeMethod(t_physics,"start");
        QMetaObject::invokeMethod(t_menuAn,"start");
        if(active==1&&enemys.size()) {
            playingSpeed = 1;
            resetTimers();
        }
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
    t->type = 1;
    t->dmg = 10;
    t->stun = 0;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 1000;
    t->currentReload = t->reload;
    t->range = 150;
    t->pspeed = 2.5;
    t->name = "Minustower";
    t->tnum = towerMenu;
    towers.push_back(t);
    tiles[towerMenu]->towernum = towers.size()-1;
    towerMenuSelected = 0;
}

void FrmMain::buyFavTower()
{
    benis -= herzTowerCost;
    Tower *t = new Tower();
    t->type = 2;
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
    towers.push_back(t);
    tiles[towerMenu]->towernum = towers.size()-1;
    towerMenuSelected = 0;
}

void FrmMain::buyRepostTower()
{
    benis -= repostTowerCost;
    Tower *t = new Tower();
    t->type = 3;
    t->dmg = 0;
    t->stun = 0;
    t->repost = 3000;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 5000;
    t->currentReload = t->reload;
    t->range = 125;
    t->hasProjectileSpeed = false;
    t->pspeed = 2.5;
    t->projectilePxID = 1;
    t->name = "Reposttower";
    t->tnum = towerMenu;
    towers.push_back(t);
    tiles[towerMenu]->towernum = towers.size()-1;
    towerMenuSelected = 0;
}

void FrmMain::buyBenisTower()
{
    benis -= benisTowerCost;
    Tower *t = new Tower();
    t->type = 4;
    t->dmg = 10;
    t->stun = 0;
    t->repost = 0;
    t->hasProjectileSpeed = false;
    t->hasRange = false;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 10000;
    t->currentReload = t->reload;
    t->range = 0;
    t->name = "Benistower";
    t->tnum = towerMenu;
    towers.push_back(t);
    tiles[towerMenu]->towernum = towers.size()-1;
    towerMenuSelected = 0;
}

void FrmMain::buyBanTower()
{
    benis -= benisTowerCost;
    Tower *t = new Tower();
    t->type = 5;
    t->dmg = 10;
    t->stun = 0;
    t->repost = 0;
    t->pos = tiles[towerMenu]->pos;
    t->reload = 20000;
    t->currentReload = t->reload;
    //t->reloadMax = 30000;
    t->hasDamage = false;
    t->hasRange = false;
    t->range = 0;
    t->name = "Bantower";
    t->tnum = towerMenu;
    t->projectilePxID = 2;
    t->pspeed = 2.5;
    towers.push_back(t);
    tiles[towerMenu]->towernum = towers.size()-1;
    towerMenuSelected = 0;
}

void FrmMain::delEnemy(int pos)
{
    for(uint i=0;i<projectiles.size();i++) {
        if(!projectiles[i]->del) {
            if(projectiles[i]->target==enemys[pos]) projectiles[i]->del = 2;
        }
    }
    checkPush(enemys[pos]->body);
    enemys[pos]->free();
    enemys.erase(enemys.begin()+pos);
}

void FrmMain::delTower(int pos)
{
    //mutex.lock();
    int n = towers[pos]->tnum;
    delete towers[pos];
    towers.erase(towers.begin()+pos);
    tiles[n]->towernum = -1;
    for(uint i=0;i<towers.size();i++) {
        if(tiles[towers[i]->tnum]->towernum != (int)i) {
            tiles[towers[i]->tnum]->towernum = i;
        }
    }
    //mutex.unlock();
}

void FrmMain::reset(int custom)
{
    if(custom!=2) {
        mapSaved = 0;
    }
    if(custom!=3) {
        mWidth = 0;
        mHeight = 0;
    }
    viewRect = QRect(0,0,1920,1080);
    chosenpath = 0;
    transX = 0;
    transY = 0;
    benis = 150;
    towerMenu = -1;
    currentWave = waves[0];
    waveCount = 0;
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
    moveAn = 4;
    active = 0;
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
    moveAn = 4;
    active = 0;
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
    int size;
    switch(type) {
    case 0:
        size = 20;
        break;
    case 1:
        size = 25;
        break;
    case 2:
        size = 30;
        break;
    case 3:
        size = 30;
        break;
    case 4:
        size = 40;
        break;
    case 5:
        size = 40;
        break;
    case 6:
        size = 50;
        break;
    case 7:
        size = 75;
        break;
    }
    return size;
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
            if(version!=split[1]) { //Outdated
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
    QMessageBox boxMsg;
    boxMsg.setWindowTitle("Info");
    boxMsg.setText("Neuer Pr0fense-P0st auf pr0! Willst du ihn öffnen?");
    boxMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    boxMsg.setDefaultButton(QMessageBox::Yes);
    int ret = boxMsg.exec();
    if(ret==QMessageBox::Yes) {
        QDesktopServices::openUrl(link);
    }
}

#ifdef Q_OS_ANDROID
int FrmMain::purchaseMediumMap()
{
    QInAppProduct *product = store->registeredProduct(QStringLiteral("mapeditor_medium"));
    if(!product) return -1;
    product->purchase();
    return 0;
}

int FrmMain::purchaseLargeMap()
{
    QInAppProduct *product = store->registeredProduct(QStringLiteral("mapeditor_large"));
    if(!product) return -1;
    product->purchase();
    return 0;
}
#endif
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
    if(transaction->status()==QInAppTransaction::PurchaseApproved) {
        //if(transaction->)
        if(transaction->product()->identifier()==QStringLiteral("mapeditor_medium")) {
            mediumUnlocked = true;
            //speichern
        } else if(transaction->product()->identifier()==QStringLiteral("mapeditor_large")) {
            mediumUnlocked = true;
            largeUnlocked = true;
            //speichern
        }
        transaction->finalize();
    } else if(transaction->status()==QInAppTransaction::PurchaseFailed) {
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
    if(active==2) {
        drawEditor(painter);
    }
    for(uint i=0;i<towers.size();i++) {
        if(towers[i]->pos.intersects(viewRect)) {
            drawTower(towers[i]->rect(),painter,i);
        }
    }
    for(uint i=0;i<enemys.size();i++) {
        painter.setOpacity(enemys[i]->opacity);
        painter.setBrush(Qt::red);
        switch(enemys[i]->type) {
        case 0: //fliesentischbesitzer
            painter.setBrush(fliese);
            break;
        case 1: //neuschwuchtel
            painter.setBrush(neuschwuchtel);
            break;
        case 2: //schwuchtel
            painter.setBrush(schwuchtel);
            break;
        case 3: //edlerSpender
            painter.setBrush(edlerSpender);
            break;
        case 4: //altschwuchtel
            painter.setBrush(altschwuchtel);
            break;
        case 5: //mods
            painter.setBrush(mod);
            break;
        case 6: //admins
            painter.setBrush(admin);
            break;
        case 7: //lebende legende
            painter.setBrush(legende);
            break;
        }
        painter.drawEllipse(enemys[i]->rect());
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
            painter.drawEllipse(enemys[i]->rectF());
            painter.setOpacity(fade);
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
            painter.drawRect(pos.x(),pos.y()+pos.height(),pos.width()*((double)enemys[i]->health/enemys[i]->maxHealth),10);
        }
    }
    if(towerMenu>-1) {
        painter.setOpacity(0.5*fade);
        painter.drawPixmap(tiles[towerMenu]->pos,auswahlTile,QRectF(0,0,80,80));
        painter.setOpacity(fade);
        if(tiles[towerMenu]->towernum>-1) {//wenn tower steht range anzeigen
            if(towers[tiles[towerMenu]->towernum]->hasRange) {
                int range = towers[tiles[towerMenu]->towernum]->range;
                painter.setBrush(Qt::NoBrush);
                QPen pen(Qt::red,2);
                painter.setPen(pen);
                painter.drawEllipse(tiles[towerMenu]->pos.center(),range,range);
                painter.setPen(Qt::NoPen);
            }
        }
    }
    for(uint i=0;i<projectiles.size();i++) {
        if(projectiles[i]) { //NULL abfrage
            switch(projectiles[i]->type) {
            case 0:
                switch(projectiles[i]->pxID) {
                case 0: //minus
                    painter.drawPixmap(projectiles[i]->rect,minus,QRectF(170,170,170,170));
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
            case 2:
                painter.setOpacity(projectiles[i]->opacity);
                painter.setPen(Qt::white);
                changeSize(painter,18,true);
                painter.drawText(projectiles[i]->rect.x(),projectiles[i]->rect.y(),
                                 projectiles[i]->text);
                painter.setPen(Qt::NoPen);
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
    }
}

void FrmMain::drawHUD(QPainter &painter)
{
    if(active==1) {
        drawTowerMenu(painter);
        painter.drawPixmap(0,0,100,100,pr0coinPx);
        painter.drawPixmap(5,75,90,90,baseHerzPx);
        painter.drawPixmap(1800,0,100,100,wavesPx);
        painter.setPen(Qt::white);
        changeSize(painter,56,true);
        painter.drawText(85,70,QString::number(benis));
        painter.drawText(85,140,QString::number(basehp));
        painter.drawText(QRect(1600,20,200,60),Qt::AlignRight,QString::number(waveCount));
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

    } else if(active==2) {
        drawEditorMenu(painter);
    }
}

void FrmMain::drawTowerMenu(QPainter &painter)
{
    if(towerMenu>-1) {
        painter.setOpacity(0.6*fade);
        painter.setBrush(Qt::black);
        painter.drawRect(towerMenuRect);
        if(towerMenuSelected) {
            painter.setOpacity(fade);
        }
        painter.setOpacity(fade);
        changeSize(painter,38,true);
        painter.setPen(Qt::white);
        int tnum = tiles[towerMenu]->towernum;
        if(tnum>-1) {//UpgradeMenü
            changeSize(painter,46,true);
            painter.setPen(Qt::white);
            painter.drawText(QRect(0,150,600,100),Qt::AlignCenter,towers[tnum]->name);
            changeSize(painter,36,true);
            QString o = "Schaden:";
            QString o2 = QString::number(towers[tnum]->dmg);
            QString o3;
            QString o4 = QString::number(towers[tnum]->range);
            if(towers[tnum]->type==2) {
                o = "Betäubungsdauer:";
                o2 = QString::number(towers[tnum]->stun/1000.0,'f',2)+"s";
            } else if(towers[tnum]->type==3) {
                o = "Repostdauer:";
                o2 = QString::number(towers[tnum]->repost/1000.0,'f',2)+"s";
            } else if(towers[tnum]->type==4) {
                o = "Benisanzahl:";
            } else if(towers[tnum]->type==5) {
                o = "Schaden:";
                o2 = "∞";
                //o4 = o2;
                //o3 = " - " + QString::number(towers[tnum]->reloadMax/1000.0,'f',2) + "s";
            }
            painter.drawText(QRect(10,230,450,300),Qt::AlignLeft,o+"\nReichweite:\nFeuerrate:\nProjektilgeschwindigkeit:");
            painter.drawText(QRect(450,230,140,300),Qt::AlignRight,o2+"\n"+
                             o4+"\n"+
                             QString::number(towers[tnum]->reload/1000.0,'f',2)+"s"+o3+"\n"+
                             QString::number(towers[tnum]->pspeed*10));
            //
            painter.setPen(Qt::NoPen);
            if(towers[tnum]->hasDamage) {
                if(towerMenuSelected!=1) painter.setOpacity(0.6*fade);
                painter.drawPixmap(btnDmgRect,btnDmg);
                drawBar(painter,towers[tnum],1);
            }
            if(towers[tnum]->hasRange) {
                if(towerMenuSelected!=2) {
                    painter.setOpacity(0.6*fade);
                } else painter.setOpacity(fade);
                painter.drawPixmap(btnRangeRect,btnRange);
                drawBar(painter,towers[tnum],2);
            }

            if(towers[tnum]->hasFirerate) {
                if(towerMenuSelected!=3) {
                    painter.setOpacity(0.6*fade);
                } else painter.setOpacity(fade);
            } else painter.setOpacity(fade);
            painter.drawPixmap(btnFirerateRect,btnFirerate);
            drawBar(painter,towers[tnum],3);

            if(towers[tnum]->hasProjectileSpeed) {
                if(towerMenuSelected!=4) {
                    painter.setOpacity(0.6*fade);
                } else painter.setOpacity(fade);
                painter.drawPixmap(btnSpeedRect,btnSpeed);
                drawBar(painter,towers[tnum],4);
            }

            if(towerMenuSelected!=5) {
                painter.setOpacity(0.6*fade);
            } else painter.setOpacity(fade);
            painter.drawPixmap(btnSellRect,btnSell);
            painter.setOpacity(fade);
            painter.setPen(Qt::white);
            switch(towerMenuSelected) {
            case 1: //dmg
            {
                switch(towers[tnum]->type) {
                case 1: //minus
                    painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Info:\n"
                                                                         "Erhöht den Schaden");
                    break;
                case 2: //fav
                    painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Info:\n"
                                                                         "Erhöht die Betäubungsdauer");
                    break;
                case 3: //repost
                    painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Info:\n"
                                                                         "Erhöht die Repostdauer");
                    break;
                case 4: //benis
                    painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Info:\n"
                                                                         "Erhöht die Benisanzahl");
                    break;
                }
                break;
            }
            case 2: //range
                painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Info:\n"
                                                                     "Erhöht die Reichweite");
                break;
            case 3: //feuerrate
                painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Info:\n"
                                                                     "Erhöht die Feuerrate");
                break;
            case 4: //speed
                painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Info:\n"
                                                                     "Erhöht die Geschwindigkeit\n"
                                                                     "von Projektilen");
                break;
            case 5: //sell
                painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Info:\n"
                                                                     "Tower verkaufen");
                break;
            }
            painter.setPen(Qt::NoPen);
        } else { //Baumenü
            drawTower(minusTowerRect,painter,-1,true);
            drawTower(favTowerRect,painter,-2,true);
            drawTower(repostTowerRect,painter,-3,true);
            drawTower(benisTowerRect,painter,-4,true);
            drawTower(banTowerRect,painter,-5,true);
        }
    }
}

void FrmMain::drawTower(QRect pos, QPainter &painter, int tnum, bool info)
{
    //pixelsize = 36
    bool menu=true;
    QRectF posF;
    painter.drawPixmap(pos,towerGroundPx);
    Tower *t = NULL;
    if(tnum>-1) {
        t = towers[tnum];
        posF = t->pos;
        tnum = t->type;
        menu=false;
    } else {
        tnum = abs(tnum);
    }
    painter.setPen(Qt::white);
    switch(tnum) {
    case 1: //minustower
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=minusTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(80,760,QString::number(minusTowerCost));
            painter.setPen(Qt::white);
            painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Minustower:\n"
                                                                 "Verschießt Minus auf Gegner\n"
                                                                 "und reduziert Gegner-Leben");
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.3,pos.height()*0.3);
        painter.drawPixmap(pos,towerBasePx);
        if(t) {
            if(t->currentReload>0) {
                painter.setOpacity((1-(double)t->currentReload/t->reload)*fade);
            }
        }
        painter.drawPixmap(pos,minus);
        painter.setOpacity(fade);
        break;
    case 2: //favoritentower
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=herzTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(80,760,QString::number(herzTowerCost));
            painter.setPen(Qt::white);
            painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Favoritentower:\n"
                                                                 "Verschießt Herzen auf Gegner\n"
                                                                 "und verlangsamt diese ein");
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.36,pos.height()*0.36);
        painter.drawPixmap(pos,towerFavBasePx);
        if(t) {
            if(t->currentReload>0) {
                painter.setOpacity((1-(double)t->currentReload/t->reload)*fade);
            }
        }
        painter.drawPixmap(pos,herzPx);
        painter.setOpacity(fade);
        break;
    case 3: //reposttower
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=repostTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(80,760,QString::number(repostTowerCost));
            painter.setPen(Qt::white);
            painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Reposttower:\n"
                                                                 "Markiert Gegner in Reichweite\n"
                                                                 "als Repost -> +50% Schaden");
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.36,pos.height()*0.36);
        painter.drawPixmap(pos,towerRepostBasePx);
        break;
    case 4:
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=benisTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(80,760,QString::number(benisTowerCost));
            painter.setPen(Qt::white);
            painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Benistower:\n"
                                                                 "Generiert Benis");
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(neuschwuchtel);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.36,pos.height()*0.36);
        painter.drawPixmap(pos,towerBasePx);
        painter.save();
        if(t!=NULL) {
            painter.translate(posF.center());
        } else {
            painter.translate(pos.center());
        }
        if(t!=NULL) {
            int rotation = (1-(double)t->currentReload/t->reload)*360;
            painter.rotate(rotation);
        }
        painter.setBrush(Qt::red);
        painter.drawRect(-2,-2,4,4);
        if(t!=NULL) {
            painter.translate(-posF.center().x(),-posF.center().y());
            painter.drawPixmap(posF,pr0coinPx,QRectF(0,0,512,512));
        } else {
            painter.translate(-pos.center().x(),-pos.center().y());
            painter.drawPixmap(pos,pr0coinPx);
        }
        painter.restore();
        break;
    case 5: //bantower
        if(info&&towerMenuSelected==tnum) { //towerinfo
            if(benis>=banTowerCost) {
                painter.setPen(Qt::green);
            } else {
                painter.setPen(Qt::red);
            }
            painter.drawText(80,760,QString::number(banTowerCost));
            painter.setPen(Qt::white);
            painter.drawText(QRect(25,785,550,200),Qt::AlignLeft,"Banntower:\n"
                                                                 "Bannt einen willkürlichen\n"
                                                                 "Gegner (Instakill)");
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(gebannt);
        painter.drawEllipse((QPointF)pos.center(),pos.width()*0.36,pos.height()*0.36);
        painter.drawPixmap(pos,towerBasePx);
        break;
    }
    painter.setPen(Qt::NoPen);
    if(towerMenuSelected==tnum&&menu) {
        pos.adjust(-10,-10,10,10);
        painter.drawPixmap(pos,auswahlPx);
        painter.drawPixmap(0,695,100,100,pr0coinPx);
    }
}

void FrmMain::drawBar(QPainter &painter, Tower *t, int num)
{
    switch(num) {
    case 1: //dmg
        for(int i=0;i<t->dmglvl;i++) {
            painter.drawPixmap(btnDmgRect.x()+106+(i*31),btnDmgRect.y()+9,19,67,barPx);
        }
        break;
    case 2: //range
        for(int i=0;i<t->rangelvl;i++) {
            painter.drawPixmap(btnRangeRect.x()+106+(i*31),btnRangeRect.y()+9,19,67,barPx);
        }
        break;
    case 3: //firerate
        for(int i=0;i<t->ratelvl;i++) {
            painter.drawPixmap(btnFirerateRect.x()+106+(i*31),btnFirerateRect.y()+9,19,67,barPx);
        }
        break;
    case 4: //bspeed
        for(int i=0;i<t->speedlvl;i++) {
            painter.drawPixmap(btnSpeedRect.x()+106+(i*31),btnSpeedRect.y()+9,19,67,barPx);
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
        if(subActiveSelected!=3) {
            painter.setOpacity(0.6*fade);
        } else {
            painter.setOpacity(fade);
        }
        painter.drawPixmap(levels[2].levelRect,communityPx);
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
            for(uint i=0;i<mainLevels;i++) {
                if(i+4!=subLevelSelected) painter.setOpacity(0.6*fade);
                painter.drawPixmap(mainLvlRects[i],lvlPreviews[i]);
                painter.setOpacity(fade);
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
        painter.drawPixmap(menuPlayRect,mPlayPx);
        if(subActiveSelected<4) painter.setOpacity(0.6*fade);
        painter.drawPixmap(menuDelRect,delPx);
        painter.setOpacity(fade);
        break;
    }
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
    painter.drawPixmap(btnContinueRect,btnContinue);
    painter.drawPixmap(btnBackRect,btnBack);
} //51

void FrmMain::towerMenuClicked(QRect pos)
{
    int tnum = tiles[towerMenu]->towernum;
    if(tnum>-1) {//turm steht, upgrade
        if(tnum>towers.size()-1) {
            error_string("tnum",QString::number(towers.size()),QString::number(tnum));
            return;
        } else if(!towers[tnum]) {
            error_string("tnum_null",QString::number(towers.size()),QString::number(tnum));
            return;
        }
        if(pos.intersects(btnDmgRect)&&towers[tnum]->hasDamage) { //dmg upgrade
            if(towers[tnum]->dmglvl<5) {
                if(towerMenuSelected==1) { //kaufen
                    switch(towers[tnum]->type) {
                    case 1: //minus
                    case 4: //benis
                        towers[tnum]->dmg+=10;
                        break;
                    case 2: //fav
                        towers[tnum]->stun+=1000;
                        break;
                    case 3: //repost
                        towers[tnum]->repost+=1000;
                        break;
                    }
                    towers[tnum]->dmglvl++;
                } else towerMenuSelected = 1;
            }
        } else if(pos.intersects(btnRangeRect)&&towers[tnum]->hasRange) { //range upgrade
            if(towers[tnum]->rangelvl<5) {
                if(towerMenuSelected==2) { //kaufen
                    towers[tnum]->range+=10;
                    towers[tnum]->rangelvl++;
                } else towerMenuSelected = 2;
            }
        } else if(pos.intersects(btnFirerateRect)&&towers[tnum]->hasFirerate) { //feuerrate upgrade
            if(towers[tnum]->ratelvl<5) {
                if(towerMenuSelected==3) { //kaufen
                    towers[tnum]->reload*=0.85;
                    towers[tnum]->ratelvl++;
                } else towerMenuSelected = 3;
            }
        } else if(pos.intersects(btnSpeedRect)&&towers[tnum]->hasProjectileSpeed) { //kugelgeschwindigkeit upgrade
            if(towers[tnum]->speedlvl<5) {
                if(towerMenuSelected==4) { //kaufen
                    towers[tnum]->pspeed+=1;
                    towers[tnum]->speedlvl++;
                } else towerMenuSelected = 4;
            }
        } else if(pos.intersects(btnSellRect)) { //tower verkaufen
            if(towerMenuSelected==5) {
                switch(towers[tnum]->type) {
                case 1: //minus
                    benis += minusTowerCost/2;
                    break;
                case 2: //fav
                    benis += herzTowerCost/2;
                    break;
                case 3:
                    benis += repostTowerCost/2;
                    break;
                case 4:
                    benis += benisTowerCost/2;
                    break;
                }
                delTower(tnum);
            } else towerMenuSelected = 5;
        }
    } else { //noch kein turm, turmauswahl
        if(pos.intersects(minusTowerRect)) { //Minustower
            if(towerMenuSelected==1&&benis>=minusTowerCost) { //kaufen
                buyMinusTower();
            } else {
                towerMenuSelected = 1;
            }
        } else if(pos.intersects(favTowerRect)) { //Favoritentower
            if(towerMenuSelected==2&&benis>=herzTowerCost) { //kaufen
                buyFavTower();
            } else {
                towerMenuSelected = 2;
            }
        } else if(pos.intersects(repostTowerRect)) { //Reposttower
            if(towerMenuSelected==3&&benis>=repostTowerCost) {
                buyRepostTower();
            } else {
                towerMenuSelected = 3;
            }
        } else if(pos.intersects(benisTowerRect)) { //Benistower
            if(towerMenuSelected==4&&benis>=benisTowerCost) {
                buyBenisTower();
            } else {
                towerMenuSelected = 4;
            }
        } else if(pos.intersects(banTowerRect)) { //Bantower
            if(towerMenuSelected==5&&benis>=banTowerCost) {
                buyBanTower();
            } else {
                towerMenuSelected = 5;
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
        for(uint i=0;i<mainLevels;i++) {
            if(pos.intersects(mainLvlRects[i])) {
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
            moveAn = 1;
            fdir = 0;
            QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
        }
        break;
    case 2: //eigene
        switch(subActiveSelected) {
        case 1: //16x9
            mWidth = 16;
            mHeight = 9;
            moveAn = 2;
            fdir = 0;
            QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
            break;
        case 2: //32x18
            if(!mediumUnlocked) { //kaufen
                //if(purchaseMediumMap()) QMessageBox::information(this,"Fehler","Fehler bei der Initialisierung!");
            } else {
                mWidth = 32;
                mHeight = 18;
                moveAn = 2;
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
                moveAn = 2;
                fdir = 0;
                QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
            }
            break;
        default: //eigene
            if(subActiveSelected>3) {
                mapSaved = subActiveSelected-1;
                mWidth = levels[subActiveSelected-1].width;
                mHeight = levels[subActiveSelected-1].height;
                moveAn = 3;
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
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,"Löschen?","Map wirklich löschen?",QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::Yes) {
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
        if((active == 2 && mapSaved) || active != 2) {
            backMenu = false;
            moveAn = 4;
            fdir = 0;
            QMetaObject::invokeMethod(t_animation,"start",Q_ARG(int,5));
        } else {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,"WARNUNG","Die Map wurde noch nicht gespeichert! Wirklich zurück?",QMessageBox::Yes|QMessageBox::No);
            if(reply == QMessageBox::Yes) {
                backMenu = false;
                moveAn = 4;
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
                } else {
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
}

void FrmMain::loadUserData()
{
    loadOptions();
    loadLevels();
    loadWaves();
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
        if(mapversion!=version) {
            if(!mapversion.contains(".")) {
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
            t->repost = data[10].toInt();
            t->projectilePxID = data[11].toInt();
            t->dmgprice = data[12].toInt();
            t->rateprice = data[13].toInt();
            t->rangeprice = data[14].toInt();
            t->speedprice = data[15].toInt();
            t->saveNum = data[16].toInt();
            towers.push_back(t);
            tiles[data[3].toInt()]->towernum = towers.size()-1;
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
            e->health = data[2].toInt();
            e->maxHealth = data[3].toInt();
            e->maxStun = data[4].toInt();
            e->ospeed = data[5].toDouble();
            e->pos = QRectF(data[6].toDouble(),data[7].toDouble(),data[8].toDouble(),data[9].toDouble());
            e->preHealth = data[10].toInt();// später falls projektile
            e->speed = data[11].toDouble();
            e->stunned = data[12].toInt();
            e->repost = data[13].toInt();
            e->soonBanned = data[14].toInt();
            e->type = data[15].toInt();
            e->opacity = data[16].toDouble();
            e->path = data[17].toInt();
            e->price = data[18].toInt();
            createBodies.push_back(e);
            //enemys.push_back(e);
        }
        //Turm Pointerzuweisung
        for(uint i=0;i<towers.size();i++) {
            if(towers[i]->type==5&&towers[i]->saveNum>-1) {
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
            p->saveNum = data[17].toInt();
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
        for(int i=0;i<waveSplit.size();i++) {
            currentWave.fliesen = waveSplit[0].toInt();
            currentWave.neuschwuchteln = waveSplit[1].toInt();
            currentWave.schwuchteln = waveSplit[2].toInt();
            currentWave.spender = waveSplit[3].toInt();
            currentWave.altschwuchteln = waveSplit[4].toInt();
            currentWave.mods = waveSplit[5].toInt();
            currentWave.admins = waveSplit[6].toInt();
            currentWave.legenden = waveSplit[7].toInt();
        }
        //------
        //Allgemeines Zeugs laden
        this->benis = split[7].toInt();
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
    spruch.push_back("Bruh Games>EA");
    spruch.push_back("Danke Merkel");
    spruch.push_back("dm780 bester mod");
    spruch.push_back("würde nochmal");
    spruch.push_back("deus vult");
    spruch.push_back("¯\\_(ツ)_/¯");
    spruch.push_back("(~￣ ³￣)~");
    spruch.push_back("( ͡° ͜ʖ ͡°)");
    spruch.push_back("\[T]/");
    spruch.push_back("Ach komm, das geht doch besser");
    spruch.push_back("Originaler Nichtskönner");
    spruch.push_back("Erwartung nicht erfüllt");
    spruch.push_back("ZOMG");
}

void FrmMain::loadWaves()
{
    // 8 2
    //waves.push_back(Wave(10,10,10,10,10,10,10,10));
    waves.push_back(Wave(10,0,0,0,0,0,0,0)); //Welle 1
    waves.push_back(Wave(8,4,2,0,0,0,0,0)); //Welle 2
    waves.push_back(Wave(10,4,2,2,0,0,0,0)); //Welle 3
    waves.push_back(Wave(15,4,2,0,0,0,0,0)); //Welle 4
    waves.push_back(Wave(10,5,5,1,0,0,0,0)); //Welle 5
    waves.push_back(Wave(0,0,0,0,10,1,1,0)); //Welle 6
    waves.push_back(Wave(50,10,0,0,0,0,0,1)); //Welle 7
    waves.push_back(Wave(10,5,5,5,5,2,1,0)); //Welle 7
    waves.push_back(Wave(100,50,0,0,0,0,0,0)); //Welle 8;
    waves.push_back(Wave(0,0,0,0,20,10,5,2)); //Welle 9
    waves.push_back(Wave(10,10,10,50,50,30,20,5)); //Welle 10
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
        if(towers[i]->type==5) {
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
    for(uint i=3;i<levels.size();i++) {
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
    out << QString::number(soundEnabled) << "#" << newestPost << "#";
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
            if(split[1].size()>3) {
                newestPost = split[1];
            }
        }
        file.close();
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
    if((gamePaused&&backMenu)||isGameOver||!active) return;
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
    if(active==0||(gamePaused&&backMenu)||isGameOver) return;
    int x = e->pos().x();
    int y = e->pos().y();
    if(towerMenu!=-1) {
        if(QRect(x/scaleFHDX,y/scaleFHDY,1,1).intersects(towerMenuRect)) return;
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
        moveAn = 4;
        fdir = 0;
        if(playingSpeed!=1) {
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
    case 0: //hauptmenü
        switch(subActive) {
        case 1: //spielen
            startMenuClicked(mRectRaw);
            break;
        case 2: //mapeditor
            editorMenuClicked(mRectRaw);
            break;
        }
        if(mRectRaw.intersects(btnContinueMenuRect)) {
            if(hasPlayingSave) {
                moveAn = 5;
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
            qApp->exit();
        } else if(mRectRaw.intersects(btnSoundRect)) { //sound
            if(soundEnabled) {
                soundEnabled = false;
                music->pause();
            } else {
                soundEnabled = true;
                music->play();
            }
        }
        break;
    case 1: //ingame
        if(mRectRaw.intersects(btnPlayingSpeedRect)&&!playingSpeedChanged) {
            //qDebug()<<enemys.size();
            switch(playingSpeed) {
            case 1: //normal
                //QMetaObject::invokeMethod(t_physics,"start",Q_ARG(int,5));
                QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,50));
                QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,5));
                QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,5));
                QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,50));
                playingSpeed = 2;
                break;
            case 2: //schnell
                //QMetaObject::invokeMethod(t_physics,"start",Q_ARG(int,5));
                QMetaObject::invokeMethod(t_projectile_full,"start",Q_ARG(int,100));
                QMetaObject::invokeMethod(t_projectile,"start",Q_ARG(int,10));
                QMetaObject::invokeMethod(t_main,"start",Q_ARG(int,10));
                QMetaObject::invokeMethod(t_wave,"start",Q_ARG(int,100));
                playingSpeed = 1;
                break;
            }
        } else if(towerMenu==-1||!mRectRaw.intersects(towerMenuRect)) {
            if(!mRectRaw.intersects(towerMenuRect)&&towerMenu>-1) {
                towerMenuSelected = 0;
                towerMenu = -1;
                for(uint i=0;i<tiles.size();i++) {
                    if(mRect.intersects(tiles[i]->rect())) {
                        if(tiles[i]->type==2) {//turm
                            towerMenu = i;
                            /*if(tiles[i]->towernum==-1) { //kein turm auf feld
                                tiles[i]->towernum=1;
                            }*/
                        }
                    }
                }
            } else {
                for(uint i=0;i<tiles.size();i++) {
                    if(mRect.intersects(tiles[i]->rect())) {
                        if(tiles[i]->type==2) {//turm
                            towerMenu = i;
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
    case 2: //Mapeditor
        editorClicked(mRectRaw,mRect);
        break;
    }
}

void FrmMain::keyPressEvent(QKeyEvent *e)
{
    if(e->key()==Qt::Key_Escape||e->key()==Qt::Key_Back) {
        if((active==1&&!isGameOver)||active==2) { //Im Spiel oder Editor
            if(!backMenu) {
                backMenu = true;
                gamePaused = true;
                mapPlaying = false;
            } else {
                backMenu = false;
                gamePaused = false;
            }
        } else if(!active) {
            if(subActive) {
                subActive = 0;
            } else {
                qApp->exit();
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
        if(active==1||active==2) painter.save();
        painter.scale(scaleX,scaleY);
        painter.setBrush(grau);
        painter.translate(transX-shakeX,transY-shakeY);
        painter.drawRect(viewRect);
        switch(active) {
        case 0: //menu
            //iwas blockiert den thread bei active = 1 für paar sec
            if(moveAn>0&&(moveAn<4||moveAn==5)&&fdir) {
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
        case 1: //ingame
        case 2: //mapeditor
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
            painter.setOpacity(fade+0.4);
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
    if(active==0||(gamePaused&&backMenu)||isGameOver) return;
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
