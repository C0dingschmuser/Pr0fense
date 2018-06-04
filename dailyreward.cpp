#include "dailyreward.h"

DailyReward::DailyReward(QObject *parent) : QObject(parent)
{
    bonusAmount.push_back(25);
    bonusAmount.push_back(50);
    bonusAmount.push_back(75);
    bonusAmount.push_back(150);
    bonusAmount.push_back(300);

    rects.push_back(QRect(mainRect.x()+107, mainRect.y()+269, 206, 305));
    rects.push_back(QRect(mainRect.x()+322, mainRect.y()+269, 206, 305));
    rects.push_back(QRect(mainRect.x()+538, mainRect.y()+269, 206, 305));
    rects.push_back(QRect(mainRect.x()+753, mainRect.y()+269, 206, 305));
    rects.push_back(QRect(mainRect.x()+969, mainRect.y()+269, 206, 305));
}

void DailyReward::clicked(QRect pos)
{
    for(uint i = 0; i < rects.size(); i++) {
        if(pos.intersects(rects[i])) {
            if(!hasTakenReward) {
                emit addBonus(bonusAmount[bonus]);
                bonus++;
                hasTakenReward = true;
                if(bonus == 5) bonus = 0;
            }
            break;
        }
    }
}

void DailyReward::drawDailyReward(QPainter &painter)
{
    QRect adj = mainRect;
    adj.adjust(0,12,0,-12);
    painter.drawPixmap(QRectF(adj), bildlaufPx, QRectF(0,anY,adj.width(),adj.height()));
    if(anY > 1793) {
        painter.drawPixmap(QRectF(adj), bildlaufPx, QRectF(0,anY2,adj.width(),adj.height()));
    }
    painter.drawPixmap(mainRect, dailyRewardPx);

    QFont f = painter.font();
    f.setPixelSize(62);
    f.setBold(true);
    painter.setFont(f);

    painter.drawPixmap(mainRect.x()+160, mainRect.y()+350, 100, 50, shekelMoney);
    painter.drawText(mainRect.x()+175, mainRect.y() + 550, QString::number(bonusAmount[0]));

    for(uint i = 0; i < 2; i++) {
        painter.drawPixmap(mainRect.x()+375, mainRect.y() + 350 +(i*10), 100, 50, shekelMoney);
    }
    painter.drawText(mainRect.x()+390, mainRect.y() + 550, QString::number(bonusAmount[1]));

    for(uint i = 0; i < 3; i++) {
        painter.drawPixmap(mainRect.x()+590, mainRect.y() + 350 +(i*10), 100, 50, shekelMoney);
    }
    painter.drawText(mainRect.x()+605, mainRect.y() + 550, QString::number(bonusAmount[2]));

    for(uint i = 0; i < 4; i++) {
        painter.drawPixmap(mainRect.x()+810, mainRect.y() + 350 +(i*10), 100, 50, shekelMoney);
    }
    painter.drawText(mainRect.x()+810, mainRect.y() + 550, QString::number(bonusAmount[3]));

    for(uint i = 0; i < 5; i++) {
        painter.drawPixmap(mainRect.x()+1030, mainRect.y() + 350 +(i*10), 100, 50, shekelMoney);
    }
    painter.drawText(mainRect.x()+1025, mainRect.y() + 550, QString::number(bonusAmount[4]));

    painter.setBrush(grau);
    painter.setPen(Qt::NoPen);
    for(int i = 0; i < 5; i++) {
        if(bonus < i) {
            painter.setOpacity(0.4);
            painter.drawRect(rects[i]);
            painter.setOpacity(1);
            painter.drawPixmap(QRect(rects[i].x(), rects[i].y()+25, 200, 200), lockPx);
        } else if(bonus > i || (bonus == i && hasTakenReward)) {
            painter.drawPixmap(QRect(rects[i].x(), rects[i].y()+25, 200, 200), hakenPx);
        }
    }
    painter.setOpacity(1);
}
