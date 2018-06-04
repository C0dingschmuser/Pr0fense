#include "enemy.h"

Enemy::Enemy()
{
    isUsed = false;
    body = nullptr;
    physicsInitialized = false;
    newspeed = 0;
}

void Enemy::init(QRectF pos, double speed, int type, int cpos, int health, int path, int price)
{
    soonBanned = false;
    physicsInitialized = false;
    this->price = price;
    this->pos = pos;
    this->cpos = cpos;
    this->health = health;
    this->maxHealth = health;
    this->preHealth = health;
    this->speed = speed;
    this->ospeed = speed;
    this->type = type;
    this->path = path;
    newspeed = 0;
    isUsed = true;
    dir = 1; //rechts
}

void Enemy::initPhysics(b2World *world)
{
    b2BodyDef BodyDef;
    BodyDef.type = b2_dynamicBody;
    BodyDef.position.Set(pos.x(),pos.y());
    BodyDef.angle = 0;
    body = world->CreateBody(&BodyDef);
    b2CircleShape circleShape;
    b2PolygonShape polyShape;
    b2FixtureDef fixtureDef;
    //if(type != 7) { //wenn nicht legende
        circleShape.m_p.Set(0,0);
        circleShape.m_radius = pos.width() / 2;
        fixtureDef.shape = &circleShape;
    /*} else if( type == 7){
        QPolygonF polygon = Engine::getRauteFromRect(pos);
        b2Vec2 array[4];
        for(uint i = 0; i < 4; i++) {
            array[i].Set(-(pos.x() + pos.width()/2) + polygon.at(i).x(), -(pos.y() + pos.height()/2) + polygon.at(i).y());
        }
        polyShape.Set(array, 4);
        fixtureDef.shape = &polyShape;
    }*/
    fixtureDef.density = getDensity(type);
    fixtureDef.filter.categoryBits = ownCategoryBits;
    fixtureDef.filter.maskBits = collidingCategoryBits;
    body->CreateFixture(&fixtureDef);
    updatePos();
    physicsInitialized = true;
}

void Enemy::moveBy(double xv, double yv)
{
    if(!physicsInitialized) return;
    //qDebug()<<body->GetWorld()->IsLocked();
    body->SetLinearVelocity(b2Vec2(xv*10,yv*10));
    updatePos();
    if(opacity<1) opacity += 0.0075;
}

void Enemy::setHealth(int health)
{
    this->health = health;
    this->maxHealth = health;
    this->preHealth = health;
}

void Enemy::setSpeed(double speed)
{
    this->speed = speed;
    this->ospeed = speed;
}

void Enemy::setStun(int stunAmount)
{
    this->stunned += stunAmount;
    this->maxStun = stunned;
}

void Enemy::setRepost(int repostAmount)
{
    this->repost += repostAmount;
    this->maxRepost = repostAmount;
}

void Enemy::reduceHealth(double amount)
{
    if(repost && !soonBanned && !poison) amount *= 1.5;
    health -= amount;
    updateWidth(0);
    flash = 10;
    if(health<0) health = 0;
}

void Enemy::free()
{
    flash = 0;
    blocked = false;
    body = nullptr;
    pos = QRectF();
    cpos = 0;
    health = 0;
    newspeed = 0;
    path = -1;
    isUsed = false;
    stunned = 0;
    maxStun = 0;
    repost = 0;
    maxRepost = 0;
    maxPoison = 0;
    poison = 0;
    poisonDmg = 1;
    animation = 0;
}

void Enemy::updatePos()
{
    if(!physicsInitialized||body==nullptr) return;
    this->pos = QRectF(body->GetPosition().x-pos.width()/2,body->GetPosition().y-pos.height()/2,pos.width(),pos.height());
}

void Enemy::calcWidth()
{
    healthWidth = this->rectF().width()*((double)health/maxHealth);
    if(healthWidth < 1) healthWidth = 1;
    healthWidthTarget = healthWidth;

}

void Enemy::updateWidth(double amount)
{
    if(health < 1) {
        healthWidth = 1;
        healthWidthTarget = 1;
    } else {
        healthWidthTarget = this->rectF().width()*((double)health/maxHealth);
        if(healthWidthTarget < 1) healthWidthTarget = 1;
        if(amount > 0) healthWidth -= amount;
        if(healthWidth <= healthWidthTarget) {
            healthWidth = healthWidthTarget;
        }
    }
}

int Enemy::getDensity(int type)
{
    int density = 0;
    switch(type) {
    case ENEMY_FLIESE:
        density = 1;
        break;
    case ENEMY_GEBANNT:
        density = 1;
        break;
    case ENEMY_NEUSCHWUCHTEL:
        density = 2;
        break;
    case ENEMY_SCHWUCHTEL:
        density = 3;
        break;
    case ENEMY_SPENDER:
        density = 3;
        break;
    case ENEMY_ALTSCHWUCHTEL:
        density = 5;
        break;
    case ENEMY_MOD:
        density = 10;
        break;
    case ENEMY_ADMIN:
        density = 12;
        break;
    case ENEMY_LEGENDE:
        density = 20;
        break;
    }
    //density = 0;
    return density;
}

QRect Enemy::rect()
{
    //return QRect(pos.x(),pos.y(),pos.width(),pos.height());
    if(body==nullptr||!physicsInitialized) {
        return QRect(pos.x(),pos.y(),pos.width(),pos.height());
    }
    return QRect(body->GetPosition().x-pos.width()/2,body->GetPosition().y-pos.height()/2,pos.width(),pos.height());
}

QRectF Enemy::rectF(int a)
{
    if(!physicsInitialized||body==nullptr) {
        if(!a) {
            return pos;
        } else {
            return QRectF(pos.x()+pos.width()/2,pos.y()+pos.height()/2,pos.width(),pos.height());
        }
    } else {
        if(!a) {
            return QRectF(body->GetPosition().x-pos.width()/2,body->GetPosition().y-pos.height()/2,pos.width(),pos.height());
        } else {
            return QRectF(body->GetPosition().x,body->GetPosition().y,pos.width(),pos.height());
        }
    }
}

QRectF Enemy::centerRect()
{
    pos = rectF();
    QRectF center = QRectF(pos.center().x() - 1, pos.center().y() - 1, 2, 2);
    return center;
}

QString Enemy::toString()
{
    pos = rectF(1);
    return QString::number(cpos) + "," +
            QString::number(dir) + "," +
            QString::number(health) + "," +
            QString::number(maxHealth) + "," +
            QString::number(maxStun) + "," +
            QString::number(ospeed) + "," +
            QString::number(pos.x()) + "," + QString::number(pos.y()) + "," + QString::number(pos.width()) + "," + QString::number(pos.height()) + "," +
            QString::number(preHealth) + "," +
            QString::number(speed) + "," +
            QString::number(stunned) + "," +
            QString::number(repost) + "," +
            QString::number(soonBanned) + "," +
            QString::number(type) + "," +
            QString::number(opacity) + "," +
            QString::number(path) + "," +
            QString::number(price) + "," +
            QString::number(animation) + "," +
            QString::number(poison) + "," +
            QString::number(maxPoison) + "," +
            QString::number(poisonDmg) + "," +
            QString::number(newdir) + "," +
            QString::number(blocked) + "," +
            QString::number(reload) + "," +
            QString::number(maxReload) + "," +
            QString::number(imgID);
}
