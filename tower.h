#ifndef TOWER_H
#define TOWER_H
#include <QRect>
#include <QString>

class Tower
{
public:
    Tower();
    QRectF pos;
    int type = 0;
    int reload = 0;
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
    int tnum = -1;
    QString name;
    QRect rect();
    QString toString();
};

#endif // TOWER_H
