#include "plays/playoff.h"


CPlayOff::CPlayOff()
{
    radLimit = 2;
    decidePlan = true;
    firstTime = true;
    agentSize = 1;
    for(int i = 0;i < agentSize;i++) positionAgent[i].stateNumber = 0;
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
    kickOffPos[0] = Vector2D(wm->ball->pos.x - 0.3,wm->ball->pos.y);
    kickOffPos[1] = Vector2D(-0.3,2);
    kickOffPos[2] = Vector2D(-0.3,-2);
    kickOffPos[3] = Vector2D(-3.3,0);

}

CPlayOff::~CPlayOff()
{
    for(int i = 0;i < 6;i++) {
        delete roleAgent[i];
        delete newRoleAgent[i];

    }
    delete tempAgent;

    for (size_t i = 0; i < fullPlans.size();i++) {
        for (size_t j = 0;j < fullPlans[i].size();j++) {
            delete fullPlans[i][j];
        }
    }

}

void CPlayOff::setPlanDir(QString directory)
{
    directory = QString::fromStdString(policy()->KKPlayOff_KKPOPlanSQL());
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

int CPlayOff::loadPlan()
{
    //kkPOPlanSQL = QSqlDatabase::database("playoff");
    QSqlQuery squery = QSqlQuery(kkPOPlanSQL);
    squery.exec("SELECT * FROM poplanlist ORDER BY id ASC");
    planListKickOff.clear();
    planListDirect.clear();
    planListIndirect.clear();

    while(squery.next())
    {
        SPlayOffPlan *tempPlan = new SPlayOffPlan();;
        tempPlan->planMode = POMODE(squery.value(2).toInt());
        tempPlan->agentSize = squery.value(3).toInt();
        loadEachPlan(tempPlan, squery.value(1).toString(), 1);

        if(squery.value(4).toString() != "na")
        {
            tempPlan->initPos.ball = convertPos(squery.value(4).toString().split("|").at(0).toInt(),
                                                squery.value(4).toString().split("|").at(1).toInt(), 1);
        }
        else
        {
            tempPlan->initPos.ball = Vector2D(100, 100);
        }

        for(int i = 0; i < 6; i++)
        {
            if(squery.value(5 + i).toString() != "na")
            {
                tempPlan->initPos.Agent[i] = convertPos(squery.value(5 + i).toString().split("|").at(0).toInt(),
                                                        squery.value(5 + i).toString().split("|").at(1).toInt(), 1);
            }
            else
            {
                tempPlan->initPos.Agent[i] = Vector2D(100, 100);
            }
        }
        if(tempPlan->planMode == KICKOFF)
            planListKickOff.append(tempPlan);
        else if(tempPlan->planMode == DIRECT)
            planListDirect.append(tempPlan);
        else if(tempPlan->planMode == INDIRECT) {
            planListIndirect.append(tempPlan);
            planListDirect.append(tempPlan);
        }
        //        debug(QString("%1").arg(int(tempPlan->planMode)), D_KK);

        //////////////////////////////////
        //////////////SYMMETRY////////////
        //////////////////////////////////
        SPlayOffPlan *symmetryPlan = new SPlayOffPlan();;

        if(policy()->KKPlayOff_KKPOSymmetry()) {

            symmetryPlan->planMode = POMODE(squery.value(2).toInt());
            symmetryPlan->agentSize = squery.value(3).toInt();
            loadEachPlan(symmetryPlan, squery.value(1).toString(), -1);

            if(squery.value(4).toString() != "na")
            {
                symmetryPlan->initPos.ball = convertPos(squery.value(4).toString().split("|").at(0).toInt(),
                                                        squery.value(4).toString().split("|").at(1).toInt(), -1);
            }
            else
            {
                symmetryPlan->initPos.ball = Vector2D(100, 100);
            }

            for(int i = 0; i < 6; i++)
            {
                if(squery.value(5 + i).toString() != "na")
                {
                    symmetryPlan->initPos.Agent[i] = convertPos(squery.value(5 + i).toString().split("|").at(0).toInt(),
                                                                squery.value(5 + i).toString().split("|").at(1).toInt(), -1);
                }
                else
                {
                    symmetryPlan->initPos.Agent[i] = Vector2D(100, 100);
                }
            }
            if(symmetryPlan->planMode == KICKOFF)
                planListKickOff.append(symmetryPlan);
            else if(symmetryPlan->planMode == DIRECT)
                planListDirect.append(symmetryPlan);
            else if(symmetryPlan->planMode == INDIRECT) {
                planListDirect.append(symmetryPlan);
                planListIndirect.append(symmetryPlan);
            }

        }
        //////////////////////////////////
    }
    qDebug()<<"PlayOff SQL Loaded!";
    return planListKickOff.count() + planListDirect.count() + planListIndirect.count();

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
    //QList<POOwnerReceive> tempPaRList;
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
        tempOaR.receiveAgent = tCurrentPlan->AgentPlan[tempOaR.ballOwnerAgent].at(tempOaR.ballOwnerIndex).targetAgent;
        tempOaR.receiveIndex = tCurrentPlan->AgentPlan[tempOaR.ballOwnerAgent].at(tempOaR.ballOwnerIndex).targetIndex;
        tempOaR.skill = temp.at(i).skill;
        tList.append(tempOaR);
    }
}

void CPlayOff::globalExecute(int agentCnt) {

    if(masterPlan != NULL) {
        qDebug() << *masterPlan;
    } else {
        qDebug() << "master is null";
        return;
    }

    //    playOnFlag = false;
    //    if(!firstTime) {
    //        if(lastAgentCount != agentCnt) {
    //            playOnFlag = true;
    //            decidePlan = true;
    //            kickOffFirstTimeFlag = true;
    //        }
    //    }
    //    else {
    //        decidePlan = true;
    //        firstTime = false;
    //        lastBallPos = wm->ball->pos;
    //    }
    //    lastAgentCount = agentCnt;
    mainPlanner(agentCnt);
}

bool CPlayOff::isBallMoved() {
    if(wm->ball->pos.dist(lastBallPos) > 0.25) return true;
    else return false;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
void CPlayOff::mainPlanner(int _agentSize) {


    if(!wm->ball->pos.isValid()) return;
    if(!wm->field->fieldRect().contains(wm->ball->pos)) return;

    setAgentSize(_agentSize);

    if (knowledge->getGameState() == CKnowledge::OurKickOff &&
            knowledge->getGameMode() == CKnowledge::Stop) {
        //        resetP();
        appendRemainingsAgents(activeAgents);
        kickOffStopModePlay(_agentSize);

        if (kickOffFirstTimeFlag) {
            matchKickOffID(_agentSize);
            kickOffFirstTimeFlag = false;
        }

        lastDecideTime = knowledge->getCurrentKKTime();
        kickOffExecute();
        return;

    }
    else {

        if (decidePlan) {
            resetP();
            debug("WHHY", D_MAHI);
            setAgentSize(_agentSize);
            lastDecideTime = knowledge->getCurrentKKTime();
            appendRemainingsAgents(activeAgents);
            debug(QString("agent Size : %1").arg(activeAgents.size()),D_MAHI);
            // Match a Plan if exist a plan for match in fill currentPlan
            if(getMatchedPlan(getPlayOffMode(), _agentSize, true)) {
                getPassTimeline(currentPlan, ownerReceiveList);
                knowledge->setNecessaryDefKick(false);
            }
            // If there's no plan for match run a dynamic plan
            else if (agentSize) {

                currentPlan = DynamicPlay();

                /*

                currentPlan = new SPlayOffPlan();
                currentPlan->agentSize = (_agentSize > 1) ? 2 : 1;
                currentPlan->initPos.ball = wm->ball->pos;
                for (size_t i = 0;i < activeAgents.size();i++) {
                    currentPlan->initPos.Agent[i] = activeAgents.at(i)->pos();
                    currentPlan->AgentPlan[i].clear();
                }

                tRobot.pos = POBALLPOS;
                tRobot.angle = 0;
                tRobot.skill[0] = MoveSkill;
                tRobot.skill[1] = PassSkill;
                tRobot.skill[2] = NoSkill;
                tRobot.targetAgent = -1;
                tRobot.targetIndex = -1;
                tRobot.tolerance = 0;

                if(knowledge->getGameMode() == CKnowledge::OurKickOff) {
                    if(wm->getIsSimulMode())
                        tRobot.skillData[1][0] = 3;
                    else
                        tRobot.skillData[1][0] = 300;
                    tRobot.skillData[1][1] = 500;
                }
                else {
                    if(wm->getIsSimulMode())
                        tRobot.skillData[1][0] = 6;
                    else
                        tRobot.skillData[1][0] = 1023;
                    tRobot.skillData[1][1] = 500;
                }
                tRobot.skillData[0][0] = 200;
                tRobot.skillData[0][1] = 200;
                tRobot.skillSize = 2;

                currentPlan->AgentPlan[0].append(tRobot);

                for(int i = 1;i < activeAgents.size();i++) {
                    markAgents.append(activeAgents.at(i));
                }
                knowledge->setNecessaryDefKick(false);

                //                getPassTimeline(currentPlan, ownerReceiveList);
                //                for(int i = 1;i<activeAgents.size() - 1;i++) {
                //                    markAgents.append(activeAgents.at(i));
                //                }

                */
            }
            // If we don't have any robot
            else {
                knowledge->setNecessaryDefKick(true);
            }

            insertActiveAgentsToList();
            assinID();
            assignTasks();

            //            initilizePositions(positionArg);
            decidePlan = false;
            fillRolesProperties( );
        }
        else {

            posExecute();
            checkEndState();
            if(agentSize > 1)
                passManager();

            fillRolesProperties();

            if(isPlanEnd()) {
                setTimer = true;
                playOnFlag = true;
                debug(QString("END."),D_MAHI);
                draw(QString("ENDIO"),Vector2D(-2,2));
                kickOffFirstTimeFlag = true;

                //                for(int i = 0;i < 6;i++) {
                //                    delete roleAgent[i];
                //                    delete newRoleAgent[i];

                //                }
                //                delete tempAgent;


                //                directory = QString::fromStdString(policy()->KKPlayOff_KKPOPlanSQL());
                //                loadSQL();
                //                loadPlan();

                decidePlan = true;
                firstTime = true;
                agentSize = 1;
                for(int i = 0;i < agentSize;i++) positionAgent[i].stateNumber = 0;
                ballEnteredKickerFlag = false;
                ballEnteredKickerChipFlag = false;
                passReceivedFlag = false;
                isPassDoneflag = false;
                for(int i = 0;i < 6;i++) {
                    isFirstTime[i] = true;
                    //                    roleAgent[i] = new CRolePlayOff();
                    //                    newRoleAgent[i] = new CRolePlayOff();
                    isBallNearRobot[i] = false;
                    isBallNearRobotF[i] = false;

                }
                isBallIn = false;
                cnt = 0;
                //                tempAgent = new CRolePlayOff();
                bugflag = false;
                doPass  = false;

            }
        }

        draw(roleAgent[0]->getTarget());
        draw(currentPlan->initPos.ball,0,QColor(Qt::blue));
        debug(QString("CHIP : %1").arg(roleAgent[0]->getChip()),D_MAHI);
        debug(QString("CHIP : %1").arg(roleAgent[0]->getKickSpeed()),D_MAHI);
        debug(QString("Time0 : %1").arg(knowledge->getCurrentKKTime() - positionAgent[0].mahiLastTime),D_MAHI);
        debug(QString("Time1 : %1").arg(knowledge->getCurrentKKTime() - positionAgent[1].mahiLastTime),D_MAHI);
        debug(QString("ballOwnerState : %1").arg(activeAgents.size()),D_KK);

        mahiDebug(5);
        mahiCircle(2);
        mahiVector(6);

    }
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
        connectPasserAndReciever();
    } else {
        newFillRoleProperties();
        newPosExecute();
        checkEndState();
        if(agentSize > 1)
            passManager();

        if(isPlanEnd()) {

        }
    }
}


void CPlayOff::dynamicExecute() {
    // TODO : Write Dynamic Execution (playoff)
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
    for(int i =0; i < agentSize;i++) {
        newRoleAgent[i]->setAvoidBall(true);
        newRoleAgent[i]->setAvoidPenaltyArea(true);
        newRoleAgent[i]->setSelectedSkill(SkillGotopointAvoid);
    }
    switch(tAgentsize) {
    /*case 0:
        break;*/
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
    default:
        break;
        /*case 5:
        break;
    case 6:
        break;*/

    }
    //    for(int i =0; i < agentSize;i++) {
    //        newRoleAgent[i]->setAgent(knowledge->getAgent(activeAgents.at(i)->id()));
    //    }
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
    newRoleAgent[0]->setTargetDir(-activeAgents.at(0)->pos() + wm->ball->pos);
    newRoleAgent[1]->setTarget(kickOffPos[1]);
    newRoleAgent[1]->setTargetDir(-activeAgents.at(1)->pos() + wm->field->oppGoal());
    newRoleAgent[2]->setTarget(kickOffPos[2]);
    newRoleAgent[2]->setTargetDir(-activeAgents.at(2)->pos() + wm->field->oppGoal());
    newRoleAgent[3]->setTarget(kickOffPos[3]);
    newRoleAgent[3]->setTargetDir(-activeAgents.at(3)->pos() + wm->field->oppGoal());
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
    if (isTimeOver() ) {
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

bool CPlayOff::isTimeOver() {
    if (setTimer) {
        tempStart = knowledge->getCurrentKKTime();
    }

    if (!Circle2D(lastBallPos, 0.5).contains(wm->ball->pos)) {
        setTimer = false;
        if(knowledge->getCurrentKKTime() - tempStart > 200) {
            setTimer = true;
            return true;
        }
    }
    return false;
}

bool CPlayOff::isBallDirChanged() {
    for (size_t agent = 0;agent < 6;agent++) {
        if(positionAgent[agent].positionArg.size()) {
            if (positionAgent[agent].getArgs().staticSkill == PassSkill) {
                debug("HEY IT'S A PASS", D_MAHI);
                if (isPassFaild(agent)) {
                    debug(QString("PASS FAILD"),D_MAHI);
                    draw(QString("PASS FAILD"),Vector2D(2,-1.5));
                    return true;
                }
            }
        }
    }
    return false;
}

bool CPlayOff::isFinalShotDone() {
    CAgent * tempagent;
    int kickLeft = 0;

    for(int i = 0;i < agentSize;i++) {
        for( int j = positionAgent[i].stateNumber; j < positionAgent[i].positionArg.size();j++) {
            if(positionAgent[i].positionArg[j].staticSkill == OneTouchSkill
                    || positionAgent[i].positionArg[j].staticSkill == ChipToGoalSkill
                    || positionAgent[i].positionArg[j].staticSkill == ShotToGoalSkill
                    || positionAgent[i].positionArg[j].staticSkill == PassSkill) {
                kickLeft++;
            }
        }
    }

    debug(QString("KICK LEFT : %1").arg(kickLeft),D_MAHI);

    for(int i = 0;i < agentSize;i++) {
        tempagent = knowledge->getAgent(kkAgentsID[i]);

        if(kickLeft == 1)
            if(positionAgent[i].positionArg.size())
                if(positionAgent[i].positionArg.at(positionAgent[i].stateNumber).staticSkill == OneTouchSkill
                        || positionAgent[i].positionArg.at(positionAgent[i].stateNumber).staticSkill == ChipToGoalSkill
                        || positionAgent[i].positionArg.at(positionAgent[i].stateNumber).staticSkill == ShotToGoalSkill) {
                    draw(Circle2D(tempagent->pos() + tempagent->dir().norm()*0.08,0.12),QColor(Qt::red));
                    bugflag = true;
                    if(Circle2D(tempagent->pos() + tempagent->dir().norm()*0.08,0.12).contains(wm->ball->pos)) {
                        isBallIn = true;
                    }
                    if(isBallIn && !Circle2D(tempagent->pos() + tempagent->dir().norm()*0.08,0.30).contains(wm->ball->pos)) {
                        isBallIn = false;
                        debug(QString("findout!!"),D_MAHI);
                        return true;
                    }
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
    }
    else {
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
    if(knowledge->getCurrentKKTime() - positionAgent[agent].mahiLastTime
            > positionAgent[agent].positionArg.at(positionAgent[agent].stateNumber).rightData + 500) {
        debug(QString("SECOND FAILD : %1").arg(agent),D_MAHI);
        return true;
    }
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
    if (ownerReceiveList.size()) {
        int tReciverAgent = ownerReceiveList.at(0).receiveAgent;
        int tReciverIndex = ownerReceiveList.at(0).receiveIndex;

        if (positionAgent[tReciverAgent].stateNumber >= tReciverIndex) {
            if (positionAgent[tReciverAgent].getArgs().staticPos.dist(knowledge->getAgent(kkAgentsID[tReciverAgent])->pos()) > currentPlan->config.tuneParams.lastDist) {
                doPass = false;
            }
            else {
                doPass = true;
            }
        }
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
            tempTime -= knowledge->getCurrentKKTime() - positionAgent[tempBallOwner].mahiLastTime;
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
            tempTime -= knowledge->getCurrentKKTime() - positionAgent[tempReceiver].mahiLastTime;
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

bool CPlayOff::isTaskDone(int agentID){
    CAgent* tAgent;
    tAgent = knowledge->getAgent(kkAgentsID[agentID]);

    if(roleAgent[agentID]->getSelectedSkill() == SkillGotopointAvoid) {
        if(isMoveDone(agentID))
            return true;
    }
    else if(roleAgent[agentID]->getSelectedSkill() == SkillKick) {
        if(isKickDone(tAgent,agentID))
            return true;
    }
    else if(roleAgent[agentID]->getSelectedSkill() == SkillReceivePass) {
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
    else if(roleAgent[agentID]->getSelectedSkill() == SkillOneTouch) {
        if(isKickDone(tAgent,agentID))
            return true;
    }
    else if(roleAgent[agentID]->getSelectedSkill() == SkillMark) {
        return true;
    }
    else {
        return false;
    }
}



////////////////////////////////////////////
///////////////////////////////////////////
bool CPlayOff::isMoveDone(int agentID) {
    long tempDiffTime;
    tempDiffTime = knowledge->getCurrentKKTime() - positionAgent[agentID].mahiLastTime;
    debug(QString("nuTimer : %1").arg(positionAgent[agentID].mahiLastTime),D_MAHI);
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
            positionAgent[i].mahiLastTime = knowledge->getCurrentKKTime();
            isFirstTime[i] = false;
        }
        roleAgent[i]->execute();
    }
}

void CPlayOff::newPosExecute() {
    for(int i = 0;i < masterPlan->common.agentSize; i++) {
        // TODO : Check First Time
        roleAgent[i]->execute();
    }
}

void CPlayOff::newCheckEndState() {
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
    for(size_t i = 0;i < masterPlan->common.agentSize; i++) {
        if (masterPlan->common.matchedID.contains(i)) {
            roleAgent[i]->setAgent(knowledge->getAgent(masterPlan->common.matchedID.value(i)));
            newAssignTask(roleAgent[i], positionAgent[i]);

        } else {

            qWarning() << "[Warning] coach -> Match function doesn't work :( ";
            return;
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
        roleAgent[agentID]->setSelectedSkill(SkillKick);
        break;
    case ReceivePassSkill:
        roleAgent[agentID]->setAvoidPenaltyArea(true);
        roleAgent[agentID]->setTarget(positionAgent[agentID].getArgs().staticPos);
        roleAgent[agentID]->setReceiveRadius(positionAgent[agentID].getArgs().staticEscapeRadius);
        roleAgent[agentID]->setIgnoreAngle(false);
        roleAgent[agentID]->setSelectedSkill(SkillReceivePass);
        break;
    case ReceivePassIASkill:
        roleAgent[agentID]->setAvoidPenaltyArea(true);
        roleAgent[agentID]->setTarget(positionAgent[agentID].getArgs().staticPos);
        roleAgent[agentID]->setReceiveRadius(positionAgent[agentID].getArgs().staticEscapeRadius);
        roleAgent[agentID]->setIgnoreAngle(true);
        roleAgent[agentID]->setTargetDir(positionAgent[agentID].getArgs().staticAng);
        roleAgent[agentID]->setSelectedSkill(SkillReceivePass);
        break;
    case ShotToGoalSkill:
        //        roleAgent[agentID]->setChip(false);
        roleAgent[agentID]->setChip(isPathClear(wm->ball->pos,wm->field->oppGoal(),0.5,0.1));
        roleAgent[agentID]->setKickSpeed(positionAgent[agentID].getArgs().leftData);
        roleAgent[agentID]->setTarget(getGoalTarget(agentID,positionAgent[agentID].stateNumber));
        roleAgent[agentID]->setIntercept(false);
        roleAgent[agentID]->setSelectedSkill(SkillKick);
        break;
    case ChipToGoalSkill:
        roleAgent[agentID]->setChip(true);
        roleAgent[agentID]->setKickSpeed(positionAgent[agentID].getArgs().leftData);
        roleAgent[agentID]->setTarget(getGoalTarget(agentID,positionAgent[agentID].stateNumber));
        roleAgent[agentID]->setIntercept(false);
        roleAgent[agentID]->setSelectedSkill(SkillKick);
        break;
    case OneTouchSkill:
        //        roleAgent[agentID]->setAvoidPenaltyArea(true);
        //        roleAgent[agentID]->setChip(false);
        roleAgent[agentID]->setWaitPos(positionAgent[agentID].getArgs().staticPos);
        roleAgent[agentID]->setKickSpeed(positionAgent[agentID].getArgs().leftData);
        roleAgent[agentID]->setTarget(getGoalTarget(agentID, positionAgent[agentID].stateNumber));
        roleAgent[agentID]->setSelectedSkill(SkillOneTouch);
        break;
    case MoveSkill:
        roleAgent[agentID]->setAvoidPenaltyArea(true);
        roleAgent[agentID]->setTargetDir(positionAgent[agentID].getArgs().staticAng);
        roleAgent[agentID]->setMaxVelocity(getMaxVel(agentID, positionAgent[agentID].stateNumber));
        if(positionAgent[agentID].getArgs().staticPos == POBALLPOS)
        {
            roleAgent[agentID]->setTarget(wm->ball->pos - tempVec*0.14);
            roleAgent[agentID]->setTargetDir(tempVec);
            roleAgent[agentID]->setSlow(true);
            roleAgent[agentID]->setMaxVelocity(1);
        }
        else
            roleAgent[agentID]->setTarget(getMoveTarget(agentID, positionAgent[agentID].stateNumber));
        roleAgent[agentID]->setSelectedSkill(SkillGotopointAvoid);
        break;
    case NoSkill:
        roleAgent[agentID]->setSelectedSkill(SkillMark);
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
    if (_roleAgent->getChip())
        _roleAgent->setKickSpeed(_posAgent.getArgs().rightData);
    else
        _roleAgent->setKickSpeed(_posAgent.getArgs().leftData);

    _roleAgent->setTarget(positionAgent[_posAgent.getArgs().PassToId].getArgs(_posAgent.getArgs().PassToState).staticPos);
    _roleAgent->setDoPass(doPass);
    _roleAgent->setIntercept(false);
    _roleAgent->setSelectedSkill(SkillKick);
}

void CPlayOff::assignReceive(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent, bool _ignoreAngle) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setIgnoreAngle(_ignoreAngle);
    _roleAgent->setTarget(_posAgent.getArgs().staticPos);
    _roleAgent->setTargetDir(_posAgent.getArgs().staticAng); /** Just Matter when we use Ignore mode **/
    _roleAgent->setReceiveRadius(_posAgent.getArgs().staticEscapeRadius);
    _roleAgent->setSelectedSkill(SkillReceivePass);
}

void CPlayOff::assignKick(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent, bool _chip) {

        _roleAgent->setChip(_chip);
        _roleAgent->setKickSpeed(_posAgent.getArgs().leftData);
        _roleAgent->setTarget(getGoalTarget(_posAgent.getArgs().rightData));
        _roleAgent->setIntercept(false);
        _roleAgent->setSelectedSkill(SkillKick);
}

void CPlayOff::assignOneTouch(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent) {

    _roleAgent->setAvoidPenaltyArea(true);
    _roleAgent->setWaitPos(_posAgent.getArgs().staticPos);
    _roleAgent->setKickSpeed(_posAgent.getArgs().leftData);
    _roleAgent->setTarget(getGoalTarget(_posAgent.getArgs().rightData));
    _roleAgent->setSelectedSkill(SkillOneTouch);
}

void CPlayOff::assignMove(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent) {
    _roleAgent->setAvoidPenaltyArea(true);

    if(_posAgent.getArgs().staticPos == POBALLPOS)
    {
        _roleAgent->setTarget(wm->ball->pos - (wm->ball->pos - _roleAgent->getTarget()).norm()*0.14);
        _roleAgent->setTargetDir((wm->ball->pos - _roleAgent->getAgent()->pos()).norm());
        _roleAgent->setSlow(true);
        _roleAgent->setMaxVelocity(1);

    } else {

        _roleAgent->setTarget(getMoveTarget(_posAgent.getArgs()));
        _roleAgent->setTargetDir(_posAgent.getArgs().staticAng);
        _roleAgent->setSlow(false);
        _roleAgent->setMaxVelocity(getMaxVel(_roleAgent, _posAgent.getArgs()));
    }
    _roleAgent->setSelectedSkill(SkillGotopointAvoid);
}

void CPlayOff::assignAfterLife(CRolePlayOff* _roleAgent, const SPositioningAgent& _posAgent) {
    // TODO : Write Ineteligence AfterLife Program

    _roleAgent->setSelectedSkill(SkillMark);
}


Vector2D CPlayOff::getMoveTarget(int agentID,int agentState) {
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

double CPlayOff::getMaxVel(int agentID,int agentState) {
    double tempVel,tDist;
    Vector2D tAgentPos;
    tAgentPos = knowledge->getAgent(kkAgentsID[agentID])->pos();
    tDist = tAgentPos.dist(positionAgent[agentID].positionArg.at(positionAgent[agentID].stateNumber).staticPos);
    tempVel = tDist/positionAgent[agentID].positionArg.at(positionAgent[agentID].stateNumber).leftData;
    if(tempVel >= 4) tempVel = 4;
    else if(tempVel <= 1.5) tempVel = 1.5;
    return tempVel;
}


double CPlayOff::getMaxVel(const CRolePlayOff* _roleAgent, const SPositioningArg& _posArg) {
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

    if(_posArg.leftData < 0)
        return true;
    else if(_posArg.rightData < 0)
        return false;
    else
        return !isPathClear(wm->ball->pos,
                            positionAgent[_posArg.PassToId].getArgs(_posArg.PassToState).staticPos,
                            0.5,   // Radius
                            0.1);  // Tereshold
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
            tempPosArg.PassToId           = currentPlan->AgentPlan[i].at(j).targetAgent;
            tempPosArg.PassToState        = currentPlan->AgentPlan[i].at(j).targetIndex;

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
    for(size_t i = 0;i < masterPlan->common.agentSize; i++) {
        Q_FOREACH(playOffRobot agentPlan, masterPlan->execution.AgentPlan[i]) {
            SPositioningArg tempPosArg;
            tempPosArg.staticPos          = agentPlan.pos;
            tempPosArg.staticAng          = Vector2D::polar2vector(1, agentPlan.angle);
            tempPosArg.PassToId           = agentPlan.targetAgent;
            tempPosArg.PassToState        = agentPlan.targetIndex;
            tempPosArg.staticEscapeRadius = agentPlan.tolerance;
            tempPosArg.staticAng.assign(tempPosArg.staticAng.x, -tempPosArg.staticAng.y);

            Q_FOREACH(playOffSkill skill, agentPlan.skill) {
                if (skill.name == PassSkill)
                    tempPosArg.staticPos = POBALLPOS;

                tempPosArg.leftData  = skill.data[0];
                tempPosArg.rightData  = skill.data[1];
                tempPosArg.staticSkill = skill.name;

                positionAgent[i].positionArg.append(tempPosArg);
            }
        }
    }
}

void CPlayOff::connectPasserAndReciever() {
    for(size_t i = 0; i < masterPlan->common.agentSize; i++) {
        Q_FOREACH(SPositioningArg posArg, positionAgent[i].positionArg) {
            if (posArg.staticSkill == PassSkill)
                // TODO : Fill Position Arg Reciver Part
                return;
        }
    }
}

void CPlayOff::findPasserIndex() {
    firstPasserID = -1;
    //    debug(QString("current size : %1 , agentplansize %2").arg(currentPlan->agentSize).arg(currentPlan->AgentPlan[0].size()),D_KK);
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
        positionAgent[i].mahiLastTime = knowledge->getCurrentKKTime();
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
    executedCycles = 0;
    activeAgents.clear();
    markAgents.clear();
    debug(QString("reset Plan"),D_MAHI);
    for(int i = 0;i < 6;i++) {
        positionAgent[i].stateNumber = 0;
        positionAgent[i].mahiLastTime = knowledge->getCurrentKKTime();
        isFirstTime[i] = 0;
        //        currentPlan->AgentPlan[i].clear();
        currentPlan = NULL;
    }
    isBallIn = false;

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
    globalExecute(0);
}

void CPlayOff::execute_1(){
    globalExecute(1);
}

void CPlayOff::execute_2(){
    globalExecute(2);
}

void CPlayOff::execute_3(){
    globalExecute(3);
}

void CPlayOff::execute_4(){
    globalExecute(4);
}

void CPlayOff::execute_5(){
    globalExecute(5);
}

void CPlayOff::execute_6(){
    globalExecute(6);
}


/////////////////////SIGNAL & SLOTS
QList< QList<SPlayOffPlan*> > CPlayOff::updatePlans() {

    //    //Destructor
    for(int i = 0;i < 6;i++) {
        delete roleAgent[i];
        delete newRoleAgent[i];

    }
    delete tempAgent;

    for (size_t i = 0; i < fullPlans.size();i++) {
        for (size_t j = 0;j < fullPlans[i].size();j++) {
            delete fullPlans[i][j];
        }
    }

    //Constructor
    radLimit = 2;
    decidePlan = true;
    kickOffFirstTimeFlag = true;
    firstTime = true;
    agentSize = 1;
    for(int i = 0;i < agentSize;i++) positionAgent[i].stateNumber = 0;
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
    currentPlan = new SPlayOffPlan();
    //Load Plans
    return loadSQLs(dirList);


}

///////////GUI
QList< QList<SPlayOffPlan*> > CPlayOff::loadSQLs(QList<QString> _directorys) {

    planListKickOff.clear();
    planListDirect.clear();
    planListIndirect.clear();

    for (size_t i = 0;i < fullPlans.size();i++) {
        fullPlans[i].clear();
    }
    fullPlans.clear();

    QList<SPlayOffPlan*> tempRes;

    for (size_t i = 0;i < _directorys.size();i++) {

        if (!dirList.contains(_directorys.at(i))) {
            dirList.append(_directorys.at(i));
        }

        planSql = QSqlDatabase::addDatabase("QSQLITE");
        planSql.setDatabaseName(_directorys.at(i));
        if (!planSql.open()) {
            QMessageBox::critical(0, "Cannot open database",
                                  "Unable to establish a database connection.\n"\
                                  "This example needs SQLite support. Please read "\
                                  "the Qt SQL driver documentation for information how "\
                                  "to build it.\n\n"\
                                  "Click Cancel to exit.", QMessageBox::Cancel);
        }
        qDebug()<<"PlayOff SQL Connected!";
        QSqlQuery squery;
        squery = QSqlQuery(planSql);
        squery.exec("SELECT * FROM poplanlist ORDER BY id ASC");

        while(squery.next())
        {
            SPlayOffPlan* tempPlan = new SPlayOffPlan();;
            tempPlan->planMode = POMODE(squery.value(2).toInt());
            tempPlan->agentSize = squery.value(3).toInt();
            loadEachPlan(tempPlan, squery.value(1).toString(), 1);

            if(squery.value(4).toString() != "na")
            {
                tempPlan->initPos.ball = convertPos(squery.value(4).toString().split("|").at(0).toInt(),
                                                    squery.value(4).toString().split("|").at(1).toInt(), 1);
            }
            else
            {
                tempPlan->initPos.ball = Vector2D(100, 100);
            }

            for(int i = 0; i < 6; i++)
            {
                if(squery.value(5 + i).toString() != "na")
                {
                    tempPlan->initPos.Agent[i] = convertPos(squery.value(5 + i).toString().split("|").at(0).toInt(),
                                                            squery.value(5 + i).toString().split("|").at(1).toInt(), 1);
                }
                else
                {
                    tempPlan->initPos.Agent[i] = Vector2D(100, 100);
                }
            }

            tempPlan->config.name   = getModeStr(tempPlan->planMode)
                    + QString(" -> no.%1").arg(squery.at())
                    + QString(" s: %1").arg(tempPlan->agentSize);

            if(tempPlan->planMode == KICKOFF)
                planListKickOff.append(tempPlan);
            else if(tempPlan->planMode == DIRECT)
                planListDirect.append(tempPlan);
            else if(tempPlan->planMode == INDIRECT) {
                planListIndirect.append(tempPlan);
                planListDirect.append(tempPlan);
            }
            tempRes.append(tempPlan);
            //////////////////////////////////
            //////////////SYMMETRY////////////
            //////////////////////////////////
            SPlayOffPlan* symmetryPlan = new SPlayOffPlan();;

            if(policy()->KKPlayOff_KKPOSymmetry()) {

                symmetryPlan->planMode = POMODE(squery.value(2).toInt());
                symmetryPlan->agentSize = squery.value(3).toInt();
                loadEachPlan(symmetryPlan, squery.value(1).toString(), -1);

                if(squery.value(4).toString() != "na")
                {
                    symmetryPlan->initPos.ball = convertPos(squery.value(4).toString().split("|").at(0).toInt(),
                                                            squery.value(4).toString().split("|").at(1).toInt(), -1);
                }
                else
                {
                    symmetryPlan->initPos.ball = Vector2D(100, 100);
                }

                for(int i = 0; i < 6; i++)
                {
                    if(squery.value(5 + i).toString() != "na")
                    {
                        symmetryPlan->initPos.Agent[i] = convertPos(squery.value(5 + i).toString().split("|").at(0).toInt(),
                                                                    squery.value(5 + i).toString().split("|").at(1).toInt(), -1);
                    }
                    else
                    {
                        symmetryPlan->initPos.Agent[i] = Vector2D(100, 100);
                    }
                }

                symmetryPlan->config.name   = getModeStr(tempPlan->planMode)
                        + QString(" -> no.%1_S").arg(squery.at())
                        + QString(" s: %1").arg(symmetryPlan->agentSize);
                if(symmetryPlan->planMode == KICKOFF)
                    planListKickOff.append(symmetryPlan);
                else if(symmetryPlan->planMode == DIRECT)
                    planListDirect.append(symmetryPlan);
                else if(symmetryPlan->planMode == INDIRECT) {
                    planListIndirect.append(symmetryPlan);
                    planListDirect.append(symmetryPlan);
                }
                tempRes.append(symmetryPlan);
            }
        }
        fullPlans.append(tempRes);
        tempRes.clear();
    }
    qDebug()<<"PlayOff SQL Loaded!";
    qDebug()<< QString("Mahi %1").arg(planListIndirect.size());
    return fullPlans;
}

void CPlayOff::addSQL(QString _directory) {


    if (dirList.contains(_directory)) return;

    dirList.append(_directory);
    planSql = QSqlDatabase::addDatabase("QSQLITE");
    planSql.setDatabaseName(_directory);
    if (!planSql.open()) {
        QMessageBox::critical(0, "Cannot open database",
                              "Unable to establish a database connection.\n"\
                              "This example needs SQLite support. Please read "\
                              "the Qt SQL driver documentation for information how "\
                              "to build it.\n\n"\
                              "Click Cancel to exit.", QMessageBox::Cancel);
    }
    qDebug()<<"PlayOff SQL Connected!";
    QSqlQuery squery;
    squery = QSqlQuery(planSql);
    squery.exec("SELECT * FROM poplanlist ORDER BY id ASC");

    while(squery.next())
    {
        SPlayOffPlan* tempPlan = new SPlayOffPlan();;
        tempPlan->planMode = POMODE(squery.value(2).toInt());
        tempPlan->agentSize = squery.value(3).toInt();
        loadEachPlan(tempPlan, squery.value(1).toString(), 1);

        if(squery.value(4).toString() != "na")
        {
            tempPlan->initPos.ball = convertPos(squery.value(4).toString().split("|").at(0).toInt(),
                                                squery.value(4).toString().split("|").at(1).toInt(), 1);
        }
        else
        {
            tempPlan->initPos.ball = Vector2D(100, 100);
        }

        for(int i = 0; i < 6; i++)
        {
            if(squery.value(5 + i).toString() != "na")
            {
                tempPlan->initPos.Agent[i] = convertPos(squery.value(5 + i).toString().split("|").at(0).toInt(),
                                                        squery.value(5 + i).toString().split("|").at(1).toInt(), 1);
            }
            else
            {
                tempPlan->initPos.Agent[i] = Vector2D(100, 100);
            }
        }
        if(tempPlan->planMode == KICKOFF)
            planListKickOff.append(tempPlan);
        else if(tempPlan->planMode == DIRECT)
            planListDirect.append(tempPlan);
        else if(tempPlan->planMode == INDIRECT) {
            planListIndirect.append(tempPlan);
            planListDirect.append(tempPlan);
        }
        //        debug(QString("%1").arg(int(tempPlan->planMode)), D_KK);

        //////////////////////////////////
        //////////////SYMMETRY////////////
        //////////////////////////////////
        SPlayOffPlan* symmetryPlan = new SPlayOffPlan();;

        if(policy()->KKPlayOff_KKPOSymmetry()) {

            symmetryPlan->planMode = POMODE(squery.value(2).toInt());
            symmetryPlan->agentSize = squery.value(3).toInt();
            loadEachPlan(symmetryPlan, squery.value(1).toString(), -1);

            if(squery.value(4).toString() != "na")
            {
                symmetryPlan->initPos.ball = convertPos(squery.value(4).toString().split("|").at(0).toInt(),
                                                        squery.value(4).toString().split("|").at(1).toInt(), -1);
            }
            else
            {
                tempPlan->initPos.ball = Vector2D(100, 100);
            }

            for(int i = 0; i < 6; i++)
            {
                if(squery.value(5 + i).toString() != "na")
                {
                    tempPlan->initPos.Agent[i] = convertPos(squery.value(5 + i).toString().split("|").at(0).toInt(),
                                                            squery.value(5 + i).toString().split("|").at(1).toInt(), -1);
                }
                else
                {
                    tempPlan->initPos.Agent[i] = Vector2D(100, 100);
                }
            }
            if(symmetryPlan->planMode == KICKOFF)
                planListKickOff.append(symmetryPlan);
            else if(symmetryPlan->planMode == DIRECT)
                planListDirect.append(symmetryPlan);
            else if(symmetryPlan->planMode == INDIRECT) {
                planListIndirect.append(symmetryPlan);
                planListDirect.append(symmetryPlan);
            }
        }
        //////////////////////////////////
    }
    qDebug()<<"PlayOff SQL Loaded!";
    qDebug()<< QString("Mahi %1").arg(planListIndirect.size());
}


QList< QList<SPlayOffPlan*> > CPlayOff::addSQLs(QStringList _directorys) {

    QList<SPlayOffPlan*> tempRes;

    for (size_t i = 0;i < _directorys.size();i++) {

        if (dirList.contains(_directorys.at(i))) continue;

        dirList.append(_directorys.at(i));
        planSql = QSqlDatabase::addDatabase("QSQLITE");
        planSql.setDatabaseName(_directorys.at(i));
        if (!planSql.open()) {
            QMessageBox::critical(0, "Cannot open database",
                                  "Unable to establish a database connection.\n"\
                                  "This example needs SQLite support. Please read "\
                                  "the Qt SQL driver documentation for information how "\
                                  "to build it.\n\n"\
                                  "Click Cancel to exit.", QMessageBox::Cancel);
        }
        qDebug()<<"PlayOff SQL Connected!";
        QSqlQuery squery;
        squery = QSqlQuery(planSql);
        squery.exec("SELECT * FROM poplanlist ORDER BY id ASC");

        while(squery.next())
        {
            SPlayOffPlan* tempPlan = new SPlayOffPlan();;
            tempPlan->planMode = POMODE(squery.value(2).toInt());
            tempPlan->agentSize = squery.value(3).toInt();
            loadEachPlan(tempPlan, squery.value(1).toString(), 1);

            if(squery.value(4).toString() != "na")
            {
                tempPlan->initPos.ball = convertPos(squery.value(4).toString().split("|").at(0).toInt(),
                                                    squery.value(4).toString().split("|").at(1).toInt(), 1);
            }
            else
            {
                tempPlan->initPos.ball = Vector2D(100, 100);
            }

            for(int i = 0; i < 6; i++)
            {
                if(squery.value(5 + i).toString() != "na")
                {
                    tempPlan->initPos.Agent[i] = convertPos(squery.value(5 + i).toString().split("|").at(0).toInt(),
                                                            squery.value(5 + i).toString().split("|").at(1).toInt(), 1);
                }
                else
                {
                    tempPlan->initPos.Agent[i] = Vector2D(100, 100);
                }
            }

            tempPlan->config.name   = getModeStr(tempPlan->planMode)
                    + QString(" -> no.%1").arg(squery.at())
                    + QString(" s: %1").arg(tempPlan->agentSize);
            if(tempPlan->planMode == KICKOFF)
                planListKickOff.append(tempPlan);
            else if(tempPlan->planMode == DIRECT)
                planListDirect.append(tempPlan);
            else if(tempPlan->planMode == INDIRECT) {
                planListIndirect.append(tempPlan);
                planListDirect.append(tempPlan);
            }
            tempRes.append(tempPlan);
            //////////////////////////////////
            //////////////SYMMETRY////////////
            //////////////////////////////////
            SPlayOffPlan* symmetryPlan = new SPlayOffPlan();;

            if(policy()->KKPlayOff_KKPOSymmetry()) {

                symmetryPlan->planMode = POMODE(squery.value(2).toInt());
                symmetryPlan->agentSize = squery.value(3).toInt();
                loadEachPlan(symmetryPlan, squery.value(1).toString(), -1);

                if(squery.value(4).toString() != "na")
                {
                    symmetryPlan->initPos.ball = convertPos(squery.value(4).toString().split("|").at(0).toInt(),
                                                            squery.value(4).toString().split("|").at(1).toInt(), -1);
                }
                else
                {
                    symmetryPlan->initPos.ball = Vector2D(100, 100);
                }

                for(int i = 0; i < 6; i++)
                {
                    if(squery.value(5 + i).toString() != "na")
                    {
                        symmetryPlan->initPos.Agent[i] = convertPos(squery.value(5 + i).toString().split("|").at(0).toInt(),
                                                                    squery.value(5 + i).toString().split("|").at(1).toInt(), -1);
                    }
                    else
                    {
                        symmetryPlan->initPos.Agent[i] = Vector2D(100, 100);
                    }
                }

                symmetryPlan->config.name   = getModeStr(tempPlan->planMode)
                        + QString(" -> no.%1_S").arg(squery.at())
                        + QString(" s: %1").arg(symmetryPlan->agentSize);
                if(symmetryPlan->planMode == KICKOFF)
                    planListKickOff.append(symmetryPlan);
                else if(symmetryPlan->planMode == DIRECT)
                    planListDirect.append(symmetryPlan);
                else if(symmetryPlan->planMode == INDIRECT) {
                    planListIndirect.append(symmetryPlan);
                    planListDirect.append(symmetryPlan);

                }
                tempRes.append(symmetryPlan);
            }

        }
        fullPlans.append(tempRes);
        tempRes.clear();
    }
    qDebug()<<"PlayOff SQL Loaded!";
    qDebug()<< QString("Mahi %1").arg(planListIndirect.size());
    return fullPlans;
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

void CPlayOff::setMasterPlan(const SPlan *_thePlan) {
    masterPlan = _thePlan;
}

void CPlayOff::setMasterMode(const EMode &_mode) {
    masterMode = _mode;
}

EMode CPlayOff::getMasterMode() {
    return masterMode;
}

///////////OverLoading Operators
QDebug operator<< (QDebug d, const NGameOff::SPlan _plan) {

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
