#ifndef DAILYREWARD_H
#define DAILYREWARD_H

#include <QObject>
#include <QPainter>
#include <QString>
#include <QDate>
#include <QFont>

class DailyReward : public QObject
{
    Q_OBJECT
private:
    QPixmap lockPx = QPixmap(":/data/images/schloss_quadrat.png");
    QPixmap shekelMoney = QPixmap(":/data/images/shekel.png");
    QPixmap hakenPx = QPixmap(":/data/images/ui/haken.png");
    std::vector <int> bonusAmount;
    std::vector <QRect> rects;
    QColor grau = QColor(22,22,24);
public:
    bool active = false;
    bool hasTakenReward = false;
    int bonus = 0;
    double anY = 0;
    double anY2 = -720;
    QPixmap bildlaufPx = QPixmap(":/data/images/Bildlauf.png");
    QPixmap dailyRewardPx = QPixmap(":/data/images/DailyBonusBenis.png");
    QRect mainRect = QRect(350,200,1280,720);
    QRect backgroundRect;
    explicit DailyReward(QObject *parent = nullptr);
    void clicked(QRect pos);
    void drawDailyReward(QPainter &painter);

signals:
    void addBonus(int amount);
public slots:
};

#endif // DAILYREWARD_H
