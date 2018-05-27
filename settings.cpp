#include "settings.h"

Settings::Settings(QObject *parent) : QObject(parent)
{

}

void Settings::drawSettings(QPainter &painter)
{
    painter.drawPixmap(datenschutzRect, datenschutzPx);
    painter.drawPixmap(bedingungenRect, bedingungenPx);
    QFont f = painter.font();
    f.setPixelSize(32);
    f.setBold(true);
    painter.setFont(f);
    painter.setPen(Qt::white);
    painter.drawText(QPoint(500, 1030),"Für eine Kopie der von uns gespeicherten Daten über dich");
    painter.drawText(QPoint(500, 1060),"wende dich bitte an @FireDiver oder firediver98@gmail.com");
}

void Settings::settingsClicked(QRect pos)
{
    if(pos.intersects(datenschutzRect)) {
        emit openPage(0);
    } else if(pos.intersects(bedingungenRect)) {
        emit openPage(1);
    }
}
