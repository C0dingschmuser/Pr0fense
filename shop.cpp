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
    shekelPrices.push_back(349.99);

    shekelPacks.push_back(50);
    shekelPacks.push_back(125);
    shekelPacks.push_back(275);
    shekelPacks.push_back(600);
    shekelPacks.push_back(1325);
    shekelPacks.push_back(2750);
    shekelPacks.push_back(6000);
    shekelPacks.push_back(75000);

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
        }
        break;
    case 1: //maps
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
                shekel -= lvlprices[subSelected];
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
        }
        break;
    case 2: //towers
        for(uint i = 0; i < towerPositions.size(); i++) {
            if(pos.intersects(towerPositions[i])) {
                subSelected = i+1;
            }
        }
        if(pos.intersects(buyRect) &&
                towerPrices[subSelected-1] > 0) {
            if(shekel >= towerPrices[subSelected-1]) {
                shekel -= towerPrices[subSelected-1];
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
                paket = "shekel_05";
                break;
            }
            emit buyShekel(paket);
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
        break;
    case 1: //maps
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
        break;
    case 2: //towers
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
            } else { //tower gesperrt
                painter.drawPixmap(towerPositions[i-1], towers[0]);
            }
        }
        //painter.setBrush(Qt::red);
        //painter.drawRect(infoRect);
        break;
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
                                 Qt::AlignLeft, " "+QString::number(shekelPacks[i]) + " Shekel Für "+QString::number(shekelPrices[i],'f',2)+"€ kaufen");
                painter.setPen(Qt::NoPen);
                painter.drawPixmap(buyRect.x() - 315, buyRect.y() - 5, 650, 85, auswahlpx);
            }
        }
        break;
    }
    painter.setOpacity(1);
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
