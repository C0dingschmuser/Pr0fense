#include "level.h"

Level::Level()
{

}

QString Level::getMapString()
{
    QString m;
    for(uint i=0;i<map.size();i++) {
        QString tmp = QString::number(map[i]) + ",";
        m.append(tmp);
    }
    return m;
}

int Level::getPathNum(int start)
{
    int num = -1;
    for(uint i=0;i<paths.size();i++) {
        if(paths[i].path[0]==start) {
            num = i;
            break;
        }
    }
    return num;
}
