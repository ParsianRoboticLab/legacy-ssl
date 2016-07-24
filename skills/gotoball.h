#ifndef GOTOBALL_H
#define GOTOBALL_H

#include <gotopoint.h>
#include <trackcurve.h>
#include "knowledge.h"
#include "pid.h"

class CCurveAroundBall : public CCurve
{
public:
    virtual double X(double t) {return sqrt(fabs(1-t))*cos(M_PI*(t-1)*(t-1));}
    virtual double Y(double t) {return sqrt(fabs(1-t))*sin(M_PI*(t-1)*(t-1));}
};

class CSkillGotoBall : public CSkill
{
protected:
    CSkillGotoPointAvoid* gotopointavoid;
    CSkillTurn turnskill;
//    CSkillFollowPoints* followpoints;
//    CSkillTrackCurve* trackcurve;
    CSkillGotoPoint* gotopoint;
	Vector2D projection;
    void goBehindTheBall(Vector2D bias);
    bool inited, wasFollowing, behindIt, reached;
//    int ss;
    Vector2D bp;
//    Vector2D tangentToCircle;
    int sz;
//    CCurveAroundBall ar;
    CPID *pidP,*pidW;
    bool noKick,GTPM;
    double ang;
    bool rot;
    bool turnEnough;
public:
    DEF_SKILL(CSkillGotoBall);
    CSkillThroughModeController *throughC;
    void resetI();
    Vector2D behindTheBall(double dist = 0.0);
    Vector2D behindTheBallDir();
    virtual void generateFromConfig(CAgent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    /**/SkillProperty(CSkillGotoBall, bool, RollingBallOnly, rollingBallOnly);
	/**/SkillProperty(CSkillGotoBall, bool, UseCurve, useCurve);
    /**/SkillProperty(CSkillGotoBall, bool, UsePoints, usePoints);
    /**/SkillProperty(CSkillGotoBall, bool, UseDynamicPoints, useDynamicPoints);
    SkillProperty(CSkillGotoBall, bool, UseControler, useControler);
    SkillProperty(CSkillGotoBall, int, Spin, spin);
    SkillProperty(CSkillGotoBall, bool, Slow, slow);
    SkillProperty(CSkillGotoBall, bool, Rotate, rotate);
    SkillProperty(CSkillGotoBall, bool, Chip, chip);
    SkillProperty(CSkillGotoBall, bool, Turn, turn);
    /**/SkillProperty(CSkillGotoBall, bool, Block, block);
    /**/SkillProperty(CSkillGotoBall, bool, Calculated, calculated);
    /**/SkillProperty(CSkillGotoBall, double, StepBack, stepback);
    SkillProperty(CSkillGotoBall, double, BallMargin, ballmargin);
    SkillProperty(CSkillGotoBall, double, FinalVel, finalVel);
    SkillProperty(CSkillGotoBall, double, AroundBallVel, aroundBallVel);
    SkillProperty(CSkillGotoBall, bool, Clear, clear);
    SkillProperty(CSkillGotoBall, bool, ThroughMode, throughMode);
    SkillProperty(CSkillGotoBall, bool, PenaltyMode, penaltyMode);
    SkillPropertyNoSet(CSkillGotoBall, Vector2D, Goal, goal);
    SkillProperty(CSkillGotoBall, bool, Struggle, struggle);
    SkillProperty(CSkillGotoBall, bool, AvoidPenaltyArea, avoidPenaltyArea);
    PropertyGet(bool, CatchingBall, catchingBall);
};

#endif // GOTOBALL_H
