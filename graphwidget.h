#ifndef CGRAPHWIDGET_H
#define CGRAPHWIDGET_H

#include <QtGui/QWidget>
#include <QGLWidget>
#include <QTimer>
#include <QPixmap>
#include <QColor>
#include <QList>
#include <QShortcut>
#include <QQueue>


class CGraphChnls
{
public:

    double gain;
    double offset;
    QColor color;
    double value;
    bool isActive;
    QString title;
    QQueue <int> history;

    CGraphChnls(QString chnlTitle)
    {
        title=chnlTitle;
		gain=1.0;
        offset=0.0;
        value=0.0;
        isActive=false;
        color = Qt::yellow;
        history.clear();
    }
};

class CGraphWidget : public QGLWidget /*QWidget*/
{
    Q_OBJECT

public:
    CGraphWidget(QWidget *parent = 0 , int w = 200 , int h = 100);


    int x;
    double y;
    QList <CGraphChnls> chnls;
    QTimer *updater;
    QPixmap *pix;
    QPainter painter;
    bool myFlag;    
    QRect clearRect;

    QColor graphBorderColor;
    QColor bgColor;
    QPen graphPen;
    int sideMargin;
    int rightMargin;
    int bottomMargin;
    QSize graphSize;
    QShortcut *stopShortcut;


    void render();
    void initScene();
    void addChnl(QString _title, QColor _color, double _gain=1.0, double _offset=0.0);
    void delChnl(QString);
    ~CGraphWidget();
protected:
    void paintEvent(QPaintEvent *);


signals:
    void stopPressed();
public slots:
    void sendStopSignal();
    void updateScene();
    void setValueForChnl(QString , double val);
};

#endif // CGraphWidget_H
