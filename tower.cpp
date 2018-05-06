#include "tower.h"

Tower::Tower()
{

}

QRect Tower::rect()
{
    return QRect(pos.x(),pos.y(),pos.width(),pos.height());
}

QString Tower::toString()
{
    return QString::number(type) + "," +
            QString::number(dmg) + "," +
            QString::number(stun) + "," +
            QString::number(tnum) + "," +
            QString::number(reload) + "," +
            QString::number(range) + "," +
            QString::number(pspeed) + "," +
            QString::number(dmglvl) + "," +
            QString::number(ratelvl) + "," +
            QString::number(speedlvl) + "," +
            QString::number(rangelvl) + "," +
            QString::number(repost) + "," +
            QString::number(projectilePxID) + "," +
            QString::number(dmgprice) + "," +
            QString::number(rateprice) + "," +
            QString::number(rangeprice) + "," +
            QString::number(speedprice) + "," +
            QString::number(angle) + "," +
            QString::number(angleSpeed) + "," +
            QString::number(targetAngle);

}
