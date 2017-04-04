#include "plays/playoff.h"


CPlayOff::CPlayOff()
{
    qDebug() << "Bring yourself back online playoff";

    radLimit = 2;
    decidePlan = true;
    firstTime = true;
    agentSize = 1;
    for(int i = 0;i < 6;i++) positionAgent[i].stateNumber = 0;
    ballEnteredKickerFlag = false;
    ballEnteredKickerChipFlag = false;
    passReceivedFlag = false;
    isPassDoneflag = false;
    for(int i = 0;i < 6;i++) {
        isFirstTime[i] = true;
        roleAgent[i] = new CRolePlayOff();
        newRoleAgent[i] = new CRolePlayOff();
        isBallNearRobot[i] = false;
        isBallNearRobotF[i] = false;

    }
    isBallIn = false;
    cnt = 0;
    tempAgent = new CRolePlayOff();
    doPass = false;
    setTimer = true;
    kickOffFirstTimeFlag = true;
    ////////////

    currentPlan = new SPlayOffPlan();
    masterPlan = NULL;
    kickOffPos[0] = Vector2D(wm->ball->pos.x - 0.3, wm->ball->pos.y);
    kickOffPos[1] = Vector2D(-0.3,  2.5);
    kickOffPos[2] = Vector2D(-0.3, -2.5);
    kickOffPos[3] = Vector2D(-2  ,  0);
    kickOffPos[4] = Vector2D(-0.5,  1);
    kickOffPos[5] = Vector2D(-0.5, -1);

    initial    = true;
    playOnFlag = false;
    havePassInPlan = false;

    //Dynamic
    ready = pass = shot = false;
    dynamicStartTime = -1;
    dynamicSelect = NOSELECT;
}

CPlayOff::~CPlayOff()
{
    qDebug() << "Playoff is gone";
    for(int i = 0;i < 6;i++) {
        delete roleAgent[i];
        delete newRoleAgent[i];
    }
    delete tempAgent;



}

void CPlayOff::loadSQL()
{
    kkPOPlanSQL = QSqlDatabase::addDatabase("QSQLITE");
    kkPOPlanSQL.setDatabaseName(directory);
    //kkPOPlanSQL = QSqlDatabase::database("playoff");
    if (!kkPOPlanSQL.open()) {
        QMessageBox::critical(0, "Cannot open database",
                              "Unable to establish a database connection.\n"\
                              "This example needs SQLite support. Please read "\
                              "the Qt SQL driver documentation for information how "\
                              "to build it.\n\n"\
                              "Click Cancel to exit.", QMessageBox::Cancel);
    }
    qDebug() << "PlayOff SQL Connected!";
}

Vector2D CPlayOff::convertPos(int _x, int _y, int _symmetry)
{
    _x -= 429;
    _y -= 328;

    double tempX = double(_x)/404.5;
    double tempY = -double(_y)/304;

    tempX = tempX*(_FIELD_WIDTH/2);
    tempY = tempY*(_FIELD_HEIGHT/2);
    return Vector2D(tempX, (_symmetry)*tempY);
}
void CPlayOff::loadEachPlan(SPlayOffPlan *_plan, QString _name, int _symmetry)
{
    QSqlQuery squery;
    squery.exec("SELECT * FROM "+_name+" ORDER BY id ASC");
    playOffRobot tempStep;
    while(squery.next())
    {
        for(int i = 0; i < 6; i++)
        {
            if(loadSQLtoStruct(squery, i, tempStep, _symmetry))
                _plan->AgentPlan[i].append(tempStep);
        }
    }
}

bool CPlayOff::loadSQLtoStruct(QSqlQuery _query,
                               int _rIndex,
                               playOffRobot &temp,
                               int _symmetry)
{
    //    if(_query.value(_rIndex*7 + 1).toString() == "na")
    //        return false;
    //    //playOffRobot temp;
    //    temp.pos = convertPos(_query.value(_rIndex*7 + 1).toString().split("|").at(0).toInt(),
    //                          _query.value(_rIndex*7 + 1).toString().split("|").at(1).toInt(), _symmetry);

    //    temp.angle = AngleDeg((_symmetry)*_query.value(_rIndex*7 + 1).toString().split("|").at(2).toInt());

    //    temp.tolerance = _query.value(_rIndex*7 + 2).toDouble()/100;
    ////    temp.skillSize = _query.value(_rIndex*7 + 3).toInt();

    //    for(int i = 0; i < 3; i++)
    //    {
    //        if(_query.value(_rIndex*7 + 4).toString() != "na")
    //        {
    //            temp.skill[i] = POffSkills(_query.value(_rIndex*7 + 4 + i).toString().split("|").at(0).toInt());
    //            temp.skillData[i][0] = _query.value(_rIndex*7 + 4 + i).toString().split("|").at(1).toInt();
    //            temp.skillData[i][1] = _query.value(_rIndex*7 + 4 + i).toString().split("|").at(2).toInt();
    //        }
    //        else
    //        {
    //            temp.skill[i] = NoSkill;
    //            temp.skillData[i][0] = 1000;
    //            temp.skillData[i][1] = 1000;
    //        }
    //    }

    //    temp.targetAgent = _query.value(_rIndex*7 + 7).toString().split("|").at(0).toInt();
    //    temp.targetIndex = _query.value(_rIndex*7 + 7).toString().split("|").at(1).toInt();

    //    return true;
}

bool CPlayOff::getMatchedPlan(POMODE _mode, int agentSize, bool _rand)
{

    QList<SPlayOffPlan*> planList;

    int planCnt = 0;
    if(_mode == KICKOFF) {
        for(int i =0 ;i < planListKickOff.size();i++) {
            planList.append(planListKickOff.at(i));
        }

    }
    else if(_mode == DIRECT)
    {
        for(int i =0 ;i < planListDirect.size();i++) {
            planList.append(planListDirect.at(i));
        }

    }
    else if(_mode == INDIRECT)
    {
        for(int i =0 ;i < planListIndirect.size();i++) {
            planList.append(planListIndirect.at(i));
        }

    }


    qDebug() << QString(" PLAN LIST SIZE : %1").arg(planList.size());
    if(planList.isEmpty()) return false;
    else {
        for(int i = 0;i < planList.size();i++) {
            if(planList.at(i)->agentSize == agentSize) planCnt++;
        }
        if(planCnt == 0) return false;
    }
    //    Vector2D tempBallPos = wm->ball->pos;
    QList<distAndId> tempBallDist;
    distAndId temp;
    double min = 1000.0;
    int minId = 0;
    for(int i = 0; i < planList.count(); i++)
    {
        if(planList.at(i)->agentSize != agentSize)
        {
            continue;
        }
        temp.id   = i;
        temp.dist = planList.at(i)->initPos.ball.dist(wm->ball->pos);
        tempBallDist.append(temp);

        if(temp.dist < min)
        {
            min = temp.dist;
            minId = i;
        }
    }
    //    debug(QString("%1, %2").arg(planList.at(0).AgentPlan[0].size()).arg(planList.count()), D_KK);

    QList<SPlayOffPlan*> selectedPlans;
    int cnt = 0;
    for(size_t i = 0; i < tempBallDist.count(); i++)
    {
        if(tempBallDist.at(i).dist <= radLimit)
        {
            selectedPlans.append(planList.at(tempBallDist.at(i).id));
        }
    }

    debugs[1] = selectedPlans.size();
    circles[0] = Circle2D(wm->ball->pos,radLimit);
    for(size_t i = 0; i < planList.size();i++) {
        draws[i] = planList.at(i)->initPos.ball;
    }

    //    for(size_t i = 0; i < 6; i++)
    //    {
    //        currentPlan->AgentPlan[i].clear();
    //    }
    currentPlan = NULL;
    if(!selectedPlans.isEmpty() && _rand)
    {
        int fullChance = 0;
        for (size_t i = 0;i < selectedPlans.size();i++) {
            fullChance += selectedPlans.at(i)->config.chance;
        }
        if (fullChance < 1) fullChance = 1;
        int selectedID = 0;
        int randomSelect = rand()%fullChance + 1;
        qDebug() << randomSelect << "RAND";
        int sumChance = 0;
        for (size_t i = 0;i < selectedPlans.size();i++) {
            sumChance += selectedPlans.at(i)->config.chance;
            if (randomSelect <= sumChance) {
                selectedID = i;
                break;
            }
        }
        qDebug() << QString(" SELECTED : %1 / %2").arg(selectedID).arg(selectedPlans.size());
        qDebug() << QString("FullChance : %1").arg(fullChance);
        currentPlan = selectedPlans.at(selectedID);

    }
    else
    {
        currentPlan = planList.at(minId);
    }
    debugs[0] = selectedPlans.size();

    return true;
}


void CPlayOff::getPassTimeline(SPlayOffPlan *tCurrentPlan, QList<POOwnerReceive> &tList)
{
    tList.clear();
    QList<kkTimeAndIndex> temp;
    kkTimeAndIndex tempStruct;

    tempStruct.agent = -1;
    tempStruct.index = -1;
    tempStruct.time = 0;
    tempStruct.skill = NoSkill;

    for(int i = 0; i < tCurrentPlan->agentSize; i++)
    {
        tempStruct.agent = -1;
        tempStruct.index = -1;
        tempStruct.time = 0;
        tempStruct.skill = NoSkill;
        for(int j = 0; j < tCurrentPlan->AgentPlan[i].count(); j++)
        {
            for(int k = 0; k < tCurrentPlan->AgentPlan[i].at(j).skill.size(); k++)
            {
                if(tCurrentPlan->AgentPlan[i].at(j).skill[k].name == PassSkill)
                {
                    tempStruct.skill = tCurrentPlan->AgentPlan[i].at(j).skill[k].name;
                    tempStruct.agent = i;
                    tempStruct.index = j;
                    temp.append(tempStruct);

                    tempStruct.agent = -1;
                    tempStruct.index = -1;
                    //tempStruct.time = 0;
                    tempStruct.skill = NoSkill;
                }
                else
                {
                    tempStruct.time += tCurrentPlan->AgentPlan[i].at(j).skill[k].data[0];
                    tempStruct.time += tCurrentPlan->AgentPlan[i].at(j).skill[k].data[1];
                }
            }
        }
    }

    for(int i = 0; i < temp.count(); i++)
    {
        for(int j = i; j < temp.count(); j++)
        {
            if(temp.at(i).time > temp.at(j).time)
            {
                temp.swap(i, j);
            }
        }
    }

    POOwnerReceive tempOaR;
    for(int i = 0; i < temp.count(); i++)
    {
        tempOaR.ballOwnerAgent = temp.at(i).agent;
        tempOaR.ballOwnerIndex = temp.at(i).index;
        tempOaR.skill = temp.at(i).skill;
        tList.append(tempOaR);
    }
}

void CPlayOff::globalExecute() {

    if (masterMode == NGameOff::StaticPlay) {
        Q_ASSERT(masterPlan != NULL);
        if(masterPlan != NULL) {
            if (initial) {
                qDebug() << *masterPlan;
                lastBallPos = wm->ball->pos;
            }
            staticExecute();
        } else {
            qDebug() << "master is null";
            initial = true;
            return;
        }

    } else {
        mainExecute();
    }
}

bool CPlayOff::isBallMoved() {
    if(wm->ball->pos.dist(lastBallPos) > 0.25) return true;
    else return false;
}


void CPlayOff::mainExecute() {
    switch(masterMode) {
    case NGameOff::StaticPlay:
        staticExecute();
        break;
    case NGameOff::DynamicPlay:
        dynamicExecute();
        break;
    case NGameOff::FirstPlay:
        firstExecute();
        break;
    case NGameOff::FastPlay:
        fastExecute();
        break;
    default:
        break;
    }
}


void CPlayOff::staticExecute() {
    if (initial) {
        newAssignTasks();

    } else {
        if (knowledge->getGameState() != CKnowledge::OurKickOff) {

            newFillRoleProperties();
            newPosExecute();
            newCheckEndState();

            debug(QString("sag id : %1").arg(roleAgent[0]->getAgent()->id()),D_MAHI);
            if(masterPlan->common.currentSize > 1 && havePassInPlan) {
                passManager();
            }
            if(newIsPlanEnd()) {
                playOnFlag = true;
            }

        } else {
            kickOffStopModePlay(masterPlan->common.currentSize);
            for (int i = 0; i < masterPlan->common.currentSize; i++) {
                newRoleAgent[i]->execute();
            }
        }
    }
}


void CPlayOff::dynamicExecute() {


    if (dynamicSelect == CHIP) {
        dynamicPlayChipToGoal();
        checkEndChipToGoal();
    } else if (dynamicSelect == KHAFAN) {
        dynamicPlayKhafan();
        checkEndKhafan();
    } else if (dynamicSelect == BLOCKER) {
        dynamicPlayBlocker();
        checkEndBlocker();
    }



        for(int i = 0;i < dynamicAgentSize;i++) {
            roleAgent[i]->execute();
        }
}


void CPlayOff::dynamicAssignID() {
    dynamicAgentSize = 6;
    for (int i = 0;i < 6;i++) {
        if (dynamicMatch[i] != -1) {
            roleAgent[i] -> setAgent(knowledge->getAgent(dynamicMatch[i]));
            roleAgent[i] -> setAgentID(dynamicMatch[i]);
        } else {
            dynamicAgentSize = i;
            break;
        }
    }
}

void CPlayOff::dynamicPlayChipToGoal() {
    if (initial) {
        dynamicAssignID();
        ready = true;

    } else if (ready) {
        roleAgent[0] -> setAvoidCenterCircle(false);
        roleAgent[0] -> setAvoidPenaltyArea(true);
        roleAgent[0] -> setChip(true);
        double speed = knowledge->getProfile(roleAgent[0]->getAgentID(), roleAgent[0]->getAgent()->pos().dist(wm->field->oppGoal()), false, false);
        roleAgent[0] -> setKickSpeed(speed); // Vartypes This
        roleAgent[0] -> setTarget(wm->field->oppGoal());
        roleAgent[0] -> setDoPass(false);
        roleAgent[0] -> setIntercept(false);
        roleAgent[0] -> setLookForward(false);
        roleAgent[0] -> setSelectedSkill(roleSkill::Kick);

        for (int i = 1; i < dynamicAgentSize; i++) {
            if (dynamicMatch[i] != -1) {
                roleAgent[i] -> setAvoidPenaltyArea(true);
                roleAgent[i] -> setAvoidBall(true);
                roleAgent[i] -> setTimeBased(false);
                roleAgent[i] -> setTarget(getDynamicTarget(i + 1));
                roleAgent[i] -> setTargetDir(wm->field->oppGoal() - roleAgent[i]->getAgent()->pos());
                roleAgent[i] -> setEventDist(0.3);
                roleAgent[i] -> setSlow(false);
                roleAgent[i] -> setSelectedSkill(roleSkill::GotopointAvoid);
            }
        }

        ready = false;

    } else if (shot) {
        roleAgent[0] -> setDoPass(true);
        shot = false;
    }
}

void CPlayOff::dynamicPlayBlocker() {
    if (initial) {
        dynamicAssignID();
        ready = true;

    } else if (ready) {
        roleAgent[0] -> setAvoidCenterCircle(false);
        roleAgent[0] -> setAvoidPenaltyArea(true);
        roleAgent[0] -> setChip(false);
        roleAgent[0] -> setKickSpeed(knowledge->getProfile(roleAgent[0]->getAgentID(), 7.8, false, false)); // Vartypes This
        roleAgent[0] -> setTarget(wm->field->oppGoal().rotatedVector((wm->ball->pos.y < 0 ? 90 : -90)));
        roleAgent[0] -> setDoPass(false);
        roleAgent[0] -> setIntercept(false);
        roleAgent[0] -> setLookForward(false);
        roleAgent[0] -> setSelectedSkill(roleSkill::Kick);

        for (int i = 1; i < dynamicAgentSize; i++) {
            if (dynamicMatch[i] != -1) {
                roleAgent[i] -> setAvoidPenaltyArea(true);
                roleAgent[i] -> setAvoidBall(true);
                roleAgent[i] -> setTimeBased(false);
                roleAgent[i] -> setTarget(getDynamicTarget(i + 1));
                roleAgent[i] -> setTargetDir(wm->field->oppGoal() - roleAgent[i]->getAgent()->pos());
                roleAgent[i] -> setEventDist(0.3);
                roleAgent[i] -> setSlow(false);
                roleAgent[i] -> setSelectedSkill(roleSkill::GotopointAvoid);
            }
        }

        ready = false;

    } else if (shot) {

        roleAgent[0] -> setKickSpeed(knowledge->getProfile(roleAgent[0]->getAgentID(), 7.8, false, false)); // Vartypes This
        roleAgent[0] -> setTarget(wm->field->oppGoal());
        roleAgent[0] -> setDoPass(true);
        roleAgent[0] -> setTargetDir(wm->field->oppGoal());
        roleAgent[0] -> setSelectedSkill(roleSkill::Kick);
        shot = false;
    }

}

void CPlayOff::dynamicPlayKhafan() {
    if (initial) {
        dynamicAssignID();
        ready = true;

    } else if (ready) {
        roleAgent[0] -> setAvoidCenterCircle(false);
        roleAgent[0] -> setAvoidPenaltyArea(true);
        roleAgent[0] -> setChip(true);

        roleAgent[0] -> setKickSpeed(policy()->DynamicPlay_LowSpeedChip()); // Vartypes This

        roleAgent[0] -> setTarget(wm->field->oppGoal());
        roleAgent[0] -> setDoPass(false);
        roleAgent[0] -> setIntercept(false);
        roleAgent[0] -> setTargetDir(wm->field->oppGoal());
        roleAgent[0] -> setSelectedSkill(roleSkill::Kick);

        for (int i = 1; i < dynamicAgentSize; i++) {
            if (dynamicMatch[i] != -1) {
                roleAgent[i] -> setAvoidPenaltyArea(true);
                roleAgent[i] -> setAvoidBall(true);
                roleAgent[i] -> setTimeBased(false);
                roleAgent[i] -> setTarget(getDynamicTarget(i));
                roleAgent[i] -> setTargetDir(wm->field->oppGoal() - roleAgent[i]->getAgent()->pos());
                roleAgent[i] -> setEventDist(0.3);
                roleAgent[i] -> setSlow(false);
                roleAgent[i] -> setSelectedSkill(roleSkill::GotopointAvoid);
            }
        }

        ready = false;

    } else if (pass) {
        roleAgent[0] -> setDoPass(true);
        pass = false;
        debug("DYNAMIC :D ", D_MAHI);

    } else if (shot) {
        roleAgent[1] -> setAvoidCenterCircle(false);
        roleAgent[1] -> setAvoidPenaltyArea(true);
        roleAgent[1] -> setChip(false);
        roleAgent[1] -> setKickSpeed(1023); // Vartypes This
        roleAgent[1] -> setTarget(wm->field->oppGoal());
        roleAgent[1] -> setDoPass(true);
        roleAgent[1] -> setIntercept(false);
        roleAgent[1] -> setTargetDir(wm->field->oppGoal());
        roleAgent[1] -> setSelectedSkill(roleSkill::Kick);
        shot = false;

        roleAgent[0] -> setAvoidPenaltyArea(true);
        roleAgent[0] -> setAvoidBall(true);
        roleAgent[0] -> setTimeBased(false);
        roleAgent[0] -> setTarget(Vector2D(0,-2));
        roleAgent[0] -> setTargetDir(wm->field->oppGoal() - roleAgent[0]->getAgent()->pos());
        roleAgent[0] -> setEventDist(0.3);
        roleAgent[0] -> setSlow(false);
        roleAgent[0] -> setSelectedSkill(roleSkill::GotopointAvoid);


    }

}


void CPlayOff::checkEndKhafan() {
    if (ready) {
        dynamicState = 2;
    } else if (pass) {
        dynamicState = 4;
    } else if (shot) {
        dynamicState = 6;
    }

    if (dynamicState == 2) {
        if (roleAgent[1] -> getAgent() -> pos().dist(roleAgent[1] -> getTarget())
                < roleAgent[1] -> getEventDist()) {
            dynamicState = 4;
            pass = true;
        }
    }

    if (dynamicState == 4) {
        debug(QString("[dastan] : %1").arg(knowledge->getCurrentTime() - dynamicStartTime), D_MAHI);
        if (wm->ball->pos.dist(wm->field->oppGoal()) - 0.5 < roleAgent[1]->getAgent()->pos().dist(wm->field->oppGoal())) {
            pass = false;
            shot = true;
            dynamicState = 6;
        }
        if (!Circle2D(roleAgent[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos) && dynamicStartTime == -1) {
            dynamicStartTime = knowledge->getCurrentTime();
        }

        if (wm->ball->vel.length() < 0.2 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;
        }

        if (knowledge->getCurrentTime() - dynamicStartTime > 100 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;

        }
    }

    if (dynamicState == 6) {
        // TODO : check this
        playOnFlag = true;
        shot = false;
        if (wm->ball->vel.length() < 0.2) {
            playOnFlag = true;
            dynamicState = 0;
        }
        debug(QString("[dastan] : %1").arg(knowledge->getCurrentTime() - dynamicStartTime), D_MAHI);

        if (knowledge->getCurrentTime() - dynamicStartTime > 200 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;

        }

    }

}

void CPlayOff::checkEndBlocker() {
    if (ready) {
        dynamicState = 2;
    } else if (shot) {
        dynamicState = 6;
    }



    if (dynamicState == 2) {
        for (int i = 0;i < wm->opp.activeAgentsCount(); i++) {
            if (Circle2D(roleAgent[0] -> getAgent() -> pos() + roleAgent[0]->getAgent()->dir().norm()*0.6, 0.3).contains(wm->opp.active(i)->pos))
                if(roleAgent[0]->getAgent()->dir().norm().dist(roleAgent[0]->getTarget().norm()) < 0.1) {
                    dynamicState = 6;
                    shot = true;
            }
        }

        dynamicStartTime = knowledge->getCurrentTime();

    }

    if (dynamicState == 6) {

        if (!Circle2D(roleAgent[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos)) {
            playOnFlag = true;
            dynamicState = 0;
        }

        if (knowledge->getCurrentTime() - dynamicStartTime > 300 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;
        }

    }
}

void CPlayOff::checkEndChipToGoal() {
    if (ready) {
        dynamicState = 2;
    } else if (shot) {
        dynamicState = 6;
    }



    if (dynamicState == 2) {
        if (Circle2D(wm->ball->pos, 0.5).contains(roleAgent[0]->getAgent()->pos())) {
            shot = true;
            dynamicState = 6;
            dynamicStartTime = knowledge->getCurrentTime();
        }
    }

    if (dynamicState == 6) {

        if (!Circle2D(roleAgent[0]->getAgent()->pos(), 0.5).contains(wm->ball->pos)) {
            playOnFlag = true;
            dynamicState = 0;
        }

        if (knowledge->getCurrentTime() - dynamicStartTime > 200 && dynamicStartTime != -1) {
            playOnFlag = true;
            dynamicState = 0;
        }

    }
}

Vector2D CPlayOff::getDynamicTarget(int i) {
    Vector2D first = wm->ball->pos+(wm->field->oppGoal() - wm->ball->pos).norm()*0.7;
    first.y += 0.3;

    switch (i) {
    case 1:
        return first;
    case 2:
        if(first.y < -_FIELD_HEIGHT / 6) return Vector2D(3,  2.25);
        if(first.y > _FIELD_HEIGHT  / 6) return Vector2D(3, -2.25);
        return Vector2D(3, -2.25);
    case 3:
        if(first.y < -_FIELD_HEIGHT / 6) return Vector2D(2,  1);
        if(first.y > _FIELD_HEIGHT  / 6) return Vector2D(2, -1);
        return Vector2D(3,  2.25);
    case 4:
        if(first.y < -_FIELD_HEIGHT / 6) return Vector2D(2,  0);
        if(first.y > _FIELD_HEIGHT  / 6) return Vector2D(2, -0);
        return Vector2D(3,  0);
    default:
        return Vector2D::INVALIDATED;
        break;
    }
}



void CPlayOff::fastExecute() {
    // TODO : Write fast Execution (playoff)

}

void CPlayOff::firstExecute() {
    // TODO : Write first Execution (playoff)

}

void CPlayOff::mahiDebug(int limit) {
    for(size_t i = 0; i < limit;i++) {
        debug(QString("No.%1 : %2").arg(i).arg(debugs[i]),D_MAHI);
    }
}

void CPlayOff::mahiCircle(int limit) {
    for(size_t i = 0; i < limit;i++) {
        draw(circles[i],QColor(Qt::red));
    }
}

void CPlayOff::mahiVector(int limit) {
    for(size_t i = 0; i < limit;i++) {
        draw(draws[i],0,QColor(Qt::blue));
    }
}

void CPlayOff::kickOffStopModePlay(int tAgentsize) {

    for (int i = 0; i < masterPlan->common.currentSize; i++) {
        if (newRoleAgent[i]->getRoleUpdate() == false) {
            newRoleAgent[i]->setUpdated(true);
            newRoleAgent[i]->setAgent(knowledge->getAgent(masterPlan->common.matchedID.value(i)));
            newRoleAgent[i]->setRoleUpdate(true);
            newRoleAgent[i]->setAvoidBall(true);
            newRoleAgent[i]->setAvoidPenaltyArea(true);
            newRoleAgent[i]->setSelectedSkill(roleSkill::GotopointAvoid);

        }
    }

    switch(tAgentsize) {
    case 1:
        oneBehindBall();
        break;
    case 2:
        oneLeftOneCentre();
        //            oneRightOneCentre();
        break;
    case 3:
        twoSidesOneCentre();
        break;
    case 4:
        twoSideOneCentreOneDef();
        break;
    case 5:
        twoSideOneCentreTwoDef();
        break;
    case 6:
        twoSideOneCentreTwoDefAndGoalie();
        break;
    default:
        break;
    }

}

void CPlayOff::oneBehindBall() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-activeAgents.at(0)->pos() + wm->ball->pos);
}

void CPlayOff::oneLeftOneCentre() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-activeAgents.at(0)->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-activeAgents.at(1)->pos() + wm->field->oppGoal());
}

void CPlayOff::oneRightOneCentre() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-activeAgents.at(0)->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-activeAgents.at(1)->pos() + wm->field->oppGoal());
}

void CPlayOff::twoSidesOneCentre() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-activeAgents.at(0)->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-activeAgents.at(1)->pos() + wm->field->oppGoal());
    newRoleAgent[2]->setTarget(kickOffPos[2]);
    newRoleAgent[2]->setTargetDir(-activeAgents.at(2)->pos() + wm->field->oppGoal());
}

void CPlayOff::twoSideOneCentreOneDef() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-newRoleAgent[0]->getAgent()->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-newRoleAgent[1]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[2]->setTarget(kickOffPos[2]);
    newRoleAgent[2]->setTargetDir(-newRoleAgent[2]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[3]->setTarget(kickOffPos[3]);
    newRoleAgent[3]->setTargetDir(-newRoleAgent[3]->getAgent()->pos() + wm->field->oppGoal());
}

void CPlayOff::twoSideOneCentreTwoDef() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-newRoleAgent[0]->getAgent()->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-newRoleAgent[1]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[2]->setTarget(kickOffPos[2]);
    newRoleAgent[2]->setTargetDir(-newRoleAgent[2]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[3]->setTarget(kickOffPos[4]);
    newRoleAgent[3]->setTargetDir(-newRoleAgent[4]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[4]->setTarget(kickOffPos[5]);
    newRoleAgent[4]->setTargetDir(-newRoleAgent[4]->getAgent()->pos() + wm->field->oppGoal());
}

void CPlayOff::twoSideOneCentreTwoDefAndGoalie() {
    newRoleAgent[0]->setTarget(kickOffPos[0]);
    newRoleAgent[0]->setTargetDir(-newRoleAgent[0]->getAgent()->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-newRoleAgent[1]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[2]->setTarget(kickOffPos[2]);
    newRoleAgent[2]->setTargetDir(-newRoleAgent[2]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[3]->setTarget(kickOffPos[3]);
    newRoleAgent[3]->setTargetDir(-newRoleAgent[3]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[4]->setTarget(kickOffPos[4]);
    newRoleAgent[4]->setTargetDir(-newRoleAgent[4]->getAgent()->pos() + wm->field->oppGoal());
    newRoleAgent[5]->setTarget(kickOffPos[5]);
    newRoleAgent[5]->setTargetDir(-newRoleAgent[5]->getAgent()->pos() + wm->field->oppGoal());
}

int CPlayOff::matchKickOffID(int _agentSize) {
    double dist2Point[6];
    double minDist = 100000;
    QList<int> matchedIDList;
    int tempId;
    for(size_t i = 0; i < 6;i++) {
        dist2Point[i] =  10000;
    }

    matchedIDList.clear();

    for(size_t i = 0;i < activeAgents.size(); i++) {
        dist2Point[i] = activeAgents.at(i)->pos().dist(wm->ball->pos);
        if(dist2Point[i] < minDist) {
            minDist = dist2Point[i];
            tempId = activeAgents.at(i)->id();
        }
    }
    newRoleAgent[0]->setAgent(knowledge->getAgent(tempId));
    matchedIDList.append(tempId);

    for(size_t i = 1; i < _agentSize;i++) {

        for(size_t k = 0; k < 6;k++)
            dist2Point[k] =  10000;
        minDist = 100000;

        for (size_t j = 0; j < activeAgents.size(); j++) {
            if (!matchedIDList.contains(activeAgents.at(j)->id())) {
                dist2Point[i] = kickOffPos[i].dist(activeAgents.at(j)->pos());
                if(dist2Point[i] < minDist) {
                    minDist = dist2Point[i];
                    tempId = activeAgents.at(j)->id();
                }
            }
        }
        newRoleAgent[i]->setAgent(knowledge->getAgent(tempId));
        matchedIDList.append(tempId);
    }



}

void CPlayOff::kickOffExecute() {
    for(int i = 0;i < agentSize;i++) {
        newRoleAgent[i]->execute();
    }
}

bool CPlayOff::isPlanEnd() {
    if (isTimeOver()) {
        draw("Time Over", Vector2D(1, -0.6));
        return true;
    }
    else if (isBallDirChanged()) {
        draw("Ball Dir is Changed", Vector2D(1, -0.6));
        return true;
    }
    else if (isFinalShotDone()) {
        draw("Final Shot Done!", Vector2D(1, -0.6));
        return true;
    }
    return false;
}

bool CPlayOff::newIsPlanEnd() {

    if (isPlanDone()) {
        debug("Plan Succeded", D_MAHI);
        return true;

    } else if (isPlanFaild()) {
        if (lastBallPos.dist(wm->ball->pos) > 0.06) {
            debug("Plan Fully Failed", D_MAHI);
            return true;
        } else {
            debug("rePlaning", D_MAHI);
            return false;
        }
    }

    return false;
}

bool CPlayOff::isPlanDone() {
    const int& tLastAgent = masterPlan->execution.theLastAgent;
    const int& tLastState = masterPlan->execution.theLastState;

    // Plan doesn't include a final shoot
    if (tLastState == -1 || tLastAgent == -1) {
        if (isAllTasksDone()) {
            debug ("Done By Fully Tasks Done", D_MAHI);
            masterPlan->common.addHistory(10); //FULL
            return true;
        }
    } else if (isFinalShotDone()) {
        debug ("Done By Final Shot !", D_MAHI);
        // TODO : IF GOAL THEN 10 ELSE 9
        masterPlan->common.addHistory(10); //FULL
        return true;
    }
    return false;
}


bool CPlayOff::isPlanFaild() {
    SFail fail = isAnyTaskFaild();
    if (isTimeOver()) {
        debug ("Faild By Time Over", D_MAHI);
        //        masterPlan->common.addHistory(); // Not Changeing History
        return true;
    } else if (isBallDirChanged()) {
        debug ("Faild By Ball Dir Changed", D_MAHI);
        masterPlan->common.addHistory(0); // MIN
        return true;
    } else if (fail = isAnyTaskFaild(), fail.fail) {
        debug(QString("Task Fail : TaskID %1").arg(fail.taskID), D_MAHI);
        debug(QString("Task Fail : AgentID %1").arg(fail.agentID), D_MAHI);
        debug(QString("Task Fail : PlanID %1").arg(fail.planID), D_MAHI);
        debug(QString("Task Fail : RoleID %1").arg(fail.roleID), D_MAHI);
        debug(QString("Task Fail : Mode %1").arg(fail.mode), D_MAHI);
        debug(QString("Task Fail : Skill %1").arg(fail.skill), D_MAHI);
        masterPlan->common.addHistory(fail.succesRate);
        return true;
    }
    return false;
}

SFail CPlayOff::isAnyTaskFaild() {
    SFail faliure;
    faliure.fail = false;
    // TODO : complete function
    // TODO : calculate succes rate
    return faliure;
}

bool CPlayOff::isAllTasksDone() {

    // if there's a agent that is not zombie
    for (size_t i = 0; i < masterPlan->common.currentSize; i++) {
        if (!positionAgent[i].zombie) {
            return false;
        }
    }

    return true;
}

bool CPlayOff::isTimeOver() {
    if (setTimer) {
        tempStart = knowledge->getCurrentTime();
    }

    if (!Circle2D(lastBallPos, 0.5).contains(wm->ball->pos)) {
        setTimer = false;
        debug(QString("Time That Left: %1").arg(knowledge->getCurrentTime() - tempStart), D_DEBUG);
        if(knowledge->getCurrentTime() - tempStart > 200) { // 2 Second
            setTimer = true;
            return true;
        }
    }
    return false;
}

bool CPlayOff::isBallDirChanged() {
    // USE PASSER FORM INITIAL LEVEL
    const int& passer = masterPlan->execution.passer.id;
    const int& recive = masterPlan->execution.reciver.id;
    Vector2D& b  = wm->ball->pos;
    if (b.dist(lastBallPos) > 0.5 && !roleAgent[passer]->getChip()) {
        Vector2D  bv = b + wm->ball->vel.norm()*_MAX_DIST;
        Circle2D  c(roleAgent[recive]->getWaitPos(), 1); // TODO : CHECK radius
        Segment2D s(b, bv);
        draw(s, QColor(Qt::blue));
        draw(c, QColor(Qt::red));
        Vector2D v1,v2;
        if (!c.intersection(s, &v1, &v2)) return true;
    } else {
        return false;
    }

}

bool CPlayOff::isFinalShotDone() {

    const int& tLastAgent = masterPlan->execution.theLastAgent;
    const int& tLastState = masterPlan->execution.theLastState;

    // Plan hasn't a final shoot
    if (tLastState == -1 || tLastAgent == -1) return false;

    CAgent* tAgent = knowledge ->
            getAgent(masterPlan -> common.matchedID[tLastAgent]);

    Circle2D cir (tAgent->pos() + tAgent->dir().norm()*0.08, 0.16);
    Circle2D cir2(tAgent->pos() + tAgent->dir().norm()*0.20, 0.40);

    draw(cir , QColor(Qt::blue));
    draw(cir2, QColor(Qt::blue));

    if (positionAgent[tLastAgent].stateNumber == tLastState) {
        if (cir.contains(wm->ball->pos)) {

            isBallIn = true;

        } else if (isBallIn && !cir2.contains(wm->ball->pos)) {

            isBallIn = false;
            return true;

        }
    }

    return false;
}


bool CPlayOff::isTaskFaild(int agent) {
    if(positionAgent[agent].positionArg.size())
        switch(positionAgent[agent].positionArg.at(positionAgent[agent].stateNumber).staticSkill) {
        case NoSkill:
            return false;
            break;
        case PassSkill:
            if(isPassFaild(agent)){
                debug(QString("PASS FAILD"),D_MAHI);
                draw(QString("PASS FAILD"),Vector2D(2,-1.5));
                return true;
            }
            return false;
            break;
        case ReceivePassSkill:
            if(isReceiveFaild(agent)) {
                debug(QString("RECIEVE FAILD"),D_MAHI);
                draw(QString("RECEIVE FAILD"),Vector2D(2,-1.5));

                return true;
            }
            return false;
            break;
        case ReceivePassIASkill:
            if(isReceiveFaild(agent)) {
                debug(QString("RECEIVE AI FAILD"),D_MAHI);
                draw(QString("RECEIVE AI FAILD"),Vector2D(2,-1.5));

                return true;
            }
            return false;
            break;
        case ShotToGoalSkill:
            debug(QString("hey : %1").arg(isKickFaild(agent)),D_MAHI);
            if(isKickFaild(agent) != 0) {
                debug(QString("KICK FAILD"),D_MAHI);
                draw(QString("KICK FAILD"),Vector2D(2,-1.5));

                return true;
            }
            else {
                return false;
            }
            break;
        case ChipToGoalSkill:
            if(isKickFaild(agent)) {
                debug(QString("CHIP FAILD"),D_MAHI);
                draw(QString("CHIP FAILD"),Vector2D(2,-1.5));

                return true;
            }
            return false;
            break;
        case MoveSkill:
            return false;
            break;
        case OneTouchSkill:
            if(isKickFaild(agent)) {
                debug(QString("ONET FAILD"),D_MAHI);
                draw(QString("ONET FAILD"),Vector2D(2,-1.5));

                return true;
            }
            return false;
            break;
        default : return false;
        }
    return false;
}

bool CPlayOff::isKickFaild(int agent) {
    if(Circle2D(roleAgent[agent]->getAgent()->pos(),1).contains(wm->ball->pos)) {
        isBallNearRobotF[agent] = true;
    }
    else if(!Circle2D(roleAgent[agent]->getAgent()->pos(),1.5).contains(wm->ball->pos) && isBallNearRobotF[agent]) {
        isBallNearRobotF[agent] = false;
        return true;
    }
    return false;
}

bool CPlayOff::isPassFaild(int agent) {
    bool isBallInCorrectWay;
    Vector2D sol1,sol2;

    if(roleAgent[agent]->getChip() == true) {
        debug("IT'S A CHIP", D_MAHI);
        return false;
    } else {
        draw(Circle2D(roleAgent[agent]->getTarget(),1.0), QColor(Qt::black));
        draw(Segment2D(wm->ball->pos,wm->ball->pos + wm->ball->vel * 10), QColor(Qt::black));
        if(Circle2D(roleAgent[agent]->getTarget(),1.0).intersection(Segment2D(wm->ball->pos,wm->ball->pos + wm->ball->vel * 10), &sol1, &sol2)) {
            isBallInCorrectWay = true;
        }
        else {
            isBallInCorrectWay = false;
        }

        if (isBallMoved() && !isBallInCorrectWay) {
            return true;
        }
    }
    return false;
}

bool CPlayOff::isReceiveFaild(int agent) {
    if(Circle2D(roleAgent[agent]->getAgent()->pos(),1).contains(wm->ball->pos)) {
        isBallNearRobotF[agent] = true;
    }
    if(isBallNearRobotF[agent] && !Circle2D(roleAgent[agent]->getAgent()->pos(),1.5).contains(wm->ball->pos)) {
        isBallNearRobotF[agent] = false;
        debug(QString("FIRST FAILD : %1").arg(agent),D_MAHI);
        return true;
    }

    //removed!
    //    if(knowledge->getCurrentKKTime() - positionAgent[agent].mahiLastTime
    //            > positionAgent[agent].positionArg.at(positionAgent[agent].stateNumber).rightData + 500) {
    //        debug(QString("SECOND FAILD : %1").arg(agent),D_MAHI);
    //        return true;
    //    }
    return false;
}

bool CPlayOff::isTasksDone() {
    int cnt = 0;
    int finalAgent;
    for(int  i  = 0;i < agentSize;i++) {
        if(positionAgent[i].stateNumber + 1 == positionAgent[i].positionArg.size()) {
            cnt++;
            finalAgent = i;
        }
    }
    if(cnt == agentSize) {

        if(isTaskFaild(finalAgent) || isTaskDone(finalAgent))
            return true;
    }
    return false;
}
///////////////PassManager///////////////////
void CPlayOff::passManager() {
    // TODO : FOR MORE THAN ONE PASS

    const AgentPoint& p = masterPlan->execution.passer;
    const AgentPoint& r = masterPlan->execution.reciver;

    const int& i = masterPlan->common.matchedID.value(r.id);

    CAgent* c    = knowledge->getAgent(i);
    if (positionAgent[r.id].stateNumber == r.state
            ||  positionAgent[r.id].stateNumber == r.state + 1) {
        debug(QString("RC : %1, %2").arg(r.id).arg(r.state), D_MAHI);
        if (positionAgent[r.id].getAbsArgs(r.state).staticPos.dist(c -> pos()) >
                masterPlan->common.lastDist) {
            doPass = false;

        } else {
            doPass = true;
        }
        roleAgent[p.id]->setDoPass(doPass);

    }
}

///////////////////////////////////////////////
long CPlayOff::timeTillPass() {
    long tempTime = 100000000000;
    int tempBallOwner,tempOnwerIndex;
    if(ownerReceiveList.size()) {
        tempBallOwner = ownerReceiveList.at(0).ballOwnerAgent;
        tempOnwerIndex = ownerReceiveList.at(0).ballOwnerIndex;
        tempTime = 0;
        for(int i = positionAgent[tempBallOwner].stateNumber;i < ownerReceiveList.at(0).ballOwnerIndex;i++) {
            tempTime += positionAgent[tempBallOwner].positionArg.at(i + 1).rightData;
            tempTime += positionAgent[tempBallOwner].positionArg.at(i + 1).leftData;
            //            tempTime -= knowledge->getCurrentKKTime() - positionAgent[tempBallOwner].mahiLastTime; // //removed!
        }
    }
    return tempTime;
}

long CPlayOff::timeTillReceive() {
    long tempTime = 0;
    int tempReceiver,tempReceiverState;
    if(ownerReceiveList.size()) {
        tempReceiver = ownerReceiveList.at(0).receiveAgent;
        tempReceiverState = ownerReceiveList.at(0).receiveIndex;
        for(int i = positionAgent[tempReceiver].stateNumber;i < ownerReceiveList.at(0).receiveIndex;i++) {

            tempTime += positionAgent[tempReceiver].positionArg.at(i).rightData;
            tempTime += positionAgent[tempReceiver].positionArg.at(i).leftData;
            //            tempTime -= knowledge->getCurrentKKTime() - positionAgent[tempReceiver].mahiLastTime; //removed!
        }
    }
    return tempTime;
}

void CPlayOff::terminateReceiverTasks() {
    if(ownerReceiveList.size()) {
        isFirstTime[ownerReceiveList.at(0).receiveAgent] = true;
        if(positionAgent[ownerReceiveList.at(0).receiveAgent].stateNumber < ownerReceiveList.at(0).receiveIndex)
            positionAgent[ownerReceiveList.at(0).receiveAgent].stateNumber++;
        //        positionAgent[ownerReceiveList.at(0).receiveAgent].stateNumber = ownerReceiveList.at(0).receiveIndex;
        //        debug(QString("NEW RECEIVER INDEX : %1").arg(positionAgent[ownerReceiveList.at(0).receiveIndex].stateNumber),D_KK);
    }
}

///////////////////////////////////////////
void CPlayOff::checkEndState() {
    SPositioningArg tempPA;
    for(int i = 0;i < agentSize;i++) {
        if(isTaskDone(i)) {
            if(positionAgent[i].stateNumber + 1 < positionAgent[i].positionArg.size())
            {
                positionAgent[i].stateNumber++;
                isFirstTime[i] = true;
            }
            else {
                //                tempPA = positionAgent[i].positionArg.at(positionAgent[i].stateNumber);
                //                tempPA.staticSkill = NoSkill;
                //                positionAgent[i].positionArg.replace(positionAgent[i].stateNumber,tempPA);
                //                markAgents.append(knowledge->getAgent(kkAgentsID[i]));
            }
        }
    }
}


/**
 * @brief CPlayOff::isTaskDone
 * @param agentID
 * @return ture if task get done
 * Old Version
 */
bool CPlayOff::isTaskDone(int agentID){
    CAgent* tAgent;
    tAgent = knowledge->getAgent(kkAgentsID[agentID]);


    if(roleAgent[agentID]->getSelectedSkill() == roleSkill::GotopointAvoid) {
        if(isMoveDone(agentID))
            return true;
    }
    else if(roleAgent[agentID]->getSelectedSkill() == roleSkill::Kick) {
        if(isKickDone(tAgent,agentID))
            return true;
    }
    else if(roleAgent[agentID]->getSelectedSkill() == roleSkill::ReceivePass) {
        if(isReceiveDone(tAgent))  {
            draw(Circle2D(Vector2D(0,0),1),QColor(Qt::black),true);
            if(ownerReceiveList.size())
                ownerReceiveList.removeFirst();
            return true;
        }
        else {
            draw(Circle2D(Vector2D(0,0),1),QColor(Qt::blue),true);
            return false;
        }
    }
    else if(roleAgent[agentID]->getSelectedSkill() == roleSkill::OneTouch) {
        if(isKickDone(tAgent,agentID))
            return true;
    }
    else if(roleAgent[agentID]->getSelectedSkill() == roleSkill::Mark) {
        return true;
    }
    else {
        return false;
    }
}
/**
 * @brief CPlayOff::isTaskDone
 * @param _roleAgent
 * @return true if the task get done
 */
bool CPlayOff::isTaskDone(CRolePlayOff* _roleAgent){

    switch (_roleAgent->getSelectedSkill()) {
    case roleSkill::Gotopoint:
    case roleSkill::GotopointAvoid:
        return isMoveDone(_roleAgent);
        break;
    case roleSkill::Kick:
        return isKickDone(_roleAgent);
        break;
    case roleSkill::OneTouch:
        return isOneTouchDone(_roleAgent);
        break;
    case roleSkill::ReceivePass:
        return isReceiveDone(_roleAgent);
        break;
        // After Life
    case roleSkill::Mark:
    case roleSkill::Support:
    case roleSkill::Defense:
        qDebug() << "got it";
        _roleAgent->setRoleUpdate(false);
        return false;
        break;
    default:
        return false;
    }
}

////////////////////////////////////////////
///////////////////////////////////////////
bool CPlayOff::isMoveDone(int agentID) {
    long tempDiffTime;
    //    tempDiffTime = knowledge->getCurrentKKTime() - positionAgent[agentID].mahiLastTime;//removed!
    //    debug(QString("nuTimer : %1").arg(positionAgent[agentID].mahiLastTime),D_MAHI);//removed!
    debug(QString("nuTimer2 : %1").arg(tempDiffTime),D_MAHI);
    if(tempDiffTime > positionAgent[agentID].positionArg.at(positionAgent[agentID].stateNumber).rightData/10 +
            positionAgent[agentID].positionArg.at(positionAgent[agentID].stateNumber).leftData/10)
        return true;
    return false;
}

///////////////////////////////////////////
bool CPlayOff::isReceiveDone(CAgent *_agent) {
    if(Circle2D(_agent->pos() + _agent->dir().norm()*0.08,0.08).contains(wm->ball->pos)) return true;
    else return false;
}

///////////////////////////////////////////
bool CPlayOff::isKickDone(CAgent *_agent,int agentID) {
    bool isBallInCorrectWay;
    Vector2D sol1,sol2;
    if(Circle2D(roleAgent[agentID]->getTarget(),0.6).intersection(Segment2D(wm->ball->pos,wm->ball->pos + wm->ball->vel * 10),&sol1,&sol2)) {
        isBallInCorrectWay = true;

    }
    else {
        isBallInCorrectWay = false;

    }
    draw(Circle2D(_agent->pos(),0.7),0,360);
    if(Circle2D(_agent->pos(),0.7).contains(wm->ball->pos)) {
        isBallNearRobot[agentID] = true;
    }
    else if(isBallNearRobot[agentID] && isBallInCorrectWay) {
        isBallNearRobot[agentID] = false;
        isBallNearRobotF[agentID] = false;
        return true;
    }
    return false;
}

void CPlayOff::posExecute() {
    for(int i = 0;i < agentSize;i++) {
        if(isFirstTime[i]) {
            debug(QString("done"),D_MAHI);
            //            positionAgent[i].mahiLastTime = knowledge->getCurrentKKTime();//removed!
            isFirstTime[i] = false;
        }
        roleAgent[i]->execute();
    }
}


void CPlayOff::newPosExecute() {
    for(int i = 0;i < masterPlan->common.currentSize; i++) {
        roleAgent[i]->execute();

    }

}

void CPlayOff::newCheckEndState() {

    for(int i = 0;i < masterPlan->common.currentSize;i++) {

        if(isTaskDone(roleAgent[i])) {

            roleAgent[i]->setRoleUpdate(false);
            roleAgent[i]->resetTime();

            if(positionAgent[i].stateNumber + 1  < positionAgent[i].positionArg.size()) {
                positionAgent[i].stateNumber++;
                isFirstTime[i] = true;

            } else {
                positionAgent[i].zombie = true;
                /////Temp
                SPositioningArg tempPA;
                tempPA = positionAgent[i].getAbsArgs(positionAgent[i].positionArg.size() - 1);
                tempPA.staticSkill = MoveSkill;
                positionAgent[i].positionArg.append(tempPA);
            }
        }
    }
}

///////////////////////////////////////////////
//////////////////////////////////////////////
void CPlayOff::fillRolesProperties(){
    for(int i = 0;i < agentSize;i++) {
        roleAgent[i]->setAgent(knowledge->getAgent(kkAgentsID[i]));
        if(positionAgent[i].stateNumber < positionAgent[i].positionArg.size())
            assignTask(i, positionAgent[i].positionArg.at(positionAgent[i].stateNumber).staticSkill);
    }
}

void CPlayOff::newFillRoleProperties() {
    for(size_t i = 0;i < agentsID.size(); i++) {
        if (masterPlan->common.matchedID.contains(i)) {
            if (roleAgent[i]->getRoleUpdate() == false) {

                roleAgent[i]->setFirstMove((positionAgent[i].stateNumber == 0));
                roleAgent[i]->setAgent(knowledge->getAgent(masterPlan->common.matchedID.value(i)));
                if (positionAgent[i].stateNumber + 1 < positionAgent[i].positionArg.size()) {
                    if (positionAgent[i].getArgs().staticSkill == MoveSkill && positionAgent[i].getArgs(1).staticSkill == OneTouchSkill) {
                        positionAgent[i].stateNumber++;
                    }
                }
                newAssignTask(roleAgent[i], positionAgent[i]);
                roleAgent[i]->setRoleUpdate(true);
                roleAgent[i]->resetTime();
            }

        } else {
            qWarning() << "[Warning] coach -> Match function doesn't work :( ";
            if (roleAgent[i]->getRoleUpdate() == false) {
                roleAgent[i]->setAgent(knowledge->getAgent(agentsID.at(i)));
                newAssignTask(roleAgent[i], positionAgent[i]);
                roleAgent[i]->setRoleUpdate(true);
            }

        }
    }
}


void CPlayOff::assignTask(int agentID, POffSkills agentSkill) {
    Vector2D tempVec = (wm->ball->pos - knowledge->getAgent(kkAgentsID[agentID])->pos()).norm();
    roleAgent[agentID]->setSlow(false);
    switch(agentSkill) {
    case PassSkill:
        roleAgent[agentID]->setAvoidCenterCircle(false);
        roleAgent[agentID]->setAvoidPenaltyArea(true);
        roleAgent[agentID]->setChip(chipOrNot(agentID,positionAgent[agentID].getArgs().PassToId,
                                              positionAgent[agentID].getArgs().PassToState));
        if (roleAgent[agentID]->getChip()) {
            roleAgent[agentID]->setKickSpeed(positionAgent[agentID].getArgs().rightData);
        }
        else {
            roleAgent[agentID]->setKickSpeed(positionAgent[agentID].getArgs().leftData);
        }
        roleAgent[agentID]->setTarget(positionAgent[positionAgent[agentID].getArgs().PassToId]. \
                positionArg[positionAgent[agentID].getArgs().PassToState].staticPos);
        roleAgent[agentID]->setDoPass(doPass);
        //        roleAgent[agentID]->setTarget(positionAgent[positionAgent[agentID].positionArg.at(positionAgent[agentID].stateNumber).PassToId] \
        //                                      .positionArg.at(positionAgent[positionAgent[agentID] \
        //                                                      .positionArg.at(positionAgent[agentID].stateNumber).PassToId] \
        //                                                      .stateNumber).staticPos);
        roleAgent[agentID]->setIntercept(false);
        roleAgent[agentID]->setSelectedSkill(roleSkill::Kick);
        break;
    case ReceivePassSkill:
        roleAgent[agentID]->setAvoidPenaltyArea(true);
        roleAgent[agentID]->setTarget(positionAgent[agentID].getArgs().staticPos);
        roleAgent[agentID]->setReceiveRadius(positionAgent[agentID].getArgs().staticEscapeRadius);
        roleAgent[agentID]->setIgnoreAngle(false);
        roleAgent[agentID]->setSelectedSkill(roleSkill::ReceivePass);
        break;
    case ReceivePassIASkill:
        roleAgent[agentID]->setAvoidPenaltyArea(true);
        roleAgent[agentID]->setTarget(positionAgent[agentID].getArgs().staticPos);
        roleAgent[agentID]->setReceiveRadius(positionAgent[agentID].getArgs().staticEscapeRadius);
        roleAgent[agentID]->setIgnoreAngle(true);
        roleAgent[agentID]->setTargetDir(positionAgent[agentID].getArgs().staticAng);
        roleAgent[agentID]->setSelectedSkill(roleSkill::ReceivePass);
        break;
    case ShotToGoalSkill:
        //        roleAgent[agentID]->setChip(false);
        roleAgent[agentID]->setChip(isPathClear(wm->ball->pos,wm->field->oppGoal(),0.5,0.1));
        roleAgent[agentID]->setKickSpeed(positionAgent[agentID].getArgs().leftData);
        roleAgent[agentID]->setTarget(getGoalTarget(agentID,positionAgent[agentID].stateNumber));
        roleAgent[agentID]->setIntercept(false);
        roleAgent[agentID]->setSelectedSkill(roleSkill::Kick);
        break;
    case ChipToGoalSkill:
        roleAgent[agentID]->setChip(true);
        roleAgent[agentID]->setKickSpeed(positionAgent[agentID].getArgs().leftData);
        roleAgent[agentID]->setTarget(getGoalTarget(agentID,positionAgent[agentID].stateNumber));
        roleAgent[agentID]->setIntercept(false);
        roleAgent[agentID]->setSelectedSkill(roleSkill::Kick);
        break;
    case OneTouchSkill:
        //        roleAgent[agentID]->setAvoidPenaltyArea(true);
        //        roleAgent[agentID]->setChip(false);
        roleAgent[agentID]->setWaitPos(positionAgent[agentID].getArgs().staticPos);
        roleAgent[agentID]->setKickSpeed(positionAgent[agentID].getArgs().leftData);
        roleAgent[agentID]->setTarget(getGoalTarget(agentID, positionAgent[agentID].stateNumber));
        roleAgent[agentID]->setSelectedSkill(roleSkill::OneTouch);
        break;
    case MoveSkill:
        roleAgent[agentID]->setAvoidPenaltyArea(true);
        roleAgent[agentID]->setTargetDir(positionAgent[agentID].getArgs().staticAng);
        roleAgent[agentID]->setMaxVelocity(getMaxVel(agentID, positionAgent[agentID].stateNumber));
        if(positionAgent[agentID].getArgs().staticPos == POBALLPOS) {
            roleAgent[agentID]->setTarget(wm->ball->pos - tempVec*0.14);
            roleAgent[agentID]->setTargetDir(tempVec);
            roleAgent[agentID]->setSlow(true);
            roleAgent[agentID]->setMaxVelocity(1);
        } else {
            roleAgent[agentID]->setTarget(getMoveTarget(agentID, positionAgent[agentID].stateNumber));
        }



        roleAgent[agentID]->setSelectedSkill(roleSkill::GotopointAvoid);
        break;
    case NoSkill:
        roleAgent[agentID]->setSelectedSkill(roleSkill::Mark);
        break;
    }
}

void CPlayOff::newAssignTask(CRolePlayOff* _roleAgent, const SPositioningAgent& _positionAgent) {

    switch(_positionAgent.getArgs().staticSkill) {
    case PassSkill:
        assignPass(_roleAgent, _positionAgent);
        break;
    case ReceivePassSkill:
        assignReceive(_roleAgent, _positionAgent, false);
        break;
    case ReceivePassIASkill:
        assignReceive(_roleAgent, _positionAgent, true);
        break;
    case ShotToGoalSkill:
        assignKick(_roleAgent, _positionAgent, false);
        break;
    case ChipToGoalSkill:
        assignKick(_roleAgent, _positionAgent, true);
        break;
    case OneTouchSkill:
        assignOneTouch(_roleAgent, _positionAgent);
        break;
    case MoveSkill:
        assignMove(_roleAgent, _positionAgent);
        break;
    case NoSkill:
        assignAfterLife(_roleAgent, _positionAgent);
        break;
    }
}

void CPlayOff::assignPass(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent) {
    _roleAgent->setAvoidCenterCircle(false);
    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setChip(chipOrNot(_posAgent.getArgs()));
    if (_roleAgent->getChip()) {
        _roleAgent->setKickRealSpeed(static_cast <double> (_posAgent.getArgs().rightData)/100);

    } else {
        _roleAgent->setKickRealSpeed(static_cast <double> (_posAgent.getArgs().leftData)/100);

    }

    _roleAgent->setTarget(positionAgent[_posAgent.getArgs().PassToId].getAbsArgs(_posAgent.getArgs().PassToState).staticPos);
    _roleAgent->setDoPass(doPass);
    _roleAgent->setIntercept(false);
    _roleAgent->setTargetDir(_posAgent.getArgs().staticAng);
    _roleAgent->setSelectedSkill(roleSkill::Kick);
}

void CPlayOff::assignReceive(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent, bool _ignoreAngle) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setIgnoreAngle(_ignoreAngle);
    _roleAgent->setTarget(_posAgent.getArgs().staticPos);
    _roleAgent->setTargetDir(_posAgent.getArgs().staticAng); /** Just Matter when we use Ignore mode **/
    _roleAgent->setReceiveRadius(_posAgent.getArgs().staticEscapeRadius);
    _roleAgent->setSelectedSkill(roleSkill::ReceivePass);
}

void CPlayOff::assignKick(CRolePlayOff* _roleAgent,
                          const SPositioningAgent& _posAgent, bool _chip) {

    _roleAgent->setChip(_chip);
    _roleAgent->setKickRealSpeed(static_cast<double>(_posAgent.getArgs().leftData)/100);
    _roleAgent->setTarget(getGoalTarget(_posAgent.getArgs().rightData));
    _roleAgent->setIntercept(false);
    _roleAgent->setSelectedSkill(roleSkill::Kick);
}

void CPlayOff::assignOneTouch(CRolePlayOff* _roleAgent,
                              const SPositioningAgent& _posAgent) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setWaitPos(_posAgent.getArgs().staticPos);
    _roleAgent->setKickSpeed(_posAgent.getArgs().leftData);
    _roleAgent->setTarget(getGoalTarget(_posAgent.getArgs().rightData));
    _roleAgent->setSelectedSkill(roleSkill::OneTouch);
}

void CPlayOff::assignMove(CRolePlayOff* _roleAgent,
                          const SPositioningAgent& _posAgent) {
    _roleAgent -> setAvoidPenaltyArea(true);
    _roleAgent -> setTime(_posAgent.getArgs().rightData); // ignore duration -> time is wait
    _roleAgent -> setAvoidBall(true);


    if (_posAgent.getArgs().staticPos == POBALLPOS) { // First Passer

        _roleAgent -> setTimeBased(true);
        _roleAgent -> setTarget(wm->ball->pos - Vector2D(0.30, 0));
        _roleAgent -> setTargetDir(wm->ball->pos - _roleAgent->getAgent()->pos());
        _roleAgent -> setSlow(true);
        _roleAgent -> setMaxVelocity(1);

    } else if (_roleAgent->getTime() > 10) {

        _roleAgent -> setTimeBased(true);
        _roleAgent -> setTarget(getMoveTarget(_posAgent.getArgs()));
        _roleAgent -> setTargetDir(_posAgent.getArgs().staticAng);
        _roleAgent -> setSlow(false);
        _roleAgent -> setMaxVelocity(getMaxVel(_roleAgent, _posAgent.getArgs()));

    } else {

        _roleAgent -> setTimeBased(false);
        _roleAgent -> setEventDist(_posAgent.getArgs().staticEscapeRadius);
        _roleAgent -> setTarget(getMoveTarget(_posAgent.getArgs()));
        _roleAgent -> setTargetDir(_posAgent.getArgs().staticAng);
        _roleAgent -> setSlow(false);
        _roleAgent -> setMaxVelocity(getMaxVel(_roleAgent, _posAgent.getArgs()));

    }

    _roleAgent -> setSelectedSkill(roleSkill::GotopointAvoid);
}

void CPlayOff::assignAfterLife(CRolePlayOff* _roleAgent,
                               const SPositioningAgent& _posAgent) {
    // TODO : Complete Ineteligence AfterLife Program
    qDebug() << "Gotcha";
    roleSkill::ESkill tSkill = chooseBestAfterLifeRoleSkill(_roleAgent,
                                                            _posAgent);
    switch (tSkill) {
    case roleSkill::Mark:
        _roleAgent->setAvoidPenaltyArea(true);
        _roleAgent->setAvoidBall(false);
        _roleAgent->setSlow(false);
        _roleAgent->setTargetDir(Vector2D(0, 1));
        _roleAgent->setTarget(Vector2D(0,0)); /*getMarkTarget(_posAgent.getArgs()*/
        _roleAgent->setSelectedSkill(roleSkill::Mark); //GPA
        break;
    case roleSkill::Defense:
        _roleAgent->setAvoidPenaltyArea(true);
        _roleAgent->setAvoidBall(false);
        _roleAgent->setTargetDir(Vector2D(0, 1));
        _roleAgent->setSlow(false);
        _roleAgent->setTarget(getDefenseTarget(_posAgent.getArgs()));
        _roleAgent->setSelectedSkill(roleSkill::Defense); //GPA
        break;
    case roleSkill::Support:
        _roleAgent->setAvoidPenaltyArea(true);
        _roleAgent->setAvoidBall(false);
        _roleAgent->setSlow(false);
        _roleAgent->setTargetDir(_roleAgent->getAgent()->pos() - wm->ball->pos);
        _roleAgent->setTarget(getSupportTarget(_posAgent.getArgs()));
        _roleAgent->setSelectedSkill(roleSkill::Support); //GPA
        break;
    default:
        _roleAgent->setAvoidPenaltyArea(true);
        _roleAgent->setAvoidBall(false);
        _roleAgent->setTargetDir(Vector2D(0, 1));
        _roleAgent->setSlow(false);
        _roleAgent->setTarget(getMarkTarget(_posAgent.getArgs()));
        _roleAgent->setSelectedSkill(roleSkill::Mark);
        break;

    }


}


Vector2D CPlayOff::getMoveTarget(int agentID,
                                 int agentState) {
    Vector2D tempTarget,finalTarget,position;
    double escapeRad;
    int oppCnt = 0;
    bool posFound;
    escapeRad = positionAgent[agentID].positionArg.at(agentState).staticEscapeRadius;
    position = positionAgent[agentID].positionArg.at(agentState).staticPos;
    finalTarget = position;
    for(double dist=0.0 ; dist<=escapeRad ; dist+=0.2 ) {
        for(double ang=-180.0 ; ang<=180.0 ; ang+=60.0 ) {
            tempTarget = position + Vector2D::polar2vector(dist,ang);
            for(int i = 0; i < wm->opp.activeAgentsCount();i++) {
                if(Circle2D(wm->opp.active(i)->pos,0.02).contains(tempTarget)) oppCnt++;
            }
            if(!oppCnt) {
                finalTarget = tempTarget;
                posFound = true;
                break;
            }
        }
        if(posFound) break;
    }

    return finalTarget;
}

Vector2D CPlayOff::getMoveTarget(const SPositioningArg& _posArg) {
    Vector2D tempTarget,finalTarget,position;
    double escapeRad;
    int oppCnt = 0;
    bool posFound;
    escapeRad = _posArg.staticEscapeRadius;
    position  = _posArg.staticPos;
    finalTarget = position;
    for(double dist=0.0 ; dist<=0.5 ; dist+=0.2 ) {

        for(double ang=-180.0 ; ang<=180.0 ; ang+=60.0 ) {

            tempTarget = position + Vector2D::polar2vector(dist,ang);

            ////should check
            if(wm->field->isInOppPenaltyArea(tempTarget + (wm->field->oppGoal() - tempTarget).norm() * 0.2))
                continue;
            for(int i = 0; i < wm->opp.activeAgentsCount();i++) {
                if(Circle2D(wm->opp.active(i)->pos,0.07).contains(tempTarget)) {
                    oppCnt = 1;
                    break;
                }

            }

            if(!oppCnt) {
                finalTarget = tempTarget;
                posFound = true;
                break;

            }
        }

        if(posFound) {
            break;

        }
    }

    return finalTarget;
}

double CPlayOff::getMaxVel(int agentID,
                           int agentState) {
    double tempVel,tDist;
    Vector2D tAgentPos;
    tAgentPos = knowledge->getAgent(kkAgentsID[agentID])->pos();
    tDist = tAgentPos.dist(positionAgent[agentID].positionArg.at(positionAgent[agentID].stateNumber).staticPos);
    tempVel = tDist/positionAgent[agentID].positionArg.at(positionAgent[agentID].stateNumber).leftData;
    if(tempVel >= 4) tempVel = 4;
    else if(tempVel <= 1.5) tempVel = 1.5;
    return tempVel;
}


double CPlayOff::getMaxVel(const CRolePlayOff* _roleAgent,
                           const SPositioningArg& _posArg) {
    double tempVel,tDist;
    Vector2D tAgentPos;
    tAgentPos = _roleAgent->getAgent()->pos();
    tDist = tAgentPos.dist(_posArg.staticPos);
    tempVel = tDist/_posArg.leftData;

    if(tempVel >= 4)
        tempVel = 4;
    else if(tempVel <= 1.5)
        tempVel = 1.5;

    return tempVel;
}


Vector2D CPlayOff::getGoalTarget(int shoterID,int shoterState) {
    Vector2D tempPos;
    double tempYPos = (double)(positionAgent[shoterID].positionArg.at(shoterState).rightData)/1000 + wm->field->oppGoalR().y;
    draw(QString("%1").arg(tempYPos), Vector2D(-2,-2));
    if(tempYPos < wm->field->oppGoalR().y) tempYPos = wm->field->oppGoalR().y;
    if(tempYPos > wm->field->oppGoalL().y) tempYPos = wm->field->oppGoalL().y;
    tempPos.assign(wm->field->oppGoal().x,tempYPos);
    return tempPos;
}

Vector2D CPlayOff::getGoalTarget(const long& _y) {
    Vector2D tempPos;
    double tempYPos = (double)(_y)/1000 + wm->field->oppGoalR().y;
    draw(QString("%1").arg(tempYPos), Vector2D(-2,-2));
    if(tempYPos < wm->field->oppGoalR().y) tempYPos = wm->field->oppGoalR().y;
    if(tempYPos > wm->field->oppGoalL().y) tempYPos = wm->field->oppGoalL().y;
    tempPos.assign(wm->field->oppGoal().x,tempYPos);
    return tempPos;
}

bool CPlayOff::kkCheckIntersectWithAgents(Segment2D tSeg)
{
    Vector2D sol1, sol2;
    for(int i = 0; i < wm->our.activeAgentsCount(); i++)
        if(Circle2D(wm->our.active(i)->pos, 0.10).intersection(tSeg, &sol1, &sol2) > 0)
            return true;

    for(int i = 0; i < wm->opp.activeAgentsCount(); i++)
        if(Circle2D(wm->opp.active(i)->pos, 0.10).intersection(tSeg, &sol1, &sol2) > 0)
            return true;

    return false;
}

bool CPlayOff::chipOrNot(int passerID, int ReceiverID, int ReceiverState){

    if(positionAgent[passerID].positionArg.at(positionAgent[passerID].stateNumber).leftData < 0) return true;
    else if(positionAgent[passerID].positionArg.at(positionAgent[passerID].stateNumber).rightData < 0) return false;
    else return !isPathClear(wm->ball->pos,
                             positionAgent[ReceiverID].positionArg.at(ReceiverState).staticPos,
                             0.5,
                             0.1);
}

bool CPlayOff::chipOrNot(const SPositioningArg& _posArg) {
    if (_posArg.leftData < 0) {
        return true;
    } else if(_posArg.rightData < 0) {
        return false;
    } else {
        const int& id = _posArg.PassToId;
        const int& ps = _posArg.PassToState;
        return !isPathClear(wm->ball->pos,
                            positionAgent[id].getAbsArgs(ps).staticPos,
                            0.5,   // Radius
                            0.1);  // Tereshold
    }
}

bool CPlayOff::isPathClear(Vector2D _pos1,
                           Vector2D _pos2,
                           double _radius,
                           double treshold) {

    Vector2D sol1, sol2, sol3;
    Line2D _path(_pos1, _pos2);
    Polygon2D _poly;
    Circle2D(_pos2,_radius + treshold).intersection(_path.perpendicular(_pos2),
                                                    &sol1,
                                                    &sol2);

    _poly.addVertex(sol1);
    sol3 = sol1;
    _poly.addVertex(sol2);
    Circle2D(_pos1,CRobot::robot_radius_new + treshold).intersection(_path.perpendicular(_pos1),
                                                                     &sol1,
                                                                     &sol2);

    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    draw(_poly,"red");

    for(int i = 0;i < wm->opp.activeAgentsCount();i++) {
        if(_poly.contains(wm->opp.active(i)->pos)) return false;
    }
    return true;
}

void CPlayOff::setAgentSize(int _agentSize){
    agentSize = _agentSize;
}

bool CPlayOff::hasPassInSkills(int _agent, int _index)
{
    bool check = false;
    for(int k = 0;k < currentPlan->AgentPlan[_agent].at(_index).skill.size(); k++)
    {
        if(currentPlan->AgentPlan[_agent].at(_index).skill[k].name == PassSkill)
        {
            check = true;
            break;
        }
    }
}

void CPlayOff::assignTasks()
{
    SPositioningArg tempPosArg;
    for(int i = 0; i < currentPlan->agentSize;i++) {
        positionAgent[i].positionArg.clear();
        for(int j = 0;j < currentPlan->AgentPlan[i].size();j++) {
            tempPosArg.staticPos          = currentPlan->AgentPlan[i].at(j).pos;
            tempPosArg.staticAng          = Vector2D::polar2vector(1, currentPlan->AgentPlan[i].at(j).angle);
            tempPosArg.staticEscapeRadius = currentPlan->AgentPlan[i].at(j).tolerance;
            //            tempPosArg.PassToId           = currentPlan->AgentPlan[i].at(j).targetAgent;
            //            tempPosArg.PassToState        = currentPlan->AgentPlan[i].at(j).targetIndex;

            if(hasPassInSkills(i, j)) tempPosArg.staticPos = POBALLPOS;
            else tempPosArg.staticPos = currentPlan->AgentPlan[i].at(j).pos;

            //            tempPosArg.staticAng = Vector2D::polar2vector(1, currentPlan->AgentPlan[i].at(j).angle);
            tempPosArg.staticAng.assign(tempPosArg.staticAng.x, -tempPosArg.staticAng.y);
            //            tempPosArg.staticEscapeRadius = currentPlan->AgentPlan[i].at(j).tolerance;

            for(int k = 0;k < currentPlan->AgentPlan[i].at(j).skill.size();k++) {
                tempPosArg.staticSkill = currentPlan->AgentPlan[i].at(j).skill[k].name;
                tempPosArg.rightData  = currentPlan->AgentPlan[i].at(j).skill[k].data[1];
                tempPosArg.leftData  = currentPlan->AgentPlan[i].at(j).skill[k].data[0];
                positionAgent[i].positionArg.append(tempPosArg);
            }
        }
    }
}

void CPlayOff::newAssignTasks() {
    int& sym = masterPlan->execution.symmetry;
    for(size_t i = 0;i < masterPlan->common.currentSize; i++) {
        positionAgent[i].positionArg.clear();
        Q_FOREACH(playOffRobot agentPlan, masterPlan->execution.AgentPlan[i]) {
            SPositioningArg tempPosArg;
            tempPosArg.staticPos          = agentPlan.pos;
            tempPosArg.staticAng          = Vector2D::polar2vector(1, agentPlan.angle);
            tempPosArg.staticAng.assign(tempPosArg.staticAng.x, -1*sym*tempPosArg.staticAng.y);
            tempPosArg.staticPos.assign(tempPosArg.staticPos.x, sym*tempPosArg.staticPos.y);
            tempPosArg.staticEscapeRadius = agentPlan.tolerance;

            Q_FOREACH(playOffSkill skill, agentPlan.skill) {
                tempPosArg.leftData           = skill.data[0];
                tempPosArg.rightData          = skill.data[1];
                tempPosArg.staticSkill        = skill.name;
                tempPosArg.PassToId           = skill.targetAgent;
                tempPosArg.PassToState        = skill.targetIndex;

                if (skill.name == PassSkill) {
                    positionAgent[i].positionArg.back().staticPos = POBALLPOS;
                } else if (skill.name == ShotToGoalSkill
                           || skill.name == ChipToGoalSkill
                           || skill.name == OneTouchSkill) {

                    if (sym < 0) {
                        tempPosArg.rightData = 1000 - tempPosArg.rightData;
                    }
                }
                positionAgent[i].positionArg.append(tempPosArg);
            }
        }
    }
}

void CPlayOff::connectPasserAndReciever() {
    int firstPasser = findFirstPasser();
    if (firstPasser != -1) {
        SBallOwner temp;
        temp.id = firstPasser;
        temp.state = 0;
        ownerList.append(temp);
        findReciver(positionAgent[firstPasser].getArgs(1).PassToId, 1); // it should be passer state but it's a little bit bugy in Visual-planner TODO <---
    }
}

int CPlayOff::findReciver(int _passer, int _state) {
    if (_state == 0) {
        return 0;
    }

    for (size_t i = _state; i < positionAgent[_passer].positionArg.size(); i++) {
        if (positionAgent[_passer].getArgs(i).staticSkill == PassSkill) {
            SBallOwner temp;
            temp.id = _passer;
            temp.state = i;
            ownerList.append(temp);
            findReciver(positionAgent[_passer].getArgs(i).PassToId,
                        positionAgent[_passer].getArgs(i).PassToState);
        }
    }
}

void CPlayOff::findPasserIndex() {
    firstPasserID = -1;
    for(int i = 0;i < currentPlan->agentSize;i++) {
        if(currentPlan->AgentPlan[i].size())
        {
            if(currentPlan->initPos.ball != Vector2D(100,100))
            {
                if(currentPlan->initPos.Agent[i] == Vector2D(100,100))
                {
                    firstPasserID = i;
                }
            }
        }
    }
}


void CPlayOff::assinID() {
    double dist2Point[6];
    double minDist = 100000;
    QList<int> matchedIDList;
    for(size_t i = 0; i < 6;i++) {
        dist2Point[i] =  10000;
    }

    findPasserIndex();

    matchedIDList.clear();

    if(firstPasserID >= 0) {
        for(size_t i = 0;i < activeAgents.size(); i++) {
            dist2Point[i] = activeAgents.at(i)->pos().dist(wm->ball->pos);
            if(dist2Point[i] < minDist) {
                minDist = dist2Point[i];
                kkAgentsID[firstPasserID] = activeAgents.at(i)->id();
            }
        }
        matchedIDList.append(kkAgentsID[firstPasserID]);
    }


    for(size_t i = 0; i < currentPlan->agentSize;i++) {

        if(i != firstPasserID) {

            for(size_t i = 0; i < 6;i++)
                dist2Point[i] =  10000;
            minDist = 100000;

            for(size_t j = 0; j < activeAgents.size(); j++) {
                if(!matchedIDList.contains(activeAgents.at(j)->id())) {
                    dist2Point[i] = currentPlan->initPos.Agent[i].
                            dist(activeAgents.at(j)->pos());
                    if(dist2Point[i] < minDist) {
                        minDist = dist2Point[i];
                        kkAgentsID[i] = activeAgents.at(j)->id();
                    }
                }
            }
            matchedIDList.append(kkAgentsID[i]);
        }
    }

}

///////////////////Dynamic

SPlayOffPlan* CPlayOff::DynamicPlay() {

}

//////////////////

void CPlayOff::getCostRec(double costArr[][6], int arrSize, QList<kkValue> &valueList, kkValue value, int size, int aId)
{
    if(size <= 1) {
        int tIndex = kkGetIndex(value, 0);
        value.IDs[tIndex] = aId;
        value.value += costArr[tIndex][aId];
        valueList.append(value);
        return;
    }
    for(int i = 0; i < size; i++) {
        kkValue tValue = value;
        int tIndex = kkGetIndex(tValue, i);
        tValue.IDs[tIndex] = aId;
        tValue.value += costArr[tIndex][aId];
        getCostRec(costArr, arrSize, valueList, tValue, size - 1, aId + 1);
    }
}

int CPlayOff::kkGetIndex(kkValue &value, int cIndex)
{
    int cnt = 0;
    for(int i = 0; i < value.agentSize; i++)
    {
        if(value.IDs[i] == -1)
        {
            if(cnt == cIndex)
                return i;
            cnt++;
        }
    }
}

int CPlayOff::insertActiveAgentsToList()
{
    int activeAgentsCount = activeAgents.length();
    agentList.clear();
    kkRobot tempRobot;
    for(int i = 0; i < activeAgents.length(); i++)
    {
        tempRobot.pos = activeAgents.at(i)->pos();
        tempRobot.vel = activeAgents.at(i)->vel();
        tempRobot.dir = activeAgents.at(i)->dir();
        tempRobot.id  = activeAgents.at(i)->id();
        agentList.append(tempRobot);
    }
    for(int i = activeAgentsCount; i < 6; i++)
    {
        tempRobot.pos = Vector2D(0,0);
        tempRobot.vel = Vector2D(0,0);
        tempRobot.dir = Vector2D(0,0);
        tempRobot.id = -1;
        agentList.append(tempRobot);
    }
    return activeAgentsCount;

}

POMODE CPlayOff::getPlayOffMode() {
    if(knowledge->getGameMode() == knowledge->OurKickOff) return KICKOFF;
    else if(knowledge->getGameState() == knowledge->OurIndirectKick || knowledge->getGameMode() == knowledge->OurIndirectKick) return INDIRECT;
    else if(knowledge->getGameState() == knowledge->OurDirectKick || knowledge->getGameMode() == knowledge->OurDirectKick) return DIRECT;
}

void CPlayOff::initilizePositions(QList<SPositioningArg> _posArg[]){
    for(int i = 0;i < agentSize;i++) {
        positionAgent[i].positionArg.clear();
        for(int j = 0;j < _posArg[i].size();j++) {
            positionAgent[i].positionArg.append(_posArg[i].at(j));
        }
    }
}

void CPlayOff::resetP(){
    executedCycles = 0;
    activeAgents.clear();
    markAgents.clear();
    debug(QString("reset Plan"),D_MAHI);
    for(int i = 0;i < 6;i++) {
        positionAgent[i].stateNumber = 0;
        //        positionAgent[i].mahiLastTime = knowledge->getCurrentKKTime(); //removed!
        isFirstTime[i] = 0;
        currentPlan = NULL;
        //        currentPlan->AgentPlan[i].clear();
    }
    isBallIn = false;
    doPass   = false;
    kickOffFirstTimeFlag = true;
    setTimer = true;
}

void CPlayOff::reset(){


    qDebug() << "Bring yourself back online playoff";

    radLimit = 2;
    decidePlan = true;
    firstTime = true;
    agentSize = 1;
    ballEnteredKickerFlag = false;
    ballEnteredKickerChipFlag = false;
    passReceivedFlag = false;
    isPassDoneflag = false;
    for(int i = 0;i < 6;i++) {
        isFirstTime[i] = true;
        positionAgent[i].stateNumber = 0;
        roleAgent[i]->reset();
        newRoleAgent[i]->reset();
        isBallNearRobot[i] = false;
        isBallNearRobotF[i] = false;
        positionAgent[i].zombie = false;
    }
    isBallIn = false;
    cnt = 0;
    tempAgent = new CRolePlayOff();
    doPass = false;
    setTimer = true;
    kickOffFirstTimeFlag = true;
    ////////////

    currentPlan = new SPlayOffPlan();
    masterPlan = NULL;

    initial    = true;
    playOnFlag = false;
    havePassInPlan = false;

    //Dynamic
    ready = pass = shot = false;
    dynamicStartTime = -1;

    executedCycles = 0;
    activeAgents.clear();
    markAgents.clear();
    debug(QString("reset Plan"),D_MAHI);
}

void CPlayOff::init(QList<int> _agents , QMap<QString , EditData*> *_editData){
    setAgentsID(_agents);
    setEditData(_editData);
    initMaster();

    //    if( knowledge->getLastPlayExecuted() != OurKickOffPlay ){
    //        reset();
    //    }
    //    knowledge->setLastPlayExecuted(OurKickOffPlay);
}

void CPlayOff::execute_0(){
    globalExecute();
}

void CPlayOff::execute_1(){
    globalExecute();
}

void CPlayOff::execute_2(){
    globalExecute();
}

void CPlayOff::execute_3(){
    globalExecute();
}

void CPlayOff::execute_4(){
    globalExecute();
}

void CPlayOff::execute_5(){
    globalExecute();
}

void CPlayOff::execute_6(){
    globalExecute();
}

void CPlayOff::debugDirs() {
    qDebug() << "Hey";
    for (size_t i = 0;i < dirList.size();i++) {
        qDebug() << dirList.at(i);
    }
}


///////////////////

void CPlayOff::clear() {
    planListKickOff.clear();
    planListDirect.clear();
    planListIndirect.clear();
    fullPlans.clear();
}

void CPlayOff::fullClear() {
    for (size_t i = 0; i < fullPlans.size();i++) {
        for (size_t j = 0;j < fullPlans[i].size();j++) {
            delete fullPlans[i][j];
        }
    }

    planListKickOff.clear();
    planListDirect.clear();
    planListIndirect.clear();

    for (size_t i = 0;i < fullPlans.size();i++) {
        fullPlans[i].clear();
    }
    fullPlans.clear();
}


//////////////////////////////

QString CPlayOff::getModeStr(POMODE _mode) {

    if (_mode == KICKOFF) {
        return "KOf";
    }
    else if (_mode == DIRECT) {
        return "Dir";
    }
    else if (_mode == INDIRECT) {
        return "InD";
    }
}

////////////////////////////////

void CPlayOff::setMasterPlan(SPlan *_thePlan) {
    masterPlan = _thePlan;
}

void CPlayOff::setMasterMode(EMode _mode) {
    masterMode = _mode;
}

EMode CPlayOff::getMasterMode() {
    return masterMode;
}
///////////////////////////////////////
/////////////Check Execution///////////

/*!
*   \brief check if ball get distance from robot,
*
*          in case that it's direct shoot
*          also check that ball is in right direction or not.
*
*
*/
bool CPlayOff::isKickDone(CRolePlayOff * _roleAgent) {

    if (Circle2D(_roleAgent->getAgent()->pos(), 0.4).contains(wm->ball->pos)) {
        _roleAgent->setBallIsNear(true);
    } else if ( !Circle2D(_roleAgent->getAgent()->pos(), 0.6).contains(wm->ball->pos)
                && _roleAgent->getBallIsNear() ) {
        _roleAgent->setBallIsNear(false);
        if (_roleAgent->getChip()) {
            debug("[playoff] chip Done", D_MAHI);
            return true;
        } else {
            /** Ball gonna touch the target point **/

            // check ball speed
            if (wm->ball->vel.length() / (_roleAgent->getAgent()->pos().dist(_roleAgent->getTarget())) > 1) {
                debug("[playoff] speed is enough", D_MAHI);
                // check ball direction
                Vector2D sol1,sol2;
                if (Circle2D(_roleAgent->getTarget(), 0.5).intersection(Ray2D(wm->ball->pos, wm->ball->pos + wm->ball->vel), &sol1, &sol2)) {
                    debug("[playoff] direction is correct", D_MAHI);
                    debug("[playoff] kick is Done", D_MAHI);
                    return true;
                }
            }
        }
    }
    return false;
}

bool CPlayOff::isReceiveDone(const CRolePlayOff * _roleAgent) {
    if(Circle2D(_roleAgent->getAgent()->pos(), 0.3).contains(wm->ball->pos)) {
        if (wm->ball->vel.length() < 0.3)
            return true;
    }
    return false;
}

bool CPlayOff::isOneTouchDone(CRolePlayOff * _roleAgent) {
    if (isKickDone(_roleAgent)) {
        debug("[playoff] OneTouch is Done", D_MAHI);
        return true;
    } else {
        return false;
    }
}

bool CPlayOff::isMoveDone(const CRolePlayOff * _roleAgent) {

    if (_roleAgent->getFirstMove() && _roleAgent->getTarget() != POBALLPOS) {
        return true ;
    }

    if (_roleAgent->getTimeBased()) {
        debug(QString("EL : %1").arg(_roleAgent->getElapsed()), D_HOSSEIN);
        debug(QString("GT : %1").arg(_roleAgent->getTime()), D_HOSSEIN);
        if (_roleAgent->getElapsed() > _roleAgent->getTime()) {
            debug("D------------------", D_HOSSEIN);
            return true;
        }
    } else {
        // TODO : vartypes this
        if (_roleAgent->getAgent()->pos().dist(_roleAgent->getTarget()) < max(0.3,_roleAgent->getEventDist()/100)) {
            return true;
        }
    }
    return false;
}


///////////OverLoading Operators
QDebug operator<< (QDebug d, NGameOff::SPlan _plan) {

    QString mode;
    if (_plan.common.planMode == KICKOFF)
        mode = "KickOff";
    else if (_plan.common.planMode == DIRECT)
        mode = "Direct";
    else if (_plan.common.planMode == INDIRECT)
        mode = "InDirect";


    d << "------------------->>>";
    d << "<Common>" ;
    d << "Agent Size" << _plan.common.agentSize;
    d << "Chance" << _plan.common.chance;
    d << "Last Dist" << _plan.common.lastDist;
    d << "Plan Mode" << mode;
    d << "Succes Rate" << _plan.common.succesRate;
    d << "Tags" << _plan.common.tags;
    d << "</Common>";
    d << "<<<-------------------";
    return d;
}

void CPlayOff::setInitial(bool _init) {
    initial = _init;
}

int CPlayOff::findFirstPasser() {
    int first = -1;
    if (masterPlan->matching.initPos.ball.x != -100) {
        for (size_t i = 0; i < masterPlan->matching.initPos.agents.size(); i++) {
            if (masterPlan->matching.initPos.agents.at(i).x == -100) {
                first = i;
                break;
            }
        }
    }

    // TODO : Check that pass wasn't in first positions
    debug(QString("FFF : %1").arg(first), D_HOSSEIN);
    return first;
}

QPair<int, int> CPlayOff::findTheLastShoot(const SExecution &_plan) {
    QPair<int, int> last;
    last.first = last.second = -1;

    QList<POffSkills> finalSkills;
    finalSkills.append(ShotToGoalSkill);
    finalSkills.append(ChipToGoalSkill);
    finalSkills.append(OneTouchSkill);

    int counter = 0;
    qDebug() << "SSS" << _plan.AgentPlan.size();
    Q_FOREACH (QList<playOffRobot> agent, _plan.AgentPlan) {
        int counter2 = 0;
        Q_FOREACH(playOffRobot node, agent) {
            Q_FOREACH(playOffSkill skill, node.skill) {
                if (finalSkills.contains(skill.name)) {
                    last.first  = counter;
                    last.second = counter2;
                    qDebug() << "MAHI " << skill.name;
                }
                counter2++;
            }
        }
        counter++;
    }

    return last;
}

void CPlayOff::analyseShoot()
{
    if (masterPlan != NULL) {
        QPair<int, int> last;
        last = findTheLastShoot(masterPlan->execution);
        masterPlan->execution.theLastAgent = last.first;
        masterPlan->execution.theLastState = last.second;
        qDebug() << "Last" << last.first << last.second;
        havePassInPlan = (last.first != -1  && last.second != -1);
    }
}

void CPlayOff::analysePass() {
    // TODO : need edit for mulitiple pass
    if (masterPlan != NULL) {
        // first : passer second : reciver
        QPair<AgentPoint, AgentPoint> tPass;
        findThePasserandReciver(masterPlan->execution, tPass);
        masterPlan->execution.passer .id     = tPass.first.id;
        masterPlan->execution.passer .state  = tPass.first.state;
        masterPlan->execution.reciver.id     = tPass.second.id;
        masterPlan->execution.reciver.state  = tPass.second.state;

    }

    qDebug() << "PI : " << masterPlan->execution.passer .id;
    qDebug() << "PS : " << masterPlan->execution.passer .state;
    qDebug() << "RI : " << masterPlan->execution.reciver.id;
    qDebug() << "RS : " << masterPlan->execution.reciver.state;
}

void CPlayOff::findThePasserandReciver(const NGameOff::SExecution & _plan,
                                       AgentPair& _pair) {

    int counter = 0;
    Q_FOREACH(QList<playOffRobot> agent, _plan.AgentPlan) {
        int counter2 = 0;
        Q_FOREACH(playOffRobot node, agent) {
            Q_FOREACH(playOffSkill skill, node.skill) {
                if (skill.name == PassSkill) {
                    _pair.first.id    = counter;
                    _pair.first.state = counter2;
                    break;
                }
            }

            if (_pair.first.id != -1) {
                break;
            }

            counter2++;
        }

        if (_pair.first.id != -1) {
            break;
        }

        counter++;
    }

    int &id = _pair.first.id;
    int &st = _pair.first.state;

    if (id >= 0 && id < _plan.AgentPlan.size()) {
        if (st >= 0 && st < _plan.AgentPlan.at(id).size()) {

            int si = (_plan.AgentPlan[id][st].skill[1].name) ? 1 : 2;


            _pair.second.id    = _plan.AgentPlan[_pair.first.id]
                    [_pair.first.state].
                    skill[si].targetAgent;

            _pair.second.state = _plan.AgentPlan[_pair.first.id]
                    [_pair.first.state].
                    skill[si].targetIndex;

        }
    }


}

roleSkill::ESkill
CPlayOff::chooseBestAfterLifeRoleSkill(CRolePlayOff *,
                                       const SPositioningAgent &_posAgent) {

    // TODO : complete the usage then remove this return
    return roleSkill::Mark;


    QList<POffSkills> mark;
    QList<POffSkills> support;
    QList<POffSkills> defense;

    mark   .append(MoveSkill);
    support.append(PassSkill);
    defense.append(NoSkill);

    roleSkill::ESkill tempRole  = roleSkill::Mark;
    POffSkills        tempSkill = _posAgent.getArgs().staticSkill;


    if (mark.contains(tempSkill)) {
        tempRole = roleSkill::Mark;

    } else if (support.contains(tempSkill)) {
        tempRole = roleSkill::Support;

    } else if (defense.contains(tempSkill)) {
        tempRole = roleSkill::Defense;

    } else {
        tempRole = roleSkill::Mark;

    }

}

Vector2D CPlayOff::getMarkTarget(const SPositioningArg &_posArg) {
    // TODO : find best point
    return _posArg.staticPos - Vector2D(1, 0);
}

Vector2D CPlayOff::getDefenseTarget(const SPositioningArg &_posArg) {
    // TODO : find best point
    return _posArg.staticPos - Vector2D(1, 0);

}

Vector2D CPlayOff::getSupportTarget(const SPositioningArg &_posArg) {
    // TODO : find best point
    return _posArg.staticPos - Vector2D(1, 0);

}
