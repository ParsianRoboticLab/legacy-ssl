#include "playmake.h"
#include <varswidget.h>
#include "conditions.h"
#include "position.h"
#include <passevaluation.h>
#include <QDebug>
#include <ctime>
using namespace std;

INIT_ROLE(CRolePlayMake, "playmake");

CRolePlayMake::CRolePlayMake(CAgent *_agent) : CRole(_agent)
{
    qqHist = 0;
    justTurn = false;
    kickToTheirDefense = false;
    kick = new CSkillKick(_agent);
    gotoball = new CSkillGotoBall(_agent);
    onetouch = new CSkillKickOneTouch(_agent);
    spin = new CSkillSpinBack(_agent);
    gotopoint = new CSkillGotoPointAvoid(_agent);
    hitTheBall = new CSkillHitTheBall(_agent);
    turn = new CSkillTurn(_agent);
    chip = slow = false;
    onetouching = -1;
    lastAgent = NULL;
    shadowPass = false;
    startKicked = false;
    indirect = false;
    direct = false;
    kickoff = false;
    indirectKhafan = false;
    chipIndirect = false;
    through = false;
    gameMode = 0;
    noKick = false;
    indirectPassRecverSelectedFrame = -1;
    penaltyRand = -1;
    penaltyCounter = 0;
    penaltyTarget.invalidate();
    manualPassReceive = false;
    allowonetouch = false;
    allowonepass = false;
    indirectTiny = false;
    passReceiver = -1;
    lastPassReceiver = -1;
    ballavoidanceState = -1;
    lastDecisionTime = 0.0;
    shoot = new CBehaviourKick;
    for (int i=0;i<PASS_BEHAVS;i++)
    {
        pass[i]  = new CBehaviourPass;
        chippass[i]  = new CBehaviourChipPass;
        spinPass[i] = new CBehaviourSpinPass;
    }
    fixedPass = new CBehaviourPass;
    chipToGoal = new CBehaviourChipToGoal;
    clear = new CBehaviourKick;
    kickToDefense = new CBehaviourKickBetweenTheirDefenders;
    lastFrameCrowded  = -1;
    ballTop.setGuardMin(-0.1);
    ballTop.setGuardMax( 0.1);
    ballLeft.setGuardMin(-0.1);
    ballLeft.setGuardMax( 0.1);
    waitBeforePass = 999999;
    orderRushInPlenalty = 999999;
    orderedRush = false;
    timerStartFlag = true;

    //    spinPass = new CBehaviourSpinPass;
    initialPoint.invalidate();
    forceRedecide = false;

    lastBounceDataFile.setFileName("lastBounce");
    out.setDevice(&lastBounceDataFile);
}

CRolePlayMake::~CRolePlayMake()
{
}

bool CRolePlayMake::spinBack()
{
    return false;
    bool pushIt = false;
    if ((stopped)
            || (kickoff)
            || (!agent->abilities.canSpin)
            || wm->ball->pos.dist(wm->field->oppCornerL() )< 0.9
            || wm->ball->pos.dist(wm->field->oppCornerR() )< 0.9)
        return false;
    if (knowledge->frameCount - lastFrameCrowded > 9)
    {
        bool flag = knowledge->isCrowdedInFrontOfAgent(agent->id(), CRobot::robot_radius_old*6);
        int ownersCount = 0;
        for (int i=0;i<wm->opp.activeAgentsCount();i++)
        {
            if ( wm->opp.active(i)->isBallOwner(0.21,CRobot::robot_radius_old))
                ownersCount++;
        }
        if( ownersCount == 0)
            flag = false;
        if (!flag) return false;
        else {
            lastFrameCrowded = knowledge->frameCount;
        }
    }
    forceRedecide = true;
    if( !initialPoint.isValid())
        initialPoint = agent->pos();
    Vector2D kickTar;
    double kickW;
    QList<int> ourRelId,oppRelId;
    oppRelId.clear();
    ourRelId.clear();
    ourRelId.append( agent->id());
    agent->setRoller(3);
    kickTar = knowledge->getEmptyPosOnGoal( agent->self()->getKickerPos(), kickW, true, ourRelId, oppRelId, 0.9);
    if (manualPassReceive) return false;
    if (indirect) return false;

    if( kickW > 0.3)
    {
        debug(QString("kicking ! w = %1").arg(kickW),D_SEPEHR);
        kick->setAgent(agent);
        kick->setChip(false);
        kick->setSlow(false);
        kick->setInterceptMode(true);
        kick->setKickSpeed(kick->getAgent()->kickSpeedValue(7.8 , false));
        kick->setDontKick(false);
        kick->setTarget(kickTar);
        kick->execute();
        return true;
    }
    else if( wm->ball->pos.dist( wm->field->oppGoal()) < 0.9)
    {
        bool hum = true;
        for(int i=0;i<knowledge->oppBlockers.count();i++)
        {
            if(wm->opp[knowledge->oppBlockers[i]]->isBallOwner(0.6,CRobot::robot_radius_old))
                hum = false;
        }
        if( !hum && wm->our[agent->id()]->isBallOwner(0.01))
        {
            agent->addRobotVel(2.1,0,0);
        }
        debug(QString("na kicke inja").arg(kickW),D_SEPEHR);
        kick->setAgent(agent);
        kick->setChip(false);
        kick->setSlow(false);
        kick->setInterceptMode(true);
        kick->setKickSpeed(kick->getAgent()->kickSpeedValue(7.8 , false));
        kick->setDontKick(true);
        kick->setTarget((wm->ball->pos - agent->pos()).norm());
        kick->execute();
        kick->setDontKick(false);
    }
    int oppBallOwner=-1;
    for(int i=0;i<knowledge->oppBlockers.count();i++)
    {
        if(wm->opp[knowledge->oppBlockers[i]]->isBallOwner(0.09))
            oppBallOwner = knowledge->oppBlockers[i];
    }
    bool rightSupporterExist, leftSupporterExist, frontSupportExist;
    rightSupporterExist = false;
    leftSupporterExist = false;
    frontSupportExist = false;
    Rect2D leftRect(Vector2D( agent->pos().x-0.3, agent->pos().y+0.36), Vector2D(agent->pos().x, agent->pos().y));
    Rect2D rightRect(Vector2D( agent->pos().x-0.3, agent->pos().y), Vector2D(agent->pos().x, agent->pos().y-0.36));
    Rect2D frontRect(Vector2D( agent->pos().x+0.27, agent->pos().y+0.21), Vector2D(agent->pos().x+0.75, agent->pos().y-0.21));
    draw(leftRect,"pink");
    draw(rightRect,"blue");
    draw(frontRect,"orange");
    for ( int i = 0; i < wm->our.activeAgentsCount(); i++)
    {
        if ( wm->our.active(i)->id == agent->self()->id)
            continue;
        if ( rightRect.contains(wm->our.active(i)->pos))
            rightSupporterExist = true;
        if ( leftRect.contains(wm->our.active(i)->pos))
            leftSupporterExist = true;
        if( frontRect.contains(wm->our.active(i)->pos))
            frontSupportExist = true;
    }
    if(wm->our[agent->id()]->isBallOwner(0.03))
    {

        //        if( wm->ball->pos.dist( wm->field->oppGoal()) < 1.2)
        //        {
        //             debug("side walking", D_SEPEHR, "darkcyan");
        //            knowledge->setSupporPlaymaker(CKnowledge::Back);
        //            agent->setRoller( 5);
        //            gotopoint->setAgent(agent);
        //            gotopoint->setNoAvoid(true);
        //            if ( spinside)
        //            {
        //                debug("right", D_SEPEHR, "orange");
        //                gotopoint->setTarget( Vector2D(agent->pos().x, agent->pos().y+0.3), Vector2D(1,0.4).norm());
        //                if ( wm->ball->pos.y > 0.6)
        //                    spinside = false;
        //            }
        //            else
        //            {
        //                debug("left", D_SEPEHR, "orange");
        //                gotopoint->setTarget( Vector2D(agent->pos().x, agent->pos().y-0.3), Vector2D(1,-0.4).norm());
        //                if ( wm->ball->pos.y < -0.6)
        //                    spinside = true;
        //            }
        //            gotopoint->execute();
        //            return true;
        //        }
        spin->setAgent(agent);
        spin->setTarget(2*agent->pos()-wm->field->oppGoal());// test it !
        if( oppBallOwner!=-1 && wm->ball->pos.dist( wm->field->ourGoal()) >= 1.8)
        {
            spin->setTarget(2*agent->pos()-wm->opp[oppBallOwner]->pos); // test it !
        }
        else
        {
            bool hey=false;
            for(int i=0;i<knowledge->oppBlockers.count();i++)
            {
                if(wm->opp[knowledge->oppBlockers[i]]->isBallOwner(0.15,CRobot::robot_radius_old))
                    hey = true;
            }
            if(hey)
            {
                debug(QString("na kicke hey").arg(kickW),D_SEPEHR);
                kick->setAgent(agent);
                kick->setChip(false);
                kick->setSlow(false);
                kick->setInterceptMode(true);
                kick->setKickSpeed(kick->getAgent()->kickSpeedValue(7.8 , false));
                kick->setDontKick(true);
                kick->setTarget((wm->ball->pos - agent->pos()).norm());
                kick->setDontKick(false);
                kick->execute();
                /*
    if(hey && wm->our[agent->id()]->isBallOwner(0.01))
    {
     agent->addRobotVel(2.1,0,0);
    }*/
            }
            else
            {
                debug("chip", D_SEPEHR, "darkcyan");
                kick->setAgent(agent);
                kick->setChip(true);
                kick->setSlow(false);
                kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist(wm->field->oppGoal())-0.6 , false));
                kick->setTolerance(0.9);
                kick->setAutoChipSpeed(false);
                kick->setTarget(wm->field->oppGoal());
                kick->setChip(false);
                kick->setKickSpeed(agent->kickSpeedValue(7.2,0));
                kick->execute();
            }
        }
        /*
  //Pushing :

  if ( agent->self()->isBallOwner(0.03) )
  {
   gotoball->setAgent(agent);
   gotoball->setStruggle(true);
   gotoball->execute();
  }
  */
        spin->setWaitFrames(9);
        spin->setTakeBack(true);
        spin->setAnglularVelocity(0);
        spin->setCorrectAngleTowardTarget(false);
        spin->setLinearVelocity(0.0);
        spin->setLinearAcceleration(2.7);
        spin->execute();
        draw("Spin 1",Vector2D(0,0),"pink",50);
        knowledge->setSupporPlaymaker(CKnowledge::Back);
        bool notBlockChipIt = true , leftClean = true, rightClean = true, backClean = true;
        if( knowledge->oppBlockers.count() > 1 )
            backClean = false;
        for (int i = 0; i < wm->opp.activeAgentsCount(); i++)
        {
            Vector2D checkDir = wm->field->oppGoal() - wm->ball->pos;
            if(wm->opp.active(i)->pos.dist(wm->ball->pos) < 1.0)
            {
                Vector2D agentDir = wm->opp.active(i)->pos - wm->ball->pos;
                if( Vector2D::angleBetween( checkDir , agentDir).degree() > 60 && Vector2D::angleBetween( checkDir , agentDir).degree() < 130)
                {
                    if( checkDir.th().degree() - agentDir.th().degree() < 0)
                        leftClean = false;
                    else
                        rightClean = false;
                }
            }
        }

        for(int i=0;i< wm->opp.activeAgentsCount();i++)
        {
            if(wm->opp.active(i)->isBallOwner(0.12,CRobot::robot_radius_old*2))
                notBlockChipIt = false;
        }
        if(notBlockChipIt && backClean)
        {
            debug("notblockchip", D_SEPEHR, "cyan");
            kick->setAgent(agent);
            kick->setChip(true);
            if( frontSupportExist)
                kick->setKickSpeed(kick->getAgent()->chipDistanceValue(1.05 , true));
            else
                kick->setKickSpeed(kick->getAgent()->chipDistanceValue(0.75 , true));
            kick->setAutoChipSpeed(false);
            kick->setTolerance(1.0);
            kick->setDontKick(false);
            kick->execute();
            kick->setChip(false);
            return true;
        }
        else if( oppBallOwner != -1)
        {
            debug("ballOwner",D_SEPEHR);
            //            if( wm->ball->pos.dist( wm->field->ourGoal()) < 2.1)
            //            {
            if(wm->opp[oppBallOwner]->isBallOwner(0.18))
            {
                knowledge->setSupporPlaymaker(CKnowledge::Back);
                if( agent->pos().dist(initialPoint) < 0.6 && wm->ball->pos.x > -1.2 )
                {
                    debug("front",D_SEPEHR,"blue");
                    knowledge->setSupporPlaymaker(CKnowledge::Front);
                }
                else if( wm->ball->pos.y > 0 && wm->opp[oppBallOwner]->isBallOwner(0.045))
                {
                    debug("right",D_SEPEHR,"green");
                    knowledge->setSupporPlaymaker(CKnowledge::Right);
                }
                else if( wm->ball->pos.y < 0 && wm->opp[oppBallOwner]->isBallOwner(0.045))
                {
                    debug("left",D_SEPEHR,"pink" );
                    knowledge->setSupporPlaymaker(CKnowledge::Left);
                }
                debug(QString("right : %1 letf : %2").arg(rightSupporterExist).arg(leftSupporterExist),D_SEPEHR);

                if( agent->self()->isBallOwner(0.009) && rightSupporterExist && wm->ball->pos.y > 0)
                {
                    debug("right micharkham :D",D_SEPEHR,"green");
                    //                    turn->setAgent(agent);
                    //					turn->setDynamic(90,0,0);
                    //					turn->setTurnMode(CSkillTurn::Dynamic);
                    //                    turn->setDirection(agent->dir().rotatedVector(-170));
                    //                    turn->execute();
                    agent->setRobotVel(0,0,-90);
                    return true;
                }
                else if( agent->self()->isBallOwner(0.009) && leftSupporterExist && wm->ball->pos.y < 0 )
                {
                    debug("left micharkham :D",D_SEPEHR,"green");
                    //                    turn->setAgent(agent);
                    //					turn->setDynamic(900,0,0);
                    //					turn->setTurnMode(CSkillTurn::Dynamic);
                    //                    turn->setDirection(agent->dir().rotatedVector(170));
                    //                    turn->execute();
                    agent->setRobotVel(0,0,90);
                    return true;
                }
                //              else
                //				{
                //					//Pushing :
                //					debug("PUSHING",D_ERROR,"purple");
                //					if ( agent->self()->isBallOwner(0.03))
                //					{
                //						gotoball->setAgent(agent);
                //						gotoball->setStruggle(true);
                //						gotoball->execute();
                //					}
                //				}
            }
            if( wm->ball->pos.dist( wm->field->ourGoal()) < 1.8)
                knowledge->setSupporPlaymaker(CKnowledge::Front);
            //			//            }
            //			else{
            //				//Pushing :

            //				debug("pushing ?!",D_ERROR,"red");

            //				if ( agent->self()->isBallOwner(0.03))
            //				{
            //					gotoball->setAgent(agent);
            //					gotoball->setStruggle(true);
            //					gotoball->execute();
            //				}


            //				//                if( wm->ball->pos.y > 0 )
            //				//                {
            //				//                    debug("right",D_SEPEHR,"green");
            //				//                    knowledge->setSupporPlaymaker(CKnowledge::Right);
            //				//                    agent->addRobotVel(0.0, 0.0, 90.0 );
            //				//                }
            //				//                else if( wm->ball->pos.y < 0 )
            //				//                {
            //				//                    debug("left",D_SEPEHR,"pink");
            //				//                    knowledge->setSupporPlaymaker(CKnowledge::Left);
            //				//                    agent->addRobotVel(0.0, 0.0, -90.0 );
            //				//                }
            //			}
        }
    }
    else
    {
        draw("Spin 2",Vector2D(0,0),"pink",50);
        //        gotoball->setAgent(agent);
        //        gotoball->setSpin(true);
        //        gotoball->setSlow(false);
        //        gotoball->setStruggle(true);
        //        gotoball->setGoal(wm->field->oppGoal());// kick->getTarget());
        //        if(oppBallOwner!=-1)
        //        {
        //            gotoball->setGoal(wm->opp[oppBallOwner]->pos);
        //        }
        //        gotoball->execute();
        kick->setAgent(agent);
        kick->setDontKick(true);
        double w1;
        QList<int> rel;
        rel.append(agent->id());
        kick->setTarget(knowledge->getEmptyPosOnGoal(wm->ball->pos, w1, true, rel, rel, 0.6));
        draw(Circle2D(kick->getTarget(), 0.3),0,190, "purple", true);
        kick->setSlow(false);
        kick->setInterceptMode(true);
        kick->setSagMode(false);
        kick->setChip(false);
        kick->setKickSpeed(agent->kickSpeedValue(7.2,0));
        kick->execute();
        kick->setDontKick(false);

        //Pushing :

        //		if ( agent->self()->isBallOwner(0.03))
        //		{
        //			gotoball->setAgent(agent);
        //			gotoball->setStruggle(false);
        //			gotoball->execute();
        //		}
    }
    return true;
}

void CRolePlayMake::passShootNew()
{	
    if (knowledge->getGameMode() != CKnowledge::OurIndirectKick && knowledge->getGameState() != CKnowledge::OurIndirectKick){
        knowledge->variables["passtopoint"] = "false";
        knowledge->variables["passpointdef"] = "false";
        knowledge->variables["passpointsw"] = "false";
    }
    //fixed pass
    if (knowledge->variables["passtopoint"]=="true")
    {
        fixedPass->agents.clear();
        fixedPass->agents.append(agent);
        fixedPass->setFixedTarget(knowledge->fixedPassPoint);
        fixedPass->setSlowKick(true);
        fixedPass->execute();
        for (int i=0;i<passreceivers.count();i++)
            passreceivers[i]->idle = false;
        agent->idle = false;
        return;
    }
    if (knowledge->variables["passpointdef"]=="true")
    {
        fixedPass->agents.clear();
        fixedPass->agents.append(agent);
        fixedPass->setFixedTarget(knowledge->fixedPassPointDef);
        fixedPass->setSlowKick(true);
        fixedPass->execute();
        for (int i=0;i<passreceivers.count();i++)
            passreceivers[i]->idle = false;
        agent->idle = false;
        return;
    }
    if (knowledge->variables["passpointsw"]=="true")
    {
        fixedPass->agents.clear();
        fixedPass->agents.append(agent);
        fixedPass->setFixedTarget(knowledge->fixedPassPointsw);
        fixedPass->setSlowKick(true);
        fixedPass->execute();
        for (int i=0;i<passreceivers.count();i++)
            passreceivers[i]->idle = false;
        agent->idle = false;
        return;
    }

    for (QMap<int, double>::iterator i=passProbs.begin();i!=passProbs.end();i++)
    {
        draw(QString("%1").arg(i.value(), 0, 'g', 2), knowledge->getAgent(i.key())->pos() + Vector2D(0.0, -0.15), "blue", 13);
    }
    double t = CProfiler::getTime();
    bool onetouching = false;
    if (currentBehaviour != NULL)
    {
        if (currentBehaviour->getName() == "kick")
        {
            if (static_cast<CBehaviourKick*> (currentBehaviour)->onetouching != -1)
            {
                onetouching = true;
            }
        }
    }
    bool redecide = false;
    if (onetouching)
    {
        if ((agent != lastAgent) || (t - lastDecisionTime > 3.0) || (currentBehaviour == NULL) || (knowledge->getGameStateChanged()) || ( currentBehaviour->getName() == "kick" && currentBehaviour->probability() < 0.3 ))
        {
            redecide = true;
        }
    }
    else {
        if (knowledge->getGameState() == CKnowledge::OurKickOff)
        {
            if ((agent != lastAgent) || (t - lastDecisionTime > 3.0) || (currentBehaviour == NULL) || (knowledge->getGameStateChanged())||( currentBehaviour->getName() == "kick" && currentBehaviour->probability() < 0.3 ))
            {
                redecide = true;
            }
        }
        else {
            if ((agent != lastAgent) || (t - lastDecisionTime > 3.0) || (currentBehaviour == NULL) || (knowledge->getGameStateChanged())||( currentBehaviour->getName() == "kick" && currentBehaviour->probability() < 0.3 ))
            {
                redecide = true;
            }
        }
    }

    if ((knowledge->getTechnicalMode().isEmpty()) &&
            (
                (knowledge->getGameMode()==CKnowledge::Start
                 &&
                 (wm->field->isInOurPenaltyArea( (wm->ball->pos-wm->field->ourGoal())*0.7 + wm->field->ourGoal() )
                  || wm->field->isInOurPenaltyArea( (wm->ball->predict(0.15)-wm->field->ourGoal())*0.7 + wm->field->ourGoal() )
                  || wm->field->isInOurPenaltyArea( (wm->ball->predict(0.3)-wm->field->ourGoal())*0.7 + wm->field->ourGoal() )
                  || wm->field->isInOurPenaltyArea( (wm->ball->predict(0.45)-wm->field->ourGoal())*0.7 + wm->field->ourGoal() )
                  || wm->field->isInOurPenaltyArea( (agent->pos()))
                  || wm->field->isInOurPenaltyArea( (agent->pos()+agent->vel()*0.15))
                  || wm->field->isInOurPenaltyArea( (agent->pos()+agent->vel()*0.3))
                  || wm->field->isInOurPenaltyArea( (agent->pos()+agent->vel()*0.45))
                  || wm->field->isInOurPenaltyArea( Vector2D(agent->pos()-wm->field->ourGoal())*0.7+wm->field->ourGoal())
                  || (knowledge->frameCount - knowledge->defenseClearingFrame < 9))

                 ))
            )
    {
        gotopoint->setAgent(agent);
        Vector2D qq = wm->ball->pos - wm->field->ourGoal();
        qq = qq.norm();
        bool rr = false;
        if (qqHist == 0)
        {
            if (qq.y > 0)
            {
                rr = false;
                qqHist = 1;
            }
            else
            {
                rr = true;
                qqHist = 2;
            }
        }
        else {
            if (qq.y > 0.1)
            {
                rr = false;
                qqHist = 1;
            }
            else if (qq.y < -0.1)
            {
                rr = true;
                qqHist = 2;
            }
            else {
                if (qqHist == 1)
                {
                    rr = true;
                }
                else if (qqHist == 2)
                {
                    rr = false;
                }
            }
        }
        if (rr)
        {
            qq = qq.rotatedVector(30);
        }
        else {
            qq = qq.rotatedVector(-30);
        }
        //            qq.y = -qq.y;
        qq = wm->field->ourGoal() + qq*1.5;
        gotopoint->init(qq, (qq-wm->field->ourGoal()).norm());
        draw(Circle2D(qq, 0.3), "red", false);
        debug("Naro tu un sagmassab!!", D_ERROR);
        gotopoint->execute();
    }
    else {
        if (redecide || forceRedecide)
        {
            forceRedecide = false;
            lastDecisionTime = t;
            //create a new decision
            for (int i=0;i<PASS_BEHAVS;i++)
            {
                pass[i]->agents.clear();
                pass[i]->agents.append(agent);
                pass[i]->setSlowKick(indirect || kickoff || direct);
                pass[i]->setChipPass(false);
                pass[i]->setShadowPass(false);
                if (khers) {
                    pass[i]->setNoChip(false);
                    pass[i]->setNoKickProb(true);
                }
                else {
                    pass[i]->setNoChip(false);
                    pass[i]->setNoKickProb(false);
                }
                chippass[i]->agents.clear();
                chippass[i]->agents.append(agent);
                chippass[i]->setSlowKick(indirect || kickoff || direct);
                spinPass[i]->agents.clear();
                spinPass[i]->agents.append(agent);
            }
            passreceivers.clear();
            passreceivers.append(knowledge->roleAssignments["position"]);
            //			for (int i=0;i<passreceivers.count();i++)
            //                if ( passreceivers.at(i)->vel().length() > 1.2)
            //				{
            //					passreceivers.removeAt(i);
            //					i--;
            //				}
            int p = 0;
            for (int i=0;i<passreceivers.count();i++)
            {
                pass[p]->agents.append(passreceivers[i]);
                chippass[p]->agents.append(passreceivers[i]);
                spinPass[p]->agents.append(passreceivers[i]);
                p++;
            }
            passProbs.clear();
            for (int i=0;i<passreceivers.count();i++)
            {
                if (!pass[i]->agents[1]->inOutState)
                {
                    passProbs.insert(pass[i]->agents[1]->id(), pass[i]->probability()*2.0);
                }
            }
            //todo: check defenders
            shoot->agents.clear();
            shoot->agents.append(agent);
            //			shoot->setSlowKick( direct || kickoff);
            shoot->setSlowKick(false);
            shoot->setChipToGoal(false);


            chipToGoal->agents.clear();
            chipToGoal->agents.append(agent);
            chipToGoal->setSlowKick( direct || kickoff);
            chipToGoal->setChipToGoal(true);

            behaviours.clear();
            for (int i=0;i<p;i++)
            {
                behaviours.append(pass[i]);
                //            behaviours.append(chippass[i]);
            }
            if ((knowledge->roleAssignments["position"].count() == 0) && (knowledge->getGameMode()==CKnowledge::OurIndirectKick))
            {
                if (!noKick) behaviours.append(chipToGoal);
            }
            else if (knowledge->getGameMode()!=CKnowledge::OurIndirectKick)
            {
                if (!noKick)
                {
                    behaviours.append(shoot);
                    if (knowledge->getGameMode() == CKnowledge::OurKickOff)
                        behaviours.append(chipToGoal);
                }
            }
            //			selectBehaviour();

            int bestPass = -1, bestChipPass= -1, bestSpinPass = -1;
            double bPass = -1, bChip = -1, bspin = -1.0;
            for( int i = 0; i < p; i++)
            {
                if ( pass[i]->probability() > bPass)
                {
                    bPass = pass[i]->probability();
                    bestPass = i;
                }
                if ( chippass[i]->probability() > bChip)
                {
                    bChip = chippass[i]->probability();
                    bestChipPass = i;
                }
                if ( spinPass[i]->probability() > bspin)
                {
                    bspin = spinPass[i]->probability();
                    bestSpinPass = i;
                }
            }

            draw(QString("Shoot : %1 Pass : %2 ChipPass : %3 ChipGoal : %4").arg(shoot->probability()).arg(pass[bestPass]->probability()).arg(chippass[bestChipPass]->probability()).arg(chipToGoal->probability()),Vector2D(-2.1,-2.1),"magenta",12);
            if( indirect && bestPass!=-1)
                currentBehaviour = pass[bestPass];
            else if( shoot->probability() > 0.18 && !indirect)
            {
                currentBehaviour = shoot;
            }
            //			else if ( bestSpinPass != -1 && spinPass[bestPass]->probability() > 0.3)
            //			{
            //				currentBehaviour = spinPass[bestSpinPass];
            //			}
            else if ( bestPass != -1 && pass[bestPass]->probability() > 0.3)
            {
                currentBehaviour = pass[bestPass];
            }
            else if ( bestChipPass != -1 &&chippass[bestChipPass]->probability() > 0.3)
            {
                currentBehaviour = chippass[bestChipPass];
            }
            else
            {
                currentBehaviour = chipToGoal;
            }
            if (kickoff && (currentBehaviour != shoot))
            {
                if (bestPass != -1)
                {
                    if (pass[bestPass]->probability() < 0.7)
                    {
                        currentBehaviour = chipToGoal;
                    }
                }
                if (bestChipPass != -1)
                {
                    if (chippass[bestChipPass]->probability() < 0.7)
                    {
                        currentBehaviour = chipToGoal;
                    }
                }

            }

        }
        if (currentBehaviour != NULL)
        {
            for (int i=0;i<passreceivers.count();i++)
                passreceivers[i]->idle = true;
            agent->idle = true;
            currentBehaviour->updateIdleStatus(); //unidle involving agents
            currentBehaviour->execute();
            draw(QString("%1 : %2").arg(currentBehaviour->getName()).arg(currentBehaviour->probability(), 0 ,'g', 2), Vector2D(1.0,-2.2), "pink", 12);
        }
        else {
            debug("Playmaker riiiide", D_ERROR);
        }
    }
    return;
}


void CRolePlayMake::executeDefault()
{
    cyclesExecuted++;
    if ( cyclesExecuted < cyclesToWait )
    {
        draw("waiting",Vector2D(0,-2));
        agent->waitHere();
        return;
    }

    clear = false;
    kick->setClear(clear);
    slow = false;
    kick->setSlow(indirect || direct);
    if( nostop)
        kick->setSlow(false);
    kick->setTurn(false);
    stopped = knowledge->getGameMode()==CKnowledge::Stop;
    if (nostop) stopped = false;
    if (stopped) passEval.reset();
    if (agent != lastAgent)
    {
        onetouching = -1;
    }
    oppBlockers.clear();
    target = knowledge->onetouchablity(agent->id(), w, ang, coming);

    if (((agent->distToBall().length() < 0.5) || (coming<-0.7)) && agent->id() == passEval.receivingAgent())
        passEval.receivedPass();


    if (!stopped)
    {
        if (spinBack()) {
            if( wm->field->ourGoal().dist(wm->ball->pos) > 1.5)
                knowledge->variables["spinning"] = "true";
            else
                knowledge->variables["spinning"] = "false";
        }
        else
        {
            initialPoint.invalidate();
            knowledge->variables["spinning"] = "false";
            double vis = 0;
            knowledge->goalVisiblity(agent->id(), vis, 1.0);
            passShootNew();
        }
    }
    else {
        stopBehindBall();
        knowledge->variables["spinning"] = "false";
    }

    lastAgent = agent;
}

void CRolePlayMake::stopBehindBall(bool penalty)
{
    knowledge->gameStarter = agent;
    if (passReceiver == -2)
    {
    }
    if( penalty)
    {
        if( knowledge->getGameState() == CKnowledge::Stop ){
            debug("stop, reset changeDirPenaltyStriker flag", D_FATEMEH);

        }

        //		draw("stopped !!!",Vector2D(0,0),"black",60);
        gotopoint->setAgent(agent);
        //		gotopoint->setPlan2(true);
        if(wm->gs->penalty_shootout())
            gotopoint->init(wm->ball->pos + (wm->ball->pos - wm->field->oppGoal())*0.03,wm->ball->pos - agent->pos());
        else{
            Vector2D direction, position;

            direction = wm->ball->pos - agent->pos();
            direction.y*=1.2;
            position = wm->ball->pos + (wm->ball->pos - wm->field->oppGoal() + Vector2D(0,0.2)).norm()*(0.13);
            gotopoint->init(position, direction);
        }

        //        gotopoint->setTargetLook( wm->ball->pos + Vector2D( wm->ball->pos - wm->field->oppGoalL()).norm()*0.15 , wm->field->oppGoalR());

        gotopoint->setSlowMode(true);
        gotopoint->setNoAvoid(false);
        gotopoint->setPenaltyKick(true);
        gotopoint->setAvoidPenaltyArea(false);
        gotopoint->setAvoidCenterCircle(false);

        gotopoint->setBallObstacleRadius(0.2);
        gotopoint->execute();
        gotopoint->setNoAvoid(false);
        gotopoint->setSlowMode(false);

    }
    else
    {
        //		draw("stopped ?!!!",Vector2D(0,0),"orange",60);
        gotopoint->setAgent(agent);
        //        gotopoint->setFastW(false);
        Vector2D shadowPoint = wm->ball->pos + Vector2D( wm->ball->pos - wm->field->oppGoal()).norm()*0.3;
        if ( kickoffmode || kickoffWing)
            shadowPoint = wm->ball->pos + Vector2D( wm->field->oppGoal() - wm->ball->pos ).norm()*0.3;
        gotopoint->setSlowMode(true);
        gotopoint->setNoAvoid(false);
        gotopoint->setAvoidPenaltyArea(true);
        gotopoint->setAvoidCenterCircle(false);
        gotopoint->setBallObstacleRadius(4*CBall::radius);
        gotopoint->setTargetLook( shadowPoint, wm->ball->pos);
        gotopoint->execute();
    }
}

void CRolePlayMake::executeOurKickOff()
{
    slow = true;
    kickoffmode = true;

    if ( cyclesExecuted < cyclesToWait )
    {
        draw("waiting",Vector2D(0,-2));
        stopBehindBall(false);
        return;
    }

    if( kickMode == FixedShoot )
        chipToOppGoal = true;

    if( knowledge->getGameMode() == CKnowledge::Stop ){
        stopBehindBall(false);
    }
    else{
        if(policy()->OurKickOff_ChipToGoal() || chipToOppGoal){
            kick->setAgent(agent);
            double w;
            kick->setTolerance(0.05);
            kick->setTarget(knowledge->goalVisiblity(agent->id(), w, policy()->PlayMaker_UnderEstimateTheirGoalie()));

            kick->setTarget(wm->field->oppGoal()*(2.0/3.0));

            int kickSpeed;
            kickSpeed = agent->chipDistanceValue(wm->ball->pos.dist(wm->field->oppCornerL()/3.0) , true);

            kick->setKickSpeed(kickSpeed);
            kick->setSpin(false);
            kick->setChip(true);
            kick->setAutoChipSpeed(false);
            kick->setSlow(false);
            kick->execute();
        }
        else{
            if (wm->ball->inSight<=0 || !wm->ball->pos.valid() || !wm->field->isInField(wm->ball->pos)) {agent->waitHere();return;}

            Vector2D target = wm->field->oppGoal();
            int kickSpeed = agent->kickSpeedValue(5 , false);

            target = pointToPass;
            kickSpeed = agent->kickSpeedValue(agent->kickValueForDistance(target.dist(agent->pos()) , 3) , false);

            kick->setAgent(agent);
            kick->setSlow(true);
            kick->setSpin(false);
            kick->setChip(false);
            kick->setTarget(target);
            kick->setKickSpeed(kickSpeed);
            kick->setWaitFrames(0);
            kick->setTolerance(0.06);
            kick->execute();
        }
    }
}
bool CRolePlayMake::ShootPenalty(){
    double w;
    QList<int> relax,empty;
    relax.append(agent->id());
    penaltyTarget = knowledge->getEmptyPosOnGoal(agent->pos(), w, true, relax, empty);
    if(penaltyTarget.dist(wm->field->oppGoal()) < 0.1){
        penaltyTarget=knowledge->getEmptyPosOnGoalForPenalty(1.0/10.0, true, 0.06,agent);

    }
    debug(QString("goalie index :%1").arg(wm->opp.data->goalieID),D_NADIA);
    if(wm->opp[wm->opp.data->goalieID] == NULL)
        return false;
    if(Segment2D(agent->pos(),penaltyTarget).dist(wm->opp[wm->opp.data->goalieID]->pos)
            > fabs(agent->pos().x-wm->opp[wm->opp.data->goalieID]->pos.x)/4)
        return true;

    else return false;
    return false;
}

double CRolePlayMake::lastBounce(){
    return (wm->field->oppGoal().dist(agent->pos()))-0.23;
}



int CRolePlayMake::getPenaltychipSpeed(){
    Vector2D oppGoaliPos=wm->opp[wm->opp.data->goalieID]->pos;
    debug(QString("chipsepeed:%1").arg(knowledge->chipGoalPropability(true)),D_NADIA);
    if(knowledge->chipGoalPropability(true)>0.1){

        return (knowledge->getProfile(agent->id(),(oppGoaliPos-agent->pos()).length(),false)
                +knowledge->getProfile(agent->id()+1,lastBounce(),false))/2;
    }
    else return -1;
}


void CRolePlayMake::firstKickInShootout(bool isChip){

    double divation=0;

    debug("first : ",D_NADIA);


    penaltyTarget=wm->field->oppGoalL()+divation*Vector2D(0,wm->field->oppGoalL().y);;
    kick->setTarget(penaltyTarget);

    if(isChip){//chip first


        if(wm->getIsSimulMode())
            kick->setKickSpeed(1);
        else
            kick->setKickSpeed(170);
        kick->setChip(true);
        if(wm->ball->vel.length()>0.4)
            firstKick=false;

    }else{//kick first


        kick->setChip(false);
        if(wm->getIsSimulMode())
            kick->setKickSpeed(1);
        else
            kick->setKickSpeed(50);
        if(wm->ball->vel.length()>0.1)
            firstKick=false;
    }
}


void CRolePlayMake::kickInitialShootout(){
    kick->setAgent(agent);
    penaltyTarget=wm->field->oppGoal();
    kick->setTarget(penaltyTarget);
    kick->setPenaltyKick(false);
    kick->setInterceptMode(false);
    kick->setChip(false);
    kick->setVeryFine(false);
    kick->setWaitFrames(0);
    kick->setTolerance(1);
    kick->setSpin(0);
}


void CRolePlayMake::ShootoutSwitching(bool isChip){

    if(wm->ball->vel.length()<0.2)
        firstKick=true;



    switch(choosePenaltyStrategy()){

    case pgoaheadShoot:
        debug("pgoahead : ",D_NADIA);
        if(agent->pos().x < 1){//agent is not ahead enough


            penaltyTarget=wm->field->oppGoalL();
            kick->setTarget(penaltyTarget);

            if(isChip){//chip first

                kick->setKickSpeed(170);
                kick->setChip(true);

            }else{//kick first

                kick->setChip(false);
                kick->setKickSpeed(50);
            }
        }
        else{ //shoot to goal

            penaltyTarget=knowledge->getEmptyPosOnGoalForPenalty(0.13,true, 10,agent);
            kick->setChip(false);
            kick->setKickSpeed(1023);
            kick->setDontKick(false);
            kick->setTarget(penaltyTarget);

        }

        break;


    case pchipShoot:
        debug("pchipshoot",D_NADIA);
        kick->setTarget(wm->field->oppGoal());
        kick->setKickSpeed(getPenaltychipSpeed());
        kick->setChip(true);
        break;

    case pshootDirect:
        debug("pdirect : ",D_NADIA);
        penaltyTarget=knowledge->getEmptyPosOnGoalForPenalty(0.13,true, 10,agent);
        kick->setTarget(penaltyTarget);
        kick->setChip(false);
        kick->setKickSpeed(1000);
        kick->setAvoidOppPenaltyArea(true);
        break;
    }
}



int CRolePlayMake::choosePenaltyStrategy(){
    if(ShootPenalty()) return pshootDirect;
    else if(getPenaltychipSpeed()!= -1) return pchipShoot;
    else if(true) return pgoaheadShoot;
    else return pgoaheadShoot;
}

void CRolePlayMake::executeOurPenaltyShootout(){

    bool chipchip=false;

    debug("penalty Shootout : ",D_NADIA);
    if (abs(wm->ball->pos.x) > 4.4)//penalty finished
        firstKick=true;

    if(wm->opp[wm->opp.data->goalieID]!= NULL )//check opp goalkeeper situation
    {
        if((wm->opp[wm->opp.data->goalieID]->pos-wm->field->oppGoal()).length()>2.5)
            goalKeeperForward=true;
    }


    if (knowledge->getGameMode()==CKnowledge::Stop)
    {//stop behind ball
        cyclesExecuted--;
        srand(time(NULL));
        stopBehindBall(true);
        penaltyCounter = 0;
        setNoKick(true);
    }
    else {      //force start

        //initial kick skill:
        kickInitialShootout();

        if(ShootPenalty())
            firstKick=false;

        if(firstKick){
            firstKickInShootout(chipchip);
        }
        else{
            ShootoutSwitching(chipchip);
        }

        kick->setShotToEmptySpot(true);
        kick->setAvoidOppPenaltyArea(false);
        kick->execute();
        draw(penaltyTarget,0,"red");
    }


}



void CRolePlayMake::executeOurPenalty()
{
    kick->setAgent(agent);
    gotopoint->setAgent(agent);

    Vector2D shift;
    Vector2D position;

    if (knowledge->getGameMode()==CKnowledge::Stop || knowledge->getGameState()==CKnowledge::Stop)
    {
        cyclesExecuted--;
        srand(time(NULL));
        stopBehindBall(true);
        penaltyTarget = knowledge->getEmptyPosOnGoalForPenalty(1.0/8.0, true, 0.03);

        changeDirPenaltyStrikerTime.restart();
        timerStartFlag = true;
    }
    else {
        penaltyTarget = knowledge->getEmptyPosOnGoalForPenalty(1.0/8.0, true, 0.03);   //////// tune
        agent->setRoller(1);

        ////////////// change robot direction before kicking //////////////
        if(timerStartFlag)
        {
            if(changeDirPenaltyStrikerTime.elapsed() < 2500){
                if(penaltyTarget.y * wm->field->oppGoalL().y < 0 && penaltyTarget.dist(wm->field->oppGoal()) > 0.25){
                    penaltyTarget.y = wm->field->oppGoalR().y*2;
                    shift = Vector2D(0,0.3);
                }
                else{
                    penaltyTarget.y = wm->field->oppGoalL().y*2;
                    shift = Vector2D(0,-0.3);
                }
                position = wm->ball->pos + (wm->ball->pos - wm->field->oppGoal() + shift).norm()*(0.13);
                gotopoint->init(position, penaltyTarget);
                gotopoint->setLookAt(wm->ball->pos);
            }
            else
            {
                timerStartFlag = false;
            }

        }

        gotopoint->setADiveMode(true);
        gotopoint->setSlowMode(false);

        kick->setTarget(penaltyTarget);
        // TODO : Fix This Speed
        // TODO : check this mhmmd
        //        kick->setKickSpeed(knowledge->getProfile(kick->getAgent()->id(),7.8 ,true, false);
        //        kick->setKickSpeed(kick->getAgent()->kickSpeedValue(7.8,false));
        //        kick->setPenaltyKick(true);
        kick->setInterceptMode(false);
        kick->setSpin(false);
        kick->setChip(false);
        kick->setVeryFine(false);
        kick->setWaitFrames(0);
        if(wm->getIsSimulMode())
            kick->setKickSpeed(7);
        else
            kick->setKickSpeed(1023);
        kick->setAvoidOppPenaltyArea(false);
        kick->setTolerance(20);
        kick->setChip(false);

        if(timerStartFlag){
            gotopoint->execute();
        }
        else{
            kick->execute();
        }
    }

    //    draw(penaltyTarget, 0, "cyan");

}


void CRolePlayMake::theirPenaltyPositioning(){
    debug("iiiin",D_NADIA);
    gotopoint->setAgent(agent);
    gotopoint->init(wm->field->oppCornerL(),wm->field->ourGoal());
    gotopoint->execute();
}


bool CRolePlayMake::canScoreGoal(){
    if( (knowledge->getGameState() == CKnowledge::OurPenaltyKick || knowledge->getGameMode() == CKnowledge::OurPenaltyKick)
            || knowledge->getGameState() == CKnowledge::OurIndirectKick
            || (knowledge->getGameState() == CKnowledge::OurKickOff && knowledge->getGameMode() == CKnowledge::Stop
                || ((knowledge->getGameState()==CKnowledge::TheirPenaltyKick
                     || knowledge->getGameMode()==CKnowledge::TheirPenaltyKick
                     || knowledge->getGameState()==CKnowledge::Start) && wm->gs->penalty_shootout()))
            ){
        return false;
    }

    double w, ang, coming;
    Vector2D targ = knowledge->onetouchablity(agent->id(), w, ang, coming);//debug(QString("id is: %1").arg(agent->id()),D_MOHAMMED);
    info()->oneToucher.removeAll(agent->self()->id);
    if(info()->oneToucher.isEmpty())
        info()->oneToucherDist2Ball = 100;
    if(agent->pos().dist(wm->ballCatchTarget(agent->self())) < info()->oneToucherDist2Ball
            &&  (agent->canOneTouch() && coming > 0.3 && ang<policy()->PlayMaker_OneTouchAngleThreshold() )
            )
    {
        Segment2D ballSeg(wm->ball->pos , wm->ball->pos+wm->ball->vel.norm());
        draw(QString("PlayMake OneTouch"),Vector2D(-2,1),"magenta",18);
        //		onetouch->setAgent(agent);
        //		onetouch->setTarget(wm->field->oppGoal());
        //		onetouch->setKickSpeed(agent->kickSpeedValue(7.8 , false));
        //		onetouch->setWaitPos(agent->pos());
        //		onetouch->setDistToBallLine(0.1);
        //		onetouch->setVelocityToBallLine(0.3);
        //		onetouch->execute();
        //		agent->intention
        shoot->agents.clear();
        shoot->agents.append(agent);
        shoot->setSlowKick(false);
        shoot->setChipToGoal(false);
        shoot->execute();
        info()->oneToucher.append(agent->self()->id);
        info()->oneToucherDist2Ball = agent->pos().dist(wm->ballCatchTarget(agent->self()));
        return true;
    }

    double region;
    QList <int> ourRelax;
    ourRelax.clear();
    ourRelax.append(wm->our.data->activeAgents);
    if( knowledge->goalie != NULL && ourRelax.contains(knowledge->goalie->self()->id)){
        ourRelax.removeOne(knowledge->goalie->self()->id);
    }
    for( int i=0 ; i<knowledge->defenseAgents.size() ; i++ ){
        if( ourRelax.contains(knowledge->defenseAgents.at(i)->self()->id) )
            ourRelax.removeOne(knowledge->defenseAgents.at(i)->self()->id);
    }
    QList <int> oppRelax;
    Vector2D target = knowledge->getEmptyPosOnGoal(wm->ball->pos , region , true , ourRelax, oppRelax);
    if( region > 0.3 ){
        kick->setAgent(agent);
        kick->setTarget(target);
        kick->setKickSpeed(kick->getAgent()->kickSpeedValue(7.8,false));
        kick->setInterceptMode(true);
        kick->setSpin(false);
        kick->setChip(false);
        kick->setSlow(false);
        kick->setWaitFrames(0);
        kick->setTolerance(0.06);
        kick->setAvoidPenaltyArea(true);
        kick->execute();
        return true;
    }
    return false;
}

void CRolePlayMake::kickPass( int kickSpeed ){
    Vector2D behindTheBall = wm->ball->pos + Vector2D( wm->ball->pos - pointToPass ).norm()*0.2;
    if( kickPassMode == KickPassFirst && agent->pos().dist(behindTheBall) > 0.01 ){
        finalTarget = wm->ball->pos;
        gotopoint->setAgent(agent);
        gotopoint->setTargetLook( behindTheBall , pointToPass );
        gotopoint->setSlowMode(true);
        gotopoint->setNoAvoid(false);
        gotopoint->setAvoidPenaltyArea(true);
        gotopoint->setAvoidCenterCircle(false);
        gotopoint->setBallObstacleRadius(0.2);
        gotopoint->execute();
    }
    else{
        kickPassMode = KickPassSecond;
        if( kickPassCyclesWait > 4 && agent->pos().dist(finalTarget) > 0.01 ){
            agent->setKick(kickSpeed+2);
            gotopoint->setAgent(agent);
            gotopoint->setTargetLook( finalTarget , pointToPass );
            gotopoint->setSlowMode(true);
            gotopoint->setNoAvoid(true);
            gotopoint->setAvoidPenaltyArea(true);
            gotopoint->setAvoidCenterCircle(false);
            gotopoint->setBallObstacleRadius(0);
            gotopoint->execute();
        }
        else
            kickPassCyclesWait++;
    }
}

void CRolePlayMake::execute()
{
    cyclesExecuted++;
    if( wm->ball->inSight <= 0
            ||	wm->ball->pos.valid() == false
            ||	wm->field->marginedField().contains(wm->ball->pos) == false ){
        agent->waitHere();
        return;
    }

    double region;
    QList <int> ourRelax;
    ourRelax.clear();
    ourRelax.append(wm->our.data->activeAgents);
    if( knowledge->goalie != NULL && ourRelax.contains(knowledge->goalie->self()->id)){
        ourRelax.removeOne(knowledge->goalie->self()->id);
    }
    for( int i=0 ; i<knowledge->defenseAgents.size() ; i++ ){
        if( ourRelax.contains(knowledge->defenseAgents.at(i)->self()->id) )
            ourRelax.removeOne(knowledge->defenseAgents.at(i)->self()->id);
    }
    QList <int> oppRelax;
    Vector2D target = knowledge->getEmptyPosOnGoal(wm->ball->pos , region , true , ourRelax, oppRelax);
    int kickSpeed = agent->kickSpeedValue(5 , false);

    if( noKick == false && canScoreGoal() ){
        return;
    }

    if( knowledge->getGameState() == CKnowledge::OurKickOff || knowledge->getGameMode() == CKnowledge::OurKickOff ){
        executeOurKickOff();

        return;
    }
    else if((knowledge->getGameState()==CKnowledge::TheirPenaltyKick
             || knowledge->getGameMode()==CKnowledge::TheirPenaltyKick
             || knowledge->getGameMode()==CKnowledge::Start) && wm->gs->penalty_shootout()){
        theirPenaltyPositioning();
        return;

    }
    else if (wm->gs->penalty_shootout()
             && (knowledge->getGameState()==CKnowledge::OurPenaltyKick
                 || knowledge->getGameMode() == CKnowledge::OurPenaltyKick)){
        debug(QString("st:%1").arg(!wm->gs->penalty_shootout()),D_NADIA);
        executeOurPenaltyShootout();
        return;
    }
    else if(knowledge->getGameState()==CKnowledge::OurPenaltyKick
            || knowledge->getGameMode() == CKnowledge::OurPenaltyKick){
        debug(QString("st___:%1").arg(!wm->gs->penalty_shootout()),D_NADIA);
        executeOurPenalty();
        return;

    }

    if ( cyclesExecuted < cyclesToWait )
    {
        stopBehindBall(false);
        return;
    }

    if( noKick )
        return;

    if( kickMode == FixedPass ){
        debug("HERE" , D_MASOOD);
        setThrough(false);
        target = pointToPass;
        kickSpeed = chip ? agent->chipDistanceValue(agent->pos().dist(target),false) : agent->kickValueForDistance(agent->pos().dist(target) , 1.5);
        kick->setSlow(false);
        kick->setChip(chip);
        kickPass(kickSpeed);
        return;
    }
    else if( kickMode == FixedShoot ){
        target = pointToShoot;
        setChip(false);
        kickSpeed = agent->kickValueForDistance(agent->pos().dist(target) , 6);
        kick->setSlow(false);
        kick->setAgent(agent);
        kick->setTarget(target);
        kick->setKickSpeed(kickSpeed);
        kick->setWaitFrames(0);
        kick->setTolerance(0.06);
        kick->setAvoidPenaltyArea(true);
        kick->execute();
        debug("HERE2" , D_MASOOD);
    }

    debug("HERE3" , D_MASOOD);
    kick->setSlow(false);
    kick->setAgent(agent);
    kick->setTarget(target);
    kick->setKickSpeed(kickSpeed);
    kick->setWaitFrames(0);
    kick->setTolerance(0.06);
    kick->setAvoidPenaltyArea(true);
    kick->execute();
}

void CRolePlayMake::parse(QStringList params)
{
    setKickToTheirDefense(false);
    setJustTurn(false);
    setSlow(false);
    setChip(false);
    setIndirectKhafan(false);
    setNoStop(false);
    setKickoffWing(false);
    setIndirectGoogooli(false);
    setChipIndirect(false);
    setThrough(false);
    setManualPassReceive(false);
    setAllowOneTouch(false);
    setAllowOnePass(false);
    setFollowSequence(false);
    setKickoffmode(false);
    setIndirectTiny(false);
    setChipInPenaltyArea(false);
    setShadowPass(false);
    setKhers(false);
    setShadowyPoint(false);
    setLongChip(false);
    setPassReceiver(-1);
    setChipToOppGoal(false);
    setSafeIndirect(false);
    setLocalAgentPassTarget("");
    setPassReceiver(-1);
    kick->setInterceptMode(false);
    for (int i=0;i<params.length();i++)
    {
        bool ok = false;
        int p = params[i].toInt(&ok);
        if (params[i].toLower()=="slow") setSlow(true);
        else if (params[i].toLower()=="chip") setChip(true);
        else if (params[i].toLower()=="indirectkhafan") setIndirectKhafan(true);
        else if (params[i].toLower()=="nostop") setNoStop(true);
        else if (params[i].toLower()=="wing") setKickoffWing(true);
        else if (params[i].toLower()=="indirectgoogooli") setIndirectGoogooli(true);
        else if (params[i].toLower()=="chipindirect") setChipIndirect(true);
        else if (params[i].toLower()=="through") setThrough(true);
        else if (params[i].toLower()=="nokick") setNoKick(true);
        else if (params[i].toLower()=="allowonetouch") setAllowOneTouch(true);
        else if (params[i].toLower()=="allowonepass") setAllowOnePass(true);
        else if (params[i].toLower()=="followsequence") setFollowSequence(true);
        else if (params[i].toLower()=="kickoff") setKickoffmode(true);
        else if (params[i].toLower()=="indirecttiny") setIndirectTiny(true);
        else if (params[i].toLower()=="chipinpenalty") setChipInPenaltyArea(true);
        else if (params[i].toLower()=="shadowpass") setShadowPass(true);
        else if (params[i].toLower()=="khers") setKhers(true);
        else if (params[i].toLower()=="kicktotheirdefense") setKickToTheirDefense(true);
        else if (params[i].toLower()=="justturn") setJustTurn(true);
        else if (params[i].toLower()=="longchip") setLongChip(true);
        else if (params[i].toLower()=="chiptogoal") setChipToOppGoal(true);
        else if (params[i].toLower()=="shadowypoint") setShadowyPoint(true);
        else if (params[i].toLower()=="safe") setSafeIndirect(true);
        else if (params[i].startsWith("@"))
        {
            localAgentPassTarget = params[i].right(params[i].length()-1);
        }
        else if (params[i].toLower()=="%") {
            setManualPassReceive(true);
            setPassReceiver(-5);
        }
        else if (ok) {
            setManualPassReceive(true);
            setPassReceiver(p);
        }
    }
}

double CRolePlayMake::progress()
{
    if (agent->self()->ballComingSpeed()<-0.6)
        return 1.01;
    return 0.0;
}

CSkillConfigWidget* CRolePlayMake::generateConfigWidget(QWidget */*parent*/)
{
    return NULL;
}

void CRolePlayMake::generateFromConfig(CAgent *a)
{
    agent = a;
}

CRolePlayMakeInfo::CRolePlayMakeInfo(QString _roleName) : CRoleInfo(_roleName)
{

}

struct DistAgent {
    CAgent* agent;
    double dist;
};

bool operator < (const DistAgent& a, const DistAgent& b)
{
    return a.dist < b.dist;
}

CAgent* CRolePlayMakeInfo::passReceiver(CAgent* self, int p, QList<int> passables)
{
    QList<DistAgent> s;
    if (p==-5)
    {
        for (int i=self->id()+1;i<self->id()+knowledge->agentCount();i++)
        {
            int k = i%knowledge->agentCount();
            if ((knowledge->getAgent(k)->isVisible()) && (knowledge->getAgent(k)->id() != self->id())
                    )
            {
                return knowledge->getAgent(k);
            }
        }
    }
    for (int i=0;i<knowledge->agentCount();i++)
    {
        if ((knowledge->getAgent(i)->isVisible()) && (knowledge->getAgent(i)->id() != self->id())
                && passables.contains(i)
                )
        {
            DistAgent a;
            a.agent = knowledge->getAgent(i);
            a.dist = (a.agent->pos() - self->pos()).length();
            s.append(a);
        }
    }
    qSort(s.begin(), s.end());
    if (p>=0 && p<s.count())
        return s[p].agent;
    return NULL;
}



CAgent* CRolePlayMakeInfo::bestPassReceiver(bool indirect)
{
    double bestW = -1.0,w;
    CAgent* best = NULL;
    //if (knowledge->getGameMode() == CKnowledge::OurDirectKick )
    for (int i=0;i<knowledge->agentCount();i++)
    {
        if ((knowledge->getAgent(i)->isVisible()) && (knowledge->getAgent(i)->skillName != CRolePlayMake::Name)
                //&& ((knowledge->getAgent(i)->canRecvPass || indirect))
                && (knowledge->getAgent(i)->skillName=="position"))
        {
            double ang = 0, coming = 0;
            knowledge->onetouchablity(knowledge->getAgent(i)->id(), w, ang, coming);
            //            knowledge->goalVisiblity(knowledge->getAgent(i)->id(), w, policy()->PlayMaker_UnderEstimateTheirGoalie());
            if ((knowledge->getAgent(i)->skillName=="position") && (knowledge->getAgent(i)->skill!=NULL))
            {
                if (static_cast<CRolePosition*> (knowledge->getAgent(i)->skill)->getPenalty())
                {
                    w *= 1.6;
                }
            }
            for(int j = 0; j < wm->opp.activeAgentsCount(); j++ )
            {
                QList<int> rs;
                rs.clear();
                if (wm->opp.active(j)->pos.dist( knowledge->getAgent(i)->pos()) <  1.0  || !knowledge->isPointClear( knowledge->getAgent(i)->pos(), wm->ball->pos, 3 * CRobot::robot_radius_old, true, rs, rs))
                    w*=0.1;
            }

            if (indirect)
                w = 1.0/(1.0+fabs(ang)/180.0);

            if (w>bestW) {bestW = w;best = knowledge->getAgent(i);}
        }
    }
    return best;
}

void CRolePlayMake::executeOurDirect()
{
    //    if (agent->self()->ballComingSpeed() < -0.6 ||
    //            fabs(knowledge->currentTime()-knowledge->getLastTimeGameStateChanged()) > 6.0 )
    //    {
    //        CKnowledge::State s = CKnowledge::Start;
    //        knowledge->setgameMode(s);
    //        slow = false;
    //        direct = false;
    //        chipPenaltyArea = false;
    //        debug("toop raft ! ",D_SEPEHR,"yellow");
    //    }

    kick->setSlow(true);


    double goal_p;
    QList<int> ourrelaxed;
    QList<int> opprelaxed;
    ourrelaxed.append(agent->self()->id);
    Vector2D tar = knowledge->getEmptyPosOnGoal(wm->ball->pos, goal_p, true, ourrelaxed, opprelaxed, 0.9, 1.0); ///calculate empty angle     //p*empty_width + (1-p)*goalwidth
    draw(Circle2D(Vector2D(0,0),1.2),0,360,"pink",true);
    draw(QString("prob : %1").arg(goal_p));
    if(goal_p > 0.3)
    {
        kick->setAgent(agent);
        kick->setTarget(tar);
        kick->setChip(false);
        kick->setKickSpeed(agent->kickSpeedValue(8.0,true));
        //        kick->setTolerance(0.3);
        kick->execute();
    }
    else if ( chipToOppGoal)
    {
        kick->setAgent(agent);
        kick->setTarget(wm->field->oppGoal());
        kick->setChip( true);
        kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist(wm->field->oppGoal())-0.3 , false));
        kick->setAutoChipSpeed(false);
        kick->setTolerance( 0.3);
        kick->execute();
        return;
    }
    else if( chipPenaltyArea)
    {
        kick->setAgent(agent);
        kick->setTarget(wm->field->oppGoal());//Vector2D(wm->field->oppPenalty().x + 0.3, wm->field->oppPenalty().y));//(wm->field->oppGoal() + wm->field->oppPenalty())/2.0);
        kick->setChip( true);
        if ( wm->ball->pos.x < 0 )  // comment in telecomp
        {
            debug("heyyyyyyyyyaaaaaaaaaaaaaaaaaaa",D_ERROR,"red");
            kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist(wm->field->oppGoal())-0.3 , false));
        }
        else
            kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist((wm->field->oppGoal()+wm->field->oppPenalty())*0.5) , false));
        kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist((wm->field->oppGoal()+wm->field->oppPenalty())*0.5) , false));
        kick->setSlow(true);
        kick->setInterceptMode(false);
        kick->setTolerance( 0.1);
        kick->execute();
        return;
    }
    else slow = true;
    double w;
    if (policy()->OurDirect_ChipToGoal() ||
            (policy()->OurDirect_ChipToGoalInOurField()
             && CGameConditions::check("ballinside",QStringList() << "field1stquarter")))
    {
        draw("Chip it: direct", Vector2D(-1,-1), "black");
        kick->setAgent(agent);
        kick->setTolerance(0.2);
        kick->setTarget(knowledge->goalVisiblity(agent->id(), w, 1.0));
        kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist((wm->field->oppGoal()+wm->field->oppPenalty())*0.5) , true));
        kick->setSpin(true);
        kick->setChip(true);
        kick->setAutoChipSpeed(false);
        kick->setSlow(true);
        kick->execute();
        /*TEMP*/gotoball->resetI();
        return;
    }
    else {
        executeDefault();
    }
}

void CRolePlayMake::executeOurIndirect()
{

    if ( indirectTiny)
    {
        if( !knowledge->executingPlays.isEmpty())
        {
            if ( knowledge->executingPlays.at(0) == "ourindirecttinyplus")
            {
                Rect2D checkRectTiny(Vector2D( wm->ball->pos.x, wm->ball->pos.y + 1.5), Vector2D( wm->ball->pos.x + 0.6, wm->ball->pos.y));
                for ( int i = 0; i < wm->opp.activeAgentsCount(); i++)
                {
                    if( checkRectTiny.contains(wm->opp[i]->pos))
                    {
                        indirectTiny = false;
                        indirect = true;
                        return;
                    }
                }
            }
        }
        debug("tinying",D_SEPEHR);
        kick->setAgent(agent);
        if ( wm->ball->pos.y > 0 )
            kick->setTarget( Vector2D(wm->ball->pos.x - 0.4, wm->ball->pos.y - 0.2));
        else
            kick->setTarget( Vector2D(wm->ball->pos.x - 0.4, wm->ball->pos.y + 0.2));

        for( int i =0; i < knowledge->agentCount();i++)
        {
            if (knowledge->getAgent(i)->skill != NULL)
            {
                if (knowledge->getAgent(i)->skill->getName() == "position")
                {
                    if( static_cast<CRolePosition*>( knowledge->getAgent(i)->skill)->getIndirectTiny())
                        kick->setTarget(knowledge->getAgent(i)->self()->getKickerPos(0.01));
                }
            }
        }
        kick->setSlow(true);
        kick->setKickSpeed(agent->kickSpeedValue(2.7, true));
        kick->setChip(false);
        kick->setTolerance(0.1);
        kick->setSpin(true);
        kick->execute();
        /*TEMP*/gotoball->resetI();
        if ((agent->self()->ballComingSpeed() < -0.45))/// ||
            //fabs(knowledge->currentTime()-knowledge->getLastTimeGameStateChanged()) > 6.0 )
        {
            //            CKnowledge::State s = CKnowledge::Start;
            //            knowledge->setgameMode(s);
            //            debug("toop raft ! ",D_SEPEHR,"orange");
            indirect = false;
            indirectTiny = false;
            slow = false;
            kick->setInterceptMode(true);
            executeDefault();
        }
        return;
    }
    else if ( indirectKhafan)
    {
        kick->setAgent(agent);
        kick->setSlow(true);
        kick->setTolerance(0.1);
        kick->setChip(true);
        kick->setSpin(true);
        bool chipIt=false;
        if ((policy()->OurIndirect_ChipToGoalInOurField() && knowledge->getGameMode()==CKnowledge::OurIndirectKick)
                ||
                (policy()->OurDirect_ChipToGoalInOurField() && knowledge->getGameMode()==CKnowledge::OurDirectKick)
                )
        {
            if (CGameConditions::check("ballinside",QStringList() << "field1stquarter"))
            {
                chipIt = true;
            }
        }
        kick->setAgent(agent);
        kick->setTarget((wm->field->oppGoal()-agent->pos()).norm()*1.5 + agent->pos());
        if (true/*info()->whoIsKhafan()==NULL*/ || chipIt)
        {
            kick->setTarget(wm->field->oppPenalty());
            kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist(wm->field->oppPenalty()) , true));
            //            kick->setAutoChipSpeed(true);
        }
        else
        {
            //			bool blocked = false;
            //			CAgent* khowf = info()->whoIsKhafan();
            //			for (int i=0;i<wm->opp.activeAgentsCount();i++)
            //			{
            //				if ((khowf->pos()-wm->opp.active(i)->pos).length() < 0.7)
            //				{
            //					if (Vector2D::angleBetween(wm->opp.active(i)->pos-khowf->pos(), wm->field->oppGoal() - khowf->pos()).abs()<30)
            //					{
            //						blocked = true;
            //					}
            //				}
            //			}
            //                        if (blocked)
            //			{
            //				draw("Chip it longer", Vector2D(-2,-1), "black", 32);
            //				kick->setTarget((wm->field->oppGoal()-agent->pos()).norm()*1.8 + agent->pos());
            //				kick->setKickSpeed(9); //KhafanChip
            //				//                kick->setAutoChipSpeed(true);
            //			}

            if (knowledge->variables["khafanmarked"]=="true")
            {
                draw("Chip it longer", Vector2D(-2,-1), "black", 32);
                kick->setTarget((wm->field->oppGoal()-agent->pos()).norm()*2.1 + agent->pos());
                kick->setKickSpeed(kick->getAgent()->chipDistanceValue(2.1, false)); //KhafanChip to be tuned
                //                kick->setAutoChipSpeed(true);
            }
            else {
                draw("Chip it khafan", Vector2D(-2,-1), "black", 32);
                kick->setKickSpeed(kick->getAgent()->chipDistanceValue(0.9 , false)); //KhafanChip
                //                kick->setAutoChipSpeed(true);
            }
        }
        kick->setSlow(true);
        kick->setChip(true);
        kick->setSpin(false);
        kick->execute();
        if ((agent->self()->ballComingSpeed() < -0.45))// ||
            //                fabs(knowledge->currentTime()-knowledge->getLastTimeGameStateChanged()) > 6.0 )
        {
            //            CKnowledge::State s = CKnowledge::Start;
            //            knowledge->setgameMode(s);
            //            debug("toop parid ! ",D_SEPEHR,"orange");
            indirect = false;
            indirectKhafan = false;
            slow = false;
            kick->setInterceptMode(true);
            executeDefault();
        }
        return;
    }
    else if( chipPenaltyArea)
    {
        kick->setAgent(agent);
        double ofset;
        if ( wm->ball->pos.y < 0)
            ofset = 0.6;
        else
            ofset = -0.6;
        kick->setTarget(wm->field->oppPenalty());//Vector2D(Vector2D((wm->field->oppGoal() + wm->field->oppPenalty())/2.0).x,Vector2D((wm->field->oppGoal() + wm->field->oppPenalty())/2.0).y+ofset));
        kick->setChip( true);
        kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist(wm->field->oppPenalty())-0.3 , false));
        if ( wm->ball->pos.x < 0 )  // comment in telecomp
        {
            debug("heyyyyyyyyyaaaaaaaaaaaaaaaaaaa",D_ERROR,"red");
            kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist(wm->field->oppPenalty())-0.3 , false));
        }
        //kick->setKickSpeed( 22);
        else
            kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist(wm->field->oppPenalty()) , false));
        kick->setAutoChipSpeed(false);
        kick->setTolerance( 0.1);
        kick->execute();
        if ((agent->self()->ballComingSpeed() < -0.45))// ||
            //                fabs(knowledge->currentTime()-knowledge->getLastTimeGameStateChanged()) > 6.0 )
        {
            //            CKnowledge::State s = CKnowledge::Start;
            //            knowledge->setgameMode(s);
            //            debug("toop parid ! ",D_SEPEHR,"orange");
            indirect = false;
            chipPenaltyArea = false;
            slow = false;
            kick->setInterceptMode(true);
            executeDefault();
        }
        return;
    }
    else if ( longchip)
    {
        kick->setAgent(agent);
        if (wm->ball->pos.y > 0)
            kick->setTarget(wm->field->oppGoalR());
        //			kick->setTarget( wm->field->OppPenalty);
        else
            kick->setTarget(wm->field->oppGoalL());
        //        kick->setTarget( wm->field->oppGoal());
        kick->setChip( true);
        kick->setSlow(true);
        // long chip dist
        kick->setKickSpeed( agent->chipDistanceValue(wm->ball->pos.dist(wm->field->oppPenalty()),true));
        kick->setSpin(true);
        kick->setTolerance( 0.3);
        kick->execute();
        if ( wm->ball->vel.length() > 0.3)
            knowledge->setRushInPenaltyArea( true);
        return;
    }
    else if ( chipToOppGoal)
    {
        kick->setAgent(agent);
        kick->setTarget(wm->field->oppGoal());
        kick->setSlow(true);
        kick->setChip( true);
        kick->setKickSpeed(kick->getAgent()->chipDistanceValue(wm->ball->pos.dist(wm->field->oppGoal()) , false));
        kick->setAutoChipSpeed(false);
        kick->setTolerance( 0.3);
        kick->execute();
        return;
    }
    else if (kickToTheirDefense)
    {
        kickToDefense->agents.clear();
        kickToDefense->agents.append(agent);
        kickToDefense->execute();
        return;
    }
    else if (justTurn)
    {
        gotoball->setAgent(agent);
        gotoball->setSlow(false);
        gotoball->setRotate(true);
        gotoball->setThroughMode(false);
        gotoball->setFinalVel(0);
        gotoball->setGoal(wm->field->oppGoal());
        for (int i=0;i<knowledge->agentCount();i++)
        {
            if (knowledge->getAgent(i)->localName == localAgentPassTarget)
            {
                if (knowledge->getAgent(i)->isVisible())
                {
                    gotoball->setGoal(knowledge->getAgent(i)->pos());
                }
            }
        }
        gotoball->execute();
        return;
    }


    debug("indirect ! ",D_SEPEHR,"orange");
    //    if ((agent->self()->ballComingSpeed() < -0.45) ||
    //            fabs(knowledge->currentTime()-knowledge->getLastTimeGameStateChanged()) > 6.0 )
    //    {
    //           debug("here ?1",D_SEPEHR);
    //        CKnowledge::State s = CKnowledge::Start;
    //        knowledge->setgameMode(s);
    //        debug("toop raft ! ",D_SEPEHR,"orange");
    //        indirect = false;
    //        indirectTiny = false;
    //        indirectGoogooli = false;
    //        indirectKhafan = false;
    //        slow = false;
    //    }
    //    else
    {
        indirect = true;
        slow = true;
    }
    if (indirect)
    {
        debug ("execute our indirect", D_ALI, "red");
    }
    if (policy()->OurIndirect_NoPass() || policy()->OurIndirect_ChipToGoal() || chipIndirect)
    {

        kick->setAgent(agent);
        double w;
        kick->setTolerance(0.2);
        kick->setTarget(knowledge->goalVisiblity(agent->id(), w, 1.0));
        Vector2D hoyBechipInja;
        if ( chipIndirect){
            //			if ( wm->ball->pos.y > 0)
            hoyBechipInja = Vector2D(wm->field->oppGoal().x - 0.30, 0);
            kick->setTarget(hoyBechipInja) ;

        }
        kick->setKickSpeed(agent->chipDistanceValue(wm->ball->pos.dist(hoyBechipInja) - 0.3, true));
        kick->setSpin(false);
        kick->setChip(policy()->OurIndirect_ChipToGoal() || chipIndirect);
        kick->setAutoChipSpeed(false);
        //        kick->setTurn(true);
        kick->setSlow(true);
        bool waitForPositioners = chipIndirect;
        //		if ( chipIndirect && (knowledge->cornerChipPhase & 1 || knowledge->cornerChipPhase & 2))
        //			waitForPositioners = false;
        if ( chipIndirect && (knowledge->cornerChipPhase & 8 || knowledge->cornerChipPhase & 4))
            waitForPositioners = false;
        if (!waitForPositioners)
            kick->execute();
        else
            stopBehindBall();
        /*TEMP*/gotoball->resetI();
        knowledge->chipPassSender = agent->id();
        knowledge->sentChipPass = true;
    }
    else if ( safeIndirect && knowledge->currentTime() - knowledge->getLastTimeGameStateChanged()< 3.6)
    {
        bool waitforP2 = false;
        if ( safeIndirect && knowledge->currentTime() - knowledge->getLastTimeGameStateChanged()< 3.6)
            waitforP2 = true;
        if( waitforP2)
            agent->waitHere();
    }
    else {
        executeDefault();
    }
}

void CRolePlayMake::resetOffPlays()
{
    indirect = false;
    setIndirectTiny( false);
    setIndirectGoogooli(false);
    setIndirectKhafan(false);
    setChipInPenaltyArea(false);
    setChipIndirect(false);
    direct = false;
    setChipInPenaltyArea(false);
}

void CRolePlayMake::resetPlayMake(){
    cyclesToWait = 0;
    cyclesExecuted = 0;
    kickPassMode = KickPassFirst;
    kickPassCyclesWait = 0;
}
