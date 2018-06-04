#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H
#include <QMutex>
#include <QSharedPointer>
#include "projectile.h"
#include "enemy.h"

class ObjectPool
{
public:
    ObjectPool(QMutex *mutex, int projectileHeap=512, int enemyHeap=256);
    ~ObjectPool();
    Projectile* getProjectile();
    Enemy* getEnemy();



private:
    QMutex *mutex;
    std::vector <Enemy*> enemys;
    std::vector <Projectile*> projectiles;
};

#endif // OBJECTPOOL_H
