#include "trackcurve.h"
#include "trajectoryplanner.h"
#include <QDebug>

INIT_SKILL(CSkillTrackCurve, "trackcurve");  

CSkillTrackCurve::CSkillTrackCurve(CAgent *_agent) : CSkill(_agent)
{
    bangBang = new CNewBangBang();
    curvePID = new _PID(1,0,0,0,0);
}

CSkillTrackCurve::~CSkillTrackCurve()
{

}




void CSkillTrackCurve::execute()
{

    agentPos = agent->pos();
    agentDir = agent->dir();
    agentVel = agent->vel();
    rad = orig.dist(agentPos);
    double vf;
    curvePID->error = desiredRad - rad;
    rad = rad + curvePID->PID_OUT();
    curve.assign(orig,rad);

    Vector2D dummyVec;
    Segment2D finalPosSeg,nextSeg;

    finalPosSeg.assign(orig,orig+Vector2D(cos(endAng.radian()),sin(endAng.radian())).norm()*(rad+0.1));
    curve.intersection(finalPosSeg,&finalPos,&dummyVec);

    draw(finalPosSeg,QColor(Qt::green));

    draw(agentPos);
    draw(finalPos);

    draw(curve,QColor(Qt::blue));

    ////////////////////find nextPos

    AngleDeg agentAng,nextAng;
    agentAng = (agentPos - orig).th();
    if(CW) {
        nextAng = agentAng - stepAngel;
    }
    else {
        nextAng = agentAng + stepAngel;
    }
    Circle2D dCircle(orig,desiredRad);
    nextSeg.assign(orig,orig+ Vector2D(cos(nextAng.radian()),sin(nextAng.radian())).norm()*(rad+0.1));
    curve.intersection(nextSeg,&nextPoint,&dummyVec);
    //////////////////start

    double dVx,dVy,dW;
    bangBang->setAccMax(conf()->BangBang_AccMax()*10);
    bangBang->setDecMax(conf()->BangBang_DecMax());

    if(slow)
    {
        bangBang->setVelMax(1.4);
        bangBang->setSlow(true);
    }
    else
    {
        bangBang->setSlow(false);
        bangBang->setVelMax(conf()->BangBang_VelMax());
    }
    bangBang->setVelMax(2);
    vf = 2;
    if(agentPos.dist(finalPos) < 0.3)
    {
        nextPoint = finalPos;
        vf = 0;
    }
    bangBang->bangBangSpeed(agentPos,agentVel,agentDir,nextPoint,robotAng,vf,0.016,dVx,dVy,dW);
    agent->setRobotAbsVel(dVx,dVy ,dW);
    agent->_ACC = 0;
    agent->_DEC = 0;



}

double CSkillTrackCurve::progress()
{
    return 0.0;
}
