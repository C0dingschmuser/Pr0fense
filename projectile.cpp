#include "projectile.h"

Projectile::Projectile()
{
    isUsed = false;
}

void Projectile::update(bool full)
{
    if(full && !type) {
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
        } else if(!target->soonBanned&&steps) {
            this->angle = newAngle;
        }
        steps++;
        vx = qCos(qDegreesToRadians((double)this->angle));
        vy = qSin(qDegreesToRadians((double)this->angle));
    } else {
        if(type != 6) {
            double velocity = vel;
            if(type == 5 || type == 7) {
                steps++;
                velocity = (vel/40)*steps;
            }
            rect.moveTo(rect.x()+vx*velocity,rect.y()+vy*velocity);
        } else {
            rect.adjust(-vel,-vel,vel,vel);
        }
    }
    if(type && opacity) {
        opacity -= opacityDecAm;
        if(opacity<0) opacity = 0;
    }
}

double Projectile::getTargetEfficiency(int target)
{
    double efficiency = 0;
    for(uint i = 0; i < targetDefinitions.size(); i++) {
        if(targetDefinitions[i].enemyType == target) {
            efficiency = targetDefinitions[i].enemyEfficiency;
            break;
        }
    }
    return efficiency;
}

void Projectile::init(QRectF rect, int angle, double dmg, Enemy *target, double vx, double vy, double vel, int stun, int repost, int pxID)
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
    this->color = QColor(255,255,255);
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

void Projectile::init_arc(QRectF rect, double vel, double opacity, double opacityDecAm, QColor color, int width)
{
    isUsed = true;
    del = false;
    steps = 0;
    this->color = color;
    this->rect = rect;
    this->angle = width;
    this->opacity = opacity;
    this->opacityDecAm = opacityDecAm;
    this->vel = vel;
    this->type = 6;
}

void Projectile::free()
{
    rect = QRectF(-100,-100,10,10);
    target = NULL;
    poisonDmg = 1;
    this->color = QColor(1,1,1,1);
    this->vx = 0;
    this->vy = 0;
    this->isUsed = false;
}

QString Projectile::toString()
{
    return Engine::rectFToString(rect) + "," +
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
            QString::number(pxID) + "," +
            QString::number(hasShekelImage) + "," +
            QString::number(color.red()) + "," + QString::number(color.green()) +
                "," + QString::number(color.blue()) + "," + QString::number(color.alpha()) + "," +
            QString::number(poisonDmg) + "," +
            Engine::rectFToString(targetRect);

}

QRectF Projectile::crect()
{
    return QRect(rect.center().x(),rect.center().y(),1,1);
}

