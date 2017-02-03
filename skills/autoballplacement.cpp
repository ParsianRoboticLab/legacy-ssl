#include "autoballplacement.h"
#include "gotoball.h"
#include <QDebug>

INIT_SKILL(CSkillAutoBallPlacement, "Auto ball Placement");

CSkillAutoBallPlacement::CSkillAutoBallPlacement(CAgent *_agent) : CSkill(_agent)
{
    isFinished = false;
    GPA = new CSkillGotoPointAvoid(_agent);
    kick = new CSkillKick(_agent);
    bangBang = new CNewBangBang();
    agent = _agent;
    ballCounter = 0;
    ballCatchFlag = false;
}

double CSkillAutoBallPlacement::progress()
{
    return 0;
}

CSkillAutoBallPlacement::~CSkillAutoBallPlacement()
{
    delete GPA;
    delete kick;
    delete bangBang;
}

void CSkillAutoBallPlacement::gotoBall()
{
    Circle2D dribblerArea (agent->pos() + agent->dir().norm()*0.09,0.1);
    double vx,vy,w;
    kick->setTarget( target);
    kick->setShotToEmptySpot(false);
    kick->setSlow(true);
    kick->setVeryFine(true);
    if(dribblerArea.contains(wm->ball->pos))
    {
        ballCounter++;
    } else
         {
             ballCounter= 0;
         }

    if(ballCounter < 50)
    {
        kick->setKickSpeed(0);
        kick->setSpin(1);
        kick->execute();
    } else if(ballCounter < 80)
         {
             kick->setKickSpeed(0);
             kick->setSpin(5);
             kick->execute();
         } else
              {
                  draw(target);
                  agent->setRoller(5);
                  bangBang->setAngInPath(true);
                  bangBang->setSlow(true);
                  bangBang->setAngKp(1);
                  bangBang->setAccMax(0.3);
                  bangBang->setDecMax(1);
                  bangBang->setVelMax(1);
                  bangBang->bangBangSpeed(agent->pos(),agent->vel(),agent->dir(),
                                          target,wm->ball->pos - agent->pos(),0,0.016,vx,vy,w);
                  agent->setRobotAbsVel(vx,vy,w);
              }

    return;

    if(ballCounter > 20 && ballCounter <=80)
    {
        kick->setSpin(7);
        kick->setKickSpeed(0);
    } else if(ballCounter > 80)
         {
             kick->setSpin(7);
             kick->setKickSpeed(350);
         } else
              {
                  kick->setSpin(1);
                  kick->setKickSpeed(0);
              }

    if(target.dist(agent->pos())< 0.05)
    {
        kick->setSpin(0);
        kick->setKickSpeed(0);
    } else if(target.dist(agent->pos())<1)
         {
             kick->setSpin(1);
             kick->setKickSpeed(0);
             kick->execute();
         } else
              {
                 kick->execute();
              }
    kick->setChip(true);
    kick->setGoalieMode(false);

//    Vector2D finalPos;
//    finalPos = ballPos + (ballPos - target).norm() * 0.12;
//    GPA->init(finalPos,ballPos - finalPos);
//    GPA->setAvoidBall(true);
//    GPA->setBallObstacleRadius(0.2);//0.2
//    GPA->setSlowMode(true);
//    GPA->setVerySlow(true);
//    GPA->execute();
    if(target.dist(wm->ball->pos) < 0.05)
        {
            agent->setRoller(0);
            agent->setRobotAbsVel(0,0,0);
            isFinished = true;
        }
}

void CSkillAutoBallPlacement::gotoTarget()
{
    Vector2D hum(target);
    Vector2D finalPos;
    finalPos = ballPos + (ballPos - target).norm() * 0.000001;
    GPA->init(finalPos,ballPos - finalPos);
    GPA->setAvoidBall(true);
    GPA->setBallObstacleRadius(0.1);///0.2
    GPA->setSlowMode(true);
    GPA->setVerySlow(false);
    GPA->execute();
    if(target.dist(ballPos)> 1) {
        agent->setRoller(1);///1
    } else
         {
            agent->setRoller(0);
         }
    if(target.dist(ballPos)>1.5) {
        target=(agent->pos().norm()*0.01);
        agent->setChip(300);
    } else
         {
            agent->setChip(0);
            target=hum;
         }
}

void CSkillAutoBallPlacement::execute()
{
    kick->setAgent(agent);
    gotoBall();
    debug(QString("tar : %1 , %2").arg(target.x).arg(target.y), D_Mahmood);

return;

    ballPos = wm->ball->pos;
    agentPos = agent->pos();
    Circle2D kickerArea(agentPos + agent->dir().norm()*0.12, 0.12);//0.11/0.1
    if(kickerArea.contains(ballPos) && target.dist(wm->ball->pos) > 0.02)//0.1
    {
        gotoTarget();
        isFinished = false;
    } else if(target.dist(wm->ball->pos) > 0.1)//0.03
        {
            gotoBall();
            isFinished= false;
        } else if(target.dist(wm->ball->pos) < 0.5)
             {
                agent->setRoller(0);
                agent->setRobotAbsVel(0,0,0);
                isFinished = true;
             } else
                  {
                     //agent->setRoller(0);
                     //agent->setRobotAbsVel(0,0,0);
                     //isFinished = true;
                  }
    debug(QString("dist: %1").arg(target.dist(wm->ball->pos)),D_Mahmood);
}
