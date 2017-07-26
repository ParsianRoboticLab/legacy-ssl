#include "kick.h" #include <QTime>

Vector2D penaltyAreaAvoidance(Vector2D agent, Vector2D target)
{
#if 1
    // consider some margin!
    Vector2D closerTarget = target.norm()*0.2;
    Vector2D closerTargetopp = target.norm()*0.2;
    if(wm->field->isInOurPenaltyArea(closerTarget) ){
        return closerTarget + (closerTarget - wm->field->ourGoal()).norm()*1;
    }
    return target;
#else
    Vector2D pos = target;

    Vector2D sol1, sol2;
    bool fff = false;
    bool inside = false;
    if (intersect_ellipse_line(agent , target, wm->field->ourGoal(), 0.9, 1.2, &sol1, &sol2)).
    {
        if ( (sol1-agent)*(sol1-target) < 0)
        {
            fff = true;
        }
        if ( (sol1-agent)*(sol2-agent) < 0)
        {
            inside = true;
        }
    }
    else {
        fff = false;
    }
    if (inside)
    {
        pos = (agent - wm->field->ourGoal()).norm() * 1.3 + wm->field->ourGoal();
    }
    else if (fff)
    {
        Vector2D p1 = (agent - wm->field->ourGoal()).norm().rotatedVector(30.0) * penaltyAreaAvoidanceRadius + wm->field->ourGoal();
        Vector2D p2 = (agent - wm->field->ourGoal()).norm().rotatedVector(-30.0) * penaltyAreaAvoidanceRadius + wm->field->ourGoal();
        if (p1.x > p2.x){
            pos = p1;
        }
        else{
            pos = p2;
        }
    }
    else {
    }
    return pos;
#endif
}


//////////////////////////////////////////////////////////////////////////new kick code for Robocup 2015 by DON MHMMD
CSkillNewKick::CSkillNewKick(CAgent *_agent)
{
    agent= _agent;
    kickSpeed = 15;
    chip = false;
    spin = false;
    slow = false;
    turn = false;
    autoChipSpeed = false;
    clear = false;
    avoidOurPenaltyArea = true;
    avoidOppPenaltyArea = true;
    dontRecvPass = false;
    tol = 0.2;
    waitFrames = 2;

    interceptMode = false;
    dontKick = false;
    chipWait = 0;
    chipSpinSpeed = 1;
    readyToChip = false;
    ballflag = false;
    ballWasFar = false;
    sagMode = false;
    penaltyKick = false;
    turnLastMomentForPenalty = false;
    isGotoPointAvoid =true;
}

CSkillNewKick * CSkillNewKick::setTarget(Vector2D val)
{
    target =val;
    return this;
}

bool CSkillNewKick::kickable()
{
    if (dontKick)
    {
        return false;
    }
    double tol = this->tol;
    if (tol < 0.4 && knowledge->getExperimentalMode() == 0 )
    {
        tol = 0.4;
    }
    double tolerance = fabs(Vector2D::angleBetween( target - (target-agent->pos()).rotatedVector(90).norm()*tol - agent->pos(), target + (target-agent->pos()).rotatedVector(90).norm()*tol - agent->pos() ).degree()) / 2.0;
    if (slow) waitFrames = 0;

    if (( fabs (Vector2D::angleBetween(target - agent->pos(), agent->dir()).degree()) <  tolerance))
    {

        return true;
    }
    return false;
}

void CSkillNewKick ::setAgent(CAgent *_agent)
{
    agent = _agent;
}

void CSkillNewKick::execute(){
    //////////////PID, set the direction
    _PID angPidd(3,0,0,0,0);

    /////
    static Circle2D ballMargin,virtualBallMargin,realMargin;
    /////////
    static AngleDeg finalDir= 0;
    ////////


    ////////////////find ball margin
    ballMargin.assign(wm->ball->pos,0.119);
    realMargin.assign(wm->ball->pos,0.09);

    if(fabs(((wm->ball->pos -  agent->pos()).th() - finalDir).degree() ) > 110 && agent->pos().dist(wm->ball->pos) > 0.5)
        virtualBallMargin.assign(wm->ball->pos,0.3);
    else
        virtualBallMargin.assign(wm->ball->pos,0.2);
    draw(realMargin,"blue");



    ///////////////find the place to stop
    Vector2D stopTarget;
    Segment2D ballPath;
    ballPath.assign(wm->ball->pos,wm->ball->pos+(((agent->pos().dist(wm->ball->pos))-0.09)*wm->ball->vel.norm()));
    if(interceptMode)
    {
        if( wm->ball->vel.length() > 0.2)
        {

            stopTarget=ballPath.nearestPoint(agent->pos())- Vector2D(0.09*cos(finalDir.radian()), 0.09*sin(finalDir.radian()));
            finalDir = (target - stopTarget).th();
            draw(stopTarget,D_HOSSEIN,"blue");
        }
        else
        {

            stopTarget.assign(wm->ball->pos.x - 0.12*cos(finalDir.radian()) + 0.20*wm->ball->vel.x, wm->ball->pos.y - 0.12*sin(finalDir.radian())+ 0.2*wm->ball->vel.y);
            finalDir = (target - wm->ball->pos).th();

        }
    }
    else
    {
        stopTarget.assign(wm->ball->pos.x - 0.12*cos(finalDir.radian()) + 0.20*wm->ball->vel.x, wm->ball->pos.y - 0.12*sin(finalDir.radian())+ 0.2*wm->ball->vel.y);

        finalDir = (target - wm->ball->pos).th();
    }

    draw(stopTarget,D_HOSSEIN,"black");
    ///////////////find the path
    Segment2D straightPath;
    straightPath.assign(stopTarget,agent->pos());


    Vector2D *path1 = new Vector2D(0,0);
    Vector2D *path2 = new Vector2D(0,0);
    Vector2D destin(0,0);


    if(ballMargin.intersection(straightPath,path1,path2) > 1)
    {
        virtualBallMargin.tangent(agent->pos(),path1,path2);
        draw(*path1,D_HOSSEIN,"green");
        draw(*path2,D_HOSSEIN,"blue");
        if(stopTarget.dist(*path1) < stopTarget.dist(*path2))
            destin = *path1;
        else
            destin = *path2;
    }
    else
    {
        destin= stopTarget ;
    }

    /////////////////////// goto target


    CSkillGotoPointAvoid *GPA = new CSkillGotoPointAvoid(agent);
    CskillNewGotoPoint *GP = new CskillNewGotoPoint(agent);


    if(agent->pos().dist(wm->ball->pos) >0.3 )
    {
        if(isGotoPointAvoid)
        {
            GPA->init(destin,Vector2D(cos(finalDir.radian()),sin(finalDir.radian())),Vector2D(0,0));
            GPA->execute();
        }
        else
        {
            GP->setMaxAcceleration(4);
            GP->setMaxDeceleration(-4);
            GP->init(destin,Vector2D(cos(finalDir.radian()),sin(finalDir.radian())),Vector2D(0,0));
            GP->execute();
        }
        agent->setKick(0);

    }
    else
    {
        double movement_theta;

        /////////////////////////////////////////kick
        if( fabs(agent->dir().th().degree() - finalDir.degree()) < 10  )
        {
            agent->setChip(chip);
            agent->setKick(kickSpeed);
        }
        else
        {
            angPidd.kp = 3;
            agent->setKick(0);
        }

        ///////////////////////////////////////turn for kick
        double reduce =1;
        if(slow)
            reduce = 0.8;
        else
        {
            if(fabs((agent->dir().th() - finalDir).degree()) > 20)
                reduce =0.5;
            else
                reduce =1.2;
        }
        if ((agent->dir().th() - finalDir).degree()  <-50 )
        {
            angPidd.error = (wm->ball->pos - agent->pos()).th().radian();

            agent->setRobotVel(cos((wm->ball->pos - agent->pos()).th().radian() - _PI/2),sin((wm->ball->pos - agent->pos()).th().radian() - _PI/2),angPidd.PID_OUT());


        }else if ((agent->dir().th() - finalDir).degree()  >50 )
        {
            angPidd.error = (wm->ball->pos - agent->pos()).th().radian();

            agent->setRobotAbsVel(cos((wm->ball->pos - agent->pos()).th().radian() + _PI/2),sin((wm->ball->pos - agent->pos()).th().radian() + _PI/2),angPidd.PID_OUT());

        }
        ////////////////////////////////////// catch ball
        else
        {
            angPidd.error = (finalDir - agent->dir().th()).radian();

            if( ((wm->ball->pos - agent->pos()).th() - finalDir).degree()  > 90  )
                movement_theta = ((wm->ball->pos - agent->pos()).th().radian() + 0.8+1.5*(0.4-agent->pos().dist(wm->ball->pos))- agent->dir().th().radian());
            else if(((wm->ball->pos - agent->pos()).th() - finalDir).degree() <- 90)
                movement_theta = ((wm->ball->pos- agent->pos()).th().radian() - 0.8- 1.5*(0.4-agent->pos().dist(wm->ball->pos))- agent->dir().th().radian());
            else if(((wm->ball->pos - agent->pos()).th() - finalDir).degree() >30)
                movement_theta = ((wm->ball->pos- agent->pos()).th().radian() +0.6 + 1.5*(0.4-agent->pos().dist(wm->ball->pos)) - agent->dir().th().radian());
            else if(((wm->ball->pos - agent->pos()).th() - finalDir).degree() < -30)
                movement_theta = ((wm->ball->pos- agent->pos()).th().radian() - 0.6 - 1.5*(0.4-agent->pos().dist(wm->ball->pos))- agent->dir().th().radian());
            else if(((wm->ball->pos - agent->pos()).th() - finalDir).degree() >10)
                movement_theta = ((wm->ball->pos- agent->pos()).th().radian() +0.3 + 1.5*(0.4-agent->pos().dist(wm->ball->pos)) - agent->dir().th().radian());
            else if(((wm->ball->pos - agent->pos()).th() - finalDir).degree() < -10)
                movement_theta = ((wm->ball->pos- agent->pos()).th().radian() - 0.3 - 1.5*(0.4-agent->pos().dist(wm->ball->pos))- agent->dir().th().radian());
            else
                movement_theta = ((wm->ball->pos- agent->pos()).th().radian() - agent->dir().th().radian());
            /////////////////////////////////////////////////////////////////////////////////
            double ballx= (wm->ball->vel.x)*cos(agent->dir().th().radian()) + (wm->ball->vel.y)*sin(agent->dir().th().radian());
            double bally= -1*(wm->ball->vel.x)*sin(agent->dir().th().radian()) + (wm->ball->vel.y)*cos(agent->dir().th().radian());
            if(interceptMode)
                agent->setRobotVel(reduce*cos(movement_theta)  + 0.4*ballx,reduce*sin(movement_theta)+0.4*bally ,angPidd.PID_OUT());
            else
                agent->setRobotVel(reduce*cos(movement_theta) + 0.6*ballx,reduce*sin(movement_theta) + 0.6*bally,angPidd.PID_OUT());
        }

    }



}

////////////////////////////////////////////////////////////////////////////////////////////////////////////Receive Pass Skill Created by DON MHMMD Shirazi

INIT_SKILL(CSkillReceivePass,"receivepass");

CSkillReceivePass::CSkillReceivePass(CAgent *_agent)
{
    agent = _agent;
    gotopointavoid = new CSkillGotoPointAvoid(_agent);
    slow = true;
    avoidOurPenaltyArea = true;
    avoidOppPenaltyArea = true;
    receiveR = 1.0;
    received = false;
    velThresh = 0;
    cirThresh = 0;
    kickCirThresh = 0;
    ignoreAngle = true;

}

double CSkillReceivePass::progress()
{
    return 0.0;
}

CSkillReceivePass::~CSkillReceivePass()
{

}

kkRPMode CSkillReceivePass::decideMode()
{
    Circle2D tempCircle(target, 0.2 + cirThresh);
    Circle2D tempCircle2(target, receiveR);
    draw(tempCircle, QColor(Qt::cyan));

    if(tempCircle.contains(kkAgentPos) && ballRealVel > 0.2 )
    {
        cirThresh = 1.0;
        Circle2D tempKickCircle(kkAgentPos, 0.3 + kickCirThresh);
        Segment2D tempBallPath(kkBallPos, kkBallPos + wm->ball->vel.norm()*10);
        draw(tempBallPath,QColor(Qt::yellow));
        Vector2D sol1, sol2;
        if(tempCircle2.intersection(tempBallPath, &sol1, &sol2) == 0 && !tempCircle.contains(kkBallPos))
        {
            kickCirThresh = 0;
            velThresh = 0;
            return RPNONE;
        }
        else if(tempKickCircle.contains(kkBallPos))
        {
            kickCirThresh = 0.5;
            velThresh = 0.2;
            return RPDAMP;
        }
        else
        {
            kickCirThresh = 0;
            velThresh = 0;
            return RPINTERSECT;
        }
    }
    else
    {
        cirThresh = 0;
        kickCirThresh = 0;
        velThresh = 0;
        return RPWAITPOS;
    }
}

void CSkillReceivePass::execute()
{
    ballRealVel = knowledge->getRealBallVel();
    gotopointavoid->setSlowMode(slow);
    gotopointavoid->setAgent(agent);
    gotopointavoid->setNoAvoid(false);
    gotopointavoid->setBallObstacleRadius(0.4);
    gotopointavoid->setAvoidBall(true);

    kkBallPos = wm->ball->pos;
    kkAgentPos = agent->pos();
    receivePassMode = decideMode();

    Segment2D ballPath;
    ballPath.assign(kkBallPos,kkBallPos + wm->ball->vel.norm()*(kkAgentPos.dist(kkBallPos)+1));
    draw(ballPath,"red");


    Vector2D oneTouchDir;
    if(ignoreAngle)
        oneTouchDir = (kkBallPos - kkAgentPos).norm();
    else
        oneTouchDir = IATargetDir;

    draw(Segment2D(Vector2D(0,0), Vector2D(0,0)+oneTouchDir.norm()), QColor(Qt::red));

    //Vector2D addVec = Vector2D(0.095*cos((target-kkAgentPos).th().radian()), 0.095*sin((target-kkAgentPos).th().radian()));
    Vector2D intersectPos;
    Line2D agentDirLine(kkAgentPos , kkAgentPos + oneTouchDir.norm());
    Line2D agentPerLine(kkAgentPos, kkAgentPos + oneTouchDir.norm());
    agentPerLine = agentDirLine.perpendicular(kkAgentPos);
    Line2D tempBallPath(kkBallPos, kkBallPos + wm->ball->vel.norm());
    double tempDampSpeed;

    Vector2D tempVecDamp, tempDampTarget;
    switch (receivePassMode)
    {
    case RPWAITPOS:
        if (target.valid())
            gotopointavoid->init(target, oneTouchDir);
        else
            gotopointavoid->init(agent->pos(), oneTouchDir);
        gotopointavoid->setSlowMode(false);
        gotopointavoid->execute();
        debug("RPWAITPOS",D_KK);
        break;
    case  RPDAMP:
    case RPRECEIVE:
        tempVecDamp = (kkAgentPos - kkBallPos).norm();
        tempDampSpeed = (ballRealVel - agent->vel().length())*0.05;
        if(tempDampSpeed > 0.003) tempDampSpeed = 0.003;
        tempDampSpeed = 0;
        tempDampTarget = kkBallPos + (kkAgentPos - kkBallPos).norm()*0.10 + tempVecDamp*tempDampSpeed;
        gotopointavoid->init(tempDampTarget,oneTouchDir);
        gotopointavoid->execute();
        debug("RPdamp-Back",D_KK);
        break;
    case RPINTERSECT:
        agent->setRoller(2);
        intersectPos = agentPerLine.intersection(tempBallPath);
        gotopointavoid->init(intersectPos,oneTouchDir);
        gotopointavoid->setSlowMode(false);
        gotopointavoid->setOneTouchMode(true);
        gotopointavoid->execute();
        debug("Intercept", D_KK);

        break;
    case RPNONE:
        gotopointavoid->init(target,oneTouchDir);
        gotopointavoid->setOneTouchMode(true);
        gotopointavoid->setSlowMode(false);
        gotopointavoid->execute();
        debug("RP",D_KK);
        break;
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

double calcBallTime(Vector2D _pos,double _dec)
{
    double _dist = wm->ball->pos.dist(_pos);
    double sol1,sol2;
    double ballVel = knowledge->getRealBallVel();
    double ballVel2 = ballVel*ballVel;
    double timeIndep = 2*_dec*_dist;
    if(-1*timeIndep > (ballVel2))
        return -1;
    else
    {
        sol1 = (-ballVel + sqrt(ballVel2 + timeIndep))/2*_dec;
        sol2 = (-ballVel - sqrt(ballVel2 + timeIndep))/2*_dec;
        //draw(QString("s1: %1 - s2: %2").arg(sol1).arg(sol2));
        if(0)//sol1 > 0 )
            return sol1;
        else if(sol2 >0)
            return sol2;
        else
            return -1;
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

INIT_SKILL(CSkillKick, "kick");

CSkillKick::CSkillKick(CAgent *_agent) : CSkill(_agent)
{

    intercept = new CSkillIntercept(_agent);
    kickSpeed = 15;
    avoidOppPenaltyArea = true;
    chip = false;
    spin = false;
    slow = false;
    veryFine = false;
    turn = false;
    autoChipSpeed = false;
    clear = false;
    avoidPenaltyArea = false;
    dontRecvPass = false;
    tol = 3;
    waitFrames = 2;
    agent0 = NULL;
    interceptMode = false;
    dontKick = false;
    chipWait = 0;
    chipSpinSpeed = 1;
    readyToChip = false;
    ballflag = false;
    ballWasFar = false;
    sagMode = false;
    penaltyKick = false;
    isGotoPointAvoid =true;
    turnLastMomentForPenalty = false;

    kickMode = KDIRECT;
    kkRadThresh = 0;
    kkRadLimit = 0.20;
    kkAngleThresh = 0;
    kkTAngleThresh = 0;
    kkKickThresh = 0;
    kkJTurnThresh = 0;
    kickIntercept = new CSkillIntercept(agent);
    gpa = new CSkillGotoPointAvoid(_agent);
    gpa->setSlowMode(false);
    angPid= new _PID(2,0,0,0,0);
    speedPid = new _PID(1,0,0,0,0);
    posPid = new _PID(1,0,0,0,0);
    veryFine = false;
    interceptMode = false;
    kkShotEmpySpot = false;
    alternateMode = false;
    kickAngTol = 50;
    isJturn = false;
    passProfiler = false;
    kickWithCenterOfDribbler = false;
    goalieMode = false;
    jTurnFromBack = false;
    playMakeMode = false;
    //fastIntercept =false;
}

CSkillKick::~CSkillKick()
{

    delete gpa;
    delete angPid;
    delete speedPid;
    delete posPid;
}

void CSkillKick::resetI()
{
    gotoball->resetI();
}

CSkillKick* CSkillKick::setTarget(Vector2D val)
{
    target = val;
    return this;
}

double CSkillKick::progress()
{
    return 0.0;
}

bool CSkillKick::kickable()
{

    return false;
}

kckMode CSkillKick::decideMode()
{
    /////////////////// must renew

    gpa->setAddVel(Vector2D(0,0));
    Vector2D tempVec1, tempVec2;
    Circle2D ballArea(ballPos,0.13);
    Circle2D dribblerArea(agentPos+agentDir.norm()*0.1,0.25);

    Circle2D robotArea(agentPos,1);

    if(agentPos.dist(ballPos) < 1.5) {
        robotArea.assign(agentPos,max(agentPos.dist(ballPos) - 0.1, 0.01));
    }

    if(goalieMode)
        robotArea.assign(agentPos,0.5);
    if(ballPos.dist(agentPos) < 1)
        robotArea.assign(agentPos,max(ballPos.dist(agentPos) - 0.01, 0.01));
    Segment2D robotpath(agentPos,ballPos-(target-ballPos).norm()*0.15);
    Segment2D ballpath(wm->ball->pos,wm->ball->pos+wm->ball->vel.norm()*(12));
    double tempAngle;
    tempVec1 = target - ballPos;
    tempAngle = fabs((tempVec1.th() - (ballPos - agentPos).th()).degree());

    ///////////////////////////////find center of dribbler for pass profiler
    Polygon2D robotKickArea;

    double distCoef =   (2.5/ballPos.dist(target))/100;
    distCoef = min(distCoef,0.03);
    distCoef = max(distCoef,0.02);
    debug(QString("distCoef: %1").arg(distCoef),D_MHMMD);

    robotKickArea.addVertex(agentPos+agent->dir().norm()*0.08+agent->dir().rotate(90).norm()*distCoef);
    robotKickArea.addVertex(agentPos+agent->dir().norm()*0.35+agent->dir().rotate(90).norm()*distCoef);
    robotKickArea.addVertex(agentPos+agent->dir().norm()*0.35-agent->dir().rotate(90).norm()*distCoef);
    robotKickArea.addVertex(agentPos+agent->dir().norm()*0.08-agent->dir().rotate(90).norm()*distCoef);


    if((!knowledge->isOurNonPlayOnKick())&&(passProfiler || kickWithCenterOfDribbler)) {
        if(dribblerArea.contains(ballPos) && robotKickArea.contains(ballPos))
        {
            kickerOn = true;
            agent->setRoller(spin);

        }
        else
            kickerOn = false;
    }
    else {
        if(dribblerArea.contains(ballPos))
        {
            kickerOn = true;
            agent->setRoller(spin);

        }
        else
            kickerOn = false;
    }

    draw(QString("theta : %1").arg(fabs(((target-agentPos).th().degree() - (ballPos-agentPos).th().degree() ))) , Vector2D(-1,-1));



    if(playMakeMode && wm->field->isInOurPenaltyArea(ballPos))
    {
        return KAVOIDOPPENALTY;
    }
    else if((!penaltyKick) && wm->field->isInOppPenaltyArea(ballPos) && !passProfiler && avoidOppPenaltyArea && !((robotArea.intersection(ballpath,&tempVec1,&tempVec2) ==2 && ballRealVel > 1 )))
    {
        return KAVOIDOPPENALTY;
    }
    else
    {

        if(dontKick)
        {
            return KDONTKICK;
        }
        else
        {
            return KDIRECT;
        }


    }


    gpa->setADiveMode(false);

    //////////////////////

}

void CSkillKick::kgoalie()
{
    if(kkShotEmpySpot)
        target = findMostPossible();
    angPid->kp = 4;

    if( ((ballPos - agentPos).th() - kickFinalDir).degree()  > 110  )
        kkMovementTheta = ((ballPos - agentPos).th().radian() + 1.2+1.5*(0.45-agentPos.dist(ballPos))- agentDir.th().radian());
    else if(((ballPos - agentPos).th() - kickFinalDir).degree() <- 110)
        kkMovementTheta = ((ballPos- agentPos).th().radian() - 1.2- 1.5*(0.45-agentPos.dist(ballPos))- agentDir.th().radian());
    //    else if(((ballPos - agentPos).th() - kkFinalDir).degree() >45)
    //        kkMovementTheta = ((ballPos- agentPos).th().radian() +1.2 + 1.5*(0.42-agentPos.dist(ballPos)) - agentDir.th().radian());
    //    else if(((ballPos - agentPos).th() - kkFinalDir).degree() < -45)
    //        kkMovementTheta = ((ballPos- agentPos).th().radian() - 1.2 - 1.5*(0.42-agentPos.dist(ballPos))- agentDir.th().radian());
    //    else if(((ballPos - agentPos).th() - kkFinalDir).degree() >5)
    //        kkMovementTheta = ((ballPos- agentPos).th().radian() + 0.5 + 1*(0.35-agentPos.dist(ballPos)) - agentDir.th().radian());
    //    else if(((ballPos - agentPos).th() - kkFinalDir).degree() < -5)
    //        kkMovementTheta = ((ballPos- agentPos).th().radian() - 0.5 - 1*(0.35-agentPos.dist(ballPos))- agentDir.th().radian());
    else
        kkMovementTheta =((ballPos- agentPos).th().radian() - agentDir.th().radian());


    angPid->error = (kickFinalDir - agentDir.th()).radian();
    double reduce = 2;



    if(kickerOn)
    {
        reduce =0.7;
    }

    double ballx= (wm->ball->vel.x)*cos(agentDir.th().radian()) + (wm->ball->vel.y)*sin(agentDir.th().radian());
    double bally= -1*(wm->ball->vel.x)*sin(agentDir.th().radian()) + (wm->ball->vel.y)*cos(agentDir.th().radian());
    Segment2D ballpath(ballPos,ballPos+wm->ball->vel.norm()*10);
    Segment2D goalLine(wm->field->ourGoalL(),wm->field->ourGoalR());
    Vector2D targetInt;
    targetInt = goalLine.intersection(ballpath);

    agent->setRobotVel(reduce*cos(kkMovementTheta)*(1+ballx)
                       ,reduce*sin(kkMovementTheta)*(1+bally) + 2*bally
                       ,angPid->PID_OUT());

}

void CSkillKick::kWaitForTurn()
{
    Segment2D ballpath(wm->ball->pos,wm->ball->pos+wm->ball->vel.norm()*(10));
    Vector2D sol1;

    sol1 = ballpath.nearestPoint(agentPos);
    agent->setRoller(3);
    gpa->init(sol1,ballPos - agentPos);
    //gpa->setADiveMode(false);
    gpa->setOneTouchMode(true);
    gpa->execute();
}

void CSkillKick::waitAndKick()
{

    Segment2D ballPath;
    double stopParam = 0.08;
    ballPath.assign(ballPos,ballPos + wm->ball->vel.norm()*(15));
    Segment2D ballLine;
    ballLine.assign(ballPos,ballPos + wm->ball->vel.norm()*(15));
    draw(ballPath,"red");



    Vector2D oneTouchDir = Vector2D::unitVector(oneTouchAngle(agentPos, agent->vel(), wm->ball->vel, agentPos - ballPos, target, conf()->SkillsParams_KickOneTouch_Landa(), conf()->SkillsParams_KickOneTouch_Gamma()));
    Vector2D kickerPoint = agentPos + agentDir.norm()*stopParam;
    Vector2D addVec = (agentPos - target).norm()*stopParam;
    Vector2D intersectPos;
    Vector2D sol1,sol2;
    double onetouchRad =2;
    double onetouchKickRad = 0.5;
    Circle2D oneTouchArea;
    Circle2D oppPenaltyArea(wm->field->oppGoal() + Vector2D(0.15,0),1.35);
    Circle2D oppPenaltyAreaWP(wm->field->oppGoal() + Vector2D(0.15,0),1.55);
    draw(oppPenaltyAreaWP,QColor(Qt::red));
    ///temp
    draw(oppPenaltyArea,QColor(Qt::red));

    if(ballPos.dist(agentPos) <= onetouchRad)
        onetouchRad = ballPos.dist(agentPos)-0.08;
    oneTouchArea.assign(agentPos,onetouchRad);
    gpa->setAddVel(Vector2D(0,0));

    gpa->setNoAvoid(false);
    intersectPos = ballPath.nearestPoint(kickerPoint);
    //        if(wm->field->isInOppPenaltyArea(intersectPos) )
    //        {
    //            if(oppPenaltyArea.intersection(ballLine,&sol1,&sol2))
    //            {
    //                if(sol1.dist(agentPos) > sol2.dist(agentPos))
    //                {
    //                    sol1 = sol2;
    //                }
    //                intersectPos = sol1;
    //            }
    //        }


    gpa->init(intersectPos +addVec,oneTouchDir);

    gpa->setOneTouchMode(true);
    gpa->execute();
    draw(intersectPos);
    if(agentPos.dist(ballPos) < 1)
    {
        if(chip)
        {
            agent->setChip(kickSpeed);
        }
        else
        {
            agent->setKick(kickSpeed);
        }
    }
    // agent->setRoller(3);


}



void CSkillKick::kDontKick()
{

    Vector2D finalPos;
    gpa->setSlowMode(true);
    finalPos=ballPos-(target-ballPos).norm()*0.23;
    gpa->setBallObstacleRadius(0.4);
    if(fabs((kickFinalDir - agentDir.th()).degree()) < 10)
    {
        if(0 && wm->ball->pos.x < -1)
        {
            agent->setRoller(5);
            finalPos=ballPos-(target-ballPos).norm()*0.11;
        }
        else
        {
            agent->setRoller(0);
            finalPos=ballPos-(target-ballPos).norm()*0.13;
        }
        gpa->setSlowMode(true);
        gpa->setBallObstacleRadius(0);
    }
    gpa->setADiveMode(false);
    gpa->setNoAvoid(false);
    gpa->init(finalPos, ballPos - agentPos);
    gpa->execute();
}

void CSkillKick::direct()
{
    findPosToGo();
    //findPosToGoAlt();
}


void CSkillKick::avoidOurPenalty()
{
    gpa->setSlowMode(slow);
    Vector2D finalPos ,dummyPos1,dummyPos;
    Vector2D tempVector;
    tempVector = ballPos - wm->field->ourGoal();
    Circle2D penaltyCircle;
    Segment2D ballSeg;
    Segment2D ballPosSeg;
    ballPosSeg.assign( wm->field->ourGoal(),wm->field->ourGoal() + tempVector.norm()*2);
    ballSeg.assign(ballPos,ballPos+wm->ball->vel.norm()*10);
    penaltyCircle.assign(wm->field->ourGoal() + Vector2D(0.15,0),1.4);


    penaltyCircle.intersection(ballPosSeg,&dummyPos1,&dummyPos);

    if(wm->field->isInField(dummyPos1))
    {
        finalPos = dummyPos1;
    }
    draw(finalPos);
    if(ballPos.dist(agentPos) > 0.2)
        finalDirVec = target - agentPos;
    gpa->setADiveMode(false);
    gpa->setAvoidPenaltyArea(true);
    gpa->setNoAvoid(false);

    gpa->init(finalPos, finalDirVec);
    gpa->execute();
    agent->setKick(kickSpeed);
}

void CSkillKick::avoidOppPenalty()
{
    gpa->setSlowMode(slow);
    Vector2D finalPos ,dummyPos1,dummyPos;
    Vector2D tempVector;
    tempVector = ballPos - wm->field->oppGoal();
    Circle2D penaltyCircle;
    Segment2D ballSeg;
    Segment2D ballPosSeg;
    Segment2D penaltyStraightLine;
    penaltyStraightLine.assign(Vector2D(_FIELD_WIDTH/2 - _FIELD_PENALTY -0.1,0.7 ),Vector2D(_FIELD_WIDTH/2 - _FIELD_PENALTY-0.1 ,-0.7 ));
    ballPosSeg.assign( wm->field->oppGoal(),wm->field->oppGoal() + tempVector.norm()*2);
    ballSeg.assign(ballPos,ballPos+wm->ball->vel.norm()*10);
    penaltyCircle.assign(wm->field->oppGoal() + Vector2D(0.15,0),1.4);

    if(wm->ball->vel.length()> 0.5)
    {
        penaltyCircle.intersection(ballSeg,&dummyPos1,&dummyPos);
        if(!wm->field->isInField(dummyPos1))
        {
            penaltyCircle.intersection(ballPosSeg,&dummyPos1,&dummyPos);
        }

    }
    else
    {
        penaltyCircle.intersection(ballPosSeg,&dummyPos1,&dummyPos);
    }

    if(wm->field->isInField(dummyPos1))
    {
        finalPos = dummyPos1;
    }
    draw(finalPos);
    if(ballPos.dist(agentPos) > 0.2)
        finalDirVec = target - agentPos;
    gpa->setADiveMode(false);
    gpa->setAvoidPenaltyArea(true);
    gpa->setNoAvoid(false);

    //    if(!wm->field->isInOppPenaltyArea(finalPos + (wm->field->oppGoal() - finalPos).norm()*0.1))
    //    {
    //        if(penaltyStraightLine.intersection(ballPosSeg).isValid())
    //        {
    //            finalPos = penaltyStraightLine.intersection(ballPosSeg);
    //        }
    //    }
    gpa->init(finalPos, finalDirVec);
    gpa->execute();
    agent->setKick(kickSpeed);
}

void CSkillKick::indirect()
{

    if(kkShotEmpySpot)
        target = findMostPossible();
    Vector2D sol1,sol2;

    Circle2D ballArea(ballPos,0.15);
    ballArea.tangent(agentPos,&sol1,&sol2);
    Vector2D finalPos;
    finalPos=ballPos-(target-ballPos).norm()*0.15;

    if(sol1.dist(finalPos) >= sol2.dist(finalPos))
        sol1 = sol2;

#define vRobotTemp 1
    if(wm->ball->vel.length() > 0.3)
    {
        for(double i = 0 ; i < 5 ; i += 0.1)
        {
            if(wm->ball->getPosInFuture(i).dist(agentPos)/vRobotTemp < i)
            {
                sol1 = wm->ball->getPosInFuture(i) - (target-wm->ball->getPosInFuture(i)).norm()*0.15;
                break;
            }
        }
        gpa->setAvoidBall(true);
    }
    else
    {
        gpa->setAvoidBall(false);
    }
    gpa->setSlowMode(slow);

    gpa->setADiveMode(false);
    gpa->init(sol1  , target - sol1);
    gpa->execute();
}

Vector2D CSkillKick::getTurnFunc(double ang, double _r, double _angOff)
{
    if(kkShotEmpySpot)
        target = findMostPossible();
    const double pi = 3.141592;
    double tempR, tempX, tempY, theta, theta2;
    double tempX2, tempY2;
    ang = pi - ang;
    theta = pi - ang;
    tempR = _r - (_r/pi)*fabs(theta);

    if(ang > 0)
    {
        theta2 = pi - ang;
        tempX = tempR*sin(theta2);
        tempY = tempR*cos(theta2);
    }
    else
    {
        theta2 = ang;
        tempX = -tempR*sin(theta2);
        tempY = tempR*cos(theta2);
    }

    if(_angOff != 0)
    {
        tempX2 = tempX*sin(_angOff) + tempY*cos(_angOff);
        tempY2 = -tempX*cos(_angOff) + tempY*sin(_angOff);
        return Vector2D(tempX2, tempY2);
    }

    return Vector2D(tempX, tempY);
}


void CSkillKick::jTurn()
{
    if(kkShotEmpySpot)
        target = findMostPossible();
    //    //// vars
    bool isFinalController = false;
    double posPidKp = 1;
    double speedPidKp = 1;
    Vector2D targetForJturnSpeed,targetForJturnPos;
    Vector2D idealPass ;
    Vector2D movementThSpeed,movementThPos;
    double movementDir = ((ballPos - agentPos).th() - kickFinalDir).degree();
    double shift = 0;
    double distCoef = 0.15;

    idealPass = (ballPos - agentPos).norm()*distCoef;
    /*
    if(abs(movementDir) < 30 && ballPos.dist(agentPos) < 0.25)
    {
        isFinalController =true;
    }
    else
    {
        isFinalController = false;
    }
    if(movementDir <= 70 && movementDir >= -70)
    {
        targetForJturnPos = ballPos + (ballPos - target).norm()*0.11;
        if(movementDir > 20)
        {
            if(wm->ball->vel.length() > 0.1)
                shift = 15 + (1-agentPos.dist(ballPos))*45;
            else
                shift = 0 + (1-agentPos.dist(ballPos))*10;

        }
        else if(movementDir < -20)
        {
            if(wm->ball->vel.length() > 0.1)
                shift = -15 - (1-agentPos.dist(ballPos))*45;

            else
                shift = 0 - (1-agentPos.dist(ballPos))*10;
        }
        else
        {
            agent->setRoller(spin);
            shift = 0;
        }

        idealPass.rotate(shift);
        targetForJturnSpeed = agentPos + idealPass;

    }
    else
    {
        if(movementDir > 0)
        {
            shift = 15 + (1-agentPos.dist(ballPos))*61;
        }
        else if(movementDir < 0)
        {
            shift = -15 - (1-agentPos.dist(ballPos))*61;
        }
        idealPass.rotate(shift);
        targetForJturnSpeed = agentPos + idealPass;
    }

*/

    if(movementDir < 20 && movementDir > -20)
        shift = 0;
    else if(movementDir > 50)
        shift = 20   + (1-agentPos.dist(ballPos))*61;
    else if(movementDir < -50)
        shift = -20 - (1-agentPos.dist(ballPos))*61;
    else if(movementDir > 25) {
        if(wm->ball->vel.length() < 0.1)
            shift = 5 + (1-agentPos.dist(ballPos))*10;
        else
            shift =10 + (1-agentPos.dist(ballPos))*45;
        distCoef = 0.17;
    }
    else if(movementDir < -25){
        if(wm->ball->vel.length() < 0.1)
            shift = -5 - (1-agentPos.dist(ballPos))*10;
        else
            shift = -10 - (1-agentPos.dist(ballPos))*45;

        distCoef = 0.17;
    }
    else if(movementDir > 0) {
        if(wm->ball->vel.length() < 0.1)
            shift = 5 + (1-agentPos.dist(ballPos))*10;
        else
            shift =5 + (1-agentPos.dist(ballPos))*22;
        distCoef = 0.17;
    }
    else if(movementDir < 0){
        if(wm->ball->vel.length() < 0.1)
            shift = -5 - (1-agentPos.dist(ballPos))*10;
        else
            shift = -5 - (1-agentPos.dist(ballPos))*22;

        distCoef = 0.17;
    }

    idealPass.rotate(shift);
    targetForJturnSpeed = agentPos + idealPass;

    movementThSpeed = (targetForJturnSpeed - agentPos).norm();
    movementThPos = (targetForJturnPos - agentPos).norm();
    double dirReduce;//= (fabs(movementDir)/70)*(fabs(movementDir)/70);

    speedPid->error  = targetForJturnSpeed.dist(agentPos);
    posPid->error = 0;//targetForJturnPos.dist(agentPos);

    ////////////set Active adaptive PIDs



    //    if(0 && (wm->field->isInOppPenaltyArea(wm->ball->pos + (wm->field->oppGoal() - wm->ball->pos).norm()*0.1) && agentPos.dist(ballPos) < 0.3))
    //    {
    //        if(isFinalController )
    //        {
    //            if(wm->ball->vel.length() > 0.2 || 1)
    //            {
    //                posPid->kp =3+(0.001/(agentPos.dist(targetForJturnPos)*agentPos.dist(targetForJturnPos)));
    //                speedPid->kp = 2;// +2.1*agentPos.dist(ballPos) + dirReduce;
    //            }
    //            else
    //            {
    //                posPid->kp = 3+(0.001/(agentPos.dist(targetForJturnPos)*agentPos.dist(targetForJturnPos)));
    //                speedPid->kp = 2;// +2.1*agentPos.dist(ballPos) + dirReduce;
    //            }
    //        }
    //        else
    //        {
    //            posPid->kp = 0;
    //            speedPid->kp = 6 +2.1*agentPos.dist(ballPos) + dirReduce;
    //        }
    //    }
    //    if(1)
    //    {
    posPid->kd = 0;

    //        if(isFinalController )
    //        {
    //            if(wm->ball->vel.length() > 0.2 )
    //            {
    //                posPid->kp =0;// 1+(0.001/(agentPos.dist(targetForJturnPos)*agentPos.dist(targetForJturnPos)));
    //                speedPid->kp = 4;// +2.1*agentPos.dist(ballPos) + dirReduce;
    //            }
    //            else
    //            {
    //                posPid->kd = 20;

    //                posPid->kp = 2+(0.01/(agentPos.dist(targetForJturnPos)*agentPos.dist(targetForJturnPos)));
    //                speedPid->kp = 3;// +2.1*agentPos.dist(ballPos) + dirReduce;
    //            }
    //        }
    //        else
    //        {

    dirReduce = (fabs(movementDir)/100)*(fabs(movementDir)/100);
    if(wm->field->isInOppPenaltyArea(ballPos + (wm->field->oppGoal() - ballPos).norm()*0.15) && agentPos.dist(ballPos)<0.25)
    {
        dirReduce -= 4;
    }
    if(knowledge->isOurNonPlayOnKick())
    {
        dirReduce -= 1;
    }

    if(wm->ball->vel.length() < 0.2)
    {
        posPid->kp = 0;
        speedPid->kp = 5 +15.1*agentPos.dist(ballPos) + dirReduce;
    }
    else
    {
        posPid->kp = 0;
        speedPid->kp = 4+8.1*agentPos.dist(ballPos) + dirReduce;

    }

    if(!jTurnFromBack)
    {
        dirReduce += 1;
    }


    if(penaltyKick)
    {
        angPid->kp = 7;
        speedPid->kp = 4;
    }

    angPid->kp = 5;
    if(wm->ball->vel.length() > 0.5)
        angPid->kp = 7;

    double vx= movementThSpeed.x * speedPid->PID_OUT() + movementThPos.x * posPid->PID_OUT();
    double vy = movementThSpeed.y * speedPid->PID_OUT()+ movementThPos.y * posPid->PID_OUT();
    angPid->error = (kickFinalDir - agentDir.th()).radian();
    agent->setRobotAbsVel(wm->ball->vel.x + vx
                          ,wm->ball->vel.y+ vy
                          ,angPid->PID_OUT());
    speedPid->pError = speedPid->error;

    posPid->pError = posPid->error;

    agent->accelerationLimiter(0,false);

}

void CSkillKick::turnForKick()
{
    if(kkShotEmpySpot)
        target = findMostPossible();
    agent->setRoller(0);

    double angReduce = 1;

    if(knowledge->isOurNonPlayOnKick())
    {
        if(fabs((agentDir.th() - kickFinalDir).degree()) < 80)
            angReduce = 0.5;
        if ((agentDir.th() - kickFinalDir).degree()  <- 10 )
        {
            angPid->kp = 4*angReduce;

            angPid->error = ((ballPos - agentPos).th() - agent->dir().th()).radian();
            agent->setRobotVel( (-0.12 + agentPos.dist(ballPos))*4 ,-1*angReduce,angPid->PID_OUT() +4*angReduce);


        }
        else if ((agentDir.th() - kickFinalDir).degree()  > 10 )
        {
            angPid->kp = 4*angReduce;

            angPid->error = ((ballPos - agentPos).th() - agent->dir().th()).radian();
            agent->setRobotVel( (-0.12 + agentPos.dist(ballPos))*4,1*angReduce,angPid->PID_OUT() - 4*angReduce) ;
        }


    }
    else
    {
        angReduce = 1;
        if(fabs((agentDir.th() - kickFinalDir).degree()) < 80)
            angReduce = 0.6;

        //agent->setRoller(1);
        if ((agentDir.th() - kickFinalDir).degree()  <- 10 )
        {
            angPid->kp = 4*angReduce;

            angPid->error = ((ballPos - agentPos).th() - agent->dir().th()).radian();
            agent->setRobotVel( 0.5 ,-1*angReduce,angPid->PID_OUT() +4*angReduce);


        }
        else if ((agentDir.th() - kickFinalDir).degree()  > 10 )
        {
            angPid->kp = 4*angReduce;

            angPid->error = ((ballPos - agentPos).th() - agent->dir().th()).radian();
            agent->setRobotVel( 0.5,1*angReduce,angPid->PID_OUT() - 4*angReduce) ;
        }
    }
}

void CSkillKick::kkKick()
{

}

void CSkillKick::kkIntercept()
{

    Vector2D stopTarget, finalDir;
    Segment2D ballPath;
    Segment2D fieldBottomline,fieldTopline,fieldLeftline,fieldRightline;
    fieldBottomline.assign(Vector2D(-1*_FIELD_WIDTH/2,-1*_FIELD_HEIGHT/2),Vector2D(_FIELD_WIDTH/2,-1*_FIELD_HEIGHT/2));
    fieldTopline.assign(Vector2D(-1*_FIELD_WIDTH/2,1*_FIELD_HEIGHT/2),Vector2D(_FIELD_WIDTH/2,1*_FIELD_HEIGHT/2));
    fieldLeftline.assign(Vector2D(-1*_FIELD_WIDTH/2,1*_FIELD_HEIGHT/2),Vector2D(-1*_FIELD_WIDTH/2,-1*_FIELD_HEIGHT/2));
    fieldRightline.assign(Vector2D(_FIELD_WIDTH/2,1*_FIELD_HEIGHT/2),Vector2D(_FIELD_WIDTH/2,-1*_FIELD_HEIGHT/2));

    Circle2D robotArea;
    Vector2D sol1,sol2;
    Vector2D bestPos;
    double ballVel = ballRealVel;
    double bTime;
    double difTime[100] = {0};
    int minNum=0;


    Circle2D nowTurn(agentPos + agentDir.norm()*0.2,0.2);
    draw(nowTurn,QColor(Qt::red));


    if(nowTurn.contains(ballPos) && ballRealVel < 0.5)
    {
        if(fabs((kickFinalDir - agentDir.th()).degree()) > 35)
            return turnForKick();
        else
            return jTurn();
    }
    else
        kickIntercept->execute();


}

void CSkillKick::kkPenalty()
{
    angPid->kp =4;
    if( ((ballPos - agentPos).th() - kickFinalDir).degree()  > 90  )
        kkMovementTheta = ((ballPos - agentPos).th().radian() + 0.8+1.5*(0.4-agentPos.dist(ballPos))- agentDir.th().radian());
    else if(((ballPos - agentPos).th() - kickFinalDir).degree() <- 90)
        kkMovementTheta = ((ballPos- agentPos).th().radian() - 0.8- 1.5*(0.4-agentPos.dist(ballPos))- agentDir.th().radian());
    else if(((ballPos - agentPos).th() - kickFinalDir).degree() >45)
        kkMovementTheta = ((ballPos- agentPos).th().radian() +0.7 + 1.5*(0.4-agentPos.dist(ballPos)) - agentDir.th().radian());
    else if(((ballPos - agentPos).th() - kickFinalDir).degree() < -45)
        kkMovementTheta = ((ballPos- agentPos).th().radian() - 0.7 - 1.5*(0.4-agentPos.dist(ballPos))- agentDir.th().radian());
    else if(((ballPos - agentPos).th() - kickFinalDir).degree() >3)
        kkMovementTheta = ((ballPos- agentPos).th().radian() +0.6 + 1.5*(0.2-agentPos.dist(ballPos)) - agentDir.th().radian());
    else if(((ballPos - agentPos).th() - kickFinalDir).degree() < -3)
        kkMovementTheta = ((ballPos- agentPos).th().radian() - 0.6 - 1.5*(0.2-agentPos.dist(ballPos))- agentDir.th().radian());
    else
        kkMovementTheta =((ballPos- agentPos).th().radian() - agentDir.th().radian());
    /////////////////////////////////////////////////////////////////////////////////
    double ballx= (wm->ball->vel.x)*cos(agentDir.th().radian()) + (wm->ball->vel.y)*sin(agentDir.th().radian());
    double bally= -1*(wm->ball->vel.x)*sin(agentDir.th().radian()) + (wm->ball->vel.y)*cos(agentDir.th().radian());
    angPid->error = (kickFinalDir - agentDir.th()).radian();
    double reduce = 0.6;
    if(interceptMode)
        agent->setRobotVel(reduce*cos(kkMovementTheta)  + 0.4*ballx,reduce*sin(kkMovementTheta)+0.4*bally ,angPid->PID_OUT());
    else
        agent->setRobotVel(reduce*cos(kkMovementTheta) + 0.2*ballx,reduce*sin(kkMovementTheta) + 0.2*bally,angPid->PID_OUT());

}

Vector2D CSkillKick::findMostPossible()
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
        if(wm->our.active(i)->id != agent->id())
            obstacles.append(Circle2D(wm->our.active(i)->pos,0.1));
    }
    double prob,angle,biggestAngle;

    knowledge->getEmptyAngle(wm->ball->pos-(wm->field->oppGoal()-ballPos).norm()*0.15,wm->field->oppGoalL(),wm->field->oppGoalR(), obstacles, prob, angle, biggestAngle);
    //debug(QString("prob: %1 , angle :%2, biggest:%3").arg(prob).arg(angle).arg(biggestAngle),D_MHMMD);

    Segment2D goalSeg(wm->field->oppGoalL(),wm->field->oppGoalR());
    Vector2D sol1,sol2;
    //    debug(QString("ang %1").arg(angle),D_MHMMD);
    draw(Segment2D(wm->ball->pos , wm->ball->pos + Vector2D(cos(_PI*(angle)/180),sin(_PI*(angle)/180)).norm()*12));

    return  goalSeg.intersection(Segment2D(wm->ball->pos , wm->ball->pos + Vector2D(cos(_PI*(angle)/180),sin(_PI*(angle)/180)).norm()*12));
}


double CSkillKick::oneTouchAngle(Vector2D pos,
                                 Vector2D vel,
                                 Vector2D ballVel,
                                 Vector2D ballDir,
                                 Vector2D goal,
                                 double landa,
                                 double gamma)
{
    float ang1 = (-ballDir).th().degree();
    float ang2 = (goal - pos).th().degree();
    float theta = AngleDeg::normalize_angle(ang2 - ang1);
    float th = fabs(theta)*_DEG2RAD;
    float vkick = 8; // agent->self()->kickValueSpeed(kickSpeed, false);// + Vector2D::unitVector(self().pos.d).innerProduct(self().vel);
    float v = (ballVel - vel).length();
    float th1 = th*0.5;
    float f,fmin=1e10;
    float th1best;
    for (int k=0;k<6000;k++)
    {
        th1 = ((float)k/6000.0)*th;
        f  = gamma*v*(1.0/tan(th-th1))*sin(th1)-landa*v*cos(th1)-vkick;
        if (fabs(f)<fmin)
        {
            fmin = fabs(f);
            th1best = th1;
        }
    }
    th1 = th1best;
    th1 *= _RAD2DEG;
    AngleDeg::normalize_angle(th1);
    th  *= _RAD2DEG;
    float ang = 0;
    if (theta>0) ang = ang1 + th1;
    else ang = ang1 - th1;

    return ang;
}

double CSkillKick::kickTimeEstimation(CAgent *_agent, Vector2D _target)
{
    QList<int> ourRelax,oppRelax;
    Vector2D finalPos;
    Vector2D ballPosInFuture;
    Vector2D s1,s2;
    Segment2D ballPath(wm->ball->pos,wm->ball->pos + wm->ball->vel.norm()*10);
    Circle2D robotAreaNear (_agent->pos(),0.4);

    if(wm->ball->vel.length() > 0.2)
    {
        if(robotAreaNear.intersection(ballPath,&s1,&s2) && wm->ball->whenBallReachToPoint(wm->ball->pos.dist(_agent->pos())) >= 0)
        {
            return wm->ball->whenBallReachToPoint(wm->ball->pos.dist(_agent->pos()));
        }
        else
        {
            for(double i = 0 ; i < 3 ; i += 0.03)
            {
                ballPosInFuture = wm->ball->getPosInFuture(i);
                finalPos = ballPosInFuture - (_target-ballPosInFuture).norm()*0.11;
                if(CSkillGotoPointAvoid::timeNeeded(_agent,finalPos,conf()->BangBang_VelMax(),ourRelax,oppRelax,true,0.2,true)<= i+0.1)
                {
                    //draw(finalPos,1,QColor(Qt::blue));
                    return i;
                }
            }
        }
    }

    finalPos = wm->ball->pos - (_target-wm->ball->pos).norm() * 0.11;
    draw(finalPos);
    return 100 - CSkillGotoPointAvoid::timeNeeded(_agent,finalPos,conf()->BangBang_VelMax(),ourRelax,oppRelax,true,0.2,false);

}

void CSkillKick::findPosToGo()
{
    Circle2D  agentNearArea(agentPos,0.15);
    Vector2D sol1,sol2;
    QList<int> ourRelax,oppRelax;
    double agentTime = 0;
    Vector2D finalDir;
    Segment2D ballPath(ballPos,ballPos + wm->ball->vel.norm()*10);
    Circle2D dribblerArea(agentPos+agentDir.norm()*0.1,0.25);
    Circle2D robotArea(agentPos,1);

    gpa->setAddVel(Vector2D(0,0));
    if(agentPos.dist(ballPos) < 1.5) {
        robotArea.assign(agentPos,max(agentPos.dist(ballPos) - 0.1, 0.01));
    }

    if(agentPos.dist(ballPos) < 0.15)
    {
        agentNearArea.assign(agentPos,agentPos.dist(ballPos) - 0.01);
    }

    if(wm->ball->vel.length() > 0.2 )
    {
        for(double i = 0 ; i < 5 ; i += 0.1)
        {

            finalPos = wm->ball->getPosInFuture(i);// - (target-wm->ball->getPosInFuture(i)).norm()*0.15;
            agentTime = CSkillGotoPointAvoid::timeNeeded(agent,finalPos,conf()->BangBang_VelMax(),ourRelax,oppRelax,!goalieMode,0.2,true);
            if(agentTime < i +0.5)
            {
                break;
            }
        }
        if((fabs(((ballPos - agentPos).th() - kickFinalDir).degree()) < 60) )
        {
            finalDir = Vector2D(cos(kickFinalDir.radian()),sin(kickFinalDir.radian()));
        }
        else
        {
            finalDir = Vector2D(cos(kickFinalDir.radian()),sin(kickFinalDir.radian()));
        }
    }

    else
    {
        finalPos = ballPos - (target-finalPos).norm() * 0.15;
        finalDir = Vector2D(cos(kickFinalDir.radian()),sin(kickFinalDir.radian()));
    }

    Vector2D oneTouchPos = ballPath.nearestPoint(agentPos);
    Segment2D kickerSeg(agentPos+agent->dir().norm()*0.08+agent->dir().rotate(90).norm()*0.02 ,agentPos+agent->dir().norm()*0.08-agent->dir().rotate(90).norm()*0.02 );
    bool canOneTouch = false;
    if(robotArea.intersection(ballPath,&sol1,&sol2) > 1 && wm->ball->vel.length() > 0.5 )
    {
        for(double i = 0 ; i < 5 ; i+=0.1)
        {
            if(wm->ball->getPosInFuture(i).dist(oneTouchPos) <= 0.2)
            {
                if(CSkillGotoPointAvoid::timeNeeded(agent,oneTouchPos,conf()->BangBang_VelMax(),ourRelax,oppRelax,!goalieMode,0,true) <= i+0.1)
                {
                    canOneTouch =true;
                    break;
                }
            }
        }

        if((canOneTouch || kickerSeg.intersection(ballPath).isValid()) && !sagMode )
        {
            debug("oneTOUCH",D_MHMMD);
            if( ( fabs(((target-agentPos).th().degree() - (ballPos-agentPos).th().degree() )) < 60 ))
                waitAndKick();
            else
                kWaitForTurn();


            return;
        }

    }

    Circle2D oppPenalty(wm->field->oppGoal() + Vector2D(0.2 , 0),1.4);
    gpa->setOneTouchMode(false);

    if(oppPenalty.contains(ballPos))
    {
        finalPos = finalPos - (target-finalPos).norm() * 0.15;
    }
    if(finalPos.x > _FIELD_WIDTH)
    {
        finalPos = knowledge->getReflectPos(wm->field->oppGoal(), 3);
    }

    if((fabs(((ballPos - agentPos).th() - kickFinalDir).degree()) < 60))
    {
        finalPos = finalPos - (target-finalPos).norm() * 0.1;
    }
    Vector2D s1,s2;
    Circle2D finalPosArea;
    draw(finalPosArea,QColor(Qt::blue));

    Segment2D directPath(agentPos,finalPos);
    draw(directPath);
    finalPosArea.assign(ballPos ,0.145);
    if(finalPosArea.intersection(directPath,&s1,&s2))
    {
        gpa->setOneTouchMode(false);

        finalPosArea.assign(ballPos ,0.245);
        finalPosArea.tangent(agentPos,&s1,&s2);
        if(s2.dist(target) >= s1.dist(target))
            s1 = s2;
        s1 = s1 + (s1 - agentPos).norm()*(finalPos.dist(ballPos))*1.8;
        finalPos = s1;
        //gpa->setAddVel(wm->ball->vel);
    }
    else
    {
        gpa->setAddVel(Vector2D(0,0));
    }

    gpa->init(finalPos,finalDir);
    gpa->setAvoidBall(false);
    gpa->setNoAvoid(false);

    if(wm->ball->vel.length() > 0.3)
        gpa->setBallObstacleRadius(0);
    else
        gpa->setBallObstacleRadius(0);
    if(((fabs(((ballPos - agentPos).th() - kickFinalDir).degree()) < 100) && (agentPos.dist(ballPos) < 1) && (wm->ball->vel.length() > 0.2)) || (agentPos.dist(ballPos) < 0.5)) {
        if(fabs((kickFinalDir - agentDir.th()).degree()) > 30 && dribblerArea.contains(ballPos))
        {
            turnForKick();
        }
        else
        {
            jTurn();
        }
        return;
    }

    gpa->setBallObstacleRadius(0);
    gpa->setSlowMode(slow);
    gpa->setADiveMode(false);
    gpa->setAvoidPenaltyArea(true);
    gpa->execute();


}

void CSkillKick::findPosToGoAlt()
{
#define vRobotTemp 2
    Circle2D ballArea(ballPos,0.2);
    Vector2D sol1,sol2;
    if(wm->ball->vel.length() > 0.2)
    {
        for(double i = 0 ; i < 5 ; i += 0.1)
        {
            if(wm->ball->getPosInFuture(i).dist(agentPos)/vRobotTemp < i)
            {
                finalPos = wm->ball->getPosInFuture(i) - (target-wm->ball->getPosInFuture(i)).norm()*0.16;
                break;
            }
        }
    }

    else
    {
        finalPos = ballPos - (target-ballPos).norm() * 0.16;
    }

    if(finalPos.x > _FIELD_WIDTH)
    {
        finalPos = knowledge->getReflectPos(wm->field->oppGoal(), 3);
    }

    Vector2D finalDir;
    Circle2D dribblerArea(agentPos+agentDir.norm()*0.1,0.1);

    if(agentPos.dist(ballPos) > 1)
    {
        finalDir = wm->ball->pos - agentPos;
    }
    else if(!dribblerArea.contains(ballPos))
    {
        finalDir = (agentPos.dist(ballPos)) * (wm->ball->pos - agentPos) + (1 - agentPos.dist(ballPos))*(target-agentPos) ;
    }
    else
    {
        finalDir = target-agentPos;
    }
    gpa->init(finalPos,finalDir);
    gpa->setAvoidBall(true);

    gpa->setBallObstacleRadius(1);
    if((fabs(((ballPos - agentPos).th() - kickFinalDir).degree()) < 60) && (agentPos.dist(ballPos) < 1) && (wm->ball->vel.length() > 0.2)) {
        jTurn();
        return;
    }

    gpa->setAvoidPenaltyArea(false);
    gpa->execute();


}

void CSkillKick::execute()
{

    ballRealVel = wm->ball->vel.length();
    agentPos = agent->pos();
    kkDist = agentPos.dist(target);
    ballPos = wm->ball->pos;
    kkBallDist = agentPos.dist(ballPos);
    AngleDeg maxAngP,maxAngN;
    AngleDeg kickTargetDir ;

    draw(Segment2D(agentPos,agentPos+agentDir*10));
    if(kkShotEmpySpot)
        target = findMostPossible();
    ///////////////dir correction
    /// ballSpeed
    double ballVelCoef = 1,robotVelCoef = 1;
    double robotVelDif = 0,ballVelDif = 0;
    double finalVelDif = 0;
    kickTargetDir= (target - ballPos).th();
    if(wm->ball->vel.length() > 0.5)
    {
        ballVelDif = ballVelCoef * (Vector2D::angleBetween((target -ballPos).norm(),wm->ball->vel.norm()).degree())*wm->ball->vel.length();
        robotVelDif = robotVelCoef*(Vector2D::angleBetween((target -ballPos).norm(),agent->vel().norm()).degree())*agent->vel().length();
        finalVelDif = ballVelDif + robotVelDif;
    }
    dirQueue.append(agent->dir());
    if(dirQueue.count() == 10) {
        dirQueue.dequeue();
    }
    agentDir = Vector2D(0,0);
    Q_FOREACH(Vector2D v,dirQueue) {
        agentDir += v;
    }
    agentDir /= dirQueue.size();
    agentDir = agent->dir();
    maxAngP = (target - ballPos).th() + kickAngTol;
    maxAngN = (target - ballPos).th() - kickAngTol;


    alternateMode = false;

    //    debug(QString("dist: %1").arg(agentPos.dist(jTurnStartPos)),D_MHMMD);
    if(alternateMode)
    {
        if((ballPos - agentPos).th().degree() > kickAngTol)
        {
            kickFinalDir = maxAngP;
        }
        else if((ballPos- agentPos).th().degree() < -1 * kickAngTol)
        {
            kickFinalDir = maxAngN;
        }
        else
        {
            kickFinalDir = ( ballPos-agentPos).th();
        }


        if(kickerOn) {
            kickFinalDir = kickTargetDir;
        }
    }
    else
    {
        kickFinalDir = kickTargetDir;
    }
    finalDirVec = target - ballPos;
    oneTouchDir=Vector2D::unitVector(oneTouchAngle(agentPos, agent->vel(), wm->ball->vel, agentPos - ballPos, target, conf()->SkillsParams_KickOneTouch_Landa(), conf()->SkillsParams_KickOneTouch_Gamma()));



    gpa->setAgent(agent);
    kickMode = decideMode();

    if(kickMode != KJTURN)
    {
        isJturn = false;
    }


    if(dontKick)
    {
        agent->setKick(0);
        agent->setChip(0);
    }
    else if(kickMode == KWAITANDKICK)
    {
        if(Circle2D(agentPos,1).contains(ballPos) && fabs((agentDir.th() - oneTouchDir.th()).degree()) < 2)
        {
            agent->setRoller(spin);
            if(chip)
                agent->setChip(kickSpeed);
            else
                agent->setKick(kickSpeed);
        }
        else
        {

            agent->setKick(0);
            agent->setChip(0);
        }
    }
    else if(veryFine)
    {
        if(kickerOn && fabs((agentDir.th() - kickTargetDir).degree()) < 2)
        {
            if(chip)
                agent->setChip(kickSpeed);
            else
                agent->setKick(kickSpeed);
        }
        else
        {
            agent->setKick(0);
            agent->setChip(0);
        }
    }

    else
    {
        if(kickerOn && fabs((agentDir.th() - kickTargetDir).degree()) < tol || kickerOn && fabs((agentDir.th() - kickTargetDir).degree()) < 3)
        {

            if(chip)
                agent->setChip(kickSpeed);
            else
                agent->setKick(kickSpeed);
        }
        else
        {
            agent->setKick(0);
            agent->setChip(0);
        }
    }

    //    if(agentPos.dist(jTurnStartPos) > 0.5 && !slow && !penaltyKick )
    //    {
    //        agent->setChip(1);
    //        jTurnStartPos = agentPos;
    //    }


    if(kkShotEmpySpot)
        target = findMostPossible();
    switch(kickMode)
    {
    case KDIRECT:
        direct();
        debug("DIRECT",D_KK);
        break;
    case KINDIRECCT:
        indirect();
        debug("INDIRECT",D_KK);
        break;
    case KJTURN:
        jTurn();
        debug("JTURN",D_KK);
        break;
    case KTURN:
        turnForKick();
        debug("TURN",D_KK);
        break;
    case KINTERCEPT:
        kkIntercept();
        debug("INTERCEPT",D_KK);
        break;
    case KPENALTY:
        kkPenalty();
        debug("PENALTY",D_KK);
        break;
    case KAVOIDOPPENALTY:
        if(wm->ball->pos.x  >0)
        {
            avoidOppPenalty();
        }
        else
        {
            avoidOurPenalty();
        }
        break;
    case KWAITANDKICK:
        waitAndKick();
        break;
    case KWAITFORTURN:
        kWaitForTurn();
        break;
    case KDONTKICK:
        kDontKick();
        break;
    case KGOALIE:
        kgoalie();
        break;


    }




}
//------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////


//------------------------------

INIT_SKILL(CSkillKickOneTouch, "kickonetouch");

CSkillKickOneTouch::CSkillKickOneTouch(CAgent *_agent) : CSkill(_agent)
{
    pidP = new CPID();
    pidW = new CPID();
    gotopointavoid = new CSkillGotoPointAvoid(agent);
    gotopointavoid->setAgent(_agent);
    kick = new CSkillKick(_agent);
    timeAfterForceKick = new QTime();
    timeAfterForceKick->start();
    forceKicked = false;
    waitpos.invalidate();
    kickSpeed = 800;
    distToBallLine = 0.0;
    velToBallLine = 0.0;
    chip = false;
    oneTouched = false;
    p_area_avoidance_state = -1;
    moveTowardTheBall = false;
    cirThresh = 0;
}

CSkillKickOneTouch::~CSkillKickOneTouch()
{
    delete gotopointavoid;
    delete kick;
    delete timeAfterForceKick;
    delete pidP;
    delete pidW;
}

double CSkillKickOneTouch::progress()
{
    return 0.0;
}

double CSkillKickOneTouch::oneTouchAngle(Vector2D pos,
                                         Vector2D vel,
                                         Vector2D ballVel,
                                         Vector2D ballDir,
                                         Vector2D goal,
                                         double landa,
                                         double gamma)
{
    float ang1 = (-ballDir).th().degree();
    float ang2 = (goal - pos).th().degree();
    float theta = AngleDeg::normalize_angle(ang2 - ang1);
    float th = fabs(theta)*_DEG2RAD;
    float vkick = 8; // agent->self()->kickValueSpeed(kickSpeed, false);// + Vector2D::unitVector(self().pos.d).innerProduct(self().vel);
    float v = (ballVel - vel).length();
    float th1 = th*0.5;
    float f,fmin=1e10;
    float th1best;
    for (int k=0;k<6000;k++)
    {
        th1 = ((float)k/6000.0)*th;
        f  = gamma*v*(1.0/tan(th-th1))*sin(th1)-landa*v*cos(th1)-vkick;
        if (fabs(f)<fmin)
        {
            fmin = fabs(f);
            th1best = th1;
        }
    }
    th1 = th1best;
    th1 *= _RAD2DEG;
    AngleDeg::normalize_angle(th1);
    th  *= _RAD2DEG;
    float ang = 0;
    if (theta>0) ang = ang1 + th1;
    else ang = ang1 - th1;

    return ang;
}

kkOTMode CSkillKickOneTouch::decideMode()
{
    Circle2D tempCircle(waitpos, 1 + cirThresh);
    Circle2D tempCircle2(waitpos, 1.5);
    draw(tempCircle, QColor(Qt::cyan));

    return OTWAITPOS;

}


Vector2D CSkillKickOneTouch::findMostPossible()
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
        if(wm->our.active(i)->id != agent->id())
            obstacles.append(Circle2D(wm->our.active(i)->pos,0.1));
    }
    double prob,angle,biggestAngle;

    knowledge->getEmptyAngle(wm->ball->pos-(wm->field->oppGoal()-wm->ball->pos).norm()*0.15,wm->field->oppGoalL(),wm->field->oppGoalR(), obstacles, prob, angle, biggestAngle);
    //debug(QString("prob: %1 , angle :%2, biggest:%3").arg(prob).arg(angle).arg(biggestAngle),D_MHMMD);

    Segment2D goalSeg(wm->field->oppGoalL(),wm->field->oppGoalR());
    Vector2D sol1,sol2;
    //    debug(QString("ang %1").arg(angle),D_MHMMD);
    draw(Segment2D(wm->ball->pos , wm->ball->pos + Vector2D(cos(_PI*(angle)/180),sin(_PI*(angle)/180)).norm()*12));

    return  goalSeg.intersection(Segment2D(wm->ball->pos , wm->ball->pos + Vector2D(cos(_PI*(angle)/180),sin(_PI*(angle)/180)).norm()*12));
}


void CSkillKickOneTouch::execute()
{
    ballRealVel = knowledge->getRealBallVel();
    gotopointavoid->setAgent(agent);
    gotopointavoid->setOneTouchMode(false);
    gotopointavoid->setNoAvoid(false);

    if(shotToEmptySpot)
        target = findMostPossible();
    if (!target.valid()) target = wm->field->oppGoal();



    Vector2D ballPos = wm->ball->pos;
    Vector2D agentPos = agent->pos();
    oneTouchMode = decideMode();

    Segment2D ballPath;
    double stopParam = 0.08;
    ballPath.assign(ballPos,ballPos + wm->ball->vel.norm()*15);
    Segment2D ballLine;
    ballLine.assign(ballPos,ballPos + wm->ball->vel.norm()*(15));
    draw(ballPath,"red");
    Vector2D kickerPoint = agentPos + agent->dir().norm()*stopParam;



    Vector2D oneTouchDir = Vector2D::unitVector(oneTouchAngle(agentPos, agent->vel(), wm->ball->vel, agentPos - ballPos, target, conf()->SkillsParams_KickOneTouch_Landa(), conf()->SkillsParams_KickOneTouch_Gamma()));

    draw(Segment2D(Vector2D(0,0), Vector2D(0,0)+oneTouchDir.norm()), QColor(Qt::red));

    Vector2D addVec = (agentPos - target).norm()*stopParam;
    Vector2D intersectPos;
    Vector2D sol1,sol2;
    double onetouchRad =0.5;
    double onetouchKickRad = 0.5;
    Circle2D oneTouchArea;
    Circle2D oppPenaltyArea(wm->field->oppGoal() + Vector2D(0.15,0),1.45);
    Circle2D oppPenaltyAreaWP(wm->field->oppGoal() + Vector2D(0.15,0),1.55);
    draw(oppPenaltyAreaWP,QColor(Qt::red));
    ///temp
    draw(oppPenaltyArea,QColor(Qt::red));

    if(ballPos.dist(agentPos) <= onetouchRad)
        onetouchRad = ballPos.dist(agentPos)-stopParam;
    oneTouchArea.assign(agentPos,onetouchRad);

    if((wm->ball->vel.length() < 0.4 && agentPos.dist(ballPos) > onetouchKickRad) || (!oneTouchArea.intersection(ballPath,&sol1,&sol2) && wm->ball->vel.length() >= 0.4 && agentPos.dist(ballPos) > onetouchKickRad))
    {
        if(!waitpos.isValid())
            waitpos = agentPos;
        gotopointavoid->init(waitpos,oneTouchDir);
        gotopointavoid->execute();
    }
    else if(oneTouchArea.intersection(ballPath,&sol1,&sol2) && wm->ball->vel.length() > 0.1)
    {
        gotopointavoid->setNoAvoid(false);
        intersectPos = ballPath.nearestPoint(kickerPoint);
        if(wm->field->isInOppPenaltyArea(intersectPos) || oppPenaltyAreaWP.contains(waitpos))
        {
            if(oppPenaltyArea.intersection(ballLine,&sol1,&sol2))
            {
                if(sol1.dist(waitpos) > sol2.dist(waitpos))
                {
                    sol1 = sol2;
                }
                intersectPos = sol1;
            }
        }


        gotopointavoid->init(intersectPos +addVec,oneTouchDir);
        gotopointavoid->setNoAvoid(true);
        gotopointavoid->setOneTouchMode(true);
        gotopointavoid->execute();
        draw(intersectPos);
        if(agentPos.dist(ballPos) < 1)
        {
            if(chip)
            {
                agent->setChip(kickSpeed);
            }
            else
            {
                agent->setKick(kickSpeed);
            }
        }
        agent->setRoller(3);
    }
    else if(ballPos.dist(agentPos) < onetouchKickRad)
    {
        kick->setAgent(agent);
        kick->setTarget(target);
        kick->setKickSpeed(kickSpeed);
        kick->setChip(chip);
        kick->execute();
    }
    else
    {
        if(!waitpos.isValid())
            waitpos = agentPos;
        gotopointavoid->init(waitpos,oneTouchDir);
        gotopointavoid->execute();
    }


}



INIT_SKILL(CSkillIntercept, "intercept");

CSkillIntercept::CSkillIntercept(CAgent *_agent)
{
    iy = 0;
    gotopoint = new CSkillGotoPoint(_agent);
    gotopointavoid = new CSkillGotoPointAvoid(_agent);
    setAgent(_agent);
    target = NULL;
    timeFactor = 1.0;
    kickMode = false;
    behindTarget = -1;
    behindTargetDist = 0.05;
    behindTargetAngle = 20;
    behindTargetAcc = 0.0;
    behindTargetVel.assign(0.0, 0.0);
    pidPX = new CPID();
    pidPX->reset();
    pidPY = new CPID();
    pidPY->reset();
    pidWB = new CPID();
    pidWB->reset();
    aside = -1;
    onetouching = false;
    avoidance1 = avoidance2 = -1;
    avoidPenaltyArea = true;
    calcd = false;
    doOneTouch = false;
    p_avoidance_state = -1;
    sagMode = false;
    clear = false;
    dontKick = false;
    isBallFrozen = true;
    minNum =10;
}

CSkillIntercept::~CSkillIntercept()
{
}

void CSkillIntercept::execute()
{
    Vector2D stopTarget, finalDir;
    Segment2D ballPathAgent;
    Segment2D ballPath;
    Segment2D fieldBottomline,fieldTopline,fieldLeftline,fieldRightline;
    fieldBottomline.assign(Vector2D(-1*_FIELD_WIDTH/2,-1*_FIELD_HEIGHT/2),Vector2D(_FIELD_WIDTH/2,-1*_FIELD_HEIGHT/2));
    fieldTopline.assign(Vector2D(-1*_FIELD_WIDTH/2,1*_FIELD_HEIGHT/2),Vector2D(_FIELD_WIDTH/2,1*_FIELD_HEIGHT/2));
    fieldLeftline.assign(Vector2D(-1*_FIELD_WIDTH/2,1*_FIELD_HEIGHT/2),Vector2D(-1*_FIELD_WIDTH/2,-1*_FIELD_HEIGHT/2));
    fieldRightline.assign(Vector2D(_FIELD_WIDTH/2,1*_FIELD_HEIGHT/2),Vector2D(_FIELD_WIDTH/2,-1*_FIELD_HEIGHT/2));
    Vector2D kkBallPos = wm->ball->pos;
    Vector2D kkAgentPos = agent->pos();
    Circle2D robotArea;
    Vector2D sol1,sol2;
    Vector2D bestPos;
    double ballVel = wm->ball->vel.length();
    double bTime;
    double difTime[100] = {0};


    ballPath.assign(kkBallPos,kkBallPos+wm->ball->vel.norm()*(10));
    ballPathAgent.assign(kkBallPos,kkBallPos+wm->ball->vel.norm()*(kkBallPos.dist(kkAgentPos)));
    robotArea.assign(kkAgentPos,0.3);
    if(wm->ball->vel.length() < 0.5)
        isBallFrozen = true;

    if(robotArea.intersection(ballPath,&sol1,&sol2) ==0 && ballVel >= 0.2)
    {

        bestPos = wm->ball->pos + wm->ball->vel.norm()*(ballVel*1.1);

        if(!wm->field->isInField(bestPos))
        {
            if(ballPath.intersection(fieldBottomline).isValid())
                bestPos = ballPath.intersection(fieldBottomline);
            else if(ballPath.intersection(fieldTopline).isValid())
                bestPos = ballPath.intersection(fieldTopline);
            else if(ballPath.intersection(fieldRightline).isValid())
                bestPos = ballPath.intersection(fieldRightline);
            else if(ballPath.intersection(fieldLeftline).isValid())
                bestPos = ballPath.intersection(fieldLeftline);
        }
        draw(bestPos);
        stopTarget = bestPos;
        finalDir = kkBallPos - bestPos;

    }
    else
    {
        stopTarget = ballPathAgent.nearestPoint(kkAgentPos);
        finalDir = kkBallPos - kkAgentPos;
    }
    draw(QString("minNum: %1").arg(minNum),Vector2D(0,1));
    draw(ballPath);
    gotopointavoid->setAgent(agent);
    gotopointavoid->setSlowMode(false);
    gotopointavoid->init(stopTarget,finalDir);
    gotopointavoid->execute();
    return;
}


Vector2D CSkillIntercept::avoidTarget(CAgent* agent, CMovingObject* target, Vector2D q, Vector2D goal, Vector2D goal2, int& state)
{
    double a0 = (Vector2D::angleBetween((goal - target->pos), agent->pos()-target->pos).degree());
    Vector2D b0 = (goal - q).norm().rotatedVector(90)*(agent->self()->robotRadius()+0.03)*(agent->vel().length()/2.0 + 1.0 + 0.1);
    if (fabs(a0) < 80.0 || state!=-2)
    {
        if (state == -2) state = -1;
        double r = (agent->self()->getKickerPos() - q).length();
        double r2 = r;
        if (r2 > 0.3) r2 = 0.4;
        r2 *= 0.8;
        double theta0 = asin(r2 / r);
        double r3 = r * cos(theta0);
        theta0 *= _RAD2DEG;
        double a1 = AngleDeg::normalize_angle((q - agent->self()->getKickerPos()).th().degree() + theta0);
        double a2 = AngleDeg::normalize_angle((q - agent->self()->getKickerPos()).th().degree() - theta0);
        Vector2D p1 = Vector2D::unitVector(a1) * r3 + agent->self()->getKickerPos();
        Vector2D p2 = Vector2D::unitVector(a2) * r3 + agent->self()->getKickerPos();
        a1 = Vector2D::angleBetween(goal2 - q, p1 - q).degree();
        a2 = Vector2D::angleBetween(goal2 - q, p2 - q).degree();
        if (state == -1 || state == -2)
        {
            if (fabs(a1) > fabs(a2))
            {
                state = 1;
            }
            else
                state = 2;
        }
        else {
            if ((fabs(a1) - fabs(a2)) > 30.0)
            {
                state = 1;
            }
            if ((fabs(a1) - fabs(a2)) < -30.0)
            {
                state = 2;
            }
        }
        if (state == 1) q = p1;
        else q = p2;
        //        draw (QString("state=%1").arg(state), goal2, "red", 12 );
        draw(Segment2D(agent->self()->getKickerPos(), q), "brown");
    }
    else state = -2;
    if (fabs(a0) > 160.0)
        state = -2;
    return q;
    //            if (aside == -1)
    //            {
    //                if (a0 > 0) aside = 0;
    //                else aside = 1;
    //            }
    //            if (a0 >  10) aside = 0;
    //            if (a0 < -10) aside = 1;
    //            if (fabs(a0) > 80) aside = -1;
    //            if (aside != -1)
    //            {
    ////                debug(QString("asided=%1").arg(a0), D_ALI);
    //            }
    //            if (aside == 0)
    //                q = q + b0;
    //            else if (aside == 1)
    //                q = q - b0;
}

double CSkillIntercept::progress()
{
    return gotopoint->progress();
}

CSkillIntercept* CSkillIntercept::setTarget(CMovingObject *val)
{
    target = val;
    if (val==NULL) return this;
    gotopoint->setAgent(agent);
    gotopoint->init(target->pos, targetDir, target->vel, false);
    return this;
}

INIT_SKILL(CSkillNewPass, "new-pass");
CSkillNewPass::CSkillNewPass(CAgent *_agent): CSkill(_agent)
{
    agent=_agent;

    pkeep = new CSkillKeep(agent);
    pKick = new CSkillKick(agent);
    gpa = new CSkillGotoPointAvoid(agent);

    shotOrNot = true;
    isPasserNearTheBall = false;
    spinFtime.start();
    spinFtime.restart();
}
CSkillNewPass::~CSkillNewPass()
{

    delete pKick;
    delete pkeep;
}

double CSkillNewPass::progress()
{
    return 0;
}

void CSkillNewPass::execute()
{
    QList<Circle2D> OppObstacles;
    OppObstacles.clear();
    Circle2D ballArea(wm->ball->pos,0.8);
    bool pathclear = true;
    for(int i = 0 ; i < wm->opp.activeAgentsCount() ; i++)
    {
        if(ballArea.contains(wm->opp.active(i)->pos))
            OppObstacles.append(Circle2D(wm->opp.active(i)->pos , 0.12));
    }
    Vector2D dummy1,dummy2;
    QList<Vector2D> sols;
    double maxdist = 0;
    double dummydist =0;
    Vector2D bestsol;
    Circle2D kickerArea (agent->pos() + agent->dir().norm()*0.08,0.05);


    sols.clear();
    for(int i = 0 ; i < 2*OppObstacles.count() ; i++)
    {
        sols.append(Vector2D(0,0));
    }
    Segment2D ballPath(wm->ball->pos,target);
    for(int i = 0 ; i < OppObstacles.count() ; i++)
    {
        if(OppObstacles[i].intersection(ballPath,&dummy1,&dummy2))
        {
            OppObstacles[i].tangent(wm->ball->pos,&sols[2*i],&sols[2*i+1]);
            pathclear = false;
        }
    }
    sols.removeAll(Vector2D(0,0));

    for(int i = 0 ; i < sols.count() ; i++)
    {
        dummydist = 0;
        for(int j = 0; j < OppObstacles.count() ; j++)
        {
            dummydist += sols[i].dist(OppObstacles[j].center());
        }
        if(dummydist >= maxdist)
        {
            bestsol = sols[i];
            maxdist = dummydist;
        }
        draw(sols[i] , 2, QColor(Qt::black));
    }

    draw(bestsol,3,QColor(Qt::blue));
    if(!shotOrNot)
    {
        pKick->setTarget(agent->pos() + (agent->pos() - wm->field->ourGoal()).norm());
        pKick->setSlow(false);
        pKick->setSagMode(true);
        pKick->setKickSpeed(0);
        pKick->setChip(0);

        pKick->execute();
        if(OppObstacles.count() > 0)
        {
            agent->setRoller(3);
        }
        spinFtime.restart();
    }
    else
    {
        if(!pathclear)
        {
            gpa->init(wm->ball->pos - (wm->ball->pos- agent->pos()).norm()*0.04,wm->ball->pos-agent->pos());
            agent->setRoller(3);
            // if(kickerArea.contains(wm->ball->pos))
            //                agent->setRobotVel(0.8,-0.5,3);
            //            else
            //                gpa->execute();

            if(spinFtime.elapsed() < 1000)
            {
                gpa->execute();
            }
            else
            {
                agent->setRobotVel(-0.5,0,0);
            }
        }
        else
        {
            spinFtime.restart();
            pKick->setTarget(target);
            pKick->setSlow(slow);
            pKick->setDontKick(false);
            pKick->setKickSpeed(passSpeed);
            pKick->setChip(chip);
            pKick->execute();
        }
    }
}

INIT_SKILL(CSkillPass, "pass");

CSkillPass::CSkillPass(CAgent *_agent)
{
    agent=_agent;
    GPAP = new CSkillGotoPointAvoid(agent);
    GPAR = new CSkillGotoPointAvoid(agentReceiver);
    pKick = new CSkillKick(agent);
    RcvP = new CSkillReceivePass(agentReceiver);
    stopPasser = false;
    passComplited = false;
    isPasserNearTheBall = false;
}
void CSkillPass::setReceiverAgent(CAgent *_agent)
{
    agentReceiver = _agent;
}
double CSkillPass::progress()
{
    return 0;
}
void CSkillPass::reset()
{
    stopPasser = false;
    passComplited = false;
    isPasserNearTheBall = false;
}

void CSkillPass::isPassComplited()
{
    Segment2D ballPath(wm->ball->pos,wm->ball->pos+wm->ball->vel.norm()*10);
    Circle2D targetArea(target,wm->ball->vel.length()/5 + 1.5);

    if(passProfiler)
        targetArea.assign(agentReceiver->pos(),0.5);

    Vector2D sol1,sol2;
    if(!stopPasser)
    {
        if(agent->pos().dist(wm->ball->pos) < 0.20)
            isPasserNearTheBall = true;
        draw(QString("k: %1").arg(isPasserNearTheBall),Vector2D(0,2));

        if(isPasserNearTheBall && wm->ball->vel.length() >= 1 && agent->pos().dist(wm->ball->pos) > 0.40)
        {
            stopPasser = true;
        }

    }

    if(passProfiler)
    {
        if(targetArea.contains(wm->ball->pos) && wm->ball->vel.length() < 0.5)
            passComplited = true;
    }
    else
    {
        if(targetArea.contains(wm->ball->pos) )
            passComplited = true;
    }
}

void CSkillPass::setPasser(CAgent *val)
{
    agent = val;
}

void CSkillPass::execute()
{

    pKick->setAgent(agent);
    pKick->setTarget(target);
    pKick->setKickSpeed((int)(kickSpeed));
    pKick->setSlow(slow);
    pKick->setPassProfiler(passProfiler);
    pKick->setInterceptMode(false);
    pKick->setPenaltyKick(false);
    pKick->setChip(chip);
    isPassComplited();
    if(passProfiler)
    {
        if(stopPasser == false && wm->ball->vel.length() < 0.3)
            pKick->execute();
    }
    else if(stopPasser == false)
        pKick->execute();
    RcvP->setTarget(target);
    RcvP->setAgent(agentReceiver);
    if(!passComplited)
        RcvP->execute();
}

CSkillPass::~CSkillPass()
{

    delete GPAP;
    delete GPAR;
    delete pKick;
}


INIT_SKILL(CSkillKeep, "keepTheBall");

CSkillKeep::CSkillKeep(CAgent *_agent): CSkill(_agent)
{
    agent=_agent;
    GPA = new CSkillGotoPointAvoid(agent);
    intercept = new CSkillIntercept(agent);
    ballAreaRad = 0.11;
    slow = false;
    angPid = new _PID(2,0,0,0,0);
    maximumAgentNum = 6;

}

keepMode CSkillKeep::decideMode()
{
    if(ballVel.length() > 0.5 && ballPos.dist(agentPos) > 0.5)
        return FARMOVING;
    else if(ballPos.dist(agentPos) > 0.5)
        return FARSTOP;
    else if(findCritAgents() == 0)
        return RELAX;
    else
        return DISTURB;

}

void CSkillKeep::farStop()
{
    finalDir = ballPos - agentPos;

    GPA->init(wm->ball->pos + (agentPos - wm->ball->pos).norm()*0.12,finalDir);
    initPos = agentPos;
    GPA->setSlowMode(false);
    GPA->setADiveMode(false);
    GPA->execute();
}
int CSkillKeep::findCritAgents()
{
    int critAgentCount=0;
    critAgents.clear();
    if(wm->opp.activeAgentsCount() == 0)
        return 0;
    for(int i = 0 ; i < wm->opp.activeAgentsCount() ; i++)
    {
        if(ballPos.dist(wm->opp.active(i)->pos) <= 3)
        {
            critAgents.append(wm->opp.active(i));
            critAgentCount++ ;
        }
    }

    ///////////////////some junk

    //////////
    return critAgentCount;
}

void CSkillKeep::disturb()
{
    Vector2D sol1,sol2;
    Segment2D critLine;

    Vector2D tempPos(0,0);
    Vector2D tempPosNormal(0,0);
    int veryCritAgentNum = 0;
    int normalCrit = 0;
    findCritAgents();
    //    if(findCritAgents() == 1)
    //    {
    //        critLine.assign(ballPos,critAgents.at(0)->pos);
    //        ballArea.intersection(critLine,&sol1,&sol2);
    //        if(ballPos.dist(critAgents.at(0)->pos) <= 0.15)
    //            waitPos = critAgents.at(0)->pos;
    //        else
    //            waitPos = sol1;
    //        finalDir = ballPos - waitPos;
    //    }
    //    else
    //    {
    //        for(int i= 0 ; i < findCritAgents() ; i++)
    //        {
    //            if(ballPos.dist(critAgents.at(i)->pos) < 0.4)
    //            {
    //                tempPos = tempPos + critAgents.at(i)->pos ;
    //                veryCritAgentNum ++;
    //            }
    //            else
    //            {
    //                tempPosNormal = tempPosNormal + critAgents.at(i)->pos;
    //                normalCrit ++;
    //            }

    //        }
    //        tempPos = tempPos / veryCritAgentNum;
    //        tempPosNormal = tempPosNormal /normalCrit;


    //        if(veryCritAgentNum == 0)
    //            tempPos = tempPosNormal;

    //        critLine.assign(ballPos,tempPos);
    //        ballArea.intersection(critLine,&sol1,&sol2);
    //        if(ballPos.dist(tempPos) <= 0.15)
    //            waitPos = tempPos;
    //        else
    //            waitPos = sol1;
    //        finalDir = ballPos - waitPos;
    //    }


    //    if ((agent->dir().th() - finalDir.th()).degree()  <- 50 )
    //    {
    //        angPid->kp = 3;
    //        angPid->error = ((ballPos - agentPos).th() - agent->dir().th()).radian();
    //        draw(QString("ang: %1 ").arg((ballPos - agentPos).th().radian() - _PI/2),Vector2D(0,0),"red");
    //        if(slow)
    //            agent->setRobotVel(agentPos.dist(ballPos) ,-1,angPid->PID_OUT() +5 );
    //        else
    //            agent->setRobotVel(agentPos.dist(ballPos) ,-1.4,angPid->PID_OUT() +5 );


    //    }
    //    else if ((agent->dir().th() - finalDir.th()).degree()  > 50 )
    //    {
    //        angPid->kp = 3;
    //        //debug(QString("50"),D_HOSSEIN);
    //        angPid->error = ((ballPos - agentPos).th() - agent->dir().th()).radian();

    //        if(slow)
    //            agent->setRobotVel( agentPos.dist(ballPos),1,angPid->PID_OUT() - 5);
    //        else
    //            agent->setRobotVel( agentPos.dist(ballPos),1.4,angPid->PID_OUT() - 5) ;


    //    }

    CRobot *oppNearest;
    double minDist = 10000;
    for(int i = 0; i < critAgents.count() ; i++) {
        if(critAgents[i]->pos.dist(ballPos) <= minDist) {
            oppNearest = critAgents[i];
            minDist = critAgents[i]->pos.dist(ballPos);
        }
    }


    critLine.assign(oppNearest->pos,ballPos);


    if(fabs(Vector2D::angleOf(initPos,ballPos,oppNearest->pos).degree()) < 45 || minDist >=1 ){
        ballArea.intersection(critLine,&sol1,&sol2);
        waitPos = sol1;
    }
    else
    {
        waitPos = agentPos;
    }

    if(minDist >= 1){
        initPos =agentPos;
    }

    if(minDist > 0.5){
        waitPos = ballPos + Vector2D(0.11,0);
    }
    else
    {
        waitPos = agentPos + (ballPos - agentPos).norm()*0.05;
    }
    finalDir = (ballPos - waitPos);
    double ballx= (wm->ball->vel.x)*cos(agent->dir().th().radian()) + (wm->ball->vel.y)*sin(agent->dir().th().radian());
    double bally= -1*(wm->ball->vel.x)*sin(agent->dir().th().radian()) + (wm->ball->vel.y)*cos(agent->dir().th().radian());
    if ((agent->dir().th() - finalDir.th()).degree()  <- 50 ){
        angPid->kp = 3;
        angPid->error = ((ballPos - agentPos).th() - agent->dir().th()).radian();
        draw(QString("ang: %1 ").arg((ballPos - agentPos).th().radian() - _PI/2),Vector2D(0,0),"red");

        if(slow)
            agent->setRobotVel(agentPos.dist(ballPos) ,-1,angPid->PID_OUT() +3 );
        else
            //agent->setRobotVel(0.4 + 2*ballx,-0.6+ 2*bally,3 + 2*((wm->ball->pos - agent->pos()).th() - agent->dir().th()).radian());
            agent->setRobotVel(agentPos.dist(ballPos) ,-1.4,angPid->PID_OUT() +5 );


    }
    else if ((agent->dir().th() - finalDir.th()).degree()  > 50 )
    {
        angPid->kp = 3;
        //debug(QString("50"),D_HOSSEIN);
        angPid->error = ((ballPos - agentPos).th() - agent->dir().th()).radian();

        if(slow)
            agent->setRobotVel( agentPos.dist(ballPos),1,angPid->PID_OUT() - 3);
        else
            //agent->setRobotVel(-0.4 + 2*ballx,0.6+ 2*bally,-3 + 2*((wm->ball->pos - agent->pos()).th() - agent->dir().th()).radian());
            agent->setRobotVel( agentPos.dist(ballPos),1.4,angPid->PID_OUT() - 5) ;


    }
    else
    {
        GPA->init(waitPos,wm->ball->pos -agentPos);
        GPA->setSlowMode(false);
        GPA->setADiveMode(false);
        GPA->execute();
    }




}
void CSkillKeep::farMoving()
{
    intercept->execute();
}



double CSkillKeep::progress()
{
    return 0;
}
void CSkillKeep::execute()
{
    ballPos = wm->ball->pos;
    agentPos = agent->pos();
    ballVel = wm->ball->vel;
    ballArea.assign(ballPos,ballAreaRad);
    switch(decideMode())
    {
    case FARSTOP:
    case RELAX:
        farStop();
        break;
    case FARMOVING:
        farMoving();
        break;
    case DISTURB:
        disturb();
        break;
    }
}

CSkillKeep::~CSkillKeep()
{
    delete GPA;
    delete intercept;
    delete angPid;
}
