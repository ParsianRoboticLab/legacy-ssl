#ifndef SRSKILLS_H
#define SRSKILLS_H


#include <skill.h>
#include "trajectoryplanner.h"
#include "planner.h"
#include "pid.h"
#include "mathtools.h"
#include "newbangbang.h"
#include <QTime>
#include <QFile>

#include <QDebug>
#include "knowledge.h"
#include <QFile>
#include <algorithm>
#include <math.h>
#include <varswidget.h>

#include "navigation.h"

class _PID{
public:
  double kp,kd,ki,_I,_P,_D,I_saturate,I_clear_error,error,pError;
  _PID () : kp(0),kd(0),ki(0),_I(0),_P(0),_D(0),I_saturate(0),I_clear_error(0) {}
  _PID (double _kp,double _kd,double _ki,double _I_sat,double _I_clear_error) : kp(_kp),kd(_kd),ki(_ki),_I(0),_P(0),_D(0),I_saturate(_I_sat),I_clear_error(_I_clear_error) {}

  double PID_OUT()
  {
    _P=kp*error;

    if(error <= I_clear_error)
      _I = 0;
    else if(_I<I_saturate)
      _I += error;
    else
      _I = I_saturate;

    _D = error-pError;

    return _P + ki*_I + kd*_D;
  }
};

enum kkGPMode {
    GPACC1 = 0,
    GPACC2 = 1,
    GPVCONST = 2,
    GPDEC1 = 3,
    GPDEC2 = 4,
    GPPOS = 5
};

class SRSkillGotoPoint : public CSkill
{
private:
    _PID *angPid;
    _PID *posPid;
    _PID *speedPidX;
    _PID *speedPidY;

    CAgent *CRAgent;
protected:
  //CAgent *agent;

  double _Vf;

  AngleDeg targetDir;
  Vector2D targetVel;
  //CskillNewGotoPoint(CAgent *_agent) : agent(_agent),_VmDesire(4),_Vf(0),_Acc(4),_Dec(-4),slow(false) {speedPid = new _PID(0,0,0,0,0);posPid = new _PID(0,0,0,0,0);angPid= new _PID(2,0,0,0,0);}
  //void execute();
  //void setAgent(CAgent *_agent);

  double kkDist;
  double kkVf, kkVc;
  double kkVx, kkVy;
  double kkVm;
  double kkAcc, kkDec;
  double kkVmaxDesire, kkVmax;
  double kkPosDist;
  double kkVDesire;

  double kkThrPos;
  double kkThrVConst;

  double kkX1, kkX2, kkX3;


  kkGPMode decideMode();
  AngleDeg kkMovementTh;

public:
  DEF_SKILL(SRSkillGotoPoint);

  kkGPMode GPMode;

  void init(int _id, Vector2D _target, Vector2D _targetDir, Vector2D _targetVel = Vector2D(0.0, 0.0),bool dynamicStart = true);

  SkillProperty(SRSkillGotoPoint, bool, DynamicStart, dynamicStart);
  SkillProperty(SRSkillGotoPoint, Vector2D, InitialPos, pos1);
  SkillProperty(SRSkillGotoPoint, Vector2D, FinalPos, targetPos);
  SkillProperty(SRSkillGotoPoint, Vector2D, InitialDir, dir1);
  SkillProperty(SRSkillGotoPoint, Vector2D, FinalDir, FinalDir);
  SkillProperty(SRSkillGotoPoint, Vector2D, InitialVel, vel1);
  SkillProperty(SRSkillGotoPoint, Vector2D, FinalVel, vel2);
  SkillProperty(SRSkillGotoPoint, Vector2D, LookAt, lookat);
  SkillProperty(SRSkillGotoPoint, Vector2D, BiasVel, biasVel);
  SkillProperty(SRSkillGotoPoint, Vector2D, ShotTarget, target);
  SkillProperty(SRSkillGotoPoint, double, MaxAcceleration, _Acc);
  SkillProperty(SRSkillGotoPoint, double, MaxDeceleration, _Dec);
  SkillProperty(SRSkillGotoPoint, double, MaxVelocity, _VmDesire);
  SkillProperty(SRSkillGotoPoint, double, MaxAccelerationNormal, aMaxNorm);
  SkillProperty(SRSkillGotoPoint, double, MaxDecelerationNormal, dMaxNorm);
  SkillProperty(SRSkillGotoPoint, double, MaxVelocityNormal, vMaxNorm);
  SkillProperty(SRSkillGotoPoint, double, ConstantVelocity, vConst);
  SkillProperty(SRSkillGotoPoint, double, TurningDist, turningDist);
  SkillProperty(SRSkillGotoPoint, bool, FasterEnd, fasterEnd);
  SkillProperty(SRSkillGotoPoint, bool, NotEnd, notEnd);
  SkillProperty(SRSkillGotoPoint, bool, FastW, fastw);
  SkillProperty(SRSkillGotoPoint, bool, RecordProfile, recordProf);
  SkillProperty(SRSkillGotoPoint, bool, BallMode, ballMode);
  SkillProperty(SRSkillGotoPoint, bool, SlowShot, slow);
  SkillProperty(SRSkillGotoPoint, bool, moveLookingTarget, lootTarget);
  SkillProperty(SRSkillGotoPoint, bool, InterceptMode, interceptMode);
  SkillProperty(SRSkillGotoPoint, bool, LookForward, lookForward);
  SkillProperty(SRSkillGotoPoint, bool, OneTouchMode, oneTouchMode);
  SkillProperty(SRSkillGotoPoint, bool, SlowMode, slowMode);
  SkillProperty(SRSkillGotoPoint, bool, NoPID, noPID);
  SkillProperty(SRSkillGotoPoint, bool, PenaltyKick, penaltyKick);
  SkillProperty(SRSkillGotoPoint, bool, GoalieMode, goalieMode);
  SkillProperty(SRSkillGotoPoint, Vector2D, MotionDir, motiondir);
  SkillProperty(SRSkillGotoPoint, float, ForwardBias, forwardBias);
  SkillProperty(SRSkillGotoPoint, int, AgentId, agentId);

};

class SRSkillGotoPointAvoid : public SRSkillGotoPoint
{
private:
    CMotionProfile *prof;
    bool pathRestarted;

    bool isCircleEmpty(double tRad);
    double matchBiggestCircle(double tDist);

    bool isPathClear(Vector2D tSPos, Vector2D tEPos, double threshold = 0.10);

    Vector2D getIntersectedPosByCircle(Circle2D tCircle, Vector2D tInnerPos, Vector2D tOutterPos);

    _PID *angPid;
    _PID *speedPidX;
    _PID *speedPidY;

    double kkVx, kkVy;
    AngleDeg kkMovementTh;
    double kkDist;

    //double kkVMaxDesire;
    double kkVMax;
    double kkVReal;

    CAgent *CRAgent;
protected:
    int stucked;
    Vector2D lastPoint;
    SRSkillGotoPoint* gotopoint;
    QList<int> ourRelaxList , oppRelaxList;
    int counter;
    bool inited;
    void followPath();
    Vector2D averageDir;


public:
    double timeStarted, timeEstimated; //for skill widget
    DEF_SKILL(SRSkillGotoPointAvoid);
    SRSkillGotoPointAvoid* noRelax();
    SRSkillGotoPointAvoid* ourRelax(int element);
    SRSkillGotoPointAvoid* oppRelax(int element);
    SRSkillGotoPointAvoid* setTargetLook(Vector2D finalPos, Vector2D lookAtPoint);
    SRSkillGotoPointAvoid* setTarget(Vector2D finalPos, Vector2D finalDir);
    QList<Vector2D> result;
    SkillProperty(SRSkillGotoPointAvoid, bool, AvoidPenaltyArea, avoidPenaltyArea);
    SkillProperty(SRSkillGotoPointAvoid, bool, KeepLooking, keeplooking);
    SkillProperty(SRSkillGotoPointAvoid, double, ExtendStep, extendStep);
    SkillProperty(SRSkillGotoPointAvoid, bool, DynamicStart, dynamicStart);
    SkillProperty(SRSkillGotoPointAvoid, bool, Plan2, plan2);
    SkillProperty(SRSkillGotoPointAvoid, bool, NoAvoid, noAvoid);
    SkillProperty(SRSkillGotoPointAvoid, bool, AvoidCenterCircle, avoidCenterCircle);
    SkillProperty(SRSkillGotoPointAvoid, double, BallObstacleRadius , ballObstacleRadius);
    SkillProperty(SRSkillGotoPointAvoid, double,  KKMaxVel, kkVMaxDesire);
    SkillProperty(SRSkillGotoPointAvoid, bool, AvoidBall, kkAvoidBall);
    SkillProperty(SRSkillGotoPointAvoid, bool, KickMode, kkKickMode);
};

#endif // SRSKILLS_H
