#include "knowledge.h"
#include "gotopoint.h"
#include <varswidget.h>
#include <QTime>
#include <passevaluation.h>
#include <exceptions.h>
#include <geom/polygon_2d.h>

#include <mathtools.h>

#define _OBS_MARGIN 0.12
#define _GOAL_STEP  35 //number of steps on goal (in FindEmptyPosOnGoal)


QStringList CKnowledge::TechnicalModes = QStringList() << "" << "khers" << "khersdefense" ;


CKnowledge *knowledge;


CKnowledge::CKnowledge(CAgent** _agents)
{

    //ABBAS
    refShortcuts = false;
    shirjeBlocking = false;
    lastFrameShirjeBlock = -1;
    ourShirjeBlocker = -1;
    lastFrameShirjeZanDetected = -1;
    shirjezan = -1;
    khafanMarker = -1;
    khafanMarkFrame = 0;
    maxLoopTime = 0;
    fixedPassPoint.assign(0.0, 0.0);
    fixedPassPointDef.assign(0.0, 0.0);
    fixedPassPointsw.assign(0.0, 0.0);
    commandFrameRate = 100;
    matchdebug = false;
    khafanTime = 0;
    defenseClearingFrame = 0;
    loopTime = 0;
    agents = _agents;
    gamestate = lastgamestate = Stop;
    gameMode = Stop;
    attaking = false;
    m_ballOwner = -1;
    m_ballOurs = true;
    isSimulMode = false;
    roleOptimization = false;
    warmup = false;
    goalie = NULL;
    supporPlaymaker = Back;
    playMaker = NULL;
    defense_state = -1;
    lastBallOwner = -1;
    lastBallOurs = false;
    frameCount = 0;
    sentChipPass = false;
    sendingChipPass = false;
    chipPassSender = -1;
    closeGoalie = -1;
    khowf = -1;
    gameStarter = NULL;
    thePassProgress = false;
    passProgressTime = 0.0;
    currentPlayAllowedAgents = -1;
    lastTimeCommandFPSCalced = -1;
    cornerChipPhase = 0;
    technicalMode = "";
    selectedId = -1;
    selectedOur = true;
    lastFrameTheirNonPlayKick = -1;
    swapDA = false;
    switchState = 0;
    rushInPenaltyArea = false;
    plotWidgetCustom = 0;
    defenseClearer = -1;
    toBeSwitchedAttacker = -1;
    lastFrameSwithed = -1;
    ballVelLowPass = 0;
    //    variables["khafanmarked"]="false";
    lastPlayExecuted = Stop;
    necessaryDefKick = false;






    staticPoses.clear();
    for(int _i = 0 ; _i < 5 ; _i++)
    {
        for(int _j = -3 ; _j < 3 ; _j++)
        {
            if(_i == 4) {
                if(_j == -1) {
                    staticPoses.append(Vector2D(3.5, -1));
                }
                else if(_j == 0) {
                    staticPoses.append(Vector2D(3.5, 1));
                }
                else
                    staticPoses.append(Vector2D(_i, (float)(_j) + 0.5));
            }
            else
                staticPoses.append(Vector2D(_i, (float)(_j) + 0.5));
        }
    }


    //////////////////////////////fill ProfilerResult
    profiler = new CNewProfiler();
    QVector< QVector<double> > KickCoeff;
    QList< QVector<double> > ChipCoeff;

    CPolynomialRegression ProRes;
    CPolynomialFit Proregg;
    QList< QPair<double, double> > dataSet;


    QList<double> values , tempResult;
    QList<int> keys;
    QVector<double> coeffRes;
    double tempRes;

    for(int i=0; i< 16; i++)
        for(int j=0; j<4; j++)
            for(int k=0; k<81; k++)
                ProfilerResult[i][j][k] = -1000;

    refRobotID = 0;

    // initialize robotCoeff
    for(int i=0; i< 16; i++)
        for(int j=0; j<4; j++)
            RobotsCoeff[i][j]=1;

    // now fill proper data for RobotsCoeff : e.g. RobotsCoeff[3][0] = 1.32537;

    //    chip
    //    RobotsCoeff[0][1] = 0.22;
    //    RobotsCoeff[1][1] = 0.36;
    //    RobotsCoeff[2][1] = 0.4;
    //    RobotsCoeff[3][1] = 0.07;
    //    RobotsCoeff[4][1] = 0.22;
    //    RobotsCoeff[5][1] = -0.03;
    //    RobotsCoeff[6][1] = -0.15;
    //    RobotsCoeff[7][1] = 0.06;

    //  kick
    //    RobotsCoeff[0][0] = 710;
    //    RobotsCoeff[1][0] = 750;
    //    RobotsCoeff[2][0] = 940;
    //    RobotsCoeff[3][0] = 990;
    //    RobotsCoeff[4][0] = 780;
    //    RobotsCoeff[5][0] = 780;
    //    RobotsCoeff[6][0] = 780;
    //    RobotsCoeff[7][0] = 800;


    profiler->load(JSON, "Chip_2_5.json");

    //    ProfilerResult[robotID][0:kick , 1:chip , 2:SpinKick , 3:SpinChip][10*distance(0-80)] ---> contains needed voltage for this distance


    // kick regression

    for(int q=0; q<16; q++){
        values = profiler->robotsProfile[q].finalKickMap.values();
        values.insert(0 , 0);
        keys = profiler->robotsProfile[q].finalKickMap.keys();
        keys.insert(0 , 0);
        KickCoeff.append(ProRes.PolynomialRegression(values , keys, 2));
    }

    for(int q=0; q<16; q++){
        if(profiler->robotsProfile[q].finalKickMap.keys().size() > 0)
            if(KickCoeff.at(q).count()>2)
                for(int dis=0; dis<81; dis++){
                    tempRes = (double)
                            KickCoeff.at(q).at(0) +
                            KickCoeff.at(q).at(1)*((double)dis/10) +
                            KickCoeff.at(q).at(2)*((double)dis/10)*((double)dis/10);
                    if(tempRes != 0 )
                        ProfilerResult[q][0][dis] = tempRes;
                    else
                        ProfilerResult[q][0][dis] = -1000;
                }
    }


    // chip regression

    for(int q=0; q<16; q++){
        values.clear();
        keys.clear();
        dataSet.clear();
        coeffRes.clear();
        tempResult.clear();

        values = profiler->robotsProfile[q].finalChipMap.values();
        values.insert(0 , 0);
        keys = profiler->robotsProfile[q].finalChipMap.keys();
        keys.insert(0 , 0);

        for(int i=0; i<values.size(); i++){
            QPair<double, double> p;
            p.second = keys.at(i);
            p.first = values.at(i);
            dataSet.append(p);
        }

        ProRes.fitToDataSet(dataSet , 3);
        tempResult = ProRes.getCoefs();
        for(int i=0; i<tempResult.size(); i++){
            coeffRes.append(tempResult.at(i));
        }

        ChipCoeff.append(coeffRes);

        QString("coeff %5 : %1  , %2 , %3 , %4").arg(
                    ChipCoeff.at(q).at(0)).arg(ChipCoeff.at(q).at(1)).arg(
                    ChipCoeff.at(q).at(2)).arg(ChipCoeff.at(q).at(3)).arg(q);
    }

    for(int q=0; q<16; q++){
        if(profiler->robotsProfile[q].finalChipMap.keys().size() > 0)
            if(ChipCoeff.at(q).count()>3)
                for(int dis=0; dis<81; dis++){
                    tempRes = (double)
                            ChipCoeff.at(q).at(0) + ChipCoeff.at(q).at(1)*((double)dis/10) +
                            ChipCoeff.at(q).at(2)*((double)dis/10)*((double)dis/10) +
                            ChipCoeff.at(q).at(3)*((double)dis/10)*((double)dis/10)*((double)dis/10) ;
                    //                    if(dis < 30)
                    //                        debug(QString("reg res : %1 , %2").arg(dis).arg(tempRes) , D_FATEMEH);
                    if(tempRes != 0 )
                        ProfilerResult[q][1][dis] = tempRes;
                    else
                        ProfilerResult[q][1][dis] = -1000;
                }
    }

}

CKnowledge::~CKnowledge()
{
    delete profiler;
}
Vector2D CKnowledge::getStaticPoses(int num)
{
    if (num < staticPoses.size()) {
        return staticPoses.at(num);
    } else {
        return Vector2D::INVALIDATED;
    }

}

int CKnowledge::
getProfile(int agentId, double realParameter, bool isKick, bool spinOn ){

    double profiledParameter=0;
    int type;

    //    if(wm->getIsSimulMode()){
    //        return (int)realParameter;
    //    }

    if(isKick && !spinOn)
    {
        type =0;
    }
    else if(!isKick && !spinOn)
    {
        type =1;
    }
    else if(isKick && spinOn){
        type =2;
    }
    else if(!isKick && spinOn){
        type =3;
    }

    if(realParameter < 0) // dummy user
        return 0;

    if(agentId > 15)  // dummy user
        return 0;


    ///////////////////////// chip //////////////////////////////////////////////////
    if(type==1)
    {
        //        realParameter+=RobotsCoeff[agentId][1];

        profiledParameter = ProfilerResult[agentId][type][(int)round(realParameter*10)];

        if(profiledParameter != -1000)
        {
            if(profiledParameter > 1023)
                return 1023;
            else if(profiledParameter > 0){
                //                debug(QString("func : %1 , %2").arg(realParameter).arg(profiledParameter) , D_FATEMEH);
                return (int)profiledParameter;
            }
            else
                return 1;
        }
        else // no data is saved for this robot
        {
            if(ProfilerResult[refRobotID][type][(int)round(realParameter*10)] != -1000){    // get data from reference robot
                profiledParameter= RobotsCoeff[agentId][type] * knowledge->getProfile(refRobotID , realParameter , isKick , spinOn);
            }
            else{   // Linear
                profiledParameter= realParameter*128;
                if(type == 1)
                    profiledParameter*=2;
            }

            if(profiledParameter > 1023)
                return 1023;
            else if(profiledParameter > 0)
                return (int)profiledParameter;
            else{
                return 1;
            }

        }
    }
    ///////////////////////// kick //////////////////////////////////////////////////
    else if(type==0)
    {
        profiledParameter = ProfilerResult[agentId][type][(int)round(realParameter*10)];

        if(realParameter > 8.0)
            return RobotsCoeff[agentId][0];

        profiledParameter = ProfilerResult[agentId][type][(int)round(realParameter*10)];

        if(profiledParameter != -1000)
        {
            if(profiledParameter > 1023)
                return 1023;
            else if(profiledParameter > 0){
                return (int)profiledParameter;
            }
            else
                return 1;
        }
        else // no data is saved for this robot
        {
            debug("no data", D_FATEMEH);
            if(ProfilerResult[refRobotID][type][(int)round(realParameter*10)] != -1000){    // get data from reference robot
                profiledParameter= RobotsCoeff[agentId][type] * knowledge->getProfile(refRobotID , realParameter , isKick , spinOn);
            }
            else{   // Linear
                profiledParameter= realParameter*128;
            }

            if(profiledParameter > 1023)
                return 1023;
            else if(profiledParameter > 0)
                return (int)profiledParameter;
            else{
                return 1;
            }

        }

        profiledParameter= realParameter*128;

        if(profiledParameter > 1023)
            return 1023;
        else if(profiledParameter > RobotsCoeff[agentId][0])
            return RobotsCoeff[agentId][0];
        else if(profiledParameter > 0)
            return (int)profiledParameter;
        else{
            return 12;
        }
    }

}

void CKnowledge::calculateCommandFrameRate()
{
    double now = CProfiler::getTime();
    double fps = 1.0 / (now - lastTimeCommandFPSCalced);
    lastTimeCommandFPSCalced = now;
    lastFrameRates.append(fps);
    if (lastFrameRates.count() >= 10)
    {
        lastFrameRates.removeFirst();
        double f = 0;
        for (int i=0;i<lastFrameRates.count();i++)
        {
            f += 1.0 / lastFrameRates[i];
        }
        f /= lastFrameRates.count();
        commandFrameRate = 1.0 / f;
    }
}

QString CKnowledge::stateToString(State s)
{
    if (s==Halt) return "halt";
    if (s==Stop) return "stop";
    if (s==OurKickOff) return "our kick off";
    if (s==TheirKickOff) return "their kick off";
    if (s==OurDirectKick) return "our direct";
    if (s==TheirDirectKick) return "their direct";
    if (s==OurIndirectKick) return "our indirect";
    if (s==TheirIndirectKick) return "their indirect";
    if (s==OurPenaltyKick) return "our penalty";
    if (s==TheirPenaltyKick) return "their penalty";

    //added
    if (s==OurBallPlacement) return "our ballplacement";
    if (s==TheirBallPlacement) return "their ballplacement";

    if (s==Start) return "start";
    if (s==NormalStart) return "normal start";
    return "";
}

bool CKnowledge::isStop(){
    return getGameState() == CKnowledge::Stop
            && getGameMode() == CKnowledge::Stop;
}

bool CKnowledge::isStart(){
    return getGameState() == CKnowledge::Start
            && getGameMode() == CKnowledge::Start;
}

bool CKnowledge::isOurNonPlayOnKick()
{
    if (getGameState()==CKnowledge::OurDirectKick
            || getGameState()==CKnowledge::OurIndirectKick
            || getGameState()==CKnowledge::OurKickOff
            || getGameState()==CKnowledge::OurPenaltyKick
            || getGameMode()==CKnowledge::OurKickOff
            || getGameMode()==CKnowledge::OurPenaltyKick)
    {
        return true;
    }
    return false;
}

bool CKnowledge::isTheirNonPlayOnKick(){
    if (knowledge->getGameState()==CKnowledge::TheirDirectKick
            || knowledge->getGameState()==CKnowledge::TheirIndirectKick
            || knowledge->getGameState()==CKnowledge::TheirKickOff
            || knowledge->getGameState()==CKnowledge::TheirPenaltyKick)
    {
        return true;
    }
    return false;
}

void CKnowledge::updateGameState()
{
    lastgamestate = gamestate;
    if (wm->gs->gameOn())
        gamestate = Start;
    else if(joystick->getButton4() && joystick->isJsConnected())
        wm->gs->transition('H');
    else if(joystick->getButton3() && joystick->isJsConnected())
        wm->gs->transition('S');
    else if( joystick->getButton2() && joystick->isJsConnected())
    {
        gamestate = OurIndirectKick;
    }

    else if (!wm->gs->canMove()) gamestate = Halt;
    else if (wm->gs->ourKickoff()) {gamestate = OurKickOff;if (wm->gs->canKickBall()) gamestate = NormalStart;}
    else if (wm->gs->theirKickoff()) gamestate = TheirKickOff;
    else if (wm->gs->ourDirectKick()) gamestate = OurDirectKick;
    else if (wm->gs->theirDirectKick()) gamestate = TheirDirectKick;
    else if (wm->gs->ourIndirectKick()) gamestate = OurIndirectKick;
    else if (wm->gs->theirIndirectKick()) gamestate = TheirIndirectKick;
    else if (wm->gs->ourPenaltyKick()) {gamestate = OurPenaltyKick;if (wm->gs->canKickBall()) gamestate = NormalStart;}
    else if (wm->gs->theirPenaltyKick()) gamestate = TheirPenaltyKick;
    else if (wm->gs->ourBallPlacement()) gamestate = OurBallPlacement;//added
    else if (wm->gs->theirBallPlacement()) gamestate = TheirBallPlacement;//added
    else gamestate = Stop;
    gamestatechanged = (lastgamestate != gamestate);
    if (gamestatechanged) lasttimegschanged = currentTime();
    if (gamestatechanged)
    {
        if (gamestate==NormalStart)
        {
            if (lastgamestate==OurPenaltyKick) gameMode = OurPenaltyKick;
            else if (lastgamestate==OurKickOff) { gameMode = OurKickOff; attaking = true; }
            else gameMode = Start;
        }
        else if (gamestate==Start)
        {
            /*            if (lastgamestate==OurIndirectKick) { gameMode = OurIndirectKick;  attaking = true; }
            else if (lastgamestate==OurDirectKick) gameMode = OurDirectKick;
//            else if (lastgamestate==TheirDirectKick) gameMode = TheirDirectKick;
//            else if (lastgamestate==TheirIndirectKick) gameMode = TheirIndirectKick;
            else */
            gameMode = Start;
        }
        else if (gamestate==Stop)
        {
            gameMode = Stop;
        }
    }
    if( (gamestate == TheirKickOff || gamestate == TheirDirectKick || gamestate == TheirIndirectKick || gamestate == TheirPenaltyKick) ||
            (gameMode == TheirKickOff || gameMode == TheirDirectKick || gameMode == TheirIndirectKick || gameMode == TheirPenaltyKick))
    {
        lastFrameTheirNonPlayKick = frameCount;
    }
}

void CKnowledge::playChanged()
{
    lastFrameplaychanged = frameCount;
}

double CKnowledge::currentTime()
{
    return CProfiler::getTime();
}

long CKnowledge::getCurrentTime()
{
    return CProfiler::getKKTime();
}

CAgent* CKnowledge::getAgent(int i)
{
    if (i < 0 || i >= agentCount())
    {
        throw new EOutOfBound(i, 0, agentCount()-1, "CKnowledge::getAgent");
        return NULL;
    }
    return agents[i];
}

QList<CAgent*> CKnowledge::getActiveAgents()
{
    QList<CAgent*> q;
    for (int i=0;i<wm->our.activeAgentsCount();i++)
        q.append(knowledge->getAgent(wm->our.active(i)->id));
    return q;
}

int CKnowledge::agentCount()
{
    return _NUM_PLAYERS;
}

Vector2D CKnowledge::getPointInDirection(Vector2D firstPoint , Vector2D secondPoint , double proportion){
    //// This function gets a point along the lines that
    //// is made up by 2 points.The position of this point varies
    //// in according to the "ratio" that is given to the function.

    firstPoint = firstPoint + (secondPoint - firstPoint).norm() * proportion * (Segment2D(secondPoint , firstPoint).length());
    return firstPoint;
}
int CKnowledge::factorial(int a)
{
    int temp = 1;
    for(int i=1;i<=a;i++)
        temp=temp*i;
    return temp;
}

double CKnowledge::getRealBallVel()
{
    ballVelLowPass = 0.9 * ballVelLowPass + 0.1 *(((ballPosHis[0].dist(ballPosHis[1])/mainLoopTime) + (wm->ball->pos.dist(ballPosHis[0])/mainLoopTime) + (wm->ball->pos.dist(ballPosHis[1])/(mainLoopTime*2)))/3);

    ballPosHis[1] = ballPosHis[0];
    ballPosHis[0] = wm->ball->pos;
    draw(QString("New Ball Vel : %1").arg(ballVelLowPass,0, 'f', 2),Vector2D(-3,-3.5));
    return ballVelLowPass;
}

int CKnowledge::Matching(const QList <CAgent*> robots, const QList <Vector2D> pointsToMatch, QList <int> &matchPoints){
    QList <int> tempForMatch;
    tempForMatch.clear();
    for(int i = 0 ; i< robots.count() ; i++){
        tempForMatch.append(i);
    }
    double D = 100000000000;
    double tempD = 0;
    QList<QList <int> > combo = generateCombinations(tempForMatch);
    matchPoints.clear();
    if(robots.count() == pointsToMatch.count()){
        for(int i = 0 ; i < factorial(robots.count()) ; i++){
            tempD=0;
            for(int j=0 ; j < robots.count() ; j++)
            {
                tempD += pointsToMatch[combo[i][j]].dist(robots[j]->pos());
            }
            if(tempD < D)
            {
                D = tempD;
                matchPoints.clear();
                matchPoints.append( combo[i] );
            }
        }
        return 1;
    }
    else{
        return -1;
    }

}
void CKnowledge::fastMatching(const QList <CAgent*> robots, const QList <Vector2D> pointsToMatch, QList <int> &matchPoints)
{
    int nearestRobot[6] = {-1};
    double nearestDist = 1000000;

    matchPoints.clear();

    if(robots.count() == pointsToMatch.count())
    {
        for(int i = 0 ;i < pointsToMatch.count() ; i++)
        {
            nearestDist = 1000000;
            for(int j = 0 ; j < robots.count() ; j++)
            {
                if(i > 0) {
                    if(robots[j]->pos().dist(pointsToMatch[i]) < nearestDist && ( j != nearestRobot[i-1] ))
                    {
                        nearestDist = robots[j]->pos().dist(pointsToMatch[i]);
                        nearestRobot[i] =  j;
                    }
                }
                else
                {
                    if(robots[j]->pos().dist(pointsToMatch[i]) < nearestDist)
                    {
                        nearestDist = robots[j]->pos().dist(pointsToMatch[i]);
                        nearestRobot[i] =  j;
                    }
                }
            }

            if(nearestRobot>=0)
            {
                matchPoints.append(nearestRobot[i]);
            }
        }
    }
    else
    {
        return;
    }

}

Vector2D CKnowledge::getBestPosToShootToGoal(Vector2D from, double &regionWidth, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, bool oppGaol )
{
    Rect2D playingField(wm->field->ourCornerL(), wm->field->oppCornerR());
    if( ! playingField.contains(from) )
    {
        regionWidth = 0.0;
        goalProbablity = 0.0;
        shootPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
        return shootPos;
    }

    Vector2D goal;
    Vector2D goalL;
    Vector2D goalR;
    if( oppGaol )
    {
        goal = wm->field->oppGoal();
        goalL = wm->field->oppGoalL();
        goalR = wm->field->oppGoalR();
    }
    else
    {
        goal = wm->field->ourGoal();
        goalL = wm->field->ourGoalL();
        goalR = wm->field->ourGoalR();
    }

    double StepOnGoal = _GOAL_WIDTH / _GOAL_STEP;
    double MaxRegionWidth = 0 , MaxRegionTemp = 0;
    double BeginPos = 0 , EndPos = 0;
    Vector2D MaxRegionCenter(Vector2D::ERROR_VALUE , Vector2D::ERROR_VALUE) , RegionCenterTemp;
    bool WasLastPosClear = false;

    for( double y = goalR.y ; y <= goalL.y ; y += StepOnGoal ){
        Vector2D pos(goal.x , y);
        if( WasLastPosClear == false )
            BeginPos = y;

        if( this->isPointClear(pos, from, (CRobot::robot_radius_old + 2*CBall::radius), true, ourRelaxedIDs, oppRelaxedIDs) ){
            WasLastPosClear = true;
            //            draw(Segment2D(from,pos) , "white");
        }
        else
            WasLastPosClear = false;

        EndPos = y;
        if( WasLastPosClear ){
            RegionCenterTemp = Segment2D(goalL , goalR).intersection(Line2D(from,(from+Vector2D(Vector2D(goal.x,BeginPos)-from).rotate(Vector2D::angleBetween( Vector2D(goal.x,BeginPos)-from , Vector2D(goal.x,EndPos)-from).degree()/2))));
            if( RegionCenterTemp.x == Vector2D::ERROR_VALUE || RegionCenterTemp.y == Vector2D::ERROR_VALUE )
                RegionCenterTemp = Vector2D(goal.x , (EndPos - BeginPos)/2);
            MaxRegionTemp = (EndPos - BeginPos + 0.001)*from.dist(Line2D(goalL , goalR).projection(from))/from.dist(RegionCenterTemp);
            if( MaxRegionWidth < MaxRegionTemp ){
                MaxRegionWidth = MaxRegionTemp;
                MaxRegionCenter = RegionCenterTemp;
            }
        }
    }
    //    if( MaxRegionCenter.x != Vector2D::ERROR_VALUE  )
    //    {
    //        if( oppGaol )
    //        {
    //            regionWidth = (MaxRegionWidth / _GOAL_WIDTH) * 0.7 +
    //                          ((Vector2D::dirTo_deg(from,goalL) - Vector2D::dirTo_deg(from,goalR)) / 180.0) * 0.3;
    //            goalProbablity = regionWidth;
    //        }
    //        else
    //        {
    //            double dirL = Vector2D::dirTo_deg(from,goalL);
    //            dirL = dirL < 0.0? dirL + 360.0 : dirL;
    //            double dirR = Vector2D::dirTo_deg(from,goalR);
    //            dirR = dirR < 0.0? dirR + 360.0 : dirR;

    //            regionWidth = (MaxRegionWidth / _GOAL_WIDTH) * 0.7 +
    //                          ((dirR - dirL) / 180.0) * 0.3;
    //            goalProbablity = regionWidth;
    //        }
    //        shootPos = MaxRegionCenter;
    //        draw(Circle2D(shootPos, 0.05), 0, 360, "blue", true);
    //        return shootPos;
    //    }
    if( MaxRegionCenter.x != Vector2D::ERROR_VALUE && MaxRegionCenter.y != Vector2D::ERROR_VALUE ){
        regionWidth = MaxRegionWidth / _GOAL_WIDTH ;
        goalProbablity = regionWidth;
        shootPos = MaxRegionCenter;
        //        draw(Segment2D(from,shootPos) , "red");
        return shootPos;
    }
    regionWidth = 0.0;
    goalProbablity = 0.0;
    shootPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    return shootPos;
}

Vector2D CKnowledge::getBestPosForPassReciever(Rect2D searchRegion, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, Rect2D avoidRect)
{
    Vector2D bestPoint(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    Vector2D bestBigPoint(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);

    double x, y;
    double minProb = 0.0;
    double rectLeft = min(searchRegion.topLeft().x, searchRegion.bottomRight().x);
    double rectRight = max(searchRegion.bottomRight().x, searchRegion.topLeft().x);
    double rectBottom = min(searchRegion.bottomRight().y, searchRegion.topLeft().y);
    double rectTop = max(searchRegion.topLeft().y, searchRegion.bottomRight().y);

    for(x = rectRight; x>= rectLeft; x -= wm->passBigStepX() )
    {
        for(y = rectBottom; y <= rectTop; y += wm->passBigStepY() )
            //for(y = rectBottom; y <= rectTop; y += wm->passBigStepY() )
        {
            Vector2D bigPoint = Vector2D(x, y);
            double s1 = sign((bigPoint-wm->ball->pos).outerProduct(wm->field->oppGoalL()-wm->ball->pos));
            double s2 = sign((bigPoint-wm->ball->pos).outerProduct(wm->field->oppGoalR()-wm->ball->pos));
            if (s1*s2<0 && ((bigPoint-wm->field->oppGoal()).length()<(wm->ball->pos-wm->field->oppGoal()).length())) continue;
            if (avoidRect.contains(bigPoint))
                continue;
            if( ! this->isPointClear(bigPoint, wm->ball->pos, (CRobot::robot_radius_old + _OBS_MARGIN) * 1.5, true, ourRelaxedIDs, oppRelaxedIDs) )
            {
                continue;
            }
            double prob = 0.0;

            Vector2D temp = shootPos;
            getBestPosToShootToGoal(bigPoint, prob, ourRelaxedIDs, oppRelaxedIDs, true);
            shootPos = temp;
            if( prob > minProb )
            {
                minProb = prob;
                bestBigPoint = bigPoint;
            }

            draw(bigPoint, 1, QColor("white"));
        }
    }


    draw(bestBigPoint, 1, QColor("red"));

    if( ! bestBigPoint.valid() )
    {
        passPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
        return passPos;
    }

    minProb = 0.0;
    for( x = max(bestBigPoint.x - wm->passBigStepX(), rectLeft); x <= min(bestBigPoint.x + wm->passBigStepX(), rectRight);
         x += wm->passSmallStepX() )
    {
        for( y = max(bestBigPoint.y - wm->passBigStepY(), rectBottom); y <= min(bestBigPoint.y + wm->passBigStepY(), rectTop);
             y += wm->passSmallStepY() )
        {
            Vector2D smallPoint = Vector2D(x, y);
            if (avoidRect.contains(smallPoint))
                continue;
            if( ! this->isPointClear(smallPoint, wm->ball->pos, (CRobot::robot_radius_old + _OBS_MARGIN) * 1.5, true, ourRelaxedIDs, oppRelaxedIDs))
            {
                continue;
            }
            double prob = 0.0;
            Vector2D temp = shootPos;
            getBestPosToShootToGoal(smallPoint, prob, ourRelaxedIDs, oppRelaxedIDs, true);
            shootPos = temp;
            if( prob > minProb )
            {
                minProb = prob;
                bestPoint = smallPoint;
            }
            draw(smallPoint, 1, QColor("white"));
        }
    }
    passPos = bestPoint;
    return passPos;
}

QList<Vector2D> getListOfPointsOnPerpendicularLine(Vector2D Point, Vector2D Q, double lLength,int n){
    QList<Vector2D> points;
    points.clear();
    Vector2D perp;
    perp = (Point - Q).norm().rotate(90);
    for(double h=(-lLength/2.0); h<lLength/2.0; h+=lLength/(double)n){
        points.append(Point + perp * h);
    }
    return points;
}

long double f1(long double &tot, long double a, long double b)
{
    tot += b;
    if (a>1.0) a=1.0;
    if (a<0.0) a=0.0;
    return a*b;
}

long double f1(long double a, long double b)
{
    if (a>1.0) a=1.0;
    if (a<0.0) a=0.0;
    return a*b;
}

long double fm1(long double a, long double b)
{
    long double r = a/b;
    if (r>1.0) r = 1.0;
    if (r<0.0) r = 0.0;
    return r;
}

Vector2D CKnowledge::findPosition(QList<Rect2D> searchRegions,  QList<Rect2D> avoidRects, int agent, int passSenderID)
{
    QList<int> ourRelaxedIDs;
    QList<int> oppRelaxedIDs;
    double stepX[2], stepY[2];
    stepX[0] = wm->passBigStepX();
    stepY[0] = wm->passBigStepY();
    stepX[1] = wm->passSmallStepX();
    stepY[1] = wm->passSmallStepY();
    Vector2D passSenderPos;
    Vector2D passSenderDir;
    Vector2D passRecieverPos = wm->our[agent]->pos;
    Vector2D passRecieverDir = wm->our[agent]->dir;
    if (passSenderID != -1)
    {
        passSenderPos = wm->our[passSenderID]->pos;
        passSenderDir  = wm->our[passSenderID]->dir;
    }
    else
    {
        passSenderPos.invalidate();
    }
    ourRelaxedIDs.append(passSenderID);
    ourRelaxedIDs.append(agent);
    Vector2D result;
    double bestprob = -1.0;
    for (int jj=0;jj<searchRegions.count();jj++)
    {
        draw(searchRegions[jj], "red", false);
        double rectLeft   = searchRegions[jj].left();
        double rectRight  = searchRegions[jj].right();
        double rectTop    = searchRegions[jj].top();
        double rectBottom = searchRegions[jj].bottom();
        double maxProb = -1;
        Vector2D bestPoint(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
        for (int k=0;k<2;k++)
        {
            for (double x = rectLeft; x <= rectRight; x += stepX[k])
            {
                for (double y = rectBottom; y <= rectTop; y += stepY[k])
                {
                    bool flag = false;

                    Vector2D point(x, y);
                    for(int ii=0;ii<avoidRects.count();ii++){
                        if (avoidRects[ii].contains(point))
                            flag = true;
                    }
                    if (flag) continue;

                    long double prob = 0.0;

                    double probForShooting = 0;
                    double probForRecieving = 0;
                    long double senderAngleFactor = 0;
                    long double senderDistanceFactor = 0;
                    long double recieverAngleFactor = 0;
                    long double recieverDistanceFactor = 0;
                    long double senderVisibleAngleFactor = 0;
                    long double recieverVisibleAngleFactor = 0;
                    long double oppsDistFactor = 0;
                    long double oneTouchAngleFactor = 0;
                    long double ballVisibilityFactor = 0;
                    long double passLineFactor = 0;

                    getEmptyPosOnGoal(point, probForShooting, true, ourRelaxedIDs, oppRelaxedIDs, 1.0);

                    if(this->isPointClear(point,wm->ball->pos,CRobot::robot_radius_old + 2*CBall::radius,true,ourRelaxedIDs, oppRelaxedIDs)){
                        ballVisibilityFactor = 1.0;
                    }
                    else
                    {
                        ballVisibilityFactor = 0.0;
                    }

                    if(passSenderPos.valid()){
                        QList<Vector2D> plist;
                        plist = getListOfPointsOnPerpendicularLine(point, passSenderPos,CRobot::robot_radius_old * 4.0, 10);
                        this->getEmptyPosOnPoints(passSenderPos, probForRecieving, plist, ourRelaxedIDs, oppRelaxedIDs);

                        senderVisibleAngleFactor = AngleDeg::normalize_angle(
                                    Vector2D::dirTo_deg(passSenderPos,plist.front()) -
                                    Vector2D::dirTo_deg(passSenderPos,plist.back())) / 180.0;

                        senderVisibleAngleFactor = fabs(senderVisibleAngleFactor);

                        senderAngleFactor   = 1.0 - fabs(AngleDeg::normalize_angle((passSenderDir.th().degree()   - Vector2D::dirTo_deg(passSenderPos,point))))/360.0;

                        senderDistanceFactor = (passSenderPos - point).length()/(hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width()));
                        double oneTouchAng = fabs((passSenderPos - point).th().degree() - (wm->field->oppGoal() - point).th().degree());
                        if(oneTouchAng <= 45)
                        {
                            oneTouchAngleFactor = 0.9*(oneTouchAng/45.0);
                        }
                        else if(oneTouchAng <= 90)
                        {
                            oneTouchAngleFactor = 0.9 + 0.1*(90-oneTouchAng)/45;
                        }
                        else
                        {
                            oneTouchAngleFactor = 0;
                        }
                    }
                    else
                    {
                        probForRecieving = 1.0;
                        senderVisibleAngleFactor = 1.0;
                        senderAngleFactor = 1.0;
                        senderDistanceFactor = 1.0;
                        oneTouchAngleFactor = 1.0;
                    }

                    recieverVisibleAngleFactor = AngleDeg::normalize_angle(
                                Vector2D::dirTo_deg(point,wm->field->oppGoalL()) -
                                Vector2D::dirTo_deg(point,wm->field->oppGoalR())) / 180.0;

                    recieverVisibleAngleFactor = fabs(recieverVisibleAngleFactor);

                    if(passRecieverPos.valid()){
                        recieverDistanceFactor = 1.0 - (passRecieverPos - point).length()/
                                (searchRegions[jj].topLeft()-searchRegions[jj].bottomRight()).length();
                    }
                    else
                    {
                        recieverDistanceFactor = 1;
                    }

                    if((passRecieverPos.valid()))
                    {
                        recieverAngleFactor = 1.0 - fabs(AngleDeg::normalize_angle((passRecieverDir.th().degree() - Vector2D::dirTo_deg(point,passSenderPos))))/360.0;
                    }
                    else
                    {
                        recieverAngleFactor = 1.0;
                    }

                    oppsDistFactor = 1.0;

                    for(int i=0; i<wm->opp.activeAgentsCount();i++){
                        oppsDistFactor *= (pow((wm->opp.active(i)->pos - point).length()/(hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width())),1));
                    }

                    Segment2D passLine(wm->ball->pos, point);
                    Vector2D  passVec = point - wm->ball->pos;
                    double len   = passVec.length();
                    double minDist = 1e10;
                    for(int i=0; i<wm->opp.activeAgentsCount();i++){
                        double prod = passVec.innerProduct(wm->opp.active(i)->pos - wm->ball->pos);
                        if(0<=prod && prod<=len*len){
                            double dist = passLine.dist(wm->opp.active(i)->pos);
                            if(dist < minDist) minDist = dist;
                        }
                    }

                    if(minDist > (hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width())))
                    {
                        passLineFactor = 1.0;
                    }
                    else
                    {
                        passLineFactor = 4.0*(minDist / (hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width())));
                    }
                    if(passLineFactor < 0) passLineFactor = 0;

                    prob = 0.0;
                    double f = 1.0;
                    long double tot=0.0;
                    prob += f1(tot,probForShooting,5.0*f);
                    prob += f1(tot,probForRecieving,0.2*f);
                    prob += f1(tot,recieverDistanceFactor,0.1*f);
                    prob += f1(tot,senderAngleFactor,0.2*f);
                    prob += f1(tot,recieverAngleFactor,0.1*f);
                    prob += f1(tot,senderVisibleAngleFactor,0.2*f);
                    prob += f1(tot,recieverVisibleAngleFactor,2.0*f);
                    prob += f1(tot,senderDistanceFactor,0.1*f);
                    prob += f1(tot,oneTouchAngleFactor,0.2*f);
                    prob  = fm1(prob,tot);
                    if (prob > 1) prob = 1;
                    if( prob > maxProb )
                    {
                        maxProb = prob;
                        bestPoint = point;
                    }
                    draw(Rect2D(point+Vector2D(-0.080,-0.080),point+Vector2D(+0.080,+0.080)),QColor(255*prob, 255*prob, 255*prob,255),true);
                }
            }
            draw(Rect2D(bestPoint+Vector2D(-0.080,-0.080),bestPoint+Vector2D(+0.080,+0.080)),QColor("red"),false);

            rectLeft   = max(bestPoint.x - stepX[k], rectLeft);
            rectRight  = min(bestPoint.x + stepX[k], rectRight);
            rectTop    = min(bestPoint.y + stepY[k], rectTop);
            rectBottom = max(bestPoint.y - stepY[k], rectBottom);
        }
        if (maxProb>bestprob)
        {
            bestprob = maxProb;
            result   = bestPoint;
        }
    }
    return result;
}

Vector2D CKnowledge::getBestPosForPassReciever(QList<Rect2D> searchRegions, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, QList<Rect2D> avoidRects,int passSenderID,int passRecieverID,double angleFactor,double angle0)
{
    bool newMethod = true;//(passRecieverID!=-1);
    if(newMethod)
    {
        Vector2D passSenderPos;
        Vector2D passSenderDir;
        if(passSenderID != -1)
        {
            passSenderPos = wm->our[passSenderID]->pos;
            passSenderDir  = wm->our[passSenderID]->dir;
            ourRelaxedIDs.append(passSenderID);
        }
        else
        {
            passSenderPos.invalidate();
        }

        Vector2D passRecieverPos;
        Vector2D passRecieverDir;
        if(passRecieverID != -1)
        {
            passRecieverPos = wm->our[passRecieverID]->pos;
            passRecieverDir = wm->our[passRecieverID]->dir;
            ourRelaxedIDs.append(passRecieverID);
        }
        else
        {
            passRecieverPos.invalidate();
        }

        Vector2D bestOfBestPoints(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
        double maxOfMaxProbs=-1;
        Vector2D bestBigPoint(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
        for(int jj=0;jj<searchRegions.count();jj++)
        {
            Vector2D bestPoint(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
            bestOfBestPoints.invalidate();

            double x, y;
            double maxProb = -1;
            double rectLeft = min(searchRegions[jj].topLeft().x, searchRegions[jj].bottomRight().x);
            double rectRight = max(searchRegions[jj].bottomRight().x, searchRegions[jj].topLeft().x);
            double rectBottom = min(searchRegions[jj].bottomRight().y, searchRegions[jj].topLeft().y);
            double rectTop = max(searchRegions[jj].topLeft().y, searchRegions[jj].bottomRight().y);

            for(x = rectRight; x>= rectLeft; x -= wm->passBigStepX() )
            {
                for(y = rectBottom; y <= rectTop; y += wm->passBigStepY() )
                {
                    bool flag = false;
                    Vector2D bigPoint = Vector2D(x, y);
                    for(int ii=0;ii<avoidRects.count();ii++){
                        if (avoidRects[ii].contains(bigPoint))
                            flag = true;
                    }
                    if (flag) continue;

                    long double prob = 0.0;

                    double probForShooting = 0;
                    double probForRecieving = 0;
                    long double senderAngleFactor = 0;
                    long double senderDistanceFactor = 0;
                    long double recieverAngleFactor = 0;
                    long double recieverDistanceFactor = 0;
                    long double senderVisibleAngleFactor = 0;
                    long double recieverVisibleAngleFactor = 0;
                    long double oppsDistFactor = 0;
                    long double oneTouchAngleFactor = 0;
                    long double ballVisibilityFactor = 0;
                    long double passLineFactor = 0;

                    getBestPosToShootToGoal(bigPoint, probForShooting, ourRelaxedIDs, oppRelaxedIDs, true);

                    if(this->isPointClear(bigPoint,wm->ball->pos,CRobot::robot_radius_old,true,ourRelaxedIDs, oppRelaxedIDs)){
                        ballVisibilityFactor = 1.0;
                    }
                    else
                    {
                        ballVisibilityFactor = 0.0;
                    }

                    if(passSenderPos.valid()){
                        QList<Vector2D> plist;
                        plist = getListOfPointsOnPerpendicularLine(bigPoint, passSenderPos,CRobot::robot_radius_old * 4.0, 10);
                        this->getEmptyPosOnPoints(passSenderPos, probForRecieving, plist, ourRelaxedIDs, oppRelaxedIDs);

                        senderVisibleAngleFactor = AngleDeg::normalize_angle(
                                    Vector2D::dirTo_deg(passSenderPos,plist.front()) -
                                    Vector2D::dirTo_deg(passSenderPos,plist.back())) / 180.0;

                        senderVisibleAngleFactor = fabs(senderVisibleAngleFactor);

                        senderAngleFactor   = 1.0 - fabs(AngleDeg::normalize_angle((passSenderDir.th().degree()   - Vector2D::dirTo_deg(passSenderPos,bigPoint))))/360.0;

                        senderDistanceFactor = (passSenderPos - bigPoint).length()/(hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width()));
                        double oneTouchAng = fabs((passSenderPos - bigPoint).th().degree() - (wm->field->oppGoal() - bigPoint).th().degree());
                        if(oneTouchAng <= 45)
                        {
                            oneTouchAngleFactor = (oneTouchAng/45.0);
                        }
                        else if(oneTouchAng <= 90)
                        {
                            oneTouchAngleFactor = 1;
                        }
                        else
                        {
                            oneTouchAngleFactor = 0;
                        }
                    }
                    else
                    {
                        probForRecieving = 1;
                        senderVisibleAngleFactor = 1;
                        senderAngleFactor = 1.0;
                        senderDistanceFactor = 1.0;
                        oneTouchAngleFactor = 1.0;
                    }

                    recieverVisibleAngleFactor = AngleDeg::normalize_angle(
                                Vector2D::dirTo_deg(bigPoint,wm->field->oppGoalL()) -
                                Vector2D::dirTo_deg(bigPoint,wm->field->oppGoalR())) / 180.0;

                    recieverVisibleAngleFactor = fabs(recieverVisibleAngleFactor);

                    if(passRecieverPos.valid()){
                        recieverDistanceFactor = 1 - (passRecieverPos - bigPoint).length()/(hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width()));
                    }
                    else
                    {
                        recieverDistanceFactor = 1;
                    }

                    if((passRecieverPos.valid()))
                    {
                        recieverAngleFactor = 1.0 - fabs(AngleDeg::normalize_angle((passRecieverDir.th().degree() - Vector2D::dirTo_deg(bigPoint,passSenderPos))))/360.0;
                    }
                    else
                    {
                        recieverAngleFactor = 1.0;
                    }

                    oppsDistFactor = 1.0;

                    for(int i=0; i<wm->opp.activeAgentsCount();i++){
                        oppsDistFactor *= (pow((wm->opp.active(i)->pos - bigPoint).length()/(hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width())),1));
                    }

                    Segment2D passLine(wm->ball->pos, bigPoint);
                    Vector2D  passVec = bigPoint - wm->ball->pos;
                    double len   = passVec.length();
                    double minDist = 1e10;
                    for(int i=0; i<wm->opp.activeAgentsCount();i++){
                        double prod = passVec.innerProduct(wm->opp.active(i)->pos - wm->ball->pos);
                        if(0<=prod && prod<=len*len){
                            double dist = passLine.dist(wm->opp.active(i)->pos);
                            if(dist < minDist) minDist = dist;
                        }
                    }

                    if(minDist > (hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width())))
                    {
                        passLineFactor = 1.0;
                    }
                    else
                    {
                        passLineFactor = 4.0*(minDist / (hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width())));
                    }
                    if(passLineFactor < 0) passLineFactor = 0;

                    prob = 0.0;
                    {
                        double f = 1.0;//3.0/16.3;//3.0/11.6;
                        prob += f1(probForShooting,2.0*f);
                        prob += f1(probForRecieving,2.0*f);
                        prob += f1(recieverDistanceFactor,0.1*f);
                        prob += f1(senderAngleFactor,1.0*f);
                        prob += f1(recieverAngleFactor,0.1*f);
                        prob += f1(senderVisibleAngleFactor,1.0*f);
                        prob += f1(recieverVisibleAngleFactor,5.0*f);
                        prob += f1(oppsDistFactor,1.0*f);
                        prob += f1(senderDistanceFactor,0.1*f);
                        prob += f1(oneTouchAngleFactor,3.0*f);
                        prob += f1(ballVisibilityFactor,3.0*f);
                        prob += f1(passLineFactor,2.0*f);
                        prob  = fm1(prob,20.3*f);
                    }

                    if( prob > maxProb )
                    {
                        maxProb = prob;
                        bestBigPoint = bigPoint;
                    }

                    double pp = (prob > 1.0) ? 1.0 : prob;

                    draw(Rect2D(bigPoint+Vector2D(-0.080,-0.080),bigPoint+Vector2D(+0.080,+0.080)),QColor(255*pp, 255*pp, 255*pp,20),true);
                }
            }
            draw(Rect2D(bestBigPoint+Vector2D(-0.080,-0.080),bestBigPoint+Vector2D(+0.080,+0.080)),QColor("red"),false);


            if( ! bestBigPoint.valid() )
            {
                passPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
                continue;
            }

            maxProb = -1;

            for( x = max(bestBigPoint.x - wm->passBigStepX(), rectLeft); x <= min(bestBigPoint.x + wm->passBigStepX(), rectRight); x += wm->passSmallStepX() )
            {
                for( y = max(bestBigPoint.y - wm->passBigStepY(), rectBottom); y <= min(bestBigPoint.y + wm->passBigStepY(), rectTop); y += wm->passSmallStepY() )
                {
                    Vector2D smallPoint = Vector2D(x, y);

                    bool flag = false;
                    for(int ii=0;ii<avoidRects.count();ii++){
                        if (avoidRects[ii].contains(smallPoint))
                            flag = true;
                    }
                    if (flag) continue;

                    long double prob = 0.0;

                    double probForShooting = 0;
                    double probForRecieving = 0;
                    long double senderAngleFactor = 0;
                    long double senderDistanceFactor = 0;
                    long double recieverAngleFactor = 0;
                    long double recieverDistanceFactor = 0;
                    long double senderVisibleAngleFactor = 0;
                    long double recieverVisibleAngleFactor = 0;
                    long double oppsDistFactor = 0;
                    long double oneTouchAngleFactor = 0;
                    long double ballVisibilityFactor = 0;
                    long double passLineFactor = 0;

                    getBestPosToShootToGoal(smallPoint, probForShooting, ourRelaxedIDs, oppRelaxedIDs, true);

                    if(this->isPointClear(smallPoint,wm->ball->pos,CRobot::robot_radius_old,true,ourRelaxedIDs,oppRelaxedIDs)){
                        ballVisibilityFactor = 1.0;
                    }
                    else
                    {
                        ballVisibilityFactor = 0.0;
                    }

                    if(passSenderPos.valid()){
                        QList<Vector2D> plist;
                        plist = getListOfPointsOnPerpendicularLine(smallPoint, passSenderPos,CRobot::robot_radius_old * 4.0, 10);
                        this->getEmptyPosOnPoints(passSenderPos, probForRecieving, plist, ourRelaxedIDs, oppRelaxedIDs);

                        senderVisibleAngleFactor = AngleDeg::normalize_angle(
                                    Vector2D::dirTo_deg(passSenderPos,plist.front()) -
                                    Vector2D::dirTo_deg(passSenderPos,plist.back())) / 180.0;

                        senderVisibleAngleFactor = fabs(senderVisibleAngleFactor);

                        senderAngleFactor   = 1.0 - fabs(AngleDeg::normalize_angle((passSenderDir.th().degree()   - Vector2D::dirTo_deg(passSenderPos,smallPoint))))/360.0;

                        senderDistanceFactor = (passSenderPos - smallPoint).length()/(hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width()));
                        double oneTouchAng = fabs((passSenderPos - smallPoint).th().degree() - (wm->field->oppGoal() - smallPoint).th().degree());
                        if(oneTouchAng <= 45)
                        {
                            oneTouchAngleFactor = (oneTouchAng/45.0);
                        }
                        else if(oneTouchAng <= 90)
                        {
                            oneTouchAngleFactor = 1;
                        }
                        else
                        {
                            oneTouchAngleFactor = 0;
                        }
                    }
                    else
                    {
                        probForRecieving = 1;
                        senderVisibleAngleFactor = 1;
                        senderAngleFactor = 1.0;
                        senderDistanceFactor = 1.0;
                        oneTouchAngleFactor = 1.0;
                    }

                    recieverVisibleAngleFactor = AngleDeg::normalize_angle(
                                Vector2D::dirTo_deg(smallPoint,wm->field->oppGoalL()) -
                                Vector2D::dirTo_deg(smallPoint,wm->field->oppGoalR())) / 180.0;

                    recieverVisibleAngleFactor = fabs(recieverVisibleAngleFactor);

                    if(passRecieverPos.valid()){
                        recieverDistanceFactor = 1 - (passRecieverPos - smallPoint).length()/(hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width()));
                    }
                    else
                    {
                        recieverDistanceFactor = 1;
                    }

                    if((passRecieverPos.valid()))
                    {
                        recieverAngleFactor = 1.0 - fabs(AngleDeg::normalize_angle((passRecieverDir.th().degree() - Vector2D::dirTo_deg(smallPoint,passSenderPos))))/360.0;
                    }
                    else
                    {
                        recieverAngleFactor = 1.0;
                    }

                    oppsDistFactor = 1.0;

                    for(int i=0; i<wm->opp.activeAgentsCount();i++){
                        oppsDistFactor *= (pow((wm->opp.active(i)->pos - smallPoint).length()/(hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width())),1));
                    }


                    Segment2D passLine(wm->ball->pos, smallPoint);
                    Vector2D  passVec = smallPoint - wm->ball->pos;
                    double len   = passVec.length();
                    double minDist = 1e10;
                    for(int i=0; i<wm->opp.activeAgentsCount();i++){
                        double prod = passVec.innerProduct(wm->opp.active(i)->pos - wm->ball->pos);
                        if(0<=prod && prod<=len*len){
                            double dist = passLine.dist(wm->opp.active(i)->pos);
                            if(dist < minDist) minDist = dist;
                        }
                    }


                    if(minDist > (hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width())))
                    {
                        passLineFactor = 1.0;
                    }
                    else
                    {
                        passLineFactor = 4.0*(minDist / (hypot(searchRegions[jj].size().length(),searchRegions[jj].size().width())));
                    }
                    if(passLineFactor < 0) passLineFactor = 0;


                    prob = 0.0;
                    {
                        double f = 1.0;//3.0/11.6;
                        prob += f1(probForShooting,2.0*f);
                        prob += f1(probForRecieving,2.0*f);
                        prob += f1(recieverDistanceFactor,0.1*f);
                        prob += f1(senderAngleFactor,1.0*f);
                        prob += f1(recieverAngleFactor,0.1*f);
                        prob += f1(senderVisibleAngleFactor,1.0*f);
                        prob += f1(recieverVisibleAngleFactor,5.0*f);
                        prob += f1(oppsDistFactor,1.0*f);
                        prob += f1(senderDistanceFactor,0.1*f);
                        prob += f1(oneTouchAngleFactor,3.0*f);
                        prob += f1(ballVisibilityFactor,3.0*f);
                        prob += f1(passLineFactor,2.0*f);
                        prob  = fm1(prob,20.3*f);
                    }

                    if( prob > maxProb )
                    {
                        maxProb = prob;
                        bestPoint = smallPoint;
                    }

                    double pp = prob;

                    draw(Rect2D(smallPoint+Vector2D(-0.020,-0.020),smallPoint+Vector2D(+0.020,+0.020)),QColor(0xFF*pp,0xFF*pp,0xFF*pp,20),true);
                }
            }
            draw(Rect2D(bestPoint+Vector2D(-0.020,-0.020),bestPoint+Vector2D(+0.020,+0.020)),QColor(0xFF*0.5,0,0),true);
            if(maxProb > maxOfMaxProbs){
                maxOfMaxProbs = maxProb;
                bestOfBestPoints = bestPoint;
            }
        }

        if(!bestOfBestPoints.valid())
        {
            passPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
            return Vector2D::INVALIDATED;
        }
        draw(Rect2D(bestOfBestPoints+Vector2D(-0.020,-0.020),bestOfBestPoints+Vector2D(+0.020,+0.020)),QColor(64,0,0),false);

        passPos = bestOfBestPoints;

        return passPos;
    }
    else
    {
        Vector2D pos0 = wm->ball->pos;
        Vector2D bestOfBestPoints(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
        double minOfMinProbs=0;
        for(int jj=0;jj<searchRegions.count();jj++)
        {
            Vector2D bestPoint(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
            Vector2D bestBigPoint(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);

            double x, y;
            double minProb = 0.0;
            double rectLeft = min(searchRegions[jj].topLeft().x, searchRegions[jj].bottomRight().x);
            double rectRight = max(searchRegions[jj].bottomRight().x, searchRegions[jj].topLeft().x);
            double rectBottom = min(searchRegions[jj].bottomRight().y, searchRegions[jj].topLeft().y);
            double rectTop = max(searchRegions[jj].topLeft().y, searchRegions[jj].bottomRight().y);

            for(x = rectRight; x>= rectLeft; x -= wm->passBigStepX() )
            {
                for(y = rectBottom; y <= rectTop; y += wm->passBigStepY() )
                {
                    bool flag = false;
                    Vector2D bigPoint = Vector2D(x, y);
                    for(int ii=0;ii<avoidRects.count();ii++){
                        if (avoidRects[ii].contains(bigPoint))
                            flag = true;
                    }
                    if (flag) continue;
                    if (passSenderID!=-1)
                    {
                        if ((wm->our[passSenderID]->pos - bigPoint).length()<2.000) continue;
                    }
                    if (fabs(AngleDeg::normalize_angle(
                                 ((wm->field->oppGoalL()-bigPoint).th().degree() - (wm->field->oppGoalR()-bigPoint).th().degree())))<5.0)
                        continue;


                    double prob = 0.0;

                    Vector2D temp = shootPos;
                    getBestPosToShootToGoal(bigPoint, prob, ourRelaxedIDs, oppRelaxedIDs, true);
                    if (prob<0.1) continue;
                    if (passSenderID!=-1)
                    {
                        double t = angle0/180.0;
                        double x = fabs(AngleDeg::normalize_angle(((pos0 - bigPoint).th().degree() - (wm->field->oppGoal() - bigPoint).th().degree()))) / 180.0;
                        double c = (3*(t*t)-2*t) / (2*t-1);
                        double A = 1.0 / (t*(t-1.0)*(t-c));
                        double f = A*x*(x-1)*(x-c);
                        prob += angleFactor*f;
                    }
                    shootPos = temp;
                    if( prob > minProb )
                    {
                        minProb = prob;
                        bestBigPoint = bigPoint;
                    }

                    draw(bigPoint, 1, QColor("white"));
                }
            }
            draw(bestBigPoint, 1, QColor("red"));
            if( ! bestBigPoint.valid() )
            {
                passPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
                continue;
            }

            minProb = 0.0;
            for( x = max(bestBigPoint.x - wm->passBigStepX()*2.0, rectLeft); x <= min(bestBigPoint.x + wm->passBigStepX()*2.0, rectRight);
                 x += wm->passSmallStepX() )
            {
                for( y = max(bestBigPoint.y - wm->passBigStepY(), rectBottom); y <= min(bestBigPoint.y + wm->passBigStepY(), rectTop);
                     y += wm->passSmallStepY() )
                {
                    Vector2D smallPoint = Vector2D(x, y);
                    bool flag = false;
                    for(int ii=0;ii<avoidRects.count();ii++){
                        if (avoidRects[ii].contains(smallPoint))
                            flag = true;
                    }
                    if (flag) continue;
                    if (fabs(AngleDeg::normalize_angle(
                                 ((wm->field->oppGoalL()-smallPoint).th().degree() - (wm->field->oppGoalR()-smallPoint).th().degree())))<10.0)
                        continue;

                    Vector2D d1=(smallPoint-pos0);
                    Vector2D d2=(wm->field->oppGoalL()-pos0);
                    Vector2D d3=(wm->field->oppGoalR()-pos0);
                    if (d1.outerProduct(d2)*d1.outerProduct(d3)<0)
                        continue;
                    double prob = 0.0;
                    Vector2D temp = shootPos;
                    getBestPosToShootToGoal(smallPoint, prob, ourRelaxedIDs, oppRelaxedIDs, true);
                    if (prob<0.3) continue;
                    shootPos = temp;
                    double t = angle0/180.0;
                    double x = fabs(AngleDeg::normalize_angle(((pos0 - smallPoint).th().degree() - (wm->field->oppGoal() - smallPoint).th().degree()))) / 180.0;
                    double c = (3*(t*t)-2*t) / (2*t-1);
                    double A = 1.0 / (t*(t-1.0)*(t-c));
                    double f = A*x*(x-1)*(x-c);
                    prob += angleFactor*f;
                    prob = 1-x;
                    if (prob>0.75) continue;
                    if( prob > minProb )
                    {
                        minProb = prob;
                        bestPoint = smallPoint;
                    }
                    draw(smallPoint, 1, QColor("cyan"));
                }
            }
            if(minProb > minOfMinProbs){
                minOfMinProbs = minProb;
                bestOfBestPoints = bestPoint;
            }
        }
        if(!bestOfBestPoints.valid())
        {
            passPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
            return Vector2D::INVALIDATED;
        }

        passPos = bestOfBestPoints;

        return passPos;
    }
}

Vector2D CKnowledge::getBestPosForPassThrough(int throughlevel, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs)
{
    const int PassThroughMaxLevel = 4;
    Rect2D region(wm->field->oppGoal().x * ((double)(throughlevel - 1))/(double)(PassThroughMaxLevel), wm->field->oppCornerR().y, wm->field->oppGoal().x * (double)(throughlevel)/(double)(PassThroughMaxLevel), wm->field->oppCornerL().y);
    return getBestPosForPassReciever(region, ourRelaxedIDs, oppRelaxedIDs, Rect2D(Vector2D::INVALIDATED, Vector2D::INVALIDATED));
}

int CKnowledge::getNearestAgentToPoint(Vector2D point, QList<int>* agentIDs)
{
    if (agentIDs==NULL)
    {
        double minLength = 1e10;
        int nearest = -1;
        for (int i=0;i<wm->our.activeAgentsCount();i++)
        {
            double dist;
            if ((dist=(wm->our.active(i)->pos - point).length())<minLength)
            {
                minLength = dist;
                nearest = wm->our.active(i)->id;
            }
        }
        return nearest;
    }
    else
    {
        double minLength = 1e10;
        int nearest = -1;
        for (int i=0;i<agentIDs->count();i++)
        {
            double dist;
            if ((*agentIDs)[i]==-1) continue;
            if ((dist=(wm->our[(*agentIDs)[i]]->pos - point).length())<minLength)
            {
                minLength = dist;
                nearest = i;
            }
        }
        return nearest;
    }
}

int CKnowledge::getNearestOppToPoint(Vector2D point)
{
    double minDist = 1.0e13;
    int nearest = -1;
    for(int i = 0; i < wm->opp.activeAgentsCount(); i++ )
    {
        if (wm->opp.active(i)->inSight <=0)
            continue;
        double dist = (wm->opp.active(i)->pos - point).length();
        if( dist < minDist )
        {
            minDist = dist;
            nearest = wm->opp.active(i)->id;
        }
    }
    return nearest;
}

int CKnowledge::findNearestopp()
{
    double dist=_INF, d;
    int nearest = -1;
    for ( int i = 0; i < wm->opp.activeAgentsCount(); i++)
    {
        d = wm->ball->pos.dist( wm->opp.active(i)->pos);
        if ( d < dist)
        {
            dist = d;
            nearest = wm->opp.active(i)->id;
        }
    }
    return nearest;
}

int CKnowledge::findOppGoalie()
{
    //TODO: boolean
    Rect2D goalRect (wm->field->oppGoalL(), _GOAL_DEPTH, _GOAL_WIDTH);
    int oppGoalieIndex = -1;
    for(int i = 0; i < wm->opp.activeAgentsCount(); i++ )
    {
        if (wm->opp.active(i)->inSight <=0)
            continue;
        if( wm->field->isInOppPenaltyArea(wm->opp.active(i)->pos) || goalRect.contains(wm->opp.active(i)->pos) ){
            if( oppGoalieIndex != -1 ){
                draw("Double defender!" , Vector2D(1.2,2.2) , "red" , 18);
                if( wm->opp.active(i)->pos.dist(wm->field->oppGoal()) < wm->opp[oppGoalieIndex]->pos.dist(wm->field->oppGoal()) )
                    oppGoalieIndex = wm->opp.active(i)->id;
            }
            else
                oppGoalieIndex = wm->opp.active(i)->id;
        }
    }
    return oppGoalieIndex;
}

bool CKnowledge::isCrowdedInFrontOfAgent(int id, double radius)
{
    Vector2D pos = wm->our[id]->pos;
    Vector2D dir = wm->our[id]->dir;
    bool flag=false;
    oppBlockers.clear();
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
        Vector2D opp=wm->opp.active(i)->pos;
        if (((pos-opp).length() < (radius+2*CRobot::robot_radius_new)) &&
                (fabs(AngleDeg::normalize_angle(Vector2D::dirTo_deg(pos,opp)-dir.th().degree())) < 60))
        {
            oppBlockers.append(wm->opp.active(i)->id);
            flag = true;
        }
    }
    return flag;
}

bool CKnowledge::isCrowdedInFrontOfBallOwner(int ballowner)
{
    return (this->isCrowdedInFrontOfAgent(ballowner,CRobot::robot_radius_old*2.0) &&
            (wm->ball->pos - wm->our[ballowner]->pos).length() < (CRobot::robot_radius_old+CBall::radius+50));
}

void CKnowledge::findDangerousOpps(QList<int> &opps)
{
    opps.clear();
    Rect2D rect( wm->field->ourCornerL(), Vector2D(0, wm->field->ourCornerR().y));
    for (int i=0; i < wm->opp.activeAgentsCount(); i++)
    {
        if( rect.contains(wm->opp.active(i)->pos) && wm->opp.active(i)->inSight > 0.0)
            opps.append(wm->opp.active(i)->id);
    }
    opps.removeOne( oppBallOwner);
}

int CKnowledge::findKickerAgent()
{
    double dist;
    double minDist = _INF;
    int id = -1;
    for (int i=0; i < wm->opp.activeAgentsCount(); i++)
    {
        dist = wm->opp.active(i)->pos.dist(wm->ball->pos);
        if( minDist == _INF || dist < minDist)
        {
            minDist = dist;
            id = i;
        }
    }
    return id;
}

int CKnowledge::findOppBallOwner()
{
    for (int i=0; i < wm->opp.activeAgentsCount(); i++)
    {
        if (((wm->opp.active(i)->pos-wm->ball->pos).length() < CRobot::robot_radius_old + CBall::radius + 0.03))
            //(fabs(AngleDeg::normalize_angle(wm->opp.active(i)->dir.th().degree() -  Vector2D::dirTo_deg(wm->opp.active(i)->pos,wm->ball->pos))) < 20 ))
        {
            oppBallOwner = wm->opp.active(i)->id;
            return oppBallOwner;
        }
    }
    oppBallOwner = -1;
    return -1;
}

int CKnowledge::findOurBallOwner()
{
    for (int i=0; i < wm->our.activeAgentsCount(); i++)
    {
        if (((wm->our.active(i)->pos-wm->ball->pos).length() < CRobot::robot_radius_old + CBall::radius + 0.03) &&
                (fabs(AngleDeg::normalize_angle(wm->our.active(i)->dir.th().degree() -  Vector2D::dirTo_deg(wm->our.active(i)->pos,wm->ball->pos))) < 20 ))
        {
            ballOwner = wm->our.active(i)->id;
            return ballOwner;
        }
    }
    ballOwner = -1;
    return -1;
}

void CKnowledge::findBallOwners()
{
    findOurBallOwner();
    if( ballOwner == -1)
        findOppBallOwner();
    else
        oppBallOwner = -1;
}

float CKnowledge::oneTouchAngleThreshold()
{
    return conf()->Coach_oneTouchAngleThreshold();
}

float CKnowledge::oneTouchKickThreshold()
{
    return conf()->Coach_oneTouchKickThreshold();
}

float CKnowledge::kickThreshold()
{
    return conf()->Coach_kickThreshold();
}

float CKnowledge::kickClosedAngle()
{
    return conf()->Coach_kickClosedAngle();
}

Vector2D CKnowledge::getReflectPos(Vector2D goal, double dist)
{
    Vector2D res;
    Vector2D ballPos(wm->ball->pos);
    Segment2D dummySeg(goal,goal+Vector2D(-5,0));
    Vector2D nearest(dummySeg.nearestPoint(ballPos));
    Vector2D sol1,sol2;
    Rect2D oppField(0,_FIELD_HEIGHT/2 -0.01,_FIELD_WIDTH/2 + 0.01,_FIELD_HEIGHT -0.01);
    Circle2D oppCircle(Vector2D(_FIELD_WIDTH/2,0)- Vector2D(1,0),dist);



    res.x = nearest.x;
    res.y = nearest.y*2 - ballPos.y;

    dummySeg.assign(wm->field->oppGoal(),(res-wm->field->oppGoal()).norm()*12);
    //    oppField.intersection(dummySeg,&sol1,&sol2);
    oppCircle.intersection(dummySeg,&sol1,&sol2);
    if(wm->field->isInField(sol1))
        res = sol1;
    else
        res = sol2;



    return res;
}

Vector2D CKnowledge::onetouchablity(int agentId, double &goalWidth, double &angle, double &coming, int senderId, double underestimateTheirGoalie)
{
    Vector2D target = goalVisiblity(agentId, goalWidth, underestimateTheirGoalie);
    if (senderId==-1)
    {
        if (wm->ball->vel.length()<0.1)
        {
            angle = fabs(Vector2D::angleBetween(wm->ball->pos-agents[agentId]->pos(), (target - agents[agentId]->pos())).degree());
        }
        else angle = fabs(Vector2D::angleBetween(-wm->ball->vel, (target - agents[agentId]->pos())).degree());
    }
    else
        angle = fabs(Vector2D::angleBetween((agents[senderId]->pos() - agents[agentId]->pos()), (target - agents[agentId]->pos())).degree());
    coming = agents[agentId]->self()->ballComingSpeed();
    draw(Circle2D(target, 0.05), 0, 360, "red", 1);
    return target;
}

Vector2D CKnowledge::goalVisiblity(int agentId, double &regionWidth, double underestimateTheirGoalie)
{
    QList<int> relax,empty;
    relax.append(agentId);
    Vector2D target = getEmptyPosOnGoal(agents[agentId]->pos(), regionWidth, true, relax, empty, underestimateTheirGoalie);
    if (!target.valid())
        target = wm->field->oppGoal();
    draw(Circle2D(target, 0.05), 0, 360, "red", 1);
    return target;
}

Vector2D CKnowledge::getEmptyPosOnGoal(Vector2D from, double &regionWidth, bool oppGoal, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, double wOpenness, bool _draw){
    QList<Circle2D> c;
    for(int i=0;i<wm->our.activeAgentsCount();i++){
        if (!ourRelaxedIDs.contains(wm->our.active(i)->id)){
            c.append(Circle2D(wm->our.active(i)->pos, wm->our.active(i)->robotRadius()));
        }
    }
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
        if (!oppRelaxedIDs.contains(wm->opp.active(i)->id))
        {
            c.append(Circle2D(wm->opp.active(i)->pos, wm->opp.active(i)->robotRadius()));
        }
    }
    Vector2D goalL, goalR;
    if (oppGoal){goalL = wm->field->oppGoalL();goalR = wm->field->oppGoalR();}
    else {goalL = wm->field->ourGoalL();goalR = wm->field->ourGoalR();}
    double angle=0.0, biggestangle=0.0;
    getEmptyAngle(from, goalL, goalR, c, regionWidth, angle, biggestangle, oppGoal, _draw);
    double goalWidth = fabs(Vector2D::angleBetween(goalL - from, goalR - from).degree());
    regionWidth = wOpenness * regionWidth + (1.0-wOpenness) * goalWidth / 180.0;
    //    if (wOpenness > 0.5)
    //        regionWidth = regionWidth * goalWidth / 30.0;
    //	debug(QString("regionWidth : %1 wOpenness : %2").arg(regionWidth).arg(wOpenness),D_SEPEHR);
    if (regionWidth > 1.0) regionWidth = 1.0;
    Vector2D p = Segment2D(goalL, goalR).intersection(Line2D(from, AngleDeg(angle)));
    if (p.valid()) return p;
    return (goalL + goalR) / 2.0;
}
Vector2D CKnowledge::getEmptyPosOnPoints(Vector2D from, double &regionWidth, QList<Vector2D> points, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs)
{
    QList<double> regions;
    QList<Vector2D> regionPoints;
    QList<Vector2D> regionsCenterPoint;

    bool wasLatPosClear=false;
    int regionCnt = 0;
    int i = 0;

    double totalDist = 0.0;

    for( int i = 0; i < points.size(); i++)
    {
        if( i >= 1 )
            totalDist += points[i].dist(points[i-1]);
        Vector2D pos = points[i];
        Segment2D l = Segment2D(from, pos);

        if( isPointClear(pos, from, CRobot::robot_radius_old, true, ourRelaxedIDs, oppRelaxedIDs) && (i != points.size() - 1) )
        {
            if( wasLatPosClear )
            {
                regions[regionCnt - 1] += points[i].dist(points[i-1]);
                regionPoints.append(pos);
            }
            else
            {
                regionCnt++;
                regions.append(0.0);
                regionPoints.append(pos);
            }
            wasLatPosClear = true;
        }
        else
        {

            if (i == points.size() - 1 && isPointClear(pos, from, CRobot::robot_radius_old, true, ourRelaxedIDs, oppRelaxedIDs) && wasLatPosClear )
            {
                if( ! regions.isEmpty() )
                {
                    if( regionCnt > 0 )
                    {
                        regions[regionCnt - 1] += points[i].dist(points[i-1]);
                    }
                    else
                    {
                        regions[0] += points[i].dist(points[i-1]);
                    }
                }
                regionPoints.append(pos);
            }

            if(wasLatPosClear == true )
            {
                Vector2D sum;
                int j;
                for( j = 0; j < regionPoints.size(); j++ )
                {
                    sum += regionPoints[j];
                }
                regionsCenterPoint.append(sum / regionPoints.size());
                regionPoints.clear();
            }
            wasLatPosClear = false;
        }

    }
    double maxRegionWidth = 0;
    int biggestIndex = 0;
    for(i = 0; i < regionsCenterPoint.size(); i++ )
    {
        draw(regionsCenterPoint[i],1,QColor("blue"));
        if( regions[i] > maxRegionWidth )
        {
            maxRegionWidth = regions[i];
            biggestIndex = i;
        }
    }
    if( ! regionsCenterPoint.isEmpty() )
    {
        regionWidth = regions[biggestIndex] / totalDist;
        return regionsCenterPoint[biggestIndex];
    }
    regionWidth = 0.0;
    return Vector2D::INVALIDATED;
}

bool CKnowledge::isPointClear(Vector2D point, Vector2D from, double rad, bool considerRelaxedIDs, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs){
    Vector2D posIntersect1(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    Vector2D posIntersect2(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    Segment2D l(from, point);
    for(int i = 0; i < wm->opp.activeAgentsCount(); i++){
        if((wm->opp.active(i)->inSight > 0.0)){
            if(considerRelaxedIDs && oppRelaxedIDs.contains(wm->opp.activeAgentID(i))){
                continue;
            }
            Circle2D c(wm->opp.active(i)->pos, rad);
            if(c.intersection(l,&posIntersect1, &posIntersect2) != 0){
                return false;
            }
        }
    }
    for(int i = 0; i < wm->our.activeAgentsCount(); i++){
        if (wm->our.active(i)->inSight > 0.0){
            if(considerRelaxedIDs && ourRelaxedIDs.contains(wm->our.activeAgentID(i))){
                continue;
            }
            Circle2D c(wm->our.active(i)->pos, rad);
            if(c.intersection(l,&posIntersect1, &posIntersect2) != 0){
                return false;
            }
        }
    }
    return true;
}

bool CKnowledge::isPointClear(Vector2D point, Vector2D from, double radBig, double radSmall, bool considerRelaxedIDs, QList<int>ourRelaxedIDs, QList<int>oppRelaxedIDs, QList<int>ourSmallIDs, QList<int>oppSmallIDs)
{
    Vector2D posIntersect1(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
    Vector2D posIntersect2(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);

    Segment2D l(from, point);
    for (int i = 0; i < wm->opp.activeAgentsCount(); i++)
    {

        if ((wm->opp.active(i)->inSight > 0.0))
        {
            if(considerRelaxedIDs && oppRelaxedIDs.contains(wm->opp.activeAgentID(i)))
                continue;
            double r = radBig;
            if (oppSmallIDs.contains(wm->opp.activeAgentID(i)))
                r = radSmall;
            Circle2D c(wm->opp.active(i)->pos, r);
            if(c.intersection(l,&posIntersect1, &posIntersect2) != 0 )
            {
                return false;
            }
        }
    }

    for (int i = 0; i < wm->our.activeAgentsCount(); i++)
    {
        if (wm->our.active(i)->inSight > 0.0)
        {
            if(considerRelaxedIDs && ourRelaxedIDs.contains(wm->our.activeAgentID(i)))
                continue;
            double r = radBig;
            if (ourSmallIDs.contains(wm->our.activeAgentID(i)))
                r = radSmall;
            Circle2D c(wm->our.active(i)->pos, r);
            if(c.intersection(l,&posIntersect1, &posIntersect2) != 0 )
            {
                return false;
            }
        }
    }
    return true;
}

Vector2D CKnowledge::getBestShadowPoint(Vector2D /*pos*/, Vector2D /*goal*/)
{
    //    Vector2D tt =
    //            //pos // + Vector2D((-CRobot::robot_radius_old-50),170.0*((pos.y>0) ? -1 : 1));
    //            pos - (goal-pos).norm()*170;
    //    tt.d = Vector2D::dirTo_deg(tt,goal);
    //    return tt;
    //return  pos + Vector2D((-CRobot::robot_radius_old-50)* ((ball.pos.y>0) ? 1 : -1),170.0);
    //    Vector2D target = (goal - pos).norm()*(-CRobot::robot_radius_old-50) -
    //               (goal - pos).norm().rotatedVector(90)*170.0 + pos;
    //    if (!field->isInField(target))
    //        target = pos*2.0 - target;
    //    target.d = 0;
    //    return target;
    return Vector2D::INVALIDATED;
}

//TODO: move this function
Vector2D CKnowledge::findBestPosToCatchTheBall(int /*agentIDI*/, Vector2D& /*lastBestPos*/)
{
    return Vector2D::INVALIDATED;
    //    if (ball.vel.length()<100)
    //    {
    //        Vector2D t;
    //        if (ball.vel.length()<10)
    //            t = ball.pos - (ball.pos - ourTeam[agentID].pos).norm()*(CBall::radius+_CENTER_FROM_KICKER);
    //        else t = ball.pos + ball.vel.norm()*(CBall::radius+_CENTER_FROM_KICKER);
    //        t.d = Vector2D::dirTo_deg(t, ball.pos);
    //        return t;
    //    }
    //    if (ballBuffer.count()<=3)
    //    {
    //        lastBestPos = ball.pos - (ourTeam[agentID].pos-ball.pos).norm()*(CBall::radius+_CENTER_FROM_KICKER);
    //        return lastBestPos;
    //    }
    //    const int stepCount[2] = {10,10};
    //    Vector2D pos0 = ball.pos;
    //    Vector2D pos1 = ballDir*5000 + pos0;
    //    double t0,t1;
    //    Vector2D bestpos = pos0;
    //    double mindt = 1e5;
    ///*
    //    //TODO: complete this code
    //     //binary search
    //    for (int j=0;j < 8;j++)
    //    {
    //        Vector2D posm1 = (pos1 - pos0)*0.25 + pos0;
    //        double trm1 = motionEstimator->estimate(ourTeam[agentID].pos, Vector2D(0,0), pos0, Vector2D(0,0));
    //        double tbm1 = whenIsBallAt((posm1-ball.pos).innerProduct(ballDir));
    //        double tm1 = fabs(tbm1-trm1);
    //        double searchStep = (pos1-pos0).length()/((double)stepCount[j]);
    //        int besti = -1;
    //        for (int i=0;i < stepCount[j];i++)
    //        {
    //            pos = pos + ballDir * searchStep;
    //            double tr = motionEstimator->estimate(ourTeam[agentID].pos, Vector2D(0,0), pos0, Vector2D(0,0));
    //            double tb = whenIsBallAt((pos-ball.pos).innerProduct(ballDir));
    //            if ((tb > tr) && (tb > 0) && (tr > 0))
    //            {
    //                if (tb - tr < mindt)
    //                {
    //                    mindt = tb - tr;
    //                    bestpos = pos;
    //                    besti = i;
    //                }
    //            }
    //            draw(pos);
    //            //qDebug() << tr << "," << tb;
    //        }
    //        if (besti==-1)
    //        {
    //            if (lastBestPos.valid())
    //                return lastBestPos;
    //        }
    //        pos0 = bestpos - ballDir * searchStep;
    //        pos1 = bestpos + ballDir * searchStep;
    //    }*/
    ////    double err=0;
    ////    for (int i=0;i<ballBuffer.count();i++)
    ////    {
    ////        double t= ballTimeBuffer[i]-ballTimeBuffer.first();
    ////        double p = t*t*ballModelC2 + t*ballModelC1 + ballModelC0;
    ////        double q = (ballBuffer[i].pos-ballBuffer.first().pos).length();
    ////        err += (p-q)*(p-q)*0.001*0.001;
    ////    }
    ////    debug(QString("Error==%1").arg(err));
    //    if (ballModelC2>=0)
    //    {
    //        double vball = ball.vel.length();
    //        if (ball.vel.innerProduct(ourTeam[agentID].pos-ball.pos)>0 && vball>100.0)
    //        {
    //            bestpos = getProjectionOfPointOnBallVeclocityDirection(ourTeam[agentID].pos);
    //            bestpos -= (ball.pos-bestpos).norm()*(CBall::radius+_CENTER_FROM_KICKER);
    //            bestpos.d = Vector2D::dirTo_deg(bestpos, ball.pos);
    //            lastBestPos = bestpos;
    //            return bestpos;
    //        }
    //    }
    //    for (int j=0;j < 2;j++)
    //    {
    //        Vector2D pos = pos0;
    //        double searchStep = (pos1-pos0).length()/((double)stepCount[j]);
    //        int besti = -1;
    //        int secondi = -1;
    //        double minfdt = 1e6;
    //        Vector2D secondpos;
    //        double ltr,ltb,ldist;
    //        ldist = -1.0;
    //        for (int i=0;i < stepCount[j];i++)
    //        {
    //            pos = pos + ballDir * searchStep;
    //            pos.d = Vector2D::dirTo_deg(pos,ball.pos);
    //            if ( !Rect2D(field->ourCornerR(), field->oppCornerL()).contains(pos) )
    //                continue;
    //            double tr;
    ///*            if ((ourTeam[agentID].pos - pos).length()<200)
    //                tr = 0;
    //            else*/
    //                tr = motionEstimator->estimate(ourTeam[agentID].pos, Vector2D(0,0), pos, Vector2D(0,0)) * 1.2;
    //            double dist = (pos-ball.pos).length();// .innerProduct(ballDir);
    //            double tb = whenIsBallAt(dist);
    //            if (ldist!=-1.0)
    //            {
    //                draw(Segment2D(dist,tr*500.0,ldist,ltr*500.0),QColor("blue"));
    //                draw(Segment2D(dist,tb*500.0,ldist,ltb*500.0),QColor("red"));
    //            }
    //            ltr = tr;
    //            ltb = tb;
    //            ldist = dist;
    //            if (fabs(tb - tr)<minfdt)
    //            {
    //                minfdt = fabs(tb - tr);
    //                secondpos = pos;
    //                secondi = i;
    //            }
    //            if ((tb > tr) && (tb > 0) && (tr > 0))
    //            {
    ///*                if (tb - tr < mindt)
    //                {
    //                    mindt = tb - tr;
    //                    bestpos = pos;
    //                    besti = i;
    //                }*/
    //                bestpos = pos;
    //                besti = i;
    //                break;
    //            }
    //            if (tb==-1)
    //                draw(pos,1,QColor("black"));
    //            else
    //                draw(pos);
    //            //qDebug() << dist << ":" << tr << "," << tb;
    //        }
    //        if (besti==-1)
    //        {
    //            if (secondi==-1)
    //            {
    //                if (lastBestPos.valid())
    //                    return lastBestPos;
    //            }
    //            else
    //            {
    //                bestpos = secondpos;
    //            }
    //        }
    //        pos0 = bestpos - ballDir * searchStep;
    //        pos1 = bestpos + ballDir * searchStep;
    //    }
    //    if (ballModelC2<0)
    //    {
    //        if ((bestpos-ball.pos).length()>(ballStopPos-ball.pos).length())
    //            bestpos = ballStopPos;
    //    }
    ////    if ((bestpos-ball.pos).length()<(lastBestPos-ball.pos).length())
    ////    {
    ////        bestpos = lastBestPos;
    ////    }
    //    bestpos -= (ball.pos-bestpos).norm()*(CBall::radius+_CENTER_FROM_KICKER);
    //    bestpos.d = Vector2D::dirTo_deg(bestpos, ball.pos);
    //    lastBestPos = bestpos;
    //    return bestpos;
}

bool CKnowledge::canSendPass(int sender, int receiver, Vector2D point, double factor)
{
    if (!point.valid()) point = agents[receiver]->self()->getKickerPos();
    QList<int> relaxed,empty;
    relaxed.append(sender);
    if (receiver!=-1)
        relaxed.append(receiver);
    return knowledge->isPointClear(point,agents[sender]->self()->getKickerPos(),
                                   CRobot::robot_radius_old*factor
                                   ,true,relaxed,empty);
}

bool CKnowledge::isBallOurs()
{
    if (frameCount - lastFrameTheirNonPlayKick < 60)
    {
        draw("Forcing Mark", Vector2D(0.0, -1.5), "red");
        return false; //after their kicks remain in mark
    }
    if( passEval.inProgress() == true && thePassProgress == false )
    {
        passProgressTime = CProfiler::getTime();
    }
    if ((CProfiler::getTime() - passProgressTime) < 0.5) {
        ballOurs = true;
        return true;
    }
    thePassProgress = passEval.inProgress();
    if (!ownerShipCalculated)
    {
        ballOurs = false;
        bool ours;
        getBallOwner(ours);
        ownership.append(ours);
        if (ownership.count() > 100)
            ownership.removeFirst();
        int oursCount = 0;
        for (int i=0;i<ownership.count();i++)
        {
            if (ownership[i]==true) oursCount ++;
        }
        if (oursCount >= ownership.count()/2.0)
            ballOurs = true;
    }
    ownerShipCalculated = true;
    return ballOurs;
}

int CKnowledge::getBallOwner(bool &ours)
{
    int owner = -1;
    findBallOwners();

    if (oppBallOwner == -1 && ballOwner == -1)
    {
        int our = -1, opp = -1;
        double minDistOur = 1000;
        double minDistOpp = 1000;
        for (int i=0;i<wm->our.activeAgentsCount();i++)
        {
            double d = (wm->ballCatchTarget(wm->our.active(i)) - wm->our.active(i)->pos).length();
            if (d < minDistOur)
            {
                minDistOur = d;
                our = wm->our.active(i)->id;
            }
        }
        for (int i=0;i<wm->opp.activeAgentsCount();i++)
        {
            double d = (wm->ballCatchTarget(wm->opp.active(i)) - wm->opp.active(i)->pos).length();
            if (d < minDistOpp)
            {
                minDistOpp = d;
                opp = wm->opp.active(i)->id;
            }
        }

        if (minDistOur - minDistOpp < 0.1) {
            owner = our;
            ours = true;
        }
        else if (minDistOpp - minDistOur < 0.1) {
            owner = opp;
            ours = false;
        }
        else
        {
            owner = lastBallOwner;
            ours = lastBallOurs;
        }
        if (owner == -1) ours = true;
    }
    else {
        if (oppBallOwner == -1)
        {
            ours = true;
            owner = ballOwner;
        }
        else {
            ours = false;
            owner = oppBallOwner;
        }
    }
    lastBallOurs = ours;
    lastBallOwner = owner;
    return owner;
}

void CKnowledge::findOppDefenders(QList<int> &defenders, int &golie)
{
    defenders.clear();;
    golie = 0;
    for(int i = 0; i < wm->opp.activeAgentsCount(); i++ )
    {
        if (wm->opp.active(i)->inSight <=0)
            continue;
        if (wm->opp.active(i)->pos.dist( wm->field->oppGoal()) <  1.6 && wm->opp.active(i)->id != knowledge->findOppGoalie())
            defenders.append(wm->opp.active(i)->id);
    }
    if ( knowledge->findOppGoalie() != -1 )
        golie++;
}

void CKnowledge::updatePlotterData()
{
    //plotterBall = wm->ball;
}

double CKnowledge::findDangerPercent(CRobot *r){
    QList <Vector2D> agentPoints;
    double ballRegion;
    double dangerPercent = 0;
    if( r->inSight > 0.0 )
    {
        Vector2D normal = ( wm->ball->pos - r->pos).norm().rotatedVector(90);
        Vector2D p1 = normal*0.100 + r->pos;
        Vector2D p2 = normal*0.050 + r->pos;
        Vector2D p3 = normal*-0.050 + r->pos;
        Vector2D p4 = normal*-0.100 + r->pos;
        agentPoints.clear();
        agentPoints.append( p1);
        agentPoints.append( p2);
        agentPoints.append( r->pos);
        agentPoints.append( p3);
        agentPoints.append( p4);
        QList<int> oppRelaxedId;
        QList<int> relaxIDs;
        oppRelaxedId.clear();
        oppRelaxedId.append(r->id);
        oppRelaxedId.append(knowledge->findOppGoalie());
        knowledge->getBestPosToShootToGoal(r->pos , dangerPercent , relaxIDs , oppRelaxedId , false);
        knowledge->getEmptyPosOnPoints(wm->ball->pos , ballRegion , agentPoints , relaxIDs , oppRelaxedId);
        dangerPercent += ballRegion/10.0;
        if( r->pos.dist( wm->field->ourGoal()) < 4.25)
            dangerPercent += 2*(1 - ((r->pos - wm->field->ourGoal()).length() / (wm->field->oppCornerL() - wm->field->ourGoal()).length()));
        Rect2D avoidMarkingRect(Vector2D( wm->field->ourGoal().x, 0.52),Vector2D(Vector2D( wm->field->ourGoal().x + 0.42, -0.52) ));
        //        draw( avoidMarkingRect, "brown");
        if( avoidMarkingRect.contains( r->pos))
            dangerPercent -= 1.5;
        if( r->pos.dist( wm->ball->pos) < 0.6) /// ZIAD SHOD VASE BAZIE OMID
            dangerPercent -= 1.0;
    }
    return dangerPercent;
}

/*
returns potential force from obstacle to target
k1 * exp (d(base, target) / k2)
*/
Vector2D CKnowledge::getPotentialOnPoint(Vector2D base, Vector2D obstacle, double k1, double k2)
{
    Q_ASSERT(k2 > 0);
    double force = k1 * exp((-1.0) * base.dist(obstacle) / k2);
    Vector2D ret = (base - obstacle).norm() * force;
    return ret;
}

/*
 id: for whom?
 searchSpace: where to find the point
 res: Search Reseloution
*/

struct pStruct
{
    Vector2D pos;
    double potential;
    double favor;
};

Vector2D CKnowledge::findGetOpenPointForAgent(int id, Rect2D searchSpace, double res)
{
    //QList<QPair <Vector2D,double> > searchPoints;
    QList<pStruct> searchPoints;
    QList<int> relaxIDs;
    relaxIDs.append(id);

    /* Generating the free to pass search points on searchSpace based on res */
    for (double x = searchSpace.left(); x < searchSpace.right(); x += res)
    {
        for (double y = searchSpace.top(); y > searchSpace.bottom(); y -= res)
        {
            //QPair<Vector2D, double> p(Vector2D(x,y), 0.1);
            pStruct p;
            p.pos = Vector2D(x,y);
            p.favor = 0.1;
            p.potential = 0.0;

            double s1 = sign((p.pos - wm->ball->pos).outerProduct(2.0 * (wm->field->oppGoalL() - wm->field->oppGoal()) + wm->field->oppGoal()-wm->ball->pos));
            double s2 = sign((p.pos - wm->ball->pos).outerProduct(2.0 * (wm->field->oppGoalR() - wm->field->oppGoal()) + wm->field->oppGoal()-wm->ball->pos));

            // The first part in the condition is for avoiding standing in front of goal for positioning
            // The second part checks the pass receiveibility of the point
            if (!(
                        (s1 * s2 < 0) &&
                        ((p.pos - wm->field->oppGoal()).length() < (wm->ball->pos - wm->field->oppGoal()).length())
                        )  && (isPointClear(p.pos, wm->ball->pos, CRobot::robot_radius_old * 3.0, true, relaxIDs, QList<int>())))
            {
                p.favor = 1.0;
            }
            searchPoints.append(p);
        }
    }

    int bestPointIndex = 0;
    double max = 0.0;


    // Finding sum of potentials on points over search space by other obstacles
    double maxEquForce = -1e20;
    for (int i = 0; i < searchPoints.count(); i++)
    {
        Vector2D equForce = Vector2D(0.0, 0.0);
        for (int j = 0; j < wm->our.activeAgentsCount(); j++)
        {
            if (wm->our.active(j)->id == id) continue;
            equForce += getPotentialOnPoint(searchPoints.at(i).pos, wm->our.active(j)->pos, 1.0, 1.0);
        }
        for (int j = 0; j < wm->opp.activeAgentsCount(); j++)
        {
            equForce += getPotentialOnPoint(searchPoints.at(i).pos, wm->opp.active(j)->pos, 1.0, 0.1);
        }
        if (equForce.length() > maxEquForce) maxEquForce = equForce.length();
        searchPoints[i].potential = equForce.length();
        //draw(QString("%1").arg(equForce.length(),0,'f',2), searchPoints.at(i).pos, "white", 8);
    }

    double favoribility;
    for (int i = 0; i < searchPoints.count(); i++)
    {
        getEmptyPosOnGoal(
                    searchPoints.at(i).pos,
                    favoribility,
                    true,
                    relaxIDs,
                    QList<int>(),
                    1.0,
                    0.2
                    );


        searchPoints[i].favor = ((3.0 * searchPoints[i].favor) + (3.0 * favoribility) + (0.15 * (1.0 - (searchPoints[i].potential / maxEquForce))));

        //draw(QString("%1").arg(searchPoints.at(i).favor,0,'f',1).arg(favoribility,0,'f',2), searchPoints.at(i).first, "white", 8);
        //draw(QString("%1").arg(searchPoints.at(i).potential / maxEquForce,0,'f',2), searchPoints.at(i).pos, "white", 8);
        //draw(QString("%1").arg(searchPoints.at(i).favor,0,'f',2), searchPoints.at(i).pos, "white", 8);

        // Add Momentum to robot's movement
        //        if (searchSpace.contains(wm->our[id]->pos))
        //        {
        //            // Normalized distance of the point to the target robot
        //            double d = searchPoints.at(i).first.dist(wm->our[id]->pos) /
        //                       (searchSpace.topLeft().dist(searchSpace.bottomRight()));
        //            d = 1.0 - d;

        //            searchPoints[i].second = (0.9 * searchPoints[i].second) + (0.1 * d);
        //        }

        if (searchPoints[i].favor > max)
        {
            max = searchPoints.at(i).favor;
            bestPointIndex = i;
        }
    }

    draw(searchPoints.at(bestPointIndex).pos, 0, "blue");
    return searchPoints.at(bestPointIndex).pos;

}

void CKnowledge::setJoystic(CJoystick *_joystic)
{
    joystick = _joystic;
}


#if 1
//similar codes can be found in cl/evalposition.c

struct range {
    float a,b;
};

inline float getangle(float x1,float y1,float x2,float y2)
{
    return atan2(y2-y1,x2-x1);
}

inline float len(float x1,float y1,float x2,float y2)
{
    return hypot(x1-x2, y1-y2);
}

inline float len2(float x1,float y1,float x2,float y2)
{
    return (x1-x2) * (x1-x2) + (y1-y2) * (y1-y2);
}


inline float normalang(float dir)
{
    const float _2PI = 2.0 * M_PI;
    if ( dir < -2.0*M_PI || 2.0*M_PI < dir )
    {
        dir = fmod( dir, _2PI );
    }
    if ( dir < -M_PI)
    {
        dir += 2.0*M_PI;
    }
    if ( dir > M_PI)
    {
        dir -= 2.0*M_PI;
    }
    return dir;
}

inline float normalangabs(float dir)
{
    const float _2PI = 2.0 * M_PI;
    if ( dir < -2.0*M_PI || 2.0*M_PI < dir )
    {
        dir = fmod( dir, _2PI );
    }
    if ( dir < -M_PI)
    {
        dir += 2.0*M_PI;
    }
    if ( dir > M_PI)
    {
        dir -= 2.0*M_PI;
    }
    if (dir < 0) return -dir;
    return dir;
}

double CKnowledge::getEmptyAngle(Vector2D p,Vector2D p1, Vector2D p2,
                                 QList<Circle2D> obs, double& percent,
                                 double &mostOpenAngle,
                                 double& biggestAngle, bool oppGoal,
                                 bool _draw)
{
    bool drawn = false;
    QColor rect_color;
    if(oppGoal)
    {
        int r , g , b;
        QColor("darkcyan").getRgb(&r , &g, &b);
        rect_color =  QColor(r, g, b, 21);
    }
    else
    {
        int r , g , b;
        QColor("magenta").getRgb(&r , &g, &b);
        rect_color = QColor(r , g, b, 21);
    }
    Vector2D goal_pos;

    if(oppGoal)
        goal_pos = wm->field->oppGoal();
    else
        goal_pos = wm->field->ourGoal();

    float gx1 = p1.x;
    float gy1 = p1.y;
    float gx2 = p2.x;
    float gy2 = p2.y;
    float x = p.x;
    float y = p.y;
    //similar codes can be found in cl/evalpos.c
    float d, a1, a2, a, l, a0, q1, q2, al;
    float la, lb, lc;
    float ox, oy;
    la = gy2-gy1;
    lb = gx1-gx2;
    lc = -gx1*la-gy1*lb;
    bool inobs = false;
    int par = 0;
    bool tmp;
    struct range tmpr;
    int count = 0;
    int i,j;
    d = 0;
    struct range r[20];
    bool flag[20];
    for (i = 0;i<20;i++)
        flag[i] = false;
    al = getangle(x, y, (gx1+gx2)*0.5, (gy1+gy2)*0.5);
    q1 = getangle(x,y,gx1,gy1)-al;
    q2 = getangle(x,y,gx2,gy2)-al;
    q1 = normalang(q1);
    q2 = normalang(q2);
    if (normalang(q1 - q2) > 0)
    {
        a = q1;
        q1 = q2;
        q2 = a;
    }
    float openangle = 0;
    for (i = 0; i < obs.count(); ++i) {
        ox = obs[i].center().x;
        oy = obs[i].center().y;
        float rad = obs[i].radius();
        l = len(x,y,ox,oy);
        if (l<rad) {inobs = true;break;}
        a1 = ox*la + oy*lb + lc;
        a2 = x*la + y*lb + lc;
        if (a1 > 0) a1 = 1;else a1 = -1;
        if (a2 > 0) a2 = 1;else a2 = -1;
        a1 = a1*a2;
        if (a1 > 0)
        {
            a = normalang(getangle(x,y,ox,oy)-al);
            a0 = asin(rad/l);
            a1 = a - a0;
            a2 = a + a0;
            if (a1 < -M_PI*0.95*0.5) a1 = -M_PI*0.95*0.5;
            if (a2 < -M_PI*0.95*0.5) a2 = -M_PI*0.95*0.5;
            if (a1 > +M_PI*0.95*0.5) a1 =  M_PI*0.95*0.5;
            if (a2 > +M_PI*0.95*0.5) a2 =  M_PI*0.95*0.5;
            if (normalang(a1-a2)>0)
            {
                a = a1;
                a1 = a2;
                a2 = a;
            }
            a1 = normalang(a1);
            a2 = normalang(a2);
            if (normalang(a1-q1)<=0) a1 = q1;
            if (normalang(a1-q2)>=0) a1 = q2;
            if (normalang(a2-q1)<=0) a2 = q1;
            if (normalang(a2-q2)>=0) a2 = q2;
            if (normalangabs(a1-a2)>=0.001)
            {
                r[count].a = a1;
                r[count].b = a2;
                count ++;
            }
        }
    }
    if (!inobs)
    {
        for (i=0;i<count;i++)
            for (j=0;j<count-1;j++)
            {
                if (normalang(r[j].a-r[j+1].a) > 0)
                {
                    tmpr = r[j];
                    r[j] = r[j+1];
                    r[j+1] = tmpr;
                }
            }
        for (i=0;i<count-1;i++)
        {
            if (normalang(r[i+1].a - r[i].b) < 0)
            {
                r[i+1].a = r[i].a;
                if (normalang(r[i+1].b - r[i].b) < 0)
                {
                    r[i+1].b = r[i].b;
                }
                flag[i] = true;
            }
        }
        bool changed = false;
        if (count > 0)
        {
            float lastBlockedDir = q1;
            int k = 0;
            mostOpenAngle = 0;
            biggestAngle = 0;
            for (i=0;i<count;i++)
            {
                if (flag[i] == false)
                {
                    if ((k == 0) && (normalang(q1 - r[i].a) >= 0))
                    {

                    }
                    /*                    else if ((i == count-1) && (normalang(r[count-1].b - q2) >= 0))
                    {

                    }*/
                    else {
                        float dist = normalangabs(r[i].a - lastBlockedDir);
                        float bisect = normalang(normalang(r[i].a - lastBlockedDir) / 2.0 + lastBlockedDir);
                        if (dist >= biggestAngle)
                        {
                            biggestAngle = dist;
                            mostOpenAngle = bisect;
                            changed = true;
                        }
                    }
                    if(_draw)
                    {
                        Line2D line1(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI));
                        Line2D line2(p, p+Vector2D::unitVector(((r[i].a+al) * 180.0 / M_PI)));
                        Line2D goal_line(goal_pos, Vector2D(goal_pos.x, goal_pos.y+2.0));
                        Vector2D p1(line1.intersection(goal_line));
                        Vector2D p2(line2.intersection(goal_line));

                        Polygon2D polygon_draw;
                        polygon_draw.addVertex(p);
                        polygon_draw.addVertex(p1);
                        polygon_draw.addVertex(p2);
                        polygon_draw.addVertex(p);
                        draw( polygon_draw, rect_color, true);
                        drawn = true;

                        //draw(Segment2D(p, p+Vector2D::unitVector((bisect+al) * 180.0 / M_PI)*5 ), "black");
                        //draw(Segment2D(p, p+Vector2D::unitVector((r[i].a+al) * 180.0 / M_PI)*5 ), "black");
                        //draw(Segment2D(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI)*5 ), "purple");
                    }
                    lastBlockedDir = r[i].b;
                    k ++;
                }
            }
            if (normalang(r[count-1].b - q2) <= 0)
            {
                float dist = normalangabs(q2 - lastBlockedDir);
                float bisect = normalang(normalang(q2 - lastBlockedDir) / 2.0 + lastBlockedDir);
                if (dist >= biggestAngle)
                {
                    biggestAngle = dist;
                    mostOpenAngle = bisect;
                    changed = true;
                }
                if(_draw)
                {
                    Line2D line1(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI));
                    Line2D line2(p, p+Vector2D::unitVector(((q2+al) * 180.0 / M_PI)));
                    Line2D goal_line(goal_pos, Vector2D(goal_pos.x, goal_pos.y+2.0));
                    Vector2D p1(line1.intersection(goal_line));
                    Vector2D p2(line2.intersection(goal_line));

                    Polygon2D polygon_draw;
                    polygon_draw.addVertex(p);
                    polygon_draw.addVertex(p1);
                    polygon_draw.addVertex(p2);
                    polygon_draw.addVertex(p);
                    draw( polygon_draw, rect_color, true);
                    drawn = true;

                    //                        draw(Segment2D(p, p+Vector2D::unitVector((bisect+al) * 180.0 / M_PI)*5 ), "blue");
                    //                        draw(Segment2D(p, p+Vector2D::unitVector((q2+al) * 180.0 / M_PI)*5 ), "red");
                    //                        draw(Segment2D(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI)*5 ), "orange");
                }
            }
        }
        for (i=0;i<count;i++)
        {
            if (flag[i] == false)
            {
                d += normalang(r[i].b - r[i].a);
                flag[i] = true;
            }
        }
        openangle = d;
        openangle = normalangabs(q2-q1) - openangle;
        d /= normalangabs(q2-q1);
        d = 1-d;
        if (!changed || (d<0.001)){
            if (count==0) biggestAngle = normalangabs(q2 - q1);
            else biggestAngle = 0;
            mostOpenAngle = normalang(normalang(q2 - q1) / 2.0 + q1);
        }
        mostOpenAngle = normalang(mostOpenAngle + al);
        biggestAngle *= 180.0 / M_PI;
        mostOpenAngle *= 180.0 / M_PI;
    }
    else {
        mostOpenAngle = normalang(0.5*(q1 + q2) + al);
        biggestAngle = 0;
        mostOpenAngle *= 180.0 / M_PI;
        //                         draw(Segment2D(p, p+Vector2D::unitVector((mostOpenAngle) )*5 ), "blue");

        d = 0.0;
    }
    percent = d;

    if(_draw && !drawn)
    {
        Vector2D p1, p2;
        if(oppGoal)
        {
            p1 = wm->field->oppGoalL();
            p2 = wm->field->oppGoalR();
        }
        else
        {
            p1 = wm->field->ourGoalL();
            p2 = wm->field->ourGoalR();
        }
        Polygon2D polygon_draw;
        polygon_draw.addVertex(p);
        polygon_draw.addVertex(p1);
        polygon_draw.addVertex(p2);
        polygon_draw.addVertex(p);
        drawn = true;

        draw( polygon_draw, rect_color, true);
    }
    return openangle * 180.0 / M_PI;
}
#else
struct emptyAngleStruct {
    bool begin;
    double angle;
};

bool operator < (const emptyAngleStruct& a, const emptyAngleStruct& b)
{
    if (AngleDeg::normalize_angle(a.angle-b.angle) < 0.0) return true;
    else return false;
}

bool operator > (const emptyAngleStruct& a, const emptyAngleStruct& b)
{
    if (AngleDeg::normalize_angle(a.angle-b.angle) > 0.0) return true;
    else return false;
}

bool operator <= (const emptyAngleStruct& a, const emptyAngleStruct& b)
{
    if (AngleDeg::normalize_angle(a.angle-b.angle) <= 0.0) return true;
    else return false;
}

bool operator >= (const emptyAngleStruct& a, const emptyAngleStruct& b)
{
    if (AngleDeg::normalize_angle(a.angle-b.angle) >= 0.0) return true;
    else return false;
}

bool operator == (const emptyAngleStruct& a, const emptyAngleStruct& b)
{
    if (AngleDeg::normalize_angle(a.angle-b.angle) == 0.0) return true;
    else return false;
}

double CKnowledge::getEmptyAngle(Vector2D p,Vector2D p1, Vector2D p2, QList<Circle2D> obs, double& percent, double &mostOpenAngle, double& biggestAngle)
{
    QList<emptyAngleStruct> r;
    emptyAngleStruct q1, q2;
    q1.begin = false;
    q1.angle = (p1 - p).th().degree();
    q2.begin = true;
    q2.angle = (p2 - p).th().degree();
    if (q2 < q1)
    {
        emptyAngleStruct tmp;
        tmp = q1;
        q1 = q2;
        q2 = tmp;
    }
    q1.begin = false;
    q2.begin = true;
    r.append(q1);
    r.append(q2);
    for (int i=0;i<obs.count();i++)
    {
        Vector2D sol1, sol2;
        if (obs[i].tangent(p, &sol1, &sol2)==2)
        {
            double ang1 = (sol1 - p).th().degree();
            double ang2 = (sol2 - p).th().degree();
            emptyAngleStruct s1, s2;
            s1.begin = true;
            s2.begin = false;
            if (AngleDeg::normalize_angle(ang1-ang2) < 0)
            {
                s1.angle = ang1;
                s2.angle = ang2;
            }
            else {
                s2.angle = ang1;
                s1.angle = ang2;
            }
            if ((s1 >= q1) && (s1 <= q2)) r.append(s1);
            if ((s2 >= q1) && (s2 <= q2)) r.append(s2);
            if ((s1 <= q1) && (s2 >= q1)) r[0].begin = true;
            if ((s2 >= q2) && (s1 <= q2)) r[1].begin = false;
            /*            if (!(s1 < q1 || (!(s1 < q2)))) r.append(s1);
            else {
                if ((s1 < q1) && (!(s2 < q1))) r[0].begin = true;
                //if (s1 < q1) r[0].begin = true;
            }
            if (!(s2 < q1 || (!(s2 < q2)))) r.append(s2);
            else {
                if ((s1 < q2) && (!(s2 < q2))) r[1].begin = false;
                //if ((s1 < q1) && (!(s2 < q1))) r[0].begin = true;
            }*/
        }
    }

    for (int i=0;i<r.count();i++)
        draw(Segment2D(p, p + Vector2D::unitVector(r[i].angle)), "red");

    emptyAngles.clear();
    AngleRange rng;
    rng.begin = r[0].angle;
    rng.end = r[0].angle;
    if (r[0].begin == true) emptyAngles.append(rng);
    rng.begin = r[1].angle;
    rng.end = r[1].angle;
    if (r[1].begin == false) emptyAngles.append(rng);
    qSort(r.begin(), r.end());
    double ang = 0.0;
    biggestAngle = 0.0;
    int par = 0;
    for (int i=0;i<r.count()-1;i++)
    {
        if (r[i].begin) par ++;
        if (!r[i].begin) par --;
        if (par < 0) par = 0;
        if (i>=0)
        {
            if ((r[i].begin == false) && (r[i+1].begin == true) && (par==0))
            {
                AngleRange rng;
                rng.begin = r[i].angle;
                rng.end = r[i+1].angle;
                emptyAngles.append(rng);
                double d = fabs(AngleDeg::normalize_angle(r[i+1].angle-r[i].angle));
                ang += d;
                if (d>biggestAngle)
                {
                    biggestAngle = d;
                    mostOpenAngle = AngleDeg::bisect(AngleDeg(r[i].angle), AngleDeg(r[i+1].angle)).degree();
                }
                //            draw(Segment2D(p, p + Vector2D::unitVector(r[i-1].angle) * 2.0), "red");
                //            draw(Segment2D(p, p + Vector2D::unitVector(r[i].angle) * 2.0), "blue");
                //draw(Segment2D(p, p + Vector2D::unitVector(r[i-1].angle) * 2.0), "red");

            }
        }
    }
    double totalang = fabs(AngleDeg::normalize_angle(q1.angle - q2.angle));
    percent = ang / totalang;
    return ang;
}

#endif

//this function generates positions for defenses and goalie simultaneously
//defenses     : number of defenses
//goalie       : if false, doesnt search for goalie
//avoidCircles : to avoid coliding with our agents, specially the defender that is blocking the ball and is not passed to this function
//goalieCurrentPos : in the case that `goalie' is false ; it searches for defender in a way that they dont cover in front of goalie; if you pass this as an invalid Vector the defender and goalie are put using default algorithm
//the positions are returned in defendersPos and goaliePos variables
//the state created this frame should'nt be so different from neighbour states
void CKnowledge::generateDefensePositions(int defenses, bool goalie, QList<Circle2D> avoidCircles, QList<Vector2D> &defendersPos, Vector2D &goaliePos, QList<DefensePositions> neighbourStates, double distanceFactor, Vector2D goalieCurrentPos)
{
    QList<Vector2D> goalie_points;
    double goal_width = (wm->field->ourGoalL() - wm->field->ourGoal()).length();
    double ang0 = atan(CRobot::robot_radius_new*2.0 / goal_width ) * 180.0 / M_PI;
    for (int i=1;i<3;i++)
    {
        double ang = -90.0 + ang0;
        while (ang < 90.0 - ang0)
        {
            Vector2D q = intersect_ellipse_dir(Vector2D::unitVector(ang), wm->field->ourGoal(), i * CRobot::robot_radius_new, goal_width, 0.0);
            //ang += 2;
            ang += 2;
            goalie_points.append(q);
            //draw(q, 1, "red");
        }
    }

    /*
    for (int i=0;i<neighbourStates.count();i++)
    {
        for (int k=0;k<neighbourStates[i].defense.count();k++)
        {
            draw(Circle2D(neighbourStates[i].defense[k], 0.06), 0, 360, "black", 1);
        }
        if (i==0)
            draw(Circle2D(neighbourStates[i].goalie, 0.06), 0, 360, "white", 1);
        else
            draw(Circle2D(neighbourStates[i].goalie, 0.06), 0, 360, "blue", 1);
    }
*/
    QList<Vector2D> points;
    QList<int> pointsn;

    for (int i=1;i<3;i++)
    {
        double ang = -80.0;
        while (ang < 80.0)
        {
            double rad = CRobot::robot_radius_old;
            double xstep = rad  * 0.5;
            double ystep = rad  * 0.5;
            /*            double xstep = rad  * 2.5;
            double ystep = rad  * 2.5;*/

            Vector2D p,q;
            Vector2D isect, n;
            q = wm->field->ourGoal() + Vector2D::unitVector(ang) * 2.0;
            QList<Vector2D> isects = wm->field->ourBigPAreaIntersect(Line2D(wm->field->ourGoal(), q), 1.0, i*ystep);
            if (isects.empty()) break;
            else {
                q = isects.front();
            }
            int s1 = sign( (wm->ball->pos - (wm->field->ourGoalL() + Vector2D(-0.5,rad*2))).outerProduct(wm->ball->pos - q));
            int s2 = sign( (wm->ball->pos - (wm->field->ourGoalR() + Vector2D(-0.5,-rad*2))).outerProduct(wm->ball->pos - q));
            bool flag = false;
            if (fabs(Vector2D::angleBetween(q-wm->field->ourGoal(), wm->ball->pos - q).degree()) < 90)
            {
                if (goalieCurrentPos.valid() && (s1*s2 < 0))
                {
                    Vector2D g = goalieCurrentPos;
                    Vector2D sol1, sol2;
                    if (Circle2D(g, knowledge->goalie->self()->robotRadius()).intersection(Ray2D(wm->ball->pos, q), &sol1, &sol2) == 0)
                        flag = true;
                }
                else flag = true;
            }
            if (flag)
            {
                pointsn.append(points.count());
                points.append(q);
                //draw(q, 1, "red");
            }
            n = wm->field->ourPAreaPerpendicularVector(ang, isect);
            q = q + xstep*n.rotatedVector(90);
            ang = (q-wm->field->ourGoal()).th().degree();
        }
    }

    QList<int> bestComb;
    double bestW = 0.0;
    QList<int> def;
    for (int i=0;i<defenses;i++)
        def.append(i);
    QList<QList<int> > defComb;
    defComb = generateCombinations(def);
    QList<QList<int> > subs = generateSubsets(pointsn, defenses);

    for (int g=0;g<goalie_points.count();g++)
    {

        double goalie_dist = 0;
        int c = 0;
        bool good = false;
        for (int k=0;k<neighbourStates.count();k++)
        {
            if (neighbourStates[k].goalie.valid())
            {
                c ++;
                goalie_dist = ((goalie_points[g] - neighbourStates[k].goalie).length());
                if (goalie_dist < 0.3) good = true;
            }
        }
        if (c==0) good = true;
        if (!good) continue;
        //goalie_dist /= (double) c;

        //debug(QString("it is %1").arg(goalie_dist), D_ALI);
        //            continue;
        //        }
        //check if goalie is too far
        for (int i=0;i<subs.count();i++)
        {
            QList<Circle2D> obs;
            obs.append(Circle2D(goalie_points[g], CRobot::robot_radius_old));
            for (int j=0;j<defenses;j++)
            {
                obs.append(Circle2D(points[subs[i][j]], CRobot::robot_radius_old /*defs[j]->self()->robotRadius()*/));
            }
            bool flag = false;
            /*            for (int j=0;j<defenses;j++)
            {
                for (int k=j+1;k<defenses;k++)
                {
                    if ((points[subs[i][j]] - points[subs[i][k]]).length() < CRobot::robot_radius_old)
                    {
                        flag = true;
                        break;
                    }
                }
                if (flag) break;
            }*/
            if (!flag)
            {
                double w = 0, a1 = 0.0, a2 = 0.0;
                int c=0;
                double distToNeighbours = 0;
                for (int k=0;k<neighbourStates.count();k++)
                {
                    double bestDist0 = 0;
                    int bestDistComb = -1;
                    if (neighbourStates[k].defense.count() == defenses)
                    {
                        c ++;
                        for (int h=0;h<defComb.count();h++)
                        {
                            double dist = 0;
                            for (int j=0;j<defenses;j++)
                            {
                                if (neighbourStates[k].defense[defComb[h][j]].valid())
                                    dist += (neighbourStates[k].defense[defComb[h][j]] - points[subs[i][j]]).length();
                            }
                            if (dist < bestDist0 || bestDistComb == -1)
                            {
                                bestDist0 = dist;
                                bestDistComb = h;
                            }
                        }
                        if (neighbourStates[k].goalie.valid())
                            bestDist0 += (goalie_points[g] - neighbourStates[k].goalie).length();
                        distToNeighbours += bestDist0;
                    }
                }
                if (c>0) distToNeighbours /= (double) c;
                knowledge->getEmptyAngle(wm->ball->pos, wm->field->ourGoalL(), wm->field->ourGoalR(), obs, w, a1, a2);
                w += distToNeighbours * distanceFactor;
                if ((w<bestW) || (bestComb.empty()))
                {
                    bestW = w;
                    bestComb.clear();
                    bestComb.append(g);
                    bestComb.append(subs[i]);
                }
            }
        }
    }
    draw(QString().setNum(bestW,'f', 2), wm->field->ourGoal() - Vector2D(0.2, 0), "blue", 12);
    if (!bestComb.empty())
    {
        if (goalie) goaliePos = goalie_points[bestComb[0]];
        defendersPos.clear();
        for (int i=0;i<defenses;i++)
            defendersPos.append(points[bestComb[i+1]]);
    }
    if (bestComb.empty())
    {
        for (int i=0;i<defenses;i++)
            defendersPos.append(Vector2D());
        goaliePos.invalidate();
        debug(QString("shit!!! %1").arg(subs.count()), D_ALI);
        for (int j=0;j<neighbourStates.count();j++)
            debug(QString("%1 , %2").arg(neighbourStates[j].goalie.x).arg(neighbourStates[j].goalie.y), D_ALI);
    }
    for (int i=0;i<defendersPos.count();i++)
        draw(Circle2D(defendersPos[i], 0.05), 0, 360, "blue", 1);
    draw(Circle2D(goaliePos, 0.05), 0, 360, "pink", 1);
}


void CKnowledge::generateDefensePositions(int defenses, bool goalie, QList<Circle2D> avoidCircles, QList<Vector2D> &defendersPos, Vector2D &goaliePos, Vector2D goalieCurrentPos)
{
    double DefensePenaltyDistFactor = 1.0;
    Vector2D ballpos  = wm->ball->pos;
    if (ballpos.x < 0.3)
    {

    }
    if (ballpos.x < wm->field->fieldRect().left()+0.02)
        ballpos.x = wm->field->fieldRect().left()+0.02;

    Vector2D ballposd = ballpos;
    Vector2D ballposg = ballpos;
    if ((ballpos - wm->field->ourGoal()).length() < 1.0)
    {
        ballposd = (ballpos - wm->field->ourGoal()).norm() * 1.0 + wm->field->ourGoal();
    }
    if ((ballpos - wm->field->ourGoal()).length() < 0.5)
    {
        ballposg = (ballpos - wm->field->ourGoal()).norm() * 0.5 + wm->field->ourGoal();
    }
    Vector2D s1 = ballposg - wm->field->ourGoalL();
    Vector2D s2 = ballposg - wm->field->ourGoalR();
    double w = (wm->field->ourGoalL() - wm->field->ourGoalR()).length();
    QList<Circle2D> obs;
    double percent, mostOpenAngle, biggestAngle;
    Vector2D shr(CRobot::robot_radius_old + 0.08, 0.0);
    goalie_shr = shr;
    double x = (ballpos.x - (wm->field->ourCornerL().x));
    if (x < 0) x = 0;

    double width = (wm->field->oppCornerL().x - wm->field->ourCornerL().x);
    if (x > width) x = width;
    x /= width;

    if (closeGoalie==-1)
    {
        if (wm->ball->pos.x < -0.3)
        {
            closeGoalie = 1;
        }
        else {
            closeGoalie = 2;
        }
    }
    else {
        if (wm->ball->pos.x < -0.3)
        {
            closeGoalie = 1;
        }
        else if (wm->ball->pos.x > -0.0)
        {
            closeGoalie = 2;
        }
    }
    //    if (closeGoalie == 2) goalie = false;
    if (goalie)
    {
        if (defenses == 2)
        {
            Vector2D g1 = (ballposd - wm->field->ourGoalL()-shr);
            Vector2D g2 = (ballposd - wm->field->ourGoalR()-shr);
            AngleDeg bisect = AngleDeg::bisect(g1.th(), g2.th());
            double goalWidth = (wm->field->ourGoalL()-wm->field->ourGoalR()).length();
            double alpha = atan(CRobot::robot_radius_old/goalWidth)*_RAD2DEG;
            double angle = (ballpos - wm->field->ourGoal()).th().degree();
            Vector2D point;
            double a1 = AngleDeg::normalize_angle(angle-90+alpha);
            double a2 = AngleDeg::normalize_angle(angle+90-alpha);

            Vector2D sol1, sol2;
            Vector2D p = Vector2D::unitVector(AngleDeg::bisect(s1.th(), s2.th()));
            goalie_ellipse_a = x*0.01 + 0.2;
            goalie_ellipse_b = w/2.0;
            intersect_ellipse_line(ballposg, ballposg + p, wm->field->ourGoal() + shr, goalie_ellipse_a ,  goalie_ellipse_b, &sol1, &sol2);

            if ((sol1 - ballposg).length() < (sol2 - ballposg).length())
                goaliePos = sol1;
            else
                goaliePos = sol2;
            if (a1>0.0 && a1<90.0)
                goaliePos = wm->field->ourGoalL() + shr;
            else if (a2<0.0 && a2>-90.0)
                goaliePos= wm->field->ourGoalR() + shr;

            obs.append(Circle2D(goaliePos, CRobot::robot_radius_old));
            getEmptyAngle(ballposd, wm->field->ourGoalL(), wm->field->ourGoalR(), obs, percent, mostOpenAngle, biggestAngle);

            for (int h=0;h<min(emptyAngles.count(), 2);h++)
            {
                p = Vector2D::unitVector(AngleDeg::bisect(emptyAngles[h].begin, emptyAngles[h].end));
                QList<Vector2D> sols = wm->field->ourPAreaIntersect(Segment2D(ballposd - p * 10, ballposd + p * 10));
                for (int i=0;i<sols.count();i++)
                {
                    sols[i] = ((sols[i]-wm->field->ourGoal()) * DefensePenaltyDistFactor) + wm->field->ourGoal();
                }
                double w = 0;
                int k = -1;
                for (int i=0;i<sols.count();i++)
                {
                    double d = (sols[i]-ballposd).length();
                    if (d<w || k==-1)
                    {
                        k = i;
                        w = d;
                    }
                }
                if (k != -1)
                {
                    Vector2D point = sols[k];
                    point += (point - wm->field->ourGoal()).norm() * (CRobot::robot_radius_old + DefenseGap);
                    double r = CRobot::robot_radius_old / sin(fabs(AngleDeg::normalize_angle(emptyAngles[h].begin.degree() - emptyAngles[h].end.degree())) * M_PI / 180.0 / 2.0);
                    double r0 = (point - ballposd).length();
                    if (r0 < r) r = r0;
                    if (r0 - r > 0.26) r = r0 - 0.26;
                    point = (point-ballposd).norm() * r + ballposd;
                    defendersPos.append(point);
                    draw (point, 1, "blue");
                }
            }
            if (defendersPos.count() == 1)
            {
                defendersPos.append((defendersPos[0]-wm->field->ourGoal()).norm().rotatedVector(90) * 0.01 + defendersPos[0]);
            }
            if (defendersPos.count() == 2)
            {
                double rad = CRobot::robot_radius_old;
                if ((defendersPos[0] - defendersPos[1]).length() < 2.0*rad)
                {
                    Vector2D q = (defendersPos[0] + defendersPos[1]) / 2.0;
                    defendersPos[0] = (defendersPos[0] - q).norm()*rad + q;
                    defendersPos[1] = (defendersPos[1] - q).norm()*rad + q;
                }
                if ((defendersPos[0].x < wm->field->fieldRect().left() + CRobot::robot_radius_old))
                {
                    defendersPos[0].x = wm->field->fieldRect().left() + CRobot::robot_radius_old;
                    Vector2D q = (defendersPos[0] + defendersPos[1]) / 2.0;
                    defendersPos[1] = (q - defendersPos[0]).norm()*rad*2.0 + defendersPos[0];
                }
                if (defendersPos[1].x < wm->field->fieldRect().left() + CRobot::robot_radius_old)
                {
                    defendersPos[1].x = wm->field->fieldRect().left() + CRobot::robot_radius_old;
                    Vector2D q = (defendersPos[0] + defendersPos[1]) / 2.0;
                    defendersPos[0] = (q - defendersPos[1]).norm()*rad*2.0 + defendersPos[1];
                }
                Desired_defPos1 = defendersPos[0];
                Desired_defPos2 = defendersPos[1];
            }
            Desired_goaliePos = goaliePos;
            //        debug(QString("Def = 2 desired goalie pos %1 %2").arg(Desired_goaliePos.x).arg(Desired_goaliePos.y), D_ALI);
        }
        else if (defenses == 1)
        {
            Vector2D sol1, sol2;
            AngleDeg sm = (AngleDeg::bisect(s1.th(), s2.th()));
            AngleDeg e1 = AngleDeg::bisect(sm, s1.th());
            AngleDeg e2 = AngleDeg::bisect(sm, s2.th());
            if (defense_state == -1)
            {
                if (ballpos.y > 0) defense_state = 1;
                if (ballpos.y < 0) defense_state = 2;
            }
            else {
                if (ballpos.y >  0.1) defense_state = 1;
                if (ballpos.y < -0.1) defense_state = 2;
            }
            Vector2D p = Vector2D::unitVector(e1);
            if (defense_state == 2) p = Vector2D::unitVector(e2);
            goalie_ellipse_a = x*0.01 + 0.2;
            goalie_ellipse_b = w/2.0;
            intersect_ellipse_line(ballposg, ballposg + p, wm->field->ourGoal() + shr, goalie_ellipse_a ,  goalie_ellipse_b, &sol1, &sol2);
            //intersect_ellipse_line(ballpos, ballpos + p, wm->field->ourGoal(), 0.4,  w/2.0, &sol1, &sol2);
            double percent, mostOpenAngle, biggestAngle;
            if ((sol1 - ballposg).length() < (sol2 - ballposg).length())
                goaliePos = sol1;
            else
                goaliePos = sol2;
            double a1, a2;
            if (defense_state == 2)
            {
                p = Vector2D::unitVector(e1);
                a1 = s1.th().degree();
                a2 = sm.degree();
            }
            else {
                p = Vector2D::unitVector(e2);
                a1 = s2.th().degree();
                a2 = sm.degree();
            }
            QList<Vector2D> sols = wm->field->ourPAreaIntersect(Segment2D(ballpos - p * 10, ballpos + p * 10));
            double w = 0;
            int k = -1;
            for (int i=0;i<sols.count();i++)
            {
                double d = (sols[i]-ballposd).length();
                if (d<w || k==-1)
                {
                    k = i;
                    w = d;
                }
            }
            if (k != -1)
            {
                Vector2D point = sols[k];
                point += (point - wm->field->ourGoal()).norm() * CRobot::robot_radius_old;
                double r = CRobot::robot_radius_old / sin(0.8*fabs(AngleDeg::normalize_angle(a1 - a2)) * M_PI / 180.0 / 2.0);
                double r0 = (point - ballposd).length();
                if (r0 < r) r = r0;
                if (r0 - r > 0.5) r = r0 - 0.5;
                point = (point-ballposd).norm() * r + ballposd;
                defendersPos.append(point);
                draw (point, 1, "blue");
            }
            if (defendersPos.count() == 1)
            {
                double rad = CRobot::robot_radius_old;
                Desired_defPos1 = defendersPos[0];
                draw(Desired_goaliePos, 1, "blue");
            }
            Desired_goaliePos = goaliePos;
        }
    }
    else {
        //No Goalie
        Vector2D l1=wm->field->ourGoalL()-ballposd;
        Vector2D l2=wm->field->ourGoalR()-ballposd;

        AngleDeg bisect = AngleDeg::bisect(l1.th(),l2.th());

        QList<Vector2D> intersectPointsList =  wm->field->ourPAreaIntersect(Line2D(ballposd,bisect));

        for (int i=0;i<intersectPointsList.count();i++)
        {
            intersectPointsList[i] = ((intersectPointsList[i]-wm->field->ourGoal()) * DefensePenaltyDistFactor) + wm->field->ourGoal();
        }

        double minDist=1e10;
        double minI=-1;

        for(int i=0;i<intersectPointsList.count();i++)
        {
            if((ballposd-intersectPointsList[i]).length()<minDist || minI==-1)
            {
                minDist=(ballposd-intersectPointsList[i]).length();
                minI=i;
            }
        }
        Vector2D bestBlockingPoint;
        if(minI!=-1)
        {
            bestBlockingPoint = intersectPointsList[minI];

        }else
        {
            Vector2D sol1,sol2;
            Circle2D(wm->field->ourGoal(),0.5).intersection(Line2D(ballposd,bisect),&sol1,&sol2);
            if((sol1-ballposd).length()<(sol2-ballposd).length())
            {
                bestBlockingPoint = sol1;
            }else
            {
                bestBlockingPoint = sol2;
            }
        }

        if (defenses == 2)
        {

            AngleDeg bisectPrependicular = AngleDeg(bisect.degree()+90);

            Vector2D A(1,0),B(1,0);
            A.setDir(bisectPrependicular);
            B.setDir(bisect);
            A.normalize();
            B.normalize();
            defendersPos.clear();
            defendersPos.append(bestBlockingPoint + A*CRobot::robot_radius_old - B*CRobot::robot_radius_old);
            defendersPos.append(bestBlockingPoint - A*CRobot::robot_radius_old - B*CRobot::robot_radius_old);


            if (defendersPos.count() == 2)
            {
                double rad = CRobot::robot_radius_old;
                if ((defendersPos[0] - defendersPos[1]).length() < 2.0*rad)
                {
                    Vector2D q = (defendersPos[0] + defendersPos[1]) / 2.0;
                    defendersPos[0] = (defendersPos[0] - q).norm()*rad + q;
                    defendersPos[1] = (defendersPos[1] - q).norm()*rad + q;
                }
                if ((defendersPos[0].x < wm->field->fieldRect().left() + CRobot::robot_radius_old))
                {
                    defendersPos[0].x = wm->field->fieldRect().left() + CRobot::robot_radius_old;
                    Vector2D q = (defendersPos[0] + defendersPos[1]) / 2.0;
                    defendersPos[1] = (q - defendersPos[0]).norm()*rad*2.0 + defendersPos[0];
                }
                if (defendersPos[1].x < wm->field->fieldRect().left() + CRobot::robot_radius_old)
                {
                    defendersPos[1].x = wm->field->fieldRect().left() + CRobot::robot_radius_old;
                    Vector2D q = (defendersPos[0] + defendersPos[1]) / 2.0;
                    defendersPos[0] = (q - defendersPos[1]).norm()*rad*2.0 + defendersPos[1];
                }
                Desired_defPos1 = defendersPos[0];
                Desired_defPos2 = defendersPos[1];
            }
            //        debug(QString("Def = 2 desired goalie pos %1 %2").arg(Desired_goaliePos.x).arg(Desired_goaliePos.y), D_ALI);
        }
        else if (defenses == 1)
        {

            Vector2D B(1,0);
            B.setDir(bisect);
            B.normalize();

            defendersPos.clear();
            defendersPos.append(bestBlockingPoint - B*CRobot::robot_radius_old);

            if (defendersPos.count() == 1)
            {
                double rad = CRobot::robot_radius_old;
                Desired_defPos1 = defendersPos[0];
                draw(Desired_goaliePos, 1, "blue");
            }
            Desired_goaliePos = goaliePos;
        }
    }
    obs.clear();
    for (int i=0;i<defendersPos.count();i++)
    {
        obs.append(Circle2D(defendersPos[i], CRobot::robot_radius_old));
        draw(obs[i], "pink", true);
    }
    draw (goaliePos, 1, "red");
    obs.append(Circle2D(goaliePos, CRobot::robot_radius_old));
    getEmptyAngle(ballpos, wm->field->ourGoalR(), wm->field->ourGoalL(), obs, percent, mostOpenAngle, biggestAngle);
    draw(QString().setNum(percent,'f', 3), wm->field->ourGoal() - Vector2D(0.2, 0), "red", 12);

    if( defendersPos.count() == 2 ){
        Vector2D pos1 = Desired_defPos1  , pos2 = Desired_defPos2;
        defendersPos.clear();
        double dist = Line2D(pos1 , pos2).dist(wm->ball->pos) / ((wm->field->ourGoal()-wm->field->oppGoal()).length());
        double close = 0.0;
        double maxDistToClose = 0.3;
        if (dist < maxDistToClose)
        {
            close = 1.0;
        }
        else {
            close = 1.0 - (dist - maxDistToClose) / (1.0 - maxDistToClose);
        }
        if (close < 0.0) close = 0.0;
        if (close > 1.0) close = 1.0;
        close *= 0.4;
        close += 0.04;
        Vector2D e = ((pos1 + pos2) / 2.0);
        //debug(QString("maxdist=%1").arg(close), D_ERROR);
        double d1 = (pos1-e).length()-close;
        double d2 = (pos2-e).length()-close;
        if (d1 < CRobot::robot_radius_old) d1 = CRobot::robot_radius_old - 0.003;
        if (d2 < CRobot::robot_radius_old) d2 = CRobot::robot_radius_old - 0.003;
        pos1 = (pos1 - e).norm() * (d1+0.0) + e;
        pos2 = (pos2 - e).norm() * (d2+0.0) + e;
        draw(Circle2D(pos1,0.05), "red", true);
        draw(Circle2D(pos2,0.05), "red", true);

        //        if( maxDist <= 6 && maxDist >= 0 ){
        //            for( int i=0 ; i<500 && pos1.dist(pos2) > maxDist+2*CRobot::robot_radius_old ; i++ ){
        //                pos1 = ((pos2 - pos1).norm()*0.003) + pos1;
        //                pos2 = ((pos1 - pos2).norm()*0.003) + pos2;
        //            }
        //        }
        defendersPos.append(pos1);
        defendersPos.append(pos2);
        Desired_defPos1 = pos1;
        Desired_defPos2 = pos2;
    }
    else {
        //        debug(QString("defs=%1 ; defs=%2").arg(defendersPos.count()).arg(defenses), D_ERROR);
    }
}

Vector2D CKnowledge::ballCatchTarget( CRobot *robot ){
    CAgent* agent = new CAgent(_MAX_NUM_PLAYERS - 1);
    CSkillGotoPoint *gp = new CSkillGotoPoint(NULL);

    agent->self()->pos = robot->pos;
    agent->self()->vel = robot->vel;
    agent->self()->acc = robot->acc;

    Vector2D togoal = (wm->field->ourGoal() - agent->self()->getKickerPos()).norm();
    double t = 10;
    double dt = 5;

    gp->setAgent(agent);
    gp->setFastW(false);
    gp->setTurningDist(0.0);
    Vector2D motiondir;
    if( wm->ball->vel.length() < 0.05 ){
        motiondir = togoal;
    }
    else{
        motiondir = wm->ball->vel.norm();
    }
    gp->setMotionDir(motiondir);


    int iters = 200;
    for(int i=0 ; i<iters ; i++)
    {
        Vector2D q = wm->ball->predict(t);
        gp->setFinalPos(q);
        gp->execute();
        if(gp->timeNeeded()>t){
            t += dt;
        }
        else{
            t -= dt;
        }
        dt /= 2.0;
    }
    delete agent;
    delete gp;
    return wm->ball->predict(t);
}


QString CKnowledge::getMarkableNumber(){
    if( variables.count("markable") )
        return variables["markable"];
    return QString("0");
}


void CKnowledge::checkShootDanger()
{
    double percent, mostopenangle, biggestangle;
    QList<Circle2D> obs;
    for (int i=0;i<wm->our.activeAgentsCount();i++)
    {
        if ((!roleAssignments["defense"].contains(agents[wm->our.active(i)->id])) &&
                (!roleAssignments["goalie"].contains(agents[wm->our.active(i)->id])))
        {
            obs.append(Circle2D(wm->our.active(i)->pos, CRobot::robot_radius_old));
        }
    }
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
        obs.append(Circle2D(wm->opp.active(i)->pos, CRobot::robot_radius_old));
    }
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
        if (wm->opp.active(i)->isBallOwner(0.02))
        {
            getEmptyAngle(wm->ball->pos, wm->field->ourGoalL(), wm->field->ourGoalR(), obs, percent, mostopenangle, biggestangle);
            if (percent > 0.6)
            {

            }
        }
    }
}


/*!
 * @abstract to execute optimized gotopoints for multiagent positioning, etc.
 * @param ids the indices of agents that are going to be assigned
 * @param points the points that agents are going to take position there
 * @param bestPermutation the best permutation for indices to match points
 * @return total position error of matching
 */
double CKnowledge::matchPositions(QList<int> ids, QList<Vector2D> points, QList<int> &bestPermutation)
{
    for (int i=0;i<points.length();i++)
        draw(points[i],1,"red");
    QList<QList<int> > combs = generateCombinations(ids);
    double bestD = 1e10;
    int bestJ = -1;

    for (int j=0;j<combs.length();j++)
    {
        double d = 0;
        for (int i=0;i<points.count();i++)
        {
            d += points[i].dist(getAgent(combs[j][i])->pos());
        }
        if (d <= bestD)
        {
            bestD = d;
            bestJ = j;
        }
    }
    if (bestJ > -1)
    {
        bestPermutation.clear();
        bestPermutation.append(combs[bestJ]);
    }
    else {
        //no match
    }
    if (bestPermutation.size() != ids.size())
    {
        matchdebug = false;
    }
    return bestD;
}

FormationCounts::FormationCounts()
{
    attackers = defenders = 2;
    goalie = 1;
}


#define LOG(key, value) debug(QString("%1:: %2").arg(key).arg(value), D_ALI);
FastestToBall CKnowledge::findFastestToBall(QList<int> ourList, QList<int> oppList)
{
    /////Extracted from DefensePlan
    /////By Pooria
    /////
    FastestToBall f;
    double time = 0.f;
    f.catch_time = 1000;
    while (true) {
        Vector2D ballPos = wm->ball->predict(min(time, f.catch_time));
        //            draw(ballPos, 1, "red");
        if (wm->ball->vel.length() < 0.05)
            ballPos = wm->ball->pos;
        double rad = time * 0.6 + 0.2;
        if (f.ourFastest == -1)
            for (int i = 0; i < ourList.count(); i++) {
                Vector2D playerPos = wm->our[ourList[i]]->pos;
                if (playerPos.dist(ballPos) < rad) {
                    f.ourFastest = ourList[i];
                    f.ourFastestTime = time;
                    if (time < f.catch_time)
                        f.catch_time = time;
                    break;
                }
            }

        if (f.oppFastest == -1)
            for (int i = 0; i < oppList.count(); i++) {
                Vector2D playerPos = wm->opp[oppList[i]]->pos;
                if (playerPos.dist(ballPos) < rad) {
                    f.oppFastest = oppList[i];
                    f.oppFastestTime = time;
                    if (time < f.catch_time)
                        f.catch_time = time;
                    break;
                }
            }

        if ((f.ourFastest > -1 || ourList.count() == 0) && (f.oppFastest > -1 || oppList.count() == 0)) {
            //				LOG("FFFF: ", f.ourFastest);
            break;
        }
        time += 0.1;
        if (time > 20) {
            if (wm->ball->vel.length() > 0.2) {
                Line2D line(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm());
                if (f.ourFastest == -1 and ourList.count() > 0) {
                    float min = 99999;
                    for (int i = 0; i < ourList.count(); i++) {
                        Vector2D playerPos = wm->our[ourList[i]]->pos;
                        float dist = line.dist(playerPos);
                        if (dist < min) {
                            f.ourFastest = ourList[i];
                            min = dist;
                        }
                    }
                    f.ourFastestTime = time;
                    if (time < f.catch_time)
                        f.catch_time = time;
                }
                if (f.oppFastest == -1 and oppList.count() > 0) {
                    float min = 99999;
                    for (int i = 0; i < oppList.count(); i++) {
                        Vector2D playerPos = wm->opp[oppList[i]]->pos;
                        float dist = line.dist(playerPos);
                        if (dist < min) {
                            f.oppFastest = oppList[i];
                            min = dist;
                        }
                    }
                    f.oppFastestTime = time;
                    if (time < f.catch_time)
                        f.catch_time = time;
                }
            }
            break;
        }
    }
    f.catch_time = min(time, f.catch_time);
    return f;
}

NewFastestToBall CKnowledge::newFastestToBall(double timeStep, QList<int> ourList, QList<int> oppList){
    ////
    ////Code By Sepehr
    ////

    // reset everything
    NewFastestToBall result;
    if(!wm->field->fieldRect().contains(wm->ball->pos))
        return result;

    double t = 0;
    Vector2D ballPredict;

    bool ourCalced[_MAX_NUM_PLAYERS];
    bool oppCalced[_MAX_NUM_PLAYERS];
    for( int i=0 ; i<_MAX_NUM_PLAYERS ; i++ ){
        ourCalced[i] = false;
        oppCalced[i] = false;
    }

    // use the correct Robot acceleration and maximum Velocity below :
    double robotMaxVel = conf()->BangBang_VelMax();
    double robotMAxAcc = conf()->BangBang_AccMaxForward();

    while ( t < 20 && (result.ourF.size() < ourList.size() || result.oppF.size() < oppList.size()) )
    {
        ballPredict = wm->ball->predict(t);

        double tToVMax;
        Vector2D center;
        double radius;
        for ( int i = 0; i < ourList.count(); i++ )
        {
            if ( ourCalced[i] )
                continue;
            center = wm->our[ourList[i]]->pos + wm->our[ourList[i]]->vel * t;
            tToVMax = ( robotMaxVel - wm->our[ourList[i]]->vel.length()) / robotMAxAcc;
            radius = 0;
            if ( tToVMax > t)
                radius = 0.5*robotMAxAcc*t*t + wm->our[ourList[i]]->vel.length() * t;
            else if ( tToVMax > 0 && wm->our[ourList[i]]->vel.length() < robotMaxVel)
            {
                radius = 0.5*robotMAxAcc*tToVMax*tToVMax + wm->our[ourList[i]]->vel.length()*tToVMax;
                radius += ( t - tToVMax ) * robotMaxVel;
            }
            else
                radius = t * wm->our[ourList[i]]->vel.length();
            radius += CRobot::robot_radius_old;
            Circle2D cir = Circle2D( center, radius);
            Vector2D s0,s2;
            if( cir.contains(ballPredict) || cir.intersection(Segment2D( wm->ball->pos, ballPredict), &s0, &s2) )
            {
                result.ourF.append(pair<double,int>(t , ourList[i]));
                ourCalced[i] = true;
                if( result.catch_time > t ){
                    result.catch_time = t;
                    result.isFastestOurs = true;
                }
            }
            //            draw( cir, 0 , 360, "red");
        }
        for ( int i = 0; i < oppList.count(); i++ )
        {
            if ( oppCalced[i] )
                continue;
            center = wm->opp[oppList[i]]->pos + wm->opp[oppList[i]]->vel * t;
            tToVMax = ( robotMaxVel - wm->opp[oppList[i]]->vel.length()) / robotMAxAcc;
            radius = 0;
            if ( tToVMax > t)
                radius = 0.5*robotMAxAcc*t*t + wm->opp[oppList[i]]->vel.length() * t;
            else if ( tToVMax > 0 && wm->opp[oppList[i]]->vel.length() < robotMaxVel)
            {
                radius = 0.5*robotMAxAcc*tToVMax*tToVMax + wm->opp[oppList[i]]->vel.length()*tToVMax;
                radius += ( t - tToVMax ) * robotMaxVel;
            }
            else
                radius = t * wm->opp[oppList[i]]->vel.length();
            radius += CRobot::robot_radius_old;
            Circle2D cir = Circle2D( center, radius) ;
            Vector2D s0,s2;
            if( cir.contains(ballPredict) || cir.intersection(Segment2D( wm->ball->pos, ballPredict), &s0, &s2))
            {
                result.oppF.append(pair<double,int>(t , oppList[i]));
                oppCalced[i] = true;
                if( result.catch_time > t ){
                    result.catch_time = t;
                    result.isFastestOurs = false;
                }
            }
            //            draw( cir, 0 , 360, "blue");
        }
        t += timeStep;
    }
    qSort(result.ourF.begin() , result.ourF.end());
    qSort(result.oppF.begin() , result.oppF.end());
    if( result.catch_time > 10 )
        result.catch_time = 0;
    return result;
}

CKnowledge::PlaymakerSelector::PlaymakerSelector()
{
    playmaker = -1;
    passRecvTarg = -1;
    passRecvTargFrame = 0;
    lastPlaymakerSelectFrame = 0;
    distance = 10;
}

int CKnowledge::PlaymakerSelector::getPlayMakerId()
{
    if (playmaker == -1)
    {
        return -1;
    }
    else {
        if (playmaker < agents.count())
            return agents[playmaker]->id();
        else
            return -1;
    }
}

int CKnowledge::PlaymakerSelector::select()
{

    if ((knowledge->frameCount - knowledge->khafanMarkFrame < 100)
            &&
            (knowledge->khafanMarker != -1)
            )
    {
        lastPlaymakerSelectFrame = knowledge->frameCount;

        for (int i=0;i<agents.count();i++)
        {
            if (agents[i]->id() == knowledge->khafanMarker)
            {
                playmaker = i;
                debug (QString("khafan marker is %1 which is %2").arg(knowledge->khafanMarker).arg(playmaker), D_SEPEHR);
                return playmaker;
            }
        }

    }


    bool same = true;
    if (agents.count() == lastAgents.count())
    {
        for (int i=0;i<agents.count();i++)
        {
            if (!agents.contains(lastAgents[i]))
            {
                same = false;
            }
        }
    }
    else same = false;
    lastAgents.clear();
    lastAgents.append(agents);
    if (((knowledge->frameCount - lastPlaymakerSelectFrame) <= 0) && (playmaker != -1)) return playmaker;
    if (passRecvTarg != -1)
    {
        int passTarget = -1;
        for (int i=0;i<agents.count();i++)
        {
            if (agents[i]->id() == passRecvTarg) passTarget = i;
        }
        if (playmaker != -1 && passTarget != -1)
        {
            if (same == true)
            {
                double d_playmaker = (wm->ballCatchTarget(agents[playmaker]->self()) - agents[playmaker]->pos()).length();
                double d_passrecver = (wm->ballCatchTarget(agents[passTarget]->self()) - agents[passTarget]->pos()).length();
                double ballVell_Playmaker = agents[playmaker]->self()->ballComingSpeed();
                double ballVell_PassRec = agents[passTarget]->self()->ballComingSpeed();
                if (d_passrecver < d_playmaker || ( ballVell_Playmaker < -0.9 && ballVell_PassRec > 0.9))
                {
                    playmaker = passTarget;
                    distance = d_passrecver;
                    lastPlaymakerSelectFrame = knowledge->frameCount;
                    passRecvTarg = -1;
                    debug (QString("pass recv targ = %1").arg(playmaker), D_SEPEHR);
                    return playmaker;
                }
            }
        }
    }
    if (((knowledge->frameCount - lastPlaymakerSelectFrame) <= 3) && (playmaker != -1)) return playmaker;
    int newPlaymaker = -1;
    double bestD = 0;
    for (int i=0;i<agents.count();i++)
    {
        double d = (wm->ballCatchTarget(agents[i]->self()) - agents[i]->pos()).length();
        if ((d < bestD) || (newPlaymaker==-1)) {
            bestD = d;
            newPlaymaker = i;
        }
    }
    if ((playmaker == -1) || (fabs(bestD - distance) > 0.3) )
    {
        playmaker = newPlaymaker;
        distance = bestD;
    }

    QString ss;
    for (int i=0;i<agents.count();i++)
    {
        ss += QString("%1 ").arg(agents[i]->id());
    }
    debug (QString("agents are = %1").arg(ss), D_SEPEHR);
    debug (QString("new playmaker = %1 distance=%2").arg(playmaker).arg(distance), D_SEPEHR);
    lastPlaymakerSelectFrame = knowledge->frameCount;
    return playmaker;
}

void CKnowledge::PlaymakerSelector::setAgents(QList<CAgent*> _agents)
{
    bool same = true;
    if (_agents.count() == agents.count())
    {
        for (int k=0;k<agents.count();k++)
        {
            if (!_agents.contains(agents[k])) {
                same = false;
                break;
            }
        }
    }
    else same = false;
    if (!same)
    {
        agents.clear();
        agents.append(_agents);
        playmaker = -1;
        passRecvTarg = -1;
    }
}

void CKnowledge::PlaymakerSelector::setPassRecvTarget(int id)
{
    if ((knowledge->frameCount - passRecvTargFrame) < 10) return; //dont-accept this value
    passRecvTargFrame = knowledge->frameCount;
    passRecvTarg = id;
}


void CKnowledge::resetEssentialVars(){
    goalie = NULL;
}

velAndAccByKK CKnowledge::getVelocityByPos(){
    /*this function calculate ball velocity and acceleration using last three ball position
   *ballPosHistory is a QList that we prepend ball position in it
   */
    velAndAccByKK res;
    if(ballPosHistory.count() < 3 ){
        res.vel = 0.0;
        res.acc = 0.0;
        return res;
    }

    double vel1 = ballPosHistory.at(0).dist(ballPosHistory.at(1))/0.016;
    double vel2 = ballPosHistory.at(1).dist(ballPosHistory.at(2))/0.016;
    double acc1 = fabs(vel1 - vel2)/0.016;
    res.vel = vel1;
    res.acc = acc1;
    return res;
}

void CKnowledge::sortByX(QList <CAgent *> &_agents ){
    for( int i=0 ; i<_agents.size() ; i++ ){
        for( int j=i+1 ; j<_agents.size() ; j++ ){
            if( _agents.at(i)->pos().x > _agents.at(j)->pos().x )
                swap(_agents[i] , _agents[j]);
        }
    }
}

void CKnowledge::sortByY(QList <CAgent *> &_agents ){
    for( int i=0 ; i<_agents.size() ; i++ ){
        for( int j=i+1 ; j<_agents.size() ; j++ ){
            if( _agents.at(i)->pos().y > _agents.at(j)->pos().y )
                swap(_agents[i] , _agents[j]);
        }
    }
}

int CKnowledge::nonPlayOnFastestSelector(QList <CAgent *> _agents){
    double minDist = 1000;
    for( int i=0 ; i<_agents.size() ; i++ ){
        if( _agents.at(i)->pos().dist(wm->ball->pos) < minDist ){
            minDist = _agents.at(i)->pos().dist(wm->ball->pos);
        }
    }

    for( int i=0 ; i<_agents.size() ; i++ ){
        if( fabs(_agents.at(i)->pos().dist(wm->ball->pos) - minDist) > 0.10 ){
            _agents.removeAt(i);
            i--;
        }
    }

    if( _agents.size() > 1 ){
        if( _agents.size() == 2 ){
            if( wm->ball->pos.y > 0 ){
                sortByY(_agents);
                return _agents.last()->id();
            }
            else if( wm->ball->pos.y < 0 ){
                sortByY(_agents);
                return _agents.first()->id();
            }
            return _agents.at(0)->id();
        }
        else{
            if( wm->ball->pos.y > _FIELD_HEIGHT/6 ){
                sortByY(_agents);
                return _agents.last()->id();
            }
            else if( wm->ball->pos.y < -_FIELD_HEIGHT/6 ){
                sortByY(_agents);
                return _agents.first()->id();
            }
            sortByY(_agents);
            return _agents.at(1)->id();
        }
    }
    else if( _agents.size() == 1 ){
        return _agents.at(0)->id();
    }

    return -1;
}

int CKnowledge::newFastestSelector(QList<CAgent *> _agents){

    if( (knowledge->isOurNonPlayOnKick() || knowledge->isTheirNonPlayOnKick())){
        return nonPlayOnFastestSelector(_agents);
    }

    double ballVel = wm->ball->vel.length();
    double distFactor = 0.4;
    double degFactor = 0.6;

    if( ballVel < 0.3 ){
        double minDist = 1000;
        int ID = -1;
        for( int i=0 ; i<_agents.size() ; i++ ){

            double deg = fabs(Vector2D::angleBetween(_agents.at(i)->pos()-wm->ball->pos , Vector2D(1 , 0)).degree());
            double dist = _agents.at(i)->pos().dist(wm->ball->pos);

            if( deg > 90 )
                degFactor = 0 , distFactor = 3;
            else
                degFactor = 0 , distFactor = 1;


            if( distFactor*(dist/6.0) + degFactor*(deg/180.0) < minDist ){
                minDist = distFactor*(dist/6.0) + degFactor*(deg/180.0);
                ID = _agents.at(i)->id();
            }
        }
        debug("first" , D_MASOOD);
        return ID;
    }
    else{

        Segment2D ballSeg(wm->ball->pos , wm->ball->pos+wm->ball->vel*3.0);
        Vector2D sol1 , sol2;

        double minDist = 1000;
        int ID = -1;
        double rad;
        for( int i=0 ; i<_agents.size() ; i++ ){
            rad = (0.4/-9.0)*ballVel - (0.4/-9.0)*10 + 0.1;
            if( rad < 0.1 )
                rad = 0.1;
            if( Circle2D(_agents.at(i)->pos() , rad).intersection(ballSeg , &sol1 , &sol2) > 0 ){
                if( _agents.at(i)->pos().dist(wm->ball->pos) < minDist ){
                    minDist = _agents.at(i)->pos().dist(wm->ball->pos);
                    ID = _agents.at(i)->id();
                }
            }
        }

        if( ID != -1 ){
            debug("second" , D_MASOOD);
            return ID;
        }

        minDist = 1000;
        ID = -1;
        for( int i=0 ; i<_agents.size() ; i++ ){
            double deg = fabs(Vector2D::angleBetween(_agents.at(i)->pos()-wm->ball->pos , Vector2D(1 , 0)).degree());
            double dist = _agents.at(i)->pos().dist(wm->ball->pos);
            if( distFactor*(dist/6.0) + degFactor*(deg/180.0) < minDist ){
                minDist = distFactor*(dist/6.0) + degFactor*(deg/180.0);
                ID = _agents.at(i)->id();
            }
        }
        debug("third" , D_MASOOD);
        return ID;
    }

    return -1;
}

void CKnowledge::SRSetAgentArg(int _id, double _Vx, double _Vy, double _Vr, double _KickSpeed, double _ChipSpeed, double _SpinSpeed)
{
    /*if(knowledge->getAgent(_id) == NULL)
        return;*/

    CRAgent[_id].Vx = _Vx;
    CRAgent[_id].Vy = _Vy;
    CRAgent[_id].Vr = _Vr;

    CRAgent[_id].KickSpeed = _KickSpeed;
    CRAgent[_id].ChipSpeed = _ChipSpeed;
    CRAgent[_id].SpinSpeed = _SpinSpeed;

    CRAgent[_id].updated = true;
}

void CKnowledge::SRSetAgentAbsArg(int _id, double _Vx, double _Vy, double _Vr, double _KickSpeed, double _ChipSpeed, double _SpinSpeed)
{
    if(knowledge->getAgent(_id) == NULL)
        return;

    double ang = -knowledge->getAgent(_id)->dir().th().radian();
    SRSetAgentArg(_id,
                  (cos(ang) * _Vx) - (sin(ang) * _Vy),
                  (sin(ang) * _Vx) + (cos(ang) * _Vy),
                  _Vr,
                  _KickSpeed,
                  _ChipSpeed,
                  _SpinSpeed);
}

bool CKnowledge::SRGetAgentArg(int _id, SRAgentArgs &_arg)
{
    if(!CRAgent[_id].updated)
    {
        CRAgent[_id].updated = false;
        return false;
    }
    CRAgent[_id].updated = false;
    _arg = CRAgent[_id];
    return true;
}

bool CKnowledge::SRIsUpdated(int _id)
{
    return CRAgent[_id].updated;
}
double CKnowledge::getKickSpeedProfile(int agentId,double kickSpeedInput){

    return ProfilerResult[0][agentId][(int)round(kickSpeedInput*10)]/100;
}

void CKnowledge::setNecessaryDefKick(bool tempNcssryDefKick) {
    necessaryDefKick = tempNcssryDefKick;
}

bool CKnowledge::getNecessaryDefKick() {
    return necessaryDefKick;
}


Vector2D CKnowledge::getChipDir(){


    chipperDistance=100;

    for(int i=0;i<wm->opp.activeAgentsCount();i++){
        chipperIDD=wm->opp.activeAgentID(i);
        if(!lastDirs.keys().contains(chipperIDD))
            lastDirs[chipperIDD]=QList<Vector2D>();
//        else
//            lastDirs.remove(chipperIDD);

        if(Circle2D(wm->opp[chipperIDD]->pos , 0.2).contains(wm->ball->pos)){
            if((wm->ball->pos-wm->opp[chipperIDD]->pos).length()<chipperDistance && wm->ball->vel.length()<0.1){
                chipperID=chipperIDD;
                chipperDistance=(wm->ball->pos-wm->opp[chipperIDD]->pos).length();
                chipperPoint=wm->opp[chipperID]->pos;
            }
            lastDirs[chipperIDD].append(wm->opp[chipperIDD]->dir);
            if(lastDirs[chipperIDD].count() > 10)
                lastDirs[chipperIDD].removeFirst();
        }
        else if (chipperID==chipperIDD && wm->ball->vel.length()>0.1) {
            DirFound=true;
        }
        if(wm->ball->vel.length()<0.1)
            DirFound=false;
    }
    debug(QString("chipperID:%1").arg(chipperID),D_NADIA);
if(DirFound)
{
    chipperDir=Vector2D(0,0);
    for(int i=lastDirs[chipperID].count()-2; i>3; i--){
        chipperDir+=lastDirs[chipperID].at(i-1);
    }
    chipperDir/=5;
    return chipperDir;
}
else
    return Vector2D(0,0);


prevBallPos=wm->ball->pos;

}

double CKnowledge::refine(double x) {
    //    return x*x*3/30 - x*2 + 70;
    return x*x/1.5 - x*0.3 + 50;
    //    return 3.98 - x*1.01 + x*x*0.13;
    //    return 3.43 - x*0.74 + x*x*0.085;
    //    return 32;
}

double CKnowledge::predictPos(){
    double sum=0;
    sum = prev_Ball_pos.at(prev_Ball_pos.length() - 1) - prev_Ball_pos.at(0);
    sum /= ( prev_Ball_pos.length()-1 );
    return sum;
}


Vector2D CKnowledge::getChipPredict(){
    dir=knowledge->getChipDir();
    draw(Segment2D(knowledge->chipperPoint , dir*10+knowledge->chipperPoint));
    debug(QString("dir_x:%1,dir_y:%2").arg(dir.x).arg(dir.y),D_NADIA);
    startChipPoint=knowledge->chipperPoint;




    if(wm->ball->vel.length() < 0.1 /*|| Circle2D(Vector2D(wm->ball->pos,0.03).contains()*/){
        chipPredictCounter=1;
        predictedBallPoses.clear();
        prev_Ball_pos.clear();

    }
    else
    {
//        prev_Ball_pos.append(wm->ball->pos.y);

        prev_Ball_pos.append(/*( ((wm->ball->pos - startChipPoint).angleWith(dir - startChipPoint)).tan() )**/(wm->ball->pos - startChipPoint).length());

        double x=predictPos()*refine(chipPredictCounter)/(double)chipPredictCounter;
        debug(QString("predict pos:%1").arg(x),D_NADIA);
        if(chipPredictCounter<2)
            refiner=1+x*0.8;
        predictedBallPoses.append(( x + 1 )*(refiner)*(dir.norm())+startChipPoint);

        predictedPosition=Vector2D(0,0);

        for(int i=0;i<predictedBallPoses.size();i+=1){
            if(i>14)
                break;
            predictedPosition+=predictedBallPoses.at(i);
            draw(predictedBallPoses.at(i));
        }
        if(predictedBallPoses.size()<15)
            predictedPosition/=predictedBallPoses.size();
        else
            predictedPosition/=15;

        debug(QString("counter: %1").arg(chipPredictCounter),D_NADIA);

    }
    chipPredictCounter++;
    return predictedPosition;
}

struct VectorIndex {
    Vector2D vec;
    int index;
};
void CKnowledge::Aminshoot(rcsc::Vector2D ball, QList<rcsc::Circle2D> obstacles, double &_empty, rcsc::Vector2D &_best) {


    VectorIndex vecc[12],temp;
    Vector2D tangents[12];
    Segment2D ttan[12];
    Ray2D     tan[12];
    Vector2D zol1, zol2;



    Segment2D oppgoalLine(wm->field->oppCornerL(), wm->field->oppCornerR());
    Segment2D oppfiledLine(wm->field->oppGoalL(),wm->field->oppGoalR());


    for (int i=0;i<obstacles.size();i++){
        obstacles[i].tangent(ball, &tangents[2*i], &tangents[2*i +1]);

    }

    int x=0,km=0;
    for(int i=0; i<obstacles.size()*2;i++,km++)
    {
        tan[i]= Ray2D(ball,tangents[i]);
        ttan[i]= Segment2D(ball,oppgoalLine.intersection(tan[i].line()));
        for (int j=0;j<obstacles.size();j++) {
            if(obstacles[j].intersection(ttan[i],&zol1,&zol2) == 2)
                x=1;

        }
        if((x==0)&&(oppfiledLine.intersection(ttan[i]).valid()))
        {
            vecc[i].vec = oppgoalLine.intersection(tan[i].line());
        }
        else vecc[i].vec = Vector2D(wm->field->oppGoal().x,-i-5);
        x=0;

    }

    for(int i=0;i<obstacles.size()*2;i=i+2)
    {
        if(tangents[i].y>tangents[i+1].y)
        {
            vecc[i].index=1;
            vecc[i+1].index=0;
        }
        else
        {
            vecc[i].index=0;
            vecc[i+1].index=1;
        }
    }



    for(int x=0;x<obstacles.size()*2 ;x++)
    {
        for(int z=0;z< obstacles.size()*2;z++)
        {
            if(vecc[z].vec.y < vecc[z+1].vec.y)
            {
                temp = vecc[z];
                vecc[z] = vecc[z+1];
                vecc[z+1] = temp;
            }
        }
    }



    Vector2D f  = Vector2D(wm->field->center());
    Vector2D s  = Vector2D(wm->field->center());
    Vector2D y1 = Vector2D(wm->field->center());
    Vector2D y2 = Vector2D(wm->field->center());


    if(vecc[0].vec.y > wm->field->oppGoalR().y && vecc[0].vec.y < wm->field->oppGoalL().y)
    {
        if(vecc[0].index==1 )
        {
            f = Vector2D(wm->field->oppGoalL().x,wm->field->oppGoalL().y-.05);
            s = vecc[0].vec;
            y1=f; y2=s;

        }
        else {
            f = vecc[0].vec;
            if(vecc[1].vec.y > wm->field->oppGoalR().y && vecc[1].vec.y < wm->field->oppGoalL().y)
                s= vecc[1].vec;
            else
                s=Vector2D(wm->field->oppGoalR().x,wm->field->oppGoalR().y+.05);
            if(fabs(y2.y-y1.y) < fabs(s.y-f.y))
            {
                y2=s;
                y1=f;
            }

        }
    }

    for(int i=1;i<obstacles.size()*2;i++)
    {

        if(vecc[i].index==1)
            continue;

        if(vecc[i].vec.y>wm->field->oppGoalR().y && vecc[i].vec.y<wm->field->oppGoalL().y)
        {
            f = vecc[i].vec;
            if(vecc[i+1].vec.y>wm->field->oppGoalR().y && vecc[i+1].vec.y<wm->field->oppGoalL().y)
                s= vecc[i+1].vec;
            else
                s=Vector2D(wm->field->oppGoalR().x,wm->field->oppGoalR().y+.05);


            if( fabs(y2.y-y1.y) < fabs(s.y-f.y))
            {
                y2=s;
                y1=f;

            }
            i++;

        }


    }

    debug(QString("AA%1 %2 %3 %4 %5").arg(_best.x).arg("||").arg(_best.y).arg("||").arg(_empty),D_ALI);


    // Line2D bisectorLine (originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    draw(y2);
    draw(y1);
    _empty = y1.y-y2.y;

    if((y1==wm->field->center() && y2 == wm->field->center()) || obstacles.isEmpty())
    {
        Line2D bisectorLine (ball , AngleDeg::bisect((wm->field->oppGoalL() - ball).th() , (wm->field->oppGoalR() - ball).th()));
        _best = oppgoalLine.intersection(bisectorLine);

        _empty=1;
        draw(_best,0,QColor(Qt::red));
    }

    else
    {


        if (y1.y==wm->field->oppGoalL().y-.05)
        {
            _best= y1;
        }
        else if(y2.y==wm->field->oppGoalR().y+.05)
        {
            _best=y2;
        }
        else
        {
            Line2D bisectorLine (ball , AngleDeg::bisect((y1 - ball).th() , (y2 - ball).th()));
            _best= oppgoalLine.intersection(bisectorLine);

        }
        draw(_best,0,QColor(Qt::red));
    }
    debug(QString("%1 %2 %3 %4 %5").arg(_best.x).arg("||").arg(_best.y).arg("||").arg(_empty),D_ALI);





}


//added
Vector2D CKnowledge::getBPPosition(){
    return bpPosition;
}
void CKnowledge::setBPPosition(float x, float y){
    bpPosition = Vector2D(x,y);
}
