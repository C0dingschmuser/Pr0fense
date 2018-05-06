#ifndef TOWER_H
#define TOWER_H
#include <QRect>
#include <QString>
#include "enemy.h"

class Tower
{
public:
    Tower();
    QRectF pos;
    bool hasDamage = true;
    bool hasRange = true;
    bool hasFirerate = true;
    bool hasProjectileSpeed = true;
    int shotsFiredSinceReload = 0;
    Enemy *target = nullptr;
    int saveNum = -1;
    int type = 0;
    int side = 0;
    int reload = 0;
    int reloadMax = 0;
    int projectilePxID = 0;
    int currentReload = 0;
    int animation = 0;
    double pspeed = 0;
    int dmg = 0;
    int stun = 0;
    int repost = 0;
    int range = 0;
    int price = 0;
    int dmglvl = 1;
    int ratelvl = 1;
    int rangelvl = 1;
    int speedlvl = 1;
    double angle = 0;
    double targetAngle = 0;
    double angleSpeed = 0;
    int dmgprice;
    int rateprice;
    int rangeprice;
    int speedprice;
    int tnum = -1;
    QString name;
    QRect rect();
    QString toString();
};

#endif // TOWER_H
