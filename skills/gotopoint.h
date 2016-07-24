#ifndef GotoPoint_H
#define GotoPoint_H

#include <skill.h>
#include "trajectoryplanner.h"
#include "planner.h"
#include "pid.h"
#include "mathtools.h"
#include "newbangbang.h"
#include <QTime>
#include <QFile>

#include "navigation.h"

class CSkillTurn : public CSkill
{
protected:
    CPID pid;
    double dynamicP,dynamicI,dynamicD;
public:
    enum TurnMode {
        Fast = 0,
        Slow = 1,
        Moving = 2,
        Penalty = 3,
        Intercept = 4,
        GoToPos = 5,
        Dynamic = 6
    };
    void setDynamic(double _P,double _I,double _D);
    DEF_SKILL(CSkillTurn);
    SkillProperty(CSkillTurn, Vector2D, Direction, direction);
    SkillProperty(CSkillTurn, TurnMode, TurnMode, turnMode);
};


///////////////////////////////////////////////////////////////////////////
/// MiroSot GotoPoint Skill


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  New Goto Point Class Powered by DON
//  start 12/4/2015
class _PID{
public:
    double kp,kd,ki,_I,_P,_D,I_saturate,I_clear_error,error,pError;
    _PID () : kp(0),kd(0),ki(0),_I(0),_P(0),_D(0),I_saturate(0),I_clear_error(0) {}
    _PID (double _kp,double _kd,double _ki,double _I_sat,double _I_clear_error) : kp(_kp),kd(_kd),ki(_ki),_I(0),_P(0),_D(0),I_saturate(_I_sat),I_clear_error(_I_clear_error) {}

    double PID_OUT()
    {
        _P=kp*error;

        if(fabs(error) < I_clear_error)
            _I = 0;
        else if(_I<=I_saturate || I_saturate == 0)
            _I += error;
        else
            _I = I_saturate/ki;

        _D = error-pError;
        return _P + ki*_I + kd*_D;
    }
};

enum gpMode {
    GPACC1 = 0,
    GPACC2 = 1,
    GPVCONST = 2,
    GPDEC1 = 3,
    GPDEC2 = 4,
    GPPOS = 5,
    GPDIVE = 6
};

class CskillNewGotoPoint : public CSkill
{
private:
    _PID *angPid;
    _PID *posPid;
    _PID *posXpid;
    _PID *posYpid;
    _PID *speedPidX;
    _PID *speedPidY;
    _PID *thPid;
    _PID *velPid;

    ///////Arash.Z//////////
    //////////if boll dec = true the function return optimal dec and
    //////////otherwise return optimal acc//////////
    double optimalAccOrDec(double agentDirInRadian, bool dec);
    ///////////////////////
protected:
    //CAgent *agent;

    double _Vf;




    AngleDeg targetDir;
    Vector2D targetVel;
    //CskillNewGotoPoint(CAgent *_agent) : agent(_agent),_VmDesire(4),_Vf(0),_Acc(4),_Dec(-4),slow(false) {/*loopTimer.start();*/speedPid = new _PID(0,0,0,0,0);posPid = new _PID(0,0,0,0,0);angPid= new _PID(2,0,0,0,0);}
    //void execute();
    //void setAgent(CAgent *_agent);

    double agentDist;
    double kkVf, kkVc;
    double _Vx, _Vy;
    double kkVm;
    double realAcc, realDec;
    double appliedAcc,appliedDec;
    double kkVmaxDesire, kkVmax;
    double kkPosDist;
    double kkVDesire;
    double decThr;
    double kkThrPos;
    double kkThrVConst;
    double angleOfMovement;

    double kkX1, kkX2, kkX3;


    gpMode decideMode();
    AngleDeg agentMovementTh;
    AngleDeg lastPath;


public:
    DEF_SKILL(CskillNewGotoPoint);
    double timeRemaining();
    static double gTimeRemaining(CAgent *_agent,Vector2D _destination = Vector2D(0,0) );

    gpMode GPMode;

    void init(Vector2D _target, Vector2D _targetDir, Vector2D _targetVel = Vector2D(0.0, 0.0),bool dynamicStart = true);

    SkillProperty(CskillNewGotoPoint, bool, DynamicStart, dynamicStart);
    SkillProperty(CskillNewGotoPoint, Vector2D, InitialPos, pos1);
    SkillProperty(CskillNewGotoPoint, Vector2D, FinalPos, targetPos);
    SkillProperty(CskillNewGotoPoint, Vector2D, InitialDir, dir1);
    SkillProperty(CskillNewGotoPoint, Vector2D, FinalDir, FinalDir);
    SkillProperty(CskillNewGotoPoint, Vector2D, InitialVel, vel1);
    SkillProperty(CskillNewGotoPoint, Vector2D, FinalVel, vel2);
    SkillProperty(CskillNewGotoPoint, Vector2D, LookAt, lookat);
    SkillProperty(CskillNewGotoPoint, Vector2D, BiasVel, biasVel);
    SkillProperty(CskillNewGotoPoint, Vector2D, ShotTarget, target);
    SkillProperty(CskillNewGotoPoint, double, MaxAcceleration, _Acc);
    SkillProperty(CskillNewGotoPoint, double, MaxDeceleration, _Dec);
    SkillProperty(CskillNewGotoPoint, double, MaxVelocity, _VmDesire);
    SkillProperty(CskillNewGotoPoint, double, MaxAccelerationNormal, aMaxNorm);
    SkillProperty(CskillNewGotoPoint, double, MaxDecelerationNormal, dMaxNorm);
    SkillProperty(CskillNewGotoPoint, double, MaxVelocityNormal, vMaxNorm);
    SkillProperty(CskillNewGotoPoint, double, ConstantVelocity, vConst);
    SkillProperty(CskillNewGotoPoint, double, TurningDist, turningDist);
    SkillProperty(CskillNewGotoPoint, bool, FasterEnd, fasterEnd);
    SkillProperty(CskillNewGotoPoint, bool, NotEnd, notEnd);
    SkillProperty(CskillNewGotoPoint, bool, FastW, fastw);
    SkillProperty(CskillNewGotoPoint, bool, RecordProfile, recordProf);
    SkillProperty(CskillNewGotoPoint, bool, BallMode, ballMode);
    SkillProperty(CskillNewGotoPoint, bool, SlowShot, slow);
    SkillProperty(CskillNewGotoPoint, bool, moveLookingTarget, lootTarget);
    SkillProperty(CskillNewGotoPoint, bool, InterceptMode, interceptMode);
    SkillProperty(CskillNewGotoPoint, bool, LookForward, lookForward);
    SkillProperty(CskillNewGotoPoint, bool, OneTouchMode, oneTouchMode);
    SkillProperty(CskillNewGotoPoint, bool, SlowMode, slowMode);
    SkillProperty(CskillNewGotoPoint, bool, NoPID, noPID);
    SkillProperty(CskillNewGotoPoint, bool, PenaltyKick, penaltyKick);
    SkillProperty(CskillNewGotoPoint, bool, GoalieMode, goalieMode);
    SkillProperty(CskillNewGotoPoint, Vector2D, MotionDir, motiondir);
    SkillProperty(CskillNewGotoPoint, float, ForwardBias, forwardBias);
    SkillProperty(CskillNewGotoPoint, bool, DiveMode, diveMode);
    SkillProperty(CskillNewGotoPoint, bool, VerySlow, verySlow);

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkillGotoPoint : public CSkill
{


private:
    _PID *angPid;
    _PID *posPid;
    _PID *posXpid;
    _PID *posYpid;
    _PID *speedPidX;
    _PID *speedPidY;
    _PID *thPid;
    _PID *velPid;
    Vector2D startingPoint;
     ///////Arash.Z//////////
    //////////if boll dec = true the function return optimal dec and
    //////////otherwise return optimal acc//////////
    double optimalAccOrDec(double agentDirInRadian, bool dec);

    ///////////////////////
    void targetValidate();
    void trajectoryPlanner();
    double appliedTh;
    //////////////////////


protected:

    Vector2D targetVel;
    Vector2D agentPos;
    Vector2D agentVel;
    double agentBestAcc;
    double agentBestDec;
    double agentDist;
    double agentVc;
    double _Vx, _Vy;
    double agentVm;
    double realAcc, realDec;
    double appliedAcc,appliedDec;
    double posPidDist;
    double agentVDesire;
    double decThr;
    double posThr;
    double vConstThr;
    double angleOfMovement;

    double agentX1, agentX2, agentX3;

    gpMode decideMode();
    AngleDeg agentMovementTh;
    AngleDeg lastPath;

public:
    virtual double timeNeeded();
    int counting;
    DEF_SKILL(CSkillGotoPoint);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget* parent);
    virtual void generateFromConfig(CAgent *a);
    double timeRemaining();
    gpMode lastGPmode;
    gpMode currentGPmode;
    void init(Vector2D _target, Vector2D _targetDir, Vector2D _targetVel = Vector2D(0.0, 0.0),bool dynamicStart = true);
    SkillProperty(CSkillGotoPoint, bool, DynamicStart, dynamicStart);
    SkillProperty(CSkillGotoPoint, Vector2D, InitialPos, pos1);
    SkillProperty(CSkillGotoPoint, Vector2D, FinalPos, targetPos);
    SkillProperty(CSkillGotoPoint, Vector2D, InitialDir, dir1);
    SkillProperty(CSkillGotoPoint, Vector2D, FinalDir, targetDir);
    SkillProperty(CSkillGotoPoint, Vector2D, InitialVel, vel1);
    SkillProperty(CSkillGotoPoint, Vector2D, FinalVel, vel2);
    SkillProperty(CSkillGotoPoint, Vector2D, LookAt, lookat);
    SkillProperty(CSkillGotoPoint, Vector2D, BiasVel, biasVel);
    SkillProperty(CSkillGotoPoint, Vector2D, ShotTarget, target);
    SkillProperty(CSkillGotoPoint, double, MaxAcceleration, _Acc);
    SkillProperty(CSkillGotoPoint, double, MaxDeceleration, _Dec);
    SkillProperty(CSkillGotoPoint, double, MaxVelocity, _VmDesire);
    SkillProperty(CSkillGotoPoint, double, MaxAccelerationNormal, aMaxNorm);
    SkillProperty(CSkillGotoPoint, double, MaxDecelerationNormal, dMaxNorm);
    SkillProperty(CSkillGotoPoint, double, MaxVelocityNormal, vMaxNorm);
    SkillProperty(CSkillGotoPoint, double, ConstantVelocity, vConst);
    SkillProperty(CSkillGotoPoint, double, TurningDist, turningDist);
    SkillProperty(CSkillGotoPoint, bool, FasterEnd, fasterEnd);
    SkillProperty(CSkillGotoPoint, bool, NotEnd, notEnd);
    SkillProperty(CSkillGotoPoint, bool, FastW, fastw);
    SkillProperty(CSkillGotoPoint, bool, RecordProfile, recordProf);
    SkillProperty(CSkillGotoPoint, bool, BallMode, ballMode);
    SkillProperty(CSkillGotoPoint, bool, SlowShot, slow);
    SkillProperty(CSkillGotoPoint, bool, moveLookingTarget, lootTarget);
    SkillProperty(CSkillGotoPoint, bool, InterceptMode, interceptMode);
    SkillProperty(CSkillGotoPoint, bool, LookForward, lookForward);
    SkillProperty(CSkillGotoPoint, bool, OneTouchMode, oneTouchMode);
    SkillProperty(CSkillGotoPoint, bool, SlowMode, slowMode);
    SkillProperty(CSkillGotoPoint, bool, NoPID, noPID);
    SkillProperty(CSkillGotoPoint, bool, PenaltyKick, penaltyKick);
    SkillProperty(CSkillGotoPoint, bool, GoalieMode, goalieMode);
    SkillProperty(CSkillGotoPoint, Vector2D, MotionDir, motiondir);
    SkillProperty(CSkillGotoPoint, float, ForwardBias, forwardBias);
    SkillProperty(CSkillGotoPoint, bool, DiveMode, diveMode);
    SkillProperty(CSkillGotoPoint, bool, VerySlow, verySlow);
    SkillProperty(CSkillGotoPoint, bool , Smooth,smooth);
};

class CSkillFollowPoints;

class CSkillGotoPointAvoid : public CSkillGotoPoint
{
private:
    CMotionProfile *prof;
    bool pathRestarted;
protected:
    int stucked;
    Vector2D lastPoint;
    CSkillGotoPoint* gotopoint;
    QList<int> ourRelaxList , oppRelaxList;
    int counter;
    bool inited;
    void followPath();
    Vector2D averageDir;
public:
    double timeStarted, timeEstimated; //for skill widget
    DEF_SKILL(CSkillGotoPointAvoid);
    CSkillGotoPointAvoid* noRelax();
    CSkillGotoPointAvoid* ourRelax(int element);
    CSkillGotoPointAvoid* oppRelax(int element);
    CSkillGotoPointAvoid* setTargetLook(Vector2D finalPos, Vector2D lookAtPoint);
    CSkillGotoPointAvoid* setTarget(Vector2D finalPos, Vector2D finalDir);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget* parent);
    virtual void generateFromConfig(CAgent *a);
    QList<Vector2D> result;
    SkillProperty(CSkillGotoPointAvoid, bool, AvoidPenaltyArea, avoidPenaltyArea);
    SkillProperty(CSkillGotoPointAvoid, bool, KeepLooking, keeplooking);
    SkillProperty(CSkillGotoPointAvoid, double, ExtendStep, extendStep);
    SkillProperty(CSkillGotoPointAvoid, bool, DynamicStart, dynamicStart);
    SkillProperty(CSkillGotoPointAvoid, bool, Plan2, plan2);
    SkillProperty(CSkillGotoPointAvoid, bool, NoAvoid, noAvoid);
    SkillProperty(CSkillGotoPointAvoid, bool, AvoidCenterCircle, avoidCenterCircle);
    SkillProperty(CSkillGotoPointAvoid, double, BallObstacleRadius , ballObstacleRadius);
    SkillProperty(CSkillGotoPoint, bool, ADiveMode, diveMode);
    SkillProperty(CSkillGotoPoint, bool, AvoidBall, avoidBall);
    SkillProperty(CSkillGotoPoint, bool, AvoidGoalPosts, avoidGoalPosts);
    friend class CSkillFollowPoints;
};


class CSkillFollowPoints : public CSkillGotoPointAvoid
{
protected:
    CSkillGotoPointAvoid* gotopointavoid;
public:
    DEF_SKILL(CSkillFollowPoints);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget* parent);
    virtual void generateFromConfig(CAgent *a);
    CSkillFollowPoints* addPoint(Vector2D point);
    CSkillFollowPoints* clearPoints();
    SkillProperty(CSkillFollowPoints, double, DecMax, dMax);
    SkillProperty(CSkillFollowPoints, double, VelMax, vMax);
    SkillProperty(CSkillFollowPoints, QList<Vector2D>, Points, points);
    SkillProperty(CSkillFollowPoints, int, CurrentPoint, currentPoint);
    SkillProperty(CSkillFollowPoints, bool, ForcePoint, forcePoint);
    SkillProperty(CSkillGotoPointAvoid, Vector2D, FinalVel, finalVel);
};

class CSkillGoAcrossLine : public CSkill
{
protected:

public:
    DEF_SKILL(CSkillGoAcrossLine);
    SkillProperty(CSkillGoAcrossLine, Vector2D, Origin, origin);
    SkillProperty(CSkillGoAcrossLine, Vector2D, Direction, dir);
    SkillProperty(CSkillGoAcrossLine, double, MaxAcceleration, maxacc);
    SkillProperty(CSkillGoAcrossLine, double, MaxDeceleration, maxdec);
    SkillProperty(CSkillGoAcrossLine, double, Target, target);
    SkillProperty(CSkillGoAcrossLine, double, VTanMax, vTanMax);
    SkillProperty(CSkillGoAcrossLine, double, VTanFinal, vTanFinal);
    SkillProperty(CSkillGoAcrossLine, double, VNormMax, vNormMax);
    SkillProperty(CSkillGoAcrossLine, Vector2D, LookAt, lookAt);
    SkillProperty(CSkillGoAcrossLine, double, Aside, aside);
    SkillProperty(CSkillGoAcrossLine, double, AgainstTarget, againstTarget);
    SkillProperty(CSkillGoAcrossLine, Vector2D, TargetPoint, targetPoint);
};

class CSkillThroughModeController : public CSkill
{
protected:
    CPID pidN,pidT,pidW;
    bool wMode;
public:
    DEF_SKILL(CSkillThroughModeController);
    void init();
    void resetI();
    SkillProperty(CSkillThroughModeController, Vector2D, Target, tar);
    SkillProperty(CSkillThroughModeController, Vector2D, CurrentPosition, pos);
    SkillProperty(CSkillThroughModeController, Vector2D, CurrentVelocity, vel);
    SkillProperty(CSkillThroughModeController, Vector2D, CurrentDirection, dir);
    SkillProperty(CSkillThroughModeController, Vector2D, FinalPosition, posf);
    SkillProperty(CSkillThroughModeController, Vector2D, FinalVelocity, velf);

};


/* New motion class, powered by Masoud and Vahid. Started Since 11/24/2012 */
class CSkillGotoPosMV : public CSkill{
private:
protected:
    vector<Vector2D> result;
    Vector2D lastMousePos;
public:
    DEF_SKILL(CSkillGotoPosMV);
    void init(CAgent *_agent , Vector2D _target , Vector2D _targetVel , Vector2D _targetDir );
    void init(CAgent *_agent , Vector2D _target , Vector2D _targetVel);
    void resetGotoPos();
    virtual CSkillConfigWidget* generateConfigWidget(QWidget* parent);
    virtual void generateFromConfig(CAgent *a);
    double sumErr , lastNErr , diff, VN , VT , targetSumErr , lastTargetErr , diffTarget;
    bool issetTargetDir;

    QList <double> velHistory;
    double LastVelHist;
    CSkillTurn turn;
    //	QFile *file;
    CMotionProfile *profile;
    QList <double> toProfile;

    //	Property(CAgent *, Agent , agent);
    SkillProperty(CSkillGotoPosMV , Vector2D , Target , target);
    SkillProperty(CSkillGotoPosMV , Vector2D , TargetVel , targetVel);
    SkillProperty(CSkillGotoPosMV , Vector2D , LastTarget , lastTarget);
    SkillProperty(CSkillGotoPosMV , Vector2D , TargetDir , targetDir);
};


#endif // GOTOPOINT_H
