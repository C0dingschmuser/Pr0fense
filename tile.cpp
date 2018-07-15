#include "tile.h"

Tile::Tile()
{

}

Tile::~Tile()
{

}

QRect Tile::rect()
{
    return QRect(pos.x(),pos.y(),pos.width(),pos.height());
}

void Tile::initPhysics(b2World *world)
{
    if(type==4||type==5) return;
    int x = pos.x();
    int y = pos.y();
    int w = pos.width();
    int h = pos.height();
    b2BodyDef def;
    def.type = b2_staticBody;
    def.position.Set(Engine::numToBox2D(x+(w/2)),Engine::numToBox2D(y+(h/2)));
    def.angle = 0;
    body = world->CreateBody(&def);
    b2PolygonShape boxShape;
    boxShape.SetAsBox(Engine::numToBox2D(w/2),Engine::numToBox2D(h/2));
    b2FixtureDef fixDef;
    fixDef.shape = &boxShape;
    fixDef.density = 2;
    fixDef.filter.categoryBits = boxType;
    body->CreateFixture(&fixDef);
}
