#include "tile.h"

Tile::Tile()
{
    towernum = -1;
    ran = 0;
    body = nullptr;
}

QRect Tile::rect()
{
    return QRect(pos.x(),pos.y(),pos.width(),pos.height());
}

void Tile::initPhysics(b2World *world)
{
    if(type==1||type==4||type==5) return;
    int x = pos.x();
    int y = pos.y();
    int w = pos.width();
    int h = pos.height();
    b2BodyDef def;
    def.type = b2_staticBody;
    def.position.Set(x+(w/2),y+(h/2));
    def.angle = 0;
    body = world->CreateBody(&def);
    b2PolygonShape boxShape;
    boxShape.SetAsBox(w/2,h/2);
    b2FixtureDef fixDef;
    fixDef.shape = &boxShape;
    fixDef.density = 2;
    body->CreateFixture(&fixDef);
}
