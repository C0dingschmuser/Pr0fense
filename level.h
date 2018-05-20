#ifndef LEVEL_H
#define LEVEL_H
#include <QPixmap>
#include <QString>
#include <QRect>
#include "path.h"

class Level
{
public:
    Level();
    std::vector <Path> paths;
    std::vector <int> map;
    QPixmap lvlImage;
    QRect levelRect;
    bool used = false;
    bool unlocked = false;
    int price = 0;
    int width = 0;
    int height = 0;
    QString lvlName;
    QString getMapString();
    int getPathNum(int start);
};

#endif // LEVEL_H
