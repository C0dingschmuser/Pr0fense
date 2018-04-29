#include "enemy.h"

Enemy::Enemy()
{
    isUsed = false;
    body = nullptr;
    physicsInitialized = false;
}

void Enemy::init(QRectF pos, double speed, int type, int cpos, int health, int path, int price)
{
    stunned = 0;
    maxStun = 0;
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
    this->repost = 0;
    this->type = type;
    this->path = path;
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
    circleShape.m_p.Set(0,0);
    circleShape.m_radius = pos.width()/2;
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 0;
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

void Enemy::reduceHealth(int amount)
{
    if(repost) amount *= 1.5;
    health -= amount;
    if(health<0) health = 0;
}

void Enemy::free()
{
    body = nullptr;
    pos = QRectF();
    cpos = 0;
    health = 0;
    isUsed = false;
    repost = 0;
    maxRepost = 0;
}

void Enemy::updatePos()
{
    if(!physicsInitialized) return;
    this->pos = QRectF(body->GetPosition().x-pos.width()/2,body->GetPosition().y-pos.height()/2,pos.width(),pos.height());
}

QRect Enemy::rect()
{
    //return QRect(pos.x(),pos.y(),pos.width(),pos.height());
    return QRect(body->GetPosition().x-pos.width()/2,body->GetPosition().y-pos.height()/2,pos.width(),pos.height());
}

QRectF Enemy::rectF(int a)
{
    Q_UNUSED(a);
    if(!a) {
        return QRectF(body->GetPosition().x-pos.width()/2,body->GetPosition().y-pos.height()/2,pos.width(),pos.height());
    } else {
        return QRectF(body->GetPosition().x,body->GetPosition().y,pos.width(),pos.height());
    }
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
            QString::number(price);
}
