#include "mixteamhandler.h"

CMixTeamHandler::CMixTeamHandler()
{
    reader = new MixTeamReader();
    setOurRobotIDs();

    for(int i = 0 ; i < MAX_OUR_ROBOTS_IN_FIELD; i++ )
        ourRols[i] = new CSkillGotoPoint(NULL);
    kicker = new CSkillKick(NULL);
    oneToucher = new CSkillKickOneTouch(NULL);
    gpa = new CSkillGotoPointAvoid(NULL);

    for(int i = 0 ; i < 6 ; i++ ){
        robots[i] = new CRolePlayOff();
//        robots[i] = new CSkillGotoPointAvoid(NULL);
    }
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
        initialMakePacket();
    }
    else if(knowledge->getGameState() == CKnowledge::Start){ //force start
        debug("task1 : forceStart", D_ATOUSA);
        task1positioning();
        task1MakePacket();
    }
    else if(knowledge->getGameState() == CKnowledge::TheirIndirectKick){
        debug("task2 : theirIndirect", D_ATOUSA);
        task2positioning();
        task2MakePacket();
    }
    else if(knowledge->getGameState() == CKnowledge::OurIndirectKick){
        debug("task3 : ourIndirect", D_ATOUSA);
        task3positioning();
        task3MakePacket();
    }
}

void CMixTeamHandler::initialPositioning()
{
    int robotsInField = knowledge->getActiveAgents().size();
    int i;
    for(i = 0 ; i < robotsInField ; i++){
        if(knowledge->getActiveAgents().at(i)->id() == knowledge->mixGoaleID)
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

    for(  ; i < robotsInField/2 ; i++ ){
        allPositions[i] = Vector2D(px, py);
        allPositions[robotsInField-i-1] = Vector2D(allPositions[i].x, -allPositions[i].y);
        draw(Circle2D(allPositions[i],0.1), "red");
        draw(Circle2D(allPositions[robotsInField-i-1],0.1), "red");
        py += offset;
    }

}

void CMixTeamHandler::initialMakePacket()
{
    int robotsInField = knowledge->getActiveAgents().size();
    qDebug() << "----" << knowledge->getActiveAgents().size();
    multi_team_comm::TeamPlan *packet = new multi_team_comm::TeamPlan();
    multi_team_comm::RobotPlan *plans[robotsInField];
    multi_team_comm::Pose *poses[robotsInField];
    multi_team_comm::Location *posLoc[robotsInField];
    multi_team_comm::Location *planLoc[robotsInField];
    int i = 0;
    for( int j = 0 ; j < robotsInField ; j++ ){
        if( knowledge->getActiveAgents().at(j)->id() != knowledge->mixGoaleID ){
            plans[i] = packet->add_plans();
            plans[i]->set_robot_id(knowledge->getActiveAgents().at(j)->id());
//            plans[i]->set_role(multi_team_comm::RobotPlan::Defense);//not important here
            poses[i] = plans[i]->mutable_nav_target();
            poses[i]->set_heading(1.57);
//            planLoc[i] = plans[i]->mutable_shot_target();
//            planLoc[i]->set_x(allPositions[i].x * 100);
//            planLoc[i]->set_y(allPositions[i].y * 100);
            posLoc[i] = poses[i]->mutable_loc();
            posLoc[i]->set_x(allPositions[i].x * 100);
            posLoc[i]->set_y(allPositions[i].y * 100);
            i++;
        }
    }

//    debug(QString("yes: plan[0].x : %1").arg(plans[0]->has_nav_target()), D_ATOUSA);
//    debug(QString("no: plan[0].x : %1, plan[0].y : %2").arg(plans[0]->shot_target().x()).arg(plans[0]->shot_target().y()), D_ATOUSA);

    int k;
    for(k = 0 ; i < robotsInField ; k++){
        if(knowledge->getActiveAgents().at(k)->id() == knowledge->mixGoaleID)
            break;
    }
    if( k != robotsInField ){
        //for goalie
        plans[robotsInField-1] = packet->add_plans();
        plans[robotsInField-1]->set_robot_id(knowledge->mixGoaleID);
        plans[robotsInField-1]->set_role(multi_team_comm::RobotPlan::Offense);
        poses[robotsInField-1] = plans[robotsInField-1]->mutable_nav_target();
//        planLoc[i] = plans[i]->mutable_shot_target();
//        planLoc[i]->set_x(wm->field->oppGoal().x*100);
//        planLoc[i]->set_y(wm->field->oppGoal().y*100);
        posLoc[robotsInField-1] = poses[robotsInField-1]->mutable_loc();
        posLoc[robotsInField-1]->set_x((wm->field->ourGoal().x + 0.12)*100);
        posLoc[robotsInField-1]->set_y(wm->field->ourGoal().y * 100);
    }

    static MixTeamSender *sender = new MixTeamSender(16);
    sender->packet = packet;
    sender->flag = true;
}


void CMixTeamHandler::task1positioning()
{
    int robotsInField = knowledge->getActiveAgents().size();
    int i;
    for(i = 0 ; i < robotsInField ; i++){
        if(knowledge->getActiveAgents().at(i)->id() == knowledge->mixGoaleID)
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
//        debug (QString("a : %1, r : %2").arg(angle).arg(robotsInField), D_MAHI);
        allPositions[i] = getXYByAngleOurGoal(angle, radius);
        draw(Circle2D(Vector2D(allPositions[i]),0.1), "black");
        allPositions[robotsInField-i-1] = Vector2D(-allPositions[i].x, allPositions[i].y);
        draw(Circle2D(Vector2D(allPositions[robotsInField-i-1]),0.1), "blue");

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
    int robotsInField = knowledge->getActiveAgents().size();
    multi_team_comm::TeamPlan *packet = new multi_team_comm::TeamPlan();
    multi_team_comm::RobotPlan *plans[robotsInField];
    multi_team_comm::Pose *poses[robotsInField];
    multi_team_comm::Location *posLoc[robotsInField];
    multi_team_comm::Location *planLoc[robotsInField];
    int i = 0;
    for( int j = 0 ; j < robotsInField/2 ; j++ ){
        if( knowledge->getActiveAgents().at(j)->id() != knowledge->mixGoaleID ){
            plans[i] = packet->add_plans();
            plans[i]->set_robot_id(knowledge->getActiveAgents().at(j)->id());
            plans[i]->set_role(multi_team_comm::RobotPlan::Defense);//not important here
            poses[i] = plans[i]->mutable_nav_target();
    //        planLoc[i] = plans[i]->mutable_shot_target();
    //        planLoc[i] = allPositions[i];
            posLoc[i] = poses[i]->mutable_loc();
            posLoc[i]->set_x(allPositions[i].x * 100);
            posLoc[i]->set_y(allPositions[i].y * 100);
            i++;
        }
    }

    for( int j = robotsInField/2 ; j < robotsInField ; j++ ){
        if( knowledge->getActiveAgents().at(j)->id() != knowledge->mixGoaleID ){
            plans[i] = packet->add_plans();
            plans[i]->set_robot_id(knowledge->getActiveAgents().at(j)->id());
            plans[i]->set_role(multi_team_comm::RobotPlan::Offense);//not important here
            poses[i] = plans[i]->mutable_nav_target();
    //        planLoc[i] = plans[i]->mutable_shot_target();
    //        planLoc[i] = allPositions[i];
            posLoc[i] = poses[i]->mutable_loc();
            posLoc[i]->set_x(allPositions[i].x * 100);
            posLoc[i]->set_y(allPositions[i].y * 100);
            i++;
        }
    }

    int k;
    for(k = 0 ; i < robotsInField ; k++){
        if(knowledge->getActiveAgents().at(k)->id() == knowledge->mixGoaleID)
            break;
    }
    if( k != robotsInField ){
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
    }

    static MixTeamSender *sender = new MixTeamSender(16);
    sender->packet = packet;
    sender->flag = true;
}

void CMixTeamHandler::task2positioning()
{
    int num = 11;
    //defense
    allPositions[0] = Vector2D(wm->field->ourGoal().x + 1.15, 0);
    allPositions[1] = Vector2D(wm->field->ourGoal().x + 1.14, 0.3);
    allPositions[2] = Vector2D(wm->field->ourGoal().x + 1.14, -0.3);
    allPositions[3] = Vector2D(wm->field->ourGoal().x + 1.1, 0.6);
    allPositions[4] = Vector2D(wm->field->ourGoal().x + 1.1, -0.6);
    allPositions[5] = Vector2D(wm->field->ourGoal().x + 0.95, 0.9);
    allPositions[6] = Vector2D(wm->field->ourGoal().x + 0.95, -0.9);
    allPositions[7] = Vector2D(wm->field->ourGoal().x + 0.72, 1.15);
    allPositions[8] = Vector2D(wm->field->ourGoal().x + 0.72, -1.15);

    //others
    allPositions[9] = Vector2D(wm->field->ourGoal().x + 4.1, 2.5);
    allPositions[10] = Vector2D(wm->field->ourGoal().x + 4.4, 2.5);

    for( int i = 0 ; i < num ; i++ ){
        draw(Circle2D(allPositions[i],0.1), "red");
    }
}

void CMixTeamHandler::task2MakePacket()
{
    int robotsInField = knowledge->getActiveAgents().size();
    multi_team_comm::TeamPlan *packet = new multi_team_comm::TeamPlan();
    multi_team_comm::RobotPlan *plans[robotsInField];
    multi_team_comm::Pose *poses[robotsInField];
    multi_team_comm::Location *posLoc[robotsInField];
    multi_team_comm::Location *planLoc[robotsInField];
    int i = 0;
    for( int j = 0 ; j < robotsInField ; j++ ){
        if( knowledge->getActiveAgents().at(j)->id() != knowledge->mixGoaleID ){
            plans[i] = packet->add_plans();
            plans[i]->set_robot_id(knowledge->getActiveAgents().at(j)->id());
            if(i < 9)
                plans[i]->set_role(multi_team_comm::RobotPlan::Defense);
            else
                plans[i]->set_role(multi_team_comm::RobotPlan::Default);
            poses[i] = plans[i]->mutable_nav_target();
    //        planLoc[i] = plans[i]->mutable_shot_target();
    //        planLoc[i] = allPositions[i];
            posLoc[i] = poses[i]->mutable_loc();
            posLoc[i]->set_x(allPositions[i].x * 100);
            posLoc[i]->set_y(allPositions[i].y * 100);
            i++;
        }
    }

    int k;
    for(k = 0 ; i < robotsInField ; k++){
        if(knowledge->getActiveAgents().at(k)->id() == knowledge->mixGoaleID)
            break;
    }
    if( k != robotsInField ){
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
    }

    static MixTeamSender *sender = new MixTeamSender(16);
    sender->packet = packet;
    sender->flag = true;
}

void CMixTeamHandler::task3positioning()
{
    int robotsInField = knowledge->getActiveAgents().size();

    selectedIDM = chooseMasterID(robotsInField);
    selectedIDS = chooseSlaveID(robotsInField);
    debug(QString("M : %1, S : %2").arg(selectedIDM).arg(selectedIDS), D_ATOUSA);

    if(selectedIDS == -1 || selectedIDM == -1){
        debug(QString("selected is -1"), D_ATOUSA);
    }

    selectedPosM = Vector2D(wm->field->oppGoal().x - 0.5, -2.5);
    selectedPosS = Vector2D(wm->field->oppGoal().x - 1, 2);
    draw(Circle2D(selectedPosS,0.1), "red");
    draw(Circle2D(selectedPosM,0.1), "red");

    double offset = 0.9;
    for(int i = 0 ; i < robotsInField ; i++ ){
        allPositions[i] = Vector2D(wm->field->ourGoal().x + offset, -2.5);
        offset += 0.3;
        draw(Circle2D(allPositions[i],0.1), "red");
    }
}

int CMixTeamHandler::chooseMasterID(int robotsInField)
{
    for(int i = 0 ; i < robotsInField ; i++){
        int tid = knowledge->getActiveAgents().at(i)->id();
        if((ourAgentIDs.contains(tid)) && (tid != knowledge->mixGoaleID)){
            return tid;
        }
    }
    return -1;
}

int CMixTeamHandler::chooseSlaveID(int robotsInField)
{
    for(int i = 0 ; i < robotsInField ; i++){
        int tid = knowledge->getActiveAgents().at(i)->id();
        if(!(ourAgentIDs.contains(tid)) && (tid != knowledge->mixGoaleID)){
            return tid;
        }
    }
    return -1;
}

void CMixTeamHandler::task3MakePacket()
{
    int robotsInField = knowledge->getActiveAgents().size();
    multi_team_comm::TeamPlan *packet = new multi_team_comm::TeamPlan();
    multi_team_comm::RobotPlan *plans[robotsInField];
    multi_team_comm::Pose *poses[robotsInField];
    multi_team_comm::Location *posLoc[robotsInField];
    multi_team_comm::Location *planLoc[robotsInField];


    //master robot plan
    plans[0] = packet->add_plans();
    plans[0]->set_robot_id(selectedIDM);//for master to pass
    plans[0]->set_role(multi_team_comm::RobotPlan::Offense);
    poses[0] = plans[0]->mutable_nav_target();
    planLoc[0] = plans[0]->mutable_shot_target();
    planLoc[0]->set_x(selectedPosS.x * 100);//shot target
    planLoc[0]->set_y(selectedPosS.y * 100);//shot target
    posLoc[0] = poses[0]->mutable_loc();
    posLoc[0]->set_x(selectedPosM.x * 100);//nav traget
    posLoc[0]->set_y(selectedPosM.y * 100);//nav target

    //slave robot plan
    plans[1] = packet->add_plans();
    plans[1]->set_robot_id(selectedIDS);//for master to pass
    plans[1]->set_role(multi_team_comm::RobotPlan::Offense);
    poses[1] = plans[1]->mutable_nav_target();
    planLoc[1] = plans[1]->mutable_shot_target();
    planLoc[1]->set_x(wm->field->oppGoal().x * 100);//shot target
    planLoc[1]->set_y(wm->field->oppGoal().y * 100);//shot target
    posLoc[1] = poses[1]->mutable_loc();
    posLoc[1]->set_x(selectedPosS.x * 100);//nav traget
    posLoc[1]->set_y(selectedPosS.y * 100);//nav target


    int i = 2;
    for( int j = 0 ; j < robotsInField ; j++ ){
        if( (knowledge->getActiveAgents().at(j)->id() != selectedIDS) && (knowledge->getActiveAgents().at(j)->id() != selectedIDM)){
            plans[i] = packet->add_plans();
            plans[i]->set_robot_id(knowledge->getActiveAgents().at(j)->id());
            plans[i]->set_role(multi_team_comm::RobotPlan::Default);
            poses[i] = plans[i]->mutable_nav_target();
    //        planLoc[i] = plans[i]->mutable_shot_target();
    //        planLoc[i] = allPositions[i];
            posLoc[i] = poses[i]->mutable_loc();
            posLoc[i]->set_x(allPositions[i].x * 100);
            posLoc[i]->set_y(allPositions[i].y * 100);
            i++;
        }
    }

    static MixTeamSender *sender = new MixTeamSender(16);
    sender->packet = packet;
    sender->flag = true;
}

///////////////////slave/////////////////////
void CMixTeamHandler::slave(bool isM)
{
    if(false){//initial
        isMaster = isM;
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
            initialReadPacket();
        }
        else if(knowledge->getGameState() == CKnowledge::OurIndirectKick){
            debug("task3 : ourIndirect", D_ATOUSA);
            task3ReadPacket();
        }
    }
    else {
        mixReadPacket();
    }

}


void CMixTeamHandler::initialReadPacket()
{
    if( knowledge->ready ){
        int counter = 0;
        for(int  i = 0 ;  i < knowledge->kPlans->plans_size() ; i++){
            multi_team_comm::RobotPlan ptemp = knowledge->kPlans->plans(i);
            if(ourAgentIDs.contains(ptemp.robot_id())){
                ourRols[counter]->setAgent(knowledge->getAgent(ptemp.robot_id()));
//                draw(Circle2D(Vector2D((double)ptemp.nav_target().loc().x()/100,(double)ptemp.nav_target().loc().y()/100),0.1), "red");
                Vector2D targetDir;
                if(ptemp.has_nav_target() && ptemp.nav_target().has_heading() && ptemp.nav_target().heading() < 500 && ptemp.nav_target().heading() > -500){
                    float rHeading = refineHeading(ptemp.nav_target().heading());
                    targetDir = targetDir.setPolar(1, AngleDeg(rHeading*_RAD2DEG));
                }
                else
                    targetDir = Vector2D(1,0);
                Vector2D target = Vector2D((double)ptemp.nav_target().loc().x()/100,(double)ptemp.nav_target().loc().y()/100);
                draw(Circle2D(targetDir + target , 0.05), "black");
                ourRols[counter]->init(target, target+targetDir);
                counter++;
            }
        }
//        debug(QString("counter = %1").arg(counter), D_ATOUSA);
        for(int i = 0 ; i < counter ; i++)
            ourRols[i]->execute();
    }
}

void CMixTeamHandler::task3ReadPacket()
{
    if( knowledge->ready ){
        int counter = 0;
        //find slave offense for kick
        int slaveID = -1;
        for(int i = 0 ; i < knowledge->kPlans->plans_size() ; i++){
            multi_team_comm::RobotPlan p = knowledge->kPlans->plans(i);
            if( p.role() == multi_team_comm::RobotPlan::Offense && !(ourAgentIDs.contains(p.robot_id()))){
                slaveID = p.robot_id();
                break;
            }
        }
        for(int  i = 0 ;  i < knowledge->kPlans->plans_size() ; i++){
            multi_team_comm::RobotPlan ptemp = knowledge->kPlans->plans(i);
            if(ourAgentIDs.contains(ptemp.robot_id())){
                if(ptemp.role() == multi_team_comm::RobotPlan::Offense){
                    if(isMaster){
//                        debug(QString("kicker = %1").arg(ptemp.robot_id()), D_ATOUSA);
                        executeMasterOffense(ptemp.robot_id(), Vector2D((double)ptemp.nav_target().loc().x()/100, (double)ptemp.nav_target().loc().y()/100), Vector2D((double)ptemp.shot_target().x()/100, (double)ptemp.shot_target().y()/100), slaveID);
                    }
                    else{
//                        debug(QString("onetoucher = %1").arg(ptemp.robot_id()), D_ATOUSA);
                        executeSlaveOffense(ptemp.robot_id(), Vector2D((double)ptemp.nav_target().loc().x()/100, (double)ptemp.nav_target().loc().y()/100), Vector2D((double)ptemp.shot_target().x()/100, (double)ptemp.shot_target().y()/100));
                    }
                }
                else{
                    ourRols[counter]->setAgent(knowledge->getAgent(ptemp.robot_id()));
                    Vector2D targetDir;
                    if(ptemp.has_nav_target() && ptemp.nav_target().has_heading() && ptemp.nav_target().heading() < 500 && ptemp.nav_target().heading() > -500){
                        float rHeading = refineHeading(ptemp.nav_target().heading());
                        targetDir = targetDir.setPolar(1, AngleDeg(rHeading*_RAD2DEG));
                    }
                    else
                        targetDir = Vector2D(1,0);
                    Vector2D target = Vector2D((double)ptemp.nav_target().loc().x()/100,(double)ptemp.nav_target().loc().y()/100);
                    draw(Circle2D(targetDir + target , 0.05), "black");
                    ourRols[counter]->init(target , target + targetDir);
    //                draw(Circle2D(Vector2D((double)ptemp.nav_target().loc().x()/100,(double)ptemp.nav_target().loc().y()/100),0.1), "red");
                    counter++;
                }
            }
        }
//        debug(QString("counter = %1").arg(counter), D_ATOUSA);
        for(int i = 0 ; i < counter ; i++)
            ourRols[i]->execute();
    }
}

void CMixTeamHandler::executeMasterOffense(int robotId, Vector2D point1, Vector2D point2, int slaveID)
{
    kicker->setAgent(knowledge->getAgent(robotId));
    kicker->setDontKick(true);
    kicker->setChip(false);
    kicker->setTarget(point2);
    kicker->setKickSpeed(4);

    gpa->setAgent(knowledge->getAgent(robotId));
    gpa->init(point1, Vector2D(1,0));

    CAgent *c = knowledge->getAgent(robotId);

//    debug(QString("vel : %1").arg(wm->ball->vel.length()), D_ATOUSA);
    if(wm->ball->vel.length() > 0.5){
        gpa->execute();
    }
    else{
        if(Circle2D(point2, 0.3).contains(knowledge->getAgent(slaveID)->pos())){//receive ID ro chi kar konam?
                debug(QString("kicker id is %1").arg(c->id()), D_ATOUSA);
                kicker->setDontKick(false);
                kicker->execute();
        }
        else{
            gpa->execute();//?
        }
    }
}

void CMixTeamHandler::executeSlaveOffense(int robotId, Vector2D point1, Vector2D point2)
{
    debug(QString("ownToucher = %1").arg(robotId), D_ATOUSA);
    oneToucher->setAgent(knowledge->getAgent(robotId));
    oneToucher->setTarget(point2);
    oneToucher->setWaitPos(point1);
    oneToucher->setKickSpeed(5);
    oneToucher->execute();
}


/////////mixed team game/////////
void CMixTeamHandler::mixReadPacket()
{
    if(knowledge->ready){
        for(int i = 0; i < knowledge->kPlans->plans_size(); i++){
            multi_team_comm::RobotPlan ptemp = knowledge->kPlans->plans(i);
            if(ourAgentIDs.contains(ptemp.robot_id())){
                int validation = isPlanValid(ptemp);
                bool v = isPosLocValid(ptemp.nav_target().loc());
                debug(QString("validation : %1, id : %2").arg(v).arg(ptemp.robot_id()), D_ATOUSA);

                if(validation != -1)
                    executePlan(ptemp, validation);
            }
        }
    }
}

int CMixTeamHandler::isPlanValid(multi_team_comm::RobotPlan plan)
{
    if(isPosValid(plan)){
        return 1;
    }
    else if(isShotTargetValid(plan)){
        return 2;
    }
    else if(isRoleValid(plan)){
        return 3;
    }
    else
        return -1;//invalid
}

void CMixTeamHandler::executePlan(multi_team_comm::RobotPlan plan, int validation)
{
    if(validation == 1){//valid pos
        if(!isShotTargetValid(plan)){
            if(!plan.has_role()){//gotopoint
                gotopointExecute(plan);
            }
            else if(plan.role() == multi_team_comm::RobotPlan::Offense){//rcv
                receiveExecute(plan);
            }
            else{//gtp
                gotopointExecute(plan);
            }
        }
        else{//kick
            kickExecute(plan);
        }
    }
    else if(validation == 2){//invalid pos & valid shotTarget
        //kick
        kickExecute(plan);
    }
    else if(validation == 3){//invalid pos & invald shotTarget & valid role(defense of goalie)
        //goalie or defense execution
    }
    else
        debug("plan validation error", D_ATOUSA);
}

void CMixTeamHandler::gotopointExecute(multi_team_comm::RobotPlan plan)
{
    int index = ourAgentIDs.indexOf(plan.robot_id());

    robots[index]->setAgent(knowledge->getAgent(plan.robot_id()));
    robots[index]->setSelectedSkill(roleSkill::GotopointAvoid);
    Vector2D target = Vector2D(plan.nav_target().loc().x()*0.01,plan.nav_target().loc().y()*0.01);
    Vector2D targetDir;
    if(plan.has_nav_target() && plan.nav_target().has_heading() && plan.nav_target().heading() < 500 && plan.nav_target().heading() > -500){
        float rHeading = refineHeading(plan.nav_target().heading());
        targetDir = targetDir.setPolar(1, AngleDeg(rHeading*_RAD2DEG));
    }
    else
        targetDir = Vector2D(1,0);
    draw(Circle2D(targetDir + target , 0.05), "black");

    robots[index]->setTargetDir(targetDir + target);
    draw(Circle2D(targetDir + target , 0.05), "black");
        qDebug() << "res   x : " << (targetDir + target).x << "   y : " << (targetDir + target).y;
    robots[index]->setTarget(target);

    robots[index]->execute();
}

void CMixTeamHandler::kickExecute(multi_team_comm::RobotPlan plan)
{
    int index = ourAgentIDs.indexOf(plan.robot_id());
    robots[index]->setAgent(knowledge->getAgent(plan.robot_id()));
    robots[index]->setSelectedSkill(roleSkill::Kick);
    Vector2D target = Vector2D(plan.shot_target().x()*0.01 , plan.shot_target().y()*0.01);
    robots[index]->setTarget(target);
    robots[index]->setDoPass(true);
    robots[index]->setKickSpeed(10);//?
    //setKickSpeed should be based on profiles!?
    robots[index]->execute();
}

void CMixTeamHandler::receiveExecute(multi_team_comm::RobotPlan plan)
{
    debug("rcv", D_ATOUSA);
    int index = ourAgentIDs.indexOf(plan.robot_id());
    robots[index]->setAgent(knowledge->getAgent(plan.robot_id()));
    robots[index]->setSelectedSkill(roleSkill::ReceivePass);
    robots[index]->setTarget(wm->ball->pos + wm->ball->vel);
//    draw(Circle2D(wm->ball->pos + wm->ball->vel, 0.1), "cyan");
    robots[index]->setAvoidPenaltyArea(true);
    robots[index]->setReceiveRadius(0.4);
    robots[index]->execute();
}

bool CMixTeamHandler::isPosValid(multi_team_comm::RobotPlan plan)
{
    if(!plan.has_nav_target())
        return false;
    else if(!plan.nav_target().has_loc())
        return false;
    else
        return isPosLocValid(plan.nav_target().loc());
}

bool CMixTeamHandler::isPosLocValid(multi_team_comm::Location loc)
{
    if( !(loc.has_x() && loc.has_y()) )
        return false;
    //heigth = 10.4, width = 14.8
    else if( ((double)loc.x()/100 > -_STADIUM_WIDTH/2)
             && ((double)loc.x()/100 < _STADIUM_WIDTH/2)
             && ((double)loc.y()/100 > -_STADIUM_HEIGHT/2)
             && ((double)loc.y()/100 < _STADIUM_HEIGHT/2)){
        return true;
    }
    else
        return false;
    //heading ham bayad check beshe
}

bool CMixTeamHandler::isShotTargetValid(multi_team_comm::RobotPlan plan)
{
    //it is not important for shot_target to be in field
    if(!plan.has_shot_target())
        return false;
    else if(!plan.shot_target().has_x() || !plan.shot_target().has_y())
        return false;
    else
        return true;
}

bool CMixTeamHandler::isRoleValid(multi_team_comm::RobotPlan plan)
{
    if(!plan.has_role())
        return false;
    else if(plan.role() == multi_team_comm::RobotPlan::Offense || plan.role() == multi_team_comm::RobotPlan::Default)
        return false;
    else
        return true;
}

float CMixTeamHandler::refineHeading(float heading)
{
    if( heading > PI ){
        while(heading > PI){
            heading -= PI*2;
        }
    }
    if( heading < -PI ){
        while(heading < -PI){
            heading += PI*2;
        }
    }
    return heading;
}
