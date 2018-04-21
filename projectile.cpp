#include "projectile.h"

Projectile::Projectile()
{
    isUsed = false;
}

void Projectile::update(bool full)
{
    if(full&&!type) {
        double angle = this->angle;
        int newAngle = Engine::getAngle(crect().center(),target->rect().center());
        if(target->soonBanned&&steps>3) {
            if(vel<2.5) {
                vel += 0.5;
            } else vel = 2.5;
            this->angle = newAngle;
        } else if(target->soonBanned&&steps<3) {
            if(newAngle>angle) {
                int diff = newAngle - angle;
                angle += (diff*steps) / 3;
            } else if(angle>newAngle) {
                int diff = angle - newAngle;
                angle -= (diff*steps) / 3;
            }
            this->angle = angle;
        } else if(!target->soonBanned) {
            this->angle = newAngle;
        }
        steps++;
        vx = qCos(qDegreesToRadians((double)this->angle));
        vy = qSin(qDegreesToRadians((double)this->angle));
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
    steps = 0;
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
    steps = 0;
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
    steps = 0;
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

QString Projectile::toString()
{
    return QString::number(rect.x()) + "," + QString::number(rect.y()) + "," + QString::number(rect.width()) + "," + QString::number(rect.height()) + "," +
            QString::number(angle) + "," +
            QString::number(dmg) + "," +
            QString::number(stun) + "," +
            QString::number(repost) + "," +
            QString::number(type) + "," +
            QString::number(opacity) + "," +
            QString::number(opacityDecAm) + "," +
            QString::number(vx) + "," +
            QString::number(vy) + "," +
            QString::number(vel) + "," +
            QString::number(del) + "," +
            text + "," +
            QString::number(pxID);

}

QRectF Projectile::crect()
{
    return QRect(rect.center().x(),rect.center().y(),1,1);
}

