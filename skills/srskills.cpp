#include "srskills.h"

INIT_SKILL(SRSkillGotoPoint, "crgotopoint");

SRSkillGotoPoint::SRSkillGotoPoint(CAgent *_agent) : CSkill(_agent)
{
    speedPidX = new _PID(1,0,0,0,0);
    speedPidY = new _PID(1,0,0,0,0);
    posPid = new _PID(1,0,0,0,0);
    angPid= new _PID(2,0,0,0,0);
    //_VmReal = 4;
    _Vf = 0;
    _Acc = 4.5;
    _Dec = -4;
    slow = false;

    GPMode = GPACC1;
    kkPosDist = 0.5;

    kkThrPos = 0;
    kkThrVConst = 0;
    _VmDesire = 4;

    kkVDesire = 0;

    agentId = -1;
}

void SRSkillGotoPoint::init(int _id, Vector2D _target, Vector2D _targetDir, Vector2D _targetVel, bool dynamicStart)
{
    agentId = _id;
    targetPos = _target;
    FinalDir = _targetDir;
    targetDir = FinalDir.th();
    targetVel = _targetVel;

}

SRSkillGotoPoint::~SRSkillGotoPoint()
{
    delete speedPidX;
    delete speedPidY;
    delete posPid;
    delete angPid;
}

kkGPMode SRSkillGotoPoint::decideMode()
{
    if(kkDist < kkPosDist + kkThrPos)
    {
        kkThrPos = 0.25;
        return GPPOS;
    }
    else
    {
        kkThrPos = 0;
        if(kkVmaxDesire < kkVm)
        {
            kkVmax = kkVmaxDesire;
            if( kkVmax - kkThrVConst <= kkVc )
            {
                kkThrVConst = 0.15;
                return GPVCONST;
            }
            else
            {
                kkThrVConst = 0;
                kkX3 = (  kkVf*kkVf - kkVmax*kkVmax )/( 2*kkDec );
                if(kkDist < kkX3 *1.07)
                    return GPDEC1;
                else
                    return GPACC1;
            }
        }
        else
        {
            kkVmax = kkVm;
            kkX3 = (  kkVf*kkVf - kkVmax*kkVmax )/( 2*kkDec );
            if(kkDist < kkX3 *1.07)
                return GPDEC2;
            else
                return GPACC2 ;
        }
    }
}

void SRSkillGotoPoint::execute()
{
    if(agentId == -1)
        return;

    CRAgent = knowledge->getAgent(agentId);

    kkVf = targetVel.length();
    if(slow)
    {
        kkAcc = _Acc/1.3;
        kkDec = _Dec/1.3;
        kkVmaxDesire = 1.7;
    }
    else
    {
        kkAcc = _Acc;
        kkDec = _Dec;
        kkVmaxDesire = _VmDesire;
    }

    kkMovementTh = (targetPos-CRAgent->pos()).th();

    angPid->error = (targetDir-CRAgent->dir().th()).radian();
    angPid->kp=2;
    angPid->kd=0;
    angPid->ki=0;
    speedPidX->kp = 3;
    speedPidY->kp =3;

    if(wm->getIsSimulMode())
        kkVc = CRAgent->vel().length();
    else
        kkVc = CRAgent->vel().length()/2;

    kkDist = CRAgent->pos().dist(targetPos) ;


    kkVm = fabs(  (  (kkDec-kkAcc)*kkVf + sqrt( (kkDec-kkAcc )*( kkDec - kkAcc )*( kkVf*kkVf - 4*kkVf )
                                                + 4*( kkDec - kkAcc )*( kkDec*kkVc*kkVc + kkAcc*kkVf*kkVf )
                                                + 8*kkAcc*kkDec*( kkDec - kkAcc )*kkDist )
                     ) / (2*(kkDec-kkAcc)) );

    GPMode = decideMode();

    posPid->kp = 4;
    posPid->kd = 0;
    posPid->ki = 0;

    switch(GPMode)
    {
    case GPACC1:
    case GPACC2:
        //debug("GPACC",D_KK);
        kkVDesire =  kkVDesire*0.2 + kkVc*0.8 + 2*kkAcc*(((double)(conf()->Common_Main_Loop_Interval()))/1000) ;
        break;
    case GPVCONST:
        //debug("GPVCONST",D_KK);
        kkVDesire = kkVmaxDesire;
        break;
    case GPDEC1:
    case GPDEC2:
        //debug("GPDEC",D_KK);
        kkVDesire = kkVc/1.3 + kkDec*(((double)(conf()->Common_Main_Loop_Interval()))/1000);
        break;
    case GPPOS:
        //debug("GPPOS",D_KK);
        posPid->error = kkDist;
        kkVDesire = posPid->PID_OUT();
        angPid->kp = 3;
        angPid->kd = 0;
        break;
    }

    if(GPMode != GPPOS)
    {
        speedPidX->error = kkVDesire*cos(kkMovementTh.radian()) - kkVc*cos(kkMovementTh.radian());
        speedPidY->error = kkVDesire*sin(kkMovementTh.radian()) - kkVc*sin(kkMovementTh.radian());
        kkVx =  (kkVDesire)*cos(kkMovementTh.radian()) + speedPidX->error;
        kkVy =  (kkVDesire)*sin(kkMovementTh.radian()) + speedPidY->error;
        knowledge->SRSetAgentAbsArg(agentId ,kkVx  ,kkVy ,angPid->PID_OUT(), 0, 0, 0);
    }
    else
    {
        if(kkDist > 0.01)
            knowledge->SRSetAgentAbsArg(agentId, kkVDesire*cos(kkMovementTh.radian()),kkVDesire*sin(kkMovementTh.radian()),angPid->PID_OUT(), 0, 0, 0);
        else
            knowledge->SRSetAgentAbsArg(agentId,0,0,angPid->PID_OUT(),0,0,0);

        posPid->pError = kkDist;
    }
}

double SRSkillGotoPoint::progress()
{
    if(knowledge->getAgent(agentId)==NULL) return 0;
    double d = (knowledge->getAgent(agentId)->pos() - targetPos).length();
    if (d<0.04) return 1.0;
    if (d<0.05) return 0.8;
    if (d<0.10) return 0.7;
    if (d<0.20) return 0.6;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

INIT_SKILL(SRSkillGotoPointAvoid, "gotopointavoid");

SRSkillGotoPointAvoid::SRSkillGotoPointAvoid(CAgent *_agent) : SRSkillGotoPoint(_agent)
{
    counter = 0;
    avoidPenaltyArea = true;
    inited = false;
    keeplooking = false;
    extendStep = -1.0;
    gotopoint = new SRSkillGotoPoint(_agent);
    dynamicStart = true;
    plan2 = false;
    prof = NULL;
    noAvoid = false;
    avoidCenterCircle = false;
    stucked = -1;
    averageDir.assign(0 , 0);
    slow = false;
    kkAvoidBall = true;
    ballObstacleRadius = 0.02;
    kkKickMode = false;
    lookat = Vector2D(100, 100);

    angPid= new _PID(2,0,0,0,0);
    kkVMaxDesire = 4;
    kkVMax = 0;
    kkVReal = 0;

    speedPidX = new _PID(1,0,0,0,0);
    speedPidY = new _PID(1,0,0,0,0);
    speedPidX->kp = 3;
    speedPidY->kp = 3;
}

SRSkillGotoPointAvoid::~SRSkillGotoPointAvoid()
{
    delete gotopoint;
    delete angPid;
    delete speedPidX;
    delete speedPidY;
}

SRSkillGotoPointAvoid* SRSkillGotoPointAvoid::noRelax()
{
    ourRelaxList.clear();
    oppRelaxList.clear();
    return this;
}

SRSkillGotoPointAvoid* SRSkillGotoPointAvoid::ourRelax(int element)
{
    if (!ourRelaxList.contains(element)) ourRelaxList.append(element);
    return this;
}

SRSkillGotoPointAvoid* SRSkillGotoPointAvoid::oppRelax(int element)
{
    if (!oppRelaxList.contains(element)) oppRelaxList.append(element);
    return this;
}

bool SRSkillGotoPointAvoid::isCircleEmpty(double tRad)
{
    Circle2D tempCircle(agent->pos(), tRad);
    Vector2D sol1, sol2;

    for(int i = 0; i < wm->opp.activeAgentsCount(); i++)
    {
        if(Circle2D(wm->opp.active(i)->pos, 0.13).intersection(tempCircle, &sol1, &sol2))
            return false;
    }

    for(int i = 0; i < wm->our.activeAgentsCount(); i++)
    {
        if(agent->id() == wm->our.active(i)->id)
            continue;
        if(Circle2D(wm->our.active(i)->pos, 0.13).intersection(tempCircle, &sol1, &sol2))
            return false;
    }

    //TODO: add avoid exception
    if(Circle2D(Vector2D(-_FIELD_WIDTH/2 -0.2, 0.0), _GOAL_WIDTH/4 + _GOAL_RAD + 0.1).intersection(tempCircle, &sol1, &sol2))
        return false;

    if(Circle2D(Vector2D(_FIELD_WIDTH/2 +0.2, 0.0), _GOAL_WIDTH/4 + _GOAL_RAD + 0.1).intersection(tempCircle, &sol1, &sol2))
        return false;

    return true;
}

double SRSkillGotoPointAvoid::matchBiggestCircle(double tDist)
{
    double tempRad = 0.10;
    double selectedRad = 0.15;
    for(tempRad = 0.10; tempRad < tDist; tempRad += 0.05)
    {
        if(!isCircleEmpty(tempRad))
            break;
        selectedRad = tempRad;
    }
    return selectedRad;
}

bool SRSkillGotoPointAvoid::isPathClear(Vector2D tSPos, Vector2D tEPos, double threshold)
{
    Vector2D tempVecNorm;
    Vector2D tempEndPos;
    double tempDist;
    tempVecNorm = (tEPos- tSPos).norm();
    tempDist = tSPos.dist(tEPos);
    tempDist += 0.3;
    tempEndPos = tSPos + tempVecNorm*tempDist;
    Line2D tempPath(tSPos, tempEndPos);
    Line2D tempLine1(tSPos, tempEndPos);
    Line2D tempLine2(tSPos, tempEndPos);
    tempLine1 = tempPath.perpendicular(tSPos);
    tempLine2 = tempPath.perpendicular(tempEndPos);

    Vector2D sol1, sol2, sol3, sol4;
    Circle2D(tSPos, threshold).intersection(tempLine1, &sol1, &sol2);
    Circle2D(tempEndPos, threshold).intersection(tempLine2, &sol4, &sol3);

    Polygon2D tempPoly;
    tempPoly.addVertex(sol1);
    tempPoly.addVertex(sol2);
    tempPoly.addVertex(sol3);
    tempPoly.addVertex(sol4);
    tempPoly.addVertex(sol1);

    //draw(tempPoly, QColor(Qt::yellow));

    for(int i = 0; i < wm->opp.activeAgentsCount(); i++)
    {
        if(tempPoly.contains(wm->opp.active(i)->pos))
            return false;
    }

    for(int i = 0; i < wm->our.activeAgentsCount(); i++)
    {
        if(agent->id() == wm->our.active(i)->id)
            continue;
        if(tempPoly.contains(wm->our.active(i)->pos))
            return false;
    }

    if(Circle2D(Vector2D(-_FIELD_WIDTH/2 -0.2, 0.0), _GOAL_WIDTH/4 + _GOAL_RAD + 0.1).intersection(Segment2D(tSPos, tempEndPos), &sol1, &sol2))
        return false;

    if(Circle2D(Vector2D(_FIELD_WIDTH/2 +0.2, 0.0), _GOAL_WIDTH/4 + _GOAL_RAD + 0.1).intersection(Segment2D(tSPos, tempEndPos), &sol1, &sol2))
        return false;

    return true;
}

Vector2D SRSkillGotoPointAvoid::getIntersectedPosByCircle(Circle2D tCircle, Vector2D tInnerPos, Vector2D tOutterPos)
{
    Vector2D sol1, sol2;
    tCircle.intersection(Segment2D(tInnerPos, tOutterPos), &sol1, &sol2);
    tCircle.assign(tCircle.center(), tCircle.radius()+0.02);
    if(tCircle.contains(sol1))
        return sol1;
    else
        return tOutterPos;
}

void SRSkillGotoPointAvoid::execute()
{
    CRAgent = knowledge->getAgent(agentId);
    double tempCurrentVel;
    if (!getFinalPos().valid())
    {
        CRAgent->waitHere();
        if (prof!=NULL)
            prof->store();
        return;
    }
    if (!vel2.valid())
        vel2.assign(0,0);

    gotopoint->setAgentId(agentId);
    gotopoint->setSlowMode(slow);
    gotopoint->setMaxAcceleration(getMaxAcceleration());
    gotopoint->setMaxDeceleration(getMaxDeceleration());
    gotopoint->setPenaltyKick(penaltyKick);

    if(kkKickMode)
        ballObstacleRadius = 0.0;

    if (!inited)
    {
        inited = true;
    }
    if( noAvoid ){
        result.clear();
        result.append(CRAgent->pos());
        result.append(getFinalPos());
    }
    else
    {
        CRAgent->initPlanner(CRAgent->id() , getFinalPos() , ourRelaxList , oppRelaxList , avoidPenaltyArea , avoidCenterCircle , ballObstacleRadius);
        result.clear();
        for( int i=0 ; i<CRAgent->pathPlannerResult.size() ; i++ )
            result.append(CRAgent->pathPlannerResult[i]);
    }

    //////////////
    Circle2D tempCircle;
    int selectedIndex = result.count() - 1;
    bool selectedFlag = false;
    Vector2D tempTarget;

    double tempRad = 0.30;
    if(!result.isEmpty())
    {
        kkDist = CRAgent->pos().dist(result.last());
        tempRad = matchBiggestCircle(kkDist);
    }

    tempCircle = Circle2D(CRAgent->pos(), tempRad);
    for(int i = 1; i < result.count(); i++)
    {
        //draw(result.at(i),0,QColor(Qt::cyan));
        //draw(tempCircle,QColor(Qt::cyan));
        if(!tempCircle.contains(result.at(i)))
        {
            selectedIndex = i;
            selectedFlag = true;
            break;
        }
    }

    //isPathClear(Vector2D(0, 0), Vector2D(-1,1), 0.4);
    if(!result.isEmpty())
    {
        tempCurrentVel = CRAgent->vel().length();
//        if(selectedFlag)
//        {
//            if(selectedIndex == 0)
//                tempTarget = getIntersectedPosByCircle(tempCircle, getFinalPos(), result.at(0));
//            else
//                tempTarget = getIntersectedPosByCircle(tempCircle, result.at(selectedIndex-1), result.at(selectedIndex));
//        }
//        else
            tempTarget = result.at(selectedIndex);

        if(isPathClear(CRAgent->pos(), result.last(), 0.19))
        {
            if(slow)
                kkVMax = 2;
            else
                kkVMax = kkVMaxDesire;
            gotopoint->init(agentId, getFinalPos(), getFinalDir(), Vector2D(0,0), true);

            gotopoint->execute();
        }
        else
        {
            if(slow)
            {
                kkVMax = ((kkVMaxDesire - 1.2)/3.5)*tempRad + 1.2;
                if(kkVMax > 2)
                    kkVMax = kkVMaxDesire;
            }
            else
            {
                kkVMax = ((kkVMaxDesire - 1.2)/1.8)*tempRad + 1.2;
                if(kkVMax > kkVMaxDesire)
                    kkVMax = kkVMaxDesire;
            }

            if(kkDist < 0.1)
            {
                gotopoint->init(agentId,result.last(), getFinalDir(), Vector2D(0,0), true);
                gotopoint->execute();
            }
            else
            {
                Vector2D tempVec;
                tempVec = tempTarget - CRAgent->pos();
                //draw(Segment2D(CRAgent->pos(), CRAgent->pos()+tempVec.norm()*0.5),QColor(Qt::red));


                kkVReal = tempCurrentVel + 10*(((double)(conf()->Common_Main_Loop_Interval()))/1000);
                if(kkVReal > kkVMax)
                    kkVReal = kkVMax;




                kkMovementTh = (tempTarget -CRAgent->pos()).norm().th();

                angPid->error = (getFinalDir().th() - CRAgent->dir().th()).radian();
                angPid->kp=2;
                angPid->kd=0;
                angPid->ki=0;

                speedPidX->error = kkVReal*cos(kkMovementTh.radian()) - tempCurrentVel*cos(kkMovementTh.radian());
                speedPidY->error = kkVReal*sin(kkMovementTh.radian()) - tempCurrentVel*sin(kkMovementTh.radian());
                kkVx =  (kkVReal)*cos(kkMovementTh.radian());
                kkVy =  (kkVReal)*sin(kkMovementTh.radian());
                knowledge->SRSetAgentAbsArg(agentId, kkVx ,kkVy ,angPid->PID_OUT());
            }
        }
    }
    else
    {
        if(slow)
            kkVMax = 2;
        else
            kkVMax = kkVMaxDesire;
        gotopoint->init(agentId, getFinalPos(), getFinalDir(), Vector2D(0,0), true);
        gotopoint->execute();
    }
}


double SRSkillGotoPointAvoid::progress()
{
    return 0.0;
}
