#ifndef KICKPROFILER_H
#define KICKPROFILER_H
#include "mainapplication.h"



using namespace std;
int firstRobotId = -1,secondRobotId = -1;
bool first = true;
bool robotSelection = true;
Vector2D pos1(-2,2),pos2(-2,-2);
int state = 1;
int robot1Speed = 150,robot2Speed = 150;
void CMainApplication::kickProfiler()
{
    if(robotSelection)
    {
        if(first)
        {
            for(int i = 0; i < wm->our.activeAgentsCount() ; i++)
            {
                if(mousePos.dist(wm->our.active(i)->pos) < 0.05 )
                {
                    firstRobotId = wm->our.activeAgentID(i);
                    first = false;
                    break;
                }
            }
        }
        else
        {
            for(int i = 0; i < wm->our.activeAgentsCount() ; i++)
            {
                if(mousePos.dist(wm->our.active(i)->pos) < 0.05 &&  wm->our.activeAgentID(i) != firstRobotId)
                {
                    secondRobotId = wm->our.activeAgentID(i);
                    robotSelection = false;
                    first = true;
                }
            }
        }
    }

    if(Circle2D(Vector2D(4,3.5),0.2).contains(mousePos))
    {
        robotSelection = true;
        firstRobotId = -1,secondRobotId = -1;
    }

    draw(QString("Robot1: %1----Robot2: %2").arg(firstRobotId).arg(secondRobotId),Vector2D(0,1));
    draw(QString("click for new selection"),Vector2D(3,3));
    draw(Circle2D(Vector2D(4,3.5),0.2),QColor(Qt::red),true);



    //////////////////////////////////////////////// main

    if(!robotSelection)
    {
        static CSkillReceivePass myrcvpass(soccer->agents[firstRobotId]);
        static CSkillPass mypass(soccer->agents[secondRobotId]);


        mypass.setSlow(true);
        mypass.setPassProfiler(true);

        if(state == 1)
        {
            myrcvpass.setAgent(soccer->agents[firstRobotId]);
            myrcvpass.setTarget(pos1);
            mypass.setAgent(soccer->agents[secondRobotId]);
            mypass.setTarget(pos1);
            mypass.setReceiverAgent(soccer->agents[firstRobotId]);
            mypass.setChip(false);
            mypass.setKickSpeed(robot1Speed);
            mypass.execute();

            if(mypass.getPassComplited())
            {

                if(wm->ball->vel.length()< 0.2)
                {


                    state =2;
                    mypass.reset();
                    robot1Speed +=50;
                }
            }

        }
        else
        {
            myrcvpass.setAgent(soccer->agents[secondRobotId]);
            myrcvpass.setTarget(pos1);
            mypass.setAgent(soccer->agents[firstRobotId]);
            mypass.setTarget(pos2);
            mypass.setReceiverAgent(soccer->agents[secondRobotId]);
            mypass.setChip(false);
            mypass.setKickSpeed(robot2Speed);
            mypass.execute();

            if(mypass.getPassComplited())
            {

                if(wm->ball->vel.length()< 0.2)
                {


                    state =1;
                    mypass.reset();
                    robot2Speed +=50;
                }
            }

        }

    }

    return;
}

#endif // MERGECAMERASEXPERIMENT_H

