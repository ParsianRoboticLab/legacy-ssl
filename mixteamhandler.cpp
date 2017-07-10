#include "mixteamhandler.h"

CMixTeamHandler::CMixTeamHandler()
{
    reader = new MixTeamReader();
    for(int i = 0 ; i < ROBOTS_IN_FIELD ; i++){//set all robot IDs
        allAgentIDs[i] = i+1;
    }
    for(int i = 0 ; i < OUR_ROBOTS_IN_FIELD ; i++){//set our robot IDs
        ourAgentIDs[i] = 2*i+1;
    }

    for(int i = 0 ; i < MAX_OUR_ROBOTS_IN_FIELD; i++ )
//        ourRols[i] = new CRolePlayOff();
        ourRols[i] = new CSkillGotoPoint(NULL);
    goalieRole = new CRolePlayOff();
}

///////////////////master/////////////////////
void CMixTeamHandler::master()
{
    if(knowledge->getGameState() == CKnowledge::Stop){
        debug("initial : stop", D_ATOUSA);
        initialPositioning();
        initialSlaveMakePacket();
    }
    else if(knowledge->getGameState() == CKnowledge::Start){ //force start
        debug("task1 : forceStart", D_ATOUSA);
        task1positioning();
        task1MakePacket();
    }
    else if(knowledge->getGameState() == CKnowledge::TheirIndirectKick){
        debug("task2 : theirIndirect", D_ATOUSA);

    }
    else if(knowledge->getGameState() == CKnowledge::OurIndirectKick){
        debug("task3 : ourIndirect", D_ATOUSA);

    }
}

void CMixTeamHandler::initialPositioning()
{
    int py = 0;
    if( (ROBOTS_IN_FIELD+1)/2 == 5 || (ROBOTS_IN_FIELD+1)/2 == 4)
        py = -3;
    else if( (ROBOTS_IN_FIELD+1)/2 == 3 || (ROBOTS_IN_FIELD+1)/2 == 2)
        py = -2;
    else if( (ROBOTS_IN_FIELD+1)/2 == 1 || (ROBOTS_IN_FIELD+1)/2 == 0)
        py = -1;

    for( int i = 0 ; i < (ROBOTS_IN_FIELD+1)/2 ; i++ ){
        if( (ROBOTS_IN_FIELD+1)/2 == 5 || (ROBOTS_IN_FIELD+1)/2 == 3 || (ROBOTS_IN_FIELD+1)/2 == 1 )
            py += 1;
        else if( (ROBOTS_IN_FIELD+1)/2 == 4 || (ROBOTS_IN_FIELD+1)/2 == 2 || (ROBOTS_IN_FIELD+1)/2 == 0 ){
            if(py == -1)
                py = 1;
            else
                py += 1;
        }

        allPositions[i] = Vector2D(wm->field->ourGoal().x + 3.5, py);
        allPositions[ROBOTS_IN_FIELD-i-1] = Vector2D(-allPositions[i].x, allPositions[i].y);
        draw(Circle2D(allPositions[i],0.1), "red");
        draw(Circle2D(allPositions[ROBOTS_IN_FIELD-i-1],0.1), "red");
    }
}

void CMixTeamHandler::initialSlaveMakePacket()
{
    multi_team_comm::TeamPlan *packet = new multi_team_comm::TeamPlan();
    multi_team_comm::RobotPlan *plans[ROBOTS_IN_FIELD]; //no plan for goali
    multi_team_comm::Pose *poses[ROBOTS_IN_FIELD];
    multi_team_comm::Location *posLoc[ROBOTS_IN_FIELD];
    multi_team_comm::Location *planLoc[ROBOTS_IN_FIELD];
    for( int i = 0 ; i < ROBOTS_IN_FIELD ; i++ ){
        plans[i] = packet->add_plans();
        plans[i]->set_robot_id(allAgentIDs[i]);
//        plans[i]->set_role(multi_team_comm::RobotPlan::Defense);//not important here
        poses[i] = plans[i]->mutable_nav_target();
//        planLoc[i] = plans[i]->mutable_shot_target();
//        planLoc[i] = allPositions[i];
        posLoc[i] = poses[i]->mutable_loc();
        posLoc[i]->set_x(allPositions[i].x);
        posLoc[i]->set_y(allPositions[i].y);
    }

    static MixTeamSender *sender = new MixTeamSender();
    sender->packet = packet;
    sender->flag = true;
}


void CMixTeamHandler::task1positioning()
{
    draw(Circle2D(wm->field->ourGoal(),1.5), "cyan");
    double radius = 1.5;
    Vector2D vec;
    double angle = 0;
    for( int i = 0 ; i < (ROBOTS_IN_FIELD+1)/2 ; i++ ){
        if( (ROBOTS_IN_FIELD+1)/2 == 5 )
            angle += 0.5;
        else if( (ROBOTS_IN_FIELD+1)/2 == 4 )
            angle += 0.6;
        else if( (ROBOTS_IN_FIELD+1)/2 == 3 )
            angle += 0.78;
        else if( (ROBOTS_IN_FIELD+1)/2 == 2 )
            angle += 1.05;
        else if( (ROBOTS_IN_FIELD+1)/2 == 1 )
            angle += 1.57;
        else if( (ROBOTS_IN_FIELD+1)/2 == 0 )
            angle += 0;
        allPositions[i] = getXYByAngleOurGoal(angle, radius);
        allPositions[ROBOTS_IN_FIELD-i-1] = Vector2D(-allPositions[i].x, allPositions[i].y);
        draw(Circle2D(allPositions[i],0.1), "red");
        draw(Circle2D(allPositions[ROBOTS_IN_FIELD-i-1],0.1), "red");
    }
}


Vector2D CMixTeamHandler::getXYByAngleOurGoal(double _angle, double _radius)
{
    double tempX;
    double tempY;
    _angle += _PI/2;
    tempX = _radius*cos(_PI-_angle);
    tempY = _radius*sin(_angle);
    tempX += wm->field->ourGoal().x - 0.2;
    if (tempX < wm->field->ourGoal().x + 0.08) {
        tempX = wm->field->ourGoal().x + 0.08;
    }
    return Vector2D(tempX, tempY);
}

void CMixTeamHandler::task1MakePacket()
{
    multi_team_comm::TeamPlan *packet = new multi_team_comm::TeamPlan();
    multi_team_comm::RobotPlan *plans[ROBOTS_IN_FIELD]; //no plan for goali
    multi_team_comm::Pose *poses[ROBOTS_IN_FIELD];
    multi_team_comm::Location *posLoc[ROBOTS_IN_FIELD];
    multi_team_comm::Location *planLoc[ROBOTS_IN_FIELD];
    for( int i = 0 ; i < ROBOTS_IN_FIELD/2 ; i++ ){
        plans[i] = packet->add_plans();
        plans[i]->set_robot_id(allAgentIDs[i]);
        plans[i]->set_role(multi_team_comm::RobotPlan::Defense);//not important here
        poses[i] = plans[i]->mutable_nav_target();
//        planLoc[i] = plans[i]->mutable_shot_target();
//        planLoc[i] = allPositions[i];
        posLoc[i] = poses[i]->mutable_loc();
        posLoc[i]->set_x(allPositions[i].x);
        posLoc[i]->set_y(allPositions[i].y);
    }
    for( int i = ROBOTS_IN_FIELD/2 ; i < ROBOTS_IN_FIELD ; i++ ){
        plans[i] = packet->add_plans();
        plans[i]->set_robot_id(allAgentIDs[i]);
        plans[i]->set_role(multi_team_comm::RobotPlan::Offense);//not important here
        poses[i] = plans[i]->mutable_nav_target();
//        planLoc[i] = plans[i]->mutable_shot_target();
//        planLoc[i] = allPositions[i];
        posLoc[i] = poses[i]->mutable_loc();
        posLoc[i]->set_x(allPositions[i].x);
        posLoc[i]->set_y(allPositions[i].y);
    }

    static MixTeamSender *sender = new MixTeamSender();
    sender->packet = packet;
    sender->flag = true;
}

void CMixTeamHandler::task2positioning()
{

}

void CMixTeamHandler::task2MakePacket()
{

}

///////////////////slave/////////////////////
void CMixTeamHandler::slave()
{
    if(knowledge->getGameState() == CKnowledge::Stop){
        debug("initial : stop", D_ATOUSA);
        initialReadPacket();
    }
    else if(knowledge->getGameState() == CKnowledge::Start){ //force start
        debug("task1 : forceStart", D_ATOUSA);

    }
    else if(knowledge->getGameState() == CKnowledge::TheirIndirectKick){
        debug("task2 : theirIndirect", D_ATOUSA);

    }
    else if(knowledge->getGameState() == CKnowledge::OurIndirectKick){
        debug("task3 : ourIndirect", D_ATOUSA);

    }
}

void CMixTeamHandler::initialReadPacket()
{
    if( knowledge->ready ){
        int counter = 0;
        for(int  i = 0 ;  i < knowledge->kPlans->plans_size() ; i++){
            multi_team_comm::RobotPlan ptemp = knowledge->kPlans->plans(i);
            if(isOurAgent(ptemp.robot_id())){
                debug(QString("i : %1").arg(ptemp.robot_id()), D_ATOUSA);

                ourRols[counter]->setAgent(knowledge->getAgent(ptemp.robot_id()));
                ourRols[counter]->init(Vector2D(ptemp.nav_target().loc().x(),ptemp.nav_target().loc().y()),Vector2D(ptemp.nav_target().loc().x(),ptemp.nav_target().loc().y()));
//                ourRols[counter]->setTarget(Vector2D(ptemp.nav_target().loc().x(),ptemp.nav_target().loc().y()));
//                ourRols[counter]->setTargetDir(Vector2D(ptemp.nav_target().loc().x(),ptemp.nav_target().loc().y()));
//                ourRols[counter]->setSelectedSkill(roleSkill::GotopointAvoid);

                counter++;
            }
        }
        debug(QString("counter = %1").arg(counter), D_ATOUSA);
        for(int i = 0 ; i < counter ; i++)
            ourRols[i]->execute();
    }
}

bool CMixTeamHandler::isOurAgent(int id)
{
    for(int i = 0 ; i < 5 ; i++){
        if(ourAgentIDs[i] == id)
            return true;
    }
    return false;
}
