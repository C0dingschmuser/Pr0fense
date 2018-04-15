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
