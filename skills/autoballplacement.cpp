#include "autoballplacement.h"
#include "gotoball.h"
#include <QDebug>

INIT_SKILL(CSkillAutoBallPlacement, "Auto ball Placement");

CSkillAutoBallPlacement::CSkillAutoBallPlacement(CAgent *_agent) : CSkill(_agent)
{
    isFinished = false;
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
    delete kick;
    delete bangBang;
}

void CSkillAutoBallPlacement::gotoBall()
{
    Circle2D dribblerArea (agent->pos() + agent->dir().norm()*0.09,0.1);
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

    if(ballCounter < 50 && isFinished == false)
    {
        kick->setKickSpeed(0);
        kick->setSpin(1);
        kick->execute();
    } else if(ballCounter < 80 && isFinished == false)
         {
             kick->setKickSpeed(0);
             kick->setSpin(knowledge->agentSetRollerABPbB[0]);
             kick->execute();
         } else
              {
                  gotoTarget();
              }

//    return;

//    if(ballCounter > 20 && ballCounter <=80)
//    {
//        kick->setSpin(7);
//        kick->setKickSpeed(0);
//    } else if(ballCounter > 80)
//         {
//             kick->setSpin(7);
//             kick->setKickSpeed(350);
//         } else
//              {
//                  kick->setSpin(1);
//                  kick->setKickSpeed(0);
//              }

//    if(target.dist(agent->pos())< 0.05)
//    {
//        kick->setSpin(0);
//        kick->setKickSpeed(0);
//    }// else if(target.dist(agent->pos())<1)
     //    {
     //        kick->setSpin(1);
     //        kick->setKickSpeed(0);
     //        kick->execute();
     //    } else
     //         {
     //            kick->execute();
     //         }
   // kick->setChip(true);
   // kick->setGoalieMode(false);

    if(target.dist(wm->ball->pos) < 0.05)
        {
            agent->setRoller(0);
            agent->setRobotAbsVel(0,0,0);
            isFinished = true;
        }
}


void CSkillAutoBallPlacement::gotoTarget()
{
    double vx,vy,w;
    draw(target);
    agent->setRoller(knowledge->agentSetRollerABPbB[0]);////inja
    bangBang->setAngInPath(true);
    bangBang->setSlow(true);
    bangBang->setAngKp(1);
    bangBang->setAccMax(0.3);
    bangBang->setDecMax(1);
    bangBang->setVelMax(1);
    bangBang->bangBangSpeed(agent->pos(),agent->vel(),agent->dir(),
                            target,wm->ball->pos - agent->pos(),0,0.016,vx,vy,w);

    if(target.dist(wm->ball->pos) < 0.2  && isFinished == false)
        {
            agent->setRoller(1);
        }
    if(target.dist(wm->ball->pos) < 0.1)
        {
            agent->setRoller(0);
            isFinished = true;

        }
    agent->setRobotAbsVel(vx,vy,w);
    if( isFinished == true)
    {
        agent->setRobotAbsVel(0,0,0);
    }
}

void CSkillAutoBallPlacement::execute()
{
    kick->setAgent(agent);

    //added
    target = Vector2D(knowledge->getBPPosition().x, knowledge->getBPPosition().y);
    debug(QString("tar : %1 , %2").arg(knowledge->getBPPosition().x).arg(knowledge->getBPPosition().y), D_MAHMOOD);
    draw(Circle2D(target, 0.10), QColor(Qt::cyan));

    gotoBall();

    debug(QString("isFinished : %1 ").arg(isFinished), D_MAHMOOD);

    ballPos = wm->ball->pos;
    agentPos = agent->pos();
    Circle2D kickerArea(agentPos + agent->dir().norm()*0.12, 0.12);//0.11/0.1
    if(kickerArea.contains(ballPos) && target.dist(wm->ball->pos) > 0.02)//0.1
    {
        gotoTarget();
        isFinished = false;
    } else if(target.dist(wm->ball->pos) > 0.1)
        {
            gotoBall();
            isFinished= false;
        } else if(target.dist(wm->ball->pos) < 0.05)
             {
                agent->setRoller(0);
                agent->setRobotAbsVel(0,0,0);
                isFinished = true;
             }


    debug(QString("dist: %1").arg(target.dist(wm->ball->pos)),D_MAHMOOD);
}
