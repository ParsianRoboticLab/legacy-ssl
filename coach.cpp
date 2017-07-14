#include "coach.h"
#include <QStringList>
#include "gamelogger.h"
#include "ui/varswidget.h"
#include "passevaluation.h"


#include <conditions.h>
#include <playmake.h>
#include <exceptions.h>

#include <QApplication>

#include <algorithm>
QMap<QString, EditData*> CCoach::editData;

CCoach::CCoach(CAgent**_agents)
{
    goalieTrappedUnderGoalNet = false;
    inited = false;
    agents = _agents;
    lastSelected = -1;
    lastAssignCycle = -10;
    lastBallVelPM = Vector2D(0,0);
    lastBallPos = Vector2D(0,0);
    passPos = Vector2D(0,0);
    passPlayMake = false;
    first = true;

    ///////////////////////////////////
    goalieTimer.start();
    ////////////////////intentions
    playOnExecTime.start();
    intentionTimePossession.start();
    playMakeIntention.start();

    possessionIntentionInterval = 200;
    playOnTime = 2000;
    playMakeIntentionInterval = 300;
    playMakeTh = 0.3;


    /////////////////////
    //initialize behaviours
    for (int i=0;i<CSkills::skillsCount();i++)
    {
        CRoleInfo* info = (CRoleInfo*) CSkills::getInfo(CSkills::skill(i)->getName().toStdString().c_str());
        if (info != NULL)
        {
            info->calculated = false;
        }
    }
    for (int i=0;i<CSkills::skillsCount();i++)
    {
        CRoleInfo* info = (CRoleInfo*) CSkills::getInfo(CSkills::skill(i)->getName().toStdString().c_str());
        if (info != NULL)
        {
            if (!info->calculated)
            {
                info->initBehaviours();
                info->calculated = true;
            }
        }
    }
    for (int i=0;i<CSkills::skillsCount();i++)
    {
        CRoleInfo* info = (CRoleInfo*) CSkills::getInfo(CSkills::skill(i)->getName().toStdString().c_str());
        if (info != NULL)
        {
            info->calculated = false;
        }
    }

    loadPostAssignment();
    //	qDebug() << "Loaded Assignment:" ;
    //	qDebug() << "Defense" ;
    //	for (int i = 0;i < lastAssign.first.count();i++)
    //		qDebug() << lastAssign.first[i]->id();
    //	qDebug() << "Offense" ;
    //	for (int i = 0;i < lastAssign.second.count();i++)
    //		qDebug() << lastAssign.second[i]->id();


    // Old Plays
    ourKickOff          = new COurKickOff;
    ourPenalty          = new COurPenalty;
    forceStart          = new CForceStart;
    ourIndirect         = new COurIndirect;
    theirDirect         = new CTheirDirect;
    theirKickOff        = new CTheirKickOff;
    theirPenalty        = new CTheirPenalty;
    theirIndirect       = new CTheirIndirect;
    ourBallPlacement    = new COurBallPlacement;
    halfTimeLineup    = new CHalftimeLineup;
    theirBallPlacement  = new CTheirBallPlacement;
    ourDoubleSizeDirect = new CDoubleSizeOurDirect;


    // New Plays
    ourPlayOff          = new CPlayOff;
    dynamicAttack       = new CDynamicAttack();

    //Stop
    stopPlay            = new CStopPlay();

    for( int i=0 ; i<_MAX_NUM_PLAYERS ; i++ ){
        stopRoles[i] = new CRoleStop(knowledge->getAgent(i));
    }

    lastDefenseAgents.clear();

    defenseTimeForVisionProblem[0].start();
    defenseTimeForVisionProblem[1].start();
    transientFlag = false;
    trasientTimeOut.start();
    translationTimeOutTime = 2000;
    exeptionPlayMake = NULL;
    exeptionPlayMakeThr = 0;

    staticPlayoffPlansCounter = 0;
    shuffleCounter = 0;
    shuffleSize = 0;
    shuffled = false;
    firstPlanRepeatInit = true;
    staticPlayoffPlansShuffleIndexing.clear();
    m_planLoader = new CLoadPlayOffJson(QDir::currentPath() + QString("/playoff"));
    goalieAgent = NULL;
    firstPlay = true;
    firstIsFinished = false;
    preferedDefenseCounts = 2;
    preferedShotSpot = EveryWhere;
    ///HMD
    overDefThr = 0;

    minChance = 10;
    minChanceRepeat = 10;

    playoffPlanSelectionDataFile.setFileName("PlayoffPlanRepeat.txt");
    out.setDevice(&playoffPlanSelectionDataFile);

    for (int i = 0; i < 12 ; i++) faultDetectionCounter[i] = 0;
}

CCoach::~CCoach()
{
    savePostAssignment();
    saveLFUReapeatData(LFUList);
}

void CCoach::saveGoalie()
{
    //    debug((QString("goalie under net timer : %1").arg(goalieTimer.elapsed())),D_MHMMD);
    if(goalieTimer.elapsed() > 100 && knowledge->goalie != NULL)
    {
        knowledge->goalie->setRobotAbsVel(1,0,0);
    }
    return;
    if (goalieTrappedUnderGoalNet && knowledge->goalie != NULL)
    {
        knowledge->goalie->setRobotAbsVel(1, 0.0, 0.0);
    }
}

bool isNan(double x)
{
    return (QString("%1").arg(x).trimmed().toLower() == "nan");
}

void CCoach::checkGoalieInsight()
{
    goalieTrappedUnderGoalNet = false;
    /////////////////////////////////////////////////// new method by Don Mhmmd
    if(goalieAgent != NULL)
    {
        if (goalieAgent->isVisible())
        {
            goalieTimer.restart();
        }
        debug("inja miad",D_MHMMD);

    }
    else
    {
        return;
    }

    /////////////////////////////////
    return;
    if (knowledge->goalie != NULL)
    {
        if ((knowledge->goalie->notVisible()) || (fabs(knowledge->goalie->getVisibility() - 0.5) < 0.01))
        {
            if ( (knowledge->goalie->pos() - wm->field->ourGoal()).length() < 1.0)
            {
                knowledge->goalie->setVisibility(1.0);
                if (!wm->our.data->activeAgents.contains(knowledge->goalie->self()->id))
                    wm->our.data->activeAgents.append(knowledge->goalie->self()->id);
                goalieTrappedUnderGoalNet = true;
            }
            else {
                //				debug(QString("%1, %2").arg(knowledge->goalie->pos().x).arg(knowledge->goalie->pos().y), D_ERROR);
            }
        }
    }
}

void CCoach::checkTransitionToForceStart(){
    Vector2D lastPos;
    if( wm->ball->hist.size() > 10 ){
        lastPos = wm->ball->hist.at(wm->ball->hist.size()-10).pos;
    }
    else{
        if( wm->ball->hist.size() )
            lastPos = wm->ball->hist.first().pos;
        else
            lastPos = wm->ball->pos;
    }

    double ballChangedPosDist = wm->ball->pos.dist(lastPos);

    if( knowledge->isStart() == false ){
        if( cyclesWaitAfterballMoved == 0 && ballChangedPosDist > 0.05 ){
            cyclesWaitAfterballMoved = 1;
        }
        else if( cyclesWaitAfterballMoved ){
            cyclesWaitAfterballMoved++;
        }
    }
    ///////////////////////////////////// by DON
    if (knowledge->isOurNonPlayOnKick())
    {
        //transition to game on

        if ( cyclesWaitAfterballMoved > 6 && selectedPlay->playOnFlag == true)
        {
            wm->gs->transition('s'); //force start
            CKnowledge::State s = CKnowledge::Start;
            knowledge->setGameMode(s);
            knowledge->updateGameState();
        }
    }

    if( knowledge->isTheirNonPlayOnKick() ){
        //transition to game on
        if ( cyclesWaitAfterballMoved > 0 )
        {
            wm->gs->transition('s'); //force start
            CKnowledge::State s = CKnowledge::Start;
            knowledge->setGameMode(s);
            knowledge->updateGameState();
        }
    }
}

void CCoach::updateKnowledgeVars(){
    knowledge->ballPosHistory.prepend(Vector2D(wm->ball->pos.x, wm->ball->pos.y));
    if(knowledge->ballPosHistory.count() > 7)
        knowledge->ballPosHistory.removeLast();

    knowledge->updateGameState();
    checkTransitionToForceStart();

    //	knowledge->frameCount++;
    knowledge->ownerShipCalculated = false;
}

void CCoach::clearIntentions(){
    knowledge->agentsWithIntention.clear();
    QQueue<int> ids = wm->our.data->activeAgents;
    for( int i=0 ; i<ids.count() ; i++ )
    {
        CAgent *agnt = knowledge->getAgent(ids.at(i));
        agnt->intention = NULL;
    }
}

void CCoach::doIntention(){
    knowledge->agentsWithIntention.clear();
    QQueue<int> ids = wm->our.data->activeAgents;
    for( int i=0 ; i<ids.count() ; i++ )
    {
        CAgent *agnt = knowledge->getAgent(ids.at(i));
        if( agnt->intention != NULL && agnt->intention->finished() == false )
        {
            knowledge->agentsWithIntention.append(ids.at(i));
        }
    }
}

void CCoach::decidePreferedDefenseAgentsCountAndGoalieAgent() {
    missMatchIds.clear();
    if(first)
    {
        if(wm->our.activeAgentsCount())
        {
            robotsIdHist.clear();
            for(int i = 0 ; i < wm->our.activeAgentsCount() ; i++)
            {
                robotsIdHist.append(wm->our.active(i)->id);
            }
            first = false;
        }
    }

    if(knowledge->isStop() || knowledge->getGameState() == CKnowledge::Halt)
    {
        if(wm->our.activeAgentsCount())
        {
            robotsIdHist.clear();
            for(int i = 0 ; i < wm->our.activeAgentsCount() ; i++)
            {
                robotsIdHist.append(wm->our.active(i)->id);
            }
        }

    }
    if(wm->our.activeAgentsCount() > 6)
    {
        missMatchIds.clear();
        for(int i = 0 ; i < wm->our.activeAgentsCount() ; i++)
        {
            for(int k = 0 ; k < robotsIdHist.count() ; k++)
            {
                if(robotsIdHist.at(k) == wm->our.active(i)->id)
                {
                    break;
                }
                if(k == robotsIdHist.count() - 1)
                {
                    missMatchIds.append(wm->our.active(i)->id);
                }
            }
        }
    }

    int agentsCount = wm->our.data->activeAgents.count();
    if (goalieAgent != NULL) {
        if (goalieAgent->isVisible()) {
            agentsCount--;
        }
    }

    if (policy()->Formation_GoalieFromGUI()) {
        preferedGoalieAgent = policy()->Formation_Goalie();
    } else {
        preferedGoalieAgent = wm->our.data->goalieID;
    }

    // handle stop
//    if (wm->ball->pos.x < 0){
//        preferedDefenseCounts = agentsCount - 1;
//    }
//    else if (wm->ball->pos.x > 1){
        preferedDefenseCounts = policy() -> Formation_Defense();
//    }

    if(!policy()->Formation_StrictFormation() || !knowledge->isStart()){
        bool oppsAttack = false;

        for(int i = 0 ; i < wm->opp.activeAgentsCount() ; i++) {
            if(wm->opp.active(i)->pos.x <= 0) oppsAttack = true;
        }

        // TODO : should be fixed :)
        if( knowledge->isStart() && transientFlag == false) {
            if (agentsCount == 1) {
                preferedDefenseCounts = 0; // just one playmake
            } else if (agentsCount == 2) {
                preferedDefenseCounts = 1; // one playmake and one defense
            } else {
                if (!oppsAttack || checkOverdef()) {
                    preferedDefenseCounts = 1;
                } else {
                    preferedDefenseCounts = 2;
                }
            }



        } else if( knowledge->isOurNonPlayOnKick()){
            preferedDefenseCounts = 0;

        } else if( knowledge->isTheirNonPlayOnKick() && knowledge->getGameState() != CKnowledge::TheirKickOff) {
            preferedDefenseCounts = max(agentsCount - 1 - missMatchIds.count(), 0);
        } else if (transientFlag
                   &&  knowledge->getGameState() != CKnowledge::TheirKickOff) {
            if (trasientTimeOut.elapsed() > 1000 && !wm->field->isInOurPenaltyArea(wm->ball->pos)) {
                preferedDefenseCounts = min(0, agentsCount - missMatchIds.count() - 1);

            } else {
                preferedDefenseCounts = agentsCount - missMatchIds.count();

            }
            debug("[coach] harchi robot mobat darim rikhtim tu defa", D_MAHI);
        }
        else if(!knowledge->isStop())
        {
            if(checkOverdef()){
                preferedDefenseCounts = 1;
            }
            else{
                preferedDefenseCounts = 2;
            }

        }
    }

    if(knowledge->isOurNonPlayOnKick() && wm->ball->pos.x < -0.5){
        preferedDefenseCounts = 2;
    }
    if(policy()->Formation_StrictFormation()){
        preferedDefenseCounts = policy()->Formation_Defense();
    }
    if(agentsCount == 2){
        preferedDefenseCounts = 2;
    }
    if(policy()->Formation_StrictFormation()){
        preferedDefenseCounts = policy()->Formation_Defense();
    }
    lastPreferredDefenseCounts = preferedDefenseCounts;
    if(knowledge->getGameState()== CKnowledge::HalfTimeLineUp){
        preferedGoalieAgent = -1;
        preferedDefenseCounts = 0;
    }

    if (wm->gs->penalty_shootout())
        preferedDefenseCounts = 0;
    //    if (knowledge->getGameState() == CKnowledge::OurIndirectKick)
    //        preferedGoalieAgent = -1;

}

void CCoach::calcDesiredMarkCounts()
{

    int agentsCount = wm->our.data->activeAgents.count();
    if (goalieAgent != NULL) {
        if (goalieAgent->isVisible()) {
            agentsCount--;
        }
    }

    // used in playoff without counting goalie

    if( agentsCount == 2 ) {
        knowledge->desiredDefCount = 0;
    } else if( agentsCount == 3 ) {

        if(knowledge->toBeMopps.count() == 0) {
            knowledge->desiredDefCount = 1;
        } else if(knowledge->toBeMopps.count() <=2 ) {
            knowledge->desiredDefCount = 2 - knowledge->toBeMopps.count();
        } else {
            knowledge->desiredDefCount = 0;
        }


    } else if( agentsCount == 4  ){


        if(knowledge->toBeMopps.count() == 0)
            knowledge->desiredDefCount = 2;
        else if(knowledge->toBeMopps.count() <=2 )
        {
            knowledge->desiredDefCount = 2 - knowledge->toBeMopps.count();
        }
        else
        {
            knowledge->desiredDefCount = 0;
        }

    } else if( agentsCount == 5 ){

        if(knowledge->toBeMopps.count() == 0)
            knowledge->desiredDefCount = 2;
        else if(knowledge->toBeMopps.count() <=3 )
        {
            knowledge->desiredDefCount = 3 - knowledge->toBeMopps.count();
        }
        else
        {
            knowledge->desiredDefCount = 0;
        }

    }

    else if( agentsCount == 6 ) {
        if(knowledge->toBeMopps.count() <= 1)
            knowledge->desiredDefCount = 1;
        else if(knowledge->toBeMopps.count() <=3 )
        {
            knowledge->desiredDefCount = 3 - knowledge->toBeMopps.count();
        }
        else
        {
            knowledge->desiredDefCount = 0;
        }
    }

}


void CCoach::assignGoalieAgent( int goalieID ){
    QQueue<int> ids = wm->our.data->activeAgents;
    goalieAgent = NULL;
    if( ids.contains(goalieID) ){
        goalieAgent = knowledge->getAgent(goalieID);
    }
}
CKnowledge::ballPossesionState CCoach::isBallOurs()
{
    int oppNearestToBall = -1, ourNearestToBall = -1;
    double oppNearestToBallDist = 100000, ourNearestToBallDist = 100000;
    double oppIntersetMin       = 100000, ourIntersetMin       = 100000;
    Vector2D  ballPos = wm->ball->pos;
    Vector2D  ballVel = wm->ball->vel;
    Segment2D ballPath(ballPos, ballPos + ballVel * 2);
    Segment2D oppPath;
    Vector2D dummy1, dummy2;
    int oppIntersectAgent = -1, ourIntersectAgent = -1;
    CKnowledge::ballPossesionState decidePState;
    QList <CAgent*> ourAgents;
    QList <CRobot*> oppAgents;

    ////////////////// our
    ourAgents = knowledge->getActiveAgents();
    if(goalieAgent != NULL) {
        ourAgents.removeOne(knowledge->goalie);
    }

    for(int i = 0 ; i<knowledge->defenseAgents.count() ; i++){
        ourAgents.removeOne(knowledge->defenseAgents[i]);
    }

    for(int i = 0 ; i < ourAgents.count() ; i++) {
        double ourDist = ourAgents[i]->pos().dist(ballPos);
        if(ourDist < ourNearestToBallDist) {
            ourNearestToBallDist = ourDist;
            ourNearestToBall = ourAgents[i]->id();
        }
        draw(ourAgents[i]->pos(),1,QColor(Qt::black));

        if(wm->ball->vel.length() > 0.5){
            if(Circle2D(ourAgents[i]->pos() , 0.12).intersection(ballPath,&dummy1,&dummy2)) {
                ourDist = ourAgents[i]->pos().dist(ballPos);
                if(ourDist < ourIntersetMin) {
                    ourIntersetMin    = ourDist;
                    ourIntersectAgent = ourAgents[i]->id();
                }
            }
        }
    }

    ////////////////// opp
    oppAgents.clear();
    for(int i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
        oppAgents.append(wm->opp.active(i));
    }

    for (int i = 0 ; i < oppAgents.count() ; i++) {
        if (oppAgents.at(i)->vel.length() < 0.5) {
            double oppDist = oppAgents[i]->pos.dist(ballPos);
            if(oppDist < oppNearestToBallDist){
                oppNearestToBallDist = oppDist;
                oppNearestToBall = oppAgents[i]->id;
            }

            if(wm->ball->vel.length() > 0.5){
                if(Circle2D(oppAgents[i]->pos , 0.12).intersection(ballPath,&dummy1,&dummy2)) {
                    if(oppAgents[i]->pos.dist(ballPos) < oppIntersetMin) {
                        oppIntersetMin    = oppAgents[i]->pos.dist(ballPos);
                        oppIntersectAgent = oppAgents[i]->id;
                    }
                }
            }

        } else {
            oppPath.assign(oppAgents[i]->pos, oppAgents[i]->pos + oppAgents[i]->vel);
            double oppDist = oppPath.nearestPoint(ballPos).dist(ballPos);
            if(oppDist < oppNearestToBallDist){
                oppNearestToBall = oppAgents[i]->id;
                oppNearestToBallDist = oppDist;
            }

            if(wm->ball->vel.length() > 0.5){
                if(oppPath.intersection(ballPath).isValid()){
                    oppDist = oppAgents[i]->pos.dist(ballPos);
                    if(oppDist < oppIntersetMin){
                        oppIntersetMin = oppDist;
                        oppIntersectAgent = oppAgents[i]->id;
                    }
                }
            }
        }
    }

    ///////////////////////////
    ///////////////////////////
    ///
    ///

    if(oppNearestToBall == -1 || (wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->ball->vel.length() < 0.5)) {
        decidePState =  CKnowledge::WEHAVETHEBALL;
    } else if(oppIntersectAgent != -1 && ourIntersectAgent == -1 && wm->ball->vel.length() > 1) {
        decidePState = CKnowledge::WEDONTHAVETHEBALL;
    } else if(oppIntersectAgent != -1 && ourIntersectAgent != -1 && (ourIntersetMin > oppIntersetMin - 0.1) && wm->ball->vel.length() > 0.5) {
        decidePState = CKnowledge::WEDONTHAVETHEBALL;
    } else if(oppNearestToBallDist <= ourNearestToBallDist) {
        decidePState = CKnowledge::WEDONTHAVETHEBALL;
    } else if(ourNearestToBallDist < 0.3 && oppNearestToBallDist > ourNearestToBallDist){
        decidePState = CKnowledge::WEHAVETHEBALL;
    } else if(oppNearestToBallDist > ourNearestToBallDist + 0.5){
        decidePState = CKnowledge::WEHAVETHEBALL;
    } else if(ourNearestToBallDist < oppNearestToBallDist && ballPos.x < 0.1){
        decidePState = CKnowledge::SOSOTHEIR;
    } else if(ourNearestToBallDist < oppNearestToBallDist && ballPos.x >= 0.1){
        decidePState = CKnowledge::SOSOOUR;
    } else if(oppIntersectAgent == -1 && ourIntersectAgent != -1 && wm->ball->vel.length() > 0.7) {
        decidePState = CKnowledge::WEHAVETHEBALL;
    } else {
        decidePState = CKnowledge::SOSOTHEIR;
    }

    if (decidePState == CKnowledge::WEHAVETHEBALL) {
        playOnExecTime.restart();
    }

    if(lastBallPossesionState == CKnowledge::WEHAVETHEBALL && (decidePState == CKnowledge::WEDONTHAVETHEBALL || decidePState == CKnowledge::SOSOTHEIR || decidePState == CKnowledge::SOSOOUR) && playOnExecTime.elapsed() < playOnTime) {
        decidePState = CKnowledge::WEHAVETHEBALL;
    }



    if(oppNearestToBall >= 0) {
        Circle2D oppNearestDribblerArea(wm->opp[oppNearestToBall]->pos + wm->opp[oppNearestToBall]->dir.norm()*0.1 , 0.15);
        draw(oppNearestDribblerArea,QColor(Qt::red));
        if(oppNearestDribblerArea.contains(ballPos) && ourNearestToBallDist > 0.3) {
            decidePState = CKnowledge::WEDONTHAVETHEBALL;
        } else if( oppNearestDribblerArea.contains(ballPos) ) {
            if(wm->ball->pos.x >= 0.1) {
                decidePState = CKnowledge::SOSOOUR;
            } else {
                decidePState = CKnowledge::SOSOTHEIR;
            }
        }
    }

    if(decidePState != lastBallPossesionState) {
        if(intentionTimePossession.elapsed() > possessionIntentionInterval) {
            intentionTimePossession.restart();
        } else {
            decidePState = lastBallPossesionState;
        }
    }


    ////////////      ///////////
    ////////////      ///////////
    //// NEW BALL POSSESSION ////
    double temp = wm->ball->pos.x + wm->ball->vel.x * 1;

    if(temp > 0.5) {
        decidePState = CKnowledge::WEHAVETHEBALL;
    } else if (temp < 0.1){
        decidePState = CKnowledge::WEDONTHAVETHEBALL;
    } else {
        decidePState = lastBallPossesionState;
    }

    if (wm->field->isInOurPenaltyArea(wm->ball->pos)
        &&  wm->ball->vel.length() < 0.1) {
        decidePState = CKnowledge::SOSOTHEIR;
    }

    if (wm->field->isInOppPenaltyArea(wm->ball->pos)
        && wm->ball->vel.length() < 0.1) {
        decidePState = CKnowledge::SOSOOUR;
    }

    lastBallPossesionState = decidePState;
    analyze("ball Possesion",decidePState,true);

    return decidePState;
}


// TODO : fix this
QList<int> CCoach::findBestPoses(int numberOfPositionAgents)
{
    QList<int> result;
    QList<int> OurAgents;
    QList<CRobot*> OppAgents;
    double posWeights[30] = {};
    int bestPosNum[5] = {};
    double bestPosWeight = 0;

    double dummyDist = 20000;
    int nearestId = 0;

    OurAgents = wm->our.data->activeAgents;
    if( goalieAgent != NULL ){
        OurAgents.removeOne(goalieAgent->self()->id);
    }
    for( int i=0 ; i<defenseAgents.size() ; i++ ) {
        if( OurAgents.contains(defenseAgents.at(i)->self()->id) ) {
            OurAgents.removeOne(defenseAgents.at(i)->self()->id);
        }
    }


    for(int i = 0 ;i<OurAgents.count() ; i++) {
        if(wm->our[OurAgents[i]]->pos.dist(wm->ball->pos + wm->ball->vel) < dummyDist) {
            dummyDist = wm->our[OurAgents[i]]->pos.dist(wm->ball->pos + wm->ball->vel);
            nearestId = OurAgents[i];
        }
    }
    OurAgents.removeOne(nearestId);


    // Add all opp agents to OppAgentsList except goalie
    for(int _i=0;_i < wm->opp.activeAgentsCount();_i++) {
        if(!wm->field->isInOppPenaltyArea(wm->opp.active(_i)->pos)) {
            OppAgents.append(wm->opp.active(_i));
        }
    }


    double minDist = 10000;
    /* TODO : check this code is working like the big commented code that is down of this code*/
    if( numberOfPositionAgents == 0)
        return result;
    int quotient = 6 / numberOfPositionAgents;
    for(int _c = 0 ; _c < numberOfPositionAgents ;_c++) {
        for(int _i = 0 ; _i < 30 ;_i++) {
            if(quotient * _c > _i || quotient * _c < _i) {
                posWeights[_i] = -10000000;
                continue;
            }
            minDist = 100000;
            posWeights[_i] = 0;
            for(int _j = 0 ; _j < OurAgents.count() ; _j++) {
                if(wm->our[OurAgents[_j]]->pos.dist(knowledge->getStaticPoses(_i)) < minDist) {
                    minDist = wm->our[OurAgents[_j]]->pos.dist(knowledge->getStaticPoses(_i));
                }
            }
            if(numberOfPositionAgents > 1) {
                if(minDist <= 0.5)
                    posWeights[_i] +=100;
                if(minDist<= 1)
                    posWeights[_i] +=2;
                else
                    posWeights[_i] -=10000;
                posWeights[_i] -= 4*minDist;
            }
            else {
                posWeights[_i] -= 2*minDist;
            }
            minDist = 100000;
            for(int _j = 0 ; _j < OppAgents.count() ; _j++) {
                if(OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i)) < minDist) {
                    minDist = OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i));
                }
                //                if(OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i)) < 1)
                //                    posWeights[_i] = 0;
            }
            posWeights[_i] += minDist;
            for(int k =0 ; k < _c ; k++)
            {
                if(knowledge->getStaticPoses(bestPosNum[k]).dist(knowledge->getStaticPoses(_i)) < 1.6)
                    posWeights[_i] = -1000;
                if(_i == bestPosNum[k])
                    posWeights[_i]=-1000;
            }
            posWeights[_i] -= 10000.0 / max(0.01, knowledge->getStaticPoses(_i).dist(wm->ball->pos));
            /* if(knowledge->getStaticPoses(_i).dist(wm->ball->pos) < 1) {
                posWeights[_i] = -10000;
            }*/
            if(knowledge->getStaticPoses(_i).dist(wm->our[nearestId]->pos) < 1.5) {
                posWeights[_i]=-1000;
            }
            posWeights[_i] -=knowledge->getStaticPoses(_i).dist(wm->ball->pos);
            if(numberOfPositionAgents == 1)
                posWeights[_i]+= 3* Vector2D::angleOf(wm->ball->pos,wm->field->oppGoal(),knowledge->getStaticPoses(_i)).radian();
            if(wm->field->isInOppPenaltyArea(knowledge->getStaticPoses(_i)) )
                posWeights[_i] = -1000000;
            bestPosWeight = -1000000;
            for(int _i = 0; _i < 30 ;_i++) {
                if(posWeights[_i] >= bestPosWeight)
                {
                    bestPosNum[_c] = _i;
                    bestPosWeight = posWeights[_i];
                }

            }
        }
    }

    /*
    switch(numberOfPositionAgents) {
    case 1:
        for(int _c = 0 ; _c < numberOfPositionAgents ;_c++) {

            for(int _i = 0 ; _i < 30 ;_i++) {

                minDist = 100000;
                posWeights[_i] = 0;
                for(int _j = 0 ; _j < OurAgents.count() ; _j++) {
                    if(wm->our[OurAgents[_j]]->pos.dist(knowledge->getStaticPoses(_i)) < minDist) {
                        minDist = wm->our[OurAgents[_j]]->pos.dist(knowledge->getStaticPoses(_i));
                    }
                }
                posWeights[_i] -= 2*minDist;




                minDist = 100000;
                for(int _j = 0 ; _j < OppAgents.count() ; _j++)
                {
                    if(OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i)) < minDist)
                    {
                        minDist = OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i));
                    }
                    //                if(OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i)) < 1)
                    //                    posWeights[_i] = 0;
                }
                posWeights[_i] += minDist;
                for(int k =0 ; k < _c ; k++)
                {
                    if(knowledge->getStaticPoses(bestPosNum[k]).dist(knowledge->getStaticPoses(_i)) < 1.6)
                        posWeights[_i] = -1000;
                    if(_i == bestPosNum[k])
                        posWeights[_i]=-1000;
                }
                if(knowledge->getStaticPoses(_i).dist(wm->ball->pos) < 1)
                {
                    posWeights[_i]=-1000;
                }
                if(knowledge->getStaticPoses(_i).dist(wm->our[nearestId]->pos) < 1.5)
                {
                    posWeights[_i]=-1000;
                }
                posWeights[_i] -=knowledge->getStaticPoses(_i).dist(wm->ball->pos);




                posWeights[_i]+= 3* Vector2D::angleOf(wm->ball->pos,wm->field->oppGoal(),knowledge->getStaticPoses(_i)).radian();
                if(wm->field->isInOppPenaltyArea(knowledge->getStaticPoses(_i)) )
                    posWeights[_i] = -100000;


            }

            bestPosWeight = -1000000;
            for(int _i = 0; _i < 30 ;_i++)
            {
                if(posWeights[_i] >= bestPosWeight)
                {
                    bestPosNum[_c] = _i;
                    bestPosWeight = posWeights[_i];
                }

            }
        }
        break;
    case 2:
        for(int _c = 0 ; _c < numberOfPositionAgents ;_c++)
        {
            for(int _i = 0 ; _i < 30 ;_i++)
            {
                if((_c == 0 && _i%6 >= 3) || (_c == 1 && _i%6 < 3)){
                    minDist = 100000;
                    posWeights[_i] = 0;
                    for(int _j = 0 ; _j <OurAgents.count() ; _j++)
                    {

                        if(wm->our[OurAgents[_j]]->pos.dist(knowledge->getStaticPoses(_i)) < minDist)
                        {
                            minDist = wm->our[OurAgents[_j]]->pos.dist(knowledge->getStaticPoses(_i));
                        }


                    }

                    if(minDist <= 0.5)
                        posWeights[_i] +=100;
                    if(minDist<= 1)
                        posWeights[_i] +=2;
                    else
                        posWeights[_i] -=100000;


                    posWeights[_i] -= 4*minDist;




                    minDist = 100000;
                    for(int _j = 0 ; _j < OppAgents.count() ; _j++)
                    {
                        if(OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i)) < minDist)
                        {
                            minDist = OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i));
                        }
                        //                if(OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i)) < 1)
                        //                    posWeights[_i] = 0;
                    }
                    posWeights[_i] += minDist;
                    for(int k =0 ; k < _c ; k++)
                    {
                        if(knowledge->getStaticPoses(bestPosNum[k]).dist(knowledge->getStaticPoses(_i)) < 1.6)
                            posWeights[_i] = -1000;
                        if(_i == bestPosNum[k])
                            posWeights[_i]=-1000;
                    }
                    if(knowledge->getStaticPoses(_i).dist(wm->ball->pos) < 1)
                    {
                        posWeights[_i]=-1000;
                    }
                    if(knowledge->getStaticPoses(_i).dist(wm->our[nearestId]->pos) < 1.5)
                    {
                        posWeights[_i]=-1000;
                    }
                    posWeights[_i] -=knowledge->getStaticPoses(_i).dist(wm->ball->pos);




                    //posWeights[_i]+= 3* Vector2D::angleOf(wm->ball->pos,wm->field->oppGoal(),knowledge->getStaticPoses(_i)).radian();
                    if(wm->field->isInOppPenaltyArea(knowledge->getStaticPoses(_i)) )
                        posWeights[_i] = -100000;


                }
                else
                {
                    posWeights[_i] = -100000;
                }
            }

            bestPosWeight = -1000000;
            for(int _i = 0; _i < 30 ;_i++)
            {
                if(posWeights[_i] >= bestPosWeight)
                {
                    bestPosNum[_c] = _i;
                    bestPosWeight = posWeights[_i];
                }

            }
        }

        break;
    case 3:
        for(int _c = 0 ; _c < numberOfPositionAgents ;_c++)
        {

            for(int _i = 0 ; _i < 30 ;_i++)
            {
                if((_c == 2 && _i%6 < 2) || (_c == 1 && (_i%6 >= 2) && (_i%6 < 4)) || (_c == 0 && _i%6 >= 4 )){
                    minDist = 100000;
                    posWeights[_i] = 0;
                    for(int _j = 0 ; _j <OurAgents.count() ; _j++)
                    {

                        if(wm->our[OurAgents[_j]]->pos.dist(knowledge->getStaticPoses(_i)) < minDist)
                        {
                            minDist = wm->our[OurAgents[_j]]->pos.dist(knowledge->getStaticPoses(_i));
                        }


                    }
                    //if(minDist <= 0.8) {
                    if(minDist <= 0.5)
                        posWeights[_i] +=100;
                    if(minDist<= 1)
                        posWeights[_i] +=2;
                    else
                        posWeights[_i] -=100000;
                    posWeights[_i] -= 4*minDist;

                    //                    }
                    //                    else
                    //                    {
                    //                         posWeights[_i] -= minDist;
                    //                    }
                    minDist = 100000;
                    for(int _j = 0 ; _j <OppAgents.count() ; _j++)
                    {
                        if(OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i)) < minDist)
                        {
                            minDist = OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i));
                        }
                        //                if(OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i)) < 1)
                        //                    posWeights[_i] = 0;
                    }
                    posWeights[_i] += minDist;
                    for(int k =0 ; k < _c ; k++)
                    {
                        if(knowledge->getStaticPoses(bestPosNum[k]).dist(knowledge->getStaticPoses(_i)) < 1.6)
                            posWeights[_i] = -1000;
                        if(_i == bestPosNum[k])
                            posWeights[_i]=-1000;
                    }
                    if(knowledge->getStaticPoses(_i).dist(wm->ball->pos) < 1)
                    {
                        posWeights[_i]=-1000;
                    }
                    if(knowledge->getStaticPoses(_i).dist(wm->our[nearestId]->pos) < 1.5)
                    {
                        posWeights[_i]=-1000;
                    }
                    posWeights[_i] -=knowledge->getStaticPoses(_i).dist(wm->ball->pos);


                    //posWeights[_i]+= 3* Vector2D::angleOf(wm->ball->pos,wm->field->oppGoal(),knowledge->getStaticPoses(_i)).radian();
                    if(wm->field->isInOppPenaltyArea(knowledge->getStaticPoses(_i)) )
                        posWeights[_i] = -100000;


                }
                else{
                    posWeights[_i] = -100000;
                }

            }

            bestPosWeight = -1000000;
            for(int _i = 0; _i < 30 ;_i++)
            {
                if(posWeights[_i] >= bestPosWeight)
                {
                    bestPosNum[_c] = _i;
                    bestPosWeight = posWeights[_i];
                }

            }
        }

        break;
    }*/
    for(int _c = 0; _c < numberOfPositionAgents ; _c++) {
        result.append(bestPosNum[_c]);
    }
    return result;
}

void CCoach::assignDefenseAgents(int defenseCount){


    QQueue<int> ids = wm->our.data->activeAgents;
    if( goalieAgent ) {
        ids.removeOne(goalieAgent->id());
    }
    if(playmakeId != -1) {
        ids.removeOne(playmakeId);
    }

    defenses.fillDefencePositionsTo(defenseTargets);
    double nearestDist = 1000000;
    double nearestRobot = -1;

    defenseAgents.clear();
    for(int i = 0 ; i < defenseCount ; i++) {
        nearestDist = 1000000;
        for(int j = 0 ; j < ids.count() ; j++) {
            if (!knowledge->getAgent(ids[j])->changeIsNeeded) {
                if (wm->our[ids[j]]->pos.dist(defenseTargets[i]) < nearestDist) {
                    nearestDist = wm->our[ids[j]]->pos.dist(defenseTargets[i]);
                    nearestRobot =  ids[j];
                }
            }
        }
        if(nearestRobot>=0) {
            defenseAgents.append(knowledge->getAgent(nearestRobot));
            ids.removeOne(nearestRobot);
        }

    }

    if(knowledge->variables["clearing"] == "true")
    {
        defenseAgents = lastDefenseAgents;
    }


    lastDefenseAgents.clear();
    lastDefenseAgents.append(defenseAgents);

    knowledge->defenseAgents.clear();
    knowledge->defenseAgents.append(defenseAgents);
}
bool CCoach::isBallcollide(){
    // TODO : change this :P
    Circle2D dummyCircle;
    Vector2D sol1,sol2;
    Segment2D ballPath(wm->ball->pos,wm->ball->pos+wm->ball->vel);
    debug("ball is colliding" , D_AHZ);
    for(int i = 0 ; i < wm->our.activeAgentsCount() ; i++){
        dummyCircle.assign(wm->our.active(i)->pos,0.08);
        if(dummyCircle.intersection(ballPath,&sol1,&sol2) && wm->our.active(i)->pos.dist(wm->ball->pos) < 0.14 && fabs((wm->ball->vel - lastBallVel).length()) > 0.5){
            lastBallVel = wm->ball->vel;
            return true;
        }
        if(wm->ball->vel.length() < 0.5 && wm->our.active(i)->pos.dist(wm->ball->pos) < 0.13) {
            lastBallVel = wm->ball->vel;
            return true;
        }
    }
    lastBallVel = wm->ball->vel;
    return false;
}

void CCoach::virtualTheirPlayOffState()
{
    CKnowledge::State currentState;
    currentState = knowledge->getGameState();
    if(lastState == CKnowledge::TheirDirectKick || lastState == CKnowledge::TheirIndirectKick /*|| lastState == CKnowledge::TheirKickOff*/) {
        if(currentState == CKnowledge::Start){
            transientFlag = true;
        }
    }

    if(transientFlag == false){
        trasientTimeOut.restart();
    }

    if(trasientTimeOut.elapsed() >= translationTimeOutTime) {
        transientFlag = false;
    }

    if(wm->ball->pos.x >= 1) {
        transientFlag = false;
    }

    if(isBallcollide() ){ // TODO : till we fix function && 0
        // transientFlag = false;
    }
    debug(QString("TS flag: %1").arg(transientFlag) , D_AHZ);
    knowledge->transientFlag = transientFlag;
    lastState  = currentState;

}

void CCoach::decideDefense(){
    assignGoalieAgent(preferedGoalieAgent);
    assignDefenseAgents(preferedDefenseCounts);

    if( knowledge->getGameState() == CKnowledge::TheirPenaltyKick ){
        defenseAgents.clear();
        defenses.initGoalKeeper(goalieAgent);
        defenses.initDefense(defenseAgents);
        defenses.execute();
    } else {
        defenses.initGoalKeeper(goalieAgent);
        defenses.initDefense(defenseAgents);
        defenses.execute();
        //        defenses.debugAgents("Defense");
    }
}


double CCoach::findMostPossible(Vector2D agentPos)
{
    QList<int> tempObstacles;
    QList <Circle2D> obstacles;
    obstacles.clear();
    for(int i = 0 ; i < wm->opp.activeAgentsCount() ; i++)
    {
        obstacles.append(Circle2D(wm->opp.active(i)->pos,0.1));
    }

    for(int i = 0 ; i < wm->our.activeAgentsCount() ; i++)
    {
        if(wm->our.active(i)->id != playmakeId)
            obstacles.append(Circle2D(wm->our.active(i)->pos,0.1));
    }
    double prob,angle,biggestAngle;

    knowledge->getEmptyAngle(agentPos-(wm->field->oppGoal()-agentPos).norm()*0.15,wm->field->oppGoalL(),wm->field->oppGoalR(), obstacles, prob, angle, biggestAngle);


    return prob;
}

void CCoach::updateAttackState()
{
    Polygon2D robotCritArea;
    double    safeRegion = 1;
    double    critLenth = 0.75;
    Circle2D  critCir;
    CAgent    *ourNearestAgent;
    CRobot    *oppNearest;
    if(wm->opp.activeAgentsCount() > 0) {
        oppNearest = wm->opp[knowledge->getNearestOppToPoint(wm->ball->pos)];
    }
    else
    {
        ourAttackState = SAFE;
        return;
    }
    QList<int> ids;
    Segment2D oppNearestPath(oppNearest->pos,oppNearest->pos + oppNearest->vel);
    ids = wm->our.data->activeAgents;
    //    ourNearestAgent = knowledge->getAgent(knowledge->getNearestAgentToPoint(wm->ball->pos,&ids));
    if (playmakeId != -1) {
        CRobot* PMA = wm->our[playmakeId];
        if(PMA != NULL)
        {
            critCir = Circle2D(PMA->pos, critLenth + 0.2);
            robotCritArea.addVertex(PMA->pos);
            robotCritArea.addVertex(PMA->pos + PMA->dir.norm() * critLenth + PMA->dir.norm().rotate(90) * critLenth);
            robotCritArea.addVertex(PMA->pos + PMA->dir.norm() * critLenth + PMA->dir.norm().rotate(-90)* critLenth);
        }
    }


    draw(robotCritArea,QColor(Qt::cyan));

    if(robotCritArea.contains(oppNearest->pos)
       || (ourAttackState == CRITICAL && critCir.contains(oppNearest->pos))) {
        ourAttackState = CRITICAL;
        debug(QString("Attack: critical"),D_MHMMD);
    }
    else if(oppNearestPath.nearestPoint(wm->ball->pos).dist(wm->ball->pos) >= safeRegion) {
        ourAttackState = SAFE;
        debug(QString("Attack: safe"),D_MHMMD);
    }
    else {
        ourAttackState = FAST;
        debug(QString("Attack: fast"),D_MHMMD);
    }

}
void CCoach::choosePlaymakeAndSupporter(bool defenseFirst)
{
    QList<int> ourPlayers = wm->our.data->activeAgents;
    if( ourPlayers.contains(preferedGoalieAgent) ) {
        ourPlayers.removeOne(preferedGoalieAgent);
    }

    if(defenseFirst){
        for( int i=0 ; i < defenseAgents.size() ; i++ ){
            if ( ourPlayers.contains(defenseAgents.at(i)->self()->id) ) {
                ourPlayers.removeOne(defenseAgents.at(i)->self()->id);
            } else {
                debug("[coach] Bad Defense assining", D_ERROR);
            }
        }
    } else {
        if (ourPlayers.size() - preferedDefenseCounts <= 0) {
            playmakeId = -1;
            lastPlayMake = -1;
            return;
        }
    }

    if (ourPlayers.size() == 0) {
        playmakeId = -1;
        lastPlayMake = -1;
        return;
    }

    ////////////////////first we choose our playmake
    // third version
    double ballVel = wm->ball->vel.length();
    Vector2D ballPos = wm->ball->pos;
    if(ballVel < 0.3)
    {
        double maxD = -1000.1;
        for(int i = 0; i < ourPlayers.size(); i++)
        {
            double o = -knowledge->getAgent(ourPlayers[i])->pos().dist(ballPos) ;
            if(ourPlayers[i] == lastPlayMake)
                o += playMakeTh;
            if(o > maxD)
            {
                maxD = o;
                playmakeId = ourPlayers[i];
            }
        }
        lastPlayMake = playmakeId;
    }
    else
    {
        if(playMakeIntention.elapsed() < playMakeIntentionInterval)
        {
            playmakeId = lastPlayMake;
            return;
        }
        else
            playMakeIntention.restart();
        //Vector2D ballVel = wm->ball->vel;
        double nearest[10] = {};
        for(int i = 0; i < ourPlayers.size(); i++)
            nearest[ourPlayers[i]] = CSkillKick::kickTimeEstimation(knowledge->getAgent(ourPlayers[i]), wm->field->oppGoal());
        if(lastPlayMake >= 0 && lastPlayMake <= 9)
            nearest[lastPlayMake] -= 0.2;
        double minT = 1e8;
        for(int i = 0; i < ourPlayers.size(); i++)
        {
            if(nearest[ourPlayers[i]] < minT)
            {
                minT = nearest[ourPlayers[i]];
                playmakeId = ourPlayers[i];
            }
        }
        for(int i = 0; i < ourPlayers.size(); i++)
            debug(QString("timeneeded of %1 is : %2 \n").arg(ourPlayers[i]).arg(nearest[ourPlayers[i]]), D_PARSA);
        lastPlayMake = playmakeId;
    }
    debug(QString("playmake is : %1").arg(playmakeId), D_PARSA);
}

void CCoach::decideAttack()
{
    ballPState = isBallOurs();
    updateAttackState();
    knowledge->ballPossesion = ballPState;

    lastBallPossesionState = ballPState;

    // find unused agents!
    QList<int> ourPlayers = wm->our.data->activeAgents;
    if( goalieAgent != NULL ){
        ourPlayers.removeOne(goalieAgent->self()->id);
    }
    for( int i=0 ; i<defenseAgents.size() ; i++ ){
        if( ourPlayers.contains(defenseAgents.at(i)->self()->id) ){
            ourPlayers.removeOne(defenseAgents.at(i)->self()->id);
        }
    }

    switch (knowledge->getGameState()) { // GAMESTATE

    case CKnowledge::Halt:
        decideHalt(ourPlayers);
        return;
        break;

    case CKnowledge::Stop:
        decideStop(ourPlayers);
        break;

    case CKnowledge::OurKickOff:
        decideOurKickOff(ourPlayers);
        break;

    case CKnowledge::TheirKickOff:
        decideTheirKickOff(ourPlayers);
        break;

    case CKnowledge::OurDirectKick:
        decideOurDirect(ourPlayers);
        break;

    case CKnowledge::TheirDirectKick:
        decideTheirDirect(ourPlayers);
        break;

    case CKnowledge::OurIndirectKick:
        decideOurIndirect(ourPlayers);
        break;

    case CKnowledge::TheirIndirectKick:
        decideTheirIndirect(ourPlayers);
        break;

    case CKnowledge::OurPenaltyKick:
        decideOurPenalty(ourPlayers);
        break;

    case CKnowledge::TheirPenaltyKick:
        decideTheirPenalty(ourPlayers);
        break;
    case CKnowledge::Start:
        decideStart(ourPlayers);
        break;

    case CKnowledge::NormalStart:
        switch (knowledge->getGameMode()) {
        case CKnowledge::OurKickOff:
            decideOurKickOff(ourPlayers);
            break;
        case CKnowledge::TheirKickOff:
            decideTheirKickOff(ourPlayers);
            break;
        case CKnowledge::OurPenaltyKick:

            decideOurPenalty(ourPlayers);
            break;
        case CKnowledge::TheirPenaltyKick:
            decideTheirPenalty(ourPlayers);
            break;
        default:
            decideNormalStart(ourPlayers);
            break;
        }
        break;
    case CKnowledge::OurBallPlacement:
        decideOurBallPlacement(ourPlayers);
        break;
    case CKnowledge::TheirBallPlacement:
        decideStop(ourPlayers);
        break;
    case CKnowledge::HalfTimeLineUp:
        decideHalfTimeLineUp(ourPlayers);
        break;
    default:
        decideNull(ourPlayers);
        return;
        break;
    }

    selectedPlay->init(ourPlayers, &editData);
    selectedPlay->execute();
    lastPlayers.clear();
    lastPlayers.append(ourPlayers);
}

void CCoach::decidePlayOff(QList<int>& _ourplayers, POMODE _mode) {

    //Decide Plan
    firstIsFinished = ourPlayOff->isFirstFinished();
    if (firstTime) {
        NGameOff::EMode tempMode;
        selectPlayOffMode(_ourplayers.size(), tempMode);
        initPlayOffMode(tempMode, _mode, _ourplayers);
        ourPlayOff->setMasterMode(tempMode);
        if ( tempMode == NGameOff::FirstPlay ) {
            if (firstPlay && !firstIsFinished) {
                firstTime = true;

            } else if (firstPlay && firstIsFinished) {
                firstTime = true;
                firstPlay = false;

            } else if (!firstPlay && firstIsFinished) {
                firstTime = false;
                firstPlay = true;
                firstIsFinished = false;
                preferedShotSpot = ourPlayOff->getShotSpot();
                ourPlayOff->resetFirstPlayFinishedFlag();
            }

        } else {
            firstTime = false;

        }

    } else {
        setPlayOff( ourPlayOff->getMasterMode() );
    }
}
void CCoach::decidePlayOn(QList<int>& ourPlayers, QList<int>& lastPlayers) {

    CKnowledge::ballPossesionState ballPState = isBallOurs();

    if(playmakeId > -1 && playmakeId < 12) {
        dynamicAttack->setPlayMake(playmakeId);
        ourPlayers.removeOne(playmakeId);
        debug(QString("playmake : %1").arg(playmakeId),D_MHMMD);
    }

    Circle2D ourDefenseArea(wm->field->ourGoal(), 1.6);

    if (knowledge->variables["clearing"] == "true"
        || (ourDefenseArea.contains(wm->ball->pos) && wm->ball->vel.length() < 1)) {
        if(playmakeId != -1) {
            ourPlayers.append(playmakeId);
            dynamicAttack->setPlayMake(-1);
        }
        dynamicAttack->setDefenseClear(true);
    } else {
        dynamicAttack->setDefenseClear(false);
    }

    if(wm->our[playmakeId] != NULL)
    {
        bool goodForKick = ((wm->ball->pos.dist(wm->field->oppGoal()) < 1.5) || (findMostPossible(wm->our[playmakeId]->pos) > (policy()->DynamicPlay_DirectTrsh() - shotToGoalthr)));
        if(goodForKick)
        {
            dynamicAttack->setDirectShot(true);
            if((findMostPossible(wm->our[playmakeId]->pos) > (policy()->DynamicPlay_DirectTrsh() - shotToGoalthr)))
                shotToGoalthr = policy()->DynamicPlay_DirectTrsh() - 0.2;
        } else {
            dynamicAttack->setDirectShot(false);
            shotToGoalthr = 0;
        }

        analyze("High Prob",goodForKick,true);
    }
    /////////////////////////////////////////////////////////////////////////

    dynamicAttack->setPositions(findBestPoses(ourPlayers.count()));
    dynamicAttack->setWeHaveBall(ballPState   == CKnowledge::WEHAVETHEBALL);
    dynamicAttack->setFast(ourAttackState     == FAST);
    dynamicAttack->setCritical(ourAttackState == CRITICAL);

    //////////////////////////////////////////////assign agents
    bool overdef;
    overdef = checkOverdef();
    double MarkNum = 0;
    if(ballPState == CKnowledge::WEHAVETHEBALL) {
        MarkNum = 0;
    } else if(ballPState == CKnowledge::WEDONTHAVETHEBALL) {
        MarkNum = (overdef) ? 3 : 2;

    } else if(ballPState == CKnowledge::SOSOOUR) {
        MarkNum = 1;

    } else if(ballPState == CKnowledge::SOSOTHEIR) {
        MarkNum = 1;

    }

    selectedPlay->markAgents.clear();
    if(wm->ball->pos.x >= 0
       && selectedPlay->lockAgents
       && lastPlayers.count() == ourPlayers.count()) {
        ourPlayers.clear();
        ourPlayers = lastPlayers;

    } else {
        qSort(ourPlayers.begin(), ourPlayers.end());
        if(ourPlayers.count()) {
            if (MarkNum == 3) {
                selectedPlay->markAgents.append(knowledge->getAgent(ourPlayers.at(0)));
                ourPlayers.removeFirst();
                if (ourPlayers.count()) {
                    selectedPlay->markAgents.append(knowledge->getAgent(ourPlayers.at(0)));
                    ourPlayers.removeFirst();
                }
                if (ourPlayers.count()) {
                    selectedPlay->markAgents.append(knowledge->getAgent(ourPlayers.at(0)));
                    ourPlayers.removeFirst();
                }
            }

            if (MarkNum == 2) {
                selectedPlay->markAgents.append(knowledge->getAgent(ourPlayers.at(0)));
                ourPlayers.removeFirst();
                if (ourPlayers.count()) {
                    selectedPlay->markAgents.append(knowledge->getAgent(ourPlayers.at(0)));
                    ourPlayers.removeFirst();
                }


            } else if (MarkNum == 1) {
                selectedPlay->markAgents.append(knowledge->getAgent(ourPlayers.at(0)));
                ourPlayers.removeFirst();
            }
        }
    }
}

void CCoach::matchPlan(NGameOff::SPlan *_plan, const QList<int>& _ourplayers) {
    MWBM matcher;
    matcher.create(_plan->common.currentSize, _ourplayers.size());
    for (size_t i = 0; i < _plan->common.currentSize; i++) {
        for (size_t j = 0; j < _ourplayers.size(); j++) {

            double weight;
            if (_plan->matching.initPos.agents.at(i).x == -100) {
                weight = knowledge->getAgent(_ourplayers.at(j))->pos().dist(wm->ball->pos);
            } else {
                weight = _plan->matching.initPos.agents.at(i).dist(knowledge->getAgent(_ourplayers.at(j))->pos());
            }
            matcher.setWeight(i, j, -(weight));
        }
    }
    qDebug() << "[Coach] matched plan with : " << matcher.findMatching();
    for (size_t i = 0; i < _plan->common.currentSize;i++) {
        int matchedID = matcher.getMatch(i);
        _plan->common.matchedID.insert(i, _ourplayers.at(matchedID));

    }
    qDebug() << "[Coach] mathched by" << _plan->common.matchedID;
}

void CCoach::checkGUItoRefineMatch(SPlan *_plan, const QList<int>& _ourplayers) {
    if (policy()->PlayOff_IDBasePasser() && _ourplayers.contains(policy()->PlayOff_PasserID())) {
        int temp = _plan->matching.common->matchedID.value(0);
        _plan->matching.common->matchedID[0] = policy()->PlayOff_PasserID();
        for (int i = 1;i < _plan->matching.common->matchedID.size(); i++) {
            if (_plan->matching.common->matchedID[i] == policy()->PlayOff_PasserID()) {
                _plan->matching.common->matchedID[i] = temp;
                break;
            }
        }
    }

    if (policy() -> PlayOff_IDBaseOneToucher()
        && _ourplayers.contains(policy() -> PlayOff_OneToucherID())) {
        int temp = _plan -> matching.common -> matchedID.value(1);
        _plan -> matching.common -> matchedID[1] = policy() -> PlayOff_OneToucherID();
        for (int i = 2;i < _plan->matching.common->matchedID.size(); i++) {
            if (_plan->matching.common->matchedID[i] == policy()->PlayOff_OneToucherID()) {
                _plan->matching.common->matchedID[i] = temp;
                break;
            }
        }
    }

    qDebug() << "[coach] final Match : " << _plan->matching.common->matchedID;
}

ShotSpot CCoach::getShotSpot(const Vector2D &_ball, const Vector2D &_shotPos) {

    Circle2D killZone(wm->field->oppGoal(), 1.6);

    Rect2D nearCenter(_FIELD_WIDTH/2 - 2,
                      _PENALTY_WIDTH/2,
                      2,
                      _PENALTY_WIDTH);

    Rect2D near1(_FIELD_WIDTH/2 - 2,
                 _FIELD_HEIGHT/2,
                 2,
                 (_FIELD_HEIGHT-_PENALTY_WIDTH)/2);

    Rect2D near2(_FIELD_WIDTH/2 - 2,
                 -_PENALTY_WIDTH/2,
                 2,
                 (_FIELD_HEIGHT-_PENALTY_WIDTH)/2);

    Rect2D far1(0,
                _FIELD_HEIGHT/2 - 4,
                _FIELD_WIDTH/2 - 2,
                2);

    Rect2D farcenter(0,
                     _FIELD_HEIGHT/2 - 2,
                     _FIELD_WIDTH/2 - 2,
                     2);

    Rect2D far2(0,
                _FIELD_HEIGHT/2,
                _FIELD_WIDTH/2 - 2,
                2);

    ShotSpot tempSpot = EveryWhere;
    if (killZone.contains(_shotPos)) {
        tempSpot = KillSpot;

    } else if (nearCenter.contains(_shotPos)) {
        tempSpot = CloseNear;

    } else if (near1.contains(_shotPos) || near2.contains(_shotPos)) {
        if (_shotPos.y*_ball.y < 0) {
            tempSpot = FarNear;

        } else {
            tempSpot = CloseNear;

        }
    } else if (farcenter.contains(_shotPos)) {
        tempSpot = FarCenter;

    } else if (far1.contains(_shotPos) || far2.contains(_shotPos)) {
        if (_shotPos.y*_ball.y < 0) {
            tempSpot = FarFar;

        } else {
            tempSpot = CloseFar;

        }
    } else {
        tempSpot = EveryWhere;
    }

    return tempSpot;
}

bool CCoach::isTagsMatched(const QStringList& base, const QStringList& required) {
    Q_FOREACH(QString tag, required)
        if (!base.contains(tag))
            return false;
    return true;
}

bool CCoach::isTagsNearMatched(const QStringList& base, const QStringList& required) {
    Q_FOREACH(QString tag, required)
        if (base.contains(tag))
            return true;
    return false;
}

bool CCoach::isRegionMatched(const Vector2D &_ball, const double& regionRadius) {

    return (wm->ball->pos.dist(_ball) < regionRadius);

}

void CCoach::ShufflePlanIndexing(QList<SPlan*> Plans){
    shuffleSize = 0;
    staticPlayoffPlansShuffleIndexing.clear();

    for(int i=0; i<Plans.size(); i++){
        debug(QString("plan%1 cahnce : %2").arg(i).arg(Plans.at(i)->common.chance) , D_FATEMEH);
        shuffleSize += (int)Plans.at(i)->common.chance;
        for(int j=0; j<(int)Plans.at(i)->common.chance; j++){
            staticPlayoffPlansShuffleIndexing.append(i);
        }
    }

    std::random_shuffle(staticPlayoffPlansShuffleIndexing.begin(), staticPlayoffPlansShuffleIndexing.end());
    std::random_shuffle(staticPlayoffPlansShuffleIndexing.begin(), staticPlayoffPlansShuffleIndexing.end());
    shuffled = true;
}

NGameOff::SPlan* CCoach::chooseMostSuccecfull(const QList<NGameOff::SPlan*>& plans) {
    QList<NGameOff::SPlan*> matchedPlan;

    matchedPlan = getMatchedPlans(preferedShotSpot, plans);

    // Tag Matching old way
    //    matchedPlan = getMatchedPlans(currentTags, plans);


    int bestScore = -1;
    QList<NGameOff::SPlan*> bestPlans;
    Q_FOREACH(NGameOff::SPlan* plan, matchedPlan) {
        if (plan->common.succesRate >= bestScore) {
            bestPlans.clear();
            bestPlans.append(plan);
        }
        else if (plan->common.succesRate == bestScore) {
            bestPlans.append(plan);
        }
    }
    // TODO : remove randomize
    if (bestPlans.isEmpty()) {
        bestPlans.append(matchedPlan);
    }
    return bestPlans[rand()%bestPlans.size()];
}

void CCoach::selectPlayOffMode(int agentSize, NGameOff::EMode &_mode) {     
    if (agentSize < 2) {
        _mode = NGameOff::DynamicPlay;
    } else if (isFastPlay() && false) { // TODO : fastPlay should be completed!
        _mode = NGameOff::FastPlay;

    } else if (knowledge->getGameState() == CKnowledge::OurKickOff
               ||  knowledge->getGameMode()  == CKnowledge::OurKickOff) {
        _mode = NGameOff::StaticPlay;

    } else if (!firstIsFinished && policy()->PlayOff_UseFirstPlay()) {
        _mode = NGameOff::FirstPlay;

    } else if (wm->ball->pos.x > -1) {
        _mode = NGameOff::StaticPlay;

    } else {
        _mode = NGameOff::DynamicPlay;

    }
}

void CCoach::initPlayOffMode(const NGameOff::EMode _mode,
                             const POMODE _gameMode,
                             const QList<int>& _ourplayers) {
    switch(_mode) {
    case NGameOff::StaticPlay:
        initStaticPlay(_gameMode, _ourplayers);
        break;
    case NGameOff::DynamicPlay:
        initDynamicPlay(_ourplayers);
        break;
    case NGameOff::FastPlay:
        initFastPlay(_ourplayers);
        break;
    case NGameOff::FirstPlay:
        initFirstPlay(_ourplayers);
        break;
    default:
        initStaticPlay(_gameMode, _ourplayers);
    }
}

void CCoach::setPlayOff(NGameOff::EMode _mode) {
    switch(_mode) {
    case NGameOff::StaticPlay:
        setStaticPlay();
        break;
    case NGameOff::DynamicPlay:
        setDynamicPlay();
        break;
    case NGameOff::FastPlay:
        setFastPlay();
        break;
    case NGameOff::FirstPlay:
        setFirstPlay();
        break;
    default:
        setStaticPlay();
    }
}

void CCoach::MinChanceOfValidplans(QList<SPlan*> validPlans){

    NGameOff::SPlan *temp = NULL;

    for(int i=0;i<validPlans.size();i++)
    {
        temp = validPlans.at(i);
        if(temp->common.chance != 0 && temp->common.chance <= minChance){
            minChance = temp->common.chance;
            if(temp->common.planRepeat <= minChanceRepeat){
                minChanceRepeat = temp->common.planRepeat;
            }
        }
    }

}

int CCoach::LFUPlan(QList<SPlan*> validPlans){

    NGameOff::SPlan *temp = NULL;
    maxLFU=0;

    for(int i=0;i<validPlans.size();i++)
    {
        temp = validPlans.at(i);
        LFU  = temp->common.chance/(double)(temp->common.planRepeat+1);
        if(maxLFU<LFU){
            LFUPlanID = i;
            maxLFU = LFU;
        }


    }
    validPlans[LFUPlanID]->common.planRepeat++;

    return LFUPlanID;
}

int CCoach::PlayoffShufflePolicy(QList<SPlan*> prevValidPlans, QList<SPlan*> validPlans){

    bool equal = true;
    if(prevValidPlans.size() == validPlans.size()){
        foreach (SPlan* p, validPlans) {
            if(!prevValidPlans.contains(p)){
                equal = false;
                break;
            }
        }
    }
    else{
        equal = false;
    }

    if(!shuffled || !equal){
        ShufflePlanIndexing(validPlans);
        equal = true;
    }

    if (shuffleCounter >= shuffleSize) {
        shuffleCounter = 0;
        shuffled = false;
    }

    debug(QString("chosen plan : %1").arg(staticPlayoffPlansShuffleIndexing.at(shuffleCounter)) , D_FATEMEH);

    shuffleCounter++;

    return staticPlayoffPlansShuffleIndexing.at(shuffleCounter-1);
}

int CCoach::PlayoffLFUPolicy(QList<SPlan*> prevValidPlans, QList<SPlan*> validPlans){

    MinChanceOfValidplans(validPlans);

    if(prevValidPlans == validPlans){
        debug("equal" , D_FATEMEH);
    }
    else
    {
        if(prevValidPlans.size() > 0){
            foreach (SPlan* p, validPlans) {
                if(!prevValidPlans.contains(p)){    // set the new plan's repeat
                    p->common.planRepeat = max(max(0, minChanceRepeat-1), p->common.planRepeat);
                }
            }
        }
    }



    minChanceRepeat = 0;

    return LFUPlan(validPlans);
}

void CCoach::initStaticPlay(const POMODE _mode, const QList<int>& _ourplayers) {

    static QList<SPlan*> validPlans, prevValidPlans;
    NGameOff::SPlan* thePlan = NULL;

    prevValidPlans = validPlans;
    validPlans.clear();

    validPlans = getValidPlans(_mode, _ourplayers);

    if (validPlans.isEmpty()) {
        debug ("[coach] WE DONT HAVE PLAN AT ALL", D_MAHI);
        return;
    }

    /** new plan selector **/
    //        thePlan = chooseMostSuccecfull(validPlans);
    /** new plan selector **/


    /** PLAN SELECTION BASED ON HISTORY **/
    /** PLAN SELECTION BASED ON HISTORY **/


    /** LFU selection **/
    //    thePlan = validPlans.at(PlayoffLFUPolicy(prevValidPlans, validPlans));

    //    debug(QString("ID: %1, chance: %2, repeat: %3, name: %4, %5").arg(LFUPlanID).arg(
    //              thePlan->common.chance).arg(thePlan->common.planRepeat).arg(
    //              thePlan->gui.planFile).arg(thePlan->gui.index[2]), D_FATEMEH);
    /** LFU selection **/



    /** SHUFFLE PLAN SELECTION**/
    thePlan = validPlans[PlayoffShufflePolicy(prevValidPlans, validPlans)];
    /** SHUFFLE PLAN SELECTION**/



    /** COUNTER PLAN SELECTION**/
    /*
    if (staticPlayoffPlansCounter >= validPlans.size()) {
        staticPlayoffPlansCounter = 0;
    }
    thePlan = validPlans[staticPlayoffPlansCounter]; //chooseMostSuccecfull(validPlans); //Choose Best valid Plan
    debug (QString("Plan Number : %1 ==> ").arg(staticPlayoffPlansCounter) + validPlans[staticPlayoffPlansCounter]->gui.planFile, D_DEBUG);
    staticPlayoffPlansCounter++;
    /** COUNTER PLAN SELECTION**/

    matchPlan(thePlan, _ourplayers); //Match The Plan
    checkGUItoRefineMatch(thePlan, _ourplayers);
    ourPlayOff->setMasterPlan(thePlan);
    ourPlayOff->analyseShoot(); // should call after setmasterplan
    ourPlayOff->analysePass();  // should call after setmasterplan
    ourPlayOff->setInitial(true);
    ourPlayOff->lockAgents = true;
    lastPlan = thePlan;
    debug(QString("chosen plan is %1").arg(lastPlan->gui.index[3]), D_MAHI);
}

void CCoach::initDynamicPlay(QList<int> _ourplayers) {

    for (int i = 0; i < 6; i++) {
        if (i >= _ourplayers.size()) {
            ourPlayOff->dynamicMatch[i] = -1;
        } else {
            ourPlayOff->dynamicMatch[i] = _ourplayers.at(i);
        }
    }
    if (_ourplayers.size() < 2) {
        ourPlayOff->dynamicSelect = CHIP;
    } else {
        ourPlayOff->dynamicSelect = KHAFAN;
    }

    ourPlayOff->setInitial(true);
    ourPlayOff->lockAgents = true;

}

void CCoach::initFastPlay(QList<int> _ourplayers) {
    // TODO : Initial Fast Play
}

void CCoach::initFirstPlay(QList<int> _ourplayers) {

    double minDist = _MAX_DIST;
    int minID = -1;
    int minOwner;
    for (int i = 0; i < _ourplayers.size(); i++) {

        int tempID = _ourplayers.at(i);
        double tempDist = knowledge->getAgent(tempID)->distToBall().length();
        if (tempDist < minDist) {
            minDist = tempDist;
            minID = tempID;
            minOwner = i;
        }

        if (i >= _ourplayers.size()) {
            ourPlayOff->dynamicMatch[i] = -1;
        } else {
            ourPlayOff->dynamicMatch[i] = _ourplayers.at(i);
        }
    }
    // fix passer :)
    int tempID = ourPlayOff -> dynamicMatch[0];
    ourPlayOff -> dynamicMatch[0] = minID;
    ourPlayOff -> dynamicMatch[minOwner] = tempID;

    ourPlayOff -> setInitial(true);
    ourPlayOff -> lockAgents = true;

    // TODO : Initial First Play
}

void CCoach::setStaticPlay() {
    // TODO : Complete staticPlay checker
    ourPlayOff->setInitial(false);
}

void CCoach::setDynamicPlay() {
    // TODO : Write Dynamic Play checker
    ourPlayOff->setInitial(false);
}

void CCoach::setFirstPlay() {
    // TODO : Write First Play checker
    ourPlayOff->setInitial(false);
    firstIsFinished = false;
}

void CCoach::setFastPlay() {
    // TODO : Write Fast Play checker

}

void CCoach::saveLFUReapeatData(QList<SPlan*> plans){
    playoffPlanSelectionDataFile.open(/*QIODevice::Truncate | */QIODevice::WriteOnly);
    foreach (SPlan* plan, plans) {
        out << plan->gui.planFile << "\n";
        out << plan->gui.index[2] << "\n";
        out << plan->common.planRepeat << "\n";
    }
    playoffPlanSelectionDataFile.close();
}


void CCoach::LFUInit(QList<NGameOff::SPlan*> allPlans){

    playoffPlanSelectionDataFile.open(QIODevice::ReadOnly);

    QString text;
    QMap<QPair<QString, int> , int> loadLFUData;
    QStringList l;
    QPair<QString, int> p;

    text = out.readAll();
    text.remove(text.size()-1, text.size());
    l = text.split("\n");

    if(text.size() > 0){
        for(int i=0; i<l.size();i+=3){
            p.first = l.at(i);
            p.second = l.at(i+1).toInt();
            loadLFUData.insert(p, l.at(i+2).toInt());
        }
    }

    Q_FOREACH(SPlan* plan, allPlans) {
        p.first = plan->gui.planFile;
        p.second = plan->gui.index[2];
        if(loadLFUData.value(p)){
            plan->common.planRepeat = loadLFUData.value(p);
        } else {
            plan->common.planRepeat = 0;
        }
    }

    playoffPlanSelectionDataFile.close();

    debug("done with LFUinit" , D_FATEMEH);
}

QList<SPlan *> CCoach::getValidPlans(const POMODE _mode, const QList<int>& _ourPlayers) {

    NGameOff::SPlan* nearestPlan = NULL;
    double minDist = _MAX_DIST;
    QList<NGameOff::SPlan*> allPlans = m_planLoader->getPlans(); // Get All of The Plans

    if(firstPlanRepeatInit){    // Initialize Plan Repeat
        LFUInit(allPlans);
        firstPlanRepeatInit = false;
    }

    LFUList = allPlans;

    QList<NGameOff::SPlan*> activePlans;
    Q_FOREACH(SPlan* plan, allPlans) {

        if (plan->gui.active) {
            activePlans.append(plan);
        }
    }

    QList<SPlan*> masterPlans;
    Q_FOREACH(SPlan* plan, activePlans) {
        if (plan->gui.master) {
            masterPlans.append(plan);
        }
    }
    if (!masterPlans.isEmpty()) {
        activePlans.clear();
        activePlans = masterPlans;
    }


    if (activePlans.isEmpty()) {
        debug("There's No Plan", D_ERROR);
        return activePlans;
    }

    int symmetry = 1;
    QList<NGameOff::SPlan*> validPlans;
    Q_FOREACH(NGameOff::SPlan* plan, activePlans) { //Find Valid Plans
        NGameOff::SMatching& matching = plan->matching;

        if (matching.common->planMode  >= _mode
            && matching.common->agentSize >= _ourPlayers.size()
            && matching.common->chance > 0
            && matching.common->lastDist >= 0) {

            //check Ball matchig with symmetry
            plan->common.currentSize = _ourPlayers.size();
            Vector2D symBall = Vector2D(matching.initPos.ball.x,
                                        (-1) * matching.initPos.ball.y);

            double tempDist    = wm->ball->pos.dist(matching.initPos.ball);
            double tempSymDist = wm->ball->pos.dist(symBall);

            if (tempDist < minDist) {
                minDist  = tempDist;
                symmetry = 1;
                nearestPlan = plan;
            }

            if (tempSymDist < minDist) {
                minDist  = tempSymDist;
                symmetry = -1;
                nearestPlan = plan;
            }

            if (isRegionMatched(matching.initPos.ball)) {
                plan->execution.symmetry = 1;
            } else if (isRegionMatched(symBall)) {
                plan->execution.symmetry = -1;
            } else {
                continue;
            }

            validPlans.append(plan);
        }
    }

    debug(QString("playoff -> there's %1 valid Plan").arg(validPlans.size()), D_DEBUG);
    if (validPlans.isEmpty()) {
        debug("[Warning] playoff -> there's no valid Plan", D_ERROR, QColor(Qt::red));
        debug("[Warning] playoff -> matching nearset plan", D_ERROR, QColor(Qt::red));
        if (nearestPlan != NULL) {
            nearestPlan->execution.symmetry = symmetry;
            validPlans.append(nearestPlan);
            debug("[Warning] playoff -> nearset plan matched", D_ERROR, QColor(Qt::red));
        }
    }

    return validPlans;
}


void CCoach::execute()
{  
    //draw penalty area
    draw(Circle2D(Vector2D(_PENALTY_AREA_CIRCLE_X , 0) , _PENALTY_AREA_CIRCLE_RAD) , 0 , 360 , "red");

    // reset idle state!
    for (int i = 0; i < _MAX_NUM_PLAYERS ; i++) {
        agents[i]->idle = false;
    }

    QTime timer;
    timer.start();

    // place your reset codes about knowledge vars in this function
    knowledge->resetEssentialVars();


    updateKnowledgeVars();

    doIntention();

    virtualTheirPlayOffState();
    decidePreferedDefenseAgentsCountAndGoalieAgent();
    debug(QString("TS : %1").arg(transientFlag), D_GAME);
    //    draw(QString("TS : %1").arg(transientFlag), Vector2D(2,-3));
    /////////////////////////////////////// choose playmake
    double critAreaRadius = 1.6;
    Circle2D critArea(wm->field->ourGoal(), critAreaRadius);
    playmakeId = -1;
    if((critArea.contains(wm->ball->pos) && wm->field->isInField(wm->ball->pos))) {
        decideDefense();
//        choosePlaymakeAndSupporter(true);
    } else {
//        choosePlaymakeAndSupporter(false);
        decideDefense();
    }

    ////////////////////////////////////////////

    decideAttack();

    // checks whether the goalie is under the net or not if it is moves out
    checkGoalieInsight();
    // Old Role Base Execution -- used for block, old_playmaker
    checkRoleAssignments();
    for (int i=0;i<_NUM_PLAYERS;i++) {
        if (agents[i]->isVisible() && agents[i]->idle == false) {
            if (agents[i]->skill != NULL) {
                agents[i]->skill->execute();
            }
        }
    }

    saveGoalie(); //if goalie is trapped under goal net , move it forward to be seen by the vision again
}

void CCoach::checkRoleAssignments()
{
    knowledge->roleAssignments.clear();

    for_visible_agents(agents, i)
    {
        knowledge->roleAssignments[agents[i]->skillName].append(agents[i]);
    }

    //////////////// Matching for marker agents to mark better! /////////////////
    CRoleMarkInfo *markInfo = (CRoleMarkInfo*) CSkills::getInfo("mark");
    markInfo->matching();
    /////////////////////////////////////////////////////////////////////////////
}

DefensePlan& CCoach::getDefense() {return defenses;}

void CCoach::savePostAssignment()
{
    QFile file("posts");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QStringList s1, s2;
    for (int i=0;i<knowledge->defenseAgents.size();i++)
    {
        s1.append(QString("%1").arg(knowledge->defenseAgents.at(i)->id()));
    }
    //	for (int i=0;i<knowledge->currentFormation.second.count();i++)
    //	{
    //		s2.append(QString("%1").arg(knowledge->currentFormation.second.at(i)));
    //	}
    QTextStream out(&file);
    out << QString(s1.join(",")/*+QString(";")+s2.join(",")*/) << "\n";
}

void CCoach::loadPostAssignment()
{
    QFile file("posts");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        str = str.trimmed();
        QStringList q = str.split(';');
        //		if (q.length() == 2)
        //		{
        //			QStringList q1 = q[0].split(',');
        //			QStringList q2 = q[1].split(',');
        //			lastAssignCycle = knowledge->frameCount;
        //			lastAssign.first.clear();
        //			for (int i=0;i<q1.count();i++)
        //				lastAssign.first.append(knowledge->getAgent(q1[i].toInt()));
        //			lastAssign.second.clear();
        //			for (int i=0;i<q2.count();i++)
        //				lastAssign.second.append(knowledge->getAgent(q2[i].toInt()));
        //		}
        if (q.length() == 1)
        {
            QStringList q1 = q[0].split(',');
            for (int i=0;i<q1.count();i++){
                int ID = q1[i].toInt();
                CAgent *agnt = knowledge->getAgent(ID);
                if( agnt != NULL ){
                    agnt->defIntent.assign(ID , knowledge->frameCount);
                    agnt->intention = &agnt->defIntent;
                    debug(QString("Loaded defense: %1").arg(ID) , D_MASOOD);
                }
            }
        }
    }
}

CPlayOff* CCoach::playOff() {
    return this->ourPlayOff;
}

CLoadPlayOffJson* CCoach::getPlanLoader() {
    return m_planLoader;
}

void CCoach::decideHalt(QList<int>& _ourPlayers) {
    firstTime = true;
    cyclesWaitAfterballMoved = 0;
    clearIntentions();
    _ourPlayers.clear();
    _ourPlayers.append(wm->our.data->activeAgents);
    for( int i = 0 ; i < _ourPlayers.count() ; i++ )
    {
        agents[_ourPlayers[i]]->waitHere();
    }
    knowledge->setLastPlayExecuted(HaltPlay);

    if(!ourPlayOff->deleted)
    {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }

}

void CCoach::decideStop(QList<int> & _ourPlayers) {
    CMasterPlay::position.reset();
    firstTime = true;
    firstPlay = true;
    cyclesWaitAfterballMoved = 0;
    lastPlayMake = -1;
    clearIntentions();
    knowledge->setLastPlayExecuted(StopPlay);
    if(!ourPlayOff->deleted)
    {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }

    QList<int> tempAgents;
    for (int i = 0; i < _ourPlayers.size(); i++) {
        CAgent* tempAgent = knowledge->getAgent(_ourPlayers.at(i));
        if (!tempAgent->changeIsNeeded) {
            stopRoles[i]->assign(knowledge->getAgent(_ourPlayers.at(i)));
        } else {
            tempAgents.append(tempAgent->id());
        }
    }

    _ourPlayers.clear();
    _ourPlayers.append(tempAgents);
    selectedPlay = stopPlay;
}

void CCoach::decideOurKickOff(QList<int> &_ourPlayers) {
    if(ourPlayOff->deleted)
    {
        ourPlayOff->deleted = false;
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, KICKOFF);
    debug(QString("ourplayers : %1").arg(_ourPlayers.size()),D_MAHI);

}

void CCoach::decideTheirKickOff(QList<int> &_ourPlayers) {
    selectedPlay = theirKickOff;
    firstTime = true;
}

void CCoach::decideOurDirect(QList<int> &_ourPlayers) {
    if(ourPlayOff->deleted)
    {
        ourPlayOff->deleted = false;
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, DIRECT);
    debug(QString("ourplayers : %1").arg(_ourPlayers.size()), D_MAHI);

}

void CCoach::decideTheirDirect(QList<int> &_ourPlayers) {
    selectedPlay = theirDirect;
    firstTime = true;
}

void CCoach::decideOurIndirect(QList<int> &_ourPlayers) {
    if(ourPlayOff->deleted)
    {
        ourPlayOff->deleted = false;
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, INDIRECT);
    debug(QString("ourplayers : %1").arg(_ourPlayers.size()),D_MAHI);

}

void CCoach::decideTheirIndirect(QList<int> &_ourPlayers) {
    selectedPlay = theirIndirect;
    firstTime = true;
}

void CCoach::decideOurPenalty(QList<int> &_ourPlayers) {
    selectedPlay = ourPenalty;
    debug("penalty",D_MHMMD);
    firstTime = true;
}

void CCoach::decideTheirPenalty(QList<int> &_ourPlayers) {
    selectedPlay = theirPenalty;
    firstTime = true;
}

void CCoach::decideStart(QList<int> &_ourPlayers) {
    selectedPlay = dynamicAttack;
    decidePlayOn(_ourPlayers, lastPlayers);
}

void CCoach::decideNormalStart(QList<int> &_ourPlayers) {
    selectedPlay = ourPlayOff;
    firstTime = true;
    if(!ourPlayOff->deleted)
    {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }
}

void CCoach::decideOurBallPlacement(QList<int> &_ourPlayers) {
    selectedPlay = ourBallPlacement;
}

void CCoach::decideTheirBallPlacement(QList<int> &_ourPlayers) {
    selectedPlay = theirBallPlacement;
}

void CCoach::decideHalfTimeLineUp(QList<int> &_ourPlayers) {
    selectedPlay = halfTimeLineup;
}


void CCoach::decideNull(QList<int> &_ourPlayers) {
    selectedPlay->markAgents.clear();
    firstTime = true;
    if(!ourPlayOff->deleted)
    {
        ourPlayOff->reset();
        ourPlayOff->deleted = true;
    }
    debug(QString("Unexpected Game State: %1 %2").arg(knowledge->stateToString(knowledge->getGameState())).arg(knowledge->getGameState()) , D_ERROR , "red");
}

bool CCoach::isFastPlay() {
    if (policy()->PlayOff_UseFastPlay()) {
        return true; // TODO : fix this by considering that opp agents
    }
}

QList<SPlan*> CCoach::getMatchedPlans(const QStringList &_tags, const QList<SPlan*>& _plans) {
    QList<SPlan*> tempPlans;

    // FULL MATCH
    Q_FOREACH(SPlan* plan, _plans) {
        if (isTagsMatched(plan->common.tags, _tags)) {
            tempPlans.append(plan);
        }
    }

    // JUST MATCH
    if (tempPlans.isEmpty()) {
        Q_FOREACH(SPlan* plan, _plans) {
            if (isTagsNearMatched(plan->common.tags, _tags)) {
                tempPlans.append(plan);
            }
        }
    }

    //NO MATCH
    if (tempPlans.isEmpty()) return _plans;
    else                     return tempPlans;

}

QList<SPlan*> CCoach::getMatchedPlans(int _shotSpot, const QList<SPlan*>& _plans) {
    QList<SPlan*> tempPlans;
    Q_FOREACH(SPlan* plan, _plans) {
        Vector2D& tempPos  = plan->matching.shotPos;
        Vector2D& tempBall = plan->matching.initPos.ball;
        ShotSpot tempSpot  = getShotSpot(tempBall, tempPos);

        if (_shotSpot & tempSpot || 1) {
            tempPlans.append(plan);
        }

    }

    //NO MATCH
    if (tempPlans.isEmpty() || _shotSpot == EveryWhere) return _plans;
    else                                                return tempPlans;

}

///HMD
bool CCoach::checkOverdef(){
    if((Vector2D::angleOf(wm->ball->pos,wm->field->ourGoal(),wm->field->ourCornerL()).abs() < 20 + overDefThr
        ||Vector2D::angleOf(wm->ball->pos,wm->field->ourGoal(),wm->field->ourCornerR()).abs() < 20 + overDefThr)
       && !Circle2D((wm->field->ourGoal() - Vector2D(0.2,0)),1.60).contains(wm->ball->pos)) {
        overDefThr = 5;
        return true;
    }
    else{
        overDefThr = 0;
        return false;
    }
}

void CCoach::checkSensorShootFault() {
    QList<int> ourPlayers = wm->our.data->activeAgents;
    for (size_t i = 0; i < 12; i++) {
        if (ourPlayers.contains(i)) {
            CAgent* tempAgent = knowledge->getAgent(i);
            if (tempAgent->shootSensor()
                &&  wm->ball->pos.dist(tempAgent->pos() + tempAgent->dir().norm()*0.08) > 0.2) {
                faultDetectionCounter[i]++;

            } else {
                faultDetectionCounter[i] = 0;
            }
        } else {
            faultDetectionCounter[i] = 0;
        }
    }

    for (size_t i = 0; i < 12; i++) {
        if ( faultDetectionCounter[i] > 300 || knowledge->getAgent(i)->changeIsNeeded) {
            knowledge->getAgent(i)->changeIsNeeded = true;
        }
    }

    for (size_t i = 0; i < ourPlayers.size(); i++) {
        if (knowledge->getAgent(i)->changeIsNeeded) {
            debug(QString("[ROBOT FAULT] %1"), D_ERROR);

        }

    }

}

