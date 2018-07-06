#ifndef STATISTICS_H
#define STATISTICS_H

#include <QObject>
#include <QPainter>
#include <QString>
#include "engine.h"


class Statistics : public QObject
{
    Q_OBJECT
private:
    QPixmap coinPx;
    QPixmap shekelPx;
    QRect coinRect = QRect(500, 500, 50, 50);
    QRect shekelRect = QRect(500, 550, 75, 37.5);
public:
    explicit Statistics(QObject *parent = nullptr);
    void drawStatistics(QPainter &painter);
    void loadGraphics();
    bool active = false;
    unsigned long long highestWave = 0;
    std::vector <unsigned long long> kills;
    unsigned long long earnedBenis = 0;
    unsigned long long earnedShekel = 0;
    unsigned long long towersBuilt = 0;
    unsigned long long towersSold = 0;
    unsigned long long enemysPassed = 0;
    unsigned long long playedGames = 0;
    unsigned long long timePlayed = 0;

signals:

public slots:
};

#endif // STATISTICS_H
