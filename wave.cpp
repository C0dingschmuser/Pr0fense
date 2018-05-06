#include "wave.h"

Wave::Wave()
{

}

Wave::Wave(int fl, int nsw, int sw, int s, int alt, int m, int a, int l, int g)
{
    this->fliesen = fl;
    this->neuschwuchteln = nsw;
    this->schwuchteln = sw;
    this->spender = s;
    this->altschwuchteln = alt;
    this->mods = m;
    this->admins = a;
    this->legenden = l;
    this->gebannte = g;
}

QString Wave::toString()
{
    return QString::number(fliesen) + "," +
            QString::number(neuschwuchteln) + "," +
            QString::number(schwuchteln) + "," +
            QString::number(spender) + "," +
            QString::number(altschwuchteln) + "," +
            QString::number(mods) + "," +
            QString::number(admins) + "," +
            QString::number(legenden) + "," +
            QString::number(gebannte);
}
