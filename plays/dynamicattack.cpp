#include "dynamicattack.h"

CDynamicAttack::CDynamicAttack() {
    for(size_t i = 0;i  < 6;i++) {
        roleAgents[i] = new CRoleDynamic();
    }

    passFlag   = false;
    repeatFlag = false;
    passerID   = -1;
    counter    = 0;
    currentPlan.positionCnt = 0;
    lastPasserRoleIndex  = -1;

    if(ballPos.x >= 0) isBallInOurField = false;
    else isBallInOurField = true;

    guards[0] = new Rect2D();

    for(size_t i = 1;i < 4;i++) {
        guards[i] = new Rect2D[i];
    }

    for(size_t i = 1;i < 4;i++) {
        guardLocations[i] = new Vector2D*[i];
    }
    for(size_t i = 1;i < 4;i++) {
        for(size_t j = 0;j < 4;j++) {
            guardLocations[i][j] = new Vector2D[3];
        }
    }

    for(size_t i = 0; i < 5;i++) {
        goToDynamic[i] = false;
    }

    assignRegions();

    assignLocations();
}

CDynamicAttack::~CDynamicAttack() {
    for(size_t i = 0;i  < 6;i++) {
        delete roleAgents[i];
    }

    delete   guards[0];
    delete   guards[1];
    delete[] guards[2];
    delete[] guards[3];

    delete guardLocations[0];

    for(size_t i = 1;i < 4;i++) {
        for(size_t j = 0;j < 4;j++) {
            delete[] guardLocations[i][j];
        }
    }

    for(size_t i = 0;i < 4;i++) {
        delete guardLocations[i];
    }

}

void CDynamicAttack::init(QList<int> _agents , QMap<QString , EditData*> *_editData){
    setAgentsID(_agents);
    setEditData(_editData);
    initMaster();

    if( knowledge->getLastPlayExecuted() != StartPlay){
        reset();
    }
    knowledge->setLastPlayExecuted(StartPlay);
}

void CDynamicAttack::reset(){
    executedCycles = 0;
    debug(QString("Dynamic Attack Reset"),D_MAHI);
}

void CDynamicAttack::execute_0() {
    debug(QString("Dynamic Attack : 0"),D_MAHI);
    globalExecute(0);
}

void CDynamicAttack::execute_1() {
    debug(QString("Dynamic Attack : 1"),D_MAHI);
    globalExecute(1);
}

void CDynamicAttack::execute_2() {
    debug(QString("Dynamic Attack : 2"),D_MAHI);
    globalExecute(2);
}

void CDynamicAttack::execute_3() {
    debug(QString("Dynamic Attack : 3"),D_MAHI);
    globalExecute(3);
}

void CDynamicAttack::execute_4() {
    debug(QString("Dynamic Attack : 3"),D_MAHI);
    globalExecute(3);
}

void CDynamicAttack::execute_5() {
    debug(QString("Dynamic Attack : 3"),D_MAHI);
    globalExecute(3);
}

void CDynamicAttack::execute_6() {
    debug(QString("Dynamic Attack : 3"),D_MAHI);
    globalExecute(3);
}

void CDynamicAttack::globalExecute(int agentSize) {
    ballPos = wm->ball->pos;
    ballVel = wm->ball->vel;
    OppGoal = wm->field->oppGoal();
    ballVel = (ballVel.length() < 0.2) ? Vector2D(0,0) : ballVel;
    ballLocation();
    dynamicPlanner(agentSize);
}

void CDynamicAttack::makePlan(int agentSize) {

    //Initialize Plan with NULL values
    currentPlan.mode = DynamicEnums::NoMode;
    currentPlan.agentSize = agentSize;

    for(size_t i = 0;i < 5;i++) {
        currentPlan.agents[i].region = DynamicEnums::NoMatter;
        currentPlan.agents[i].role   = DynamicEnums::NoRole;
        currentPlan.agents[i].skill  = DynamicEnums::NoSkill;
        currentPlan.positionCnt = -1;
    }

    if (playmakeID == -1) {
        isDefenseClearing = true;
    }

    debug(QString("NEW BUG: %1").arg(playmakeID), D_MAHI);

    if (isDefenseClearing) {
        currentPlan.mode = DynamicEnums::DefenseClear;

        for (size_t i = 0; i < agentSize; i++) {
            currentPlan.agents[i].region = DynamicEnums::Near;
            currentPlan.agents[i].role   = DynamicEnums::Position;
            currentPlan.agents[i].skill  = DynamicEnums::Ready;
        }
        currentPlan.positionCnt = agentSize;
        debug(QString("NEW: %1").arg(playmakeID), D_MAHI);
        return;

    }
    // if Defense isn't clearing and
    // we Don't have ball
    else if (knowledge->ballPossesion != CKnowledge::WEHAVETHEBALL) {
        currentPlan.mode = DynamicEnums::NotWeHaveBall;

        currentPlan.agents[0].init(DynamicEnums::PlayMaker,
                                   DynamicEnums::Shot,
                                   DynamicEnums::Goal);

        for(size_t i = 0;i < agentSize;i++) {
            currentPlan.agents[i + 1].region = DynamicEnums::Near;
            currentPlan.agents[i + 1].role   = DynamicEnums::Position;
            currentPlan.agents[i + 1].skill  = DynamicEnums::Ready;
        }

        currentPlan.positionCnt = agentSize;
    }
    // if Defense isn't clearing and
    // we have ball and
    // shot prob is more than 50%
    else if (directShot) {

        currentPlan.mode = DynamicEnums::HighProb;
        currentPlan.agents[0].init(DynamicEnums::PlayMaker,
                                   DynamicEnums::Shot,
                                   DynamicEnums::Goal);

        for(size_t i = 0;i < agentSize;i++) {
            currentPlan.agents[i + 1].region = DynamicEnums::Far;
            currentPlan.agents[i + 1].role   = DynamicEnums::Position;
            currentPlan.agents[i + 1].skill  = DynamicEnums::Mark;
        }
        currentPlan.positionCnt = agentSize;

    }
    // if Defense isn't clearing and
    // we have ball and
    // shot prob isn't more than 50% and
    // there is a critical situation
    else if(critical) {

        currentPlan.mode = DynamicEnums::Critical;
        currentPlan.agents[0].init(DynamicEnums::PlayMaker,
                                   DynamicEnums::Shot,
                                   DynamicEnums::Goal);

        for(size_t i = 0;i < agentSize;i++) {
            currentPlan.agents[i + 1].region = DynamicEnums::Near;
            currentPlan.agents[i + 1].role   = DynamicEnums::Position;
            currentPlan.agents[i + 1].skill  = DynamicEnums::Ready;
        }
        currentPlan.positionCnt = agentSize;
    }
    // if Defense isn't clearing and
    // we have ball and
    // shot prob isn't more than 50% and
    // there isn't a critical situation and
    // it's needed to be fast
    else if(fast) {
        currentPlan.mode = DynamicEnums::Fast;
        currentPlan.agents[0].init(DynamicEnums::PlayMaker,
                                   DynamicEnums::Pass,
                                   DynamicEnums::Near);
        for(size_t i = 0;i < agentSize;i++) {
            currentPlan.agents[i + 1].region = DynamicEnums::Near;
            currentPlan.agents[i + 1].role   = DynamicEnums::Position;
            currentPlan.agents[i + 1].skill  = DynamicEnums::Ready;
        }
        currentPlan.positionCnt = agentSize;
    }
    // if Defense isn't clearing and
    // we have ball and
    // shot prob isn't more than 50% and
    // there isn't a critical situation and
    // there's no need to be fast and
    // there is no plan for this situation
    else if(noPlanException) {

        currentPlan.mode = DynamicEnums::NoPlanExeption;
        currentPlan.agents[0].init(DynamicEnums::PlayMaker,
                                   DynamicEnums::Pass,
                                   DynamicEnums::Best);
        for(size_t i = 0;i < agentSize;i++) {
            currentPlan.agents[i + 1].region = DynamicEnums::Best;
            currentPlan.agents[i + 1].role   = DynamicEnums::Position;
            currentPlan.agents[i + 1].skill  = DynamicEnums::Ready;
        }
        currentPlan.positionCnt = agentSize;
    }
    // if Defense isn't clearing and
    // we have ball and
    // shot prob isn't more than 50% and
    // there is plan for this situation
    // SO WHY THIS CODE EXECUTED !!!
    else {
        debug(QString("ERR: Dynamic Attacks Executed Wrong"),D_MAHI);
        currentPlan.mode = DynamicEnums::NoMode;

        currentPlan.agents[0].init(DynamicEnums::PlayMaker,
                                   DynamicEnums::Shot,
                                   DynamicEnums::NoMatter);

        for(size_t i = 0;i < agentSize;i++) {
            currentPlan.agents[i + 1].region = DynamicEnums::Best;//regionsList.at(i - 1);
            currentPlan.agents[i + 1].role   = DynamicEnums::Position;
            currentPlan.agents[i + 1].skill  = DynamicEnums::Mark;
        }
        currentPlan.positionCnt = agentSize;
    }

//    if (currentPlan.agents[0].skill == DynamicEnums::Pass) {
//        if(agentSize == 0) {
//            if (isBallInOurField) {
//                currentPlan.agents[0].init(DynamicEnums::PlayMaker,
//                                           DynamicEnums::Chip,
//                                           DynamicEnums::Forward);
//            }
//            else {
//                currentPlan.agents[0].init(DynamicEnums::PlayMaker,
//                                           DynamicEnums::Shot,
//                                           DynamicEnums::Goal);
//            }
//            currentPlan.positionCnt = 0;
//        }
//        else {

//            if (isBallInOurField) {
//                currentPlan.agents[0].init(DynamicEnums::PlayMaker,
//                                           DynamicEnums::Chip,
//                                           DynamicEnums::Forward);
//            }
//            /*
//            else {
//                currentPlan.agents[0].init(DynamicEnums::PlayMaker,
//                                           DynamicEnums::Chip,
//                                           DynamicEnums::Goal);
//            }*/
//            currentPlan.positionCnt = agentSize;
//        }
//    }
    if(isBallInOurField)
    {
//        if(agentSize == 0)
//        {
//            currentPlan.agents[0].init(DynamicEnums::PlayMaker,
//                                       DynamicEnums::Shot,
//                                       DynamicEnums::Forward);
//        }
        currentPlan.agents[0].init(DynamicEnums::PlayMaker,
                                   DynamicEnums::Chip,
                                   DynamicEnums::Forward);
        for(size_t i = 0;i < agentSize;i++) {
            currentPlan.agents[i + 1].region = DynamicEnums::Near;//regionsList.at(i - 1);
            currentPlan.agents[i + 1].role   = DynamicEnums::Position;
            currentPlan.agents[i + 1].skill  = DynamicEnums::Ready;
        }
        currentPlan.positionCnt = agentSize;
    }
    debug(QString("DC : %1").arg(currentPlan.positionCnt),D_MAHI);
}

void CDynamicAttack::assignId() {

    double dist2Point[5];
    double minDist = 100000;
    int tempIndex;
    int temp = 0;
    size_t starter = 1;
    QList<int> matchedIDList;
    for(size_t i = 0; i < 6;i++) {
        dist2Point[i] =  10000;
    }

    if(currentPlan.agents[0].role == DynamicEnums::PlayMaker) {
        //        for(size_t i = 0;i < activeAgents.count(); i++) {
        //            dist2Point[i] = activeAgents.at(i)->pos().dist(ballPos + ballVel*2);
        //            if(dist2Point[i] < minDist) {
        //                minDist = dist2Point[i];
        //                mahiAgentsID[0] = activeAgents.at(i)->id();
        //            }
        //        }
        //        mahiPlayMaker = knowledge->getAgent(mahiAgentsID[0]);

        mahiPlayMaker = knowledge->getAgent(playmakeID);
        debug(QString("mahi PM %1").arg(playmakeID),D_MHMMD);
        temp = 1;
    }
    else starter = 0;

    mahiPoisitionAgents.clear();
    guardIndexList.clear();
    for(size_t i = 0; i < activeAgents.size(); i++) {

        for(size_t k = 0; k < 6;k++) {
            dist2Point[i] =  10000;
        }

        minDist = 100000;
        tempIndex  = -1;
        for(size_t j = 0;j < currentPlan.positionCnt;j++) {
            if(!matchedIDList.contains(j)) {
                dist2Point[j] = guards[currentPlan.positionCnt][j].center()
                        .dist(activeAgents.at(i)->pos());
                if(dist2Point[j] < minDist) {
                    minDist = dist2Point[j];
                    tempIndex = j;

                }
            }
        }
        matchedIDList.append(tempIndex);
        guardIndexList.append(tempIndex);
        if(i < activeAgents.size()) {
            mahiAgentsID[i] = activeAgents.at(i)->id();
            mahiPoisitionAgents.append(activeAgents.at(i));
        }
    }

    if(mahiPoisitionAgents.size() > 0)
        debug(QString("1 : %1 , %2").arg(dist2Point[0]).arg(mahiPoisitionAgents.at(0)->id()),D_MAHI);
    if(mahiPoisitionAgents.size() > 1)
        debug(QString("2 : %1 , %2").arg(dist2Point[1]).arg(mahiPoisitionAgents.at(1)->id()),D_MAHI);
    if(mahiPoisitionAgents.size() > 2)
        debug(QString("3 : %1 , %2").arg(dist2Point[2]).arg(mahiPoisitionAgents.at(2)->id()),D_MAHI);

//    for(size_t i = 0; i < currentPlan.agentSize;i++) {
//        for(size_t i = starter; i < 6;i++)
//            dist2Point[i] =  10000;
//        minDist = 100000;

//        for(size_t j = 0; j < activeAgents.size(); j++) {
//            if(!matchedIDList.contains(activeAgents.at(j)->id())) {
//                dist2Point[i] = dynamicPosition.at(i).dist(activeAgents.at(j)->pos());
//                if(dist2Point[i] < minDist) {
//                    minDist = dist2Point[i];
//                    mahiAgentsID[i] = activeAgents.at(j)->id();
//                }
//            }
//        }
//        matchedIDList.append(mahiAgentsID[i]);
//        mahiPoisitionAgents.append(knowledge->getAgent(mahiAgentsID[i]));
//    }

}

void CDynamicAttack::assignTasks() {
    if(currentPlan.agents[0].role == DynamicEnums::PlayMaker) {
        playMake();
        if(currentPlan.agentSize > 0)
            positioning(1, currentPlan.agents[1].skill);
    }
    else {
        positioning(0, currentPlan.agents[1].skill);
    }
}


void CDynamicAttack::dynamicPlanner(int agentSize) {

    activeAgents.clear();
    for(size_t i = 0; i < 5;i++)
        mahiAgentsID[i] = -1;

    for(size_t i = 0;i < agentSize;i++) {
        activeAgents.append(knowledge->getAgent(agentsID.at(i)));
    }

    makePlan(agentSize);
    assignId();
    chooseBestPosForPass();
    if(agentSize > 0 || currentPlan.mode == DynamicEnums::DefenseClear) {
        chooseBestPositons();
        chooseMarkPos();
    }
    assignTasks();
    //    ballLocation();
    //    managePasser();

    debug(QString("MODE : %1").arg(getString(currentPlan.mode)),D_MAHI,QColor(Qt::red));
    debug(QString("ball : %1").arg(isBallInOurField),D_MAHI,QColor(Qt::red));

    for(size_t i = 0;i < agentSize;i++) {
        if(mahiAgentsID[i] >= 0) {
            roleAgents[i + 1]->execute();
            //            debug(QString("W %1").arg(mahiAgentsID[1]), D_MAHI);
        }
    }

    debug(QString("SKILL: %1").arg(roleAgents[0]->getSelectedSkill()), D_MAHI, QColor(Qt::red));
    roleAgents[0]->execute();

    if(semiDynamicPosition.size() > 0)
        draw(semiDynamicPosition[0], 0, QColor(Qt::black));
    if(semiDynamicPosition.size() > 1)
        draw(semiDynamicPosition[1], 0, QColor(Qt::black));
    if(semiDynamicPosition.size() > 2)
        draw(semiDynamicPosition[2], 0, QColor(Qt::black));
    if(semiDynamicPosition.size() > 3)
        draw(semiDynamicPosition[3], 0, QColor(Qt::black));

    //        debug(QString("1 : %1").arg(semiDynamicPosition[1]),D_MAHI);
    //    debug(QString("X : %1").arg(dynamicPosition.size()),D_MAHI);
    //    debug(QString("GIL : %1").arg(guardIndexList.size()),D_MAHI);

    for(size_t i = 0;i < dynamicPosition.size();i++) {
        draw(Circle2D(dynamicPosition.at(i),0.2),QColor(Qt::red),false);
    }
    if (currentPlan.positionCnt > 0 && currentPlan.positionCnt < 4) {
        showRegions(currentPlan.positionCnt, QColor(Qt::gray));
        showLocations(currentPlan.positionCnt, QColor(Qt::red));
    }

    //    debug(QString("Hey : %1").arg(markPositions.size()),D_MAHI);

    if(isPlayMakeChanged()) {
        for(size_t i = 0;i < 5;i++) {
            goToDynamic[i] = false;
        }
    }

}

void CDynamicAttack::playMake() {

    draw(Circle2D(mahiPlayMaker->pos(), 0.1),QColor(Qt::red),true);
    if(wm->getTeamColor() == _COLOR_BLUE)
        draw(Circle2D(mahiPlayMaker->pos() + mahiPlayMaker->dir()*0.08,0.06),QColor(Qt::blue),true);
    else
        draw(Circle2D(mahiPlayMaker->pos() + mahiPlayMaker->dir()*0.08,0.06),QColor(Qt::yellow),true);


    roleAgents[0]->setAgent(mahiPlayMaker);
    roleAgents[0]->setAgentID(mahiAgentsID[0]);
    roleAgents[0]->setAvoidPenaltyArea(true);

    switch(currentPlan.agents[0].skill) {
    case DynamicEnums::Pass:
        roleAgents[0]->setChip(chipOrNot(currentPlan.passPos, 0.5, 0.1));
        if(roleAgents[0]->getChip()) {
            roleAgents[0]->setKickSpeed(appropriateChipSpeed());
        }
        else {
            roleAgents[0]->setKickSpeed(appropriatePassSpeed());
        }
        roleAgents[0]->setTarget(currentPlan.passPos);
        roleAgents[0]->setEmptySpot(false);
        roleAgents[0]->setSelectedSkill(DynamicEnums::Pass);// Skill Kick
        if(isRightTimeToPass()) {
            roleAgents[0]->setNoKick(false); //TEST
        }
        else {
            roleAgents[0]->setNoKick(true);
        }
        break;
    case DynamicEnums::Chip:
        roleAgents[0]->setNoKick(false);
        if (currentPlan.agents[0].region == DynamicEnums::Goal) {
            roleAgents[0]->setTarget(wm->field->oppGoal());
            roleAgents[0]->setKickSpeed(policy()->DynamicPlay_LowSpeedChip());
        }
        else if (currentPlan.agents[0].region == DynamicEnums::Forward) {
            roleAgents[0]->setTarget(Vector2D(1000, 0));
            roleAgents[0]->setKickSpeed(policy()->DynamicPlay_MediumSpeedChip());
        }
        else {
            roleAgents[0]->setTarget(wm->field->oppGoal());
            roleAgents[0]->setKickSpeed(policy()->DynamicPlay_LowSpeedChip());
        }
        roleAgents[0]->setChip(true);
        roleAgents[0]->setSelectedSkill(DynamicEnums::Chip);// Skill Chip
        break;
    default:
    case DynamicEnums::Shot:
        roleAgents[0]->setEmptySpot(true);
        roleAgents[0]->setChip(false);
        roleAgents[0]->setNoKick(false);
        if(wm->getIsSimulMode()) {
            roleAgents[0]->setKickSpeed(8);
        }
        else {
            roleAgents[0]->setKickSpeed(1023);
        }
        roleAgents[0]->setSelectedSkill(DynamicEnums::Shot);// Skill Kick
        break;
    }
}

void CDynamicAttack::positioning(int starter,
                                 DynamicEnums::DynamicSkill _primerySkill) {
    for(size_t i = starter;i < (currentPlan.agentSize + starter);i++) {
        if(mahiAgentsID[i - starter] >= 0) {
            roleAgents[i]->setAgentID(mahiAgentsID[i - starter]);
            roleAgents[i]->setAgent(mahiPoisitionAgents.at(i - starter));
            roleAgents[i]->setAvoidPenaltyArea(true);
            if(i - starter < semiDynamicPosition.size()) {

                switch(_primerySkill) {
                case DynamicEnums::Ready:
                case DynamicEnums::Mark:

                    if(roleAgents[i]->getAgent()->pos()
                            .dist(semiDynamicPosition.at(i - starter)) < 0.1
                            || goToDynamic[i]
                            /*|| fast*/) {
                        goToDynamic[i] = true;
                        if(wm->ball->vel.length() < 1)
                        {
                            roleAgents[i]->setTarget(semiDynamicPosition.at(i - starter));
//                            roleAgents[i]->setTarget(markPositions[i-starter]);
                        }
                        //                        roleAgents[0]->setNoKick(false);
                    }
                    else {
                        if(wm->ball->vel.length() < 1)
                        {
                            roleAgents[i]->setTarget(semiDynamicPosition.at(i - starter));
//                            roleAgents[i]->setTarget(markPositions[i-starter]);
                        }

                        //                        roleAgents[0]->setNoKick(true);
                    }
                    roleAgents[i]->setReceiveRadius(.1);
                    roleAgents[i]->setSelectedSkill(DynamicEnums::Ready);// Receive Skill
                    break;
                    //                case DynamicEnums::Mark:
                    //                    roleAgents[i]->setTarget(markPositions.at(i));
                    //                    roleAgents[i]->setReceiveRadius(0.1);
                    //                    roleAgents[i]->setSelectedSkill(DynamicEnums::Mark);// Receive Skill
                    //                    break;
                case DynamicEnums::Move:
                    roleAgents[i]->setTarget(semiDynamicPosition.at(i - starter));
                    roleAgents[i]->setTargetDir(ballPos - roleAgents[i]->getAgent()->pos());
                    roleAgents[i]->setAvoidPenaltyArea(true);
                    roleAgents[i]->setSelectedSkill(DynamicEnums::Move);
                    break;
                case DynamicEnums::NoSkill:
                    roleAgents[i]->setSelectedSkill(DynamicEnums::Ready);// Receive Skill
                    break;
                }
            }
        }
    }
}


inline bool CDynamicAttack::chipOrNot(Vector2D target,
                                      double _radius, double _treshold) {
    return !isPathClear(ballPos, target, _radius, _treshold);
}

bool CDynamicAttack::keepOrNot() {
    return true;
}

bool CDynamicAttack::isPathClear(Vector2D _pos1,Vector2D _pos2,
                                 double _radius,double treshold) {
    Vector2D sol1,sol2,sol3;
    Line2D _path(_pos1,_pos2);
    Polygon2D _poly;
    Circle2D(_pos2,_radius + treshold).
            intersection(_path.perpendicular(_pos2),&sol1,&sol2);

    _poly.addVertex(sol1);
    sol3 = sol1;
    _poly.addVertex(sol2);
    Circle2D(_pos1,CRobot::robot_radius_new + treshold).
            intersection(_path.perpendicular(_pos1),&sol1,&sol2);

    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    draw(_poly,"red");

    for(int i = 0;i < wm->opp.activeAgentsCount();i++) {
        if(_poly.contains(wm->opp.active(i)->pos)) return false;
    }
    return true;
}

bool CDynamicAttack::isPlayMakeChanged() {

    if(currentPlan.agents[0].role == DynamicEnums::PlayMaker) {
        if(mahiPlayMaker->id() != lastPlayMakerId) {
            lastPlayMakerId = mahiPlayMaker->id();
            return true;
        }
    }
    return false;
}

int CDynamicAttack::appropriatePassSpeed() {

    double tempDistance;

    if(currentPlan.agents[0].role == DynamicEnums::PlayMaker) {
        tempDistance = mahiPlayMaker->pos().dist(currentPlan.passPos);
        if(tempDistance < 2) {
            return policy()->DynamicPlay_LowSpeedPass();
        }
        else if(tempDistance > 4) {
            return policy()->DynamicPlay_HighSpeedPass();
        }
    }
    return policy()->DynamicPlay_MediumSpeedPass();
    policy()->KKPlayOn_KKChipSpeed();
}


int CDynamicAttack::appropriateChipSpeed() {

    double tempDistance;

    if(currentPlan.agents[0].role == DynamicEnums::PlayMaker) {
        tempDistance = mahiPlayMaker->pos().dist(currentPlan.passPos);
        if(tempDistance < 2) {
            return policy()->DynamicPlay_LowSpeedChip();
        }
        else if(tempDistance > 4) {
            return policy()->DynamicPlay_HighSpeedChip();
        }
    }
    return policy()->DynamicPlay_MediumSpeedChip();
}

void CDynamicAttack::chooseBestPositons() {

    int tempIndex = 0;
    double tempDist, minDist;

    semiDynamicPosition.clear();
    for(size_t i = 0;i < guardIndexList.size();i++) {
        for(size_t j = 0;j < 3;j++) {
            tempDist = currentPlan.passPos
                    .dist(guardLocations[currentPlan.positionCnt]
                    [i]
                    [j]);
            if(tempDist < minDist) {
                minDist = tempDist;
                tempIndex = j;
            }
        }
        if(guards[currentPlan.positionCnt][guardIndexList.at(i)]
                .contains(ballPos + ballVel)) {
            tempIndex = farGuardFromPoint(i, ballPos + ballVel);
        }
        if(i < currentPlan.positionCnt) {
            semiDynamicPosition.append(guardLocations[currentPlan.positionCnt]
                    [guardIndexList.at(i)]
                    [tempIndex]);
        }
        else if(currentPlan.mode == DynamicEnums::DefenseClear) {
            semiDynamicPosition.append(Vector2D(0, 0));
        }
        else {
            semiDynamicPosition.append(guardLocations[currentPlan.positionCnt]
                    [guardIndexList.at(currentPlan.positionCnt - 1)]
                    [tempIndex]);
        }
    }

}


void CDynamicAttack::chooseMarkPos() {
    markPositions.clear();
    for(int i = 0; i < 3; i++)
        markPositions.append((Vector2D(5000, 5000)));
    if(roleAgents[0]->getAgent()== NULL)
        return;
    Vector2D reflectPos[3];
    Vector2D temp[4];
    Circle2D oppCircle(Vector2D(_FIELD_WIDTH/2,0) + Vector2D(0.5,0),1.5);
    draw(oppCircle,QColor(Qt::black));
    //first and second marker
    double k = 0;
    bool flag = 1;
    while(flag)
    {
        flag = false;
        oppCircle.intersection(Segment2D(wm->ball->pos, Vector2D(wm->field->oppGoalR().x, wm->field->oppGoalR().y - k)), &temp[0], &temp[3]);
        oppCircle.intersection(Segment2D(wm->ball->pos, Vector2D(wm->field->oppGoalL().x, wm->field->oppGoalL().y + k)), &temp[1], &temp[3]);
        for(int i = 0; i < 2; i++)
        {
            reflectPos[i] = knowledge->getReflectPos(temp[i], 2.5);
            //debug(QString("our reflect points are: %1 %2").arg(reflectPos[i].x).arg(reflectPos[i].y),D_ALI);
        }
        if(reflectPos[0].dist(reflectPos[1]) < 1.5)
            flag = true;

        k += 0.1;
    }
    //third marker
    Vector2D good = Vector2D(_FIELD_WIDTH/2,0);
    if(roleAgents[0]->getAgent()!= NULL && roleAgents[0]->getAgent()->pos().y <= 0)
        good = wm->field->oppGoalR();
    else
        good = wm->field->oppGoalL();
    oppCircle.intersection(Segment2D(wm->ball->pos, good), &temp[2], &temp[3]);
    if(roleAgents[0]->getAgent()!= NULL)
        reflectPos[2] = knowledge->getReflectPos((Vector2D(_FIELD_WIDTH/2,0) + good)/2, max(1.5, min(2.0, roleAgents[0]->getAgent()->pos().dist(Vector2D(_FIELD_WIDTH/2,0)) - 0.9)));
    int l = 3;
    if(roleAgents[0]->getAgent()!= NULL)
    {
        l = 0;
        vector <Vector2D> vt;
        for(int i = 0; i < 3; i++)
            if(reflectPos[i].x < 20)
            {
                l++;
                vt.push_back(reflectPos[i]);
            }
        for(int i = 0; i < l; i++)
            for(int j = 0; j < l - 1; j++)
                if(roleAgents[0]->getAgent()->pos().dist(vt[j]) < roleAgents[0]->getAgent()->pos().dist(vt[j + 1]))
                    swap(vt[j], vt[j + 1]);
        /*for(int i = 0; i < 3; i++)
        {
            reflectPos[i].x = reflectPos[i].y = 5000;
        }*/
        for(int i = 0; i < l; i++)
        {
            reflectPos[i] = vt[i];
        }
    }
    /*reflectPos[0] = knowledge->getReflectPos(wm->field->oppGoal());
    reflectPos[1] = knowledge->getReflectPos(wm->field->oppGoalL());
    reflectPos[2] = knowledge->getReflectPos(wm->field->oppGoalR());*/

    markPositions.clear();

    for(size_t i = 0;i < 3;i++) {
        draw(reflectPos[i], 0, QColor(Qt::cyan));
    }



    MWBM matcher;
    int n = mahiPoisitionAgents.size();
   // debug(QString("%1").arg(n),D_ALI);
    /*for (int i = 0; i < n; ++i) {
        debug(QString("Agent  : %1").arg(mahiPoisitionAgents.at(i)->id()),D_ALI);
    }*/

    matcher.create(n, 3);
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < 3; j++) {
            matcher.setWeight(i,
                              j,
                              (-1) *
                              (mahiPoisitionAgents[i] ->
                              pos().dist(reflectPos[j]) - (reflectPos[i].x < 30 ? 5 * roleAgents[0]->getAgent()->pos().dist(reflectPos[j]) : 0)));

        }
    }
    matcher.findMatching();
    for (int i = 0; i < l; ++i)
    {
        //debug(QString("%1").arg(matcher.getMatch(i)),D_ALI);
        markPositions.append(reflectPos[matcher.getMatch(i)]);
        draw(markPositions.at(i), 0, QColor(Qt::black));
    }
    //debug(QString("Done"),D_ALI);

//    double tempDist,minDist = 10000;
//    int tempIndex = -1;
//    QList<int> matchedIDs;
//    for(size_t i = 0;i < mahiPoisitionAgents.size();i++) {

//        tempDist  = 0;
//        minDist   = 10000;
//        tempIndex = -1;
//        for(size_t j = 0;j < 3;j++) {
//            if(!matchedIDs.contains(j)) {
//                tempDist = mahiPoisitionAgents.at(i)->pos()
//                        .dist(reflectPos[j]);
//                if(tempDist < minDist) {
//                    minDist = tempDist;
//                    tempIndex = j;
//                }
//            }
//        }
//        matchedIDs.append(tempIndex);
//        markPositions.append(reflectPos[tempIndex]);
//    }


}


void CDynamicAttack::chooseBestPosForPass() {
    //    if(regionsList.count()) {
    //        currentPlan.passPos = dynamicPosition.at(0);
    //    }
    //#ifdef SEMIDYNAMIC

    int tempIndex = 0;
    if(isBallInOurField) {
        if(policy()->DynamicPlay_FarForward()) {
            tempIndex = minHorizontalDistID(semiDynamicPosition);
        }
        else {
            tempIndex = maxHorizontalDistID(semiDynamicPosition);
        }
    }
    else {
        if(policy()->DynamicPlay_NearForward()) {
            tempIndex = minHorizontalDistID(semiDynamicPosition);
        }
        else {
            tempIndex = maxHorizontalDistID(semiDynamicPosition);
        }
    }
    if(tempIndex < semiDynamicPosition.size()) {
        currentPlan.passPos = semiDynamicPosition.at(tempIndex);
        currentPlan.passID = tempIndex;
    }

    //    for(size_t i = 0;i < dynamicPosition.size();i++) {

    //        else {

    //        }
    //        tempValue = getDynamicValue(dynamicPosition.at(i));
    //        if(tempValue > maxValue) {
    //            maxValue  = tempValue;
    //            tempIndex = i;
    //        }
    //    }
    //    if(dynamicPosition.size() > tempIndex)
    //        currentPlan.passPos = dynamicPosition.at(tempIndex);
    //#endif
}
double CDynamicAttack::getDynamicValue(const Vector2D &_dynamicPos) const {
    double defMoveAngle, openAngle;
    defMoveAngle = Vector2D::angleOf(ballPos, OppGoal, _dynamicPos).degree();
    return defMoveAngle;
}

Vector2D CDynamicAttack::neaerstGuardToPoint(const Vector2D &startVec) const {
    int regionIndex = 0;
    double tempDist,minDist = 10000;
    const int positionAgentCnt = currentPlan.agentSize - 1;
    int index;
    for(size_t i = 0;i < positionAgentCnt;i++) {
        if(guards[positionAgentCnt][i].contains(startVec)) {
            regionIndex = i;
            break;
        }
    }

    for(size_t i = 0;i < 3;i++) {
        tempDist = guardLocations[positionAgentCnt][regionIndex][i].dist(startVec);
        if(tempDist < minDist) {
            minDist = tempDist;
            index = i;
        }
    }

    return guardLocations[positionAgentCnt][regionIndex][index];
}

bool CDynamicAttack::isRightTimeToPass() {
    double minDist = 99999, tempDist;
    int tempDefIndex;

    for(size_t i = 0;i < mahiPoisitionAgents.size();i++) {
        tempDist = mahiPoisitionAgents.at(i)->pos().dist(currentPlan.passPos);
        if(tempDist < minDist) {
            minDist      = tempDist;
            tempDefIndex = i;
        }
    }
    if(semiDynamicPosition.size() > tempDefIndex) {
        if(mahiPoisitionAgents.at(tempDefIndex)->pos()
                .dist(semiDynamicPosition.at(tempDefIndex)) < policy()->DynamicPlay_Area()) {
            return true;
        }
    }



    return false;
}

void CDynamicAttack::managePasser() {

    //    Vector2D sol1,sol2;

    //    if(!repeatFlag) {
    //        if(currentPlan.agents[0].role == DynamicEnums::PlayMaker) {
    //            if(currentPlan.agents[0].skill == DynamicEnums::Pass) {
    //                if(Circle2D(mahiPlayMaker->pos() + mahiPlayMaker->dir()*0.08,
    //                            0.08).contains(ballPos)) {
    //                    passFlag = true;
    //                }
    //                else if(!Circle2D(mahiPlayMaker->pos(),0.3).contains(ballPos)) {
    //                    if(Circle2D(currentPlan.passPos,1)
    //                            .intersection(Ray2D(ballPos,
    //                                                ballPos + ballVel),
    //                                          &sol1, &sol2)) {
    //                        if(passFlag) {
    //                            passFlag   = false;
    //                            repeatFlag = true;
    //                            passerID   = mahiPlayMaker->id();
    //                        }
    //                    }
    //                }
    //            }
    //        }
    //    }        /////////ISPASSNOTDONE///////////
    //    if(repeatFlag && (counter < 100)) {
    //        counter++;
    //        repeatFlag = true;
    //        for(size_t i = 0; i  < agentsID.size();i++) {
    //            if(roleAgents[i]->getAgent()->id() == passerID)
    //                lastPasserRoleIndex = i;
    //        }

    //        roleAgents[lastPasserRoleIndex]->setAgent(knowledge->getAgent(passerID));
    //        roleAgents[lastPasserRoleIndex]->setAgentID(passerID);
    //        roleAgents[lastPasserRoleIndex]->setAvoidPenaltyArea(true);
    //        roleAgents[lastPasserRoleIndex]->setTarget(Vector2D(0,0));
    //        roleAgents[lastPasserRoleIndex]->setReceiveRadius(0.1);
    //        roleAgents[lastPasserRoleIndex]->setSelectedSkill(DynamicEnums::Mark);

    ////        counter    = 0;
    ////        repeatFlag = false;
    ////        passerID   = -1;
    //    }
    //    else {
    //        passerID = -1;
    //    }

    //    debug(QString("PasserID %1 , Counter : %2").arg(passerID).arg(counter),D_MAHI);
    //    debug(QString("playMake %1 , skill : %2"  ).arg(mahiPlayMaker->id())
    //          .arg(roleAgents[0]->getSelectedSkill()),D_MAHI);
    //    //    debug(QString("LPRI %1 , skill : %2").arg(mahiPlayMaker->id()).arg(roleAgents[0]->getSelectedSkill()),D_MAHI);

}

int CDynamicAttack::farGuardFromPoint(const int &_guardIndex,
                                      const Vector2D &_point) {
    int tempDist, tempIndex, tempMax = 0;
    tempIndex = -1;
    for(size_t i = 0;i < 3;i++) {
        tempDist = guardLocations[currentPlan.positionCnt]
                [guardIndexList.at(_guardIndex)]
                [i].dist(_point);
        if(tempDist  > tempMax) {
            tempMax = tempDist;
            tempIndex = i;
        }
    }
    return tempIndex;
}

void CDynamicAttack::checkPoints(QList<Vector2D>& _points) {

    Rect2D validField(-_FIELD_WIDTH/2,
                      _FIELD_HEIGHT/2,
                      _FIELD_WIDTH,
                      _FIELD_HEIGHT);

    for(size_t i = 0; i < _points.size() - 1;i++) {
        if(!validField.contains(_points.at(i))) {
            _points.removeAt(i);
        }
    }
}

int CDynamicAttack::minHorizontalDistID(const QList<Vector2D> &_points) {
    double tempDist,minDist = 1000;
    int tempIndex;

    for(size_t i = 0;i < dynamicPosition.size();i++) {
        tempDist = fabs(ballPos.y - dynamicPosition.at(i).y);
        if(tempDist < minDist) {
            minDist = tempDist;
            tempIndex = i;
        }
    }
    return tempIndex;
}

int CDynamicAttack::maxHorizontalDistID(const QList<Vector2D> &_points) {
    double tempDist,maxDist = -1;
    int tempIndex;

    for(size_t i = 0;i < dynamicPosition.size();i++) {
        tempDist = fabs(ballPos.y - dynamicPosition.at(i).y);
        if(tempDist > maxDist) {
            maxDist = tempDist;
            tempIndex = i;
        }
    }
    return tempIndex;
}

inline void CDynamicAttack::assignLocations() {

    /////////////////////////////////////
    //No    Region
    /////////////////////////////////////
    //    guardLocations[0][0][0].assign(0,0);
    //    guardLocations[0][0][1].assign(0,0);
    //    guardLocations[0][0][2].assign(0,0);

    /////////////////////////////////////
    //One   Region///////////////////////
    /////////////////////////////////////
    //Opp Feild
    guardLocations[1][0][0].assign(_FIELD_WIDTH/4, _FIELD_HEIGHT/4);
    guardLocations[1][0][1].assign(_FIELD_WIDTH/4, 0);
    guardLocations[1][0][2].assign(_FIELD_WIDTH/4, -_FIELD_HEIGHT/4);
    ///////////////////////////////////
    //Two   Regions////////////////////
    ///////////////////////////////////
    //Top Opp Half
    guardLocations[2][0][0].assign(0.5, 1  );
    guardLocations[2][0][1].assign(2  , 1.5);
    guardLocations[2][0][2].assign(3.5, 2  );

    //    guardLocations[2][0][0].assign(_FIELD_WIDTH/8, _FIELD_HEIGHT/4);
    //    guardLocations[2][0][1].assign(_FIELD_WIDTH/4, _FIELD_HEIGHT/4);
    //    guardLocations[2][0][2].assign(_FIELD_WIDTH/2 - _FIELD_WIDTH/8, //x
    //                                   _FIELD_HEIGHT/4); //y
    //Bottom Opp Half
    guardLocations[2][1][0].assign(0.5, -1  );
    guardLocations[2][1][1].assign(2  , -1.5);
    guardLocations[2][1][2].assign(3.5, -2  );
    //    guardLocations[2][1][0].assign(_FIELD_WIDTH/8, -_FIELD_HEIGHT/4);
    //    guardLocations[2][1][1].assign(_FIELD_WIDTH/4, -_FIELD_HEIGHT/4);
    //    guardLocations[2][1][2].assign(_FIELD_WIDTH/2 - 1, //x
    //                                   -_FIELD_HEIGHT/4);  //y

    //////////////////////////////////////
    //Three Regions///////////////////////
    //////////////////////////////////////1
    //Top Opp Tertium
    guardLocations[3][0][0].assign(1  , 1);
    guardLocations[3][0][1].assign(2.5, 2);
    guardLocations[3][0][2].assign(3.5, 2);
    //Middle Opp Tertium
    guardLocations[3][1][0].assign(0.5, 0);
    guardLocations[3][1][1].assign(1.5, 0);
    guardLocations[3][1][2].assign(2.5, 0);
    //Bottom Opp Tertium
    guardLocations[3][2][0].assign(1  , -1);
    guardLocations[3][2][1].assign(2.5, -2);
    guardLocations[3][2][2].assign(3.5, -2);

    //    //////////////////////////////////////
    //    //Three Regions///////////////////////
    //    //////////////////////////////////////
    //    //Top Opp Tertium
    //    guardLocations[3][0][0].assign(_FIELD_WIDTH/8, _FIELD_HEIGHT/3);
    //    guardLocations[3][0][1].assign(_FIELD_WIDTH/4, _FIELD_HEIGHT/3);
    //    guardLocations[3][0][2].assign(_FIELD_WIDTH/2 - _FIELD_WIDTH/8, //x
    //                                   _FIELD_HEIGHT/3); //y
    //    //Middle Opp Tertium
    //    guardLocations[3][1][0].assign(_FIELD_WIDTH/8, 0);
    //    guardLocations[3][1][1].assign(_FIELD_WIDTH/4 - 0.5, 0);
    //    guardLocations[3][1][2].assign(_FIELD_WIDTH/2 - _FIELD_WIDTH/8 - 1, 0);
    //    //Bottom Opp Tertium
    //    guardLocations[3][2][0].assign(_FIELD_WIDTH/8, -_FIELD_HEIGHT/3);
    //    guardLocations[3][2][1].assign(_FIELD_WIDTH/4, -_FIELD_HEIGHT/3);
    //    guardLocations[3][2][2].assign(_FIELD_WIDTH/2 - _FIELD_WIDTH/8, //x
    //                                   -_FIELD_HEIGHT/3); //y

}

inline void CDynamicAttack::assignRegions() {

    //Opp Field
    guards[0]->assign(-_CENTER_CIRCLE_RAD,                 //top_X
                      _FIELD_HEIGHT/2,                     //top_Y
                      _FIELD_WIDTH/2 + _CENTER_CIRCLE_RAD, //X_Length
                      _FIELD_HEIGHT);                      //Y_Length

    //Opp Field
    guards[1][0].assign(-_CENTER_CIRCLE_RAD,                 //top_X
                        _FIELD_HEIGHT/2,                     //top_Y
                        _FIELD_WIDTH/2 + _CENTER_CIRCLE_RAD, //X_Length
                        _FIELD_HEIGHT);                      //Y_Length

    //Top Opp Half
    guards[2][0].assign(-_CENTER_CIRCLE_RAD,                //top_X
                        _FIELD_HEIGHT/2,                    //top_Y
                        _FIELD_WIDTH/2 + _CENTER_CIRCLE_RAD,//X_Length
                        _FIELD_HEIGHT/2);                   //Y_Length

    //Bottom Opp Half
    guards[2][1].assign(-_CENTER_CIRCLE_RAD,                 //top_X
                        0,                                   //top_Y
                        _FIELD_WIDTH/2 + _CENTER_CIRCLE_RAD, //X_Length
                        _FIELD_HEIGHT/2);                    //Y_Length

    //Top Opp Tertium
    guards[3][0].assign(-_CENTER_CIRCLE_RAD,                 //top_X
                        _FIELD_HEIGHT/2,                     //top_Y
                        _FIELD_WIDTH/2 + _CENTER_CIRCLE_RAD, //X_Length
                        _FIELD_HEIGHT/3);                    //Y_Length

    //Middle Opp Tertium
    guards[3][1].assign(-_CENTER_CIRCLE_RAD,                 //top_X
                        _FIELD_HEIGHT/2 - _FIELD_HEIGHT/3,   //top_Y
                        _FIELD_WIDTH/2 + _CENTER_CIRCLE_RAD, //X_Length
                        _FIELD_HEIGHT/3);                    //Y_Length

    //Bottom Opp Tertium
    guards[3][2].assign(-_CENTER_CIRCLE_RAD,                 //top_X
                        _FIELD_HEIGHT/2 - 2*_FIELD_HEIGHT/3, //top_Y
                        _FIELD_WIDTH/2 + _CENTER_CIRCLE_RAD, //X_Length
                        _FIELD_HEIGHT/3);                    //Y_Length
}

inline void CDynamicAttack::showRegions(unsigned int agentSize,
                                        QColor color) {

    for(size_t i = 0; i < agentSize; i++) {
        draw(guards[agentSize][i], color);
    }
}

inline void CDynamicAttack::showLocations(unsigned int agentSize,
                                          QColor color) {
    for(size_t i = 0;i < agentSize;i++) { //RegionsCount
        for(size_t j = 0;j < 3;j++) {
            draw(guardLocations[agentSize][i][j], 0, color);
        }
    }
}

QString CDynamicAttack::getString(const DynamicEnums::DynamicMode &_mode) const {
    switch(_mode) {
    default:
    case DynamicEnums::NoMode:
        return QString("NoMode");
        break;
    case DynamicEnums::DefenseClear:
        return QString("DefenseClear");
        break;
    case DynamicEnums::NotWeHaveBall:
        return QString("NotWeHaveBall");
        break;
    case DynamicEnums::HighProb:
        return QString("HighProb");
        break;
    case DynamicEnums::NoPlanExeption:
        return QString("NoPlanExeption");
        break;
    case DynamicEnums::Fast:
        return QString("Fast");
        break;
    case DynamicEnums::Critical:
        return QString("Critical");
        break;
    case DynamicEnums::Plan:
        return QString("NewPlan");
        break;
    }
}

void CDynamicAttack::ballLocation() {
    if(ballPos.x < -1 ) {
        isBallInOurField = true;
    }
    else if(ballPos.x > 0) {
        isBallInOurField = false;
    }
}

void CDynamicAttack::setDefenseClear(bool _isDefenseClearing) {
    isDefenseClearing = _isDefenseClearing;
}

void CDynamicAttack::setDirectShot(bool _directShot) {
    directShot = _directShot;
}

void CDynamicAttack::setPositions(QList<int> _positioningRegion) {
    regionsList.clear();
    dynamicPosition.clear();
    for(size_t i = 0;i < _positioningRegion.size();i++) {
        regionsList.append(_positioningRegion.at(i));
        dynamicPosition.append(knowledge->getStaticPoses
                               (_positioningRegion.at(i)));
    }
}

void CDynamicAttack::setPlayMake(int _playMake) {
    playmakeID = _playMake;
}

void CDynamicAttack::setWeHaveBall(bool _ballPoss) {
    isWeHaveBall = _ballPoss;
}

void CDynamicAttack::setNoPlanException(bool _noPlanException) {
    noPlanException = _noPlanException;
}

void CDynamicAttack::setCritical(bool _critical) {
    critical = _critical;
}

void CDynamicAttack::setFast(bool _fast) {
    fast = _fast;
}
