#include "skillsthread.h"

#include <agent.h>
skillsThread::skillsThread(QObject *parent, int _thId) :
    QThread(parent)
{
    threadId = _thId;
    timer = new QTimer(this);
    timer->setInterval(16);
    connect(timer,SIGNAL(timeout()),this,SLOT(mainLoop()));
    loopTime.start();
}



void skillsThread::run()
{
    timer->start();
}

void skillsThread::mainLoop()
{

    emit robotVel(threadId, 0.6, 0.4, 0.5);
    //debug(QString("lopp : %1").arg(loopTime.elapsed()),D_MHMMD);
    loopTime.restart();
}

void skillsThread::upadateWM()
{

}

void skillsThread::initSkill(selectedSkill _skill)
{

}
