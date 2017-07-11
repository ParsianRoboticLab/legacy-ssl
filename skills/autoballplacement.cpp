#include "autoballplacement.h"
#include "gotoball.h"
#include <QDebug>

INIT_SKILL(CSkillAutoBallPlacement, "Auto ball Placement");

CSkillAutoBallPlacement::CSkillAutoBallPlacement(CAgent *_agent) : CSkill(_agent)
{
    isFinished = false;
    kick = new CSkillKick(_agent);
    kick->setAvoidOppPenaltyArea(false);
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
    Circle2D dribblerArea (agent->pos() + agent->dir().norm()*0.09,0.15);
    kick->setTarget(target);
    kick->setShotToEmptySpot(false);
    kick->setSlow(true);
    kick->setVeryFine(true);
    kick->setSpin(3);
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
        kick->setSpin(3);
        kick->execute();
    } else if(ballCounter < 80 && isFinished == false)
         {
             kick->setKickSpeed(0);
             kick->setSpin(6);
             kick->execute();
         } else
              {
                  agent->setRoller(5);
                  gotoTarget();
              }


}


void CSkillAutoBallPlacement::gotoTarget()
{
    double vx,vy,w;
    draw(target);
    agent->setRoller(5);////inja
    bangBang->setAngInPath(true);
    bangBang->setSlow(true);
    bangBang->setAngKp(1);
    bangBang->setAccMax(0.3);
    bangBang->setDecMax(1);
    bangBang->setVelMax(1);
    bangBang->bangBangSpeed(agent->pos(),agent->vel(),agent->dir(),
                            target,wm->ball->pos - agent->pos(),0,0.016,vx,vy,w);


    if(target.dist(wm->ball->pos) < 0.25)
        {
            agent->setRoller(0);
            isFinished = true;

        }
    else
    {
        isFinished = false;
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
    //target = Vector2D(knowledge->getBPPosition().x, knowledge->getBPPosition().y);
    debug(QString("tar : %1 , %2").arg(knowledge->getBPPosition().x).arg(knowledge->getBPPosition().y), D_MAHMOOD);
    draw(Circle2D(target, 0.10), QColor(Qt::cyan));

    gotoBall();

    debug(QString("isFinished : %1 ").arg(isFinished), D_MAHMOOD);

    ballPos = wm->ball->pos;
    agentPos = agent->pos();
    Circle2D kickerArea(agentPos + agent->dir().norm()*0.12, 0.12);//0.11/0.1
    debug(QString("dist: %1").arg(target.dist(wm->ball->pos)),D_MAHMOOD);
}


void autoBallPlacement()
{
    static int id = -1;
    static bool isRun = false;
    static Vector2D lastMousePos;
    Circle2D clear(Vector2D(3,3.5),0.15);
    QList <Circle2D> robotPos;
    robotPos.clear();
    for(int i = 0;i< knowledge->getActiveAgents().count() ; i++)
    {
        if(Circle2D(knowledge->getActiveAgents().at(i)->pos(),0.1).contains(knowledge->getMousePos()) && isRun == false)
        {
            id = knowledge->getActiveAgents().at(i)->id();

            lastMousePos = knowledge->getMousePos();
        }
    }
    draw(clear,QColor(Qt::red),true);
    draw(QString("Id : %1").arg(id),Vector2D(2.8,3));
    if(clear.contains(knowledge->getMousePos()))
    {
        id = -1;
        isRun = false;
    }
    if(knowledge->getMousePos() != lastMousePos && id != -1)
    {
        isRun = true;
    }
    if(isRun && id != -1)
    {
        static CSkillAutoBallPlacement abpm(knowledge->getAgent(id));
        abpm.setAgent(knowledge->getAgent(id));
        abpm.setTarget(knowledge->getMousePos());
        abpm.execute();
    }


}
