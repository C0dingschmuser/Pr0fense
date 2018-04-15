#ifndef LEVEL_H
#define LEVEL_H
#include <QPixmap>
#include <QString>
#include <QRect>

class Level
{
public:
    Level();
    std::vector <int> path;
    std::vector <int> map;
    QPixmap lvlImage;
    QRect levelRect;
    bool used=false;
    int width = 0;
    int height = 0;
    QString lvlName;
    QString getMapString();
};

#endif // LEVEL_H
