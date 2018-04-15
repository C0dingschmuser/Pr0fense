#include "projectile.h"

Projectile::Projectile()
{

}

void Projectile::update(bool full)
{
    if(full&&!type) {
        double angle = Engine::getAngle(crect().center(),target->rect().center());
        vx = qCos(qDegreesToRadians((double)angle));
        vy = qSin(qDegreesToRadians((double)angle));
    } else {
        rect.moveTo(rect.x()+vx*vel,rect.y()+vy*vel);
    }
    if(type&&opacity) {
        opacity -= opacityDecAm;
        if(opacity<0) opacity = 0;
    }
}

void Projectile::init(QRectF rect, int angle, int dmg, Enemy *target, double vx, double vy, double vel, int stun, int repost, int pxID)
{
    isUsed = true;
    del = false;
    this->type = 0;
    this->rect = rect;
    this->angle = angle;
    this->dmg = dmg;
    this->target = target;
    this->vx = vx;
    this->vy = vy;
    this->vel = vel;
    this->stun = stun;
    this->repost = repost;
    this->pxID = pxID;
    this->opacity = 1;
}

void Projectile::init2(QRectF rect, int type, int angle, double vx, double vy, double opacityDecAm, double vel, int pxID, double opacity)
{

    isUsed = true;
    del = false;
    this->pxID = pxID;
    this->rect = rect;
    this->type = type;
    this->angle = angle;
    this->vx = vx;
    this->vy = vy;
    this->opacityDecAm = opacityDecAm;
    this->vel = vel;
    this->opacity = opacity;
}

void Projectile::init3(QString text, QPoint pos, int angle, double vx, double vy, double opacityDecAm, double vel, double opacity)
{
    isUsed = true;
    del = false;
    this->text = text;
    this->rect = QRectF(pos.x(),pos.y(),1,1);
    this->type = 2;
    this->angle = angle;
    this->vx = vx;
    this->vy = vy;
    this->opacityDecAm = opacityDecAm;
    this->vel = vel;
    this->opacity = opacity;
}

void Projectile::free()
{
    rect = QRectF();
    target = NULL;
    this->vx = 0;
    this->vy = 0;
    this->isUsed = false;
}

QRectF Projectile::crect()
{
    return QRect(rect.center().x(),rect.center().y(),1,1);
}

