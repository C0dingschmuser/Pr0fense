#ifndef SHOP_H
#define SHOP_H

#include <QObject>
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <QFont>
#include "level.h"
#include "tower.h"

class Shop : public QObject
{
    Q_OBJECT
private:
    QPixmap mapsPx = QPixmap(":/data/images/ui/shop/maps.png");
    QPixmap shekelPx = QPixmap(":/data/images/ui/shop/shekel.png");
    QPixmap towerPx = QPixmap(":/data/images/ui/shop/tuerme.png");
    QPixmap auswahlpx = QPixmap(":/data/images/auswahl_buy.png");
    std::vector <QPixmap> towers;
    std::vector <QPixmap> shekelPacksPx;
    QRect buyRect = QRect(1525, 975, 325, 168);
    QRect infoRect = QRect(500, 980, 1000, 100);
    std::vector <QRect> positions;
    std::vector <QRect> towerPositions;
    uint selected = 0;
    int subSelected = 0;

public:
    explicit Shop(QObject *parent = nullptr);
    QPixmap shekelMoney = QPixmap(":/data/images/shekel.png");
    QPixmap lockPx;
    QPixmap lock_quadratPx = QPixmap(":/data/images/schloss_quadrat.png");
    QColor grau = QColor(22,22,24);
    QColor edlerSpender = QColor(28,185,146);
    std::vector <QPixmap> lvlPreviews;
    std::vector <uint> lvlprices;
    std::vector <uint> towerPrices;
    std::vector <double> shekelPrices;
    std::vector <uint> shekelPacks;
    std::vector <bool> towerLocks;
    QFont f;
    int subMenu = 0;
    uint mainlvls;
    unsigned long long shekel = 100;
    void shopClicked(QRect pos);
    void drawShop(QPainter &painter);
    void drawPrice(QPainter &painter, QRect pos, uint price);

signals:
    void buyMap(int pos);
    void buyTower(int pos);
    void buyShekel(QString paket);
    void error_buy(int id);

public slots:
};

#endif // SHOP_H
