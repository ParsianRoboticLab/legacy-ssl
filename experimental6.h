#ifndef EXPERIMENTAL6_H
#define EXPERIMENTAL6_H

#include "mainapplication.h"
#include <joystick.h>
#include <QColor>
#include <callibration.h>
#include <defensepositioning.h>
#include <time.h>
clock_t t;
//#define speedTest

#define kickTest
QList <Vector2D> agentpath;
void CMainApplication::Experimental6()
{
#ifdef speedTest
    int agentNum = 4;
    soccer->agents[agentNum]->setRobotAbsVel(1,0,0);
    draw(QString("speed :%1").arg(soccer->agents[agentNum]->vel().x));
    draw(QString("speed2 :%1").arg(knowledge->mainLoopTime),Vector2D(1,1));
    return;
#endif
    static bool stopFlag = true;
    if(knowledge->joystick->getButton3())
        stopFlag = false;
    if(knowledge->joystick->getButton4())
        stopFlag = true;
    int skillAgent = 4;




#ifdef kickTest
    static CSkillKick mmkick(soccer->agents[skillAgent]);
    mmkick.setTarget(wm->field->oppGoal());
    mmkick.setShotToEmptySpot(false);
    mmkick.setKickSpeed(1000);
    mmkick.setGoalieMode(false);

    if(!stopFlag)
        mmkick.execute();
    return;
#endif
    static CSkillGotoPointAvoid mGP(soccer->agents[skillAgent]);
    if(soccer->agents[skillAgent]->vel().length() < 0.05)
    {
        agentpath.clear();
    }
    else
    {
        agentpath.append(soccer->agents[skillAgent]->pos());
    }

    for(int i = 0 ; i <agentpath.size() ; i++){
        draw(Circle2D(agentpath.at(i), 0.05),QColor(Qt::red));
    }

    mGP.init(mousePos,Vector2D(0,1));
    mGP.setNoAvoid(false);

    mGP.setAvoidPenaltyArea(false);
    mGP.execute();
    draw(mousePos,1,QColor(Qt::black));
    return;

    static CSkillGotoPointAvoid mgpa(soccer->agents[1]);



    for(int i=0;i<28;i++)
    {
        draw(knowledge->getStaticPoses(i));
    }
    //    mgpa.init(wm->ball->pos + (soccer->agents[1]->pos() -wm->ball->pos).norm()*0.11,wm->ball->pos - soccer->agents[1]->pos());
    //    soccer->agents[1]->setRoller(5);
    ////    if(soccer->agents[1]->pos().dist(wm->ball->pos) >= 0.11)
    ////    mgpa.execute();
    ////    else
    //        soccer->agents[1]->setRobotVel(-0.5,0.5,3);
    //    return;
    //        static CSkillGotoPointAvoid mgpa(soccer->agents[1]);
    //        mgpa.init(mousePos,Vector2D(0,0));
    //        mgpa.execute();
    //    soccer->agents[1]->setRoller(6);
    //    if(!stopFlag)
    //    soccer->agents[1]->setRobotVel(-1,0,0);
    //       return;

    return;

    ////ahz
    Segment2D ahzSeg;
    Circle2D ahzcircle(wm->field->oppGoal(),1);
    Vector2D dummy1,dummy2;

    ahzSeg.assign(wm->ball->pos,wm->field->oppGoal());

    ahzcircle.intersection(ahzSeg,&dummy1,&dummy2);

    draw(dummy1);

    ahzcircle.tangent(wm->ball->pos,&dummy1,&dummy2);

    draw(dummy1);
    draw(dummy2);

    draw(ahzSeg,QColor(Qt::red));
    draw(ahzcircle,QColor(Qt::cyan));

    return;
    static CSkillKick nokick(soccer->agents[0]);
    nokick.setTarget(mousePos);

    nokick.setChip(true);
    nokick.setVeryFine(true);
    if(soccer->agents[0]->pos().dist(mousePos) > 0.5)
        nokick.setKickSpeed(2);
    else
        nokick.setKickSpeed(0);
    nokick.setPassProfiler(true);

    //if(stopFlag == false)
    nokick.execute();

    return;
    static CSkillGotoPointAvoid mymark(soccer->agents[3]);
    static CSkillKickOneTouch mkick(soccer->agents[0]);
    mkick.setTarget(wm->field->oppGoal());
    mkick.setWaitPos(mousePos);
    mkick.setKickSpeed(1023);


    Circle2D robotCircle(soccer->agents[0]->pos(), 0.5);



    Vector2D sol1,sol2;




    if(robotCircle.intersection(Segment2D(wm->ball->pos,wm->ball->pos + wm->ball->vel.norm()*10) , &sol1 , &sol2))
    {
        sol1 = Segment2D(wm->ball->pos,wm->ball->pos + wm->ball->vel.norm()*10).nearestPoint(soccer->agents[3]->pos());

    }
    else
    {


        sol1 = soccer->agents[0]->pos() + (wm->field->oppGoal() - soccer->agents[0]->pos()).norm()*0.5  + soccer->agents[0]->vel()/8;


    }


    mymark.init(sol1,Vector2D(0,1));

    if(stopFlag == false)
    {

        mkick.execute();
        mymark.execute();
    }

    return;



    //    QList <int> ourRelax;
    //    QList <Vector2D> result;
    //    ourRelax.clear();
    //    soccer->agents[3]->initPlanner(3,mousePos,ourRelax,ourRelax,true,false,1);
    //    result.clear();
    //    for( int i=0 ; i<soccer->agents[3]->pathPlannerResult.size() ; i++ )
    //    {
    //        result.append(soccer->agents[3]->pathPlannerResult[i]);
    //        draw(soccer->agents[3]->pathPlannerResult[i]);
    //    }
    static CSkillGotoPointAvoid mygpa(soccer->agents[3]);
    mygpa.init(mousePos,Vector2D(0,0));
    mygpa.execute();
    return;
    ///////////////////////////////////////////
    //    static CskillNewGotoPoint mygp(soccer->agents[4]);
    //    Vector2D p1(-3,0),p2(0,0),p3(0,2),p4(-3,2);
    //    static int Estate = 0;

    //    switch(Estate)
    //    {
    //    case 0 :
    //        mygp.init(p1,Vector2D(0,1));
    //        if(soccer->agents[4]->pos().dist(p1) < 0.5)
    //            Estate =1;
    //        break;
    //    case 1 :
    //        mygp.init(p2,Vector2D(0,1));
    //        if(soccer->agents[4]->pos().dist(p2) < 0.5)
    //            Estate =0;
    //        break;
    //    case 2 :
    //        mygp.init(p3,Vector2D(0,1));
    //        if(soccer->agents[4]->pos().dist(p3) < 0.5)
    //            Estate =3;
    //        break;
    //    case 3 :
    //        mygp.init(p4,Vector2D(0,1));
    //        if(soccer->agents[4]->pos().dist(p4) < 0.5)
    //            Estate =0;
    //        break;



    //    }




    //    if(stopFlag == false)
    //        mygp.execute();

    //    return;




    static  CSkillKick mykick(soccer->agents[4]);
    mykick.setTarget(wm->field->ourGoal());
    mykick.setSlow(false);
    mykick.setInterceptMode(false);
    mykick.setSagMode(false);

    mykick.setKickSpeed(500);
    mykick.setVeryFine(false);
    if(stopFlag  == false)
        mykick.execute();




    return;


    draw(QString("js: %1").arg(soccer->joystick->getButton1()),Vector2D(0,1));
    draw(mousePos);

    return;
    if( wm->field->isInOurPenaltyArea(mousePos) )
        draw(mousePos);

    return;


    //Technical Challenge Shoothing for Robocup 2013
    int id = 0;
    static CSkillKick* kick = new CSkillKick( soccer->agents[id]);
    static CSkillGotoPointAvoid* gp = new CSkillGotoPointAvoid( soccer->agents[id]);

    kick->setAgent(soccer->agents[id]);
    gp->setAgent(soccer->agents[id]);

    static int mode = 0;

    knowledge->updateGameState();

    if( knowledge->getGameState() == CKnowledge::Halt)
        mode = -1;
    else if( knowledge->getGameState() == CKnowledge::Stop)
        mode = 0;
    else if( knowledge->getGameState() == CKnowledge::OurDirectKick)
        mode = 1;
    static double bestW = 0.6;
    static double maxW = 0.0;
    static double t;

    if( mode == 0)
    {
        bestW = 0.6;
        maxW = 0.0;
        t = knowledge->currentTime();
        gp->setTargetLook((wm->field->ourGoal() - wm->ball->pos).norm()*(0.5+CRobot::center_from_kicker_new)+wm->ball->pos, wm->ball->pos);
        gp->execute();
    }
    else if( mode == -1)
    {
        //		haltAllRobots();
        soccer->agents[id]->waitHere();
        bestW = 0.6;
        maxW = 0.0;
        t = knowledge->currentTime();
    }
    else if( mode = 1)
    {
        if( knowledge->currentTime() - t > 6.0)
            bestW = maxW - 0.06;
        double goal_p;
        QList<int> ourrelaxed;
        QList<int> opprelaxed;
        ourrelaxed.append(id);
        Vector2D tar = knowledge->getEmptyPosOnGoal(wm->ball->pos, goal_p, true, ourrelaxed, opprelaxed, 0.9, 1.0); ///calculate empty angle     //p*empty_width + (1-p)*goalwidth
        //        tar = wm->field->ourGoalL();
        debug(QString("Goal prob : %1").arg(goal_p),D_SEPEHR);
        if( maxW < goal_p)
            maxW = goal_p;
        gp->setTargetLook((wm->ball->pos - tar).norm()*0.15+wm->ball->pos, wm->ball->pos);
        kick->setTarget(tar);
        kick->setChip(false);
        kick->setInterceptMode(true);
        kick->setKickSpeed(999);
        if( goal_p > bestW)
            kick->execute();
        else
            gp->execute();
    }



    return;


    //	draw(wm->field->getRegion("tc2012bottom1"),"red",true);
    //	draw(wm->field->getRegion(CField::TC2012Bottom2),"red",true);
    //	draw(wm->field->getRegion(CField::TC2012Bottom3),"red",true);
    //	draw(wm->field->getRegion(CField::TC2012Bottom4),"red",true);
    //	draw(wm->field->getRegion(CField::TC2012Top1),"red",true);
    //	draw(wm->field->getRegion(CField::TC2012Top2),"red",true);
    //	draw(wm->field->getRegion(CField::TC2012Top3),"red",true);
    //	draw(wm->field->getRegion(CField::TC2012Top4),"red",true);

    static CSkillGotoPointAvoid* gp0 = new CSkillGotoPointAvoid(soccer->agents[0]);
    static CSkillGotoPointAvoid* gp1 = new CSkillGotoPointAvoid(soccer->agents[1]);
    static CSkillGotoPointAvoid* gp2 = new CSkillGotoPointAvoid(soccer->agents[2]);
    static CSkillGotoPointAvoid* gp3 = new CSkillGotoPointAvoid(soccer->agents[3]);

    gp0->setFinalDir(Vector2D(0,0));
    gp0->setLookAt(Vector2D(0,0));
    gp1->setFinalDir(Vector2D(0,0));
    gp1->setLookAt(Vector2D(0,0));
    gp2->setFinalDir(Vector2D(0,0));
    gp2->setLookAt(Vector2D(0,0));
    gp3->setFinalDir(Vector2D(0,0));
    gp3->setLookAt(Vector2D(0,0));

    static Vector2D tar0[2]={Vector2D(2.3 , 0.65) , Vector2D(0 , 0.65)};
    static Vector2D tar1[2]={Vector2D(-0.3 , 0.65) , Vector2D(-2.6 , 0.65)};
    static Vector2D tar2[2]={Vector2D(0.3 , -0.65) , Vector2D(2.6 , -0.65)};
    static Vector2D tar3[2]={Vector2D(-2.3 , -0.65) , Vector2D(0 , -0.65)};

    static int flags[4] = {0};
    static bool start = false;
    static int num = 0 , cnt = 0;

    if( cnt == 500 )
        cnt = 0 , start = false , num = 0;


    if( start ){
        if( soccer->agents[0]->pos().dist(tar0[flags[0]]) < 0.05 ){
            flags[0] = 1-flags[0];
        }

        if( soccer->agents[1]->pos().dist(tar1[flags[1]]) < 0.05 ){
            flags[1] = 1-flags[1];
        }

        if( soccer->agents[2]->pos().dist(tar2[flags[2]]) < 0.05 ){
            flags[2] = 1-flags[2];
        }

        if( soccer->agents[3]->pos().dist(tar3[flags[3]]) < 0.05 ){
            flags[3] = 1-flags[3];
        }

        gp0->setTarget(tar0[flags[0]] , Vector2D(1,0));
        gp0->setMaxVelocity(1);

        gp1->setTarget(tar1[flags[1]] , Vector2D(1,0));
        gp1->setMaxVelocity(1);

        gp2->setTarget(tar2[flags[2]] , Vector2D(1,0));
        gp2->setMaxVelocity(1);

        gp3->setTarget(tar3[flags[3]] , Vector2D(1,0));
        gp3->setMaxVelocity(1);
    }
    else{
        gp0->setTarget(tar0[0] , Vector2D(1,0));
        gp0->setMaxVelocity(1);

        gp1->setTarget(tar1[0] , Vector2D(1,0));
        gp1->setMaxVelocity(1);

        gp2->setTarget(tar2[0] , Vector2D(1,0));
        gp2->setMaxVelocity(1);

        gp3->setTarget(tar3[0] , Vector2D(1,0));
        gp3->setMaxVelocity(1);

        if( soccer->agents[0]->pos().dist(tar0[0]) < 0.05 )
            num |= 1<<0;
        if( soccer->agents[1]->pos().dist(tar1[0]) < 0.05 )
            num |= 1<<1;
        if( soccer->agents[2]->pos().dist(tar2[0]) < 0.05 )
            num |= 1<<2;
        if( soccer->agents[3]->pos().dist(tar3[0]) < 0.05 )
            num |= 1<<3;
        if( num == 15 )
            start = true;
    }

    gp0->setNoAvoid(true);
    gp1->setNoAvoid(true);
    gp2->setNoAvoid(true);
    gp3->setNoAvoid(true);

    gp0->execute();
    gp1->execute();
    gp2->execute();
    gp3->execute();
    cnt++;


#ifndef GAME_MODE
    //    QList<Circle2D> avoid;
    //    QList<Vector2D> defendersPos;
    //    Vector2D goaliePos;
    //    QList<int> re;
    //    double w;
    //    knowledge->getEmptyPosOnGoal(knowledge->getMousePos(), w, false, re, re, false);
    //    debug(QString("ww=%1").arg(w), D_ALI);
    /*
    static CDefensePositioning dp;
    dp.createLookupTable(2, true, 0.5, 10, 10);
    return;
    QList<Circle2D> c;
    QList<Vector2D> defs;
    Vector2D g;
//knowledge->generateDefensePositions(2,true,c,defs,g);
debug(QString("defs = %1").arg(defs.count()), D_ALI);
for (int k=0;k<defs.count();k++)
    draw(defs[k],1,"blue");
    draw(g,1,"yellow");


 static int active = -1;
 static CKickCallibrator cal;
 cal.setAgent(soccer->agents[0]);
 cal.setKickPoint(wm->field->ourPenalty());
 cal.setTarget(wm->field->center());
 cal.execute();*/
    /* if ( knowledge->joystick->getButton6() || )
 {
     active *= -1;
 }*/

    /*    double ax1 = knowledge->joystick->getAxes( 0);
    double ax2 = knowledge->joystick->getAxes( 1);
    double ax3 = knowledge->joystick->getAxes( 2);
    qDebug() << ax1 << " " << ax2 ;
    soccer->agents[4]->setRobotVel( ax2, ax1, 0.0);
//soccer->agents[3]->setKick( knowledge->joystick->getButton1(), 0);*/

    /*    static CSkillGotoPointAvoid* gp = new CSkillGotoPointAvoid(soccer->agents[4]);

    gp->setNoAvoid(true);

    gp->setTarget( Vector2D(soccer->agents[4]->pos().x, soccer->agents[4]->pos().y+0.5), Vector2D(1,0.4).norm());

    soccer->agents[4]->setRoller( 6);

                                  gp->execute();
*/

#endif
}


#endif // EXPERIMENTAL6_H
