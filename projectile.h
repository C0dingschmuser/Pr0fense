#ifndef PROJECTILE_H
#define PROJECTILE_H
#include <QRectF>
#include <QDebug>
#include <QString>
#include "enemy.h"
#include "engine.h"

class Projectile
{
public:
    Projectile();
    bool isUsed=false;
    void update(bool full=false);
    QRectF rect;
    int angle;
    int dmg;
    int stun;
    int repost;
    int type = 0;
    double opacity = 1;
    double opacityDecAm = 0;
    Enemy *target = NULL;
    int saveNum = -1;
    int steps = 0;
    double vx;
    double vy;
    double vel;
    int del=false;
    int pxID=0;
    QString text;
    void init(QRectF rect, int angle, int dmg, Enemy *target, double vx, double vy, double vel=2.5, int stun=0, int repost=0, int pxID=0);
    void init2(QRectF rect, int type, int angle, double vx, double vy, double opacityDecAm, double vel=2.5, int pxID=1, double opacity=1);
    void init3(QString text, QPoint pos, int angle, double vx, double vy, double opacityDecAm, double vel=2.5, double opacity=1);
    void free();
    QString toString();
    QRectF crect();
};

#endif // PROJECTILE_H
