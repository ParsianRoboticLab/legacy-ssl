#include "coach.h"
#include <QStringList>
#include "gamelogger.h"
#include "ui/varswidget.h"
#include "passevaluation.h"


#include <conditions.h>
#include <playmake.h>
#include <exceptions.h>

#include <QApplication>

QMap<QString, EditData*> CCoach::editData;

CCoach::CCoach(CAgent**_agents)
{
    goalieTrappedUnderGoalNet = false;
    inited = false;
    agents = _agents;
    lastSelected = -1;
    lastAssignCycle = -10;

    ///////////////////////////////////
    goalieTimer.start();
    ////////////////////intentions
    playOnExecTime.start();
    intentionTimePossession.start();
    playMakeIntention.start();

    possessionIntentionInterval = 200;
    playOnTime = 2000;
    playMakeIntentionInterval = 1000;


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


    ourKickOff    = new COurKickOff;
    ourIndirect   = new COurIndirect;
    ourDirect     = new CDoubleSizeOurDirect;
    ourPenalty    = new COurPenalty;
    ourBallPlacement = new COurBallPlacement;
    theirKickOff  = new CTheirKickOff;
    theirIndirect = new CTheirIndirect;
    theirDirect   = new CTheirDirect;
    theirPenalty  = new CTheirPenalty;
    theirBallPlacement = new CTheirBallPlacement;
    forceStart    = new CForceStart;

    ourPlayOff    = NULL;
    dynamicAttack = new CDynamicAttack();

    for( int i=0 ; i<_MAX_NUM_PLAYERS ; i++ ){
        stopRoles[i] = new CRoleStop(knowledge->getAgent(i));
    }

    lastDefenseAgents.clear();

    defenseTimeForVisionProblem[0].start();
    defenseTimeForVisionProblem[1].start();
    translationFlag = false;
    translateTimeOut.start();
    translationTimeOutTime =  1000;
    exeptionPlayMake = NULL;
    exeptionPlayMakeThr = 0;


    m_planLoader = new CLoadPlayOffJson(QDir::currentPath() + QString("/playoff"));

}

CCoach::~CCoach()
{
    savePostAssignment();
}

void CCoach::saveGoalie()
{
    debug((QString("sag shode timer :%1").arg(goalieTimer.elapsed())),D_MHMMD);
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
                if (!wm->our.t->activeAgents.contains(knowledge->goalie->self()->id))
                    wm->our.t->activeAgents.append(knowledge->goalie->self()->id);
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
    ///////////////////////////////////// f**d up by DON
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
    QQueue<int> ids = wm->our.t->activeAgents;
    for( int i=0 ; i<ids.count() ; i++ )
    {
        CAgent *agnt = knowledge->getAgent(ids.at(i));
        agnt->intention = NULL;
    }
}

void CCoach::doIntention(){
    knowledge->agentsWithIntention.clear();
    QQueue<int> ids = wm->our.t->activeAgents;
    for( int i=0 ; i<ids.count() ; i++ )
    {
        CAgent *agnt = knowledge->getAgent(ids.at(i));
        if( agnt->intention != NULL && agnt->intention->finished() == false )
        {
            knowledge->agentsWithIntention.append(ids.at(i));
        }
    }
}

void CCoach::decidePreferedDefenseAgentsCountAndGoalieAgent(){
    preferedGoalieAgent = policy()->Formation_Goalie();


    if( policy()->Formation_StrictFormation() ){
        preferedDefenseCounts = policy()->Formation_Defense();
        lastPreferredDefenseCounts = preferedDefenseCounts;
        return;
    }
    else
    {
        preferedDefenseCounts = policy()->Formation_Defense();
    }


    int agentsCount = wm->our.t->activeAgents.count();

    if( agentsCount == 1 ){
        if( knowledge->isOurNonPlayOnKick() ){
            preferedGoalieAgent = -1;
        }
    }
    else if( agentsCount == 2 ){
        if( knowledge->isStart() ){
            preferedDefenseCounts = 0;
        }
        else if( knowledge->isTheirNonPlayOnKick() ){
            preferedDefenseCounts = 0;
        }
        else if( knowledge->isOurNonPlayOnKick() ){
            preferedGoalieAgent = -1;
            preferedDefenseCounts = 0;
        }
    }
    else if( agentsCount == 3 ){
        if( knowledge->isStart() ){
            preferedDefenseCounts = 1;
        }
        else if( knowledge->isTheirNonPlayOnKick() ){
            if(knowledge->toBeMopps.count() == 0)
                preferedDefenseCounts = 1;
            else if(knowledge->toBeMopps.count() <=2 )
            {
                preferedDefenseCounts = 2 - knowledge->toBeMopps.count();
            }
            else
            {
                preferedDefenseCounts = 0;
            }

        }
        else if( knowledge->isOurNonPlayOnKick() ){
            preferedDefenseCounts = 0;
        }
    }
    else if( agentsCount == 4  ){
        bool oppsAttack = false;
        for(int i = 0; i<wm->opp.activeAgentsCount() ; i++) {
            if(wm->opp.active(i)->pos.x <= 0 )
                oppsAttack = true;
        }

        if( knowledge->isStart() ) {

            preferedDefenseCounts = 2;
        }
        else if( knowledge->isOurNonPlayOnKick() ) {
            preferedDefenseCounts = 1;
        }
        if( knowledge->getGameState() == CKnowledge::TheirKickOff) {
            if(knowledge->toBeMopps.count() <= 1)
                preferedDefenseCounts = 2;
            else if(knowledge->toBeMopps.count() <=2 )
            {
                knowledge->desiredDefCount = 2 - knowledge->toBeMopps.count();
            }
            else
            {
                knowledge->desiredDefCount = 0;
            }
        }
        else if( knowledge->isTheirNonPlayOnKick() || translationFlag) {
            if(wm->our.activeAgentsCount() > 1)
                preferedDefenseCounts = wm->our.activeAgentsCount() - 2;
        }
    }
    else if( agentsCount == 5 ){
        bool oppsAttack = false;
        for(int i = 0; i<wm->opp.activeAgentsCount() ; i++) {
            if(wm->opp.active(i)->pos.x <= 0 )
                oppsAttack = true;
        }

        if( knowledge->isStart() ) {

            preferedDefenseCounts = 2;

        }
        else if( knowledge->isOurNonPlayOnKick() ) {
            preferedDefenseCounts = 1;
        }
        if( knowledge->getGameState() == CKnowledge::TheirKickOff) {
            if(knowledge->toBeMopps.count() <= 1)
                preferedDefenseCounts = 2;
            else if(knowledge->toBeMopps.count() <=3 )
            {
                knowledge->desiredDefCount = 3 - knowledge->toBeMopps.count();
            }
            else
            {
                knowledge->desiredDefCount = 0;
            }
        }
        else if( knowledge->isTheirNonPlayOnKick() || translationFlag) {
            if(wm->our.activeAgentsCount() > 1)
                preferedDefenseCounts = wm->our.activeAgentsCount() - 2;
        }
    }

    else if( agentsCount == 6 ) {
        bool oppsAttack = false;
        for(int i = 0; i<wm->opp.activeAgentsCount() ; i++) {
            if(wm->opp.active(i)->pos.x <= 0 )
                oppsAttack = true;
        }

        if( knowledge->isStart() ) {

            preferedDefenseCounts = 2;

        }
        else if( knowledge->isOurNonPlayOnKick() ) {
            preferedDefenseCounts = 1;
        }
        if( knowledge->getGameState() == CKnowledge::TheirKickOff) {
            if(knowledge->toBeMopps.count() <= 1)
                preferedDefenseCounts = 2;
            else if(knowledge->toBeMopps.count() <=4 )
            {
                knowledge->desiredDefCount = 4 - knowledge->toBeMopps.count();
            }
            else
            {
                knowledge->desiredDefCount = 0;
            }
        }
        else if( knowledge->isTheirNonPlayOnKick() || translationFlag) {
            Circle2D ourDefenseArea(wm->field->ourGoal() + Vector2D(-0.2 , 0),1.6);
            if(wm->our.activeAgentsCount() > 1)
            {
                if((ourDefenseArea.contains(wm->ball->pos) && wm->ball->vel.length() < 1))
                {
                    preferedDefenseCounts = wm->our.activeAgentsCount() - 1;
                }
                else
                {
                    preferedDefenseCounts = wm->our.activeAgentsCount() - 2;
                }
            }
        }
    }


    lastPreferredDefenseCounts = preferedDefenseCounts;
}

void CCoach::calcDesiredMarkCounts()
{

    int agentsCount = wm->our.t->activeAgents.count();
    if( agentsCount == 2 ){
        knowledge->desiredDefCount = 0;
    }
    else if( agentsCount == 3 ){

        if(knowledge->toBeMopps.count() == 0)
            knowledge->desiredDefCount = 1;
        else if(knowledge->toBeMopps.count() <=2 )
        {
            knowledge->desiredDefCount = 2 - knowledge->toBeMopps.count();
        }
        else
        {
            knowledge->desiredDefCount = 0;
        }


    }
    else if( agentsCount == 4  ){


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

    }
    else if( agentsCount == 5 ){

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
    QQueue<int> ids = wm->our.t->activeAgents;
    goalieAgent = NULL;
    if( ids.contains(goalieID) ){
        CAgent *agnt = knowledge->getAgent(goalieID);
        goalieAgent = agnt;
    }
}
CKnowledge::ballPossesionState CCoach::isBallOurs()
{
    int oppNearestToBall=-1,ourNearestToBall=-1;
    double oppNearestToBallDist = 100000, ourNearestToBallDist = 100000;
    Segment2D ballPath(wm->ball->pos,wm->ball->pos+wm->ball->vel*2);
    Segment2D oppPath;
    Vector2D ballPos = wm->ball->pos;
    Vector2D dummy1,dummy2;
    double oppIntersetMin = 100000,ourIntersetMin = 100000;
    int oppIntersectAgent = -1,ourIntersectAgent = -1;
    CKnowledge::ballPossesionState decidePState;
    QList <CAgent*> ourAgents;
    QList <CRobot*> oppAgents;
    ourAgents.clear();

    ////////////////// our

    ourAgents = knowledge->getActiveAgents();

    if(knowledge->goalie != NULL){
        ourAgents.removeOne(knowledge->goalie);
    }
    for(int i = 0 ; i<knowledge->defenseAgents.count() ; i++){
        ourAgents.removeOne(knowledge->defenseAgents[i]);
    }
    ////////////////// opp
    oppAgents.clear();
    for(int i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
        oppAgents .append( wm->opp.active(i));
    }



    for(int i = 0 ; i < ourAgents.count() ; i++) {
        if(ourAgents[i]->pos().dist(ballPos) < ourNearestToBallDist) {
            ourNearestToBall = ourAgents[i]->id();
            ourNearestToBallDist = ourAgents[i]->pos().dist(ballPos);
        }
        draw(ourAgents[i]->pos(),1,QColor(Qt::black));

        if(wm->ball->vel.length() > 0.5){
            if(Circle2D(ourAgents[i]->pos() , 0.12).intersection(ballPath,&dummy1,&dummy2)){
                if(ourAgents[i]->pos().dist(ballPos) < ourIntersetMin)
                {
                    ourIntersetMin = ourAgents[i]->pos().dist(ballPos);
                    ourIntersectAgent = ourAgents[i]->id();
                }
            }
        }
    }

    for(int i = 0 ; i < oppAgents.count() ; i++){
        if(wm->opp.active(i)->vel.length()<0.5){
            Circle2D(oppAgents[i]->pos,0.3).tangent(ballPos,&dummy1,&dummy2);
            oppPath.assign(oppAgents[i]->pos,oppAgents[i]->pos);
        }
        else{
            oppPath.assign(oppAgents[i]->pos,oppAgents[i]->pos + oppAgents[i]->vel);
        }


        if(oppPath.nearestPoint(ballPos).dist(ballPos) < oppNearestToBallDist){
            oppNearestToBallDist = oppPath.nearestPoint(ballPos).dist(ballPos);
            oppNearestToBall = oppAgents[i]->id;
        }

        if(wm->ball->vel.length() > 0.5){
            if(oppPath.intersection(ballPath).isValid()){
                if(oppAgents[i]->pos.dist(ballPos) < oppIntersetMin){
                    oppIntersetMin = oppAgents[i]->pos.dist(ballPos);
                    oppIntersectAgent = oppAgents[i]->id;
                }
            }
        }
        draw(oppPath.nearestPoint(ballPos));


    }




    if(oppNearestToBall == -1 || (wm->field->isInOurPenaltyArea(wm->ball->pos)  && wm->ball->vel.length()<0.5))
    {
        decidePState =  CKnowledge::WEHAVETHEBALL;
    }
    else if(oppIntersectAgent != -1 && ourIntersectAgent == -1 && wm->ball->vel.length() > 1)
        decidePState = CKnowledge::WEDONTHAVETHEBALL;
    else if(oppIntersectAgent != -1 && ourIntersectAgent != -1 && ourIntersetMin > oppIntersetMin  - 0.1 && wm->ball->vel.length() > 0.5)
        decidePState = CKnowledge::WEDONTHAVETHEBALL;
    else if(oppNearestToBallDist <= ourNearestToBallDist){
        decidePState = CKnowledge::WEDONTHAVETHEBALL;
    }
    else if(ourNearestToBallDist < 0.3 && oppNearestToBallDist > ourNearestToBallDist){
        decidePState = CKnowledge::WEHAVETHEBALL;
    }
    else if(oppNearestToBallDist > ourNearestToBallDist +0.5){
        decidePState = CKnowledge::WEHAVETHEBALL;
    }
    else if(ourNearestToBallDist < oppNearestToBallDist && ballPos.x < 0.1){
        decidePState = CKnowledge::SOSOTHEIR;
    }
    else if(ourNearestToBallDist < oppNearestToBallDist && ballPos.x >= 0.1){
        decidePState = CKnowledge::SOSOOUR;
    }
    else if(oppIntersectAgent == -1 && ourIntersectAgent != -1 && wm->ball->vel.length() > 0.7)
        decidePState = CKnowledge::WEHAVETHEBALL;
    else{
        decidePState = CKnowledge::SOSOTHEIR;
    }




    if(decidePState == CKnowledge::WEHAVETHEBALL)
        playOnExecTime.restart();
    if(lastBallPossesionState == CKnowledge::WEHAVETHEBALL && (decidePState == CKnowledge::WEDONTHAVETHEBALL || decidePState == CKnowledge::SOSOTHEIR || decidePState == CKnowledge::SOSOOUR) && playOnExecTime.elapsed() < playOnTime)
    {
        decidePState = CKnowledge::WEHAVETHEBALL;
    }



    if(oppNearestToBall >= 0){
        Circle2D oppNearestDribblerArea(wm->opp[oppNearestToBall]->pos + wm->opp[oppNearestToBall]->dir.norm()*0.1 , 0.15);
        draw(oppNearestDribblerArea,QColor(Qt::red));
        if(oppNearestDribblerArea.contains(ballPos) && ourNearestToBallDist > 0.3)
            decidePState = CKnowledge::WEDONTHAVETHEBALL;
        else if( oppNearestDribblerArea.contains(ballPos) )
        {
            if(wm->ball->pos.x >= 0.1)
            {
                decidePState = CKnowledge::SOSOOUR;
            }
            else
            {
                decidePState = CKnowledge::SOSOTHEIR;
            }
        }
    }

    if(decidePState != lastBallPossesionState)
    {
        if(intentionTimePossession.elapsed() > possessionIntentionInterval)
        {
            intentionTimePossession.restart();
        }
        else
        {
            decidePState = lastBallPossesionState;
        }
    }

    lastBallPossesionState = decidePState;
    return decidePState;
}



QList<int> CCoach::findBestPoses(int numberOfPositionAgents,bool semiDynamic)
{
    QList<int> result;
    result.clear();
    QList<int> OurAgents;
    QList<CRobot*> OppAgents;
    double posWeights[30] = {0};
    int bestPosNum[5] = {0};
    double bestPosWeight = 0;

    double dummyDist = 20000;
    int nearestId = 0;



    OurAgents = wm->our.t->activeAgents;
    OppAgents.clear();
    for(int i = 0 ;i<OurAgents.count() ; i++)
    {
        if(wm->our[OurAgents[i]]->pos.dist(wm->ball->pos + wm->ball->vel) < dummyDist)
        {
            dummyDist = wm->our[OurAgents[i]]->pos.dist(wm->ball->pos + wm->ball->vel);
            nearestId = OurAgents[i];
        }
    }

    if( goalieAgent != NULL ){
        OurAgents.removeOne(goalieAgent->self()->id);
    }
    for( int i=0 ; i<defenseAgents.size() ; i++ ){
        if( OurAgents.contains(defenseAgents.at(i)->self()->id) ){
            OurAgents.removeOne(defenseAgents.at(i)->self()->id);
        }
    }

    for(int _i=0;_i < wm->opp.activeAgentsCount();_i++)
    {
        if(!wm->field->isInOppPenaltyArea(wm->opp.active(_i)->pos))
            OppAgents.append(wm->opp.active(_i));
    }
    OurAgents.removeOne(nearestId);
    for(int _i = 0 ;_i <OppAgents.count() ; _i++)
    {
        draw(OppAgents[_i]->pos,1,QColor(Qt::red));
    }


    double minDist = 10000;
    int minNum = 0;
    if(!semiDynamic || numberOfPositionAgents <= 1 ){
        for(int _c = 0 ; _c < numberOfPositionAgents ;_c++)
        {

            for(int _i = 0 ; _i < 30 ;_i++)
            {

                minDist = 100000;
                posWeights[_i] = 0;
                for(int _j = 0 ; _j <OurAgents.count() ; _j++)
                {

                    if(wm->our[OurAgents[_j]]->pos.dist(knowledge->getStaticPoses(_i)) < minDist)
                    {
                        minDist = wm->our[OurAgents[_j]]->pos.dist(knowledge->getStaticPoses(_i));
                        minNum = _j;
                    }


                }
                posWeights[_i] -= 2*minDist;
                minDist = 100000;
                for(int _j = 0 ; _j <OppAgents.count() ; _j++)
                {
                    if(OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i)) < minDist)
                    {
                        minDist = OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i));
                        minNum = _j;
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
    }
    else if(numberOfPositionAgents == 2){
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
                            minNum = _j;
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
                    for(int _j = 0 ; _j <OppAgents.count() ; _j++)
                    {
                        if(OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i)) < minDist)
                        {
                            minDist = OppAgents[_j]->pos.dist(knowledge->getStaticPoses(_i));
                            minNum = _j;
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

    }
    else if(numberOfPositionAgents == 3){
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
                            minNum = _j;
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
                            minNum = _j;
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

    }

    for(int _c = 0; _c < numberOfPositionAgents ; _c++)
    {
        result.append(bestPosNum[_c]);
    }


    return result;


}

void CCoach::assignDefenseAgents( int defenseCount){


    QQueue<int> ids = wm->our.t->activeAgents;
    if( goalieAgent ){
        ids.removeOne(goalieAgent->id());
    }
    double nearestDist = 1000000;
    double nearestRobot = -1;

    if(playmakeId != -1)
    {
        ids.removeOne(playmakeId);
    }

    defenses.getDefencePoses(defenseTargets);
    //////////////////calculate playmake dist
    double nDistFPM = 10000;
    int nDistId = -1;
    Circle2D ourDefenseArea(wm->field->ourGoal() + Vector2D(-0.2 , 0),1.6);
    for(int i = 0 ; i < ids.count() ; i++) {
        if(wm->our[ids[i]]->pos.dist(wm->ball->pos) <  nDistFPM) {
            nDistFPM = wm->our[ids[i]]->pos.dist(wm->ball->pos);
            nDistId = ids[i];
        }
    }
    if(nDistFPM < 0.3 + exeptionPlayMakeThr && !ourDefenseArea.contains(wm->ball->pos) && knowledge->getGameState() == CKnowledge::Start ) {
        exeptionPlayMakeThr = 0.2;
        exeptionPlayMake = knowledge->getAgent(nDistId);
        ids.removeOne(nDistId);
    }
    else {
        exeptionPlayMakeThr = 0;
    }

    defenseAgents.clear();
    for(int i =0 ;i < defenseCount ; i++)
    {
        nearestDist = 1000000;
        for(int j = 0 ; j < ids.count() ; j++)
        {
            if(wm->our[ids[j]]->pos.dist(defenseTargets[i]) < nearestDist)
            {
                nearestDist = wm->our[ids[j]]->pos.dist(defenseTargets[i]);
                nearestRobot =  ids[j];
            }
        }
        if(nearestRobot>=0)
        {
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
bool CCoach::isBallcollide()
{
    Circle2D dummyCircle;
    Vector2D sol1,sol2;
    Segment2D ballPath(wm->ball->pos,wm->ball->pos+wm->ball->vel);
    for(int i = 0 ; i < wm->our.activeAgentsCount() ; i++) {
        dummyCircle.assign(wm->our.active(i)->pos,0.08);
        if(dummyCircle.intersection(ballPath,&sol1,&sol2) && wm->our.active(i)->pos.dist(wm->ball->pos) < 0.14) {
            return true;
        }
        if(wm->ball->vel.length() < 0.5 && wm->our.active(i)->pos.dist(wm->ball->pos) < 0.13) {
            return true;
        }
    }
    return false;
}

void CCoach::virtualPlayOffState()
{
    CKnowledge::State currentState;
    currentState = knowledge->getGameState();
    if(lastState == CKnowledge::TheirDirectKick || lastState == CKnowledge::TheirIndirectKick || lastState == CKnowledge::TheirKickOff) {
        if(currentState == CKnowledge::Start) {
            translationFlag = true;
        }
    }
    if(translationFlag == false) {
        translateTimeOut.restart();
    }
    if(translateTimeOut.elapsed() >= translationTimeOutTime) {
        translationFlag = false;
    }
    if(wm->ball->pos.x >= 0) {
        translationFlag = false;
    }
    if(isBallcollide()){
        translationFlag = false;
    }
    knowledge->translationFlag = translationFlag;

    lastState  = currentState;

}

void CCoach::decideDefense(){
    assignGoalieAgent(preferedGoalieAgent);
    draw(QString("TS : %1").arg(translationFlag), Vector2D(2,-3));
    assignDefenseAgents(preferedDefenseCounts);
    calcDesiredMarkCounts();


    if( knowledge->getGameState() == CKnowledge::TheirPenaltyKick ){
        defenseAgents.clear();
        defenses.initGoalie(goalieAgent);
        defenses.initDefense(defenseAgents);
        defenses.execute();
    }
    else{
        defenses.initGoalie(goalieAgent);
        defenses.initDefense(defenseAgents);
        //		defenses.debugAgents("Defense");
        defenses.execute();
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
    double safeRegion = 1;
    double critLenth = 0.5;
    CAgent *ourNearestAgent;
    CRobot *oppNearest;
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
    ids = wm->our.t->activeAgents;
    ourNearestAgent = knowledge->getAgent(knowledge->getNearestAgentToPoint(wm->ball->pos,&ids));
    robotCritArea.addVertex(ourNearestAgent->pos());
    robotCritArea.addVertex(ourNearestAgent->pos() + ourNearestAgent->dir().norm() * critLenth + ourNearestAgent->dir().norm().rotate(90)* critLenth );
    robotCritArea.addVertex(ourNearestAgent->pos() + ourNearestAgent->dir().norm() * critLenth + ourNearestAgent->dir().norm().rotate(-90)* critLenth);
    draw(robotCritArea,QColor(Qt::cyan));


    if(oppNearestPath.nearestPoint(wm->ball->pos).dist(wm->ball->pos) >= safeRegion) {
        ourAttackState = SAFE;
        debug(QString("Attack: safe"),D_MHMMD);
    }
    else if(robotCritArea.contains(oppNearest->pos)) {
        ourAttackState = CRITICAL;
        debug(QString("Attack: critial"),D_MHMMD);
    }
    else {
        ourAttackState = FAST;
        debug(QString("Attack: fast"),D_MHMMD);
    }

}
void CCoach::choosePlaymakeAndSupporter(bool needSupporter,bool defenseFirst)
{
    QList<int> ourPlayers = wm->our.t->activeAgents;
    static QList<int> lastPlayers;
    if( goalieAgent != NULL ){
        ourPlayers.removeOne(goalieAgent->self()->id);
    }
    if(defenseFirst){
        for( int i=0 ; i<defenseAgents.size() ; i++ ){
            if( ourPlayers.contains(defenseAgents.at(i)->self()->id) ){
                ourPlayers.removeOne(defenseAgents.at(i)->self()->id);
            }
        }
    }


    ////////////////////first we choose our playmake
    double playMakeParam[6];//edited 5 to 6
    double biggestPoint = -1000;
    double ballVelCoef = 0.5;
    for(int i = 0 ; i < ourPlayers.count() ; i++)
    {
        playMakeParam[i] = 1/(wm->our[ourPlayers[i]]->pos.dist(wm->ball->pos+wm->ball->vel*ballVelCoef));
        if(ourPlayers[i] == lastPlayMake)
            playMakeParam[i] += playMakeTh;
    }

    for(int i = 0 ; i < ourPlayers.count() ; i++)
    {
        if(playMakeParam[i] > biggestPoint)
        {
            biggestPoint = playMakeParam[i];
            playmakeId = ourPlayers[i];
        }
    }


    if(playmakeId != lastPlayMake )
    {
        if(playMakeIntention.elapsed() > playMakeIntentionInterval)
        {
            playMakeIntention.restart();
        }
        else
        {
            playmakeId = lastPlayMake;
        }

    }
    lastPlayMake = playmakeId;

}

bool CCoach::decideAttack()
{

    bool toReturn = false;

    ballPState = isBallOurs();
    updateAttackState();
    knowledge->ballPossesion = ballPState;

//    double dummyDist = 25400;
//    int nearestId = 0;

    lastBallPossesionState = ballPState;

    // find unused agents!



    QList<int> ourPlayers = wm->our.t->activeAgents;




    if( goalieAgent != NULL ){
        ourPlayers.removeOne(goalieAgent->self()->id);
    }
    for( int i=0 ; i<defenseAgents.size() ; i++ ){
        if( ourPlayers.contains(defenseAgents.at(i)->self()->id) ){
            ourPlayers.removeOne(defenseAgents.at(i)->self()->id);
        }
    }



    //selectedPlay = NULL;

    switch (knowledge->getGameState()) { // GAMESTATE

    case CKnowledge::Halt:
        return decideHalt(ourPlayers);
        break;

    case CKnowledge::Stop:
        return decideStop(ourPlayers);
        break;

    case CKnowledge::OurKickOff:
        toReturn = decideOurKickOff(ourPlayers);
        break;

    case CKnowledge::TheirKickOff:
        toReturn = decideTheirKickOff(ourPlayers);
        break;

    case CKnowledge::OurDirectKick:
        toReturn = decideOurDirect(ourPlayers);
        break;

    case CKnowledge::TheirDirectKick:
        toReturn = decideTheirDirect(ourPlayers);
        break;

    case CKnowledge::OurIndirectKick:
        toReturn = decideOurIndirect(ourPlayers);
        break;

    case CKnowledge::TheirIndirectKick:
        toReturn = decideTheirIndirect(ourPlayers);
        break;

    case CKnowledge::OurPenaltyKick:
        toReturn = decideOurPenalty(ourPlayers);
        break;

    case CKnowledge::TheirPenaltyKick:
        toReturn = decideTheirPenalty(ourPlayers);
        break;
    case CKnowledge::Start:
        decideStart(ourPlayers);
        break;

    case CKnowledge::NormalStart:
        switch (knowledge->getGameMode()) {
        case CKnowledge::OurKickOff:
            toReturn = decideOurKickOff(ourPlayers);
            break;
        case CKnowledge::TheirKickOff:
            toReturn = decideTheirKickOff(ourPlayers);
            break;
        case CKnowledge::OurPenaltyKick:
            toReturn = decideOurPenalty(ourPlayers);
            break;
        case CKnowledge::TheirPenaltyKick:
            toReturn = decideTheirPenalty(ourPlayers);
            break;
        default:
            toReturn = decideNormalStart(ourPlayers);
            break;

        }
        break;
    case CKnowledge::OurBallPlacement:
        toReturn = decideOurBallPlacement(ourPlayers);
        break;
    case CKnowledge::TheirBallPlacement:
        toReturn = decideTheirBallPlacement(ourPlayers);
        break;
    default:
        return decideNull(ourPlayers);
        break;
    }

    selectedPlay->init(ourPlayers, &editData);
    selectedPlay->execute();
    debug(selectedPlay->whoami(), D_MAHI, QColor(Qt::blue));
    lastPlayers.clear();
    lastPlayers.append(ourPlayers);
    //return toReturn;
    return true;
}

void CCoach::decidePlayOff(QList<int>& _ourplayers, POMODE _mode) {

    //Decide Plan
    if (firstTime) {
        NGameOff::EMode tempMode;
        selectPlayOffMode(tempMode);
        initPlayOffMode(tempMode, _mode, _ourplayers);
        ourPlayOff->setMasterMode(tempMode);
        firstTime = false;
        qDebug() << "[Coach] first time config done";
    } else {
        setPlayOff( ourPlayOff->getMasterMode() );

    }
}
void CCoach::decidePlayOn(QList<int>& ourPlayers, QList<int>& lastPlayers) {

    CKnowledge::ballPossesionState ballPState = isBallOurs();

    if(playmakeId != -1)
    {
        dynamicAttack->setPlayMake(playmakeId);
        ourPlayers.removeOne(playmakeId);
        debug(QString("playmake : %1").arg(playmakeId),D_MHMMD);
    }

    double PosNum= 0;
    double MarkNum = 0;


#ifdef KK_PLAYON
    if(kkLastState != knowledge->getGameState())
        forceStart->setDecidePlan(true);
#endif
    selectedPlay = forceStart;

    Circle2D ourDefenseArea(wm->field->ourGoal() + Vector2D(-0.2 , 0),1.6);

    if(knowledge->variables["clearing"] == "true"
            || (ourDefenseArea.contains(wm->ball->pos) && wm->ball->vel.length()
                < 1))
    {
        if(playmakeId != -1)
        {
            ourPlayers.append(playmakeId);
            dynamicAttack->setPlayMake(-1);
        }
        dynamicAttack->setDefenseClear(true);
    }
    else
    {
        dynamicAttack->setDefenseClear(false);
    }

    if(findMostPossible(wm->ball->pos) > (0.7 - shotToGoalthr )  )
    {
        dynamicAttack->setDirectShot(true);
        shotToGoalthr = 0.6;
    }
    else
    {
        dynamicAttack->setDirectShot(true);
        shotToGoalthr = 0;
    }
    //////////////////////////////////////////////assign agents
    GlobalBallPState = ballPState;
    if(ballPState == CKnowledge::WEHAVETHEBALL) {
        MarkNum = 0;
    } else if(ballPState == CKnowledge::WEDONTHAVETHEBALL)
    {
        MarkNum = 2;
    }
    else if(ballPState == CKnowledge::SOSOOUR)
    {
        MarkNum = 0;
    }
    else if(ballPState == CKnowledge::SOSOTHEIR)
    {
        MarkNum = 1;
    }

    ////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////
    dynamicAttack->setPositions(findBestPoses(ourPlayers.count(), true));

    dynamicAttack->setNoPlanException(true);
    if(ballPState == CKnowledge::WEHAVETHEBALL)
        dynamicAttack->setWeHaveBall(true);
    else
        dynamicAttack->setWeHaveBall(false);
    if(ourAttackState == FAST) {
        dynamicAttack->setFast(true);
    }
    else {
        dynamicAttack->setFast(false);
    }

    if(ourAttackState == CRITICAL) {
        dynamicAttack->setCritical(true);
    }
    else {
        dynamicAttack->setCritical(false);
    }
    selectedPlay = dynamicAttack;


    selectedPlay->markAgents.clear();
    if(wm->ball->pos.x >= 0
            && selectedPlay->lockAgents
            && lastPlayers.count() == ourPlayers.count())
    {
        ourPlayers.clear();
        ourPlayers = lastPlayers;
        selectedPlay->markAgents.clear();

    }
    else
    {
        selectedPlay->markAgents.clear();

        if(ourPlayers.count())
        {
            if(MarkNum == 2)
            {
                for(int i = ourPlayers.count()-1 ; i >=0 ;i--)
                {

                    selectedPlay->markAgents.append(knowledge->getAgent(ourPlayers.at(i)));
                    ourPlayers.removeAt(i);


                }
                ourPlayers.clear();
            }
            else if(MarkNum == 1)
            {
                if(ourPlayers.count() > 1)
                {
                    int x= -1000;
                    int bestX =-1;
                    for(int i =0 ; i < ourPlayers.count() ; i++)
                    {
                        if(wm->our[ourPlayers.at(i)]->pos.x > x)
                        {
                            x = wm->our[ourPlayers.at(i)]->pos.x;
                            bestX = ourPlayers.at(i);
                        }
                    }

                    for(int i =0 ; i < ourPlayers.count() ; i++)
                    {
                        if(ourPlayers[i]  != bestX)
                        {
                            selectedPlay->markAgents.append(knowledge->getAgent(ourPlayers.at(i)));
                            ourPlayers.removeAt(i);
                        }
                    }
                    ourPlayers.clear();
                    ourPlayers.append(bestX);
                }
                else
                {
                    selectedPlay->markAgents.append(knowledge->getAgent(ourPlayers.at(0)));
                    ourPlayers.clear();
                }
            }
        }
    }
}

void CCoach::matchPlan(NGameOff::SPlan *_plan, const QList<int>& _ourplayers) {
    //  TODO : Matching Function [DONE]
    MWBM matcher;
    matcher.create(_plan->common.currentSize, _ourplayers.size());
    for (size_t i = 0; i < _plan->common.currentSize; i++) {
        for (size_t j = 0; j < _ourplayers.size(); j++) {

            double weight;
            if (_plan->matching.initPos.agents.at(i).x == -100) {
                weight = knowledge->getAgent(j)->pos().dist(wm->ball->pos);
            } else {
                weight = _plan->matching.initPos.agents.at(i).dist(knowledge->getAgent(_ourplayers.at(j))->pos());
            }
            matcher.setWeight(i, j, (int)((1/(weight + 1))*10));
        }
    }
    qDebug() << "[Coach] matched plan with : " << matcher.findMatching();
    for (size_t i = 0; i < _plan->common.currentSize;i++) {
        int matchedID = matcher.getMatch(i);
        _plan->common.matchedID.insert(i, _ourplayers.at(matchedID));

    }
    qDebug() << "[Coach] mathched by" << _plan->common.matchedID;
}

bool CCoach::isTagsMatched(const QStringList& base, const QStringList& required) {
    Q_FOREACH(QString tag, required)
        if (!base.contains(tag))
            return false;
    return true;
}

bool CCoach::isRegionMatched(const Vector2D &_ball, const double& regionRadius) {

    return (wm->ball->pos.dist(_ball) < regionRadius);

}

NGameOff::SPlan* CCoach::chooseMostSuccecfull(const QList<NGameOff::SPlan*>& plans) {
    QList<NGameOff::SPlan*> bestPlans;
    int bestScore = -1;
    Q_FOREACH(NGameOff::SPlan* plan, plans) {
        if (plan->common.succesRate > bestScore) {
            bestPlans.clear();
            bestPlans.append(plan);
        }
        else if (plan->common.succesRate == bestScore) {
            bestPlans.append(plan);
        }
    }
    // TODO : remove randomize
    if (bestPlans.isEmpty()) {
        bestPlans.append(plans);
    }
    return bestPlans[rand()%bestPlans.size()];
}

void CCoach::selectPlayOffMode(NGameOff::EMode &_mode) {
    // TODO : a real one needed
    _mode = NGameOff::StaticPlay;
}

void CCoach::initPlayOffMode(const NGameOff::EMode _mode,
                             const POMODE _gameMode,
                             const QList<int>& _ourplayers) {
    switch(_mode) {
    case NGameOff::StaticPlay:
        initStaticPlay(_gameMode, _ourplayers);
        break;
    case NGameOff::DynamicPlay:
        initDynamicPlay();
        break;
    case NGameOff::FastPlay:
        initFastPlay();
        break;
    case NGameOff::FirstPlay:
        initFirstPlay();
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


void CCoach::initStaticPlay(const POMODE _mode, const QList<int>& _ourplayers) {

    QList<NGameOff::SPlan*> validPlans;
    QList<NGameOff::SPlan*> plans = m_planLoader->getPlans(); // Get All of The Plans
    Q_FOREACH(NGameOff::SPlan* plan, plans) { //Find Valid Plans
        NGameOff::SMatching& matching = plan->matching;

        // Just For Debugging
        if (1) {
            qDebug() << "-----------> plan name" << plan->gui.name;
            if (matching.common->planMode  >= _mode)
                qDebug() << "[Coach] Mode is Valid";
            if (matching.common->agentSize >= _ourplayers.size())
                qDebug() << "[Coach] AgentSize is Valid";
            if (matching.common->chance >= 0)
                qDebug() << "[Coach] Chance is Valid";
            if (matching.common->lastDist >= 0)
                qDebug() << "[Coach] LastDist is Valid";
            if (isTagsMatched(matching.common->tags, currentTags))
                qDebug() << "[Coach] Tags is Valid";
            if (isRegionMatched(matching.initPos.ball))
                qDebug() << "[Coach] Ball is Valid";
            qDebug() << "<----------- plan END.";
        }

        if (matching.common->planMode  >= _mode
                && matching.common->agentSize >= _ourplayers.size()
                && matching.common->chance > 0
                && matching.common->lastDist >= 0
                && isTagsMatched(matching.common->tags, currentTags)) {

            //check Ball matchig with symmetry
            Vector2D symBall = Vector2D(matching.initPos.ball.x,
                                        -matching.initPos.ball.y);
            if (isRegionMatched(matching.initPos.ball)) {
                plan->execution.symmetry = 1;
            } else if (isRegionMatched(symBall)) {
                plan->execution.symmetry = -1;
            }

            plan->common.currentSize = _ourplayers.size();
            validPlans.append(plan);

        }
    }


    debug(QString("playoff -> there's %1 valid Plan").arg(validPlans.size()), D_DEBUG);
    if (validPlans.isEmpty()) {
        debug("[Warning] playoff -> there's no valid Plan", D_ERROR, QColor(Qt::red));
        qWarning() << "[Warning] playoff -> there's no valid Plan from " << m_planLoader->getPlans().size() << "Plans";
        return;
    }

    NGameOff::SPlan* thePlan = chooseMostSuccecfull(validPlans); //Choose Best valid Plan
    matchPlan(thePlan, _ourplayers); //Match The Plan
    ourPlayOff->setMasterPlan(thePlan);
    ourPlayOff->analyseShoot(); // should call after setmasterplan
    ourPlayOff->analysePass(); // should call after setmasterplan
    ourPlayOff->setInitial(true);
    ourPlayOff->lockAgents = true;
    lastPlan = thePlan;
}

void CCoach::initDynamicPlay() {
    // TODO : Initial Dynamic Play
}

void CCoach::initFastPlay() {
    // TODO : Initial Fast Play
}

void CCoach::initFirstPlay() {
    // TODO : Initial First Play
}

void CCoach::setStaticPlay() {
    // TODO : Complete staticPlay checker
    ourPlayOff->setInitial(false);
}

void CCoach::setDynamicPlay() {
    // TODO : Write Dynamic Play checker

}

void CCoach::setFirstPlay() {
    // TODO : Write First Play checker

}

void CCoach::setFastPlay() {
    // TODO : Write Fast Play checker

}


void CCoach::execute()
{
//    debug(QString("state : %1 \n").arg(knowledge->getGameState()), D_ATOUSA);
//    debug(QString(" mode : %1 \n").arg(knowledge->getGameState()), D_ATOUSA);

    draw(Circle2D(Vector2D(_PENALTY_AREA_CIRCLE_X , 0) , _PENALTY_AREA_CIRCLE_RAD) , 0 , 360 , "red");
    bool falseingNan = false;
    for (int i=0;i<wm->our.activeAgentsCount();i++)
        if (isNan(wm->our.active(i)->vel.length())) falseingNan = true;
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
        if (isNan(wm->opp.active(i)->vel.length())) falseingNan = true;
    if (isNan(wm->ball->vel.length())) falseingNan = true;
    if (falseingNan)
    {
        debug("falseing Nan", D_ERROR);
    }

    // reset idle state!
    for (int i = 0; i < _MAX_NUM_PLAYERS ; i++)
        agents[i]->idle = false;

    QTime timer;
    timer.start();

    // place your reset codes about knowledge vars in this function
    knowledge->resetEssentialVars();

    // checks whether the goalie is under the net or not if it is moves out
    checkGoalieInsight();

    updateKnowledgeVars();

    doIntention();

    //  debug(QString("%1) Coach Time1: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
    //  timer.restart();
    virtualPlayOffState();
    decidePreferedDefenseAgentsCountAndGoalieAgent();
    /////////////////////////////////////// choose playmake
    double critAreaRadius = 1.6;
    Circle2D critArea(wm->field->ourGoal(),critAreaRadius);
    playmakeId = -1;
    if((critArea.contains(wm->ball->pos) && wm->field->isInField(wm->ball->pos)) || wm->ball->vel.length() > 2)
    {
        decideDefense();

        choosePlaymakeAndSupporter(false,true);
    }
    else
    {
        choosePlaymakeAndSupporter(false,false);
        decideDefense();
    }

    ////////////////////////////////////////////

    decideAttack();


    checkRoleAssignments();

    //  debug(QString("%1) Coach Time2: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
    //  timer.restart();

    for (int i=0;i<_NUM_PLAYERS;i++)
    {
        if (agents[i]->isVisible() && agents[i]->idle == false)
        {
            if (agents[i]->skill != NULL)
            {
                agents[i]->skill->execute();
            }
        }
    }

    //  debug(QString("%1) Coach Time3: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
    //  timer.restart();
    //
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

bool CCoach::decideHalt(QList<int>& _ourPlayers) {
    firstTime = true;
    cyclesWaitAfterballMoved = 0;
    clearIntentions();
    _ourPlayers.clear();
    _ourPlayers.append(wm->our.t->activeAgents);
    for( int i = 0 ; i < _ourPlayers.count() ; i++ )
    {
        agents[_ourPlayers[i]]->waitHere();
    }
    knowledge->setLastPlayExecuted(HaltPlay);

    if (ourPlayOff != NULL) {
        delete ourPlayOff;
        ourPlayOff = NULL;
    }

    return true;
}

bool CCoach::decideStop(QList<int> & _ourPlayers) {
    firstTime = true;
    cyclesWaitAfterballMoved = 0;
    clearIntentions();
    CMasterPlay::position.reset();
    for( int i=0 ; i < _ourPlayers.size() ; i++ ){
        stopRoles[i]->assign(knowledge->getAgent(_ourPlayers.at(i)));
    }
    knowledge->setLastPlayExecuted(StopPlay);

    if (ourPlayOff != NULL) {

        delete ourPlayOff;
        ourPlayOff = NULL;

    }

    return true;
}

bool CCoach::decideOurKickOff(QList<int> &_ourPlayers) {
    if (ourPlayOff == NULL) {
        ourPlayOff = new CPlayOff();
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, KICKOFF);
//      lastPlayers.append(ourPlayers); // WHY ??
    debug(QString("ourplayers : %1").arg(_ourPlayers.size()),D_MAHI);

}

bool CCoach::decideTheirKickOff(QList<int> &_ourPlayers) {
    selectedPlay = theirKickOff;
    firstTime = true;
}

bool CCoach::decideOurDirect(QList<int> &_ourPlayers) {
    if (ourPlayOff == NULL) {
        ourPlayOff = new CPlayOff();
    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, DIRECT);
    debug(QString("ourplayers : %1").arg(_ourPlayers.size()), D_MAHI);

}

bool CCoach::decideTheirDirect(QList<int> &_ourPlayers) {
    selectedPlay = theirDirect;
    firstTime = true;
}

bool CCoach::decideOurIndirect(QList<int> &_ourPlayers) {
    if (ourPlayOff == NULL) {
        ourPlayOff = new CPlayOff();

    }
    selectedPlay = ourPlayOff;
    decidePlayOff(_ourPlayers, INDIRECT);
    debug(QString("ourplayers : %1").arg(_ourPlayers.size()),D_MAHI);

}

bool CCoach::decideTheirIndirect(QList<int> &_ourPlayers) {
    selectedPlay = theirIndirect;
    firstTime = true;
}

bool CCoach::decideOurPenalty(QList<int> &_ourPlayers) {
    selectedPlay = ourPenalty;
    debug("penalty",D_MHMMD);
    firstTime = true;
}

bool CCoach::decideTheirPenalty(QList<int> &_ourPlayers) {
    selectedPlay = theirPenalty;
    firstTime = true;

}

bool CCoach::decideStart(QList<int> &_ourPlayers) {
    decidePlayOn(_ourPlayers, lastPlayers);
    firstTime = true;
    if (ourPlayOff != NULL) {

        delete ourPlayOff;
        ourPlayOff = NULL;

    }
}

bool CCoach::decideNormalStart(QList<int> &_ourPlayers) {
    selectedPlay = ourPlayOff;
    firstTime = true;
    if (ourPlayOff != NULL) {
        delete ourPlayOff;
        ourPlayOff = NULL;
    }
}

bool CCoach::decideOurBallPlacement(QList<int> &_ourPlayers) {
    selectedPlay = ourBallPlacement;
}

bool CCoach::decideTheirBallPlacement(QList<int> &_ourPlayers) {
    selectedPlay = theirBallPlacement;
}

bool CCoach::decideNull(QList<int> &_ourPlayers) {
    selectedPlay->markAgents.clear();
    firstTime = true;
    if (ourPlayOff != NULL) {
        delete ourPlayOff;
        ourPlayOff = NULL;
    }
    debug(QString("Unexpected Game State: %1 %2").arg(knowledge->stateToString(knowledge->getGameState())).arg(knowledge->getGameState()) , D_ERROR , "red");
    return false;
}


