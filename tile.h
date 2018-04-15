#ifndef TILE_H
#define TILE_H
#include <QRectF>

class Tile
{
public:
    Tile();
    QRectF pos;
    int type;
    int towernum;
    int ran;
    QRect rect();

};

#endif // TILE_H
