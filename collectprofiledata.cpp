
#include "collectprofiledata.h"
#include <knowledge.h>
#include "geom/triangle_2d.h"
#include "logger.h"


CollectProfileData *collectKickProfile;
bool ProfilerExecute=false;


CollectProfileData::CollectProfileData()
{
    for(int i=0;i<10;i++)
        activeRobots[i]=-1;
    activeRobotsCount=0;
    prfl1 = new CRolePlayOff();
    prfl2 = new CRolePlayOff();
    profiler = new CNewProfiler();
    prfState = InitState;
    MinSpeed = 300;
    middleSpeed = 500;

    isChip = true;
}

void CollectProfileData::init(int p1, int p2){

    debug(QString("__%1__%2__").arg(p1).arg(p2),D_NADIA);
    prfl1->setAgent(knowledge->getAgent(p1));
    prfl1->setAgentID(p1);
    prfl1->setKickSpeed(kickSpeed1);
    prfl1->setReceiveRadius(1.2);
    prfl1->setTolerance(0.01);
    prfl1->setChip(false);
    prfl1->setSlow(true);
    prfl2->setAgent(knowledge->getAgent(p2));
    prfl2->setAgentID(p2);
    prfl2->setKickSpeed(kickSpeed2);
    prfl2->setReceiveRadius(1.2);
    prfl2->setTolerance(0.01);
    prfl2->setChip(false);
    prfl2->setSlow(true);

    kickerPos = knowledge->getAgent(prfl1->getAgentID())->pos();

    ballSpeed = 0;
    kickSpeed2 = MinSpeed;
    kickSpeed1 = MinSpeed;
    prfl1_Kicked = true;

    speedStep = 100;
    counter1 = -2;
    counter2 = -2;

}

void CollectProfileData::ChipInit(int p1){
    //    debug(QString("%1_________").arg(p1), D_FATEMEH);

    prfl1->setAgent(knowledge->getAgent(p1));
    prfl1->setAgentID(p1);
    prfl1->setKickSpeed(kickSpeed1);
    prfl1->setReceiveRadius(1.2);
    prfl1->setTolerance(0.01);
    prfl1->setChip(true);
    prfl1->setSlow(true);

    kickSpeed1 = MinSpeed;
    speedStep = 100;
    counter1 = -2;

    BallPos.append(wm->ball->pos);
}

void CollectProfileData::positioning(double xpos1, double ypos1, double xpos2, double ypos2){
    prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
    prfl1->setTarget(Vector2D(xpos1, ypos1));
    prfl1->setWaitPos(Vector2D(xpos1, ypos1));

    prfl2->setSelectedSkill(roleSkill::GotopointAvoid);
    prfl2->setTarget(Vector2D(xpos2, ypos2));
    prfl2->setWaitPos(Vector2D(xpos2, ypos2));

    prfl1->execute();
    prfl2->execute();
}

void CollectProfileData::positioning(double xpos, double ypos){
    prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
    prfl1->setTarget(Vector2D(xpos, ypos));
    prfl1->setWaitPos(Vector2D(xpos, ypos));

    prfl1->execute();
}

bool CollectProfileData::BallIsNear(CRolePlayOff * agent, double rad){
    if(Circle2D(knowledge->getAgent(agent->getAgentID())->pos(), rad).contains(wm->ball->pos)){
        rcvr = prfl2;
        pss = prfl1;
        kickStat = prfl1_Iskicking;
        return true;
    }
    return false;
    return true;
}

void CollectProfileData::profilerDraws(){
    if(prfl1->getSelectedSkill()==roleSkill::Kick)
        draw(Circle2D( knowledge->getAgent(prfl1->getAgentID())->pos(), 0.5 ), QColor(Qt::black));

    if(prfl2->getSelectedSkill()==roleSkill::Kick)
        draw(Circle2D( knowledge->getAgent(prfl2->getAgentID())->pos(), 0.5 ), QColor(Qt::black));

    if(prfl1->getSelectedSkill()==roleSkill::ReceivePass)
        draw(Circle2D( knowledge->getAgent(prfl1->getAgentID())->pos(), 0.5 ), QColor(Qt::darkGreen));

    if(prfl2->getSelectedSkill()==roleSkill::ReceivePass)
        draw(Circle2D( knowledge->getAgent(prfl2->getAgentID())->pos(), 0.5 ), QColor(Qt::darkGreen));
    if(prfl1->getSelectedSkill()==roleSkill::GotopointAvoid)
        draw(Circle2D( knowledge->getAgent(prfl1->getAgentID())->pos(), 0.5 ), QColor(Qt::darkCyan));

    if(prfl2->getSelectedSkill()==roleSkill::GotopointAvoid)
        draw(Circle2D( knowledge->getAgent(prfl2->getAgentID())->pos(), 0.5 ), QColor(Qt::darkCyan));
}

void CollectProfileData::LowSpeed(){

    static Triangle2D downTri = Triangle2D( Vector2D( 0, 0) ,Vector2D(-_FIELD_WIDTH/2, 0), Vector2D(0, _FIELD_HEIGHT/2) );
    static Triangle2D upTri = Triangle2D( Vector2D(-_FIELD_WIDTH/2, _FIELD_HEIGHT/2) ,Vector2D(-_FIELD_WIDTH/2, 0), Vector2D(0, _FIELD_HEIGHT/2) );
    static Triangle2D *currTri;

    draw(Segment2D(Vector2D(-_FIELD_WIDTH/2, 0), Vector2D(0, _FIELD_HEIGHT/2) ), QColor(Qt::white));
    draw(Segment2D(Vector2D(0, 0), Vector2D(0, _FIELD_HEIGHT/2) ), QColor(Qt::white));
    draw(Segment2D(Vector2D(-_FIELD_WIDTH/2, 0), Vector2D(0, 0) ), QColor(Qt::white));

    profilerDraws();

    switch(kickStat){
    case prfl1_Iskicking:
        if(wm->ball->vel.length() < 0.1){           // ball is in downTri and its velocity is near 0 so agent1 kicks the ball
            prfl1->setSelectedSkill(roleSkill::Kick);
            prfl2->setSelectedSkill(roleSkill::ReceivePass);

            prfl1->setTarget(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            prfl2->setTarget(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            prfl1->setWaitPos(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            prfl2->setWaitPos(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            prfl1->setKickSpeed(kickSpeed1);
            debug(QString("prfl1 kick speed:%1").arg(prfl1->getKickSpeed()),D_NADIA);

            if(!Circle2D(prfl1->getTarget(), 0.15).contains(knowledge->getAgent(prfl2->getAgentID())->pos()))  // agent1 wait until agent2 is in its target
                prfl1->setDoPass(false);
            else
                prfl1->setDoPass(true);
        }
        else if(!Circle2D(kickerPos, 0.2).contains(wm->ball->pos)){    // agent1 kicked the ball and the ball is far enough
            prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
            prfl1->setWaitPos(Vector2D( -0.5, 0.5));
            prfl1->setTarget(Vector2D( -0.5, 0.5));
            currTri = &downTri;
            prfl1_Kicked = true;
            kickStat = ChangeStat;
        }

        break;


    case prfl2_Iskicking:
        if(wm->ball->vel.length() < 0.1){           // ball is in upTri and its velocity is near 0 so agent2 kicks the ball
            prfl2->setTarget(Vector2D( -0.5, 0.5));
            prfl1->setTarget(Vector2D( -0.5, 0.5));
            prfl2->setWaitPos(Vector2D( -0.5, 0.5));
            prfl1->setWaitPos(Vector2D( -0.5, 0.5));
            prfl2->setKickSpeed(kickSpeed2);
            if(!Circle2D(prfl2->getTarget(), 0.15).contains(knowledge->getAgent(prfl1->getAgentID())->pos()))  // agent2 wait until agent1 is in its target
                prfl2->setDoPass(false);
            else
                prfl2->setDoPass(true);
        }
        else if(!Circle2D(kickerPos, 0.2).contains(wm->ball->pos) ){   // agent2 kicked the ball and the ball is far enough
            prfl2->setSelectedSkill(roleSkill::GotopointAvoid);
            prfl2->setWaitPos(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            prfl2->setTarget(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            currTri = &upTri;
            prfl1_Kicked = false;
            kickStat = ChangeStat;
        }

        break;


    case ChangeStat:

        if(wm->ball->vel.length() > ballSpeed){
            ballSpeed = wm->ball->vel.length();
        }


        if(wm->ball->vel.length() < 0.01){

            if(upTri.contains(wm->ball->pos)){ // ball is near to receiver; changing skills


                if(!BallIsNear(prfl2,1)){
                    prfl2->setSelectedSkill(roleSkill::GotopointAvoid);
                    prfl2->setWaitPos(wm->ball->pos);
                    prfl2->setTarget(wm->ball->pos);
                }
                else{
                    saveMaxBallSpeed();
                    kickerPos = knowledge->getAgent(prfl2->getAgentID())->pos();
                    kickStat = prfl2_Iskicking;
                }
            }
            else{


                if(!BallIsNear(prfl1,1)){
                    prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
                    prfl1->setWaitPos(wm->ball->pos);
                    prfl1->setTarget(wm->ball->pos);
                }else{
                    saveMaxBallSpeed();
                    kickerPos = knowledge->getAgent(prfl1->getAgentID())->pos();
                    kickStat = prfl1_Iskicking;
                }
            }

        }


        break;
    }
    if(kickSpeed1 >= middleSpeed && kickSpeed2 >= middleSpeed )
        prfState = Pos2;
}

void CollectProfileData::saveMaxBallSpeed(){
    if(prfl1_Kicked){
        if(counter1 >= 0){
            if(!(ballSpeed<=(kickSpeed1/300))){
                p1RealSpeedRec.append(ballSpeed);
                p1KickSpeed.append(kickSpeed1);
                debug(QString("1 : max ball speed : %1, kSpeed : %2").arg(ballSpeed).arg(kickSpeed1), D_NADIA);
            }
            else
                counter1--;
        }
        counter1++;
        if(kickSpeed1==1000 && counter1==repeat){
            counter1 = 0;
            profiler->robotsProfile[prfl1->getAgentID()].kickMap.insert(kickSpeed1 , p1RealSpeedRec);
            p1RealSpeedRec.clear();
            kickSpeed1 = MaxSpeed;
        }
        else if(counter1 == repeat && kickSpeed1<MaxSpeed+1){
            counter1 = 0;
            profiler->robotsProfile[prfl1->getAgentID()].kickMap.insert(kickSpeed1, p1RealSpeedRec);
            p1RealSpeedRec.clear();
            kickSpeed1 += speedStep;
        }
    }else{
        if(counter2 >= 0){
            if(!(ballSpeed<=(kickSpeed2/300))){
                p2RealSpeedRec.append(ballSpeed);
                p2KickSpeed.append(kickSpeed2);
                debug(QString("2 : max ball speed : %1, kSpeed : %2").arg(ballSpeed).arg(kickSpeed2), D_NADIA);
            }
            else counter2--;
        }
        counter2++;
        if(kickSpeed2==1000  && counter1==repeat){
            counter2 = 0;
            profiler->robotsProfile[prfl2->getAgentID()].kickMap.insert(kickSpeed2 , p2RealSpeedRec);
            p2RealSpeedRec.clear();
            kickSpeed2 = MaxSpeed;
        }
        else if(counter2 == repeat && kickSpeed2<MaxSpeed+1){
            counter2 = 0;
            profiler->robotsProfile[prfl2->getAgentID()].kickMap.insert(kickSpeed2, p2RealSpeedRec);
            p2RealSpeedRec.clear();
            kickSpeed2 += speedStep;
        }
    }
    ballSpeed = 0;
}

void CollectProfileData::HighSpeed(){
    static Triangle2D downTri = Triangle2D( Vector2D( 0, 0) ,Vector2D(-_FIELD_WIDTH/2, 0), Vector2D(0, _FIELD_HEIGHT/2) );
    static Triangle2D upTri = Triangle2D( Vector2D(-_FIELD_WIDTH/2, _FIELD_HEIGHT/2) ,Vector2D(-_FIELD_WIDTH/2, 0), Vector2D(0, _FIELD_HEIGHT/2) );
    static Triangle2D *currTri;


    draw(Segment2D(Vector2D(-_FIELD_WIDTH/2, 0), Vector2D(0, _FIELD_HEIGHT/2) ), QColor(Qt::white));
    draw(Segment2D(Vector2D(0, 0), Vector2D(0, _FIELD_HEIGHT/2) ), QColor(Qt::white));
    draw(Segment2D(Vector2D(-_FIELD_WIDTH/2, 0), Vector2D(0, 0) ), QColor(Qt::white));

    switch(kickStat){
    case prfl1_Iskicking:
        if(wm->ball->vel.length() < 0.1){           // ball is in downTri and its velocity is near 0 so agent1 kicks the ball

            prfl1->setTarget(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            prfl2->setTarget(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            prfl1->setWaitPos(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            prfl2->setWaitPos(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            prfl1->setKickSpeed(kickSpeed1);

            if(!Circle2D(prfl1->getTarget(), 0.15).contains(knowledge->getAgent(prfl2->getAgentID())->pos()))  // agent1 wait until agent2 is in its target
                prfl1->setDoPass(true);
            else
                prfl1->setDoPass(false);
        }
        else if(!Circle2D(kickerPos, 0.2).contains(wm->ball->pos)){    // agent1 kicked the ball and the ball is far enough
            prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
            prfl1->setWaitPos(Vector2D( -0.8, -_FIELD_HEIGHT/2+0.8));
            prfl1->setTarget(Vector2D( -0.8, -_FIELD_HEIGHT/2+0.8));
            currTri = &downTri;
            prfl1_Kicked = true;
            kickStat = ChangeStat;
        }

        break;


    case prfl2_Iskicking:


        if(wm->ball->vel.length() < 0.1){           // ball is in upTri and its velocity is near 0 so agent2 kicks the ball
            prfl2->setTarget(Vector2D( -0.8, -_FIELD_HEIGHT/2+0.8));
            prfl1->setTarget(Vector2D( -0.8, -_FIELD_HEIGHT/2+0.8));
            prfl2->setWaitPos(Vector2D( -0.8, -_FIELD_HEIGHT/2+0.8));
            prfl1->setWaitPos(Vector2D( -0.8, -_FIELD_HEIGHT/2+0.8));
            prfl2->setKickSpeed(kickSpeed2);

            if(!Circle2D(prfl2->getTarget(), 0.15).contains(knowledge->getAgent(prfl1->getAgentID())->pos()))  // agent2 wait until agent1 is in its target
                prfl2->setDoPass(true);
            else
                prfl2->setDoPass(false);
        }
        else if(!Circle2D(kickerPos, 0.2).contains(wm->ball->pos) ){   // agent2 kicked the ball and the ball is far enough
            prfl2->setSelectedSkill(roleSkill::GotopointAvoid);
            prfl2->setWaitPos(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            prfl2->setTarget(Vector2D(-_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8));
            currTri = &upTri;
            prfl1_Kicked = false;
            kickStat = ChangeStat;
        }

        break;


    case ChangeStat:

        if(wm->ball->vel.length() > ballSpeed){
            ballSpeed = wm->ball->vel.length();
        }
        //        qDebug()<<wm->ball->vel.length();

        if(wm->ball->vel.length() < 0.01){

            if(wm->ball->pos.y > 0){ // ball is near to receiver; changing skills

                if(!BallIsNear(prfl2,0.6)){
                    prfl2->setSelectedSkill(roleSkill::GotopointAvoid);
                    prfl2->setWaitPos(wm->ball->pos);
                    prfl2->setTarget(wm->ball->pos);
                }
                else{
                    saveMaxBallSpeed();
                    kickerPos = knowledge->getAgent(prfl2->getAgentID())->pos();
                    kickStat = prfl2_Iskicking;

                    prfl2->setSelectedSkill(roleSkill::GotopointAvoid);
                    prfl2->setTarget(knowledge->getAgent(prfl2->getAgentID())->pos());
                    prfl2->setWaitPos(knowledge->getAgent(prfl2->getAgentID())->pos());

                }
            }else{

                if(!BallIsNear(prfl1,0.6)){
                    prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
                    prfl1->setWaitPos(wm->ball->pos);
                    prfl1->setTarget(wm->ball->pos);
                }else{
                    saveMaxBallSpeed();
                    kickerPos = knowledge->getAgent(prfl1->getAgentID())->pos();
                    kickStat = prfl1_Iskicking;

                    prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
                    prfl1->setTarget(knowledge->getAgent(prfl1->getAgentID())->pos());
                    prfl1->setWaitPos(knowledge->getAgent(prfl1->getAgentID())->pos());

                }
            }


            break;
        }
    }

    prfl2->execute();
    prfl1->execute();

    if(kickSpeed1 > 1000 && kickSpeed2 > 1000 ){
        activeRobotsCount+=2;
        prfState=goOutState;
    }
}

bool CollectProfileData::FindChipPos(){
//    FoundChipPos = BallPos.at(BallPos.size()/2);
//    BallPos.clear();
//    return true;

    int startIndex = kickSpeed1/80+7;
    Vector2D V = BallPos.at(startIndex);
    double tangent = V.th().tan();
    bool result = false;

    for(int i=startIndex; i<BallPos.size()-5; i++)
    {
        tangent = V.th().tan();
        V = BallPos.at(i) - BallPos.at(i+5);

        if(tangent * V.th().tan() < 0 && fabs(tangent - V.th().tan()) > 0.6){
            FoundChipPos = BallPos.at(i);
            result = true;
            debug("________ball pos cleared in func", D_FATEMEH);
            BallPos.clear();
        }
    }

    return result;

}

void CollectProfileData::StartChip(){

    static double dist=0;
    static int posSize = 50, cnt = 0, rep = 1;
    static bool result = false;
    //    static bool chipped = true;

    if(BallPos.size() == 0)
        BallPos.append(wm->ball->pos);

    if(/*fabs(wm->ball->pos.x - BallPos.last().x) > 0.005 &&*/ fabs(wm->ball->pos.y - BallPos.last().y) > 0.005 )
        BallPos.append(wm->ball->pos);

    if(BallPos.size() > posSize)
        BallPos.removeFirst();

    debug(QString("STATE2 : %1").arg(ChipStat), D_MAHI);

    switch (ChipStat) {
    case Emplacement:

        prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
        prfl1->setTarget(Vector2D(-_FIELD_WIDTH/2+1, -_FIELD_HEIGHT/2+0.4));
        prfl1->execute();

        if ( Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos(), 0.4).contains(wm->ball->pos) && wm->ball->vel.length() < 0.2)
        {
            debug("done with emplacement", D_FATEMEH);

            ChipStat = ChipKick;

            debug("________ball pos cleared in emplacement", D_FATEMEH);
            BallPos.clear();
        }

        break;

    case ChipKick:

        prfl1->setSelectedSkill(roleSkill::Kick);
        prfl1->setChip(true);
        prfl1->setTarget(Vector2D(-_FIELD_WIDTH/2+1, _FIELD_HEIGHT/2-0.4));
        prfl1->setKickSpeed(kickSpeed1);
        prfl1->setTolerance(0.02);
        prfl1->setDoPass(true);
        prfl1->execute();

        if(!Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos(), 0.12).contains(wm->ball->pos) && wm->ball->vel.length() > 0.1)
            ChipStat = SavingChipPos;

        if(!Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos(), 0.3).contains(wm->ball->pos) && wm->ball->vel.length() > 0.2)
        {
            ChipStartPoint = wm->ball->pos;

            debug("done with chip kick", D_FATEMEH);

            result = false;

            debug("________ball pos cleared", D_FATEMEH);
            BallPos.clear();
        }

        break;

    case SavingChipPos:
        prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
        prfl1->setTarget(Vector2D(-_FIELD_WIDTH/2+1, -_FIELD_HEIGHT/2+0.4));
        prfl1->execute();

        if(BallPos.size() == posSize)
            result = FindChipPos();

        if (result) // position is found;
        {
            debug("pos saved", D_FATEMEH);

            SavedChipPos.append(ChipStartPoint);
            SavedChipPos.append(FoundChipPos);
            cnt++;

            if(cnt == repeat) {
                kickSpeed1 += speedStep;
                cnt = 0;
            }
            if (kickSpeed1 > 1023) {
                prfState = SaveProf;
            }

            ChipStat = Emplacement;
        }

        if(!Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos(), 5.5).contains(wm->ball->pos)
                && !result)   // ball is far enough and position isn't found, reset
        {
            BallPos.clear();
            debug("________ball pos cleared, pos not found", D_FATEMEH);
            ChipStat = Emplacement;
        }

        break;
    }


    for(int i=0; i<SavedChipPos.size();i+=2)
        draw(Segment2D(SavedChipPos.at(i), SavedChipPos.at(i+1)), QColor(Qt::black));

    Q_FOREACH(Vector2D v, BallPos)
        draw(v, 0, QColor(Qt::cyan), 1);

    static Segment2D sgm ;
    if(SavedChipPos.size()>1){
        draw(SavedChipPos.at(0));
        draw(SavedChipPos.at(1));

        sgm = Segment2D(SavedChipPos.at(SavedChipPos.size()-2), SavedChipPos.at(SavedChipPos.size()-1));
    }
    draw(sgm, QColor(Qt::darkRed));

    if(SavedChipPos.size() == 2*rep){
        dist = 0;
        for(int i=0; i<SavedChipPos.size();i+=2)
            dist += SavedChipPos.at(i).dist(SavedChipPos.at(i+1));
        dist/=(SavedChipPos.size()/2.0);

        ChipResult.keys().append(kickSpeed1);
        ChipResult.values().append(dist);

        draw(SavedChipPos.at(0), 0, QColor(Qt::darkRed));
        draw(SavedChipPos.at(1), 0, QColor(Qt::white));

        SavedChipPos.clear();
        debug(QString("speed : %1, dist : %2").arg(kickSpeed1).arg(dist), D_FATEMEH);
    }

}

void CollectProfileData::start(){


    debug(QString("STATE : %1").arg(prfState), D_MAHI);
    switch(prfState){

    case InitState:
        if(!isChip)    //kick
        {
            if(activeRobots[activeRobotsCount]!=-1){
                if(activeRobots[activeRobotsCount+1]!= -1){
                    init(activeRobots[activeRobotsCount],activeRobots[activeRobotsCount+1]);
                }
                else
                    init(activeRobots[activeRobotsCount],activeRobots[0]);
                prfState = Pos1;
            }
            else{
                prfState=SaveProf;
            }
        }
        else        //chip
        {
            debug("chip init", D_FATEMEH);
            prfState = InitStateChip;
        }

        break;

    case Pos1:
        positioning( -0.5, 0.5, -_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8);
        if( Circle2D(knowledge->getAgent(prfl2->getAgentID())->pos(), 0.1).contains(prfl2->getTarget()) &&
                Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos(), 0.1).contains(prfl1->getTarget()) ){
            kickStat=ChangeStat;
            prfState = StartLow;
        }

        break;

    case goOutState:
        positioning( -_FIELD_WIDTH/2, -_FIELD_HEIGHT/2, -_FIELD_WIDTH/2, -_FIELD_HEIGHT/2);
        if( Circle2D(knowledge->getAgent(prfl2->getAgentID())->pos(), 0.4).contains(prfl2->getTarget()) &&
                Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos(), 0.4).contains(prfl1->getTarget()) ){
            prfState=InitState;
        }
        else
            debug("finished",D_NADIA);

        break;

    case StartLow:
        LowSpeed();

        break;

    case Pos2:
        positioning( -0.8, -_FIELD_HEIGHT/2+0.8,  -_FIELD_WIDTH/2+0.8, _FIELD_HEIGHT/2-0.8);
        if( Circle2D(knowledge->getAgent(prfl2->getAgentID())->pos(), 0.1).contains(prfl2->getTarget()) &&
                Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos(), 0.1).contains(prfl1->getTarget()) ){
            kickStat=ChangeStat;
            prfState = StartHigh;
        }


        break;

    case StartHigh:
            HighSpeed();

        break;

    case SaveProf:
        profiler->save(JSON);
        prfState=endState;

        break;
        //chip
    case InitStateChip:

        ChipInit(activeRobots[0]);
        positioning(-_FIELD_WIDTH/2+1, -_FIELD_HEIGHT/2+0.4);
        draw(Vector2D(-_FIELD_WIDTH/2+1, -_FIELD_HEIGHT/2+0.4));

        //        positioning(-_FIELD_WIDTH/4, _FIELD_HEIGHT/4);
        //        draw(Vector2D(-_FIELD_WIDTH/4, _FIELD_HEIGHT/4));

        //        debug("done with pos", D_FATEMEH);

        if(Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos(), 0.15).contains(prfl1->getTarget())){
            //            debug("robot in position", D_FATEMEH);
            if(Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos(), 0.2).contains(wm->ball->pos)){
                prfState = ChipStart;
                ChipStat = Emplacement;
            }
        }

        break;

    case ChipStart:
        //        debug("start chip", D_FATEMEH);
        StartChip();
        break;

    }
}
