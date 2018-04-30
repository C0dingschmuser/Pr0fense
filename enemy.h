#ifndef ENEMY_H
#define ENEMY_H
#include <QRect>
#include <QDebug>
#include <QString>
#include "Box2D/Box2D.h"

class Enemy
{
public:
    Enemy();
    QRectF pos; //Koordinaten
    int stunned=0;
    int maxStun=0;
    bool isUsed = false;
    int soonBanned = 0;
    bool physicsInitialized = false;
    int repost = 0;
    int maxRepost = 0;
    int cpos; //Knoten von Weg
    int dir;
    int health; //Leben
    int preHealth;
    int maxHealth;
    int type = 0;
    int path = 0;
    int price;
    double speed;
    double ospeed; //originalgeschwindigkeit
    double opacity = 0;
    b2Body *body;
    void init(QRectF pos, double speed, int type, int cpos=0, int health=100, int path=0, int price=25);
    void initPhysics(b2World *world);
    void moveBy(double xv, double yv);
    void setHealth(int health);
    void setSpeed(double speed);
    void setStun(int stunAmount);
    void setRepost(int repostAmount);
    void reduceHealth(int amount);
    void free();
    void updatePos();
    QRect rect();
    QRectF rectF(int a=0);
    QString toString();
};

#endif // ENEMY_H
