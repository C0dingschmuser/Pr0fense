#include "enemy.h"

Enemy::Enemy()
{

}

void Enemy::init(QRectF pos, double speed, int cpos, int health)
{
    stunned = 0;
    maxStun = 0;
    this->pos = pos;
    this->cpos = cpos;
    this->health = health;
    this->maxHealth = health;
    this->preHealth = health;
    this->speed = speed;
    this->ospeed = speed;
    isUsed = true;
    speed = 1;
    dir = 1; //rechts
}

void Enemy::moveBy(double xv, double yv)
{
    pos.moveTo(pos.x()+xv,pos.y()+yv);
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
    pos = QRectF();
    cpos = 0;
    health = 0;
    isUsed = false;
    repost = 0;
    maxRepost = 0;
}

QRect Enemy::rect()
{
    return QRect(pos.x(),pos.y(),pos.width(),pos.height());
}

QString Enemy::toString()
{
    return QString::number(cpos) + "," +
            QString::number(dir) + "," +
            QString::number(health) + "," +
            QString::number(maxHealth) + "," +
            QString::number(maxStun) + "," +
            QString::number(ospeed) + "," +
            QString::number(pos.x()) + "," + QString::number(pos.y()) + "," + QString::number(pos.width()) + "," + QString::number(pos.height()) + "," +
            QString::number(preHealth) + "," +
            QString::number(speed) + "," +
            QString::number(stunned);
}
