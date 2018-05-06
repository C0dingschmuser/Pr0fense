#ifndef WAVE_H
#define WAVE_H
#include <QString>

class Wave
{
public:
    Wave();
    Wave(int fl, int nsw, int sw, int s, int alt, int m, int a, int l, int g);
    int fliesen = 0;
    int neuschwuchteln = 0;
    int schwuchteln = 0;
    int spender = 0;
    int altschwuchteln;
    int mods = 0;
    int admins = 0;
    int legenden = 0;
    int gebannte = 0;
    bool empty = false;
    QString toString();
};

#endif // WAVE_H
