#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QPainter>

class Settings : public QObject
{
    Q_OBJECT
private:
    QPixmap datenschutzPx = QPixmap(":/data/images/ui/Datenschutz.png");
    QPixmap bedingungenPx = QPixmap(":/data/images/ui/Nutzungsbedingungen.png");
    QRect datenschutzRect = QRect(500,900,400,97);
    QRect bedingungenRect = QRect(950, 900, 491, 97);

public:
    explicit Settings(QObject *parent = nullptr);
    void drawSettings(QPainter &painter);
    void settingsClicked(QRect pos);

signals:
    void openPage(int id);
public slots:
};

#endif // SETTINGS_H
