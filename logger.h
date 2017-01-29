#ifndef LOGGER_H_
#define LOGGER_H_

#define D_ERROR 0
#define D_GAME  1
#define D_EXPERIMENT  2
#define D_DEBUG  4
#define D_TEMP  8
#define D_MANI  16
#define D_ARASH  32
#define D_ALI  64
#define D_SEPEHR  128
#define D_MASOOD  256
#define D_MOHAMMED  512
#define D_HOSSEIN  1024
#define D_KK 2048
#define D_MHMMD 4096
#define D_ERF 8192
#define D_MAHI 16386


#include <QString>
#include <QColor>
#include <geom.h>

void initLogger(void*);
void initDrawer(void*);
void initInfo(void *);
void setDebugLevel(int l);
void setDrawLevel(int l);
void debug(QString text, long type, QColor color = QColor(0,0,0));
bool drawLock(int ms=-1);
void drawUnlock();
void draw (Rect2D _rect = Rect2D(Vector2D(0, 0), Vector2D(0, 0)), QColor _color = QColor(255, 255, 255), bool _filled = false, int level=-1 , bool toBeloged=true);
void draw (QString _text = "", Vector2D _pos = Vector2D(0,0), QColor _color = QColor("black"), int _size = 12, int level=-1, bool toBeloged=true);
void draw (Circle2D _circle = Circle2D(Vector2D(0,0), 0 ), int _startAng = 0, int _endAng = 360, QColor _color=QColor(255, 255, 255), bool _filled = false, int level=-1, bool toBeloged=true);
void draw (Circle2D _circle = Circle2D(Vector2D(0,0), 0 ), QColor _color=QColor(255, 255, 255), bool _filled = false, int level=-1, bool toBeloged=true);
void draw (Polygon2D _polygon = Polygon2D(), QColor _color = QColor(255, 255, 255), bool _filled = false, int level=-1, bool toBeloged=true);
void draw (Segment2D _seg = Segment2D(Vector2D(0,0), Vector2D(0,0)), QColor _color = QColor(255, 255, 255), int level=-1, bool toBeloged=true);
void draw (Vector2D _point = Vector2D(0,0), int _type = 0, QColor _color = QColor(255, 255, 255), int level=-1, bool toBeloged=true);

#endif // LOGGER_H
