#include "tile.h"

Tile::Tile()
{
    towernum = -1;
    ran = 0;
}

QRect Tile::rect()
{
    return QRect(pos.x(),pos.y(),pos.width(),pos.height());
}
