#ifndef WAVE_H
#define WAVE_H
#include <QString>
#include <QDebug>
#include "engine.h"
#include "enemy.h"

class Wave
{
public:
    Wave();
    Wave(int fl, int nsw, int sw, int s, int alt, int m, int a, int l, int g);
    int enemysPerWave = 2;
    int maxEnemysPerWave = 2;
    int waveCount = 0;
    int waveTime = 0;
    int shekel = 0;
    int fliesen, neuschwuchteln, schwuchteln, spender, altschwuchteln, mods, admins, legenden, gebannte;
    bool empty = false;
    QString toString();
    int generateEnemyType(bool hasFlak = false);
    int calcShekel();
    void updateEnemysPerWave(bool fullTime, int mapWidth);
    int startNextWave(int width);
    void resetWave(bool full = false);
    void addEnemy(int type);
};

#endif // WAVE_H
