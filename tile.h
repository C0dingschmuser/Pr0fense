#ifndef TILE_H
#define TILE_H
#include <QRectF>
#include "Box2D/Box2D.h"
#include "tower.h"
#include "engine.h"

enum _entityCategory {
    BOUNDARY =      0x0001,
    NORMAL_ENEMY =  0x0002,
    FLYING_ENEMY =  0x0004,
    WALL =          0x0005,
};

class Tile
{
public:
    Tile();
    ~Tile();
    QRectF pos;
    b2Body *body = nullptr;
    uint16_t boxType = BOUNDARY;
    int type;
    int wallTime = 0;
    Tower *t = nullptr;
    int towernum = -1;
    int ran = 0;
    int animation = 0;
    bool anDir = 0;
    double opacity = 0;
    QRect rect();
    void initPhysics(b2World *world);

};

#endif // TILE_H
