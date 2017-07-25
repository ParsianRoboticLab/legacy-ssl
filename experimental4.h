#ifndef EXPERIMENTAL4_H
#define EXPERIMENTAL4_H
#include <mainapplication.h>

#include <QTime>

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

static CRolePlayOn * pss , * rcvr;
static CRolePlayOn * prfl1 = new CRolePlayOn() ,  * prfl2 = new CRolePlayOn();
static int p2 = 1 , p1 = 5 , repeat = 3;
static QList<double> RealSpeedRec;
static double kickSpeed = 600 , MaxSpeed = 1023 , MinSpeed = 400 , speedStep = 200 , xpos2 = -2.8 , ypos2 = 2 , xpos1 , ypos1;

struct BallDistVelo {
    double vel, dist;
};

static int fcount = 2;
QList<BallDistVelo> ballhist;
static bool kickSpeedCalculated = false;

void calckickspeed(){
    double kickspeed;
    Vector2D kickStartPoint;

    if (kickSpeedCalculated) return;
    double vball = wm->ball->vel.length();

    if (vball > 0.2)
    {
        if (ballhist.count() == 0)
        {
            kickStartPoint = wm->ball->pos;
        }
        BallDistVelo b;
        b.dist = (wm->ball->pos - kickStartPoint).length();
        b.vel  = vball;
        ballhist.append(b);
    }


    //    double s = (wm->ball->pos - kickPoint).innerProduct(wm->ball->pos - target);

    if (vball < 0.2 /*|| s > 0*/)
    {
        if (ballhist.count() > 10)
        {
            for (int i=0;i<ballhist.count()*3/4;i++)
            {
                double v2s = 0.0;
                double vs = 0.0;
                for (int j=i+1;j<ballhist.count();j++)
                {
                    double v = sqrt (ballhist[j].vel*ballhist[j].vel + 2.0 * ballhist[j].dist * BallFriction() * Gravity);
                    v2s += v*v;
                    vs  += v;
                }
                double n = ballhist.count() - (i + 1);
                double e = (vs / n);
                double s = v2s / n - e * e;
                kickspeed = e;
                //                    RealSpeedRec.append(kicks);
                if (s < 0.05)
                {
                    break;
                }
            }
            kickSpeedCalculated = true;
            fcount++;
            debug(QString("Robot%4 : Real Speed: %1 , kickSpeedf: %3 ; n = %2").arg(kickspeed).arg(ballhist.count()).arg(kickSpeed).arg(p1), D_FATEMEH);
            ballhist.clear();
        }
    }
    draw(kickStartPoint, 1, "red");

}

void calcChipPos(QList<Vector2D> ballposss){
    QList<Vector2D> save;
    Vector2D v12 = ballposss.at(0);
    double tan=0;

    for(int i=0; i<ballposss.size() - 10; i+=2){
        tan = v12.th().tan();
        v12 = ballposss.at(i) - ballposss.at(i+5);

        if(save.size() < 7)
        {
            if(tan*v12.th().tan() < 0 && fabs(tan - v12.th().tan()) > 0.6){
                save.append(ballposss.at(i));
                debug(QString("saved %4 : x : %1 , y : %2 | speed : %3").arg(ballposss.at(i).x).arg(ballposss.at(i).y).arg(kickSpeed).arg(save.size()) , D_FATEMEH);
            }
            if(save.size() > 2){
                if(save.at(0).dist(save.at(2))< 0.9){
                    kickSpeedCalculated = true;
                    debug(QString("invalid data : dist 0-2 : %1 , speed : %2").arg(save.at(0).dist(save.at(2))).arg(kickSpeed) , D_FATEMEH , QColor(Qt::darkCyan));
                    return;
                }
            }
        }
        else
        {

            draw(Segment2D(save.at(0) , save.at(2)) , QColor(Qt::darkMagenta));
            debug(QString("dist 0-2 : %1 , speed : %2").arg(save.at(0).dist(save.at(2))).arg(kickSpeed) , D_FATEMEH , QColor(Qt::blue));
            kickSpeedCalculated = true;
        }
    }

    Q_FOREACH(Vector2D v , save)
        draw(v , 0 , QColor(Qt::darkRed) , 1);

    save.clear();
    return;

}


QList<Vector2D> ballposss;

void CMainApplication::Experimental4(){

    static bool fff=true;

    static QFile file;
    file.setFileName("regression_data");
    static QTextStream out;
    out.setDevice(&file);


    const int /*robotID1 = 4,*/ robotID2 = 6;


    QMap<int, double> kick1, kick2, chip1;

    kick2 = knowledge->profiler->robotsProfile[robotID2].finalKickMap;

    if(fff){
        file.open(/*QIODevice::Truncate | */QIODevice::WriteOnly);

        for(int robotID1 = 7; robotID1 < 8; robotID1++){

            kick1 = knowledge->profiler->robotsProfile[robotID1].finalKickMap;
            chip1 = knowledge->profiler->robotsProfile[robotID1].finalChipMap;

            out << "Final kick data collected for robot " << robotID1 << "\n";
            for(int i=0; i<kick1.keys().size();i++){
                out << QString("%1   ,   %2\n").arg(kick1.keys().at(i)).arg(kick1.values().at(i));
            }
            out << "\nRegression on kick data for robot " << robotID1 << "\n";
            for(int i=0; i<81; i++){
                out << QString("%1 , %2\n").arg(i/10.0).arg(knowledge->getProfile(robotID1, i/10.0, true, false));
            }


            out << "\nFinal chip data collected for robot " << robotID1 << "\n";
            for(int i=0; i<chip1.keys().size(); i++){
                out << QString("%1   ,   %2\n").arg(chip1.keys().at(i)).arg(chip1.values().at(i)) ;
            }
            out << "\nRegression on chip data for robot " << robotID1 << "\n";
            for(int i=0; i<40; i++){
                out << QString("%1 , %2\n").arg(i).arg(knowledge->getProfile(robotID1, i/10.0, false, false));
            }
        }

//        out << "\nFinal kick data collected for robot " << robotID2 << "\n";
//        for(int i=0; i<kick2.keys().size();i++){
//            out << QString("%1   ,   %2\n").arg(kick2.keys().at(i)).arg(kick2.values().at(i));
//        }
//        out << "\nRegression on kick data for robot " << robotID2 << "\n";
//        for(int i=0; i<81; i++){
//            out << QString("%1 , %2\n").arg(i/10.0).arg(knowledge->getProfile(robotID2, i/10.0, true, false));
//        }

        file.close();
        fff = false;
    }

    return;

//    static double moshtagh2=0;
//    analyze("kickspeed" , wm->ball->vel.length(),true);
//    debug(QString("ball Speed :%1").arg(wm->ball->vel.length()),D_NADIA);
//    analyze("moshtagh2",(wm->ball->vel.length()-moshtagh2)*1000,true);
//    debug(QString("moshtagh2 :%1 _ r2:%2").arg(moshtagh2).arg((wm->ball->vel.length()-moshtagh2)*1000),D_NADIA);
//    moshtagh2=wm->ball->vel.length();


    int kicking =0;

    prfl1->setAgent(knowledge->getAgent(p1));
    prfl1->setAgentID(p1);
    prfl1->setReceiveRadius(1.2);
    prfl1->setTolerance(0.01);
    prfl1->setIsActive(true);
    prfl1->setChip(true);
    prfl1->setSlow(true);


    if(ballposss.size()==0)
        ballposss.append(wm->ball->pos);

    if(fabs(wm->ball->pos.x - ballposss.last().x) > 0.003 && fabs(wm->ball->pos.y - ballposss.last().y) > 0.003)
        ballposss.append(wm->ball->pos);

    if(ballposss.size() > 100)
        ballposss.removeFirst();

    if(kicking == 0){

//        debug("first go to point " , D_FATEMEH);
        prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
        prfl1->setTarget(Vector2D(-_FIELD_WIDTH/2+0.4 , -_FIELD_HEIGHT/2+0.4));
        prfl1->execute();

        if ( Circle2D(knowledge->getAgent(p1)->pos(), 0.5).contains(wm->ball->pos)  &&  wm->ball->vel.length() < 0.2 )
        {
            kicking = 1;
        }

    }
    if(kicking == 1){

        debug("kicking" , D_FATEMEH);
        prfl1->setSelectedSkill(roleSkill::Kick);
        prfl1->setKickSpeed(kickSpeed);

        prfl1->setTarget(Vector2D(-_FIELD_WIDTH/2+0.8 , _FIELD_HEIGHT/2-0.8));

        prfl1->execute();

        if(Circle2D(knowledge->getAgent(p1)->pos() , 0.08).contains(wm->ball->pos)){
            ballposss.clear();
            debug("cleared" , D_FATEMEH);
            ballposss.append(wm->ball->pos);
        }

        if(wm->ball->vel.length() > 2.3){
            debug("leaving kick state" , D_FATEMEH);
            kicking = 2;
            kickSpeedCalculated = false;
        }

    }
    if(kicking == 2){
        debug("second go to point " , D_FATEMEH);
        prfl1->setSelectedSkill(roleSkill::GotopointAvoid);
        prfl1->setTarget(Vector2D(-_FIELD_WIDTH/2+0.4 , -_FIELD_HEIGHT/2+0.4));
        prfl1->execute();

        if(ballposss.size() > 80){
            debug("call func" , D_FATEMEH);
            calcChipPos(ballposss);
        }

        if (kickSpeedCalculated)
        {
            kicking = 0;
        }

    }


    Q_FOREACH(Vector2D v , ballposss)
        draw(v , 0 , QColor(Qt::white) , 1);



    return;




//        static QList<Vector2D> save;
//    //    static Vector2D v12;
//    //    static double tan=0;

//    if(ballposss.size()==0){
//        ballposss.append(wm->ball->pos);
//        save.append(wm->ball->pos);
//    }

//    if(fabs(wm->ball->pos.x - ballposss.last().x) > 0.003 && fabs(wm->ball->pos.y - ballposss.last().y) > 0.003)
//        ballposss.append(wm->ball->pos);


//    if(ballposss.size() > 100){
//        ballposss.removeFirst();

//        for(int i=0; i<ballposss.size() - 10; i+=2){
//            tan = v12.th().tan();
//            v12 = ballposss.at(i) - ballposss.at(i+5);
//            draw(Segment2D(ballposss.at(i) , ballposss.at(i+5)) , QColor(Qt::white));

//            if(save.size() < 10){
//                if(tan*v12.th().tan() < 0 && fabs(tan - v12.th().tan()) > 0.6){
//                    save.append(ballposss.at(i));
//                    debug(QString("shib1 : %1 , shib2 : %2").arg(tan).arg(v12.th().tan()) , D_FATEMEH);
//                }
//            }
//            else
//                save.clear();
//            //        v23 = ballposss.at(i+5) - ballposss.at(i+10);
//        }
//    }

//    Q_FOREACH(Vector2D v , ballposss)
//        draw(v , 0 , QColor(Qt::darkRed) , 1);

//    Q_FOREACH(Vector2D v , save)
//        draw(v , 0 , QColor(Qt::cyan) , 1);

//    return;

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

    if(pos.size()==0)
        pos.append(pos.at(0));

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






    // chip

    static double rect_width = 1.7;
    static int cnt = 0;
    static QList<Vector2D> ballPos , savePositionUp , savePositionDown , savePositionLeft , savePositionRight;
    static Vector2D ballVel = wm->ball->vel , ballPosition = wm->ball->pos;


    Polygon2D *up = new Polygon2D();
    up->addVertex(Vector2D(-_FIELD_WIDTH/2 , _FIELD_HEIGHT/2));
    up->addVertex(Vector2D(-_FIELD_WIDTH/2 , _FIELD_HEIGHT/2-rect_width));
    up->addVertex(Vector2D(0 , _FIELD_HEIGHT/2-rect_width));
    up->addVertex(Vector2D(0 , _FIELD_HEIGHT/2));

    Polygon2D *down = new Polygon2D();
    down->addVertex(Vector2D(-_FIELD_WIDTH/2 , -_FIELD_HEIGHT/2));
    down->addVertex(Vector2D(-_FIELD_WIDTH/2 , -_FIELD_HEIGHT/2+rect_width));
    down->addVertex(Vector2D(0 , -_FIELD_HEIGHT/2+rect_width));
    down->addVertex(Vector2D(0 , -_FIELD_HEIGHT/2));

    Polygon2D *left = new Polygon2D();
    left->addVertex(Vector2D(-_FIELD_WIDTH/2 , _FIELD_HEIGHT/2));
    left->addVertex(Vector2D(-_FIELD_WIDTH/2+rect_width , _FIELD_HEIGHT/2));
    left->addVertex(Vector2D(-_FIELD_WIDTH/2+rect_width , -_FIELD_HEIGHT/2));
    left->addVertex(Vector2D(-_FIELD_WIDTH/2 , -_FIELD_HEIGHT/2));

    Polygon2D *right = new Polygon2D();
    right->addVertex(Vector2D(0 , _FIELD_HEIGHT/2));
    right->addVertex(Vector2D(-rect_width , _FIELD_HEIGHT/2));
    right->addVertex(Vector2D(-rect_width , -_FIELD_HEIGHT/2));
    right->addVertex(Vector2D(0 , -_FIELD_HEIGHT/2));

    if(savePositionUp.size()==0){
        savePositionUp.append(Vector2D(_FIELD_WIDTH/2   , 0));
        savePositionDown.append(Vector2D(_FIELD_WIDTH/2 , 0));
        savePositionLeft.append(Vector2D(_FIELD_WIDTH/2 , 0));
        savePositionRight.append(Vector2D(_FIELD_WIDTH/2 , 0));
    }

    draw(*up   , QColor(Qt::black));
    draw(*down , QColor(Qt::black));
    draw(*left , QColor(Qt::black));
    draw(*right , QColor(Qt::black));


    foreach(Vector2D b , ballPos)
        draw(b , 0 , QColor(Qt::blue) , 1);


    foreach(Vector2D spu , savePositionUp)
        draw(spu , 0 , QColor(Qt::red) , 1);

    foreach(Vector2D spd , savePositionDown)
        draw(spd , 0 , QColor(Qt::black) , 1);

    foreach(Vector2D spl , savePositionLeft)
        draw(spl , 0 , QColor(Qt::white) , 1);

    foreach(Vector2D spr , savePositionRight)
        draw(spr , 0 , QColor(Qt::darkYellow) , 1);



    ballPos.append(wm->ball->pos);

    if(ballPos.size() == 121)
        ballPos.removeFirst();

    if(fabs(wm->ball->vel.x) > 0.02 && fabs(wm->ball->vel.y) > 0.02){

        if(up->contains(wm->ball->pos)){
            if((/*ballVel.x < 0 &&*/ ballVel.y <= 0) && (/*wm->ball->vel.x < 0 && */wm->ball->vel.y > 0)
                    //                    || (ballVel.x > 0 && ballVel.y <= 0) && (wm->ball->vel.x > 0 && wm->ball->vel.y > 0)
                    ){
                debug("up" , D_FATEMEH);
                debug(QString("curr ballspeed: x: %1 , y: %2").arg(wm->ball->vel.x).arg(wm->ball->vel.y) , D_FATEMEH);
                debug(QString("prev ballspeed: x: %1 , y: %2").arg(ballVel.x).arg(ballVel.y) , D_FATEMEH);
                savePositionUp.append(ballPosition);
            }
        }

        if(down->contains(wm->ball->pos)){
            if((/*ballVel.x > 0 &&*/ ballVel.y >= 0) && (/*wm->ball->vel.x > 0 &&*/ wm->ball->vel.y < 0)
                    //                    || (ballVel.x < 0 && ballVel.y >= 0) && (wm->ball->vel.x < 0 && wm->ball->vel.y < 0)
                    ){
                debug("down" , D_FATEMEH);
                debug(QString("curr ballspeed: x: %1 , y: %2").arg(wm->ball->vel.x).arg(wm->ball->vel.y) , D_FATEMEH);
                debug(QString("prev ballspeed: x: %1 , y: %2").arg(ballVel.x).arg(ballVel.y) , D_FATEMEH);
                savePositionDown.append(ballPosition);
            }
        }

        if(left->contains(wm->ball->pos)){
            if((ballVel.x >= 0 /*&& ballVel.y > 0*/) && (wm->ball->vel.x < 0 /*&& wm->ball->vel.y > 0*/)
                    //                    || (ballVel.x >= 0 && ballVel.y < 0) && (wm->ball->vel.x < 0 && wm->ball->vel.y < 0)
                    ){
                debug("left" , D_FATEMEH);
                debug(QString("curr ballspeed: x: %1 , y: %2").arg(wm->ball->vel.x).arg(wm->ball->vel.y) , D_FATEMEH);
                debug(QString("prev ballspeed: x: %1 , y: %2").arg(ballVel.x).arg(ballVel.y) , D_FATEMEH);
                savePositionLeft.append(ballPosition);
            }
        }

        if(right->contains(wm->ball->pos)){
            if((ballVel.x <= 0 /*&& ballVel.y > 0*/) && (wm->ball->vel.x > 0 /*&& wm->ball->vel.y > 0*/)
                    //                    || (ballVel.x <= 0 && ballVel.y < 0) && (wm->ball->vel.x > 0 && wm->ball->vel.y < 0)
                    ){
                debug("right" , D_FATEMEH);
                debug(QString("curr ballspeed: x: %1 , y: %2").arg(wm->ball->vel.x).arg(wm->ball->vel.y) , D_FATEMEH);
                debug(QString("prev ballspeed: x: %1 , y: %2").arg(ballVel.x).arg(ballVel.y) , D_FATEMEH);
                savePositionRight.append(ballPosition);
            }
        }

    }

    // update
    if(cnt==4){
        ballPosition = wm->ball->pos;
        cnt = 0;
    }
    else
        cnt++;

    ballVel = wm->ball->vel;

    return;





    // chip, nth attempt

    static QList<Vector2D> ballPosi , hit;
    static bool ballIsInStraightLine = false , flag =true;
    static int num = 20 , firstIndex , counter=0 , hitsize=0 , n=7;
    static Vector2D pos1 , pos2;

    if(ballPosi.size()==0)
        ballPosi.append(wm->ball->pos);

    if(fabs(wm->ball->pos.x - ballPosi.last().x) > 0.003 && fabs(wm->ball->pos.y - ballPosi.last().y) > 0.003)
        ballPosi.append(wm->ball->pos);


    if(ballPosi.size() == 122){
        ballPosi.removeFirst();

        for(int i=0 ; i < ballPosi.size()-num-n ; i++){

            for(int j=0; j<n; j++){
                if(( ballPosi.at(i)-ballPosi.at(i+num/2+j) ).angleWith( ballPosi.at(i+num/2+j)-ballPosi.at(i+num+j) ).degree() > 1){
                    flag = false;
                }
            }

            if(flag){
                firstIndex = i;
                pos1 = ballPosi.at(i);
                pos2 = ballPosi.at(firstIndex+num+2);
                counter++;
                //                if(counter==2){
                ballIsInStraightLine = true;
                //                    counter=0;
                break;
                //                }
            }
            flag = true;
        }

    }

    hitsize = hit.size();

    if(ballIsInStraightLine){
        for(int i=15 ; i < firstIndex ; i++){

            if( ( ballPosi.at(i)-ballPosi.at(firstIndex) ).angleWith( ballPosi.at(i)-ballPosi.at(firstIndex+num) ).degree() < 1
                    //                    && ( ballPosi.at(i)-ballPosi.at(firstIndex+1) ).angleWith( ballPosi.at(i)-ballPosi.at(firstIndex+num+1) ).degree() < 0.5
                    //                    && ( ballPosi.at(i)-ballPosi.at(firstIndex+2) ).angleWith( ballPosi.at(i)-ballPosi.at(firstIndex+num+2) ).degree() < 0.5
                    )
            {
                hit.append(ballPosi.at(i));
                //                debug(QString("hit pos : x : %1 , y : %2").arg(ballPosi.at(i).x).arg(ballPosi.at(i).y) , D_FATEMEH);
                debug(QString("saved pos : x %1 , : y : %2").arg(ballPosi.at(i).x).arg(ballPosi.at(i).y) , D_FATEMEH);
                i+=2;
            }

        }

    }

    ballIsInStraightLine = false;
    flag = true;

    //    if(wm->ball->vel.length() < 0.01 && hit.size() > hitsize ){
    //        ballPosi.clear();
    //    }

    draw(pos1 , 1 , QColor(Qt::black) , 1);
    //        draw(pos2 , 1 , QColor(Qt::red) , 1);
    draw(Segment2D(pos1 , pos2 ) ,QColor(Qt::red));

    foreach(Vector2D b , ballPosi)
        draw(b , 0 , QColor(Qt::cyan) , 1);

    foreach(Vector2D h , hit){
        draw(h , 1 , QColor(Qt::darkBlue) , 1);
    }

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
    tempo.time = knowledge->getCurrentTime();
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

/*
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
*/
