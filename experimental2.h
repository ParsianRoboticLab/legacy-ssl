
#ifndef EXPERIMENTAL2_H
#define EXPERIMENTAL2_H
#include "mainapplication.h"
#include "trajectoryplanner.h"
double dist=3,v=0;
#include "skills/autoballplacement.h"
#include "theirpenalty.h"
#include "ourpenalty.h"
#include "coach.h"
#include "geom.h"

COurPenalty *ourPenaltyTech;
CTheirPenalty *TheirPenaltyTech;

enum PenaltyState{
    KICKK = 0,
    KICK_SHOT = 1,
    KICK_CHIP = 2
};


int countt = 0;
Vector2D lastBallPos = Vector2D(0,0);
bool start = true;
void CMainApplication::Experimental2()
{
    static CSkillKick *kicker = new CSkillKick(knowledge->getActiveAgents().at(1));
    kicker->setDontKick(true);
    kicker->setChip(false);
    kicker->setTarget(Vector2D(2,0));
    kicker->setKickSpeed(5);

    if(Circle2D(Vector2D(2,0), 0.3).contains(knowledge->getActiveAgents().at(0)->pos())){
        kicker->setDontKick(false);
        kicker->execute();
    }
    debug(QString("vel : %1").arg(wm->ball->vel.length()), D_ATOUSA);
    if(wm->ball->vel.length() > 0.5){
        kicker->setDontKick(true);
    }
    else{
        kicker->setDontKick(false);
    }


    static CSkillKickOneTouch *oneToucher = new CSkillKickOneTouch(knowledge->getActiveAgents().at(0));
    oneToucher->setTarget(wm->field->oppGoal());
    oneToucher->setWaitPos(Vector2D(2,0));
    oneToucher->setKickSpeed(10);
    oneToucher->execute();



    return;
    //technicalChalenge Penalty

    int id = 2;
    int oppActiveIndx = 0;
    draw(Vector2D(+1.5,0));

    knowledge->updateGameState();

    draw(Circle2D(Vector2D(-1.5, 0), 0.1), "cyan");

//    draw(Segment2D (sol1, wm->field->ourGoal()-Vector2D(2,0)), QColor(Qt::darkYellow));

    if( knowledge->getGameState() == CKnowledge::Stop ){

        draw(QString("Stop"), Vector2D(-4, 2.5), "blue");
        Vector2D sol1, sol2;
        Line2D tempLine(wm->ball->pos, wm->field->oppGoal());
//        draw(Circle2D(wm->ball->pos, 0.6), QColor(Qt::cyan));
        Circle2D(wm->ball->pos, 0.6).intersection(tempLine, &sol1, &sol2);
//        draw(Segment2D (sol1, wm->field->oppGoal()), QColor(Qt::darkYellow));
//        draw(Segment2D (sol2, wm->field->oppGoal()), QColor(Qt::darkYellow));
//        draw(Circle2D(sol1, 0.2), QColor(Qt::blue));
//        draw(Circle2D(sol2, 0.2), QColor(Qt::blue));

        Vector2D agentPos;
        if( (wm->field->oppGoal()-sol1).length() < (wm->field->oppGoal()-sol2).length() ){
            agentPos = sol2;
        }
        else
            agentPos = sol1;

        static CSkillGotoPointAvoid *stop = new CSkillGotoPointAvoid(knowledge->getAgent(id));
        stop->init(agentPos, wm->ball->pos);
        stop->setTargetLook(agentPos, wm->ball->pos);
        stop->execute();

    }
    else if( knowledge->getGameState() == CKnowledge::OurPenaltyKick ){

        draw(QString("ourPenalty"), Vector2D(-4, 2.5), "blue");

        Vector2D sol1, sol2;
        Line2D tempLine(wm->ball->pos, wm->field->oppGoal());
        Circle2D(wm->ball->pos, 0.2).intersection(tempLine, &sol1, &sol2);

        Vector2D agentPos;
        if( (wm->field->oppGoal()-sol1).length() < (wm->field->oppGoal()-sol2).length() ){
            agentPos = sol2;
        }
        else
            agentPos = sol1;

        static CSkillGotoPointAvoid *goBehindBall = new CSkillGotoPointAvoid(knowledge->getAgent(id));
        goBehindBall->init(agentPos, wm->ball->pos);
        goBehindBall->setTargetLook(agentPos, wm->ball->pos);
        goBehindBall->execute();

    }

    else if( knowledge->getGameState() == CKnowledge::TheirPenaltyKick ){
        draw(QString("theirPenalty"), Vector2D(-4, 2.5), "blue");
//        draw(Circle2D(wm->field->ourGoal(), 0.5), QColor(Qt::blue));
        static CSkillGotoPointAvoid *goToOurGoal = new CSkillGotoPointAvoid(knowledge->getAgent(id));

//        goToOurGoal->init(wm->field->ourGoal(), wm->ball->pos);
//        goToOurGoal->setTargetLook(wm->field->ourGoal(), wm->ball->pos);

        //what should goalie do?

        /*
        //dar jahate opp and ball mimune
        Line2D ourLine(wm->field->ourGoal() + Vector2D(0, 0.5), wm->field->ourGoal() + Vector2D(0, -0.5));
        draw(Segment2D(wm->field->ourGoal() + Vector2D(0, 0.5), wm->field->ourGoal() + Vector2D(0, -0.5)), "red");
        Line2D ballLine(wm->opp.active(oppActiveIndx)->pos, wm->ball->pos);
        draw(Segment2D(wm->opp.active(oppActiveIndx)->pos, wm->ball->pos), "red");
        Vector2D sol1;
        sol1 = ourLine.intersection(ballLine);
        draw(Circle2D(sol1, 0.2), "red");

        Vector2D targetLook;
        if( sol1.y > 0.5 ){
            targetLook = Vector2D(-4.3, 0.3);
        }
        else if( sol1.y < -0.5 ){
            targetLook = Vector2D(-4.3, -0.3);
        }
        else{
            targetLook = sol1 + Vector2D(0.2, 0);
        }

        goToOurGoal->setTargetLook(targetLook, wm->ball->pos);
        goToOurGoal->execute();
        */


/*
        static CRolePlayOn *goalie = new CRolePlayOn();

        goalie->setAgent(knowledge->getAgent(id));
        goalie->setAgentID(id);
//        goalie->setReceiveRadius(1);
//        goalie->setTolerance(0.01);
        goalie->setIsActive(true);
//        goalie->setChip(false);
        goalie->setSlow(true);
*/



        /*
        if( !start ){

            goalie->setSelectedSkill(roleSkill::GotopointAvoid);

            if(Circle2D(Vector2D(-4.4, 0.4), 0.4).contains(knowledge->getAgent(id)->pos())){
                goalie->setWaitPos(Vector2D(-4.4,-0.4));
                goalie->setTarget(Vector2D(-4.4,-0.4));
            }
            if(Circle2D(Vector2D(-4.4,-0.5), 0.4).contains(knowledge->getAgent(id)->pos())){
                goalie->setWaitPos(Vector2D(-4.4,0.4));
                goalie->setTarget(Vector2D(-4.4,0.4));
            }
        }
        else{
            goalie->setSelectedSkill(roleSkill::GotopointAvoid);
            goalie->setTarget(Vector2D(-4.4, 0.4));
            goalie->setTargetDir(wm->ball->pos);
            start = false;
        }
        */

        /*
        goalie->setSelectedSkill(roleSkill::Kick);
        goalie->setTarget(Vector2D(0, -4));
        goalie->setChip(true);
        goalie->setKickSpeed(1000/100);

        goalie->execute();
        */


        if( !Circle2D(wm->field->ourGoal()+Vector2D(6,0),0.5).contains(wm->ball->pos) ){//ball is not in 6 meters
            debug("in circle", D_ATOUSA);
            if( wm->ball->vel.length() < 1){//kick the ball

                static CRolePlayOn *goalie = new CRolePlayOn();

                goalie->setAgent(knowledge->getAgent(id));
                goalie->setAgentID(id);
                goalie->setIsActive(true);
                goalie->setSlow(false);
                goalie->setSelectedSkill(roleSkill::Kick);
                goalie->setTarget(Vector2D(0, -4));
                goalie->setChip(true);
                goalie->setKickSpeed(1000);

                goalie->execute();

            }
            else{ //dar tule darvaze dar jahate felan bashe
//                if( countt == 10 ){
                    static CSkillKickOneTouch mOT(knowledge->getAgent(id));
                    Line2D ourLine(wm->field->ourGoal() + Vector2D(0, 0.5), wm->field->ourGoal() + Vector2D(0, -0.5));
                    draw(Segment2D(wm->field->ourGoal() + Vector2D(0, 0.5), wm->field->ourGoal() + Vector2D(0, -0.5)), "red");
                    Line2D ballLine(lastBallPos, wm->ball->pos);
                    draw(Segment2D(lastBallPos, wm->ball->pos), "red");
                    Vector2D sol1;
                    sol1 = ourLine.intersection(ballLine);
                    draw(Circle2D(sol1, 0.2), "red");

                    Vector2D targetLook;
                    if( sol1.y > 0.5 ){
                        targetLook = Vector2D(-4.3, 0.3);
                    }
                    else if( sol1.y < -0.5 ){
                        targetLook = Vector2D(-4.3, -0.3);
                    }
                    else{
                        targetLook = sol1 + Vector2D(0.2, 0);
                    }
                    mOT.setWaitPos(targetLook);
                    mOT.setTarget(Vector2D(10,10));
                    mOT.setKickSpeed(1000);
                    mOT.setChip(true);
                    mOT.execute();

                    //                }
//                else
//                    countt++;
            }
        }
        else{// is in 6 meters
//            debug("not in circle", D_ATOUSA);
            static CSkillGotoPointAvoid *goToOurGoal = new CSkillGotoPointAvoid(knowledge->getAgent(id));
            goToOurGoal->init(wm->field->ourGoal(),Vector2D(1,0));
            goToOurGoal->execute();
        }

    }

//    else if( knowledge->getGameState() == CKnowledge::Halt ){
//        draw(QString("Halt"), Vector2D(-4, 2.5), "blue");
//        haltAllRobots();//oke
//    }

//    else if( knowledge->getGameState() == CKnowledge::NormalStart ){

//        if( knowledge->getGameMode() == CKnowledge::OurPenaltyKick ){
//            draw(QString("ourPenaltyNormalStart"), Vector2D(-3,-2), "blue");

////            //    KICK, KICK_SHOT, KICK_CHIP
////            PenaltyState state = KICKK;
////            switch(state){

////            case KICKK:

////                Vector2D targetGoal;
////                double upY = abs(((wm->field->oppGoal() + Vector2D(0, 0.5))- wm->opp.active(oppActiveIndx)->pos).y);
////                double downY = abs(((wm->field->oppGoal() + Vector2D(0, -0.5))- wm->opp.active(oppActiveIndx)->pos).y);
////                if( upY < downY ){
////                    targetGoal = wm->field->oppGoal() + Vector2D(0, -0.5) + Vector2D(0.05, downY/3);
////                }
////                else{
////                    targetGoal = wm->field->oppGoal() + Vector2D(0,  0.5) + Vector2D(0.05, -1*(upY/3));
////                }

////                draw(Segment2D(targetGoal, knowledge->getAgent(id)->pos()), "cyan");

////                static CSkillKick *kicker = new CSkillKick(knowledge->getAgent(id));
////                kicker->setTarget(targetGoal);
////                kicker->setKickSpeed(1000);
////                //un circle ro chikar konm? ke vagti kck mkone nare donbalesh(??)
////                if( !Circle2D(Vector2D(-1.5, 0), 0.5).contains(wm->ball->pos) )
////                    kicker->setDontKick(true);
////                else
////                    kicker->setDontKick(false);
////                kicker->execute();

////                break;


////            case KICK_SHOT:
////                //
////                break;
////            case KICK_CHIP:

////                break;
////            }

//            static CSkillKick mkick(knowledge->getAgent(id));
//            mkick.setShotToEmptySpot(true);
//            if(wm->ball->pos.x < 1)
//            {
//                mkick.setKickSpeed(50);
//            }
//            else
//            {
//                mkick.setKickSpeed(1000);
//            }

//            mkick.execute();
//        }

//    }




















//    static CSkillGotoPointAvoid *stop = new CSkillGotoPointAvoid(knowledge->getAgent(id));
//    stop->init(Vector2D(0,0), wm->ball->pos);
//    stop->execute();

/*
    static CRolePlayOn * skiiill= new CRolePlayOn();

    id = 5;
    skiiill->setAgent(knowledge->getAgent(id));
    skiiill->setAgentID(id);
    skiiill->setReceiveRadius(1.2);
    skiiill->setTolerance(0.01);
    skiiill->setIsActive(true);
    skiiill->setChip(false);
    skiiill->setSlow(true);

    skiiill->setSelectedSkill(roleSkill::GotopointAvoid);

    if(Circle2D(Vector2D(4,2.5), 0.4).contains(knowledge->getAgent(skiiill->getAgentID())->pos())){
        skiiill->setWaitPos(Vector2D(-4,2.5));
        skiiill->setTarget(Vector2D(-4,2.5));
    }
    if(Circle2D(Vector2D(-4,2.5), 0.4).contains(knowledge->getAgent(skiiill->getAgentID())->pos())){
        skiiill->setWaitPos(Vector2D(4,2.5));
        skiiill->setTarget(Vector2D(4,2.5));
    }
    skiiill->execute();
*/

    return;

    static COurBallPlacement *obp = new COurBallPlacement();
    QList<int> ag;
    //    ag.append(0);
    ag.append(1);
    //    ag.append(2);
    //    ag.append(3);
    //    ag.append(4);
    ag.append(5);
    obp->init(ag, NULL);
    obp->execute();

    /*
    static CSkillAutoBallPlacement *abp = new CSkillAutoBallPlacement(knowledge->getAgent(0));
    abp->execute();
*/
    //gotoPoint

    /*
    static CSkillGotoPointAvoid *skillll = new CSkillGotoPointAvoid(knowledge->getAgent(0));
    skillll->init(Vector2D(-2,-2), Vector2D(0,0));
    skillll->execute();
    */

    //debug(QString("Hamed %1").arg(policy()->Mark_Test()), D_MAHI);
    return;
    /////////////////////////////////////////////////////////
    //    static CSkillNEWKeep *keepBall = new CSkillNEWKeep( soccer->agents[0] );
    //    keepBall->execute();
    //    return;
    for(int i = 0; i < 8; i++)
        knowledge->SRSetAgentArg(i, i, (rand()%1023)/100, 2, 3, 4, 5);
    return;
    static CSkillReceivePass* RPA = new CSkillReceivePass(knowledge->getAgent(2));
    RPA->setReceiveRadius(1);
    RPA->setTarget(knowledge->getMousePos());
    RPA->execute();

    return;
    static Vector2D lastPos = Vector2D(0,0);
    static double speed = 0;

    speed = wm->ball->pos.dist(lastPos)*1000/16;
    if(speed > 0.1)
        debug(QString("speed : (%1)").arg(speed),D_MAHI);
    lastPos = wm->ball->pos;

    return;

    double r = 3.0;

    CRolePlayOn tRole;
    tRole.setAgent(knowledge->getAgent(0));
    tRole.setIsActive(true);
    tRole.setAgentID(0);
    tRole.setTask(PassDefensive);
    tRole.setTarget(Vector2D(2,2));
    tRole.setTolerance(0.1);
    tRole.setChip(true);
    tRole.setKickSpeed(10);
    tRole.setAvoidPenaltyArea(true);
    tRole.setAvoidCenterCircle(false);
    tRole.setIsGotoPointAvoid(true);
    tRole.setSlow(true);
    tRole.setSelectedSkill(roleSkill::Kick);
    Vector2D kickerPos = knowledge->getAgent(0)->pos();
    Vector2D targetPos = tRole.getTarget();
    AngleDeg diff = AngleDeg(asin((r/2)/(kickerPos.dist(targetPos)))*(180/3.141593));
    AngleDeg Dstans = (targetPos - kickerPos).norm().dir();
    Segment2D seg1 = Segment2D(kickerPos,Vector2D().setPolar(5,diff + Dstans) + kickerPos);
    Segment2D seg2 = Segment2D(kickerPos,Vector2D().setPolar(5,-diff + Dstans) + kickerPos);
    draw(Circle2D(targetPos, r/2),QColor(Qt::cyan));
    draw(seg1);
    draw(seg2);
    if(Triangle2D(seg1,Vector2D().setPolar(5,-diff + Dstans) + kickerPos).contains(wm->ball->pos))
        debug(QString("mahi"),D_MAHI);
    else
        debug(QString("!mahi"),D_MAHI);

    QList<Circle2D> Obstacles;
    for(int i = 0;i<wm->opp.activeAgentsCount();i++){
        Obstacles.append(Circle2D(wm->opp.active(i)->pos,0.20));
        if(Obstacles.at(i).contains(wm->ball->pos)) debug(QString("!Mahi"),D_KK);
    }


    //tRole.execute();

    return;
    //draw(QString("sag is: %1").argf(knowledge->getAgent(0)->abilities.canChip),Vector2D(1,1));
    CskillNewGotoPoint *agent = new CskillNewGotoPoint(NULL);
    agent->setAgent(knowledge->getAgent(0));
    agent->setFinalPos(Vector2D(0,0));
    agent->setFinalVel(Vector2D(0,0));
    agent->setSlowMode(true);

    agent->execute();

    return;
    //CRolePlayOn agent;


    return;
    soccer->agents[0]->waitHere();
    qDebug() << soccer->agents[0]->vforward;
    return;
#ifndef GAME_MODE
    static CSkillGotoPointAvoid* gp = new CSkillGotoPointAvoid( soccer->agents[0]);
    static CSkillTurn* turn = new CSkillTurn( soccer->agents[0]);
    static int mode = 0;
    static CSkillKick* kick = new CSkillKick( soccer->agents[6]);

    //    draw(wm->field->getRegion("oppmidfieldbottom"),"black",true);
    draw(Circle2D(Vector2D( 2.3, -1.0),CRobot::robot_radius_new),0,360,"orange",true);
    draw(Circle2D(Vector2D( 2.16, -0.8),CRobot::robot_radius_new),0,360,"magenta",true);
    draw(Circle2D(Vector2D( 2.5, 1.1),CRobot::robot_radius_new),0,360,"cyan",true);
    if ( knowledge->joystick->getButton6())
        mode = 1;
    if ( knowledge->joystick->getButton8())
        mode = 0;
    if ( knowledge->joystick->getButton5())
        mode = 2;

    if ( mode )
    {
        kick->setTarget(wm->field->oppGoal());
        kick->setInterceptMode(true);
        kick->execute();
    }


    return;
    //	draw(wm->ball->predict(1),0,"cyan");
    //	draw(wm->ball->predict(0.5),0,"pink");
    //	draw(wm->ball->predict(0.75),0,"brown");
    //	debug(QString("TT 1 : %1").arg(simulator->timeNeededForGotoPoint(wm->ball->predict(1),Vector2D(0,0),0.06,10,0,0.016)),D_SEPEHR);
    //	debug(QString("TT 0.5 : %1").arg(simulator->timeNeededForGotoPoint(wm->ball->predict(0.5),Vector2D(0,0),0.06,10,0,0.016)),D_SEPEHR);
    //	debug(QString("TT 0.75 : %1").arg(simulator->timeNeededForGotoPoint(wm->ball->predict(0.75),Vector2D(0,0),0.06,10,0,0.016)),D_SEPEHR);

    draw(wm->ballCatchTarget(soccer->agents[0]->self()),0,"purple");

    //	Vector2D catchPoint = wm->ball->pos;

    //	bool found = false;

    //	double tt = 0;
    //	while( !found)
    //	{
    //		if (simulator->timeNeededForGotoPoint(wm->ball->predict(tt),Vector2D(0,0),0.06,10,0,0.016) - tt < 0.3)
    //		{
    //			found = true;
    //			catchPoint = wm->ball->predict(tt);
    //		}
    //		else
    //			tt+=0.1;
    //	}
    //	draw(catchPoint,0,"cyan");

    if ( mode == 1)
    {
        gp->setAgent(soccer->agents[0]);
        Vector2D pointToGo;
        Vector2D target = wm->field->oppGoal();
        pointToGo = (wm->ball->pos - target ).norm() * 0.24 + wm->ball->pos;
        gp->setTargetLook(pointToGo,wm->ball->pos);

        double vx = 0, vy = 0, vxball = 0, vyball = 0;
        soccer->agents[0]->setRobotAbsVel(wm->ball->vel.x,wm->ball->vel.y,0);
        vxball = soccer->agents[0]->vforward;
        vyball = soccer->agents[0]->vnormal;
        soccer->agents[0]->setRobotVel(0,0,0);
        debug(QString("ballVel : %1, %2").arg(vxball).arg(vyball),D_SEPEHR);
        wm->ball->obstacleRadius = 0.15;

        gp->execute();

        Vector2D desiredVelGP(soccer->agents[0]->vforward,soccer->agents[0]->vnormal);
        debug(QString("gp : %1, %2").arg(desiredVelGP.x).arg(desiredVelGP.y),D_SEPEHR);

        Vector2D agent2ball = pointToGo - soccer->agents[0]->pos();
        //	if ( desiredVelGP.innerProduct( Vector2D(vxball,vyball)) > 0)
        //	if ( agent2ball.innerProduct( Vector2D(vxball,vyball)) > 0)
        if ( agent2ball.length() > 0.3 && desiredVelGP.innerProduct( Vector2D(vxball,vyball)) > 0)
        {
            Vector2D tempV = desiredVelGP.norm()*(Vector2D(vxball,vyball).innerProduct(desiredVelGP.norm()));
            //		Vector2D tempV( vxball,vyball);
            //		Vector2D tempV = agent2ball.norm()*(Vector2D(vxball,vyball).innerProduct(agent2ball.norm()));
            vx = tempV.x ;
            vy = tempV.y;
            debug(QString("image : %1, %2").arg(vx).arg(vy),D_SEPEHR);
            //		turn->setDirection(desiredVelGP);
        }
        else if ( agent2ball.length() < 0.3)
        {
            Vector2D tempV( vxball,vyball);
            vx = tempV.x ;
            vy = tempV.y;
            //		turn->setDirection(target - wm->ball->pos);
        }

        vx += desiredVelGP.x;
        vy += desiredVelGP.y;

        //	if( fabs(Vector2D::angleBetween((wm->ball->pos - soccer->agents[0]->pos()),(target-soccer->agents[0]->pos() )).degree()) < 1.0)
        //		vx = 0;


        soccer->agents[0]->setRoller(0);
        soccer->agents[0]->setKick(0);

        debug(QString("angle : %1").arg(fabs(Vector2D::angleBetween((wm->ball->pos - soccer->agents[0]->pos()),(target-soccer->agents[0]->pos() )).degree())),D_SEPEHR,"purple");
        draw(Segment2D(target,soccer->agents[0]->pos()),"orange");
        draw(Segment2D(wm->ball->pos,soccer->agents[0]->pos()),"purple");

        double difAng;
        if ( soccer->agents[0]->self()->getKickerPos().dist(wm->ball->pos) > 0.15)
            difAng = 9.0;
        else
        {
            difAng = ((0.15-soccer->agents[0]->self()->getKickerPos().dist(wm->ball->pos))*60)+9;
        }

        debug(QString("Dif angle : %1").arg(difAng),D_SEPEHR,"pink");

        if( fabs(Vector2D::angleBetween((wm->ball->pos - soccer->agents[0]->pos()),(target-soccer->agents[0]->pos() )).degree()) < difAng && target.dist(wm->ball->pos) < target.dist(soccer->agents[0]->pos()))
        {
            vx+= min(0.6,0.6*(difAng/fabs(Vector2D::angleBetween((wm->ball->pos - soccer->agents[0]->pos()),(target-soccer->agents[0]->pos() )).degree())));

            soccer->agents[0]->setRoller(7);
            soccer->agents[0]->setKick(soccer->agents[0]->kickSpeedValue(6.0,true));
        }

        //	if ( fabs(Vector2D::angleBetween((soccer->agents[0]->dir()),(target-soccer->agents[0]->pos() )).degree()))
        //		soccer->agents[0]->setKick(soccer->agents[0]->kickSpeedValue(6.0,true));

        debug(QString("final : %1, %2").arg(vx).arg(vy),D_SEPEHR);

        soccer->agents[0]->setRobotVel(vx, vy, 0);

        turn->setDirection(target - wm->ball->pos);
        turn->setAgent(soccer->agents[0]);
        turn->setTurnMode(CSkillTurn::Intercept);
        turn->execute();

        Vector2D givenVel( vx,vy);
        if ( givenVel.length() > 3.9)
        {
            givenVel = givenVel.norm()*3.9;
            soccer->agents[0]->setRobotVel(givenVel.x, givenVel.y, 0);
        }


    }
    else if ( mode == 2)
    {
        soccer->agents[0]->setRobotAbsVel(wm->ball->vel.x,wm->ball->vel.y,0);
        Vector2D target = wm->field->oppGoal();
        turn->setAgent(soccer->agents[0]);
        turn->setDirection(target - wm->ball->pos);
        turn->setTurnMode(CSkillTurn::Intercept);
        turn->execute();
    }
    else
        soccer->agents[0]->setRobotVel(0, 0, 0);

    /*    double a,t,dt=0.05;
    v = CTrajectoryPlanner::plan(fabs(dist),sign(dist)*v,0.0,8.0,6.0,3.0,dt,a,t);
    dist -= sign(dist)*v*dt;
    draw(Vector2D(0,0));
    QColor color;
    if (a==0.0) color=QColor("black");
    if (a> 0.0) color=QColor("blue");
    if (a< 0.0) color=QColor("red");
    debug(QString("dist=%1").arg(dist), D_EXPERIMENT);
    draw(Vector2D(dist,0),1);//,color);*/

    /*draw(wm->field->getRegion(CField::OurMidFieldTop),"yellow",true);
    draw(wm->field->getRegion(CField::OppMidFieldTop),"orange",true);
    draw(wm->field->getRegion(CField::OurMidFieldBottom),"gray",true);
    draw(wm->field->getRegion(CField::OppMidFieldBottom),"blue",true);

    static CSkillKick* kick = new CSkillKick(soccer->agents[0]);

    static int active = -1;
    static int cntr = 50;
    static int mode = 1;
    static int age = 0;

    if (knowledge->joystick->getButton5() && cntr > 50)
    {
        mode *= -1;
        cntr = 0;
    }
    if ( knowledge->joystick->getButton6() && cntr > 50)
    {
        active *= -1;
        cntr = 0;
    }
    cntr ++;
    if ( knowledge->joystick->getButton7())
        age = 0;
    if ( knowledge->joystick->getButton1())
        age = 1;
    if ( knowledge->joystick->getButton2())
        age = 2;
    if ( knowledge->joystick->getButton4())
        age = 4;
    if ( knowledge->joystick->getButton3())
        age = 3;
    if ( knowledge->joystick->getButton5())
        age = 5;

    kick->setAgent(soccer->agents[age]);
        kick->setTarget( wm->field->oppGoal());
        kick->setKickSpeed(soccer->agents[age]->kickValueForDistance( (wm->ball->pos-wm->field->oppGoal()).length(), 5.0));
        qDebug() << soccer->agents[age]->kickValueForDistance( (wm->ball->pos-wm->field->oppGoal()).length(), 5.0);
        kick->setThroughMode(false);
        kick->setSlow(true);
        kick->setInterceptMode(false);
        kick->setParallelMode(false);
        kick->setWaitFrames( 0);
        kick->setTolerance( 0.03);
        kick->setDontKick(false);
        kick->setSpin(0);
        kick->setChip(false);
        if ( active == 1)
            kick->execute();
        else
            kick->getAgent()->waitHere();
*/
    //    soccer->agents[4]->setRobotVel(0.0, 0.0, 10);
    //soccer->agents[0]->setRobotVel(0.4, 0.0, 0.0);
    //debug(QString("vel=%1").arg(soccer->agents[2]->vel().length()), D_ERROR, "red");
    //	static CSkillKick* kick = new CSkillKick(soccer->agents[0]);
    //	kick->setTarget( wm->field->oppGoal());
    //	kick->setKickSpeed(25);
    ////	kick->setSlow(true);
    //	kick->setInterceptMode(true);
    //	kick->execute();

    /*
    static CSkillGotoPointAvoid* gp = new CSkillGotoPointAvoid( soccer->agents[2]);
    gp->setNoAvoid(true);
    gp->setTargetLook( Vector2D( soccer->agents[2]->pos() - wm->ball->pos).norm()*(soccer->agents[2]->self()->centerFromKicker()+CBall::radius)+wm->ball->pos, wm->ball->pos);
    gp->execute();


    draw(wm->field->getRegion(CField::OppCornerLineBottom),"orange",true);
    draw(wm->field->getRegion(CField::OppCornerLineTop),"gray",true);
*/

    /*
    knowledge->findFastest(0.01);
    for ( int i = 0 ; i < knowledge->ourFastest.count(); i++)
        debug(QString("Our %1 nearest agent is %2").arg(i).arg( knowledge->ourFastest.at(i)), D_SEPEHR, "blue");
    for ( int i = 0 ; i < knowledge->oppFastest.count(); i++)
        debug(QString("Opp %1 nearest agent is %2").arg(i).arg( knowledge->oppFastest.at(i)), D_SEPEHR, "red");
*/
#endif
}

#endif // EXPERIMENTAL2_H
