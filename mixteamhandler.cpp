#include "mixteamhandler.h"

CMixTeamHandler::CMixTeamHandler()
{
    for(int i = 0 ; i < 10 ; i++){
        agentIDs[i] = i+1;
    }
}

void CMixTeamHandler::master()
{
    if(knowledge->getGameState() == CKnowledge::Stop){
        debug("initial : stop", D_ATOUSA);
        initialPositioning();
        initialSlaveMakePacket();

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

void CMixTeamHandler::task1positioning()
{
    draw(Circle2D(wm->field->ourGoal(),1.5), "cyan");
    double radius = 1.5;
    Vector2D vec;
    double angle = 0;
    for( int i = 0 ; i < 5 ; i++ ){
        angle += 0.5;
        ourPos[i] = getXYByAngleOurGoal(angle, radius);
        slavePos[i] = Vector2D(-ourPos[i].x, ourPos[i].y);
        draw(Circle2D(slavePos[i],0.1), "red");
        draw(Circle2D(ourPos[i],0.1), "red");
    }
}

void CMixTeamHandler::initialSlaveMakePacket()
{
    multi_team_comm::TeamPlan *packet = new multi_team_comm::TeamPlan();
    multi_team_comm::RobotPlan *plans[10]; //no plan for goali
    multi_team_comm::Pose *poses[10];
    multi_team_comm::Location *posLoc[10];
    multi_team_comm::Location *planLoc[10];
    for( int i = 0 ; i < 10 ; i++ ){
        plans[i] = packet->add_plans();
        plans[i]->set_robot_id(agentIDs[i]);
//        plans[i]->set_role(multi_team_comm::RobotPlan::Defense);//not important here
        poses[i] = plans[i]->mutable_nav_target();
        planLoc[i] = plans[i]->mutable_shot_target();
        posLoc[i] = poses[i]->mutable_loc();
    }
    double py = -3;
    for( int i = 0 ; i < 5 ; i++){
        posLoc[i]->set_x(wm->field->ourGoal().x + 3.5);
        py += 1;
        posLoc[i]->set_y(py);
        draw(Circle2D(Vector2D(posLoc[i]->x(), posLoc[i]->y()), 0.1), "red");
    }
    py = -3;
    for( int i = 5 ; i < 10 ; i++){
        posLoc[i]->set_x(wm->field->oppGoal().x - 3.5);
        py += 1;
        posLoc[i]->set_y(py);
        draw(Circle2D(Vector2D(posLoc[i]->x(), posLoc[i]->y()), 0.1), "red");
    }
    py = -3;
    for( int i = 0 ; i < 5 ; i++){
        planLoc[i]->set_x(wm->field->ourGoal().x + 3.5);
        py += 1;
        planLoc[i]->set_y(py);
        draw(Circle2D(Vector2D(posLoc[i]->x(), planLoc[i]->y()), 0.1), "red");
    }
    py = -3;
    for( int i = 5 ; i < 10 ; i++){
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

}
