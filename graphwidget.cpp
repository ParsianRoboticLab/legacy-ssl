#include "graphwidget.h"

#include <cmath>
#include <QDebug>
#include <ctime>
#include <iostream>
using namespace std;


CGraphWidget::CGraphWidget(QWidget *parent , int w , int h )
    : QGLWidget(parent)
    //: QWidget(parent)
{
    x=0.0;
    y=0.0;
    updater = new QTimer(this);
    updater->setInterval(20);

    graphBorderColor=QColor(0x86, 0x86, 0x86);
    bgColor=QColor(0x00, 0x00, 0x00);
    sideMargin = 0;
    rightMargin=0;
    bottomMargin=0;
    graphSize.setWidth(w);
    graphSize.setHeight(h);
    this->setFixedWidth(graphSize.width()+rightMargin+2*sideMargin);
    this->setFixedHeight(graphSize.height()+2*sideMargin+bottomMargin);

    pix = new QPixmap(graphSize+QSize(2*sideMargin+rightMargin, 2*sideMargin+bottomMargin));

    myFlag=false;

    initScene();
    srand(time(0));

    stopShortcut = new QShortcut(QKeySequence(Qt::Key_Escape),this);

    connect(updater, SIGNAL(timeout()), this, SLOT(updateScene()));
    connect(stopShortcut, SIGNAL(activated()), this, SLOT(sendStopSignal()));

    updater->start();
}

void CGraphWidget::paintEvent(QPaintEvent *)
{

    QPainter widgetPainter(this);
    widgetPainter.drawPixmap(0, 0, *pix);
}

void CGraphWidget::sendStopSignal()
{
    emit stopPressed();
}

void CGraphWidget::updateScene()
{
    if(chnls.count() <= 0)
        return;

    updater->stop();
    int yyy , sz;
    painter.begin(pix);
    painter.setWorldMatrixEnabled(true);
    int tempCnt;

    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    clearRect.setRect( 0 , 0, graphSize.width() , graphSize.height());
    painter.drawRect(clearRect);

    painter.setPen(graphBorderColor);
    painter.setBrush(Qt::NoBrush);
    for(int i=0; i <= 5; i++)
    {
        painter.drawLine(QPoint(sideMargin+rightMargin,sideMargin+(graphSize.height()/5)*i),
                         QPoint(sideMargin+rightMargin+graphSize.width(),sideMargin+(graphSize.height()/5)*i));
    }

    for(int i = 0 ; 50*i <= graphSize.width(); i++)
    {
        painter.drawLine(QPoint((50)*i+sideMargin+rightMargin,sideMargin),
                         QPoint((50)*i+sideMargin+rightMargin,graphSize.height()+sideMargin));
    }

    for( tempCnt = 0 ; tempCnt<chnls.count() ; tempCnt++ ){

        yyy = -1*chnls[tempCnt].value*chnls[tempCnt].gain-chnls[tempCnt].offset+(graphSize.height()/2);
        if( chnls[tempCnt].history.size() >= 500 )
            chnls[tempCnt].history.pop_front();
        chnls[tempCnt].history.push_back(yyy);

        graphPen.setColor(chnls[tempCnt].color);
        graphPen.setStyle(Qt::SolidLine);
        graphPen.setWidth(1);
        painter.setPen(graphPen);
        painter.setBrush(Qt::NoBrush);

        sz = chnls[tempCnt].history.size()-1;

        for( int j=sz ; j>0 ; j-- )
            painter.drawLine(QPoint( (j+1) ,chnls[tempCnt].history.at(j)), QPoint(j,chnls[tempCnt].history.at(j-1)));
    }

    painter.setWorldMatrixEnabled(false);
    painter.end();
    this->repaint();
    updater->start();
}

void CGraphWidget::setValueForChnl( QString _title , double val)
{
    for( int i=0 ; i<chnls.count() ; i++ )
        if( chnls[i].title == _title ){
            chnls[i].value=val;
            return;
        }
}

void CGraphWidget::render()
{
}

void CGraphWidget::initScene()
{
    painter.begin(pix);

    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    clearRect.setRect( 0 , 0, graphSize.width() , graphSize.height());
    painter.drawRect(clearRect);

    painter.setPen(graphBorderColor);
    painter.setBrush(Qt::NoBrush);
    for(int i=0; i <= 5; i++)
    {
        painter.drawLine(QPoint(sideMargin+rightMargin,sideMargin+(graphSize.height()/5)*i),
                         QPoint(sideMargin+rightMargin+graphSize.width(),sideMargin+(graphSize.height()/5)*i));
    }

    for(int i =0; 50*i <= graphSize.width(); i++)
    {
        painter.drawLine(QPoint((50)*i+sideMargin+rightMargin,sideMargin),
                         QPoint((50)*i+sideMargin+rightMargin,graphSize.height()+sideMargin));
    }

    painter.end();
    this->repaint();
}

void CGraphWidget::addChnl(QString _title, QColor _color, double _gain, double _offset)
{
    chnls.append(CGraphChnls(_title));
    chnls.last().color=_color;
    chnls.last().gain=_gain;
    chnls.last().offset=_offset;
    chnls.last().history.push_back(graphSize.height()/2);
    chnls.last().history.push_back(graphSize.height()/2);
}

void CGraphWidget::delChnl( QString _title ){
    for( int i=0 ; i<chnls.count() ; i++ )
        if( chnls[i].title == _title )
            chnls.erase(chnls.begin() + i);
}

CGraphWidget::~CGraphWidget()
{

}
