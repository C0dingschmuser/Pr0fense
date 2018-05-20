#include "objectpool.h"

ObjectPool::ObjectPool(QMutex *mutex, int projectileHeap, int enemyHeap)
{
    this->mutex = mutex;
    for(int i=0;i<projectileHeap;i++) {
        projectiles.push_back(new Projectile());
    }
    for(int i=0;i<enemyHeap;i++) {
        enemys.push_back(new Enemy());
    }
}

ObjectPool::~ObjectPool()
{
    for(uint i=0;i<projectiles.size();i++) {
        delete projectiles[i];
    }
    for(uint i=0;i<enemys.size();i++) {
        delete enemys[i];
    }
}

Projectile* ObjectPool::getProjectile()
{
    Projectile *p = NULL;
    for(uint i=0;i<projectiles.size();i++) {
        if(!projectiles[i]->isUsed) {
            p = projectiles[i];
            break;
        }
    }
    if(p==NULL) { //wenn kein freies gefunden
       //mutex->lock();
        p = new Projectile();
        projectiles.push_back(p);
        //mutex->unlock();
    }
    p->isUsed = true;
    return p;
}

Enemy* ObjectPool::getEnemy()
{
    Enemy *e = nullptr;
    for(uint i=0;i<enemys.size();i++) {
        if(!enemys[i]->isUsed) {
            e = enemys[i];
            break;
        }
    }
    if(e==nullptr) { //wenn kein freies gefunden
        //mutex->lock();
        e = new Enemy();
        enemys.push_back(e);
        //mutex->unlock();
    }
    e->isUsed = true;
    return e;
}
