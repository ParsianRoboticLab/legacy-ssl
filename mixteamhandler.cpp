#include "mixteamhandler.h"

CMixTeamHandler::CMixTeamHandler()
{
    reader = new MixTeamReader();
    setOurRobotIDs();

//    for(int i = 0 ; i < ourAgentIDs.size() ; i++){//set our robot IDs
//        qDebug() << "^"<<ourAgentIDs.at(i);
//    }

    for(int i = 0 ; i < MAX_OUR_ROBOTS_IN_FIELD; i++ )
        ourRols[i] = new CSkillGotoPoint(NULL);
}

void CMixTeamHandler::setOurRobotIDs()
{
    QString IDs = QString::fromStdString(conf()->LocalSettings_MixTeamIDs());

    for(int i = 0 ; i < IDs.size() ; i++){
        int num = -1;
        QString sub = IDs.mid(i, 1);
        if( sub == "0") num = 0;
        if( sub == "1") num = 1;
        if( sub == "2") num = 2;
        if( sub == "3") num = 3;
        if( sub == "4") num = 4;
        if( sub == "5") num = 5;
        if( sub == "6") num = 6;
        if( sub == "7") num = 7;
        if( sub == "8") num = 8;
        if( sub == "9") num = 9;
        if( sub == "a") num = 10;
        if( sub == "b") num = 11;
        if( sub == "c") num = 12;
        if( sub == "d") num = 13;
        if( sub == "e") num = 14;
        if( sub == "f") num = 15;

        ourAgentIDs.append(num);
    }
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
    int robotsInField = knowledge->activesInField.size();
    int i;
    for(i = 0 ; i < robotsInField ; i++){
        if(knowledge->activesInField.at(i)->id() == knowledge->mixGoaleID)
            break;
    }
    if( i != robotsInField )
        robotsInField--;

    double px, py, offset;
    px = -1;
    offset = 0.4;
    if(robotsInField % 2 == 0){
        py = 0.2;
        i = 0;
    }
    else{
        allPositions[(robotsInField-1)/2] = Vector2D(px,0);
        draw(Circle2D(allPositions[(robotsInField-1)/2],0.1), "red");
        py = 0.4;
        i = 0;
    }

//    qDebug() << "alaki " << robotsInField;

    for(  ; i < robotsInField/2 ; i++ ){
        allPositions[i] = Vector2D(px, py);
        allPositions[robotsInField-i-1] = Vector2D(allPositions[i].x, -allPositions[i].y);
        qDebug() << "i : " << i << ",   j : " << robotsInField-i-1 ;
        draw(Circle2D(allPositions[i],0.1), "red");
        draw(Circle2D(allPositions[robotsInField-i-1],0.1), "red");
        py += offset;
    }

}

void CMixTeamHandler::initialSlaveMakePacket()
{
    int robotsInField = knowledge->activesInField.size();
    multi_team_comm::TeamPlan *packet = new multi_team_comm::TeamPlan();
    multi_team_comm::RobotPlan *plans[robotsInField];
    multi_team_comm::Pose *poses[robotsInField];
    multi_team_comm::Location *posLoc[robotsInField];
    multi_team_comm::Location *planLoc[robotsInField];
    int i = 0;
    for( int j = 0 ; j < robotsInField ; j++ ){
        if( knowledge->activesInField.at(j)->id() != knowledge->mixGoaleID ){
            plans[i] = packet->add_plans();
            plans[i]->set_robot_id(knowledge->activesInField.at(j)->id());
    //        plans[i]->set_role(multi_team_comm::RobotPlan::Defense);//not important here
            poses[i] = plans[i]->mutable_nav_target();
    //        planLoc[i] = plans[i]->mutable_shot_target();
    //        planLoc[i] = allPositions[i];
            posLoc[i] = poses[i]->mutable_loc();
            posLoc[i]->set_x(allPositions[i].x * 100);
            posLoc[i]->set_y(allPositions[i].y * 100);
            i++;
        }
    }

    //for goalie
    plans[robotsInField-1] = packet->add_plans();
    plans[robotsInField-1]->set_robot_id(knowledge->mixGoaleID);
    plans[robotsInField-1]->set_role(multi_team_comm::RobotPlan::Goalie);
    poses[robotsInField-1] = plans[robotsInField-1]->mutable_nav_target();
//        planLoc[i] = plans[i]->mutable_shot_target();
//        planLoc[i] = allPositions[i];
    posLoc[robotsInField-1] = poses[robotsInField-1]->mutable_loc();
    posLoc[robotsInField-1]->set_x((wm->field->ourGoal().x + 0.12)*100);
    posLoc[robotsInField-1]->set_y(wm->field->ourGoal().y * 100);

    static MixTeamSender *sender = new MixTeamSender();
    sender->packet = packet;
    sender->flag = true;
}


void CMixTeamHandler::task1positioning()
{
    int robotsInField = knowledge->activesInField.size();
    int i;
    for(i = 0 ; i < robotsInField ; i++){
        if(knowledge->activesInField.at(i)->id() == knowledge->mixGoaleID)
            break;
    }
    if( i != robotsInField )
        robotsInField--;
    draw(Circle2D(wm->field->ourGoal(),1.5), "cyan");
    double radius = 1.5;
    Vector2D vec;
    double angle = 0;
    for( int i = 0 ; i < (robotsInField+1)/2 ; i++ ){
        if( (robotsInField+1)/2 == 5 )
            angle += 0.5;
        else if( (robotsInField+1)/2 == 4 )
            angle += 0.6;
        else if( (robotsInField+1)/2 == 3 )
            angle += 0.78;
        else if( (robotsInField+1)/2 == 2 )
            angle += 1.05;
        else if( (robotsInField+1)/2 == 1 )
            angle += 1.57;
        else if( (robotsInField+1)/2 == 0 )
            angle += 0;
        allPositions[i] = getXYByAngleOurGoal(angle, radius);
        allPositions[robotsInField-i-1] = Vector2D(-allPositions[i].x, allPositions[i].y);
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
    int robotsInField = knowledge->activesInField.size();
    multi_team_comm::TeamPlan *packet = new multi_team_comm::TeamPlan();
    multi_team_comm::RobotPlan *plans[robotsInField];
    multi_team_comm::Pose *poses[robotsInField];
    multi_team_comm::Location *posLoc[robotsInField];
    multi_team_comm::Location *planLoc[robotsInField];
    int i = 0;
    for( int j = 0 ; j < robotsInField/2 ; j++ ){
        if( knowledge->activesInField.at(j)->id() != knowledge->mixGoaleID ){
            plans[i] = packet->add_plans();
            plans[i]->set_robot_id(knowledge->activesInField.at(j)->id());
            plans[i]->set_role(multi_team_comm::RobotPlan::Defense);//not important here
            poses[i] = plans[i]->mutable_nav_target();
    //        planLoc[i] = plans[i]->mutable_shot_target();
    //        planLoc[i] = allPositions[i];
            posLoc[i] = poses[i]->mutable_loc();
            posLoc[i]->set_x(allPositions[i].x * 100);
            posLoc[i]->set_y(allPositions[i].y * 100);
            draw(Circle2D(Vector2D(allPositions[i]),0.1), "blue");
            i++;
        }
    }

    for( int j = robotsInField/2 ; j < robotsInField ; j++ ){
        if( knowledge->activesInField.at(j)->id() != knowledge->mixGoaleID ){
            plans[i] = packet->add_plans();
            plans[i]->set_robot_id(knowledge->activesInField.at(j)->id());
            plans[i]->set_role(multi_team_comm::RobotPlan::Offense);//not important here
            poses[i] = plans[i]->mutable_nav_target();
    //        planLoc[i] = plans[i]->mutable_shot_target();
    //        planLoc[i] = allPositions[i];
            posLoc[i] = poses[i]->mutable_loc();
            posLoc[i]->set_x(allPositions[i].x * 100);
            posLoc[i]->set_y(allPositions[i].y * 100);
            draw(Circle2D(allPositions[i],0.1), "blue");
            i++;
        }
    }

    //for goalie
    plans[robotsInField-1] = packet->add_plans();
    plans[robotsInField-1]->set_robot_id(knowledge->mixGoaleID);
    plans[robotsInField-1]->set_role(multi_team_comm::RobotPlan::Goalie);
    poses[robotsInField-1] = plans[robotsInField-1]->mutable_nav_target();
//        planLoc[i] = plans[i]->mutable_shot_target();
//        planLoc[i] = allPositions[i];
    posLoc[robotsInField-1] = poses[robotsInField-1]->mutable_loc();
    posLoc[robotsInField-1]->set_x((wm->field->ourGoal().x + 0.12)*100);
    posLoc[robotsInField-1]->set_y(wm->field->ourGoal().y * 100);

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
        initialReadPacket();
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
//            debug(QString("planID: %1 ").arg(ptemp.robot_id()), D_ATOUSA);
            if(ourAgentIDs.contains(ptemp.robot_id())){
//                debug(QString("i : %1").arg(ptemp.robot_id()), D_ATOUSA);
                ourRols[counter]->setAgent(knowledge->getAgent(ptemp.robot_id()));
                ourRols[counter]->init(Vector2D((double)ptemp.nav_target().loc().x()/100,(double)ptemp.nav_target().loc().y()/100),Vector2D(0,0));
//                draw(Circle2D(Vector2D((double)ptemp.nav_target().loc().x()/100,(double)ptemp.nav_target().loc().y()/100),0.1), "red");

                counter++;
            }
        }
        debug(QString("counter = %1").arg(counter), D_ATOUSA);
        for(int i = 0 ; i < counter ; i++)
            ourRols[i]->execute();
    }
}
