#include "mixteamhandler.h"

CMixTeamHandler::CMixTeamHandler()
{
    reader = new MixTeamReader();
    knowledge->getOurRobotIDsFromGUIMixTeam();

    for(int i = 0 ; i < MAX_OUR_ROBOTS_IN_FIELD; i++ )
        ourRols[i] = new CSkillGotoPoint(NULL);
    kicker = new CSkillKick(NULL);
    oneToucher = new CSkillKickOneTouch(NULL);
    gpa = new CSkillGotoPointAvoid(NULL);
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
        if((knowledge->ourAgentIDsMixTeam.contains(tid)) && (tid != knowledge->mixGoaleID)){
            return tid;
        }
    }
    return -1;
}

int CMixTeamHandler::chooseSlaveID(int robotsInField)
{
    for(int i = 0 ; i < robotsInField ; i++){
        int tid = knowledge->getActiveAgents().at(i)->id();
        if(!(knowledge->ourAgentIDsMixTeam.contains(tid)) && (tid != knowledge->mixGoaleID)){
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

void CMixTeamHandler::initialReadPacket()
{
    if( knowledge->ready ){
        int counter = 0;
        for(int  i = 0 ;  i < knowledge->kPlans->plans_size() ; i++){
            multi_team_comm::RobotPlan ptemp = knowledge->kPlans->plans(i);
            if(knowledge->ourAgentIDsMixTeam.contains(ptemp.robot_id())){
                ourRols[counter]->setAgent(knowledge->getAgent(ptemp.robot_id()));
                ourRols[counter]->init(Vector2D((double)ptemp.nav_target().loc().x()/100,(double)ptemp.nav_target().loc().y()/100),Vector2D(0,0));
                //                draw(Circle2D(Vector2D((double)ptemp.nav_target().loc().x()/100,(double)ptemp.nav_target().loc().y()/100),0.1), "red");

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
            if( p.role() == multi_team_comm::RobotPlan::Offense && !(knowledge->ourAgentIDsMixTeam.contains(p.robot_id()))){
                slaveID = p.robot_id();
                break;
            }
        }
        for(int  i = 0 ;  i < knowledge->kPlans->plans_size() ; i++){
            multi_team_comm::RobotPlan ptemp = knowledge->kPlans->plans(i);
            if(knowledge->ourAgentIDsMixTeam.contains(ptemp.robot_id())){
                if(ptemp.role() == multi_team_comm::RobotPlan::Offense){
                    if(isMaster){
                        debug(QString("kicker = %1").arg(ptemp.robot_id()), D_ATOUSA);
                        executeMasterOffense(ptemp.robot_id(), Vector2D((double)ptemp.nav_target().loc().x()/100, (double)ptemp.nav_target().loc().y()/100), Vector2D((double)ptemp.shot_target().x()/100, (double)ptemp.shot_target().y()/100), slaveID);
                    }
                    else{
                        debug(QString("onetoucher = %1").arg(ptemp.robot_id()), D_ATOUSA);
                        executeSlaveOffense(ptemp.robot_id(), Vector2D((double)ptemp.nav_target().loc().x()/100, (double)ptemp.nav_target().loc().y()/100), Vector2D((double)ptemp.shot_target().x()/100, (double)ptemp.shot_target().y()/100));
                    }
                }
                else{
                    ourRols[counter]->setAgent(knowledge->getAgent(ptemp.robot_id()));
                    ourRols[counter]->init(Vector2D((double)ptemp.nav_target().loc().x()/100,(double)ptemp.nav_target().loc().y()/100),Vector2D(0,0));
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
    debug(QString("*kicker = %1").arg(robotId), D_ATOUSA);
    kicker->setAgent(knowledge->getAgent(robotId));
    kicker->setDontKick(true);
    kicker->setChip(false);
    kicker->setTarget(point2);
    kicker->setKickSpeed(4);

    gpa->setAgent(knowledge->getAgent(robotId));
    gpa->init(point1, Vector2D(0,1));

    CAgent *c = knowledge->getAgent(robotId);

    //    debug(QString("vel : %1").arg(wm->ball->vel.length()), D_ATOUSA);
    if(wm->ball->vel.length() > 0.5){
        gpa->execute();
    }
    else{
        if(Circle2D(point2, 0.3).contains(knowledge->getAgent(slaveID)->pos())){//receive ID ro chi kar konam?
            //            debug(QString("*role id defalut %1").arg(c->id()), D_ATOUSA);
            kicker->setDontKick(false);
            kicker->execute();
        }
    }
}

void CMixTeamHandler::executeSlaveOffense(int robotId, Vector2D point1, Vector2D point2)
{
    debug(QString("*ownToucher = %1").arg(robotId), D_ATOUSA);
    oneToucher->setAgent(knowledge->getAgent(robotId));
    oneToucher->setTarget(point2);
    oneToucher->setWaitPos(point1);
    oneToucher->setKickSpeed(5);

    if(wm->ball->vel.length() > 0.3)
        oneToucher->execute();

}


CMixTeamCoach::CMixTeamCoach(){
    markRadiusStrict = 1.39;
}

CKnowledge::ballPossesionState CMixTeamCoach::ballPossess(){
    CKnowledge::ballPossesionState ballPoss;
    double temp = wm->ball->pos.x + wm->ball->vel.x * 1;

    if(temp > 1.7) {
        ballPoss = CKnowledge::WEHAVETHEBALL;
    } else if (temp < 0.5){
        ballPoss = CKnowledge::WEDONTHAVETHEBALL;
    } else {
        ballPoss = lastBallPossess;
    }

    if (wm->field->isInOurPenaltyArea(wm->ball->pos)
            &&  wm->ball->vel.length() < 0.2) {
        ballPoss = CKnowledge::SOSOTHEIR;
    }
    if (wm->field->isInOppPenaltyArea(wm->ball->pos)
            && wm->ball->vel.length() < 0.2) {
        ballPoss = CKnowledge::SOSOOUR;
    }

    lastBallPossess = ballPoss;

    return ballPoss;
}

void CMixTeamCoach::decideMarkAndDefenseCount(){

    int agentCount = wm->our.activeAgentsCount();

    if(agentCount == 0) {
        defenseCount = 0;
        markCount = 0;
    } else if(agentCount == 1) {
        defenseCount = 0;
        markCount = 0;
    } else if(agentCount == 2) {
        defenseCount = 1;
        markCount = 0;
    } else if(agentCount == 3) {
        defenseCount = 2;
        markCount = 0;
    }
    else
    {
        if(knowledge->isStart())    // playon
        {
            CKnowledge::ballPossesionState bp = ballPossess();

            switch (bp) {
            case CKnowledge::WEHAVETHEBALL:
                defenseCount = 2;
                markCount = 0;
                break;
            case CKnowledge::SOSOOUR:   // isInOppPenaltyArea
                defenseCount = 2;
                markCount = min(5, agentCount - defenseCount - 2);  // at least 2 agenst for offense
                break;
            case CKnowledge::WEDONTHAVETHEBALL:
                defenseCount = 2;
                markCount = agentCount - defenseCount - 1;
                break;
            case CKnowledge::SOSOTHEIR: // isInOurPenaltyArea
                defenseCount = 2;
                markCount = max(1, agentCount - defenseCount - 5);  // at most 3 mark agnet, send robots for offense role
                break;
            }
        }
        else if(knowledge->getGameState() == CKnowledge::TheirKickOff) {
            defenseCount = 1;
            markCount = agentCount - defenseCount - 1;
        }
        else if(knowledge->getGameState() == CKnowledge::OurKickOff) {
            defenseCount = 1;
            markCount = 0;
        }
        else if(knowledge->isOurNonPlayOnKick()) {
            if(wm->ball->pos.x > 0.7) {
                defenseCount = 1;
                markCount = 0;
            } else if(wm->ball->pos.x < 0) {
                defenseCount = 1;
                markCount = 2; // check
            }
        }
        else if(knowledge->isTheirNonPlayOnKick()){
            if(wm->ball->pos.x > 0.7) {
                defenseCount = 1;
                markCount = agentCount - defenseCount - 1;
            } else if(wm->ball->pos.x < 0) {
                defenseCount = 0;
                markCount = agentCount - defenseCount - 1;
            }
        }

    }


    if (wm->gs->penalty_shootout()) {
        defenseCount = 0;
    }
    if(knowledge->getGameState()== CKnowledge::HalfTimeLineUp) {
        defenseCount = 0;
        goalieID = -1;
    }
}

void CMixTeamCoach::setPositions(){
    tempDefPos = defPos.getDefPositions(wm->ball->pos, defenseCount, 1.5, 3);

    oppPos.clear();
    for(int i=0; i<wm->opp.activeAgentsCount(); i++){
        oppPos.append(wm->opp.active(i)->pos);
    }

    sortedDangerousOpp = sortdangerpassplayon(oppPos);

    for(int i=0; i < markCount; i++){
//        markPos.append(ShootBlockRatio(0.5, sortedDangerousOpp.at(i).Pos));
//        markPos.append(PassBlockRatio(0.5, sortedDangerousOpp.at(i).Pos));
    }
}

void CMixTeamCoach::makePlanPacket(){

}

CMixTeamCoach::SPosAndHeading CMixTeamCoach::ShootBlockRatio(double ratio, Vector2D opp){
    //// This function produces a point between opponent agents to be mark &&
    //// ourGoal to block the shot path. Also checks if this point is in the
    //// penalty area, produces a point that is intersection of penalty area with
    //// shot path.
    CMixTeamCoach::SPosAndHeading temp;
    CDefPos test;
    temp.heading = Vector2D(0,0);
    temp.position = Vector2D(0,0);
    Segment2D tempSeg;
    tempSeg.assign(opp + (wm->field->ourGoal() - opp) * (-10), wm->field->ourGoal());
    Vector2D pos = opp + (wm->field->ourGoal() - opp) * ratio;
    if((wm->field->ourGoal() - pos).length() < markRadiusStrict){
        temp.position = test.getIntersectionWithPenaltyAreaDef(1.39,tempSeg);
        temp.heading = opp - wm->field->ourGoal();
        draw(tempSeg, "blue");
    }
    else{
        temp.position = pos;
        temp.heading = opp - wm->field->ourGoal();
        draw(tempSeg, "blue");
    }
    return temp;
}

CMixTeamCoach::SPosAndHeading CMixTeamCoach::PassBlockRatio(double ratio, Vector2D opp){
    //// This function produces a point that block the pass path.Also if the
    //// resulted point is in the penalty area, this function geneates a suitable
    //// point.

    CMixTeamCoach::SPosAndHeading temp;
    Segment2D tempSeg;
    temp.position = Vector2D(0,0);
    temp.heading = Vector2D(0,0);
    tempSeg.assign(wm->ball->pos, wm->ball->pos + (opp - wm->ball->pos) * 10);
    Vector2D pos = wm->ball->pos + (opp - wm->ball->pos) * ratio;
    CDefPos test;
    double distance = (wm->ball->pos - opp).length();
    debug(QString("Dist %1").arg(distance), D_MAHI);
    if(distance > 0.6){
        if(ratio * distance > 0.1){
        debug(QString("First"),D_HAMED);
        }else{
            debug(QString("second"),D_HAMED);
            pos = wm->ball->pos + (opp - wm->ball->pos) * 0.15 / distance;
        }
    }
    else{
        debug(QString("Third"),D_HAMED);
        pos = wm->ball->pos + (opp - wm->ball->pos) * (1 + 0.15 / distance);
    }
    if((wm->field->ourGoal() - pos).length() < markRadiusStrict){
        temp.position = test.getIntersectionWithPenaltyAreaDef(2, tempSeg);
        temp.heading = wm->ball->pos - opp;
        draw(tempSeg, "red");
        debug(QString("this is in the penalty area, Block pass Mode"), D_HAMED);
    }
    else{
        temp.position = pos;
        temp.heading = wm->ball->pos - opp;
        draw(tempSeg, "red");
    }
    return temp;
}

QList<CMixTeamCoach::SDangerousOpp > CMixTeamCoach::sortdangerpassplayon(QList<Vector2D> oppposdanger) {

    double KA=1; //Angle Coefficient
    double KDB=1;  //Distance To Ball
    double KDG=1;  //Distnce To Goal
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(),Vector2D(-1.0 * (_FIELD_WIDTH / 2 - _GOAL_RAD), 0), wm->field->ourGoalL()).degree();
    //draw(Vector2D(-1.0 * (_FIELD_WIDTH - _GOAL_WIDTH), 0), QColor(Qt::red));
    // double RangeofAngle2 = Vector2D::angleOf(wm->field->ou,Vector2D(0, -1.0 * (_FIELD_WIDTH - _GOAL_WIDTH)), wm->field->ourGoalL()).degree();

    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), Vector2D(-1.0 * _FIELD_WIDTH/2,-1.0 * _FIELD_HEIGHT /2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), wm->field->ourGoal()).length());

    double angle, distancetoball, distancetogoal,danger;

    QList<CMixTeamCoach::SDangerousOpp> output;
    CMixTeamCoach::SDangerousOpp dv;
    for(int i = 0; i<oppposdanger.count(); i++) {
        dv.Pos = oppposdanger[i];

        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL() ).degree();
        distancetoball =  (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal =  (oppposdanger[i] - wm->field->ourGoal()).length();
        danger = (KA * fabs(angle) / RangeofAngle) + ( KDB * 1 - (distancetoball / RangeofDistancetoBall) ) + (KDG * 1 -(distancetogoal / RangeofDistancetoGoal));

        dv.danger = danger;
        output.append(dv);
        //draw(QString("HMD danger=%1").arg(danger), oppposdanger[i] + Vector2D(0,0.3), QColor(Qt::red));

        // finding nearest to intersect
        Segment2D tempsegment;
        tempsegment.assign(oppposdanger[i],wm->field->ourGoal());

        double mintempdis = 0.0;
        if(wm->our.activeAgentsCount() != 0)
            mintempdis = tempsegment.dist(wm->our.active(0)->pos);

        for(int j=0; j<wm->our.activeAgentsCount(); j++)
        {
            if(tempsegment.dist(wm->our.active(j)->pos) < mintempdis)
            {
                mintempdis = tempsegment.dist(wm->our.active(j)->pos);
            }

        }

        //        draw(QString("mindistance%1").arg(mintempdis), oppposdanger[i] + Vector2D(0,0.5), QColor(Qt::blue));
    }

    ///sorting the Qlist
    for(int i = 0; i< output.count(); i++)
    {
        for(int j = 0; j< output.count() - 1; j++ )
        {
            if(output.at(j).danger < output.at(j).danger)
                output.swap(j, j+1);
        }
    }

    for(int i=0; i<output.count(); i++)
    {
        //draw(QString("HMD Danger New%1" ).arg(output[i].second),output[i].first + Vector2D(0,.2),QColor(Qt::red));
    }

    return output;
}


QList<CMixTeamCoach::SDangerousOpp > CMixTeamCoach::sortdangerpassplayoff(QList<Vector2D> oppposdanger){
    double danger;
    /////////////// Polygon
    double radius = .1;
    double treshold = 1;

    Vector2D sol1,sol2,sol3;
    Vector2D _pos1 = wm->ball->pos;

    Vector2D _pos2 = wm->ball->pos + (10.0  * wm->ball->vel.norm() * knowledge->getRealBallVel());
    Line2D _path(_pos1,_pos2);
    Polygon2D _poly;
    Circle2D(_pos2,radius + treshold).
            intersection(_path.perpendicular(_pos2),&sol1,&sol2);


    _poly.addVertex(sol1);
    sol3 = sol1;
    _poly.addVertex(sol2);
    Circle2D(_pos1,CRobot::robot_radius_new + treshold).
            intersection(_path.perpendicular(_pos1),&sol1,&sol2);

    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    draw(_poly,"cyan");



    double KAP = 1; //Angle parameter
    double KDBP = 1; //distancetoball
    double KDIP = 2; //distancetointersect

    double AngleP;
    double distanceToBallProjectionP;
    double distanceToIntersectP;


    double RangeofAngleP = 90;
    double RangeofdistanceToBallProjectionP = Segment2D(Vector2D(-1.0 * _FIELD_WIDTH / 2, -1.0 * _FIELD_HEIGHT /2 ), Vector2D(_FIELD_WIDTH / 2 , _FIELD_HEIGHT / 2)).length();
    double RangeofdistanceToIntersectP =  radius;
    double danger2;


    /////////////////////


    double KA=1; //Angle Coefficient
    double KDB=0;  //Distance To Ball
    double KDG=1;  //Distnce To Goal
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(),Vector2D(-1.0 * (_FIELD_WIDTH / 2 - _GOAL_RAD), 0), wm->field->ourGoalL()).degree();
    //draw(Vector2D(-1.0 * (_FIELD_WIDTH - _GOAL_WIDTH), 0), QColor(Qt::red));
    // double RangeofAngle2 = Vector2D::angleOf(wm->field->ou,Vector2D(0, -1.0 * (_FIELD_WIDTH - _GOAL_WIDTH)), wm->field->ourGoalL()).degree();

    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), Vector2D(-1.0 * _FIELD_WIDTH/2,-1.0 * _FIELD_HEIGHT /2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), wm->field->ourGoal()).length());

    //double RangeofTempDis = 2;
    double angle, distancetoball, distancetogoal,danger1;

    QList<CMixTeamCoach::SDangerousOpp> output;
    CMixTeamCoach::SDangerousOpp dv;
    double Polycontain;
    for(int i = 0; i<oppposdanger.count(); i++) {
        if(Polycontain == _poly.contains(oppposdanger[i]))
        {
            Polycontain = 1;
        }
        else
        {
            Polycontain = 0;
        }
        dv.Pos = oppposdanger[i];


        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL() ).degree();
        distancetoball =  (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal =  (oppposdanger[i] - wm->field->ourGoal()).length();

        ////poly

        AngleP = Vector2D::angleOf( oppposdanger[i], wm->ball->pos, _path.projection(oppposdanger[i]) ).degree();
        distanceToIntersectP = _path.dist(oppposdanger[i]); //distanse of opponent to the path
        distanceToBallProjectionP = _path.projection(oppposdanger[i]).length(); //distance of the ball to the projection of opponent to the path

        danger1 = (KA * fabs(angle) / RangeofAngle) + ( KDB *( 1 - (distancetoball / RangeofDistancetoBall)) ) + (KDG * (1 -(distancetogoal / RangeofDistancetoGoal)));
        danger2 = KAP * ( 1 - AngleP/RangeofAngleP) + KDBP * (1 - distanceToBallProjectionP/RangeofdistanceToBallProjectionP ) + KDIP * (1 - distanceToIntersectP / RangeofdistanceToIntersectP);
        // debug(QString("angle: %1, rangeofangle: %2, distansetoball:%3, RangeofDistancetoBall:%4,distancetogoal:%5,rangeofdistansetogoal:%6").arg(angle).arg(RangeofAngle).arg(distancetoball).arg(RangeofDistancetoBall).arg(distancetogoal).arg(RangeofDistancetoGoal),D_HAMED);
        // debug(QString("angleP: %1, rangeofangleP: %2, distansetoballProjectionP:%3, RangeofDistancetoBallProjectionP:%4,distancetointersect:%5,rangeofdistansetointesrsect:%6").arg(AngleP).arg(RangeofAngleP).arg(distanceToBallProjectionP).arg(RangeofdistanceToBallProjectionP).arg(distanceToIntersectP).arg(RangeofdistanceToIntersectP),D_HAMED);
        if( knowledge->getRealBallVel() < .1)
            danger = danger1;
        else
            danger = 10* Polycontain * danger2 + danger1;

        dv.danger = danger;
        output.append(dv);
        draw(QString("HMD danger=%1").arg(danger), oppposdanger[i] + Vector2D(0,0.3), QColor(Qt::red));
        //draw(_poly, QColor(Qt::blue));



        //        draw(QString("mindistance%1").arg(mintempdis), oppposdanger[i] + Vector2D(0,0.5), QColor(Qt::blue));
    }
    ///sorting the Qlist
    for(int i = 0; i< output.count(); i++)
    {
        for(int j = 0; j< output.count() - 1; j++ )
        {
            if(output.at(j).danger < output.at(j+1).danger)
                output.swap(j, j+1);
        }
    }

    return output;
}
