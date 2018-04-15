#ifndef ENEMY_H
#define ENEMY_H
#include <QRect>
#include <QDebug>
#include <QString>

class Enemy
{
public:
    Enemy();
    QRectF pos; //Koordinaten
    int stunned=0;
    int maxStun=0;
    bool isUsed=false;
    int repost = 0;
    int maxRepost = 0;
    int cpos; //Knoten von Weg
    int dir;
    int health; //Leben
    int preHealth;
    int maxHealth;
    double speed;
    double ospeed;
    void init(QRectF pos, double speed, int cpos=0, int health=100);
    void moveBy(double xv, double yv);
    void setHealth(int health);
    void setSpeed(double speed);
    void setStun(int stunAmount);
    void setRepost(int repostAmount);
    void reduceHealth(int amount);
    void free();
    QRect rect();
    QString toString();
};

#endif // ENEMY_H
