#ifndef MOTIONTRAINER_H
#define MOTIONTRAINER_H

#include "geom.h"
#include "agent.h"
#include "worldmodel.h"
#include "drawer.h"
#include <QTime>
#include <QFile>
#include "skills.h"
#include "motionestimator.h"

class CMotionTrainer
{
public:
    Vector2D pos,lpos,lvel,mid;
    QList<TrainStruct> trainlist;
    CWorldModel* wm;
    CAgent ** agents;
    int mode;
    int counter;
    QTime timer;
public:
     CSkillGotoPointAvoid* gotopoint;
    int id;
    int rmax;
    int rmin;
    int x1;
    int y1;
    int x2;
    int y2;
    bool onlyv2zero;
    CMotionTrainer(CWorldModel* _wm,CAgent **_agents);
    ~CMotionTrainer();
    void moveRobotToRandomPoints();
    void save(QString filename);
};

struct BallTrainStruct
{
	double x,v,a,t;
    int num;
};

class CBallTrainer
{
    CWorldModel* wm;
    CAgent **agents;
    QList<BallTrainStruct> trainlist;
    int tt,num;
    Vector2D pos0;
    QTime timer;
    bool isViaSim;
public:
    CBallTrainer(CWorldModel* _wm, CAgent **_agents);
    void save(QString filename);
    void record();
    void reset(Vector2D p);
};

#endif // MOTIONTRAINER_H
