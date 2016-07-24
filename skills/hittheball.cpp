#include "hittheball.h"

INIT_SKILL(CSkillHitTheBall, "hittheball");  

CSkillHitTheBall::CSkillHitTheBall(CAgent *_agent) : CSkill(_agent)
{    
    gotoball = new CSkillGotoBall(_agent);
    gotopoint = new CSkillGotoPointAvoid(_agent);
    goacrossline = new CSkillGoAcrossLine(_agent);
    kick = new CSkillKick(_agent);
    throughMode = false;
    lastThroughed = false;
    gotoball->setUsePoints(true);
    kick->setDontKick( true);
}

CSkillHitTheBall::~CSkillHitTheBall()
{
    delete gotoball;
    delete gotopoint;
    delete goacrossline;
    delete kick;
}

CSkillHitTheBall* CSkillHitTheBall::setTarget(Vector2D val)
{
    gotoball->setAgent(agent);
    target = val;
    gotoball->setGoal(val);
    kick->setAgent( agent);
    kick->setTarget( target);
    return this;
}

CSkillHitTheBall* CSkillHitTheBall::setThroughMode(bool val)
{
    throughMode = val;
    lastThroughed = false;
    return this;
}

double CSkillHitTheBall::progress()
{
    return 0.0;
}

void CSkillHitTheBall::execute()
{
    gotopoint->setAgent(agent);
    if (!target.valid())
        setTarget(wm->field->oppGoal());
    kick->setAgent(agent);
    kick->setDontKick( true);

#if 1
    gotoball->setAgent(agent);
    gotoball->setFinalVel(1.2); //TODO: optimize this value
    bool throughable = false;    
    if ((wm->ball->vel.length()>0.500 &&
         (fabs(AngleDeg::normalize_angle(wm->ball->vel.th().degree()-Vector2D::dirTo_deg(agent->pos(),target)))<60))
        && (! agent->self()->isBallComingTowardMe(2.0,0.500)))
        {
            throughable = true;
        }

    // TODO: set lastThroughed false in some conditions
    if (throughMode && (throughable || lastThroughed))
    {        
        double asideness = 0.4;
        double boundBehind = 1.0;
        double distBehind = 1.0;
        double distFwdCoef = 1.8;
        double distFwdConst = 0.02;
        double finalVelBallCoef = 1.2;
        double vMax = 3.0;
        double vMaxNorm = 1.0;
        double vBall = wm->ball->getVel();
        if ((throughMode && vBall>0.7 &&
            (fabs(AngleDeg::normalize_angle(wm->ball->vel.th().degree()-Vector2D::dirTo_deg(agent->pos(),target)))<45)) || lastThroughed)
        {
            lastThroughed = true;
            double dist = (wm->ball->getProjectionOfPointOnBallVeclocityDirection
                           (agent->pos()+(target-agent->pos()).norm()*CRobot::robot_radius_old)
                           -wm->ball->pos).innerProduct(wm->ball->vel.norm());
            bool flag = dist>0.0;
            double aside = 0;
            double vv=0;
            if (flag)
            {
                aside = asideness*CRobot::robot_radius_old;
                vv = vBall*(1.0-min(fabs(dist),boundBehind)/boundBehind);
                dist = distBehind;
            }
            else
            {
                vv = vMax;
                dist *= distFwdCoef;
                dist -= distFwdConst;
            }
            goacrossline->setOrigin(wm->ball->pos - (target-agent->pos()).norm()*(agent->self()->centerFromKicker() +CBall::radius));
            goacrossline->setDirection(wm->ball->vel.norm());
            goacrossline->setTarget(fabs(dist));
            goacrossline->setVTanMax(vv);
            goacrossline->setVTanFinal(vBall*finalVelBallCoef);
            goacrossline->setVNormMax(vMaxNorm);
            goacrossline->setLookAt(target);
            goacrossline->setAside(aside);
            goacrossline->setAgent(agent);
            goacrossline->execute();
            if (agent->self()->isBallOwner(0.1) &&
                fabs(AngleDeg::normalize_angle((agent->dir().th().degree()-Vector2D::dirTo_deg(agent->pos(),target))))<12 )
            {
                gotopoint->setTargetLook(wm->ball->pos, target)->setFinalVel(target)->execute();
            }
            return;
        }
    }
    if (agent->self()->isBallOwner(0.1) && (fabs(Vector2D::angleBetween((wm->ball->pos - agent->self()->pos).norm(), agent->self()->dir).degree())<12)
        && (fabs(Vector2D::angleBetween(agent->dir(), target - agent->pos()).degree()) < 20)
        )
    {        
        gotopoint->setTargetLook(wm->ball->pos, target)->setFinalVel(target)->execute();
    }
    gotoball->setSpin(false);
    gotoball->execute();
#endif


}
