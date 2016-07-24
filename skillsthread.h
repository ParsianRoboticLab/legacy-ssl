#ifndef SKILLSTHREAD_H
#define SKILLSTHREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QTime>
#include <geom.h>


#include <skill.h>
#include "trajectoryplanner.h"
#include "planner.h"
#include "pid.h"
#include "mathtools.h"
#include "newbangbang.h"
#include <QTime>
#include <QFile>

#include <gotopoint.h>
#include <kick.h>
#include <gotoball.h>
#include <trackcurve.h>
#include <spin.h>
#include <hittheball.h>

#include "navigation.h"
struct selectedSkill
{
    int id;
    SKILLINITVAL _skill;
};

class skillsThread : public QThread
{
    Q_OBJECT
public:
    skillsThread(QObject *parent, int _thId);
    void run();
private:
    int threadId;
    QTimer *timer;
    QTime loopTime;
signals:
    void robotVel(int _id, double _x, double _y, double _w);
public slots:
    void upadateWM();
    void initSkill(selectedSkill _skill);
    void mainLoop();

};

#endif // SKILLSTHREAD_H
