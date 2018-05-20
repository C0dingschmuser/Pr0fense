#ifndef ENEMY_H
#define ENEMY_H
#include <QRect>
#include <QDebug>
#include <QString>
#include "Box2D/Box2D.h"
#include "engine.h"

enum _enemyTypes {
    ENEMY_FLIESE =        0,
    ENEMY_NEUSCHWUCHTEL = 1,
    ENEMY_SCHWUCHTEL =    2,
    ENEMY_SPENDER =       3,
    ENEMY_ALTSCHWUCHTEL = 4,
    ENEMY_MOD =           5,
    ENEMY_ADMIN =         6,
    ENEMY_LEGENDE =       7,
    ENEMY_GEBANNT =       8,
};

class Enemy
{
public:
    Enemy();
    QRectF pos; //Koordinaten
    int stunned=0;
    int maxStun=0;
    bool isUsed = false;
    bool blocked = false;
    int newdir = 1;
    int soonBanned = 0;
    bool physicsInitialized = false;
    uint16_t ownCategoryBits;
    uint16_t collidingCategoryBits;
    int repost = 0;
    int maxRepost = 0;
    int poison = 0;
    int poisonDmg = 1;
    int maxPoison = 0;
    int cpos; //Knoten von Weg
    int dir;
    double health; //Leben
    double preHealth;
    double maxHealth;
    int type = 0;
    int path = -1;
    int animation = 0;
    int price;
    double speed;
    double newspeed;
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
    void reduceHealth(double amount);
    void free();
    void updatePos();
    QRect rect();
    QRectF rectF(int a=0);
    QRectF centerRect();
    QString toString();
};

#endif // ENEMY_H
