#include "settings.h"

Settings::Settings(QObject *parent) : QObject(parent)
{

}

void Settings::loadGraphics()
{
    datenschutzPx = QPixmap(":/data/images/ui/Datenschutz.png");
    bedingungenPx = QPixmap(":/data/images/ui/Nutzungsbedingungen.png");
    lowPx = QPixmap(":/data/images/ui/settings/Niedrig.png");
    mediumPx = QPixmap(":/data/images/ui/settings/Mittel.png");
    highPx = QPixmap(":/data/images/ui/settings/Hoch.png");
    uncheckedPx = QPixmap(":/data/images/ui/settings/unchecked.png");
    checkedPx = QPixmap(":/data/images/ui/settings/checked.png");
    normalPx = QPixmap(":/data/images/ui/settings/normal.png");
    buttonsPx = QPixmap(":/data/images/ui/settings/buttons.png");
}

void Settings::drawSettings(QPainter &painter)
{
    QFont f = painter.font();
    f.setPixelSize(42);
    painter.setFont(f);
    painter.setPen(Qt::white);
    painter.drawText(500, 375, "Grafikqualität");

    painter.setPen(Qt::NoPen);
    if(graphicQuality != GRAPHIC_LOW) {
        painter.setOpacity(0.6);
    } else painter.setOpacity(1);
    painter.drawPixmap(lowRect, lowPx);
    if(graphicQuality != GRAPHIC_MEDIUM) {
        painter.setOpacity(0.6);
    } else painter.setOpacity(1);
    painter.drawPixmap(mediumRect, mediumPx);
    if(graphicQuality != GRAPHIC_HIGH) {
        painter.setOpacity(0.6);
    } else painter.setOpacity(1);
    painter.drawPixmap(highRect, highPx);

    painter.setOpacity(1);
    if(!energieSparen) {
        painter.drawPixmap(energieRect, uncheckedPx);
    } else painter.drawPixmap(energieRect, checkedPx);
    painter.setPen(Qt::white);
    f.setPixelSize(32);
    painter.drawText(energieRect.x() + 125, energieRect.y() + 70, "Energiesparmodus (30FPS)");

    f.setPixelSize(42);
    painter.setFont(f);
    painter.setPen(Qt::white);
    painter.drawText(500, 675, "Zoom-Modus");

    if(zoomMode == 1) {
        painter.setOpacity(1);
    } else painter.setOpacity(0.6);
    painter.drawPixmap(buttonRect, buttonsPx);

    if(!zoomMode) {
        painter.setOpacity(1);
    } else painter.setOpacity(0.6);
    painter.drawPixmap(normalRect, normalPx);

    painter.setOpacity(1);
    painter.drawPixmap(datenschutzRect, datenschutzPx);
    painter.drawPixmap(bedingungenRect, bedingungenPx);
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
    } else if(pos.intersects(lowRect) && graphicQuality != GRAPHIC_LOW) {
        graphicQuality = GRAPHIC_LOW;
        emit graphicsChanged(GRAPHIC_LOW);
    } else if(pos.intersects(mediumRect) && graphicQuality != GRAPHIC_MEDIUM) {
        graphicQuality = GRAPHIC_MEDIUM;
        emit graphicsChanged(GRAPHIC_MEDIUM);
    } else if(pos.intersects(highRect) && graphicQuality != GRAPHIC_HIGH) {
        graphicQuality = GRAPHIC_HIGH;
        emit graphicsChanged(GRAPHIC_HIGH);
    } else if(pos.intersects(energieRect)) {
        if(!energieSparen) {
            graphicQuality = GRAPHIC_LOW;
            energieSparen = true;
        } else energieSparen = false;
        emit graphicsChanged(graphicQuality);
        emit energieSparenChanged(energieSparen);
    } else if(pos.intersects(buttonRect)) {
        if(zoomMode != 1) {
            zoomMode = 1;
            emit zoomChanged();
        }
    } else if(pos.intersects(normalRect)) {
        if(zoomMode != 0) {
            zoomMode = 0;
            emit zoomChanged();
        }
    }
}
