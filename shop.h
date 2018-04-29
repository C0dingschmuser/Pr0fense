#ifndef SHOP_H
#define SHOP_H

#include <QObject>
#include <QPainter>

class Shop : public QObject
{
    Q_OBJECT
public:
    explicit Shop(QObject *parent = nullptr);
    void drawShop(QPainter &painter);

signals:

public slots:
};

#endif // SHOP_H
