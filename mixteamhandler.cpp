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
}

void CMixTeamHandler::master()
{
    if(knowledge->getGameState() == CKnowledge::Stop){
        debug("initial : stop", D_ATOUSA);
//        initialPositioning();
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

}

void CMixTeamHandler::task1positioning()
{
    draw(Circle2D(wm->field->ourGoal(),1.5), "cyan");
    double radius = 1.5;
    Vector2D vec;
    double angle = 0;
    //robots = 10 --> 0.5
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
        planLoc[i] = plans[i]->mutable_shot_target();
        posLoc[i] = poses[i]->mutable_loc();
    }
    double py = -3;
    for( int i = 0 ; i < ROBOTS_IN_FIELD/2 ; i++){
        posLoc[i]->set_x(wm->field->ourGoal().x + 3.5);
        py += 1;
        posLoc[i]->set_y(py);
        draw(Circle2D(Vector2D(posLoc[i]->x(), posLoc[i]->y()), 0.1), "red");
    }
    py = -3;
    for( int i = ROBOTS_IN_FIELD/2 ; i < ROBOTS_IN_FIELD ; i++){
        posLoc[i]->set_x(wm->field->oppGoal().x - 3.5);
        py += 1;
        posLoc[i]->set_y(py);
        draw(Circle2D(Vector2D(posLoc[i]->x(), posLoc[i]->y()), 0.1), "red");
    }
    py = -3;
    for( int i = 0 ; i < ROBOTS_IN_FIELD/2 ; i++){
        planLoc[i]->set_x(wm->field->ourGoal().x + 3.5);
        py += 1;
        planLoc[i]->set_y(py);
        draw(Circle2D(Vector2D(posLoc[i]->x(), planLoc[i]->y()), 0.1), "red");
    }
    py = -3;
    for( int i = ROBOTS_IN_FIELD/2 ; i < ROBOTS_IN_FIELD ; i++){
        planLoc[i]->set_x(wm->field->oppGoal().x - 3.5);
        py += 1;
        planLoc[i]->set_y(py);
        draw(Circle2D(Vector2D(posLoc[i]->x(), planLoc[i]->y()), 0.1), "red");
    }

    static MixTeamSender *sender = new MixTeamSender();
    sender->packet = packet;
    sender->flag = true;

}



void CMixTeamHandler::slave()
{
    if(knowledge->getGameState() == CKnowledge::Stop){
        debug("initial : stop", D_ATOUSA);
        stopSlavePositioning();
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

void CMixTeamHandler::stopSlavePositioning()
{
    if( knowledge->ready ){
        for(int  i = 0 ;  i < knowledge->kPlans->plans_size() ; i++){
            multi_team_comm::RobotPlan ptemp = knowledge->kPlans->plans(i);
            if(isOurAgent(ptemp.robot_id())){
                debug(QString("i : %1").arg(i), D_ATOUSA);

            }
        }
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
