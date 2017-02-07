#ifndef EXPERIMENTAL4_H
#define EXPERIMENTAL4_H
#include <mainapplication.h>

#include "tools/loadplayoffjson.h"

const int VEL_SAMPLES = 10;

static const double FrameRate = 61.51;
static const double FramePeriod = 1.0 / FrameRate;

class C {
public:
    long time;
    Vector2D loc;


private:
};

QList<C> history;

Vector2D fitVelocity() {

    if (history.size() < VEL_SAMPLES || history[0].time - history[history.size() - 1].time > 2 * VEL_SAMPLES * wm->getVisionLatancy()) {
        return Vector2D(0, 0);
    }


    int samples = VEL_SAMPLES;
    double t0 = history[0].time;
    double sxx = 0, sxy = 0, sx = 0, sy = 0;
    double txx = 0, txy = 0, tx = 0, ty = 0;
    for (size_t i = 0; i < history.size(); i++) {

        const C& h = history[i];

        double t = h.time - t0;
        double x = h.loc.x;
        double y = h.loc.y;

        sxx += t * t;
        sxy += t * x;
        sx += t;
        sy += x;

        txx += t * t;
        txy += t * y;
        tx += t;
        ty += y;
    }

    double vx = (samples * sxy - sx * sy) / (samples * sxx - sx * sx);
    double vy = (samples * txy - tx * ty) / (samples * txx - tx * tx);

    return Vector2D(vx, vy);
}

void drawGraph(QList<double> data, QColor color) {

    draw(Segment2D(Vector2D(0, -_FIELD_HEIGHT/2), Vector2D(0, _FIELD_HEIGHT/2)), QColor(Qt::black));
    draw(Segment2D(Vector2D(0, -_FIELD_HEIGHT/2), Vector2D(_FIELD_WIDTH/2, -_FIELD_HEIGHT/2)), QColor(Qt::black));
    draw(Segment2D(Vector2D(0, -_FIELD_HEIGHT/2 + 1), Vector2D(_FIELD_WIDTH/2, -_FIELD_HEIGHT/2 + 1)), QColor(Qt::black));
    draw(Segment2D(Vector2D(0, -_FIELD_HEIGHT/2 + 2), Vector2D(_FIELD_WIDTH/2, -_FIELD_HEIGHT/2 + 2)), QColor(Qt::black));
    draw(Segment2D(Vector2D(0, -_FIELD_HEIGHT/2 + 3), Vector2D(_FIELD_WIDTH/2, -_FIELD_HEIGHT/2 + 3)), QColor(Qt::black));
    draw(Segment2D(Vector2D(0, -_FIELD_HEIGHT/2 + 4), Vector2D(_FIELD_WIDTH/2, -_FIELD_HEIGHT/2 + 4)), QColor(Qt::black));


    Polygon2D poly;

    for (size_t i = 0;i < data.size();i++) {
        double tempY, tempX;
        tempX = i * ((_FIELD_WIDTH / 2) / data.size() );
        tempY = (-_FIELD_HEIGHT)/2 + (double)(data[i])/2;

        poly.addVertex(Vector2D(tempX, tempY));
    }

    draw(poly, color);
}

QList<double> graph, graph2;


static CRolePlayOn * rcvr ;
static CRolePlayOn *pss ;

static CRolePlayOn * prfl1 = new CRolePlayOn();

static CRolePlayOn * prfl2 = new CRolePlayOn();

static int p2 = 4 , p1 = 6 , kickSpeed = 200 , MaxSpeed = 1023;
static double xpos2 = -3 , ypos2 = 2.5 , xpos1 = -3 , ypos1 = -2.5;

void CMainApplication::Experimental4()
{
    static bool ballIsNear = false , first = true;






    // fill profiler

    /*
      ////////////////////
      //////// not working
      ////////////////////

    static int rcvID = 5 , passID = 1 , tmp , kickSpeed = 4;
    static bool flag = false , ballIsNear = false;

//    static CRolePlayOff
    CSkillReceivePass* rcv = new CSkillReceivePass(knowledge->getAgent(rcvID));
    rcv->setReceiveRadius(1);
    rcv->setTarget(knowledge->getMousePos());
    rcv->setSlow(true);
    rcv->execute();

    CSkillKick* pass = new CSkillKick(knowledge->getAgent(passID));
    pass->setTarget(knowledge->getAgent(rcvID)->pos());
    pass->setKickSpeed(kickSpeed);
    pass->setThroughMode(false);
    pass->setSlow(true);
    pass->setInterceptMode(false);
    pass->setParallelMode(false);
    pass->setWaitFrames(0);
    pass->setTolerance(0.01);
    pass->setDontKick(false);
    pass->setChip(false);
    pass->execute();

    if(knowledge->getAgent(passID)->pos().dist(knowledge->getAgent(rcvID)->pos()) < 0.4)
        rcv->getAgent()->waitHere();
    else
        rcv->execute();

    if (Circle2D(pass->getAgent()->pos(), 0.3).contains(wm->ball->pos)) {
        ballIsNear = true;
    } else if ( !Circle2D(pass->getAgent()->pos(), 0.1*kickSpeed).contains(wm->ball->pos) && ballIsNear) {
        ballIsNear = false;
        flag = true;
    }

    if(flag){
        debug(QString("speed: %3  ,  rcv : %1 , pass : %2").arg(rcvID).arg(passID).arg(kickSpeed) , D_MAHI);

        flag = false;

        if(kickSpeed < 10)
            kickSpeed++;
        else
            kickSpeed = 1;

        tmp = rcvID;
        rcvID = passID;
        passID = tmp;
    }
*/



    //////////////////////////////////
    //////// changing position problem
    //////// almost fine /////////////
    //////////////////////////////////


    prfl1->setAgent(knowledge->getAgent(p1));
    prfl1->setAgentID(p1);
    prfl1->setKickSpeed(kickSpeed);
    prfl1->setReceiveRadius(2);
    prfl1->setTolerance(0.01);
    prfl1->setIsActive(true);
    prfl1->setChip(true);
    prfl1->setSlow(true);
    prfl1->execute();

    prfl2->setAgent(knowledge->getAgent(p2));
    prfl2->setAgentID(p2);
    prfl2->setKickSpeed(kickSpeed);
    prfl2->setReceiveRadius(2);
    prfl2->setTolerance(0.01);
    prfl2->setIsActive(true);
    prfl2->setChip(true);
    prfl2->setSlow(true);
    prfl2->execute();


    if(first){
        prfl2->setSelectedSkill(roleSkill::GotopointAvoid);
        prfl1->setSelectedSkill(roleSkill::GotopointAvoid);

        prfl1->setTargetDir(wm->ball->pos - knowledge->getAgent(p1)->pos());
        prfl2->setTargetDir(wm->ball->pos - knowledge->getAgent(p2)->pos());

        prfl1->setTarget(Vector2D(xpos1 , ypos1));
        prfl2->setTarget(Vector2D(xpos2 , ypos2));


        if(Circle2D(knowledge->getAgent(prfl2->getAgentID())->pos() , 0.4).contains(wm->ball->pos) ||
                Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos() , 0.4).contains(wm->ball->pos) ){
            prfl1->setSelectedSkill(roleSkill::ReceivePass);
            prfl2->setSelectedSkill(roleSkill::Kick);
            first = false;
        }
    }

    if(!first){
        if(/*prfl1->getSelectedSkill() == roleSkill::ReceivePass*/wm->ball->pos.y >= 0){
//            rcvr = prfl1;
//            pss = prfl2;
//            if(prfl2->getSelectedSkill()==roleSkill::GotopointAvoid)
//                prfl2->setTarget(Vector2D(xpos2,ypos2));
//            else
//                prfl2->setTarget(Vector2D(xpos1,ypos1));

//            rcvr->setSelectedSkill(roleSkill::ReceivePass);
//            if(wm->ball->vel.length() < 0.3)
//                pss->setSelectedSkill(roleSkill::Kick);
//            else
//                pss->setSelectedSkill(roleSkill::GotopointAvoid);
//            rcvr->setWaitPos(Vector2D(xpos1,ypos1));
//            rcvr->setTarget(Vector2D(xpos1,ypos1));


            if(wm->ball->vel.length() < 0.3)
            {
                prfl2->setSelectedSkill(roleSkill::Kick);
                prfl2->setTarget(Vector2D(xpos1,ypos1));

                rcvr = prfl1;
                pss = prfl2;

                if(!Circle2D(knowledge->getAgent(prfl1->getAgentID())->pos() , 0.1).contains(prfl1->getTarget()))
                    prfl2->setDontKick(true);
                else
                    prfl2->setDontKick(false);
            }
            else
            {
                prfl2->setSelectedSkill(roleSkill::ReceivePass);
                prfl2->setTarget(Vector2D(xpos2,ypos2));
                prfl2->setWaitPos(Vector2D(xpos2,ypos2));

                pss = prfl1;
                rcvr = prfl2;
            }

            prfl1->setSelectedSkill(roleSkill::ReceivePass);
            prfl1->setTarget(Vector2D(xpos1,ypos1));
            prfl1->setWaitPos(Vector2D(xpos1,ypos1));


        }
        else{
//            rcvr = prfl2;
//            pss = prfl1;

//            if(prfl1->getSelectedSkill()==roleSkill::GotopointAvoid)
//                prfl1->setTarget(Vector2D(xpos1,ypos1));
//            else
//                prfl1->setTarget(Vector2D(xpos2,ypos2));

//            rcvr->setSelectedSkill(roleSkill::ReceivePass);
//            if(wm->ball->vel.length() < 0.3)
//                pss->setSelectedSkill(roleSkill::Kick);
//            else
//                pss->setSelectedSkill(roleSkill::GotopointAvoid);

//            rcvr->setWaitPos(Vector2D(xpos2,ypos2));
//            rcvr->setTarget(Vector2D(xpos2,ypos2));




            if(wm->ball->vel.length() < 0.3)
            {
                prfl1->setSelectedSkill(roleSkill::Kick);
                prfl1->setTarget(Vector2D(xpos2,ypos2));

                rcvr = prfl2;
                pss = prfl1;

                if(!Circle2D(knowledge->getAgent(prfl2->getAgentID())->pos() , 0.1).contains(prfl2->getTarget()))
                    prfl1->setDontKick(true);
                else
                    prfl1->setDontKick(false);
            }
            else
            {
                prfl1->setSelectedSkill(roleSkill::ReceivePass);

                rcvr = prfl1;
                pss = prfl2;

                prfl1->setTarget(Vector2D(xpos1,ypos1));
                prfl1->setWaitPos(Vector2D(xpos1,ypos1));
            }

            prfl2->setSelectedSkill(roleSkill::ReceivePass);
            prfl2->setTarget(Vector2D(xpos2,ypos2));
            prfl2->setWaitPos(Vector2D(xpos2,ypos2));
        }

        if(prfl1->getSelectedSkill()==roleSkill::GotopointAvoid)
            debug("chetor momkene!!!",D_MHMMD);
        draw(Circle2D( knowledge->getAgent(pss->getAgentID())->pos() , 0.25 ) , QColor(Qt::blue));
        draw(Circle2D( knowledge->getAgent(pss->getAgentID())->pos() , 0.5 ) , QColor(Qt::black));

        //////////////////// kicker wait until receiver is in it's right position

//        if( rcvr->getSelectedSkill()==roleSkill::ReceivePass && !Circle2D(rcvr->getTarget() , 0.07).contains(knowledge->getAgent(rcvr->getAgentID())->pos()) )
//            pss->setDontKick(true);
//        else
//            pss->setDontKick(false);

//        if(wm->ball->vel.length() > 0.2)
//        {
//            pss->setKickSpeed(0);
//            rcvr->setKickSpeed(0);
//        }
//        else
//        {
//            pss->setKickSpeed(kickSpeed);
//            rcvr->setKickSpeed(kickSpeed);
//        }

        //////////////////// check if robot kicked the ball
        if ( Circle2D( knowledge->getAgent(pss->getAgentID())->pos() , 0.25 ).contains(wm->ball->pos) ){
            ballIsNear = true;
        }

        //////////////////// changing roles
        if(ballIsNear)
            if ( !Circle2D( knowledge->getAgent(pss->getAgentID())->pos() , 0.5 ).contains(wm->ball->pos))
            {

                debug(QString("kick speed : %1").arg(kickSpeed) , D_MAHI);

//                if(wm->ball->vel.length() > 0.2)
//                    rcvr->setSelectedSkill(roleSkill::ReceivePass);
//                else

//                    rcvr->setSelectedSkill(roleSkill::Kick);
////                if(!Circle2D(pss->getTarget() , 0.2).contains(knowledge->getAgent(pss->getAgentID())->pos()))
////                    pss->setSelectedSkill(roleSkill::Gotopoint);
////                else
//                pss->setSelectedSkill(roleSkill::ReceivePass);

                if(kickSpeed < MaxSpeed){
                    //                xpos1 -= kickSpeed/13;
                    //                ypos1 += kickSpeed/13;

                    kickSpeed+=50;
                }
                else{
                    kickSpeed = 200;
                    //                if(p2 != 5){
                    //                    p1 += 2;
                    //                    p2 += 2;
                    //                }
                }

                ballIsNear = false;
            }
    }
    return;





    //chip hit the ground


    /////////////////////////
    //////// not working fine
    /////////////////////////

    static QList<Vector2D> pos , chipPos;
    static Vector2D v1 , v2;
    static int mini=0;
    static AngleDeg minAng = 180.0;
    const int maax=20;


    //    static CSkillKick* test4 = new CSkillKick(knowledge->getAgent(4));
    //    test4->setTarget(wm->ball->pos);
    //    test4->setChip(true);
    //    test4->setAutoChipSpeed(true);
    //    test4->execute();


    pos.append(wm->ball->pos);

    if(chipPos.size()==0)
        chipPos.append(pos.at(0));

    if(pos.size() > maax){

        pos.pop_front();

        v1 = pos.at(0) - pos.at(maax/2);
        v2 = pos.at(maax-1) - pos.at(maax/2);

        draw(Segment2D(pos.at(0) , pos.at(maax/2)) , QColor(Qt::red));
        draw(Segment2D(pos.at(maax-1) , pos.at(maax/2)) , QColor(Qt::red));
        draw(QString("min angle : %1 , v1 v2 : %2").arg(minAng.degree()).arg(v2.angleWith(v1).degree()) , Vector2D(2,2));

        if(v1.length() > 0.2 && v2.length() > 0.2){

            if(v2.angleWith(v1).radian() < minAng.radian()){
                minAng=v1.angleWith(v2);
                mini=maax/2;
            }

            else if((v1.angleWith(v2) /*- minAng*/).degree() > 10  &&  mini != 0){

                if(1||fabs(chipPos.last().y - pos.at(mini).y) > 0.01 || fabs(chipPos.last().x - pos.at(mini).x) > 0.01){
                    chipPos.append(pos.at(mini));
                    debug(QString("Saved Posision[%4] : %1 , %2 , min angle : %3").arg(pos.at(mini).x).arg(pos.at(mini).y).arg(minAng.radian()).arg(chipPos.size()) , D_MAHI);
                    minAng = 180.0;
                }

            }

        }

    }

    Q_FOREACH(Vector2D v , pos)
        draw(v , 0 , QColor(Qt::blue) , 1);

    Q_FOREACH(Vector2D cp , chipPos)
        draw(cp , 0, QColor(Qt::black));

    return;







    /*

      //////////////////////////////
      //////// not tested compeletly
      //////////////////////////////

    static CSkillKick* test4 = new CSkillKick(knowledge->getAgent(3));
    test4->setTarget(mousePos);
    test4->setKickSpeed(6);
    test4->setChip(true);
    test4->execute();

    static AngleDeg ang=0;
    static Vector2D chipPos[1000000];
    static int ii = 0;

    //    if(ii < 10){

    //        if( ang.sin() * wm->ball->vel.th().sin() < 0
    //            && fabs( wm->ball->vel.th().sin() ) > 0.1){
    //            ang = wm->ball->vel.th().sin();
    //            chipPos[ii] = wm->ball->pos;
    //            ii++;

    //            qDebug()<<"ball pos : "<<chipPos[ii].x<<" , "<<chipPos[ii].y<<"      , Angle :  " << ang.radian();
    //            draw(chipPos[ii] , 0 , QColor(Qt::red));
    //        }
    //        else
    //            ang = wm->ball->vel.th().sin();

    //        qDebug()<<"ii : "<<ii;

    //    }

    static double velY=0;

    if(velY * wm->ball->vel.y < 0 && ( wm->ball->vel.y  > 0.0099 || wm->ball->vel.y  < -0.0099 ) ){
        velY = wm->ball->vel.y;
        chipPos[ii] = wm->ball->pos;
        ii++;

        qDebug()<<"ball pos "<<ii<<" : "<<chipPos[ii].x<<" , "<<chipPos[ii].y<<"    , velY :"<<velY;
    }
    else
        velY = wm->ball->vel.y;

    //qDebug()<<"velY : "<<wm->ball->vel.y;

    return;
*/


    /*

    //ball max velocity

    ////////////////////////
    //////// working fine :)
    ////////////////////////

    //    CskillNewGotoPoint *test1 = new CskillNewGotoPoint(soccer->agents[4]);
    //    test1->init(knowledge->getMousePos(),Vector2D(0,0));
    //    test1->execute();
    //    test1->setMaxAcceleration(4);
    //    test1->setMaxDeceleration(-3);
    //    test1->setMaxVelocity(4);
    //    test1->setSlowMode(false);
    //    test1->execute();

    static CSkillKick* test3 = new CSkillKick(knowledge->getAgent(4));
    test3->setTarget(mousePos);
    test3->setKickSpeed(6);
    test3->setChip(false);
    test3->execute();

    static double maxBallVel , avg1=0, avg2=0;
    static int cnt = 1;

    if(avg2 >= avg1){
        cnt++;
        avg1 = avg2;
        avg2 += (avg1*(cnt-1)+wm->ball->vel.length())/cnt;

        qDebug()<<"ball max vel :"<<maxBallVel;

        if(maxBallVel <  wm->ball->vel.length())
            maxBallVel =  wm->ball->vel.length();
    }

    if(wm->ball->vel.length() < 0.2){
        maxBallVel = 0;
        avg2=0;
        avg1=0;
    }


    */

    return;






    static CLoadPlayOffJson loader(QDir::currentPath() + QString("/playoff"));
    loader.setAutoUpdate(true);

    return;
    C tempo;
    tempo.loc  = wm->ball->pos;
    tempo.time = knowledge->getCurrentKKTime();
    history.append(tempo);
    if(history.size() > VEL_SAMPLES) {
        history.pop_front();
    }

    Vector2D mahiVel = fitVelocity();

    if(mahiVel.length()*100 > .1) {
        debug(QString("History : %1").arg(mahiVel.length()*100),D_MAHI);

        graph2.append(mahiVel.length()*100);
        if(graph2.size() > 30) {
            graph2.pop_front();
            debug("HEY", D_MAHI);
        }
    }

    double aaaa = knowledge->getRealBallVel();

    if(aaaa > 0.1) {
        graph.append(aaaa);
        if(graph.size() > 30)
            graph.pop_front();
    }

    drawGraph(graph, QColor(Qt::cyan));
    drawGraph(graph2, QColor(Qt::red));



    return;

    int AHZcounter = 0;
    int AHZcounterTest = 0;
    QList<CAgent*> agents;
    int i = 0;
    static bool ahz = true;
    static QList<CSkillGotoPointAvoid*> AHZGotoPointAvoid;
    //CSkillGotoPointAvoid *gpa = new CSkillGotoPointAvoid();
    // gpa->setTarget(Vector2D(0,0), Vector2D(0,1));
    // gpa->execute();

    agents.clear();
    for(size_t i = 0;i < 16;i++) {
        if(knowledge->getAgent(i)->isVisible()) {
            agents.append(knowledge->getAgent(i));
        }
    }

    if(ahz) {
        AHZGotoPointAvoid.clear();
        for(size_t i = 0;i < agents.size();i++) {
            CSkillGotoPointAvoid* temp = new CSkillGotoPointAvoid(agents.at(i));
            AHZGotoPointAvoid.append(temp);
        }
        ahz = false;
    }

    AHZcounterTest = AHZGotoPointAvoid.size();
    for(size_t i = 0;i < AHZcounterTest;i++) {
        AHZGotoPointAvoid[i]->setTarget(Vector2D(-0.1,(-_FIELD_HEIGHT / (2 * AHZcounterTest)) - (i * _FIELD_HEIGHT /(2 * AHZcounterTest)) + 1.5),Vector2D(-1,0));
        AHZGotoPointAvoid[i]->execute();

    }

    return;
    //TEST SVN
    char x = 'c';
    double b = 1.1;
    double a = 2;

    return;
    CNewProfiler mahiiii;
    mahiiii.insertRecord(PKICK,100,2,4);
    mahiiii.save(JSON);
    static double maxSpeed = 0;
    static Vector2D max(0,0);
    if(wm->ball->vel.length()/2 > maxSpeed)
    {
        maxSpeed = wm->ball->vel.length()/2;
        max = wm->ball->pos;
    }

    draw(QString("max: %1").arg(maxSpeed),Vector2D(0,1));
    draw(max);
    return;


    CNewProfiler *mahi0 = new CNewProfiler();
    mahi0->mahiRecord(0,1,PKICK);


    return;
    CNewProfiler mahi;
    static CNewProfiler mahi2;
    static bool f = true;

    /*
mahi.robotsProfile[0].kickMap.insert(50,0.2);
mahi.robotsProfile[0].kickMap.insert(100,0.5);
mahi.robotsProfile[0].kickMap.insert(150,0.6);
mahi.robotsProfile[0].kickMap.insert(200,0.8);
mahi.robotsProfile[0].kickMap.insert(250,1.101);
mahi.robotsProfile[0].kickMap.insert(300,1.2);
mahi.robotsProfile[0].kickMap.insert(350,1.5);
mahi.robotsProfile[0].kickMap.insert(400,4.6);
mahi.robotsProfile[0].kickMap.insert(450,4.8);
mahi.robotsProfile[0].kickMap.insert(500,5.2);
mahi.robotsProfile[0].kickMap.insert(550,5.5);
mahi.robotsProfile[0].kickMap.insert(600,5.87);
mahi.robotsProfile[0].kickMap.insert(650,6.02);
mahi.robotsProfile[0].kickMap.insert(700,6.33);
mahi.robotsProfile[0].kickMap.insert(750,6.5);
mahi.robotsProfile[0].kickMap.insert(800,6.6);
mahi.robotsProfile[0].kickMap.insert(850,7);
mahi.robotsProfile[0].kickMap.insert(900,7.4);
mahi.robotsProfile[0].kickMap.insert(950,7.9);
mahi.robotsProfile[0].kickMap.insert(1000,8.0);
mahi.robotsProfile[0].kickMap.insert(1023,8.4);
mahi.robotsProfile[0].kickMap.insert(55,0.3);
mahi.robotsProfile[0].kickMap.insert(105,0.6);
mahi.robotsProfile[0].kickMap.insert(155,0.7);
mahi.robotsProfile[0].kickMap.insert(205,0.9);
mahi.robotsProfile[0].kickMap.insert(255,1.11);
mahi.robotsProfile[0].kickMap.insert(305,1.3);
mahi.robotsProfile[0].kickMap.insert(355,1.6);
mahi.robotsProfile[0].kickMap.insert(405,4.7);
mahi.robotsProfile[0].kickMap.insert(455,4.9);
mahi.robotsProfile[0].kickMap.insert(505,5.3);
mahi.robotsProfile[0].kickMap.insert(555,5.6);
mahi.robotsProfile[0].kickMap.insert(605,5.97);
mahi.robotsProfile[0].kickMap.insert(655,6.12);
mahi.robotsProfile[0].kickMap.insert(705,6.43);
mahi.robotsProfile[0].kickMap.insert(755,6.6);
mahi.robotsProfile[0].kickMap.insert(805,6.7);
mahi.robotsProfile[0].kickMap.insert(855,7.1);
mahi.robotsProfile[0].kickMap.insert(905,7.5);
mahi.robotsProfile[0].kickMap.insert(955,7.95);
mahi.robotsProfile[0].kickMap.insert(1005,8.1);


if(f) {
    mahi.save(JSON);
    f = false;
    debug(QString("mahi"),D_MAHI);
    mahi2.load(JSON);
}
*/

    mahi.robotsProfile[0].drawProfile();

    //    QMap<int,double> Mahimap = mahi2.robotsProfile[0].kickMap;
    //    mahi.robotsProfile[0].fillArray(mahi.robotsProfile[0].kickMap,mahi.robotsProfile[0].kickArr,true);
    //    mahi.robotsProfile[0].sortPairArrByValue(mahi.robotsProfile[0].kickArr,0,KICK_ARRAY_SIZE);
    //    debug(QString("%2  : %1").arg(mahi.robotsProfile[0].kickArr[0].second).arg(mahi.robotsProfile[0].kickArr[0].first),D_MAHI);
    //    debug(QString("%2 : %1").arg(mahi.robotsProfile[0].kickArr[1].second).arg(mahi.robotsProfile[0].kickArr[1].first),D_MAHI);
    //    debug(QString("%2  : %1").arg(mahi.robotsProfile[0].kickArr[2].second).arg(mahi.robotsProfile[0].kickArr[2].first),D_MAHI);
    //    debug(QString("%2 : %1").arg(mahi.robotsProfile[0].kickArr[3].second).arg(mahi.robotsProfile[0].kickArr[3].first),D_MAHI);
    //    debug(QString("150 : %1").arg(mahi.robotsProfile[0].kickArr[2].second),D_MAHI);
    //    debug(QString("200 : %1").arg(mahi.robotsProfile[0].kickArr[3].second),D_MAHI);
    //    debug(QString("250 : %1").arg(mahi.robotsProfile[0].kickArr[4].second),D_MAHI);
    //    debug(QString("300 : %1").arg(mahi.robotsProfile[0].kickArr[5].second),D_MAHI);
    //    debug(QString("350 : %1").arg(mahi.robotsProfile[0].kickArr[6].second),D_MAHI);
    //    debug(QString("400 : %1").arg(mahi.robotsProfile[0].kickArr[20].second),D_MAHI);
    //    debug(QString("450 : %1").arg(mahi.robotsProfile[0].kickArr[21].second),D_MAHI);
    //    debug(QString("500 : %1").arg(mahi.robotsProfile[0].kickArr[22].second),D_MAHI);
    //    debug(QString("550 : %1").arg(mahi.robotsProfile[0].kickArr[23].second),D_MAHI);
    //    debug(QString("600 : %1").arg(mahi.robotsProfile[0].kickArr[24].second),D_MAHI);

    //    debug(QString("%1 : %2").arg(mahi.robotsProfile[0].kickMap.value(kic)),D_MAHI);
    debug(QString("%1 : %2").arg(mahi.robotsProfile[0].getKickSpeed(0.1)).arg(0.1),D_MAHI);
    debug(QString("%1 : %2").arg(mahi.robotsProfile[0].getKickSpeed(0.25)).arg(0.25),D_MAHI);
    debug(QString("%1 : %2").arg(mahi.robotsProfile[0].getKickSpeed(0.65)).arg(0.65),D_MAHI);
    debug(QString("%1 : %2").arg(mahi.robotsProfile[0].getKickSpeed(0.95)).arg(0.95),D_MAHI);
    debug(QString("%1 : %2").arg(mahi.robotsProfile[0].getKickSpeed(1.15)).arg(1.15),D_MAHI);
    debug(QString("%1 : %2").arg(mahi.robotsProfile[0].getKickSpeed(1.55)).arg(1.55),D_MAHI);

    return;
    CskillNewGotoPoint *gotopointSkill = new CskillNewGotoPoint(soccer->agents[1]);

    return;

    CSkillTurn *turnSkill = new CSkillTurn(soccer->agents[1]);
    turnSkill->setDirection(Vector2D(0,1));
    turnSkill->setTurnMode(CSkillTurn::Fast);

    draw(QString("dir = %1").arg(soccer->agents[1]->dir().dir().degree()),Vector2D(1,1));
    turnSkill->execute();
    // soccer->agents[1]->setRobotVel(-1,0,10);





    return;
    double tempSpeed;
    SRAgentArgs temp;
    for(int i = 0; i <_MAX_NUM_PLAYERS; i++)
    {
        if(knowledge->SRGetAgentArg(i, temp))
        {
            soccer->agents[i]->setRobotVel(temp.Vx, temp.Vy, temp.Vr);
            if(temp.KickSpeed != 0)
            {
                tempSpeed = int(temp.KickSpeed*(1023/8));
                if(tempSpeed > 1023) tempSpeed = 1023;
                soccer->agents[i]->setKick(tempSpeed);
            }
            if(temp.ChipSpeed != 0)
            {
                tempSpeed = int(temp.ChipSpeed*(1023/8));
                if(tempSpeed > 1023) tempSpeed = 1023;
                soccer->agents[i]->setChip(tempSpeed);
            }
            if(temp.KickSpeed != 0)
            {
                tempSpeed = int(temp.SpinSpeed*(1023/8));
                if(tempSpeed > 1023) tempSpeed = 1023;
                soccer->agents[i]->setRoller(tempSpeed);
            }
        }
    }

    return;
    CSkillKick *ttt= new CSkillKick(soccer->agents[5]);
    ttt->setTarget(wm->field->ourGoal());
    ttt->setKickSpeed(1000);
    ttt->setSlow(false);
    ttt->execute();
    return;
    CskillNewGotoPoint *test = new CskillNewGotoPoint(soccer->agents[4]);
    test->init(knowledge->getMousePos(),Vector2D(0,0));
    test->execute();
    test->setMaxAcceleration(4);
    test->setMaxDeceleration(-3);
    test->setMaxVelocity(4);
    test->setSlowMode(false);
    test->execute();
    return;
#ifndef GAME_MODE
    //    static CSkillKick* kick = new CSkillKick(soccer->agents[0]);
    //    static CSkillKickOneTouch* onetouch = new CSkillKickOneTouch(soccer->agents[0]);
    //    static int active = -1;
    //    static int cntr = 50;
    //    static int mode = 1;
    //    if (knowledge->joystick->getButton11() && cntr > 50)
    //    {
    //        if (kick->getAgent() != NULL)
    //            kick->getAgent()->self()->tracker->reset();
    //    }
    //    if (knowledge->joystick->getButton8() && cntr > 50)
    //    {
    //        mode *= -1;
    //        cntr = 0;
    //    }
    //    if ( knowledge->joystick->getButton6() && cntr > 50)
    //    {
    //        active *= -1;
    //        cntr = 0;
    //    }
    //    cntr ++;
    //    if ( knowledge->joystick->getButton7())
    //        kick->setAgent(soccer->agents[0]);
    //    if ( knowledge->joystick->getButton1())
    //        kick->setAgent(soccer->agents[1]);
    //    if ( knowledge->joystick->getButton2())
    //        kick->setAgent( soccer->agents[2]);
    //    if ( knowledge->joystick->getButton4())
    //        kick->setAgent( soccer->agents[4]);
    //    if ( knowledge->joystick->getButton3())
    //        kick->setAgent( soccer->agents[3]);
    //    if ( knowledge->joystick->getButton5())
    //        kick->setAgent( soccer->agents[5]);

    //    if (mode == 1)
    //    {
    //        draw(QString( " kick "), Vector2D(0,2),"yellow");
    //        kick->setTarget( wm->field->oppGoal());
    //        kick->setKickSpeed(9);
    //        kick->setThroughMode(false);
    //        kick->setSlow(false);
    //        kick->setInterceptMode(true);
    //        kick->setParallelMode(false);
    //        kick->setWaitFrames( 1);
    //        kick->setTolerance( 0.1);
    //        kick->setDontKick(false);
    //        kick->setTarget(wm->field->oppGoal());
    //        kick->setSpin(0);
    //        kick->setChip(false);
    //        if ( active == 1)
    //            kick->execute();
    //        else
    //            kick->getAgent()->waitHere();
    //    }
    //    else {
    //        draw(QString( " one "), Vector2D(0,2),"orange");
    //        onetouch->setAgent(kick->getAgent());
    //        onetouch->setTarget( wm->field->oppGoal());
    //        onetouch->setKickSpeed(31);
    //        onetouch->setTarget(wm->field->oppGoal());
    //        onetouch->setChip(false);
    //        if ( active == 1)
    //            onetouch->execute();
    //        else
    //            onetouch->getAgent()->waitHere();

    //    }
    static CSkillGotoPointAvoid* gp0 = new CSkillGotoPointAvoid( soccer->agents[0]);
    static CSkillGotoPointAvoid* gp1= new CSkillGotoPointAvoid( soccer->agents[1]);
    static CSkillGotoPointAvoid* gp4 = new CSkillGotoPointAvoid( soccer->agents[5]);
    static CSkillTurn* turn0 = new CSkillTurn( soccer->agents[0]);
    static CSkillTurn* turn1 = new CSkillTurn( soccer->agents[1]);
    static CSkillTurn* turn4 = new CSkillTurn( soccer->agents[5]);

    static int mode = 0;
    if ( knowledge->joystick->getButton6())
        mode = 1;
    if ( knowledge->joystick->getButton8())
        mode = 0;
    if ( mode )
    {
        gp0->setTargetLook(Vector2D(-1.5,1), Vector2D(0,1));
        gp1->setTargetLook(Vector2D(-1.5,0), Vector2D(0,1));
        gp4->setTargetLook(Vector2D(-1.5,-1), Vector2D(0,1));
    }
    else
    {
        gp0->setTargetLook(Vector2D(1.5,1), Vector2D(0,1));
        gp1->setTargetLook(Vector2D(1.5,0), Vector2D(0,1));
        gp4->setTargetLook(Vector2D(1.5,-1), Vector2D(0,1));
    }

    gp0->execute();
    gp1->execute();
    gp4->execute();

    turn0->setAgent(soccer->agents[0]);
    turn0->setDirection(Vector2D(0,1));
    turn0->setTurnMode(CSkillTurn::Intercept);
    turn0->execute();

    turn1->setAgent(soccer->agents[1]);
    turn1->setDirection(Vector2D(0,1));
    turn1->setTurnMode(CSkillTurn::Intercept);
    turn1->execute();

    turn4->setAgent(soccer->agents[5]);
    turn4->setDirection(Vector2D(0,1));
    turn4->setTurnMode(CSkillTurn::Intercept);
    turn4->execute();


#endif
}

#endif // EXPERIMENTAL4_H


double CMainApplication::maxBallSpeed(){

    //    static CSkillKick* test3 = new CSkillKick(knowledge->getAgent(agentID));
    //    test3->setTarget(mousePos);
    //    test3->setKickSpeed(6);
    //    test3->setChip(false);
    //    test3->execute();

    static double maxBallVel , avg1=0, avg2=0;
    static int cnt = 1;

    if(avg2 >= avg1){
        cnt++;
        avg1 = avg2;
        avg2 += (avg1*(cnt-1)+wm->ball->vel.length())/cnt;

        if(maxBallVel <  wm->ball->vel.length())
            maxBallVel =  wm->ball->vel.length();
    }
    else

        return maxBallVel;
    //    if(wm->ball->vel.length() < 0.2){
    //        maxBallVel = 0;
    //        avg2=0;
    //        avg1=0;
    //    }

    return 0;

}
