#include "autoballplacement.h"
#include "gotoball.h"
#include <QDebug>

INIT_SKILL(CSkillAutoBallPlacement, "Auto ball Placement");

CSkillAutoBallPlacement::CSkillAutoBallPlacement(CAgent *_agent) : CSkill(_agent)
{
    isFinished = false;
    GPA = new CSkillGotoPointAvoid(_agent);
    agent = _agent;
}

double CSkillAutoBallPlacement::progress()
{
    return 0;
}

CSkillAutoBallPlacement::~CSkillAutoBallPlacement()
{
    delete GPA;
}
void CSkillAutoBallPlacement::gotoBall()
{
    Vector2D finalPos;
    finalPos = ballPos + (ballPos - target).norm() * 0.12;
    GPA->init(finalPos,ballPos - finalPos);
    GPA->setAvoidBall(true);
    GPA->setBallObstacleRadius(0.2);//0.2
    GPA->setSlowMode(true);
    GPA->setVerySlow(true);
    if(agent->pos().dist(ballPos)>2) {
        agent->setRoller(7);

    } else {
      agent->setRoller(0);
      }
    GPA->execute();
}
void CSkillAutoBallPlacement::gotoTarget()
{
    Vector2D finalPos;
    finalPos = ballPos + (ballPos - target).norm() * 0.000001;
    GPA->init(finalPos,ballPos - finalPos);
    GPA->setAvoidBall(true);
    GPA->setBallObstacleRadius(0.2);
    GPA->setSlowMode(true);
    GPA->setVerySlow(false);
    GPA->execute();
    if(target.dist(ballPos)< 0.5) {
        agent->setRoller(0);///1
    } else {
      agent->setRoller(0);
      }

    if(target.dist(ballPos)>1) {
        agent->setRoller(2);

    } else {
      agent->setRoller(0);
      }
    if(target.dist(ballPos)>1.5) {

        agent->setChip(300);

    } else {
      agent->setChip(0);

      }
}

void CSkillAutoBallPlacement::execute()
{
    ballPos = wm->ball->pos;
    agentPos = agent->pos();
    Circle2D kickerArea(agentPos + agent->dir().norm()*0.11, 0.1);
    if(kickerArea.contains(ballPos) && target.dist(wm->ball->pos) > 0.1)
    {
        gotoTarget();
        isFinished = false;
    }
    else if(target.dist(wm->ball->pos) > 0.03)
    {
        gotoBall();
        isFinished= false;
    }
    else
    {
        agent->setRoller(0);
        agent->setRobotAbsVel(0,0,0);
        isFinished = true;
    }
    debug(QString("dist: %1").arg(target.dist(wm->ball->pos)),D_MHMMD);
}
