#ifndef TOWER_H
#define TOWER_H
#include <QRect>
#include <QString>
#include "enemy.h"
#include "targetdefinition.h"

enum _towerTypes {
    TOWER_MINUS =    1,
    TOWER_HERZ =     2,
    TOWER_REPOST =   3,
    TOWER_BENIS =    4,
    TOWER_BAN =      5,
    TOWER_SNIPER =   6,
    TOWER_FLAK =     7,
    TOWER_LASER =    8,
    TOWER_POISON =   9,
    TOWER_MINIGUN = 10,
};

class Tower
{
public:
    Tower();
    QRectF pos;
    QRect upgradePos;
    bool hasDamage = true;
    bool hasRange = true;
    bool hasFirerate = true;
    bool hasProjectileSpeed = true;
    bool hasTurnSpeed = true;
    int shotsFiredSinceReload = 0;
    Enemy *target = nullptr;
    int saveNum = -1;
    int type = 0;
    int side = 0;
    int reload = 0;
    int reloadMax = 0;
    int projectilePxID = 0;
    int currentReload = 0;
    bool isShooting = false;
    bool sellingDisabled = false;
    bool disabledFlagged = false;
    int animation = 0;
    int animationMax = 0;
    double pspeed = 0;
    double dmg = 0;
    int disabled = 0;
    int stun = 0;
    int repost = 0;
    int range = 0;
    int tmpRange = 0;
    int tmpRangeDir = 2;
    int price = 0;
    int dmglvl = 1;
    int ratelvl = 1;
    int rangelvl = 1;
    int speedlvl = 1;
    int turnlvl = 1;
    double angle = 0;
    double targetAngle = 0;
    double angleSpeed = 0;
    int tnum = -1;
    std::vector <TargetDefinition> targetTypes;
    //std::vector <int>
    void addTargetDef(int target, int efficiency);
    bool isTargetValid(int target);
    double getTargetEfficiency(int target);
    double getDmg(int target);
    QString name;
    QRect rect();
    QString toString();
    int calcFullUpgradeCost(double upgradeHighConst, double upgradeLowConst, int type = 0, int lvl = 0);
    static QString getInfo(int type, int pos, bool rem = false);
};

#endif // TOWER_H
