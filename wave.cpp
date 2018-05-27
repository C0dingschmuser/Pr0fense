#include "wave.h"

Wave::Wave()
{
    waveCount = 0;
    enemysPerWave = 0;
    maxEnemysPerWave = 0;
    waveTime = 0;
}

QString Wave::toString()
{
    return QString::number(fliesen) + "," +
            QString::number(neuschwuchteln) + "," +
            QString::number(schwuchteln) + "," +
            QString::number(spender) + "," +
            QString::number(altschwuchteln) + "," +
            QString::number(mods) + "," +
            QString::number(admins) + "," +
            QString::number(legenden) + "," +
            QString::number(gebannte) + "," +
            QString::number(waveCount) + "," +
            QString::number(maxEnemysPerWave) + "," +
            QString::number(enemysPerWave) + "," +
            QString::number(waveTime);
}

int Wave::generateEnemyType(bool hasFlak)
{
    int max = 0;
    for(int i = 0; i < waveCount && i < 36; i += 4) {
        max++;
    }
    bool save = false;
    int enemyType = 0;
    if(waveCount) {
        if(hasFlak) {
            if((waveCount % 4 == 0 && waveCount > 10) ||
                    (waveCount % 6 == 0 && waveCount < 10)) {
                if(gebannte < (maxEnemysPerWave / 4)) {
                    enemyType = ENEMY_GEBANNT;
                    save = true;
                }
            }
        }
        if(waveCount % 10 == 0 && waveCount && max >= ENEMY_MOD && enemysPerWave <= 11) {
            //Bosswave
            if(!legenden) {
                enemyType = ENEMY_LEGENDE;
                save = true;
            }
            if(enemysPerWave > 1) {
                if(admins < 3) {
                    enemyType = ENEMY_ADMIN;
                    save = true;
                }
                if(mods < 7) {
                    enemyType = ENEMY_MOD;
                    save = true;
                }
            }
        }
    }
    if(!save) { //Gegner wurde noch nicht festgelegt
        int num = Engine::random(0,100);
        if(num < 50 || max <= 1) {
            enemyType = ENEMY_FLIESE;
        } else if(num < 60 || max <= 2) {
            enemyType = ENEMY_NEUSCHWUCHTEL;
        } else if(num < 80 || max <= 3) {
            enemyType = ENEMY_SCHWUCHTEL;
        } else if(num < 90 || max <= 4) {
            enemyType = ENEMY_SPENDER;
        } else if(num < 100 || max <= 5) {
            enemyType = ENEMY_ALTSCHWUCHTEL;
        }
    }
    /*if(legenden) {
        enemyType = ENEMY_FLIESE;
    } else enemyType = ENEMY_LEGENDE;*/
    return enemyType;
}

int Wave::calcShekel()
{
    int shekel = 0;
    if(waveCount) {
        if(waveCount % 10 == 0) {
            shekel = 10;
            if(waveCount > 50) shekel = 20;
            if(waveCount > 80) shekel = 30;
            if(waveCount > 120) shekel = 40;
        }
    }
    return shekel;
}

void Wave::updateEnemysPerWave(bool fullTime, int mapWidth)
{
    int time;
    switch(mapWidth) {
    case 16:
        time = 15000;
        break;
    case 32:
        time = 30000;
        break;
    case 64:
        time = 45000;
        break;
    }
    int wTime = waveTime;
    if(fullTime) {
        wTime = time;
    }
    int rsp = 2;
    if(waveCount) {
        rsp = waveCount+1 * (time / wTime);
        if(rsp < 1) rsp = 1;
    }
    enemysPerWave = rsp;
    maxEnemysPerWave = enemysPerWave;
}

int Wave::startNextWave(int width)
{
    int shekel = calcShekel();
    waveCount++;
    updateEnemysPerWave(false, width);
    resetWave();
    return shekel;
}

void Wave::resetWave(bool full)
{
    if(full) {
        waveCount = 0;
    }
    waveTime = 0;
    fliesen = 0;
    neuschwuchteln = 0;
    schwuchteln = 0;
    spender = 0;
    altschwuchteln = 0;
    mods = 0;
    admins = 0;
    legenden = 0;
    gebannte = 0;
}

void Wave::addEnemy(int type)
{
    switch(type) {
    case ENEMY_FLIESE:
        fliesen++;
        break;
    case ENEMY_NEUSCHWUCHTEL:
        neuschwuchteln++;
        break;
    case ENEMY_SCHWUCHTEL:
        schwuchteln++;
        break;
    case ENEMY_SPENDER:
        spender++;
        break;
    case ENEMY_ALTSCHWUCHTEL:
        altschwuchteln++;
        break;
    case ENEMY_MOD:
        mods++;
        break;
    case ENEMY_ADMIN:
        admins++;
        break;
    case ENEMY_LEGENDE:
        legenden++;
        break;
    case ENEMY_GEBANNT:
        gebannte++;
        break;
    }
}
