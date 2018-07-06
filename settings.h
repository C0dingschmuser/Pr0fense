#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QPainter>

enum _graphicQuality {
    GRAPHIC_LOW = 0,
    GRAPHIC_MEDIUM = 1,
    GRAPHIC_HIGH = 2,
};

class Settings : public QObject
{
    Q_OBJECT
private:
    QPixmap datenschutzPx;
    QPixmap bedingungenPx;
    QPixmap lowPx;
    QPixmap mediumPx;
    QPixmap highPx;
    QPixmap checkedPx;
    QPixmap uncheckedPx;
    QPixmap normalPx;
    QPixmap buttonsPx;
    QRect datenschutzRect = QRect(500,900,400,97);
    QRect bedingungenRect = QRect(950, 900, 491, 97);
    QRect lowRect = QRect(500, 400, 200, 100);
    QRect mediumRect = QRect(725, 400, 200, 100);
    QRect highRect = QRect(950, 400, 200, 100);
    QRect energieRect = QRect(500, 525, 100, 100);
    QRect buttonRect = QRect(500, 700, 283, 64);
    QRect normalRect = QRect(800, 700, 283, 64);

public:
    explicit Settings(QObject *parent = nullptr);
#ifdef Q_OS_ANDROID
    int graphicQuality = GRAPHIC_MEDIUM;
#else
    int graphicQuality = GRAPHIC_HIGH;
#endif
    bool energieSparen = false;
    bool zoomMode = 0;
    void loadGraphics();
    void drawSettings(QPainter &painter);
    void settingsClicked(QRect pos);

signals:
    void openPage(int id);
    void graphicsChanged(int graphics);
    void energieSparenChanged(bool mode);
    void zoomChanged();

public slots:
};

#endif // SETTINGS_H
