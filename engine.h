#ifndef ENGINE_H
#define ENGINE_H
#include <QtMath>
#include <QRectF>
#include <QPointF>

class Engine
{
public:
    static double getAngle(QPointF p1, QPointF p2)
    {
        double theta = atan2(p2.y()-p1.y(),p2.x()-p1.x());
        double angle = qRadiansToDegrees(theta);
        if(angle<0) angle+=360;
        return angle;
    }
    static double getDistance(QPointF p1,QPointF p2)
    {
        return qSqrt(qPow(qFabs(p2.x()-(double)p1.x()),2)+qPow(qFabs(p2.y()-(double)p1.y()),2));
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
            int e1 = pow((r1-r2),2);
            int e2 = pow((x1-x2),2)+pow((y1-y2),2);
            int e3 = pow((r1+r2),2);
            if(e1<=e2&&e2<=e3) {
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
            randn = rand() %(max-min) + min;
        }
        return randn;
    }
private:
    Engine() {}
};

#endif // ENGINE_H
