#include "newbangbang.h"
#include <QDebug>

#ifdef Q_OS_MAC
using namespace std;
#endif

CNewBangBang::CNewBangBang()
{
    lastV.clear();
    lastV1.clear();
    posPidDist = 0.15;
    posPidThr = 0;
    decThr = 0;
    posPid = new _PID(3.5,2,0,0,0);
    angPid = new _PID(3,0,0,0,0);
    thPid = new _PID(1.5,0,0,0,0);
    smooth = 0;
    for( int i = 0; i < _MAX_NUM_PLAYERS; i++)
    {
        lastV.append(0.f);
        lastV1.append(0.f);
    }
    vmax = 4;
    angPath = false;
}

/*
double CNewBangBang::optimalAccOrDec(double agentDir, bool dec)
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
*/
bangBangMode CNewBangBang::decidePlan()
{

    double _x3;

    if(Vel2 == 0)
    {
        _x3 = ((posPidDist*posPid->kp) *(posPidDist*posPid->kp)  - currentVel*currentVel) / (-1.8 * fabs(dmax)) + 0.1*currentVel;
    }
    else
    {
        _x3 = (Vel2*Vel2 - currentVel*currentVel) / (-1.5 * fabs(dmax)) + 0.05 *currentVel;
    }


    if(((agentPos.dist(pos2) < posPidDist + posPidThr) && (Vel2 == 0) )/* || (agentPos.dist(pos2) < 0.15)*/) {
        decThr = 0;
        constThr = 0;
        return _bangBangPosPID;

    }
    else {
        posPidThr = 0;
        if((_x3 > 0) && (agentPos.dist(pos2) < _x3 + decThr) ) {
            if(currentVel < 0.5)
                decThr = 0;
            else
                decThr = 0.1;
            constThr = 0;
            return _bangBangDec;

        }
        else if((agentVel.length() >= velMax - constThr)) {
            constThr = 0.3;
            decThr = 0;
            return _bangBangConst;
        }
        else {
            decThr=0;
            constThr = 0;
            return _bangBangAcc;
        }


    }
}

void CNewBangBang::trajectoryPlanner()
{
    velMax = vmax;

    if(smooth)
    {
        if((agentMovementTh - lastPath).degree() > 20 && (agentMovementTh - lastPath).degree() < 100 && currentVel > 1) {
            agentMovementTh = lastPath + 60;
            velMax = 1;

        }
        else if((agentMovementTh - lastPath).degree() < -20 && (agentMovementTh - lastPath).degree() > -100 && currentVel > 1) {
            agentMovementTh = lastPath - 60;
            velMax = 1;

        }
        else if((agentMovementTh - lastPath).degree() >= 100 && currentVel > 1) {
            agentMovementTh = lastPath + 80;
            velMax = 0.5;

        }
        else if((agentMovementTh - lastPath).degree() <= -100 && currentVel > 1) {
            agentMovementTh = lastPath - 80;
            velMax = 0.5;

        }
    }
    ///////////////////////////////////////////// th pid
    thPid->kp =0;
    thPid->error = (agentMovementTh - agentVel.norm().th()).radian();
    if(fabs(thPid->error > 1) || currentVel < 0.5 || agentPos.dist(pos2) >3 ||( fabs((agentMovementTh - agentDir.th()).degree()) > 80 && fabs((agentMovementTh - agentDir.th()).degree()) < 100 )   )
        thPid->error =0;

    appliedTh = agentMovementTh.radian() +thPid->PID_OUT();


}
void CNewBangBang::bangBangSpeed(Vector2D _agentPos,Vector2D _agentVel,Vector2D _agentDir,Vector2D _pos2,Vector2D _dir2,double _V2,double dt,double & _Vx,double & _Vy, double & _W)
{
    pos2 = _pos2;
    dir2 = _dir2;
    Vel2 = _V2;
    agentPos =_agentPos;
    agentVel = _agentVel;
    currentVel = agentVel.length();
    agentDir =_agentDir;
    movementTh = pos2 - agentPos;
    if(angPath) {
        if(angKp)
            angPid->kp = angKp;
        else
            angPid->kp = 1;
    }
    else
    {
        angPid->kp = 3;
    }
    angPid->error = (dir2.th() -  agentDir.th()).radian();

    draw(QString("vel2 : %1").arg(Vel2),Vector2D(2,1.5));
    agentMovementTh = movementTh.th();

    if ( fabs((agentMovementTh - agentDir.th()).degree()) > 80 && fabs((agentMovementTh - agentDir.th()).degree()) < 100 )
    {
        amax = 60;
    }
    if(slow)
    {
        posPid->kp = 2;
        amax = 10;
        posPid->kd = conf()->BangBang_posKD();
        posPid->ki = conf()->BangBang_posKI();
    }
    else if(oneTouch)
    {
        posPid->kp = 7;
        amax = 60;
        posPid->kd = 5;
        posPid->ki = conf()->BangBang_posKI();
    }
    else
    {
        posPid->kp = conf()->BangBang_posKP();
        posPid->kd = conf()->BangBang_posKD();
        posPid->ki = conf()->BangBang_posKI();
    }
    thPid->kp = conf()->BangBang_thKP();
    thPid->ki = conf()->BangBang_thKI();
    thPid->kd = conf()->BangBang_thKD();
    //////////////////////// dec calculations
    double vp =(posPidDist*posPid->kp);
    double moreDec = 0.7;
    double decOffset = 0.5;





    switch(decidePlan())
    {
    case _bangBangPosPID:
        posPid->error = agentPos.dist(pos2);
        vDes = min(posPid->PID_OUT(),velMax);
        break;
    case _bangBangConst:
        vDes = vmax;
        break;
    case _bangBangDec:
        if(v2 == 0)
        {
            vDes = sqrt(fabs(2*dmax*(agentPos.dist(pos2))*moreDec) + vp *vp) - decOffset;
        }
        else
        {
            vDes = sqrt(fabs(2*dmax*(agentPos.dist(pos2))*moreDec) + Vel2*Vel2) - decOffset;
        }
        break;
    case _bangBangAcc:
        if(currentVel < 0.2)
        {
            vDes = 0.3;
        }
        else
        {
            vDes = currentVel + dt*(amax);
        }
        break;
    }
    trajectoryPlanner();
    lastPath = agentVel.th();
    /////////////////////th pid
    _Vx =  (vDes)*cos(appliedTh);
    _Vy =  (vDes)*sin(appliedTh);
    _W = angPid->PID_OUT();
    posPid->pError = posPid->error;

}
