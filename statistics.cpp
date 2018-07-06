#include "statistics.h"

Statistics::Statistics(QObject *parent) : QObject(parent)
{
    for(uint i = 0; i < 9; i++) {
        kills.push_back(0);
    }
}

void Statistics::drawStatistics(QPainter &painter)
{
    unsigned long long totalKills = 0;
    for(uint i = 0; i < kills.size(); i++) {
        totalKills += kills[i];
    }
    QFont f = painter.font();
    f.setPixelSize(42);
    f.setBold(true);
    painter.setFont(f);
    painter.setPen(Qt::white);
    painter.drawText(QRect(500,430,800,50),Qt::AlignLeft, "Höchste Welle: ");
    painter.drawText(QRect(500,475,800,50),Qt::AlignLeft, "Kills insgesamt: ");
    painter.drawText(QRect(500,520,800,50),Qt::AlignLeft, "Shekel verdient: ");
    painter.drawText(QRect(500,565,800,50),Qt::AlignLeft, "Benis verdient: ");
    painter.drawText(QRect(500,605,800,50),Qt::AlignLeft, "Spiele gespielt: ");
    painter.drawText(QRect(500,650,800,50),Qt::AlignLeft, "Gegner durchgelassen: ");
    painter.drawText(QRect(500,695,800,50),Qt::AlignLeft, "Tower gebaut: ");
    painter.drawText(QRect(500,740,800,50),Qt::AlignLeft, "Tower verkauft: ");
    painter.drawText(QRect(500,785,800,50),Qt::AlignLeft, "Spielzeit: ");
    painter.drawText(QRect(500,430,800,50),Qt::AlignRight, QString::number(highestWave));
    painter.drawText(QRect(500,475,800,50),Qt::AlignRight, Engine::toShortString(totalKills));
    painter.drawText(QRect(500,520,800,50),Qt::AlignRight, Engine::toShortString(earnedShekel));
    painter.drawText(QRect(500,565,800,50),Qt::AlignRight, Engine::toShortString(earnedBenis));
    painter.drawText(QRect(500,605,800,50),Qt::AlignRight, Engine::toShortString(playedGames));
    painter.drawText(QRect(500,650,800,50),Qt::AlignRight, Engine::toShortString(enemysPassed));
    painter.drawText(QRect(500,695,800,50),Qt::AlignRight, Engine::toShortString(towersBuilt));
    painter.drawText(QRect(500,740,800,50),Qt::AlignRight, Engine::toShortString(towersSold));
    painter.drawText(QRect(500,785,800,50),Qt::AlignRight, Engine::toTimeString(timePlayed));

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    for(uint i = 0; i < 9; i++) {
        painter.drawRect(500, 475 + (45*i), 800, 2);
    }
    painter.setPen(Qt::white);
    painter.drawText(1500, 430, "Killverteilung");
    painter.setPen(Qt::NoPen);
    f.setPixelSize(35);
    int num = 0;
    for(uint i = 0; i < kills.size(); i++) {
        if(kills[i] > 0) {
            painter.setBrush(Engine::getColor(i));
            painter.drawEllipse(QPoint(1500, 780 + (num*35)), 15, 15);
            painter.setPen(Qt::white);
            f.setPixelSize(25);
            painter.setFont(f);
            painter.drawText(1520, 789 + (num*35), Engine::toShortString(kills[i]));
            if(i == ENEMY_LEGENDE) {
                f.setPixelSize(20);
                painter.setFont(f);
                painter.drawText(1494,787 + (num*35), "B");
            }
            num++;
            painter.setPen(Qt::NoPen);
        }
    }
    painter.setPen(Qt::white);

    painter.setPen(Qt::NoPen);
    if(totalKills > 0) {
        std::vector <int> angles;
        std::vector <int> ids;
        for(uint i = 0; i < kills.size(); i++) {
            if(kills[i] > 0) {
                int angle = ((double)kills[i]/totalKills)*5760; //(16*360)
                angles.push_back(angle);
                ids.push_back(i);
            }
        }
        int start = 0;
        for(uint i = 0; i < angles.size(); i++) {
            if(i > 0) {
                start += angles[i-1];
            }
            painter.setBrush(Engine::getColor(ids[i]));
            painter.drawPie(1500,450,300,300,start,angles[i]);
        }
    }
}

void Statistics::loadGraphics()
{
    coinPx = QPixmap(":/data/images/towers/coin.png");
    shekelPx = QPixmap(":/data/images/shekel.png");
}
