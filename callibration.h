#ifndef CALLIBRATION_H
#define CALLIBRATION_H

#include <knowledge.h>
#include <skills.h>
#include <QDebug>

struct BallDistVel {
    double vel, dist;
};

class CKickCallibrator
{
    QList<BallDistVel> ballhist;
    Vector2D kickStartPoint;
    int kicking;
public:
    double kickspeed;
    bool kickSpeedCalculated;
    CSkillKick* kick;
    CSkillGotoPointAvoid* gotopoint;
    CKickCallibrator();
    void calcKickSpeed();
    void execute(int speed=1,bool chip=false,bool spin=false);
    Property(CAgent*, Agent, agent);
    Property(Vector2D, KickPoint, kickPoint);
    Property(Vector2D, Target, target);
};

class CKickCallibratorManiMasoud
{
public:
    CKickCallibratorManiMasoud();
    ~CKickCallibratorManiMasoud();
    double kickspeed;
    bool kickSpeedCalculated;
    CSkillKick* kick;
    CSkillGotoPointAvoid* gotopoint;
    void execute(int speed=1,bool chip=false,bool spin=false);
    QList < QPair<double,double> > Values;
    QTime timeStart;
    Vector2D BallPos;
    Property(CAgent*, Agent, agent);
    Property(Vector2D, KickPoint, kickPoint);
    Property(Vector2D, Target, target);
    Property(bool , EndFlag , endFlag );
    Property(bool , FirstTimeBallPos , firstTimeBallPos );
    Property(int , BallStopCntr , ballStopCntr );
    Property(bool , Finish , finish);
};

#endif // CALLIBRATION_H
