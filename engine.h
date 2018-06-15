#ifndef ENGINE_H
#define ENGINE_H
#include <QtMath>
#include <QRectF>
#include <QPointF>
#include <QPolygonF>
#include <QElapsedTimer>
#include <QFont>
#include <QPainter>

class Engine
{
public:
    static QString rectFToString(QRectF rect, QString sep = ",")
    {
        return QString(QString::number(rect.x()) + sep +
                       QString::number(rect.y()) + sep +
                       QString::number(rect.width()) + sep +
                       QString::number(rect.height()));
    }
    static double getAngle(QPointF p1, QPointF p2)
    {
        double theta = atan2(p2.y()-p1.y(),p2.x()-p1.x());
        double angle = qRadiansToDegrees(theta);
        if(angle<0) angle+=360;
        return angle;
    }
    static double getDistance(QPointF p1,QPointF p2)
    {
        //292-585-877
        double v1 = qFabs(p2.x()-(double)p1.x());
        double v2 = qFabs(p2.y()-(double)p1.y());
        double result = qSqrt((v1*v1)+(v2*v2));
        return result;
    }
    static bool getTurnSide(int current, int target)
    {
        int diff = target - current;
        if(diff < 0)
            diff += 360;
        if(diff > 180)
             return false; // left turn
        else
             return true; // right turn
    }
    static bool isEqual(double a, double b, double epsilon = 0.001)
    {
        bool ok = false;
        if(std::fabs(a-b)<epsilon) ok = true;
        return ok;
    }
    static bool intersectsWithCircle(QRectF rect, QRectF circle)
    {
        bool coll=false;
        if(rect.intersects(circle)) {
            double distX = abs(circle.center().x()-rect.center().x());
            double distY = abs(circle.center().y()-rect.center().y());
            double radius = circle.width()/2;
            if((distX>(rect.width()/2+radius))||
                    (distY>(rect.height()/2+radius))) return false;
            if((distX<=(rect.width()/2))||
                    (distY<=(rect.height()/2))) return true;
            double dx=distX-rect.width()/2;
            double dy=distY-rect.height()/2;
            if(dx*dx+dy*dy<=(radius*radius)) return true;
        }
        return coll;
    }
    static bool intersectsWithCircle2(QRectF rect, int radius, QPoint center)
    {
        bool coll=false;
        double distX = abs(center.x()-rect.center().x());
        double distY = abs(center.y()-rect.center().y());
        if((distX>(rect.width()/2+radius))||
                (distY>(rect.height()/2+radius))) return false;
        if((distX<=(rect.width()/2))||
                (distY<=(rect.height()/2))) return true;
        double dx=distX-rect.width()/2;
        double dy=distY-rect.height()/2;
        if(dx*dx+dy*dy<=(radius*radius)) return true;
        return coll;
    }
    static bool intersectsCirclewithCircle(QRectF circle1, QRectF circle2)
    {
        bool coll=false;
        if(circle1.intersects(circle2)) {
            int r1 = circle1.width()/2;
            int r2 = circle2.width()/2;
            int x1 = circle1.center().x();
            int x2 = circle2.center().x();
            int y1 = circle1.center().y();
            int y2 = circle2.center().y();
            int e1 = (r1 - r2) * (r1 - r2);
            int e2 = ((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2));
            int e3 = (r1 + r2) * (r1 + r2);
            if(e1 <= e2 && e2 <= e3) {
                coll = true;
            }
        }
        return coll;
    }
    static int random(int min, int max)
    {
        int randn;
        if(min==max) {
            randn = min;
        } else {
            randn = qrand() %(max-min) + min;
        }
        return randn;
    }
    static double getAngleDifference(double a1, double a2)
    {
        double diff = 0;
        diff = a1 - a2;
        if(diff > 180) diff -= 360;
        if(diff < -180) diff += 360;
        return std::fabs(diff);
    }
    static double numToBox2D(double num)
    {
        num /= 500;
        return num;
    }
    static double numToNormal(double num)
    {
        num *= 500;
        return num;
    }
    static QPolygonF getRauteFromRect(QRectF rect)
    {
        QPolygonF poly;
        poly.append(QPointF(rect.x() + rect.width() / 2, rect.y()));
        poly.append(QPointF(rect.right(), rect.y() + rect.height()/2));
        poly.append(QPointF(rect.x() + rect.width() / 2, rect.bottom()));
        poly.append(QPointF(rect.x(), rect.y() + rect.height() / 2));
        return poly;
    }
    static bool polygonIntersectsRect(QRectF rect, QPolygonF poly)
    {
        QPolygonF poly_rect;
        poly_rect.append(QPointF(rect.x(),rect.y()));
        poly_rect.append(QPointF(rect.right(),rect.y()));
        poly_rect.append(QPointF(rect.right(),rect.bottom()));
        poly_rect.append(QPointF(rect.x(),rect.bottom()));
        bool coll = false;
        coll = poly_rect.intersects(poly);
        return coll;
    }
    static void changeSize(QFont &font, QPainter &painter, int pixelSize, bool bold = false)
    {
        font.setPixelSize(pixelSize);
        font.setBold(bold);
        painter.setFont(font);
    }
    static void drawButton(QPainter &painter, QRect rect, QString text, int size = 48, QColor color = QColor(Qt::white))
    {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(color,3));
        painter.drawRect(rect);
        QFont f = painter.font();
        changeSize(f, painter, size, true);
        painter.drawText(rect, Qt::AlignCenter, text);
    }
private:
    Engine() {}
};

#endif // ENGINE_H
