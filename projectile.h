#ifndef PROJECTILE_H
#define PROJECTILE_H
#include <QRectF>
#include <QDebug>
#include <QString>
#include <QColor>
#include "enemy.h"
#include "engine.h"
#include "targetdefinition.h"

class Projectile
{
public:
    Projectile();
    std::vector <TargetDefinition> targetDefinitions;
    bool isUsed=false;
    void update(bool full = false);
    QRectF rect;
    QRectF targetRect;
    int angle;
    double dmg;
    int stun;
    int repost;
    int type = 0;
    bool hasShekelImage = false;
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
    int poisonDmg = 1;
    QString text;
    QColor color = QColor(1,1,1,1);
    double getTargetEfficiency(int target);
    void init(QRectF rect, int angle, double dmg, Enemy *target, double vx, double vy, double vel=2.5, int stun=0, int repost=0, int pxID=0);
    void init2(QRectF rect, int type, int angle, double vx, double vy, double opacityDecAm, double vel=2.5, int pxID=1, double opacity=1);
    void init3(QString text, QPoint pos, int angle, double vx, double vy, double opacityDecAm, double vel=2.5, double opacity=1);
    void init_arc(QRectF rect, double vel, double opacity, double opacityDecAm, QColor color, int width);
    void free();
    QString toString();
    QRectF crect();
};

#endif // PROJECTILE_H
