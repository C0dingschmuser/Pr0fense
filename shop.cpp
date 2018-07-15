#include "shop.h"

Shop::Shop(QObject *parent) : QObject(parent)
{
    for(uint i = 0; i < 11; i++) {
        towers.push_back(QPixmap(":/data/images/ui/shop/tuerme/tower"+QString::number(i)+".png"));
    }
    for(uint i = 0; i < 8; i++) {
        shekelPacksPx.push_back(QPixmap(":/data/images/ui/shop/shekelpacks/shekel"+QString::number(i)+".png"));
    }

    shekelPrices.push_back(0.59);
    shekelPrices.push_back(0.99);
    shekelPrices.push_back(1.99);
    shekelPrices.push_back(3.99);
    shekelPrices.push_back(7.99);
    shekelPrices.push_back(14.99);
    shekelPrices.push_back(29.99);
    shekelPrices.push_back(49.99);

    shekelPacks.push_back(50);
    shekelPacks.push_back(100);
    shekelPacks.push_back(300);
    shekelPacks.push_back(600);
    shekelPacks.push_back(1300);
    shekelPacks.push_back(2800);
    shekelPacks.push_back(6000);
    shekelPacks.push_back(15000);

    items.push_back(Item("speed_superfast", "Schaltet 3x Geschwindigkeit frei", 0.99, 0));
    items.push_back(Item("art13", "Temporärer Uploadfilter", 250, 1, true));
    items[1].locked = false;


    int y = 0;
    int a = 0;
    for(uint i = 0;i < 8 + 1; i++) {
        QRect rect = QRect(500 + (325 * a),400 + (193 * y),300,168);
        a++;
        if(a==4) {
            a=0;
            y++;
        }
        positions.push_back(rect);
    }
    y = 0;
    a = 0;
    for(uint i = 0;i < towers.size(); i++) {
        QRect rect = QRect(500 + (225 * a),400 + (210 * y),200,200);
        a++;
        if(a == 5) {
            a = 0;
            y++;
        }
        towerPositions.push_back(rect);
    }
}

void Shop::loadGraphics()
{
    mapsPx = QPixmap(":/data/images/ui/shop/maps.png");
    shekelPx = QPixmap(":/data/images/ui/shop/shekel.png");
    towerPx = QPixmap(":/data/images/ui/shop/tuerme.png");
    itemPx = QPixmap(":/data/images/ui/shop/Items.png");
    auswahlpx = QPixmap(":/data/images/auswahl_buy.png");
    shekelMoney = QPixmap(":/data/images/shekel.png");
    lockPx = QPixmap(":/data/images/schloss.png");
    lock_quadratPx = QPixmap(":/data/images/schloss_quadrat.png");
    shekelPlusPx = QPixmap(":/data/images/ui/shop/PlusButton.png");
    sternPx = QPixmap(":/data/images/stern.png");
    items[1].image = QPixmap(":/data/images/effects/art13/0.png");
    playingModeNormal = QPixmap(":/data/images/normal.png");
    playingModeMaze = QPixmap(":/data/images/maze.png");
}

void Shop::shopClicked(QRect pos)
{
    switch(subMenu) {
    case 0: //hauptmenü
        if(pos.intersects(positions[0])) {
            subSelected = -1;
            subMenu = 1; //maps
        } else if(pos.intersects(positions[1])) {
            subSelected = -1;
            subMenu = 2; //towers
        } else if(pos.intersects(positions[2])) {
            subSelected = -1;
            subMenu = 3; //shekelpacks
        } else if(pos.intersects(positions[3])) {
            subSelected = -1;
            subMenu = 4;
        }
        break;
    case 1: //maps
        if(playingMode == MODE_NORMAL) {
            for(uint i = 0; i < mainlvls; i++) {
                if(pos.intersects(positions[i])) {
                    subSelected = i;
                }
            }
            if(pos.intersects(buyRect) &&
                    lvlprices[subSelected] > 0) {
                int ok = 0;
                if(subSelected) {
                    if(lvlprices[subSelected - 1]) ok = 1;
                }
                if(!ok) {
                    if(lvlprices[subSelected] > shekel) {
                        ok = 2;
                    }
                }
                if(!ok) {
                    emit setShekel(shekel - lvlprices[subSelected], false);
                    lvlprices[subSelected] = 0;
                    switch(subSelected) {
                    case 1:
                        towerLocks[2] = false;
                        towerLocks[3] = false;
                        break;
                    case 2:
                        towerLocks[4] = false;
                        towerLocks[5] = false;
                        towerLocks[6] = false;
                        break;
                    case 3:
                        towerLocks[7] = false;
                        towerLocks[8] = false;
                        towerLocks[9] = false;
                        break;
                    }
                    emit buyMap(subSelected);
                } else {
                    emit error_buy(ok);
                }
            } else if(pos.intersects(playingModeRect)) {
                playingMode = MODE_MAZE;
            }
        } else if(playingMode == MODE_MAZE) {
            for(uint i = 0; i < mazelvlPrices.size(); i++) {
                if(pos.intersects(positions[i])) {
                    subSelected = i;
                }
            }
            if(pos.intersects(buyRect) &&
                    mazelvlPrices[subSelected] > 0) {
                int ok = 0;
                if(subSelected) {
                    if(mazelvlPrices[subSelected - 1]) ok = 1;
                }
                if(!ok) {
                    if(mazelvlPrices[subSelected] > shekel) {
                        ok = 2;
                    }
                }
                if(!ok) {
                    emit setShekel(shekel - mazelvlPrices[subSelected], false);
                    mazelvlPrices[subSelected] = 0;
                    emit buyMap(subSelected, 1);
                } else {
                    emit error_buy(ok);
                }
            } else if(pos.intersects(playingModeRect)) {
                playingMode = MODE_NORMAL;
            }
        }
        break;
    case 2: //towers
        for(uint i = 0; i < towerPositions.size(); i++) {
            if(pos.intersects(towerPositions[i])) {
                subSelected = i+1;
            }
        }
        if(pos.intersects(buyRect) &&
                towerPrices[subSelected-1] > 0 &&
                !towerLocks[subSelected-1]) {
            if(shekel >= towerPrices[subSelected-1]) {
                emit setShekel(shekel - towerPrices[subSelected-1]);
                towerPrices[subSelected-1] = 0;
                emit buyTower(subSelected-1);
            }
        }
        break;
    case 3: //shekelpacks
        for(uint i = 0; i < shekelPacksPx.size(); i++) {
            if(pos.intersects(positions[i])) {
                subSelected = i;
            }
        }
        if(pos.intersects(QRect(buyRect.x() - 290, buyRect.y()+15, buyRect.width() + 600, buyRect.height()))) {
            QString paket;
            switch(subSelected) {
            case 0:
                paket = "shekel_supersmall";
                break;
            case 1:
                paket = "shekel_small";
                break;
            case 2:
                paket = "shekel_normal";
                break;
            case 3:
                paket = "shekel_big";
                break;
            case 4:
                paket = "shekel_large";
                break;
            case 5:
                paket = "shekel_shekel";
                break;
            case 6:
                paket = "shekel_ultra";
                break;
            case 7:
                paket = "shekel_super";
                break;
            }
            emit buyShekel(paket);
        }
        break;
    case 4: //items
        for(uint i = 0; i < items.size(); i++) {
            if(pos.intersects(towerPositions[i])) {
                subSelected = i;
            }
        }
        if(pos.intersects(buyRect) && (items[subSelected].locked || items[subSelected].hasCount)) {
            if(items[subSelected].name.contains("_")) { //InApp
                emit buyShekel(items[subSelected].name);
            } else {
                if(shekel >= items[subSelected].price) {
                    emit setShekel(shekel - items[subSelected].price, false);
                    items[subSelected].locked = false;
                    if(items[subSelected].hasCount) items[subSelected].count++;
                    emit buyItem(subSelected);
                }
            }
        }
        break;
    }
}

void Shop::drawShop(QPainter &painter)
{
    painter.setOpacity(1);
    switch(subMenu) {
    case 0: //hauptmenü
        painter.drawPixmap(positions[0], mapsPx);
        painter.drawPixmap(positions[1], towerPx);
        painter.drawPixmap(positions[2], shekelPx);
        painter.drawPixmap(positions[3], itemPx);
        break;
    case 1: //maps
        if(playingMode == MODE_NORMAL) {
            painter.drawPixmap(playingModeRect, playingModeNormal);
            for(int i = 0; i < (int)mainlvls; i++) {
                painter.drawPixmap(positions[i], lvlPreviews[i]);
                if(subSelected != i) {
                    painter.setOpacity(0.6);
                    painter.setBrush(grau);
                    painter.drawRect(positions[i]);
                }
                painter.setOpacity(1);
                if(lvlprices[i] > 0) {
                    painter.drawPixmap(positions[i], lockPx);
                    if(i == subSelected) {
                        drawPrice(painter, buyRect, lvlprices[i]);
                    }
                }
            }
        } else if(playingMode == MODE_MAZE) {
            painter.drawPixmap(playingModeRect, playingModeMaze);
            for(int i = 0; i < (int)mazelvlPrices.size(); i++) {
                painter.drawPixmap(positions[i], mazelvlPreviews[i]);
                if(subSelected != i) {
                    painter.setOpacity(0.6);
                    painter.setBrush(grau);
                    painter.drawRect(positions[i]);
                }
                painter.setOpacity(1);
                if(mazelvlPrices[i] > 0) {
                    painter.drawPixmap(positions[i], lockPx);
                    if(i == subSelected) {
                        drawPrice(painter, buyRect, mazelvlPrices[i]);
                    }
                }
            }
        }
        break;
    case 2: {//towers
        for(int i = 1; i < (int)towers.size(); i++) {
            if(i == subSelected) {
                painter.setOpacity(1);
            } else painter.setOpacity(0.6);

            if(!towerLocks[i-1]) { //tower entsperrt -> kann gekauft werden
                painter.drawPixmap(towerPositions[i-1], towers[i]);
                if(towerPrices[i-1]) {
                    painter.drawPixmap(towerPositions[i-1], lock_quadratPx);
                }
                if(i == subSelected) {
                    painter.setPen(Qt::white);
                    Engine::changeSize(f, painter, 38, true);
                    painter.drawText(infoRect, Qt::AlignLeft, Tower::getInfo(i, 0));
                    Engine::changeSize(f, painter, 38, false);
                    painter.drawText(infoRect, Qt::AlignLeading, "\n" + Tower::getInfo(i, 1, true));
                    painter.setPen(Qt::NoPen);
                    if(towerPrices[i-1]) {
                        drawPrice(painter, buyRect, towerPrices[i-1]);
                    }
                }
            } else {
                painter.drawPixmap(towerPositions[i-1], towers[0]);
            }
        }

        bool ok = false;
        if(subSelected > -1) {
            if(towerLocks[subSelected-1]) {
                ok = true;
            }
        } else ok = true;

        if(ok) {
            painter.setOpacity(1);
            painter.setPen(Qt::white);
            Engine::changeSize(f, painter, 38, false);
            painter.drawText(infoRect, Qt::AlignLeft, "Info: Falls du nicht weißt wie du Türme freischaltest,\n"
                                                        "findest du beim ? unter 'Fortschrittssystem' Hilfe");
            painter.setOpacity(0.6);
        }

        painter.setPen(Qt::NoPen);

        //painter.setBrush(Qt::red);
        //painter.drawRect(infoRect);
        break; }
    case 3: //shekelpacks
        for(int i = 0; i < (int)shekelPacksPx.size(); i++) {
            if(i == subSelected) {
                painter.setOpacity(1);
            } else painter.setOpacity(0.6);
            painter.drawPixmap(positions[i], shekelPacksPx[i]);
            if(i == subSelected) {
                painter.setPen(Qt::white);
                Engine::changeSize(f, painter, 38, true);
                painter.drawText(QRect(buyRect.x() - 290, buyRect.y()+15, buyRect.width() + 600, buyRect.height()),
                                 Qt::AlignLeft, " "+QString::number(shekelPacks[i]) +
                                 " Shekel Für "+QString::number(shekelPrices[i],'f',2)+"€ kaufen");
                painter.setPen(Qt::NoPen);
                painter.drawPixmap(buyRect.x() - 315, buyRect.y() - 5, 650, 85, auswahlpx);
            }
        }
        break;
    case 4: //Items
        for(int i = 0; i < (int)items.size(); i++) {
            if(i == subSelected) {
                painter.setOpacity(1);
            } else painter.setOpacity(0.6);

            painter.drawPixmap(towerPositions[i], items[i].image);
            if(items[i].name.contains("_")) {

                painter.drawPixmap(QRect(towerPositions[i].x()+towerPositions[i].width()-80,
                                         towerPositions[i].y(),
                                         75,
                                         75),
                                   sternPx);
            }
            if(items[i].locked || items[i].hasCount) {
                if(items[i].locked) {
                    painter.drawPixmap(towerPositions[i], lock_quadratPx);
                }
                painter.setOpacity(1);
                if(i == subSelected) {
                    painter.drawPixmap(buyRect.x() - 65, buyRect.y() - 5, 365, 85, auswahlpx);
                    f.setPixelSize(48);
                    f.setBold(true);
                    painter.setFont(f);
                    painter.setPen(Qt::white);
                    if(items[i].locked) {
                        painter.drawText(QRect(buyRect.x() - 50,
                                               buyRect.y() + 10,
                                               buyRect.width()+100,
                                               75),
                                         Qt::AlignLeft,
                                         "Kaufen ("+QString::number(items[i].price,'f',2)+" €)");
                    } else if(items[i].hasCount) {
                        f.setPixelSize(32);
                        f.setBold(true);
                        painter.setFont(f);
                        painter.drawText(QRect(buyRect.x() - 65,
                                               buyRect.y() - 5,
                                               365,
                                               85),
                                         Qt::AlignCenter,
                                         "Kaufen ("+QString::number(items[i].price,'f',0)+" Shekel)");
                    }
                    painter.setPen(Qt::NoPen);
                }

                if(items[i].hasCount) {
                    f.setPixelSize(38);
                    f.setBold(true);
                    painter.setFont(f);
                    painter.setPen(Qt::white);
                    painter.drawText(towerPositions[i], Qt::AlignBottom | Qt::AlignCenter, "x"+QString::number(items[i].count));
                }

            }

        }
        break;
    }
    painter.setOpacity(1);
    painter.setPen(Qt::NoPen);
}

void Shop::drawPrice(QPainter &painter, QRect pos, uint price)
{
    painter.drawPixmap(pos.x(), pos.y(), 150, 75, shekelMoney);
    f.setPixelSize(65);
    f.setBold(true);
    painter.setFont(f);
    if(shekel >= price) {
        painter.setPen(Qt::green);
    } else painter.setPen(Qt::red);
    painter.drawText(QRect(pos.x() + 175, pos.y(), pos.width(), 75), Qt::AlignLeft, QString::number(price));
    painter.setPen(Qt::NoPen);
    painter.drawPixmap(pos.x() - 15, pos.y() - 5, 350, 85, auswahlpx);
}
