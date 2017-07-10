#include "gotopoint.h" #include <QDebug>
#include "knowledge.h"
#include <QFile>
#include <mathtools.h>
#include <algorithm>
#include <math.h>
#include <varswidget.h>

#define EPS 1e-6


//#define USE_CORNELL

#define DELTA_T 0.060      //wm->commandSampleTime()
//#define DELTA_T wm->commandSampleTime()
////////////////////////////////////////////////////////////////////////////////
// goto point modified by DON & MAHI
INIT_SKILL(CskillNewGotoPoint, "gotopoint");

CskillNewGotoPoint::CskillNewGotoPoint(CAgent *_agent) : CSkill(_agent)
{
    /*loopTimer.start();*/
    agent = _agent;
    speedPidX = new _PID(1,0,0,0,0);
    speedPidY = new _PID(1,0,0,0,0);
    posPid = new _PID(1,0,0,0,0);
    velPid = new _PID(1,0,0,0,0);
    angPid= new _PID(2,0,0,0,0);
    posXpid = new _PID(1,0,0,0,0);
    posYpid = new _PID(1,0,0,0,0);
    thPid = new _PID(1,0,0,0,0);
    //_VmReal = 4;
    _Vf = 0;
    _Acc = 5;
    _Dec = 4;
    slow = false;
    slowMode = false;
    verySlow = false;

    lookat.invalidate();

    GPMode = GPACC1;
    kkPosDist = 0.4;

    decThr = 0.2;
    kkThrPos = 0;
    kkThrVConst = 0;
    _VmDesire = 4;

    kkVDesire = 0;
    diveMode = false;


}

void CskillNewGotoPoint::init(Vector2D _target, Vector2D _targetDir, Vector2D _targetVel, bool dynamicStart)
{
    targetPos = _target;
    FinalDir = _targetDir;
    targetDir = FinalDir.th();
    targetVel = _targetVel;

}

CskillNewGotoPoint::~CskillNewGotoPoint()
{
    delete speedPidX;
    delete speedPidY;
    delete posPid;
    delete velPid;
    delete angPid;
    delete posXpid;
    delete posYpid;
}

gpMode CskillNewGotoPoint::decideMode()
{


    if(diveMode)
        kkPosDist = 1;
    kkPosDist = 1/sqrt(kkVc);
    if(kkPosDist > 0.6)
        kkPosDist = 0.6;
    if(kkPosDist < 0.2)
        kkPosDist = 0.2;

    kkPosDist = 0.5;
    if(agentDist < kkPosDist + kkThrPos)

    {
        //        debug(QString("state: pos"),D_MHMMD);
        kkThrPos = 0.2;
        decThr = 0;
        return GPPOS;
    }
    else
    {
        kkThrPos = 0;

        if(fabs((agentMovementTh - agent->dir().th()).degree())<70)
        {

            kkX3 = fabs(    ( (kkVc-(posPid->kp*kkPosDist))*(kkVc-(posPid->kp*kkPosDist)) )/( -2*_Dec )  + ( ((posPid->kp*kkPosDist))*(kkVc-(posPid->kp*kkPosDist)) )/( -1*_Dec )   );// + 0.15*kkVc;

        }
        else
        {
            kkX3 = fabs(    ( (kkVc-(posPid->kp*kkPosDist))*(kkVc-(posPid->kp*kkPosDist)) )/( -2*_Dec )  + ( ((posPid->kp*kkPosDist))*(kkVc-(posPid->kp*kkPosDist)) )/( -1*_Dec )   ) ;//.+ 0.05*kkVc;

        }
        //        if(kkVc < 1)
        //            kkX3 = 0;
        draw(Circle2D(targetPos, kkX3 + decThr),QColor(Qt::cyan));


        if(agentDist - kkPosDist < kkX3 + decThr/* - agent->vel().length() * 0.02*/)
        {
            decThr = 1;
            return GPDEC1;

        }
        else if ( agent->vel().length() >= _VmDesire - kkThrVConst)
        {
            kkThrVConst = 0.2;
            decThr = 0;
            return GPVCONST;

        }
        else
        {
            decThr = 0;
            return GPACC1;
        }


    }
}

void CskillNewGotoPoint::execute()
{

    posPid->kp = 3;
    realAcc = optimalAccOrDec( Vector2D::angleBetween(target-agent->pos(),agent->dir()).radian(), false);
    realDec = fabs(optimalAccOrDec ( Vector2D::angleBetween(target-agent->pos(),agent->dir()).radian(), true));
    appliedAcc = 1.5* realAcc;
    appliedDec = 2* _Dec;
    angleOfMovement = Vector2D::angleBetween(targetPos-agent->pos(),agent->dir()).radian();
    _VmDesire = 4;


    //    debug(QString("ang: %1").arg(_Vx),D_MHMMD);
    if(lookat.isValid())
    {
        targetDir = (lookat - agent->pos()).th();
    }

    //targetPos.x
    if (conf()->LocalSettings_ParsianWorkShop()) {
        if(conf()->LocalSettings_OurTeamSide() == "Right")
        {
            if(targetPos.x < 0.35)
            {
                targetPos.x = 0.35;
            }
        }
        else
        {
            if(targetPos.x > 4.35)
            {
                targetPos.x = 4.35;
            }
        }
    }
    kkVf = 0;
    agentDist = agent->pos().dist(targetPos) ;
    agentMovementTh = (targetPos-agent->pos()).th();


    if(wm->getIsSimulMode())
    {
        kkVc = agent->vel().length();
        _VmDesire = 1.5;
    }
    else
        kkVc = agent->vel().length();
    kkVc = agent->vel().length();
    GPMode = decideMode();

    angPid->error = (targetDir-agent->dir().th()).radian();
    angPid->kp=4;
    angPid->kd=0;
    angPid->ki=0;

    posPid->kp = 3;
    posPid->kd = 0;
    posPid->ki = 0;

    velPid->kp = 0.1;
    velPid->kd = 0;
    velPid->ki = 0.1;

    switch(GPMode)
    {
    case GPACC1:
    case GPACC2:
        debug("GPACC",D_KK);
        /////////new bang bang////////
        kkVDesire = _VmDesire ;

        agent->_ACC =appliedAcc;
        agent->_DEC = 0;
        /////////////////////////////
        break;
    case GPVCONST:
        debug("GPVCONST",D_KK);
        /////////new bang bang////////
        kkVDesire = _VmDesire;
        agent->_ACC = 0;
        agent->_DEC = 0;
        ////////////////////////////
        break;
    case GPDEC1:
    case GPDEC2:
        debug("GPDEC",D_KK);
        /////////new bang bang////////
        if(1 || kkVc < posPid->kp*kkPosDist)
        {
            double sspeed = 1.8;
            kkVDesire = sspeed;//posPid->kp*kkPosDist;
            agent->_ACC = 0;
            agent->_DEC = 0;
        }
        else
        {
            kkVDesire = 0.1;
            agent->_ACC = 0;
            agent->_DEC = appliedDec;
        }

        //////////////////////////////
        break;
    case GPPOS:
        debug("GPPOS",D_KK);
        /////////new bang bang////////
        agent->_ACC = 0;
        agent->_DEC = 0;
        break;

    }

    if(GPMode != GPPOS && GPMode != GPDEC1)
    {
        thPid->kp =0.7;
        thPid->error = (agentMovementTh - agent->vel().norm().th()).radian();
        if(fabs(thPid->error > 1) || kkVc < 0.2 || agentDist >3 ||( fabs((agentMovementTh - agent->dir().th()).degree()) > 80 && fabs((agentMovementTh - agent->dir().th()).degree()) < 100 )   )
            thPid->error =0;

        ///////////////speed
        _Vx =  (kkVDesire)*cos(agentMovementTh.radian() + thPid->PID_OUT());
        _Vy =  (kkVDesire)*sin(agentMovementTh.radian() + thPid->PID_OUT());

        agent->setRobotAbsVel(_Vx  ,_Vy ,angPid->PID_OUT());
    }
    else if(GPMode == GPDEC1)
    {
        //        posXpid->kp = 2/((targetPos.dist(agent->pos()))+0.05*agent->vel().length());
        //        posYpid->kp = 2/((targetPos.dist(agent->pos()))+0.05*agent->vel().length());
        //        posXpid->kd = 0.5;
        //        posXpid->kd = 0.5;

        //        if(posXpid->kp > 4 && !diveMode)
        //     posXpid->kp = 4;
        //        if(posYpid->kp >4 && !diveMode)
        //            posYpid->kp = 4;
        //        if(posXpid->kp < 1.5 && !diveMode)
        //            posXpid->kp = 1.5;
        //        if(posYpid->kp < 1.5 && !diveMode)
        //            posYpid->kp = 1.5;
        //        if(agentDist < 0.1)
        //        {
        //            posXpid->kp =5;
        //            posYpid->kp = 5;
        //        }





        //        posXpid->error = (targetPos - agent->pos()).x;
        //        posYpid->error = (targetPos - agent->pos()).y;


        //        agent->setRobotAbsVel(posXpid->PID_OUT() ,posYpid->PID_OUT(),angPid->PID_OUT());

        //        posXpid->pError = (targetPos - agent->pos()).x;
        //        posYpid->pError = (targetPos - agent->pos()).y;
        //        posPid->pError = 0;

        thPid->error = (agentMovementTh - agent->vel().norm().th()).radian();
        if(fabs(thPid->error > 1) || kkVc < 0.2 || agentDist >3 ||( fabs((agentMovementTh - agent->dir().th()).degree()) > 80 && fabs((agentMovementTh - agent->dir().th()).degree()) < 100 )   )
            thPid->error =0;

        ///////////////speed
        _Vx =  (kkVDesire)*cos(agentMovementTh.radian() + thPid->PID_OUT());
        _Vy =  (kkVDesire)*sin(agentMovementTh.radian() + thPid->PID_OUT());

        agent->setRobotAbsVel(_Vx  ,_Vy ,angPid->PID_OUT());
    }
    else
    {
        if(agentDist > 0.01)
        {



            posPid->error = targetPos.dist(agent->pos());
            posPid->kd = 2;
            //            posXpid->kp = 2/((targetPos.dist(agent->pos()))+0.2*agent->vel().length());
            //            posYpid->kp = 2/((targetPos.dist(agent->pos()))+0.2*agent->vel().length());
            posXpid->kp = 4;
            posYpid->kp = 4;
            posXpid->kd = 0;
            posXpid->kd = 0;




            if(posXpid->kp > 5 && !diveMode)
                posXpid->kp = 5;
            if(posYpid->kp >5 && !diveMode)
                posYpid->kp = 5;
            if(posXpid->kp < 2 && !diveMode)
                posXpid->kp = 2;
            if(posYpid->kp < 2 && !diveMode)
                posYpid->kp = 2;
            if(agentDist < 0.1)
            {
                posXpid->kp =5;
                posYpid->kp = 5;
            }





            posXpid->error = (targetPos - agent->pos()).x;
            posYpid->error = (targetPos - agent->pos()).y;


            agent->setRobotAbsVel((posPid->PID_OUT())*cos(agentMovementTh.radian()) ,(posPid->PID_OUT())*sin(agentMovementTh.radian()),angPid->PID_OUT());
            // agent->setRobotAbsVel(speed*cos(kkMovementTh.radian()) ,speed*sin(kkMovementTh.radian()),angPid->PID_OUT());

        }
        else
        {
            agent->setRobotAbsVel(0,0,angPid->PID_OUT());
            agent->_ACC = 0;
            agent->_DEC = 0;

        }

        posXpid->pError = (targetPos - agent->pos()).x;
        posYpid->pError = (targetPos - agent->pos()).y;
        posPid->pError = agentDist;
    }

    angPid->pError = (targetDir-agent->dir().th()).radian();
    lastPath = agent->vel().th();
}
double CskillNewGotoPoint::gTimeRemaining(CAgent *_agent,Vector2D _destination)
{
    double maxVel = 3;
    double acceleration = 4 , deceleration = -4;
    return ((_agent->pos().dist(_destination)/maxVel) + ((maxVel) / (acceleration*2)) -(maxVel / (deceleration*2)));
}

double CskillNewGotoPoint::timeRemaining()
{
    switch(GPMode)
    {
    case GPACC1:
    case GPACC2:
        return ((agentDist/kkVmax) + ((kkVmax-kkVc) / _Acc) -((kkVmax) / _Dec) );
        break;
    case GPVCONST:
        return ((agentDist/kkVc)  - ((kkVmax) / _Dec) );
        break;
    case GPDEC1:
    case GPDEC2:
        return ((agentDist/kkVc)  - ((kkVmax) / _Dec) );
        break;
    case GPPOS:
        return 0;
        break;

    }
}

double CskillNewGotoPoint::progress()
{
    if(agent==NULL) return 0;
    double d = (agent->pos() - targetPos).length();
    if (d<0.04) return 1.0;
    if (d<0.05) return 0.8;
    if (d<0.10) return 0.7;
    if (d<0.20) return 0.6;
    return 0;
}

double CskillNewGotoPoint::optimalAccOrDec(double agentDir , bool dec)
{
    double Vx = cos(agentDir);
    double Vy = sin(agentDir);
    double fWheels[4];
    double biggest = 0.0;
    double optimalAcc , optimalDec;
    double Ff , Fn;
    //////////////Calculate Jacobian Matrix//////////
    fWheels[0] = -(Vx * 0.8660) + (Vy * 0.5);
    fWheels[1] = -(Vx * 0.7071) - (Vy * 0.7071);
    fWheels[2] =  (Vx * 0.7071) - (Vy * 0.7071);
    fWheels[3] =  (Vx * 0.8660) + (Vy * 0.5);
    ////////////////////////////////////////////////
    ///////////find biggest value in Jacob//////////
    for(int i = 0; i < 4 ; i++) {
        if( fabs(fWheels[i]) > biggest ) {
            biggest = fabs(fWheels[i]);
        }
    }
    /////////////////////////////////////////////////
    //////////normalize Jacob's Value////////////////
    for(int i = 0; i < 4 ; i++) {

        fWheels[i] = fWheels[i]/biggest;
    }
    /////////////////////////////////////////////////
    ///////////calculate forward force vector and normal force vector////////////////////
    Ff = ((fWheels[3]-fWheels[0])*(sqrt(3)/2)) + ((fWheels[2] - fWheels[1])*(sqrt(2)/2));
    Fn = ((fWheels[3]+fWheels[0])*0.5) + (-1*(fWheels[2] + fWheels[1])*(sqrt(2)/2));
    ////////////////////////////////////////////////////////////////////////////////////
    /////////////////2.8868 is max of sum Ff and Fn and this derivation is for nomalization of Max Acc = _Acc/////////
    optimalAcc = _Acc * sqrt((Ff*Ff) + (Fn*Fn))/2.8868;
    optimalDec = _Dec * sqrt((Ff*Ff) + (Fn*Fn))/2.8868;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////if boll dec = true the function return optimal dec///////////
    if(dec) {
        return optimalDec;
    }
    //////////otherwise return optimal acc//////////
    return optimalAcc;
}

////////////////////////////////////////////////////////////////////////////////

INIT_SKILL(CSkillTurn, "turn");

CSkillTurn::CSkillTurn(CAgent *_agent)
{
    agent = _agent;
    pid.reset();
    pid.resetI();
    dynamicP = dynamicI = dynamicD = 0;
}

double CSkillTurn::progress()
{
    return 0;
}

CSkillTurn::~CSkillTurn()
{

}

void CSkillTurn::execute()
{
    if ( agent->abilities.hasGyro && 0)
    {
        agent->setRobotVel(agent->vforward, agent->vnormal, Vector2D::angleBetween(agent->agentAngelForGyro , direction).radian());
        return;
    }
    double DT = conf()->Common_Command_Interval()/1000.0;
    pid.setDT(DT);
    pid.setSumLimit(10);
    if (getTurnMode() == Fast)
    {
        pid.setParams(7,0.0,0.0);
    }
    else if (getTurnMode() == Slow)
    {
        pid.setParams(2.4,0.0,0.0);
    }
    else if (getTurnMode() == Moving)
    {
        pid.setParams(3.3,0.0,0.0);
    }
    else if (getTurnMode() == Penalty)
    {
        pid.setParams(4.0,0.0,0.0);
    }
    else if (getTurnMode() == Intercept)
    {
        pid.setParams(1.5,0.1,0.00);
    }
    else if( getTurnMode() == GoToPos ){
        pid.setParams(3.0,0.02,0.0);
    }
    else if( getTurnMode() == Dynamic ){
        pid.setParams(dynamicP,dynamicI,dynamicD);
    }
    double ew = Vector2D::angleBetween(agent->dir(), direction).radian();
    double w = pid.step(-ew);
    agent->setRobotVel(agent->vforward,0, w);
}

void CSkillTurn::setDynamic(double _P, double _I, double _D)
{
    dynamicP = _P;
    dynamicI = _I;
    dynamicD = _D;
}

INIT_SKILL(CSkillGotoPoint, "gotopoint");

//////////////////////////// up to here
CSkillGotoPoint::CSkillGotoPoint(CAgent *_agent) : CSkill(_agent)
{
    lookat.invalidate();
    agent = _agent;
    speedPidX = new _PID(1,0,0,0,0);
    speedPidY = new _PID(1,0,0,0,0);
    posPid = new _PID(1,0,0,0,0);
    velPid = new _PID(1,0,0,0,0);
    angPid= new _PID(2,0,0,0,0);
    posXpid = new _PID(1,0,0,0,0);
    posYpid = new _PID(1,0,0,0,0);
    thPid = new _PID(1,0,0,0,0);

    _Acc = 4;
    _Dec = 4;


    lookat.invalidate();

    lastGPmode = GPACC1;
    posPidDist = 0.5;

    decThr = 0.2;
    posThr = 0;
    vConstThr = 0;
    _VmDesire = 5;

    agentVDesire = 0;
    ////modes
    slow = false;
    slowMode = false;
    verySlow = false;
    diveMode = false;
    penaltyKick = false;
    smooth = false;
}

CSkillGotoPoint::~CSkillGotoPoint()
{
    //    delete speedPidX;
    //    delete speedPidY;
    //    delete posPid;
    //    delete angPid;
    //    delete posXpid;
    //    delete posYpid;
    //    delete thPid;
}

void CSkillGotoPoint::init(Vector2D target, Vector2D _targetDir, Vector2D _targetVel, bool dynamicStart)
{
    targetPos = target;
    targetDir = _targetDir;
    targetVel = _targetVel;
}

double CSkillGotoPoint::timeNeeded()
{
    return 0 ;
}
gpMode CSkillGotoPoint::decideMode()
{
    //    posPidDist = 1/agentVc;
    //    if(posPidDist > 1)
    //        posPidDist = 1;
    if(agentDist < posPidDist + posThr) {
        decThr = 0;
        return GPPOS;
    }
    else {
        agentX3 = fabs(((posPidDist*posPid->kp)*(posPidDist*posPid->kp) - (agentVc*agentVc))/(2*_Dec)) + 0.05 *agentVc;

        if(agentDist <= agentX3 + decThr) {
            if(agentVc < 0.5)
                decThr = 0;
            else
                decThr = 0.5;
            return GPDEC1;
        }
        else if(agentVc >= _VmDesire)
        {
            decThr = 0;
            return GPVCONST;
        }
        else {
            decThr = 0;
            return GPACC1;
        }

    }

}
double CSkillGotoPoint::optimalAccOrDec(double agentDir, bool dec)
{
    double Vx = cos(agentDir);
    double Vy = sin(agentDir);
    double fWheels[4];
    double biggest = 0.0;
    double optimalAcc , optimalDec;
    double Ff , Fn;
    //////////////Calculate Jacobian Matrix//////////
    fWheels[0] = -(Vx * 0.8660) + (Vy * 0.5);
    fWheels[1] = -(Vx * 0.7071) - (Vy * 0.7071);
    fWheels[2] =  (Vx * 0.7071) - (Vy * 0.7071);
    fWheels[3] =  (Vx * 0.8660) + (Vy * 0.5);
    ////////////////////////////////////////////////
    ///////////find biggest value in Jacob//////////
    for(int i = 0; i < 4 ; i++) {
        if( fabs(fWheels[i]) > biggest ) {
            biggest = fabs(fWheels[i]);
        }
    }
    /////////////////////////////////////////////////
    //////////normalize Jacob's Value////////////////
    for(int i = 0; i < 4 ; i++) {

        fWheels[i] = fWheels[i]/biggest;
    }
    /////////////////////////////////////////////////
    ///////////calculate forward force vector and normal force vector////////////////////
    Ff = ((fWheels[3]-fWheels[0])*(sqrt(3)/2)) + ((fWheels[2] - fWheels[1])*(sqrt(2)/2));
    Fn = ((fWheels[3]+fWheels[0])*0.5) + (-1*(fWheels[2] + fWheels[1])*(sqrt(2)/2));
    ////////////////////////////////////////////////////////////////////////////////////
    /////////////////2.8868 is max of sum Ff and Fn and this derivation is for nomalization of Max Acc = _Acc/////////
    optimalAcc = _Acc * sqrt((Ff*Ff) + (Fn*Fn))/2.8868;
    optimalDec = _Dec * sqrt((Ff*Ff) + (Fn*Fn))/2.8868;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////if boll dec = true the function return optimal dec///////////
    if(dec) {
        return optimalDec;
    }
    //////////otherwise return optimal acc//////////
    return optimalAcc;
}
void CSkillGotoPoint::targetValidate()
{
    if (targetPos.x < wm->field->ourCornerL().x - 0.2) targetPos.x = wm->field->ourCornerL().x;
    if (targetPos.x > wm->field->oppCornerL().x + 0.2) targetPos.x = wm->field->oppCornerL().x;
    if (targetPos.y < wm->field->ourCornerR().y - 0.2) targetPos.y = wm->field->ourCornerR().y;
    if (targetPos.y > wm->field->ourCornerL().y + 0.2) targetPos.y = wm->field->ourCornerL().y;

    if (conf()->LocalSettings_ParsianWorkShop()) {
        if(conf()->LocalSettings_OurTeamSide() == "Right")
        {
            if(targetPos.x < 0.2)
            {
                targetPos.x = 0.2;
            }
        }
        else
        {
            if(targetPos.x > 4.3)
            {
                targetPos.x = 4.3;
            }
        }
    }

    if (lookat.valid())
    {
        targetDir = (lookat - agentPos).norm();
    }
}

void CSkillGotoPoint::trajectoryPlanner()
{
    agentMovementTh = (targetPos -agentPos ).th();
    //////////////////acc dec
    agentBestAcc = optimalAccOrDec(Vector2D::angleBetween(targetPos-agentPos,agent->dir()).radian(),false);
    agentBestDec = optimalAccOrDec(Vector2D::angleBetween(targetPos-agentPos,agent->dir()).radian(),true);
    appliedAcc =1.5* _Acc;

    if(smooth)
    {
        if((agentMovementTh - lastPath).degree() > 20 && (agentMovementTh - lastPath).degree() < 100 && agentVc > 1) {
            agentMovementTh = lastPath + 60;
            _VmDesire = 1;
            appliedAcc = 0;

        }
        else if((agentMovementTh - lastPath).degree() < -20 && (agentMovementTh - lastPath).degree() > -100 && agentVc > 1) {
            agentMovementTh = lastPath - 60;
            _VmDesire = 1;
            appliedAcc = 0;

        }
        else if((agentMovementTh - lastPath).degree() >= 100 && agentVc > 1) {
            agentMovementTh = lastPath + 80;
            _VmDesire = 0.5;
            appliedAcc = 0;

        }
        else if((agentMovementTh - lastPath).degree() <= -100 && agentVc > 1) {
            agentMovementTh = lastPath - 80;
            _VmDesire = 0.5;
            appliedAcc = 0;

        }
    }
    ///////////////////////////////////////////// th pid
    thPid->kp =0;
    thPid->error = (agentMovementTh - agent->vel().norm().th()).radian();
    if(fabs(thPid->error > 1) || agentVc < 0.5 || agentDist >3 ||( fabs((agentMovementTh - agent->dir().th()).degree()) > 80 && fabs((agentMovementTh - agent->dir().th()).degree()) < 100 )   )
        thPid->error =0;

    appliedTh = agentMovementTh.radian() +thPid->PID_OUT();


}

void CSkillGotoPoint::execute()
{
    _VmDesire = 4;
    if(slow || slowMode || penaltyKick)
    {
        _VmDesire = 1.5;
    }
    if(diveMode)
    {
        _VmDesire = 4;
    }
    if(wm->getIsSimulMode()) {
        _VmDesire = 2;
    }
    targetValidate();

    /////////////////decide and exec

    agentPos = agent->pos();
    agentVel = agent->vel();
    agentDist = agentPos.dist(targetPos);
    currentGPmode = decideMode();
    angPid->error = (targetDir.th() - agent->dir().th()).radian();
    agentVc = agentVel.length();
    //////////////// set params
    posPid->kd = 3;
    if(startingPoint.dist(agentPos) < 0.05)
    {
        posPid->kp = 4;
        posPid->kd = 0;
    }
    else if(startingPoint.dist(agentPos) < 0.3)
    {
        posPid->kp = 0.37/agentDist;
        if(posPid->kp > 3)
            posPid->kp = 3;

        posPid->kd = 10;
    }
    else
        posPid->kp = 1.9;


    if(slow || slowMode || penaltyKick)
    {
        posPid->kp = 1.6;
    }
    if(diveMode)
    {
        posPid->kp = 1.8/agentDist;
        posPid->kd = 10;
        if(posPid->kp > 4)
            posPid->kp = 4;posPidDist = 1;
    }
    else
    {
        posPidDist = 0.5;
    }
    posPid->kd = 1;

    diveMode = false;



    //    debug(QString("dive %1").arg(diveMode),D_MHMMD);

    angPid->kp = 3;
    angPid->kd = 1;



    trajectoryPlanner();

    //////////////////////// dec calculations
    double vp =(posPidDist*posPid->kp);
    double moreDec = 0.65;
    double decOffset = 0.8;
    if(agentVc < 0.2)
        startingPoint = agentPos;


    ////////////////////////////
    if(currentGPmode == GPPOS) {
        ////////////////ACC + DEC
        agent->_ACC = 0;
        agent->_DEC = 0;
        ////////////////
        posPid->error = agentDist;
        _Vx = posPid->PID_OUT()*cos(agentMovementTh.radian());
        _Vy = posPid->PID_OUT()*sin(agentMovementTh.radian());
        if(agentDist  < 0.015){
            _Vx = 0;
            _Vy = 0;
        }
        ///////////////////////////////////////////////PID Previous error
        posPid->pError = agentDist;
        velPid->_I = 0;
    }
    else if(currentGPmode == GPVCONST) {
        /////////////////ACC + DEC
        agent->_ACC = 0;
        agent->_DEC = 0;
        agentVDesire = _VmDesire;
        velPid->_I = 0;
        ////////////////
        _Vx = _VmDesire*cos(appliedTh);
        _Vy = _VmDesire*sin(appliedTh);

    }
    else if(currentGPmode == GPDEC1) {
        agent->_ACC = 0;
        agent->_DEC = 0;
        agentVDesire = sqrt(fabs(2*_Dec*agentDist*moreDec) + vp *vp) - decOffset;
        _Vx =  agentVDesire*cos(appliedTh) ;
        _Vy =  agentVDesire*sin(appliedTh) ;

    }
    else if(currentGPmode == GPACC1) {
        if(agentVc > 0.3)
        {
            agent->_ACC = appliedAcc;
            agent->_DEC = 0;
            agentVDesire = _VmDesire;
        }
        else if(!slow && !slowMode && !penaltyKick)
        {
            agent->_ACC = 0;
            agent->_DEC = 0;
            agentVDesire = 0.7;
        }
        else
        {
            agent->_ACC = 0;
            agent->_DEC = 0;
            agentVDesire = 0.5;
        }
        ////////////////
        _Vx = agentVDesire*cos(appliedTh);
        _Vy = agentVDesire*sin(appliedTh);
        ///////////////////////////////////////////////PID Previous error
    }
    else
    {
        agent->waitHere();
        velPid->_I = 0;
    }
    agent->setRobotAbsVel(_Vx,_Vy,angPid->PID_OUT());
    angPid->pError = angPid->error;


    lastPath = agentVel.th();

}

double CSkillGotoPoint::progress()
{
    if(agent==NULL) return 0;
    double d = (agentPos - targetPos).length();
    if (d<0.04) return 1.0;
    if (d<0.05) return 0.8;
    if (d<0.10) return 0.7;
    if (d<0.20) return 0.6;
    return 0;
}


motionProfile::motionProfile(Vector2D _p, Vector2D _v, double _t, double _forwardVel, double _normalVel, Vector2D dir, double _angVel)
{
    position = _p;
    velocity = _v;
    time = _t;
    forwardVel = _forwardVel;
    normalVel = _normalVel;
    direction = dir;
    angularVel = _angVel;
}



//--------------------------------------------------
INIT_SKILL(CSkillGotoPointAvoid, "gotopointavoid");

CSkillGotoPointAvoid::CSkillGotoPointAvoid(CAgent *_agent) : CSkillGotoPoint(_agent)
{
    counter = 0;
    avoidPenaltyArea = true;
    inited = false;
    keeplooking = false;
    extendStep = -1.0;
    gotopoint = new CSkillGotoPoint(_agent);
    bangBang = new CNewBangBang();
    dynamicStart = true;
    plan2 = false;
    prof = NULL;
    noAvoid = false;
    avoidCenterCircle = false;
    avoidGoalPosts = true;
    stucked = -1;
    counting = 0;
    averageDir.assign(0 , 0);
    addVel.assign(0,0);
    nextPos.invalidate();
    diveMode = false;
    oneTouchMode = false;
    drawPath = false;
}

CSkillGotoPointAvoid::~CSkillGotoPointAvoid()
{
    delete gotopoint;
}

CSkillGotoPointAvoid* CSkillGotoPointAvoid::noRelax()
{
    ourRelaxList.clear();
    oppRelaxList.clear();
    return this;
}

CSkillGotoPointAvoid* CSkillGotoPointAvoid::ourRelax(int element)
{
    if (!ourRelaxList.contains(element)) ourRelaxList.append(element);
    return this;
}

CSkillGotoPointAvoid* CSkillGotoPointAvoid::oppRelax(int element)
{
    if (!oppRelaxList.contains(element)) oppRelaxList.append(element);
    return this;
}

void CSkillGotoPointAvoid::execute()
{
    agentPos = agent->pos();
    agentVel = agent->vel();
    double dVx,dVy,dW;
    bangBang->setDecMax(conf()->BangBang_DecMax());
    bangBang->setOneTouch(oneTouchMode );
    bangBang->setDiveMode(diveMode);
    if(slowMode || slow)
    {
        bangBang->setVelMax(1.4);
        bangBang->setSlow(true);
    }
    else
    {
        bangBang->setSlow(false);
        bangBang->setVelMax(conf()->BangBang_VelMax());
    }
    if (!targetPos.valid())
    {
        agent->waitHere();
        if (prof!=NULL)
            prof->store();
        return;
    }
    if (!targetVel.valid())
        vel2.assign(0,0);

    if(drawPath)
    {
        if(agentVel.length() < 0.1)
        {
            pathPoints.clear();
        }
        else
        {
            pathPoints.append(agentPos);
            for(int i = 0 ; i < pathPoints.size() ; i++)
            {
                draw(Circle2D(pathPoints[i],0.02),QColor(Qt::blue),true);
            }
        }
    }
    else
    {
        pathPoints.clear();
    }

    /////////////////
    if (targetPos.x < wm->field->ourCornerL().x - 0.2) targetPos.x = wm->field->ourCornerL().x;
    if (targetPos.x > wm->field->oppCornerL().x + 0.2) targetPos.x = wm->field->oppCornerL().x;
    if (targetPos.y < wm->field->ourCornerR().y - 0.2) targetPos.y = wm->field->ourCornerR().y;
    if (targetPos.y > wm->field->ourCornerL().y + 0.2) targetPos.y = wm->field->ourCornerL().y;

    if (conf()->LocalSettings_ParsianWorkShop()) {
        if(conf()->LocalSettings_OurTeamSide() == "Right")
        {
            if(targetPos.x < 0.2)
            {
                targetPos.x = 0.2;
            }
        }
        else
        {
            if(targetPos.x > 4.3)
            {
                targetPos.x = 4.3;
            }
        }
    }

    if (lookat.valid())
    {
        targetDir = (lookat - agentPos).norm();
    }

    knowledge->plotWidgetCustom[1] = agentVel.length();
    //    debug(QString("speed: %1").arg(agentVel.length()),D_MHMMD);
    ///////////
    targetValidate();
    if(noAvoid)
    {
        result.clear();
    }
    else
    {
        agent->initPlanner(agent->id() , targetPos , ourRelaxList , oppRelaxList , avoidPenaltyArea , avoidCenterCircle ,0);//avoidBall* ballObstacleRadius);
        result.clear();
        for( int i=agent->pathPlannerResult.size()-1 ; i>=0 ; i-- )
        {
            result.append(agent->pathPlannerResult[i]);
        }
    }

    double dist = 0.0;
    bool flag = false;
    Vector2D dir(0,0);

    if( result.size() > 1 )
        dir = (result[1] - result[0]).norm();

    double D = 0 , alpha = 0 , d = 0 , vf = 0;
    Vector2D lllll ;
    if(result.count())
    {
        lllll= result.last();
    }



    //    for( int i=1 ; i<result.size() ; i++ ){
    //        alpha = 0;
    //        if(fabs(Vector2D::angleBetween(result[i] - result[0] , result[i+1] - result[i]).degree()) > 3 ){
    //            if( i+1 == result.size() ){
    //                vf = 0;
    //                alpha = 0;
    //            }
    //            else{
    //                alpha = fabs(Vector2D::angleBetween(result[i] - result[0] , result[i+1] - result[i]).degree());
    //                vf = -1.0259280143 * log(alpha) + 4.570475303;
    //                vf = max(vf , 0.5);
    //            }
    //            D = result[i].dist(result[0]);
    //            lllll = result[i];
    //            d = dist - D;


    //            flag = false;
    //            break;
    //        }
    //    }

    if(result.size() >= 3)
    {
        alpha = fabs(Vector2D::angleBetween(result[1] - result[0] , result[2] - result[1]).degree());
        debug(QString("alpha : %1").arg(alpha),D_MHMMD);
        lllll = result[1];
        vf = -2 * log(alpha) + 8;
        vf = max(vf , 0.5);
        vf = min (vf,4);
    }
    else
    {
        vf = 0;
        lllll = targetPos;
    }

    ////////////////////// avoid goal posts
    Segment2D goalPostL,goalPostR;
    goalPostL.assign(wm->field->ourGoalL() - Vector2D(0.2,0),wm->field->ourGoalL() + Vector2D(0.1,0));
    goalPostR.assign(wm->field->ourGoalR() - Vector2D(0.2,0),wm->field->ourGoalR() + Vector2D(0.1,0));
    avoidGoalPosts = true;
    Segment2D agenPath(agent->pos(),lllll);
    if(avoidGoalPosts)
    {
        if(agenPath.intersection(goalPostL).isValid())
        {
            lllll = wm->field->ourGoalL() + Vector2D(0.12,0);
        }
        else if(agenPath.intersection(goalPostR).isValid())
        {
            lllll = wm->field->ourGoalR() + Vector2D(0.12,0);
        }

    }
    /////////////////////


    if( noAvoid || result.size() < 3){
        lllll = targetPos;
        vf = 0;
    }

    bangBang->setSmooth(true);// = false;
    bangBang->bangBangSpeed(agentPos,agentVel,agent->dir(),lllll,targetDir,vf,0.016,dVx,dVy,dW);
    agent->setRobotAbsVel(dVx + addVel.x,dVy + addVel.y,dW);
    agent->accelerationLimiter(vf,false);



    counter ++;

    if (prof!=NULL)
        prof->store();
}

double CSkillGotoPointAvoid::progress()
{
    if(agentPos.dist(targetPos) < 0.05)
    {
        return 1;
    }
    return 0;
}

CSkillGotoPointAvoid* CSkillGotoPointAvoid::setTargetLook(Vector2D finalPos, Vector2D lookAtPoint)
{
    init(finalPos, Vector2D(1.0, 0.0));
    setFinalVel(Vector2D(0.0, 0.0));
    setLookAt(lookAtPoint);
    setKeepLooking(true);
    return this;
}

CSkillGotoPointAvoid* CSkillGotoPointAvoid::setTarget(Vector2D finalPos, Vector2D finalDir)
{
    init(finalPos, finalDir);
    setFinalVel(Vector2D(0.0, 0.0));
    setLookAt(Vector2D::INVALIDATED);
    setKeepLooking(false);
    return this;
}

double CSkillGotoPointAvoid::timeNeeded(CAgent *_agentT,Vector2D posT,double vMax,QList <int> _ourRelax,QList <int> _oppRelax ,bool avoidPenalty,double ballObstacleReduce,bool _noAvoid)
{

    double _x3;
    double acc = conf()->BangBang_AccMaxForward();
    double dec = conf()->BangBang_DecMax();
    double xSat;
    Vector2D tAgentVel = _agentT->vel();
    Vector2D tAgentDir = _agentT->dir();
    double veltan= (tAgentVel.x)*cos(tAgentDir.th().radian()) + (tAgentVel.y)*sin(tAgentDir.th().radian());
    double offset = 0.15;
    double velnorm= -1 * (tAgentVel.x)*sin(tAgentDir.th().radian()) + (tAgentVel.y)*cos(tAgentDir.th().radian());
    double distCoef = 1, distEffect = 1, angCoef = 0.003;
    double dist = 0;
    double rrtAngSum = 0;
    QList <Vector2D> _result;
    Vector2D _target;

    if(_noAvoid)
    {
        _result.clear();
    }
    else
    {
        _agentT->initPlanner(_agentT->id(),posT,_ourRelax,_oppRelax,avoidPenalty,false,ballObstacleReduce);
        _result.clear();
        for( int i = _agentT->pathPlannerResult.size()-1 ; i>=0 ; i-- )
        {
            _result.append(_agentT->pathPlannerResult[i]);
        }
    }

    if( _result.size() >= 3) {
        for(int i = 0 ; i < _result.size() - 1; i++)
        {
            dist += _result[i].dist(_result[i+1]);
        }
        for(int i = 1 ; i < _result.size() - 1; i++)
        {
            rrtAngSum += fabs(Vector2D::angleBetween(_result[i] - _result[i-1] , _result[i+1] - _result[i]).degree());
        }
        distEffect = dist / _agentT->pos().dist(posT);
        distEffect += rrtAngSum*angCoef;
        distEffect = max(1,distEffect);
    }

    if(tAgentVel.length() < 0.2)
    {
        acc = (conf()->BangBang_AccMaxForward() + conf()->BangBang_AccMaxNormal())/2;
    }
    else
    {
        acc = conf()->BangBang_AccMaxForward()*(fabs(veltan)/tAgentVel.length()) + conf()->BangBang_AccMaxNormal()*(fabs(velnorm)/tAgentVel.length());
    }

    double vMaxReal = sqrt(((_agentT->pos().dist(posT) + (tAgentVel.length()*tAgentVel.length()/2*acc))*2*acc*dec)/(acc+dec));
    vMaxReal = min(vMaxReal, 4);
    vMax = min(vMax, vMaxReal);
    xSat = ((vMax*vMax)-(tAgentVel.length()*tAgentVel.length()))/acc + (vMax*vMax)/dec;
    _x3 = ( -1* tAgentVel.length()*tAgentVel.length()) / (-2 * fabs(conf()->BangBang_DecMax())) ;

    if(_agentT->pos().dist(posT) < _x3 ) {
        return max(0,(tAgentVel.length()/conf()->BangBang_DecMax() - offset) * distEffect);
    }
    else if(tAgentVel.length() < (vMax)){
        if(_agentT->pos().dist(posT) < xSat)
        {
            return max(0, (-1*offset + vMax/dec + (vMax-tAgentVel.length())/acc + (_agentT->pos().dist(posT) - ((vMax*vMax/(2*dec)) + ((vMax+tAgentVel.length())*(vMax-tAgentVel.length())/acc))/2)/vMax) * distEffect);
        }
        else
        {
            return max(0, (vMax/dec + (vMax-tAgentVel.length())/acc - offset)*distEffect);
        }
    }
    else
    {
        return max(0,(vMax/dec + (_agentT->pos().dist(posT) - ((vMax*vMax/(2*dec)) ))/vMax - offset) * distEffect);
    }

}

//------------------------------------------------------------

INIT_SKILL(CSkillFollowPoints, "followpoints");

CSkillFollowPoints::CSkillFollowPoints(CAgent *_agent) : CSkillGotoPointAvoid(_agent)
{
    gotopointavoid = new CSkillGotoPointAvoid(_agent);
    gotopointavoid->setAgent(_agent);
    currentPoint = -1;
    dMax = 2.0;
    vMax = 1.5;
    finalVel.assign(0.0, 0.0);
    forcePoint = false;
}

CSkillFollowPoints::~CSkillFollowPoints()
{
    delete gotopointavoid;
}

CSkillFollowPoints* CSkillFollowPoints::addPoint(Vector2D point)
{
    points.append(point);
    return this;
}

CSkillFollowPoints* CSkillFollowPoints::clearPoints()
{
    points.clear();
    return this;
}

void CSkillFollowPoints::execute()
{
    //never goes to points[0] {don't change it} ; this point is used to indicate initial direction
    gotopointavoid->oppRelaxList = oppRelaxList;
    gotopointavoid->ourRelaxList = ourRelaxList;
    gotopointavoid->setAgent(agent);
    gotopointavoid->setLookAt(lookat);
    gotopointavoid->setKeepLooking(keeplooking);
    gotopointavoid->setMaxAcceleration(getMaxAcceleration());
    gotopointavoid->setMaxDeceleration(getMaxDeceleration());
    gotopointavoid->setMaxAccelerationNormal(getMaxAccelerationNormal());
    gotopointavoid->setMaxDecelerationNormal(getMaxDecelerationNormal());
    gotopointavoid->setMaxVelocity(getMaxVelocity());
    gotopointavoid->setMaxVelocityNormal(getMaxVelocityNormal());
    gotopointavoid->setConstantVelocity(getConstantVelocity());
    if (points.count()==1)
    {
        gotopointavoid->init(points[0], finalVel);
    }
    else if (points.count()==0)
    {
        return;
    }
    else {
        if (currentPoint == -1)
        {
            gotopointavoid->setDynamicStart(true);
        }
        else gotopointavoid->setDynamicStart(false);
        double nearestDist = 0.0;
        int nearestPoint = -1;
        for (int i=1;i<points.count()-1;i++)
        {
            Vector2D bisect = Vector2D::unitVector(AngleDeg::bisect((points[i-1] - points[i]).th(), (points[i+1] - points[i]).th()));
            double b1 = bisect ^ (agent->pos()-points[i]);
            bool near = ((agent->pos()-points[i]).length()<0.02);
            int s1 = sign((bisect ^ (points[i-1]-points[i]))*b1);
            int s2 = sign((bisect ^ (points[i+1]-points[i]))*b1);
            if (s1*s2<0 || near)
            {
                int j = i;
                if (s2>0 || near) j = i + 1;
                double d = Segment2D(points[j-1], points[j]).dist(agent->pos());
                if (nearestPoint==-1 || d<nearestDist || near)
                {

                    nearestDist = d;
                    nearestPoint = j;
                }
            }
        }
        if (nearestPoint==-1)
        {
            nearestPoint = points.count() - 1;
        }
        int k = nearestPoint;
        if (forcePoint)
        {
            k = currentPoint;
        }
        double dist = 0.0, dist2 = 0.0;
        dist += (points[k] - agent->pos()).length();
        dist2 = dist;
        for (int i=k+1;i<points.count();i++)
        {
            dist += (points[i] - points[i-1]).length();
        }
        dist2 = dist - dist2;
        double vTarget = vMax;
        if ((dMax>0) && (dist < (vMax*vMax / (2.0*dMax))))
            vTarget = sqrt(fabs(2.0*dMax*dist2 + finalVel * finalVel));
        if (k==points.count() - 1)
            vTarget = 0.0;
        if (k==0)
        {
            gotopointavoid->init(points[0], (points[1] - points[0]).norm(), vTarget * (points[1] - points[0]).norm() );
        }
        else {
            if (k != currentPoint)
            {
                gotopointavoid->setInitialPos(points[k-1]);
                gotopointavoid->setInitialDir(agent->dir());
                gotopointavoid->setInitialVel(agent->vel());
            }
            gotopointavoid->setFinalPos(points[k]);
            if (k<points.count() - 1) {
                gotopointavoid->setFinalDir((points[k+1] - points[k]).norm());
                vTarget *= fabs(((points[k+1] - points[k]).norm() * (points[k] - points[k-1]).norm()));
            }
            else gotopointavoid->setFinalDir((points[k] - points[k-1]).norm());
            if (k==points.count() - 1)
                gotopointavoid->setFinalVel(finalVel);
            else
                gotopointavoid->setFinalVel(vTarget * (points[k] - points[k-1]).norm());
        }
        currentPoint = k;
    }
    draw(Circle2D(points[currentPoint],0.1),0,360,"pink",true,0);
    for (int j=0;j<points.count();j++)
    {
        draw(points[j], 1, QColor("yellow"));
    }
    gotopointavoid->execute();
}

double CSkillFollowPoints::progress()
{
    return 0.0;
}

//-----------------------------------------------------------------------------
INIT_SKILL(CSkillGoAcrossLine, "goacrossline");

CSkillGoAcrossLine::CSkillGoAcrossLine(CAgent *_agent) : CSkill(_agent)
{
    aside = 0.0;
    target = 5.0;
    vNormMax = 1.0;
    vTanMax = 3.0;
    vTanFinal = 0.0;
    againstTarget = 0.0;
    maxacc = -1.0;
    maxdec = -1.0;
    targetPoint.assign(0,0);
}

CSkillGoAcrossLine::~CSkillGoAcrossLine()
{
}

double CSkillGoAcrossLine::progress()
{
    return 0.0;
}

void CSkillGoAcrossLine::execute()
{

}

//------------------------------------------------------------------------------
INIT_SKILL(CSkillThroughModeController, "throughmodecontroller");

CSkillThroughModeController::CSkillThroughModeController(CAgent *_agent) : CSkill(_agent)
{
    setAgent(_agent);
    tar.invalidate();
    init();
    wMode=true;
}

CSkillThroughModeController::~CSkillThroughModeController()
{
}

void CSkillThroughModeController::init()
{
    pidT.reset();
    pidN.reset();
    pidW.reset();
}

void CSkillThroughModeController::resetI()
{
    //debug("reseting I",1);
    pidT.resetI();
    pidN.resetI();
    pidW.resetI();
}

void CSkillThroughModeController::execute()
{

}

double CSkillThroughModeController::progress()
{
    return 0;
}

INIT_SKILL(CSkillGotoPosMV, "gotoposmv");

CSkillGotoPosMV::CSkillGotoPosMV(CAgent *_agent){
    agent = _agent;

    profile = new CMotionProfile(agent);
    LastVelHist=0;
}



CSkillGotoPosMV::~CSkillGotoPosMV(){
}


void CSkillGotoPosMV::init(CAgent *_agent , Vector2D _target , Vector2D _targetVel , Vector2D _targetDir){
    agent = _agent;
    turn.setAgent(agent);
    target = _target;
    lastTarget = _target;
    targetVel = _targetVel;

    targetDir = _targetDir;
    issetTargetDir = true;

    resetGotoPos();
}

void CSkillGotoPosMV::init(CAgent *_agent , Vector2D _target , Vector2D _targetVel){
    agent = _agent;
    turn.setAgent(agent);
    target = _target;
    lastTarget = _target;
    targetVel = _targetVel;

    issetTargetDir = false;

    resetGotoPos();
}


void CSkillGotoPosMV::resetGotoPos(){
    sumErr = 0;
    lastNErr = 0;
    diff = 0;
    VN = 0;
    VT = 0;
    lastTargetErr = 0;
    diffTarget = 0;
    targetSumErr = 0;

    velHistory.push_back(agent->vel().length());

    turn.setAgent(agent);
}


void CSkillGotoPosMV::execute(){
    //    targetVel.assign(0,0);
    //    bool avoidPenaltyArea = true;
    //    bool avoidCenterCircle = false;
    //    if( agent != NULL && target.valid() == true ){
    //        draw(Circle2D(target,0.1) , "blue" , true);

    //        //agent->runPlanner(agent->id(), target, avoidPenaltyArea, avoidCenterCircle);
    //        agent->initPlanner(agent->id() , target , QList<int>() , QList<int>() , true , true , 0);
    //        result = agent->pathPlannerResult;

    //        if( result.size() > 1 ){

    //            // theta_1 is the angle of robot velocity with respect to X
    //            // theta_r is the angle of robot direction with respect to X
    //            // theta_t is the angle of path direction with respect to X
    //            // etha is the angle of robot velocity with respect to path direction
    //            // beta is the angle of robot direction with respect to path direction
    //            // d_n is the normalized vector of perpendicular vector to path direction
    //            // d_t is the normalized vector of path direction vector
    //            // d_r is the normalized vector of robot direction vector

    //            Vector2D d_t = (result[1] - result[0]).norm();

    //            if( issetTargetDir == false ){
    //                targetDir = Vector2D(1,0);
    //            }

    //            //Vector2D d_t = (target - agent->pos()).norm();
    //            Vector2D d_n = d_t.rotatedVector(90);

    //            if( lastTarget != target )
    //                sumErr = 0;


    //            double distToTarget = 0;
    //            for( int i=1 ; i<result.size() ; i++ ){
    //                draw(Segment2D(result[i-1] , result[i]) , "black");
    //                distToTarget += result[i-1].dist(result[i]);
    //            }


    //            //			draw(QString("%1 %2").arg(distanceX).arg(distanceY) , Vector2D(0,1.6) , );

    //            double etha = (d_t.dir() - agent->vel().dir()).radian();
    //            double beta = (d_t.dir() - agent->dir().dir()).radian();
    //            //			debug(QString("etha %1").arg(etha * 180 / 3.141592) , D_ERROR);
    //            //			debug(QString("beta %1").arg(beta * 180 / 3.141592) , D_ERROR);


    //            /* a_mxg= amyl + (amxl - amyl)*fabs(cos(B))*/
    //            double a_MaxT = conf()->BangBang_AccNormal_Max() + (conf()->BangBang_AccTangent_Max() - conf()->BangBang_AccNormal_Max())*fabs(cos(beta));

    //            double d_MaxT = conf()->BangBang_DecNormal_Max() + (conf()->BangBang_DecTangent_Max() - conf()->BangBang_DecNormal_Max())*fabs(cos(beta));


    //            /* a_myg= amyl + (amxl - amyl)*fabs(sin(B))*/
    //            double a_MaxN = conf()->BangBang_AccNormal_Max() + (conf()->BangBang_AccTangent_Max() - conf()->BangBang_AccNormal_Max())*fabs(sin(beta));

    //            double v_MaxT = conf()->BangBang_VelNormal_Max() + (conf()->BangBang_VelTangent_Max() - conf()->BangBang_VelNormal_Max())*fabs(cos(beta));
    //            double v_MaxN = conf()->BangBang_VelNormal_Max() + (conf()->BangBang_VelTangent_Max() - conf()->BangBang_VelNormal_Max())*fabs(sin(beta));



    //            Vector2D v_T = agent->vel() * cos(etha);
    //            double vt_desired;

    //            double DT_C = fabs((targetVel.r2() - v_T.r2())/(2 * d_MaxT));

    //            Vector2D desiredVel;
    //            double disStepT = fabs(agent->pos().dist(result[1]));
    //            double tt=(conf()->Common_Command_Interval() / 1000.0);

    //            if( cos(etha) < (0 ) && agent->vel().length() > 0.01 ){
    //                vt_desired = -1*((conf()->Common_Command_Interval() / 1000.0) * (-d_MaxT)*5 + fabs(velHistory.back()));//agent->vel().length() * cos(etha) );
    //                debug(QString("HIST %1 %2").arg(velHistory.back()).arg(etha) , D_ERROR);
    //                debug(QString("STOPPING %1 %2").arg(vt_desired)  , D_ERROR);
    //            }
    //            else if( DT_C + .01 > distToTarget ){
    //                vt_desired = (-1.0*d_MaxT*tt + velHistory.back());
    //                debug(QString("HIST %1 %2").arg(velHistory.back()).arg(etha) , D_ERROR);
    //                debug(QString("DEC %1 %2").arg(vt_desired) .arg(DT_C), D_ERROR);

    //            }
    //            else{
    //                tt = (conf()->Common_Command_Interval() / 1000.0);
    //                vt_desired = (1.0*a_MaxT*tt + velHistory.back());
    //                if( vt_desired > v_MaxT )
    //                    vt_desired = v_MaxT;
    //                debug(QString("ACC %1 %2").arg(vt_desired) .arg(DT_C) , D_ERROR);
    //            }


    //            if( velHistory.count() > 100 )
    //                velHistory.pop_front();
    //            velHistory.push_back(vt_desired);




    //            double DT = conf()->Common_Command_Interval() / 1000.0;

    //            // this part is related to omitting point error


    //            if( distToTarget < 0.7 ){

    //                double distErr = (result[1] - agent->pos()).innerProduct(d_t);
    //                targetSumErr += distErr * DT;
    //                diffTarget = 0.5 * diffTarget + (1-0.5) * ( distErr - lastTargetErr) / DT;
    //                lastTargetErr = distErr;


    //                double kp = conf()->BangBang_KP();
    //                double kd = conf()->BangBang_KD();
    //                double ki = conf()->BangBang_KI();
    //                double gain = conf()->BangBang_Gain();

    //                VT = kp * distErr + ki * targetSumErr + kd * diffTarget + gain * VT;
    //                vt_desired = ((0.7 - distToTarget) * VT + distToTarget * vt_desired);
    //                debug(QString("POINT %1 %2").arg(vt_desired) .arg(DT_C) , D_ERROR);

    //            }

    //            desiredVel = d_t * vt_desired;
    //            LastVelHist=velHistory.back();



    //            // this part is related to navigate the robot to the path


    //            double distNErr = (result[1] - agent->pos()).innerProduct(d_n);

    //            sumErr += (distNErr * DT);
    //            diff = 0.5 * diff + (1 - 0.5)*(distNErr - lastNErr)/DT;
    //            lastNErr = distNErr;

    //            double kp = conf()->BangBang_KP();
    //            double kd = conf()->BangBang_KD();
    //            double ki = conf()->BangBang_KI();
    //            double gain = conf()->BangBang_Gain();

    //            VN = kp * distNErr + ki * sumErr + kd * diff + gain * VN;
    //            double vn_desired = VN;

    //            desiredVel += d_n * vn_desired;


    //            // this part is related to Turn the robot to the  desired direction

    //            DT = conf()->Common_Command_Interval() / 1000.0;

    //            agent->setRobotAbsVel(desiredVel.x , desiredVel.y , 0);


    //            if( distToTarget < 1 )
    //                turn.setDirection(targetDir);
    //            else
    //                turn.setDirection(targetDir);
    //            turn.setTurnMode(CSkillTurn::GoToPos);
    //            turn.execute();



    //            // MOTION PROFILE

    //            toProfile.clear();
    //            toProfile.append(agent->vel().length() * cos(etha));
    //            toProfile.append(VN);
    //            profile->myStore(toProfile);


    //            knowledge->plotWidgetCustom[0] = vt_desired;

    //            //////////
    //            /*
    //                        double dirErr = agent()->dir;

    //                        sumErr += (distNErr * DT);
    //                        diff = 0.5 * diff + (1 - 0.5)*(distNErr - lastNErr)/DT;


    //                        double kp = conf()->BangBang_KP();
    //                        double kd = conf()->BangBang_KD();
    //                        double ki = conf()->BangBang_KI();
    //                        double gain = conf()->BangBang_Gain();

    //                        VN = kp * distNErr + ki * sumErr + kd * diff + gain * VN;
    //                        double vn_desired = VN;

    //                        desiredVel += d_n * vn_desired; */


    //        }

    //    }
    //    else
    //        qDebug() << "NOT SET";
}

double CSkillGotoPosMV::progress()
{
    if(agent==NULL) return 0;
    double d = (agent->pos() - target).length();
    if (d<0.04) return 1.0;
    if (d<0.05) return 0.8;
    if (d<0.10) return 0.7;
    if (d<0.20) return 0.6;
    return 0;
}

