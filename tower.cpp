#include "tower.h"

Tower::Tower()
{

}

void Tower::addTargetDef(int target, int efficiency)
{
    targetTypes.push_back(TargetDefinition(target,efficiency));
    QRect tmp = this->pos.toRect();
    tmp.moveTo(tmp.x()+40, tmp.y()-40);
    tmp.adjust(20, 20, -20, -20);
    upgradePos = tmp;
}

bool Tower::isTargetValid(int target)
{
    bool ok = false;
    for(uint i = 0; i < targetTypes.size(); i++) {
        if(targetTypes[i].enemyType == target) {
            ok = true;
            break;
        }
    }
    return ok;
}

double Tower::getTargetEfficiency(int target)
{
    double efficiency = 0;
    for(uint i = 0; i < targetTypes.size(); i++) {
        if(targetTypes[i].enemyType == target) {
            efficiency = targetTypes[i].enemyEfficiency;
            break;
        }
    }
    return efficiency;
}

double Tower::getDmg(int target)
{
    return (this->dmg * (getTargetEfficiency(target) / 100.0));
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
            QString::number(angle) + "," +
            QString::number(angleSpeed) + "," +
            QString::number(targetAngle) + "," +
            QString::number(isShooting) + "," +
            QString::number(turnlvl) + "," +
            QString::number(price) + "," +
            QString::number(animation) + "," +
            QString::number(animationMax) + "," +
            QString::number(disabled) + "," +
            QString::number(disabledFlagged);

}

int Tower::calcFullUpgradeCost(double upgradeHighConst, double upgradeLowConst, int type, int lvl)
{
    int cost = 0;

    switch(type) {
    case 0: //full
        if(hasDamage) {
            for(int tmpDmgLvl = dmglvl; tmpDmgLvl < 3; tmpDmgLvl++) {
                cost += calcFullUpgradeCost(upgradeHighConst, upgradeLowConst, 1, tmpDmgLvl);
            }
        }

        if(hasRange) {
            for(int tmpRangeLvl = rangelvl; tmpRangeLvl < 3; tmpRangeLvl++) {
                cost += calcFullUpgradeCost(upgradeHighConst, upgradeLowConst, 2, tmpRangeLvl);
            }
        }

        if(hasFirerate) {
            for(int tmpRateLvl = ratelvl; tmpRateLvl < 3; tmpRateLvl++) {
                cost += calcFullUpgradeCost(upgradeHighConst, upgradeLowConst, 3, tmpRateLvl);
            }
        }

        if(hasProjectileSpeed) {
            for(int tmpSpeedLvl = speedlvl; tmpSpeedLvl < 3; tmpSpeedLvl++) {
                cost += calcFullUpgradeCost(upgradeHighConst, upgradeLowConst, 4, tmpSpeedLvl);
            }
        }

        if(hasTurnSpeed) {
            for(int tmpTurnLvl = turnlvl; tmpTurnLvl < 3; tmpTurnLvl++) {
                cost += calcFullUpgradeCost(upgradeHighConst, upgradeLowConst, 5, tmpTurnLvl);
            }
        }

        break;
    case 1: //dmg
        if(!lvl) lvl = dmglvl;
        cost = (price * upgradeHighConst) * (lvl+1);
        break;
    case 2: //range
        if(!lvl) lvl = rangelvl;
        cost = (price * upgradeLowConst) * (lvl+1);
        break;
    case 3: //feuerrate
        if(!lvl) lvl = ratelvl;
        cost = (price * upgradeHighConst) * (lvl+1);
        break;
    case 4: //speed
        if(!lvl) lvl = speedlvl;
        cost = (price * upgradeLowConst) * (lvl+1);
        break;
    case 5: //turnspeed
        if(!lvl) lvl = turnlvl;
        cost = (price * upgradeHighConst) * (lvl+1);
        break;
    }
    return cost;
}

QString Tower::getInfo(int type, int pos, bool rem)
{
    QString ret = "";
    switch(type) {
    case TOWER_MINUS:
        if(!pos) {
            ret = "Minustower:";
        } else {
            ret = "\n"
                  "Verursacht normalen \n"
                  "Schaden an einem Gegner";
        }
        break;
    case TOWER_HERZ:
        if(!pos) {
            ret = "Favoritentower:";
        } else {
            ret = "\n"
                  "Verschießt Herzen auf Gegner \n"
                  "und verlangsamt diese";
        }
        break;
    case TOWER_REPOST:
        if(!pos) {
            ret = "Reposttower:";
        } else {
            ret = "\n"
                  "Markiert Gegner in Reichweite \n"
                  "als Repost -> +50% Schaden";
        }
        break;
    case TOWER_BENIS:
        if(!pos) {
            ret = "Benistower:";
        } else {
            ret = "\n"
                  "Generiert Benis";
        }
        break;
    case TOWER_BAN:
        if(!pos) {
            ret = "Banntower:";
        } else {
            ret = "\n"
                  "Bannt einen willkürlichen \n"
                  "Gegner (Instakill)";
        }
        break;
    case TOWER_SNIPER:
        if(!pos) {
            ret = "Snipertower:";
        } else {
            ret = "\n"
                  "Verursacht viel Schaden \n"
                  "an einem Gegner";
        }
        break;
    case TOWER_FLAK:
        if(!pos) {
            ret = "Flaktower:";
        } else {
            ret = "\n"
                  "Greift Fluggegner an";
        }
    break;
    case TOWER_LASER:
        if(!pos) {
            ret = "Lasertower:";
        } else {
            ret = "\n"
                  "Verschießt einen Laser, der kon\n"
                  "tinuierlich Schaden macht";
        }
    break;
    case TOWER_POISON:
        if(!pos) {
            ret = "Gifttower:";
        } else {
            ret = "\n"
                  "Verschießt Giftgas";
        }
    break;
    case TOWER_MINIGUN:
        if(!pos) {
            ret = "Miniguntower:";
        } else {
            ret = "\n"
                  "Hohe Feuerrate, dafür \n"
                  "wenig Schaden";
        }
    break;
    }
    if(rem) ret.remove("\n");
    return ret;
}
