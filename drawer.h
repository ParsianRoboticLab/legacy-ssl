#ifndef CDRAWER_H
#define CDRAWER_H

#include <QQueue>
#include <QColor>

#include "base.h"

class CStatusText
{
    public:
    CStatusText(QString _text = "", QColor _color = QColor("black"), int _size = 12)
    {
        text= _text;
        color = _color;
        size = _size;
    }

    QString text;
    QColor color;
    int size;
};

class CGraphicalText
{
public:
    CGraphicalText(QString _text = "", Vector2D _pos = Vector2D(0,0), QColor _color = QColor("black"), int _size = 12)
    {
        text= _text;
        pos = _pos;
        color = _color;
        size = _size;
    }
    QString text;
    QColor color;
    Vector2D pos;
    int size;
};

class CGraphicalRobot
{
public:
    CGraphicalRobot(Vector2D _pos = Vector2D(0,0), Vector2D _dir = Vector2D(1,0), QColor _color = QColor(255, 255, 0), int _ID = 0, int _comID=0, QString _str="", bool _newRobots = false)
    {
        pos = _pos;
        dir = _dir;
        color = _color;
        ID = _ID;
        comID = _comID;
        str = _str;
        newRobots = _newRobots;
    }
    int ID, comID;
    QColor color;
    Vector2D pos;
    Vector2D dir;
    QString str;
    bool newRobots;
};

class CGraphicalArc
{
public:
    CGraphicalArc(Circle2D _circle = Circle2D(Vector2D(0,0), 0 ), int _startAng = 0, int _endAng = 360, QColor _color=QColor(255, 255, 255), bool _filled = false)
    {
        circle = _circle;
        startAng = _startAng;
        endAng = _endAng;
        filled = _filled;
        color = _color;
    }
    Circle2D circle;
    int startAng;
    int endAng;
    bool filled;
    QColor color;
};

class CGraphicalPolygon
{
public:
    CGraphicalPolygon(Polygon2D _polygon = Polygon2D(), QColor _color=QColor(255, 255, 255), bool _filled = false)
    {
        polygon = _polygon;
        filled = _filled;
        color = _color;
    }
    Polygon2D polygon;
    bool filled;
    QColor color;
};

class CGraphicalSegment
{
public:
    CGraphicalSegment(Segment2D _seg = Segment2D(Vector2D(0,0), Vector2D(0,0)), QColor _color = QColor(255, 255, 255))
    {
        seg=_seg;
        color = _color;
    }

    Segment2D seg;
    QColor color;
};

class CGraphicalRect
{
public:
    CGraphicalRect(Rect2D _rect=Rect2D(Vector2D(0,0), Vector2D(0,0)), QColor _color = QColor(255, 255, 255), bool _filled = false)
    {
        filled = _filled;
        rect = _rect;
        color = _color;
    }
    Rect2D rect;
    bool filled;
    QColor color;
};

class CGraphicalPoint
{
public:
    CGraphicalPoint(Vector2D _point = Vector2D(0,0), int _type = 0, QColor _color = QColor(255, 255, 255))
    {
        point = _point;
        type = _type;
        color = _color;
    }
    Vector2D point;
    QColor color;
    int type;
};

class CDrawer
{
public:

    QQueue<CGraphicalRect> rectBuffer;
    QQueue<CGraphicalArc> arcBuffer;
    QQueue<CGraphicalPolygon> polygonBuffer;
    QQueue<CGraphicalSegment> segBuffer;
    QQueue<CGraphicalPoint> pointBuffer;
    QQueue<CGraphicalText> textBuffer;
    QQueue<CGraphicalRobot> robotBuffer;

    CDrawer();
    void draw (Rect2D _rect = Rect2D(Vector2D(0, 0), Vector2D(0, 0)), QColor _color = QColor(255, 255, 255), bool _filled = false);
    void draw (QString _text = "", Vector2D _pos = Vector2D(0,0), QColor _color = QColor("black"), int _size = 12);
    void draw (Circle2D _circle = Circle2D(Vector2D(0,0), 0 ), int _startAng = 0, int _endAng = 360, QColor _color=QColor(255, 255, 255), bool _filled = false);
    void draw (Polygon2D _polygon = Polygon2D(), QColor _color = QColor(255, 255, 255), bool _filled = false);
    void draw (Segment2D _seg = Segment2D(Vector2D(0,0), Vector2D(0,0)), QColor _color = QColor(255, 255, 255));
    void draw (Vector2D _point = Vector2D(0,0), int _type = 0, QColor _color = QColor(255, 255, 255));
    void drawRobot (Vector2D _pos = Vector2D(0,0), Vector2D _dir = Vector2D(0,0), QColor _color = QColor(255, 255, 0), int _ID = 0,int _comID=0, QString _str="", bool newRobots = false);
    void clear();
};

class CStatusPrinter
{
    public:
    CStatusPrinter() {}

    QQueue<CStatusText> textBuffer;
};

#endif // CDRAWER_H
