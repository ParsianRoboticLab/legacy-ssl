#include "plays/doublesizeourdirect.h"

#include "roles/playmake.h"
#include "soccer.h"
#include "coach.h"


CDoubleSizeOurDirect::CDoubleSizeOurDirect(){
    //	lastPlayExeced = &CDoubleSizeOurDirect::middleCenterAreaOffensive3;
}

CDoubleSizeOurDirect::~CDoubleSizeOurDirect(){

}


void CDoubleSizeOurDirect::reset(){
    position.reset();
    cyclesToExecute = 0;
    playMakeRole.resetPlayMake();
    executedCycles = 0;
    lastPlayExeced = NULL;
    oneToucherID = -1;
    staticPoints.clear();
    for( int i = 0 ; i < MAX_POSITIONERS ; i++ )
    {
        staticVec[i].clear();
        cyclesToWait[i].clear();
        facePoints[i].clear();
        escapeRadius[i].clear();
    }
}

void CDoubleSizeOurDirect::init(QList<int> _agents , QMap<QString , EditData*> *_editData){
    setAgentsID(_agents);
    setEditData(_editData);
    initMaster();

    if( knowledge->getLastPlayExecuted() != OurDirectPlay ){
        reset();
    }
    knowledge->setLastPlayExecuted(OurDirectPlay);
}

void CDoubleSizeOurDirect::passOrNot()
{
    int id = getAgent(oneToucherID);
    if( id == -1 )
    {
        return;
    }
    //	draw(positionAgents.at(id)->pos()+positionAgents.at(id)->vel()*.5,D_MOHAMMED,"red");
    //	draw(Circle2D(pointForPass,.7),D_MOHAMMED,"cyan",-1,false);
    if( (positionAgents.at(id)->pos()+positionAgents.at(id)->vel()*.5).dist(pointForPass[0]) < .7 || executedCycles > MAX_WEIGHT_CYCLES )
        playMakeRole.setNoKick(false);
    playMakeRole.setChip(true);
}

void CDoubleSizeOurDirect::setPosIntentions()
{
    for( int i = 0 ; i < positionAgents.count() ; i++ )
    {
        positionAgents.at(i)->positionIntent.assign(positionAgents.at(i)->self()->id , knowledge->frameCount , 600);
        positionAgents.at(i)->intention = &positionAgents.at(i)->positionIntent;
    }
}

QList<directPlayFunctionDouble> CDoubleSizeOurDirect::getValidPlays(QList<QPair<directPlayFunctionDouble,directPlayConditionDouble> > candidates)
{
    QList<directPlayFunctionDouble> validPlays;
    validPlays.clear();
    for( int i = 0 ; i < candidates.size() ; i++ )
    {
        if( runWithSymmetry((this->*candidates[i].second)) )
            validPlays.append(candidates[i].first);
    }
    return validPlays;
}

int CDoubleSizeOurDirect::getNearestAgent(Vector2D p)
{
    double dist = 1e6;
    int idx = -1;
    for( int i = 0 ; i < positionAgents.count() ; i++ )
    {
        if( dist > positionAgents.at(i)->pos().dist(p) ){
            dist = positionAgents.at(i)->pos().dist(p);
            idx = i;
        }
    }
    return idx;
}

void CDoubleSizeOurDirect::addNearestRobot2Mid(Vector2D p)
{
    if( knowledge->defenseAgents.count() == 0 )
        return;
    QList<int> ourDefenses;
    for( int i = 0 ; i < knowledge->defenseAgents.count() ; i++ )
        ourDefenses.append(knowledge->defenseAgents.at(i)->id());
    int nearestDefense = knowledge->getNearestAgentToPoint(p,&ourDefenses);

    CAgent* defFront = knowledge->defenseAgents.at(nearestDefense);

    knowledge->defenseAgents.removeAt(nearestDefense);

    defFront->positionIntent.assign(defFront->id(),knowledge->frameCount,600);
    defFront->intention = &defFront->positionIntent;

    positionAgents.push_back(defFront);
}

int CDoubleSizeOurDirect::getAgent(int id)
{
    for( int i = 0 ; i < positionAgents.count() ; i++ )
    {
        if( positionAgents.at(i)->id() == id )
            return i;
    }
    return -1;
}

Vector2D CDoubleSizeOurDirect::whoIsBetterForPass(int symmetry)
{
    return Vector2D(wm->ball->pos.x-1,symmetry*-1.5);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////START OF PLAYS//////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CDoubleSizeOurDirect::theirOneThirdFlankArea2(int symmetry)
{
    draw(QString("theirOneThirdFlankArea2"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(3.5,symmetry*-1);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(true);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);//should be handled inside playmake

        staticVec[0] << Vector2D(2.8,symmetry*.25) << Vector2D(2.59,symmetry*.092) << Vector2D(2.6,symmetry*-.203) << Vector2D(2.69,symmetry*-.512) << pointForPass[0];
        cyclesToWait[0] << 30 << 0 << 0 << 0 << 70;
        escapeRadius[0] << 0 << 0 << 0 << 0 << 0;
        facePoints[0] << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL ;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        oneToucherID = positionAgents.at(0)->id();

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::theirOneThirdFlankArea2;
}
bool CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::theirOneThirdCenterArea3(int symmetry)
{
    draw(QString("theirOneThirdCenterArea3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(wm->ball->pos.x+.2,-.5*symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(true);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << pointForPass[0];
        cyclesToWait[0] << waitBeforeExecution;
        escapeRadius[0] << 0;
        facePoints[0] << TOOPPGOAL;

        staticVec[1] << Vector2D(pointForPass[0].x,-pointForPass[0].y);
        cyclesToWait[1] << waitBeforeExecution;
        escapeRadius[1] << .5;
        facePoints[1] << TOOPPGOAL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
        oneToucherID = positionAgents.at(0)->id();

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::theirOneThirdCenterArea3;
}
bool CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea3(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::theirOneThirdFlankAreaSimple3(int symmetry)
{
    draw(QString("theirOneThirdFlankAreaSimple3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(3.5,symmetry*-1);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(true);
        playMakeRole.setNoKick(true);
        playMakeRole.setSpinBool(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << Vector2D(1.6,symmetry*-.3);
        cyclesToWait[0] << 200;
        escapeRadius[0] << .5;
        facePoints[0] << TOOPPGOAL;

        staticVec[1] << Vector2D(1.7,symmetry*-.6) << pointForPass[0];
        cyclesToWait[1] << waitBeforeExecution << 200;
        escapeRadius[1] << .5 << .5;
        facePoints[1] << TOOPPGOAL << TOOPPGOAL;

        int nearestIdx = getNearestAgent(staticVec[1].first());
        staticPoints.append(holdingPoints(positionAgents.at(nearestIdx),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
        staticPoints.append(holdingPoints(positionAgents.at(1-nearestIdx),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        oneToucherID = positionAgents.at(nearestIdx)->id();
        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::theirOneThirdFlankAreaSimple3;
}
bool CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaSimple3(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3(int symmetry)
{
    draw(QString("theirOneThirdFlankAreaNoAdd3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(3.5,symmetry*-1);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(true);
        playMakeRole.setNoKick(true);
        playMakeRole.setSpinBool(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << Vector2D(2.8,symmetry*.25) << Vector2D(2.59,symmetry*.092) << Vector2D(2.6,symmetry*-.203) << Vector2D(2.69,symmetry*-.512) << pointForPass[0];
        cyclesToWait[0] << 30 << 0 << 0 << 0 << 70;
        escapeRadius[0] << 0 << 0 << 0 << 0 << 0;
        facePoints[0] << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL ;

        staticVec[1] << Vector2D(2.8,symmetry*-.25) << Vector2D(2.8,0) << Vector2D(2.8,symmetry*.25);
        cyclesToWait[1] << 40 << 0 << 50;
        escapeRadius[1] << 0 << 0 << 0;
        facePoints[1] << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL;

        int nearestIdx = getNearestAgent(staticVec[1].first());
        staticPoints.append(holdingPoints(positionAgents.at(1-nearestIdx),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        staticPoints.append(holdingPoints(positionAgents.at(nearestIdx),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
        oneToucherID = positionAgents.at(1-nearestIdx)->id();

        setPosIntentions();
    }

    foreach( Vector2D v , staticVec[0] )
        draw(v,D_MOHAMMED,"cyan");

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3;
}
bool CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3(int symmetry)
{
    Rect2D emptyArea(Vector2D(2.7,.75),.3,1.5);
    draw(emptyArea,"blue",false);
    for( int i = 0 ; i < wm->opp.activeAgentsCount() ; i++ )
    {
        if( emptyArea.contains(wm->opp[i]->pos) )
            return false;
    }

    return true;
}
void CDoubleSizeOurDirect::middleFlankAreaDefensive3(int symmetry)
{
    draw(QString("middleFlankAreaDefensive3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(2.2,-1.5*symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;
        Circle2D aroundBall(wm->ball->pos,.5);
        Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
        Vector2D intersects[2];
        if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
            onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);//should be handled inside playmake

        staticVec[0] << Vector2D(2,symmetry*-1) << pointForPass[0];
        cyclesToWait[0] << waitBeforeExecution << 0;
        escapeRadius[0] << .5 << 0;
        facePoints[0] << TOOPPGOAL << TOOPPGOAL;

        staticVec[1] << onTheBallLine << Vector2D(2.5,symmetry*1.3);
        cyclesToWait[1] << waitBeforeExecution << 0;
        escapeRadius[1] << 0 << .5;
        facePoints[1] << TOOPPGOAL << TOBALL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
            staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));

        oneToucherID = positionAgents.at(0)->id();

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::middleFlankAreaDefensive3;
}
bool CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive3(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::middleFlankAreaDefensive2(int symmetry)
{
    draw(QString("middleFlankAreaDefensive2"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(wm->ball->pos.x-.5,symmetry*-1);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;
        Circle2D aroundBall(wm->ball->pos,.5);
        Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
        Vector2D intersects[2];
        if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
            onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(false);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << onTheBallLine << pointForPass[0];
        cyclesToWait[0] << 20 << 0;
        escapeRadius[0] << 0 << 0;
        facePoints[0] << TOOPPGOAL << TOBALL;
        if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
            staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

        oneToucherID = positionAgents.at(0)->id();

        setPosIntentions();
    }

    executedCycles++;

    passOrNot();

    setStaticPoints(staticPoints);

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::middleFlankAreaDefensive2;
}
bool CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive2(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::middleFlankAreaOffensive2(int symmetry)
{
    draw(QString("middleFlankAreaOffensive2"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(wm->ball->pos.x-0.75,-1*symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << Vector2D(2,symmetry*-1.5) << Vector2D(1,symmetry*-1.5);
        cyclesToWait[0] << waitBeforeExecution << 0;
        escapeRadius[0] << .5 << 0;
        facePoints[0] << TOBALL << TOBALL;

        staticVec[1] << pointForPass[0];
        cyclesToWait[1] << waitBeforeExecution;
        escapeRadius[1] << 0;
        facePoints[1] << TOBALL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

        setPosIntentions();
    }

    executedCycles++;

    if( executedCycles == waitBeforeExecution )
    {
        addNearestRobot2Mid(Vector2D(0,0));					//debug(QString("ddddddd:%1").arg(positionAgents.count()),D_MOHAMMED);
        staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
        oneToucherID = positionAgents.at(1)->id();
    }

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::middleFlankAreaOffensive2;
}
bool CDoubleSizeOurDirect::conditionMiddleFlankAreaOffensive2(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::middleFlankAreaOffensive3(int symmetry)
{
    draw(QString("middleFlankAreaOffensive3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(wm->ball->pos.x-0.75,-1*symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;
        Circle2D aroundBall(wm->ball->pos,.5);
        Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
        Vector2D intersects[2];
        if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
            onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        Vector2D reboundPos1 = ((Vector2D(0,0)-wm->ball->pos)*4+wm->field->oppGoal()-wm->ball->pos)*.3+wm->ball->pos;
        staticVec[0] << reboundPos1 << reboundPos1+Vector2D(1,symmetry*-1);
        cyclesToWait[0] << waitBeforeExecution/2 << 300;
        escapeRadius[0] << .5 << .5;
        facePoints[0] << TOBALL << TOOPPGOAL;

        Vector2D reboundPos2 = (wm->field->oppGoal()-onTheBallLine)*.3+onTheBallLine+(pointForPass[0]-onTheBallLine)*.3;
        staticVec[1] << onTheBallLine << reboundPos2;
        cyclesToWait[1] << waitBeforeExecution/2 << 300;
        escapeRadius[1] << 0 << .3;
        facePoints[1] << TOOPPGOAL << TOOPPGOAL;

        staticVec[2] << pointForPass[0];
        cyclesToWait[2] << 200;
        escapeRadius[2] << 0;
        facePoints[2] << TOBALL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
        {
            staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
            staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
        } else
            staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

        setPosIntentions();
    }

    executedCycles++;

    if( executedCycles == waitBeforeExecution )
    {
        addNearestRobot2Mid(Vector2D(0,0));
        oneToucherID = positionAgents.last()->id();
        staticPoints.append(holdingPoints(positionAgents.at(getAgent(oneToucherID)),staticVec[getAgent(oneToucherID)],cyclesToWait[getAgent(oneToucherID)],escapeRadius[getAgent(oneToucherID)],facePoints[getAgent(oneToucherID)]));
    }

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::middleFlankAreaOffensive3;
}
bool CDoubleSizeOurDirect::conditionMiddleFlankAreaOffensive3(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::middleCenterAreaDefensive1(int symmetry)
{
    draw(QString("middleCenterAreaDefensive1"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(wm->field->oppGoal());
        playMakeRole.setKickMode(FixedShoot);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(false);
        playMakeRole.setCyclesToWait(waitBeforeExecution);
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::middleCenterAreaDefensive1;
}
bool CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive1(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::middleCenterAreaOffensive2(int symmetry)
{
    middleFlankAreaOffensive2(symmetry);

    lastPlayExeced = &CDoubleSizeOurDirect::middleCenterAreaOffensive2;
}
bool CDoubleSizeOurDirect::conditionMiddleCenterAreaOffensive2(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::middleCenterAreaDefensive3(int symmetry)
{
    middleFlankAreaDefensive3(symmetry);

    lastPlayExeced = &CDoubleSizeOurDirect::middleCenterAreaDefensive3;
}
bool CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive3(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::middleCenterAreaOffensive3(int symmetry)
{
    draw(QString("middleCenterAreaOffensive3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = whoIsBetterForPass(symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << Vector2D(.6,-1.3*symmetry) << Vector2D(2.3,-1.4*symmetry);
        cyclesToWait[0] << waitBeforeExecution/2 << 200;
        escapeRadius[0] << 0 << 0;
        facePoints[0] << TOOPPGOAL << TOOPPGOAL;

        Vector2D reboundPos = Vector2D(2.5,symmetry*.5);
        staticVec[1] << Vector2D(.6,1.3*symmetry) << reboundPos;
        cyclesToWait[1] << waitBeforeExecution/2 << 200;
        escapeRadius[1] << 0 << 0;
        facePoints[1] << TOOPPGOAL << TOOPPGOAL;

        staticVec[2] << pointForPass[0];
        cyclesToWait[2] << 200;
        escapeRadius[2] << 0;
        facePoints[2] << TOBALL;

        staticVec[3] << Vector2D(playMakeAgent->pos().x,playMakeAgent->pos().y+symmetry*.8) << Vector2D(playMakeAgent->pos().x+1.2,playMakeAgent->pos().y+symmetry*1);
        cyclesToWait[3] << 0 << 200;
        escapeRadius[3] << 0 << 0;
        facePoints[3] << TOBALL << TOBALL;

        int nearestIdx = getNearestAgent(staticVec[0].first());
        staticPoints.append(holdingPoints(positionAgents.at(nearestIdx),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        staticPoints.append(holdingPoints(positionAgents.at(1-nearestIdx),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
        setPosIntentions();
    }
    draw(staticVec[3].at(1),D_MOHAMMED,"red");
    executedCycles++;

    if( executedCycles == waitBeforeExecution )
    {
        addNearestRobot2Mid(staticVec[2].first());
        oneToucherID = positionAgents.last()->id();
        addNearestRobot2Mid(staticVec[3].first());
        staticPoints.append(holdingPoints(positionAgents.at(2),staticVec[2],cyclesToWait[2],escapeRadius[2],facePoints[2]));
        staticPoints.append(holdingPoints(positionAgents.at(3),staticVec[3],cyclesToWait[3],escapeRadius[3],facePoints[3]));
    }

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::middleCenterAreaOffensive3;
}
bool CDoubleSizeOurDirect::conditionMiddleCenterAreaOffensive3(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::ourOneThirdAreaDefensive2(int symmetry)
{
    draw(QString("ourOneThirdAreaDefensive2"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(-1.5,symmetry*-1);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;
        Circle2D aroundBall(wm->ball->pos,.5);
        Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
        Vector2D intersects[2];
        if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
            onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(false);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << onTheBallLine << pointForPass[0];
        cyclesToWait[0] << waitBeforeExecution/3 << 0;
        escapeRadius[0] << 0 << 0;
        facePoints[0] << TOOPPGOAL << TOBALL;

        if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
            staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        oneToucherID = positionAgents.last()->id();

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::ourOneThirdAreaDefensive2;
}
bool CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive2(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::ourOneThirdAreaOffensive2(int symmetry)
{
    middleFlankAreaOffensive2(symmetry);
    lastPlayExeced = &CDoubleSizeOurDirect::middleFlankAreaOffensive2;
}
bool CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive2(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::ourOneThirdAreaOffensive3(int symmetry)
{
    draw(QString("ourOneThirdAreaOffensive3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(wm->ball->pos.x+0.75,-1*symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;
        Circle2D aroundBall(wm->ball->pos,.5);
        Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
        Vector2D intersects[2];
        if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
            onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        Vector2D reboundPos1 = ((Vector2D(0,0)-wm->ball->pos)*4+wm->field->oppGoal()-wm->ball->pos)*.3+wm->ball->pos;
        staticVec[0] << reboundPos1 << reboundPos1+Vector2D(1,symmetry*-1);
        cyclesToWait[0] << waitBeforeExecution/2 << 300;
        escapeRadius[0] << .5 << .5;
        facePoints[0] << TOBALL << TOOPPGOAL;

        Vector2D reboundPos2 = (wm->field->oppGoal()-onTheBallLine)*.1+onTheBallLine+(Vector2D(0,0)-onTheBallLine)*.5;
        staticVec[1] << onTheBallLine << reboundPos2;
        cyclesToWait[1] << waitBeforeExecution/2 << 300;
        escapeRadius[1] << 0 << .3;
        facePoints[1] << TOOPPGOAL << TOOPPGOAL;

        staticVec[2] << pointForPass[0];
        cyclesToWait[2] << 200;
        escapeRadius[2] << 0;
        facePoints[2] << TOOPPGOAL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
        {
            staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
            staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
        } else
            staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

        setPosIntentions();
    }
    //	draw(onTheBallLine,D_MOHAMMED,"cyan");
    executedCycles++;

    if( executedCycles == waitBeforeExecution )
    {
        addNearestRobot2Mid(Vector2D(0,0));
        oneToucherID = positionAgents.last()->id();
        staticPoints.append(holdingPoints(positionAgents.at(getAgent(oneToucherID)),staticVec[getAgent(oneToucherID)],cyclesToWait[getAgent(oneToucherID)],escapeRadius[getAgent(oneToucherID)],facePoints[getAgent(oneToucherID)]));
    }

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");
    lastPlayExeced = &CDoubleSizeOurDirect::ourOneThirdAreaOffensive3;
}
bool CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive3(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::middleFlankAreaOnlyTwoAgents(int symmetry)
{
    middleCenterAreaOnlyTwoAgents(symmetry);
    lastPlayExeced = &CDoubleSizeOurDirect::middleFlankAreaOnlyTwoAgents;
}
bool CDoubleSizeOurDirect::conditionMiddleFlankAreaOnlyTwoAgents(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::middleCenterAreaOnlyTwoAgents(int symmetry)
{
    draw(QString("middleCenterAreaOnlyTwoAgents"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(wm->ball->pos.x-.4,-1*symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(true);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << pointForPass[0];
        cyclesToWait[0] << waitBeforeExecution*2;
        escapeRadius[0] << .3;
        facePoints[0] << TOOPPGOAL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        oneToucherID = positionAgents.last()->id();

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::middleCenterAreaOnlyTwoAgents;
}
bool CDoubleSizeOurDirect::conditionMiddleCenterAreaOnlyTwoAgents(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::theirOneThirdCenterArea2(int symmetry)
{
    draw(QString("theirOneThirdCenterArea2"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(wm->ball->pos.x+.2,-.5*symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(true);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << pointForPass[0];
        cyclesToWait[0] << waitBeforeExecution;
        escapeRadius[0] << 0;
        facePoints[0] << TOOPPGOAL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        oneToucherID = positionAgents.last()->id();

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::theirOneThirdCenterArea2;

}
bool CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::ourOneThirdAreaDefensive3(int symmetry)
{
    draw(QString("ourOneThirdAreaDefensive3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(-1,-1.5*symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;
        Circle2D aroundBall(wm->ball->pos,.5);
        Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
        Vector2D intersects[2];
        if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
            onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << Vector2D(1,symmetry*-1) << pointForPass[0];
        cyclesToWait[0] << waitBeforeExecution << 0;
        escapeRadius[0] << .5 << 0;
        facePoints[0] << TOBALL << TOBALL;

        staticVec[1] << onTheBallLine << Vector2D(2.5,symmetry*1.3);
        cyclesToWait[1] << waitBeforeExecution << 0;
        escapeRadius[1] << 0 << .5;
        facePoints[1] << TOOPPGOAL << TOBALL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
            staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));

        oneToucherID = positionAgents.first()->id();

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::middleFlankAreaDefensive3;
}
bool CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::swapNearestDefAndMid(Vector2D p, int midIdx)
{
    if( knowledge->defenseAgents.count() == 0 )
        return;
    QList<int> ourDefenses;
    for( int i = 0 ; i < knowledge->defenseAgents.count() ; i++ )
        ourDefenses.append(knowledge->defenseAgents.at(i)->id());
    int nearestDefense = knowledge->getNearestAgentToPoint(p,&ourDefenses);

    CAgent* defFront = knowledge->defenseAgents.at(nearestDefense);
    CAgent* posFront = positionAgents.at(midIdx);

    positionAgents.removeAt(midIdx);
    knowledge->defenseAgents.removeAt(nearestDefense);

    defFront->positionIntent.assign(defFront->id(),knowledge->frameCount);
    defFront->intention = &defFront->positionIntent;

    posFront->defIntent.assign(posFront->id(),knowledge->frameCount);
    posFront->intention = &posFront->defIntent;

    positionAgents.push_front(defFront);
    knowledge->defenseAgents.push_front(posFront);
}
void CDoubleSizeOurDirect::theirOneThirdFlankAreaAddDefense3(int symmetry)
{
    draw(QString("theirOneThirdFlankAreaAddDefense3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(3.5,symmetry*-1);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(true);
        playMakeRole.setNoKick(true);
        playMakeRole.setSpinBool(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << Vector2D(2.8,symmetry*.25) << Vector2D(2.59,symmetry*.092) << Vector2D(2.6,symmetry*-.203) << Vector2D(2.69,symmetry*-.512) << pointForPass[0];
        cyclesToWait[0] << 30 << 0 << 0 << 0 << 70;
        escapeRadius[0] << 0 << 0 << 0 << 0 << 0;
        facePoints[0] << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL ;

        staticVec[1] << Vector2D(2.8,symmetry*-.20) << Vector2D(2.8,0) << Vector2D(2.8,symmetry*.25);
        cyclesToWait[1] << 40 << 0 << 50;
        escapeRadius[1] << 0 << 0 << 0;
        facePoints[1] << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL;

        staticVec[2] << Vector2D(.3,symmetry*-1);
        cyclesToWait[2] << 200;
        escapeRadius[2] << .5;
        facePoints[2] << TOBALL;

        int nearestIdx = getNearestAgent(staticVec[1].first());debug(QString("id:%1").arg(positionAgents.at(nearestIdx)->id()),D_MOHAMMED);
        staticPoints.append(holdingPoints(positionAgents.at(1-nearestIdx),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        staticPoints.append(holdingPoints(positionAgents.at(nearestIdx),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
        oneToucherID = positionAgents.at(1-nearestIdx)->id();

        setPosIntentions();
    }

    foreach( Vector2D v , staticVec[0] )
        draw(v,D_MOHAMMED,"cyan");
    if( executedCycles == waitBeforeExecution  )
    {
        addNearestRobot2Mid(staticVec[2].first());
        staticPoints.append(holdingPoints(positionAgents.at(2),staticVec[2],cyclesToWait[2],escapeRadius[2],facePoints[2]));
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::theirOneThirdFlankAreaAddDefense3;
}
bool CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaAddDefense3(int symmetry)
{
    Rect2D emptyArea(Vector2D(1.7,.5),.3,1);

    for( int i = 0 ; i < wm->opp.activeAgentsCount() ; i++ )
    {
        if( emptyArea.contains(wm->opp[i]->pos) )
            return false;
    }

    return true;
}
void CDoubleSizeOurDirect::middleCenterAreaDefensive2(int symmetry)
{
    draw(QString("middleCenterAreaDefensive2"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(wm->ball->pos.x-.3,-1.5*symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << Vector2D(-0.1,symmetry*1.7);
        cyclesToWait[0] << 0;
        escapeRadius[0] << 0;
        facePoints[0] << TOBALL;

        staticVec[1] << pointForPass[0];
        cyclesToWait[1] << 200;
        escapeRadius[1] << 0;
        facePoints[1] << TOBALL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

        setPosIntentions();
    }

    executedCycles++;

    if( executedCycles == waitBeforeExecution && knowledge->defenseAgents.count() > 0 )
    {
        staticPoints.clear();
        swapNearestDefAndMid(pointForPass[0],getAgent(positionAgents.first()->id()));
        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
        oneToucherID = positionAgents.at(0)->id();
    }
    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurKickOff");

    lastPlayExeced = &CDoubleSizeOurDirect::middleCenterAreaDefensive2;
}
bool CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive2(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::ourOneThirdAreaOnlyTwoAgents(int symmetry)
{
    draw(QString("ourOneThirdAreaOnlyTwoAgents"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(wm->ball->pos.x+.4,-1*symmetry);
        waitBeforeExecution = 100;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(true);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << pointForPass[0];
        cyclesToWait[0] << waitBeforeExecution*2;
        escapeRadius[0] << .3;
        facePoints[0] << TOOPPGOAL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        oneToucherID = positionAgents.last()->id();

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::ourOneThirdAreaOnlyTwoAgents;
}
bool CDoubleSizeOurDirect::conditionOurOneThirdAreaOnlyTwoAgents(int symmetry)
{
    return true;
}
bool CDoubleSizeOurDirect::conditionourSimple(int symmetry)
{
    return true;
}
void CDoubleSizeOurDirect::ourSimple(int symmetry)
{
    draw(QString("ourSimple"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass[0] = Vector2D(wm->ball->pos.x-1,-2*symmetry);
        waitBeforeExecution = 100;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass[0]);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(true);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << pointForPass[0];
        cyclesToWait[0] << waitBeforeExecution*2;
        escapeRadius[0] << 0;
        facePoints[0] << TOOPPGOAL;

        staticVec[1] << Vector2D(wm->ball->pos.x-.4,2*symmetry);
        cyclesToWait[1] << waitBeforeExecution*2;
        escapeRadius[1] << 0;
        facePoints[1] << TOOPPGOAL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        oneToucherID = positionAgents.at(0)->id();
        staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &CDoubleSizeOurDirect::ourSimple;
}


void CDoubleSizeOurDirect::execute_0(){
    debug(QString("0"),D_HOSSEIN);
}


void CDoubleSizeOurDirect::execute_1(){
    debug(QString("1"),D_HOSSEIN);
}
#if 0
void CDoubleSizeOurDirect::execute_2(){
    if( cyclesToExecute > 0 )
    {
        runWithSymmetry((this->*lastPlayExeced));
        cyclesToExecute--;
    }
    else {
        if( wm->ball->pos.x > _FIELD_WIDTH/6 )
        {
            if( fabs(wm->ball->pos.y) > 1.5 ){
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            } else if( fabs(wm->ball->pos.y) < 1 ){
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            } else {
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            }
        } else if( wm->ball->pos.x > _FIELD_WIDTH*-1/6 )
        {
            if( fabs(wm->ball->pos.y) > 1.5 ){
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionMiddleFlankAreaOnlyTwoAgents);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionMiddleFlankAreaOnlyTwoAgents);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive2,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaOffensive2,&CDoubleSizeOurDirect::conditionMiddleFlankAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {//offensive plays with 2 defense
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive2,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaOffensive2,&CDoubleSizeOurDirect::conditionMiddleFlankAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            } else if( fabs(wm->ball->pos.y) < 1 ){
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionMiddleCenterAreaOnlyTwoAgents);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionMiddleCenterAreaOnlyTwoAgents);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaDefensive2,&CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaOffensive2,&CDoubleSizeOurDirect::conditionMiddleCenterAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {//offensive plays with 2 defense
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaDefensive2,&CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaOffensive2,&CDoubleSizeOurDirect::conditionMiddleCenterAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            } else {
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionMiddleCenterAreaOnlyTwoAgents);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionMiddleCenterAreaOnlyTwoAgents);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive2,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaOffensive2,&CDoubleSizeOurDirect::conditionMiddleFlankAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {//offensive plays with 2 defense
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive2,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaOffensive2,&CDoubleSizeOurDirect::conditionMiddleFlankAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            }
        } else
        {
            if( fabs(wm->ball->pos.y) > 1.5 ){
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {//offensive plays with 2 defense
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            } else if( fabs(wm->ball->pos.y) < 1 ){
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {//offensive plays with 2 defense]
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            } else {
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOnlyTwoAgents,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {//offensive plays with 2 defense
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive2,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            }
        }
    }
}

void CDoubleSizeOurDirect::execute_3(){
    if( cyclesToExecute > 0 )
    {
        runWithSymmetry((this->*lastPlayExeced));
        cyclesToExecute--;
    }
    else {
        if( wm->ball->pos.x > 3 )
        {//check whether f collides with execute_2 function
            if( fabs(wm->ball->pos.y) > 1.5 ){
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3) << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaSimple3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaSimple3);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3) << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaSimple3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaSimple3);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3) << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaSimple3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaSimple3);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaAddDefense3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaAddDefense3) << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaSimple3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaSimple3);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3) << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaSimple3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaSimple3);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3) << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaAddDefense3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaAddDefense3) << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaSimple3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaSimple3);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            } else if( fabs(wm->ball->pos.y) < 1 ){
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdCenterArea2,&CDoubleSizeOurDirect::conditionTheirOneThirdCenterArea2);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            } else {
                if( knowledge->defenseAgents.count() == 0 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else if( knowledge->defenseAgents.count() == 1 ) {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                } else {
                    defPlaysCan.clear();
                    defPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
                    defPlays = getValidPlays(defPlaysCan);
                    offPlaysCan.clear();
                    offPlaysCan << qMakePair(&CDoubleSizeOurDirect::theirOneThirdFlankAreaNoAdd3,&CDoubleSizeOurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
                    offPlays = getValidPlays(offPlaysCan);
                    decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                }
            }
        } else if( wm->ball->pos.x > _FIELD_WIDTH*-1/6 )
        {
            if( fabs(wm->ball->pos.y) > 1.5 )
            {
                if( lastPlayExeced == &CDoubleSizeOurDirect::middleFlankAreaOffensive2 || lastPlayExeced == &CDoubleSizeOurDirect::middleCenterAreaOffensive2 )
                    runWithSymmetry((this->*lastPlayExeced));
                else {
                    if( knowledge->defenseAgents.count() == 0 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else if( knowledge->defenseAgents.count() == 1 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaOffensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else {//plays with 2 defenses
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaOffensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    }
                }
            }
            else if( fabs(wm->ball->pos.y) < 1 )
            {
                if( lastPlayExeced == &CDoubleSizeOurDirect::middleFlankAreaOffensive2 || lastPlayExeced == &CDoubleSizeOurDirect::middleCenterAreaOffensive2 )
                    runWithSymmetry((this->*lastPlayExeced));
                else {
                    if( knowledge->defenseAgents.count() == 0 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else if( knowledge->defenseAgents.count() == 1 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaOffensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else {//plays with 2 defenses
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaOffensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    }
                }
            }
            else{//this is hysteresis
                if( lastPlayExeced == &CDoubleSizeOurDirect::middleFlankAreaOffensive3 || lastPlayExeced == &CDoubleSizeOurDirect::middleCenterAreaOffensive3 ){
                    runWithSymmetry((this->*lastPlayExeced));
                }
                else{//this is for the first time
                    //					if( fabs(wm->ball->pos.y) > 1.25 ) {
                    if( knowledge->defenseAgents.count() == 0 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else if( knowledge->defenseAgents.count() == 1 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaOffensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else {//plays with 2 defenses
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleFlankAreaOffensive3,&CDoubleSizeOurDirect::conditionMiddleFlankAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    }
                    //					}
                    //					else {
                    //						if( knowledge->defenseAgents.count() == 0 ) {
                    //							defPlaysCan.clear();
                    //							defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive3);
                    //							defPlays = getValidPlays(defPlaysCan);
                    //							offPlaysCan.clear();
                    //							offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive3);
                    //							offPlays = getValidPlays(offPlaysCan);
                    //							decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    //						} else if( knowledge->defenseAgents.count() == 1 ) {
                    //							defPlaysCan.clear();
                    //							defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive3);
                    //							defPlays = getValidPlays(defPlaysCan);
                    //							offPlaysCan.clear();
                    //							offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaOffensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaOffensive3);
                    //							offPlays = getValidPlays(offPlaysCan);
                    //							decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    //						} else {//plays with 2 defenses
                    //							defPlaysCan.clear();
                    //							defPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaDefensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaDefensive3);
                    //							defPlays = getValidPlays(defPlaysCan);
                    //							offPlaysCan.clear();
                    //							offPlaysCan << qMakePair(&CDoubleSizeOurDirect::middleCenterAreaOffensive3,&CDoubleSizeOurDirect::conditionMiddleCenterAreaOffensive3);
                    //							offPlays = getValidPlays(offPlaysCan);
                    //							decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    //						}
                    //					}
                }
            }
        } else {//check whether f collides with execute_2 function
            if( fabs(wm->ball->pos.y) > 1.5 ){
                if( lastPlayExeced == &CDoubleSizeOurDirect::middleFlankAreaOffensive2 )
                    runWithSymmetry((this->*lastPlayExeced));
                else {
                    if( knowledge->defenseAgents.count() == 0 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlamiddleCenterAreaDefensive2ysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else if( knowledge->defenseAgents.count() == 1 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else {//plays with 2 defenses
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    }
                }
            } else if( fabs(wm->ball->pos.y) < 1 ){
                if( lastPlayExeced == &CDoubleSizeOurDirect::middleFlankAreaOffensive2 )
                    runWithSymmetry((this->*lastPlayExeced));
                else {
                    if( knowledge->defenseAgents.count() == 0 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else if( knowledge->defenseAgents.count() == 1 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    }
                }
            } else {
                if( lastPlayExeced == &CDoubleSizeOurDirect::middleFlankAreaOffensive2 )
                    runWithSymmetry((this->*lastPlayExeced));
                else {
                    if( knowledge->defenseAgents.count() == 0 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else if( knowledge->defenseAgents.count() == 1 ) {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        defPlays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    } else {
                        defPlaysCan.clear();
                        defPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaDefensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaDefensive3);
                        def	debug(QString("Pos is: %1").arg(playOffLocation()),D_HOSSEIN);Plays = getValidPlays(defPlaysCan);
                        offPlaysCan.clear();
                        offPlaysCan << qMakePair(&CDoubleSizeOurDirect::ourOneThirdAreaOffensive3,&CDoubleSizeOurDirect::conditionOurOneThirdAreaOffensive3);
                        offPlays = getValidPlays(offPlaysCan);
                        decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
                    }
                }
            }
        }
    }
}

void CDoubleSizeOurDirect::execute_4(){
    if( cyclesToExecute > 0 )
    {
        runWithSymmetry((this->*lastPlayExeced));
        cyclesToExecute--;
    }
    else {
        if( wm->ball->pos.x > _FIELD_WIDTH/6 )
        {

        } else if( wm->ball->pos.x > _FIELD_WIDTH*-1/6 )
        {
            if( lastPlayExeced )
                runWithSymmetry((this->*lastPlayExeced));
        } else {
            if( lastPlayExeced )
                runWithSymmetry((this->*lastPlayExeced));
        }
    }
}

void CDoubleSizeOurDirect::execute_5(){
    if( cyclesToExecute > 0 )
    {
        runWithSymmetry((this->*lastPlayExeced));
        cyclesToExecute--;
    }
    else {
        if( wm->ball->pos.x > _FIELD_WIDTH/6 )
        {
        } else if( wm->ball->pos.x > _FIELD_WIDTH*-1/6 )
        {
            if( lastPlayExeced )
                runWithSymmetry((this->*lastPlayExeced));newMahiSimple
        } else {
        }
    }
}
#endif
void CDoubleSizeOurDirect::execute_2(){

}
void CDoubleSizeOurDirect::execute_3(){

}
void CDoubleSizeOurDirect::execute_4(){

}
void CDoubleSizeOurDirect::execute_5(){

}

void CDoubleSizeOurDirect::execute_6(){
    debug(QString("6"),D_HOSSEIN);
}
