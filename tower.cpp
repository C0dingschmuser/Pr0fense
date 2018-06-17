#include "tower.h"

Tower::Tower()
{

}

void Tower::addTargetDef(int target, int efficiency)
{
    targetTypes.push_back(TargetDefinition(target,efficiency));
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
