#include "drawer.h"

CDrawer::CDrawer()
{

}

void CDrawer::draw(Rect2D _rect, QColor _color, bool _filled)
{
    CGraphicalRect newItem(_rect,_color,_filled);
    rectBuffer.enqueue(newItem);
}

void CDrawer::draw(QString _text, Vector2D _pos, QColor _color, int _size)
{
    CGraphicalText newItem(_text, _pos, _color,_size);
    textBuffer.enqueue(newItem);
}

void CDrawer::drawRobot (Vector2D _pos, Vector2D _dir, QColor _color, int _ID, int _comID, QString _str, bool _newRobots)
{
    CGraphicalRobot newItem(_pos, _dir, _color, _ID, _comID, _str, _newRobots);
    robotBuffer.enqueue(newItem);
}

void CDrawer::draw (Circle2D _circle, int _startAng, int _endAng, QColor _color, bool _filled)
{
    CGraphicalArc newItem(_circle, _startAng, _endAng, _color, _filled);
    arcBuffer.enqueue(newItem);
}

void CDrawer::draw (Polygon2D _polygon, QColor _color, bool _filled)
{
    CGraphicalPolygon newItem(_polygon, _color, _filled);
    polygonBuffer.enqueue(newItem);
}

void CDrawer::draw (Segment2D _seg, QColor _color)
{
    CGraphicalSegment newItem(_seg, _color);
    segBuffer.enqueue(newItem);
}

void CDrawer::draw (Vector2D _point, int _type, QColor _color)
{
    CGraphicalPoint newItem(_point, _type, _color);
    pointBuffer.enqueue(newItem);
}

void CDrawer::clear(){
    rectBuffer.clear();
    arcBuffer.clear();
    polygonBuffer.clear();
    segBuffer.clear();
    pointBuffer.clear();
    textBuffer.clear();
    robotBuffer.clear();
}
