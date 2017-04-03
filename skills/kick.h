#ifndef KICK_H
#define KICK_H

#include <gotoball.h>
#include "pid.h"

class QTime;

class CSkillKickOneTouch;
class CSkillKick;



#define penaltyAreaAvoidanceRadius (0.9)

class CSkillIntercept : public CSkill
{
protected:
    CSkillGotoPoint* gotopoint;
    CSkillGotoPointAvoid* gotopointavoid;
    int behindTarget;
    Vector2D behindTargetVel;
    int aside;
    bool calcd;
    bool doOneTouch;
    bool onetouching;
    bool goForKick;
    CPID* pidPX;
    CPID* pidPY;
    CPID* pidWB;
    int avoidance1, avoidance2;
    int p_avoidance_state;
    double iy;
    CSkillTurn turn;
    CAgent* fakeAgent;
    Vector2D lastV;
    bool isBallFrozen;
    int minNum;
public:
    DEF_SKILL(CSkillIntercept);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget* parent);
    virtual void generateFromConfig(CAgent *a);
    static Vector2D avoidTarget(CAgent* agent, CMovingObject* target, Vector2D q, Vector2D goal, Vector2D goal2, int& state);
    SkillProperty(CSkillIntercept, Vector2D, TargetDir, targetDir);
    SkillProperty(CSkillIntercept, double, TimeFactor, timeFactor);
    SkillProperty(CSkillIntercept, bool, KickMode, kickMode);
    SkillProperty(CSkillIntercept, bool, AvoidPenaltyArea, avoidPenaltyArea);
    SkillProperty(CSkillIntercept, double, BehindTargetAcc, behindTargetAcc);
    SkillProperty(CSkillIntercept, double, BehindTargetDist, behindTargetDist);
    SkillProperty(CSkillIntercept, double, BehindTargetAngle, behindTargetAngle);
    SkillProperty(CSkillIntercept, bool, SagMode, sagMode);
    SkillProperty(CSkillIntercept, Vector2D, Goal, goal);
    SkillProperty(CSkillIntercept, bool, Clear, clear);
    SkillProperty(CSkillIntercept, bool, DontKick, dontKick);
    SkillPropertyNoSet(CSkillIntercept, CMovingObject*, Target, target);
};


class CSkillNewKick
{
protected:

    bool lastThroughed;
    int chipWait;
    int chipSpinSpeed;
    bool readyToChip;
    CAgent* agent0;
    bool ballflag;
    bool ballWasFar;

public:
    CSkillNewKick(CAgent *_agent);

    CAgent *agent;
    bool kickable();
    void execute();
    void setAgent(CAgent *_agent);
    SkillProperty(CSkillNewKick, double, Tolerance, tol);
    SkillProperty(CSkillNewKick, bool, Chip, chip);
    SkillProperty(CSkillNewKick, int, KickSpeed, kickSpeed);
    SkillProperty(CSkillNewKick, int, Spin, spin);
    SkillProperty(CSkillNewKick, bool, Slow, slow);
    SkillProperty(CSkillNewKick, bool, Turn, turn);
    SkillProperty(CSkillNewKick, bool, AutoChipSpeed, autoChipSpeed);
    SkillProperty(CSkillNewKick, bool, ThroughMode, throughMode);
    SkillProperty(CSkillNewKick, bool, ParallelMode, parallelMode);
    SkillProperty(CSkillNewKick, bool, Clear, clear);
    SkillProperty(CSkillNewKick, int, WaitFrames, waitFrames);
    SkillProperty(CSkillNewKick, bool, AvoidOurPenaltyArea, avoidOurPenaltyArea);
    SkillProperty(CSkillNewKick, bool, AvoidOppPenaltyArea, avoidOppPenaltyArea);
    SkillProperty(CSkillNewKick, bool, InterceptMode, interceptMode);
    SkillProperty(CSkillNewKick, bool, DontKick, dontKick);
    SkillProperty(CSkillNewKick, bool, SagMode, sagMode);
    SkillProperty(CSkillNewKick, bool, DontRecvPass, dontRecvPass);
    SkillProperty(CSkillNewKick, bool, RecvChip, recvChip);
    SkillProperty(CSkillNewKick, bool, IsGotoPointAvoid, isGotoPointAvoid);
    SkillProperty(CSkillNewKick, bool, PenaltyKick, penaltyKick);
    SkillProperty(CSkillNewKick, bool, TurnLastMomentForPenalty, turnLastMomentForPenalty);
    SkillPropertyNoSet(CSkillNewKick, Vector2D, Target, target);



};



/////////////////////////////////////////////////////////////// receive pass skill created by DON MHMMD SHIRAZI
enum kkRPMode {
    RPWAITPOS = 1,
    RPINTERSECT = 2,
    RPDAMP = 3,
    RPRECEIVE = 4,
    RPNONE = 5
};

class CSkillReceivePass : public CSkill
{
private:
    CSkillGotoPointAvoid* gotopointavoid;
    Vector2D kkBallPos;
    Vector2D kkAgentPos;
    kkRPMode receivePassMode;
    kkRPMode decideMode();
    double cirThresh;
    double kickCirThresh;
    double velThresh;
public:
    double ballRealVel;
    DEF_SKILL(CSkillReceivePass);
    /*CSkillReceivePass() {CSkillReceivePass(NULL);}
    CSkillReceivePass(CAgent*);*/
    bool received;
    /*void execute();
    void setAgent(CAgent*);*/
    bool isReceived();
    SkillProperty(CSkillReceivePass, Vector2D, Target, target);
    SkillProperty(CSkillReceivePass, bool , AvoidOurPenaltyArea, avoidOurPenaltyArea);
    SkillProperty(CSkillReceivePass, bool , AvoidOppPenaltyArea, avoidOppPenaltyArea);
    SkillProperty(CSkillReceivePass, bool , Slow, slow);
    SkillProperty(CSkillReceivePass, float , ReceiveRadius, receiveR);
    SkillProperty(CSkillReceivePass, bool , IgnoreAngle, ignoreAngle);
    SkillProperty(CSkillReceivePass, Vector2D, IATargetDir, IATargetDir);

};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum kckMode {
    KDIRECT = 0,
    KINDIRECCT = 1,
    KJTURN = 2,
    KTURN = 3,
    KINTERCEPT = 4,
    KKICK = 5,
    KPENALTY = 6,
    KAVOIDOPPENALTY =7,
    KWAITANDKICK = 8,
    KDONTKICK = 9,
    KWAITFORTURN = 10,
    KGOALIE = 11

};

class CSkillKick : public CSkill
{
private:
    double kkRadLimit;
    kckMode decideMode();
    kckMode kickMode;
    _PID *angPid;
    _PID *speedPidX;
    _PID *speedPidY;
    Vector2D agentPos;
    Vector2D agentDir;
    QQueue <Vector2D> dirQueue;
    double kkDist;
    double kkBallDist;
    double kkRadThresh;
    double kkAngleThresh;
    double kkTAngleThresh;
    double kkKickThresh;
    double kkKickAngThresh;
    double jTurnThr;
    Vector2D oneTouchDir;

    double kkJTurnThresh;
    double kkMovementTheta;
    Vector2D ballPos;
    double ballRealVel;
    AngleDeg kickFinalDir;
    Vector2D finalDirVec;
    Vector2D finalPos;
    void direct();
    void indirect();
    void jTurn();
    void turnForKick();
    void kkKick();
    void kkIntercept();
    void kkPenalty();
    void avoidOppPenalty();
    void waitAndKick();
    void kDontKick();
    void kWaitForTurn();
    void kgoalie();
    void findPosToGo();
    void findPosToGoAlt();
    double oneTouchAngle(Vector2D pos,Vector2D vel,Vector2D ballVel,Vector2D ballDir,Vector2D goal,double landa,double gamma);
    Vector2D jTurnStartPos;
    bool isJturn;

    CSkillGotoPointAvoid *gpa;
    CSkillIntercept *kickIntercept;
    Vector2D getTurnFunc(double ang, double _r, double _angOff = 0);



protected:
    CSkillGotoBall* gotoball;
    CSkillGotoPointAvoid* gotopoint;
    CSkillIntercept* intercept;
    bool lastThroughed;
    int chipWait;
    int chipSpinSpeed;
    bool readyToChip;
    CAgent* agent0;
    bool ballflag;
    bool ballWasFar;
    bool kickerOn;
public:
    DEF_SKILL(CSkillKick);
    void resetI();
    bool kickable();
    Vector2D findMostPossible();
    virtual void generateFromConfig(CAgent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    SkillProperty(CSkillKick, double, Tolerance, tol);
    SkillProperty(CSkillKick, bool, Chip, chip);
    SkillProperty(CSkillKick, int, KickSpeed, kickSpeed);
    SkillProperty(CSkillKick, int, Spin, spin);
    SkillProperty(CSkillKick, bool, Slow, slow);
    SkillProperty(CSkillKick, bool, Turn, turn);
    SkillProperty(CSkillKick, bool, AutoChipSpeed, autoChipSpeed);
    SkillProperty(CSkillKick, bool, ThroughMode, throughMode);
    SkillProperty(CSkillKick, bool, ParallelMode, parallelMode);
    SkillProperty(CSkillKick, bool, Clear, clear);
    SkillProperty(CSkillKick, int, WaitFrames, waitFrames);
    SkillProperty(CSkillKick, bool, AvoidPenaltyArea, avoidPenaltyArea);
    SkillProperty(CSkillKick, bool, AvoidOppPenaltyArea, avoidOppPenaltyArea);
    SkillProperty(CSkillKick, bool, InterceptMode, interceptMode);
    SkillProperty(CSkillKick, bool, DontKick, dontKick);
    SkillProperty(CSkillKick, bool, SagMode, sagMode);
    SkillProperty(CSkillKick, bool, DontRecvPass, dontRecvPass);
    SkillProperty(CSkillKick, bool, RecvChip, recvChip);
    SkillProperty(CSkillKick, bool, PenaltyKick, penaltyKick);
    SkillProperty(CSkillKick, bool, TurnLastMomentForPenalty, turnLastMomentForPenalty);
    SkillPropertyNoSet(CSkillKick, Vector2D, Target, target);
    SkillProperty(CSkillKick, bool, IsGotoPointAvoid, isGotoPointAvoid);
    SkillProperty(CSkillKick, bool, ShotToEmptySpot, kkShotEmpySpot);
    SkillProperty(CSkillKick, bool, PassProfiler, passProfiler);
    SkillProperty(CSkillKick, bool, VeryFine, veryFine);
    SkillProperty(CSkillKick, bool, GoalieMode, goalieMode);
    SkillProperty(CSkillKick, bool, AlternateMode, alternateMode);
    SkillProperty(CSkillKick, double,KickAngTol, kickAngTol);
    SkillProperty(CSkillKick, bool,KickWithCenterOfDribbler, kickWithCenterOfDribbler);

};


////////////////////////////////////////////

enum keepMode {
    DISTURB = 0,
    RELAX = 1,
    FARSTOP = 2,
    FARMOVING = 3
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkillKeep : public CSkill
{
private:
    keepMode decideMode();
    Circle2D ballArea;
    double ballAreaRad;
    Vector2D ballPos;
    Vector2D agentPos;
    Vector2D ballVel;
    void farStop();
    void farMoving();
    void relax();
    void disturb();
    int findCritAgents();
    Vector2D finalDir;
    Vector2D waitPos;
    _PID *angPid;
    int maximumAgentNum;
    QList<CRobot*> critAgents;

    Vector2D initPos;
protected:
    CAgent* agent0;
    CSkillGotoPointAvoid *GPA;
    CSkillIntercept *intercept;

public:

    void reset();
    DEF_SKILL(CSkillKeep);
    //SkillProperty(CSkillKeep, double, Tolerance, tol);
    SkillProperty(CSkillKeep, bool, Slow, slow);

};


enum passMode {
    KICK = 0,
    WAIT = 1
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkillNewPass : public CSkill
{
private:

    passMode decideMode();
    CSkillKick *pKick;
    CSkillKeep *pkeep;
    CSkillGotoPointAvoid *gpa;
    QTime spinFtime;
public:
    void setPasser(CAgent *val);
    bool isPasserNearTheBall;
    DEF_SKILL(CSkillNewPass);

    SkillProperty(CSkillNewPass, bool, Chip, chip);
    SkillProperty(CSkillNewPass, bool, Slow, slow);
    SkillProperty(CSkillNewPass, bool, AvoidPenaltyArea, avoidPenaltyArea);
    SkillProperty(CSkillNewPass, Vector2D, Target, target);
    SkillProperty(CSkillNewPass, bool, IsGotoPointAvoid, isGotoPointAvoid);
    SkillProperty(CSkillNewPass, int, PassSpeed, passSpeed);
    SkillProperty(CSkillNewPass, bool, ShotOrNot, shotOrNot);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSkillPass : public CSkill
{
private:

    passMode decideMode();
    bool stopPasser;
protected:
    CAgent* agentReceiver;
    CAgent* agent0;
    CSkillGotoPointAvoid *GPAP,*GPAR;
    CSkillKick *pKick;
    CSkillReceivePass *RcvP;

public:
    void setPasser(CAgent *val);
    void isPassComplited();
    bool isPasserNearTheBall;
    void setReceiverAgent(CAgent *_agent);
    void reset();
    DEF_SKILL(CSkillPass);
    //	virtual void generateFromConfig(CAgent *a);
    //	virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    SkillProperty(CSkillPass, double, Tolerance, tol);
    SkillProperty(CSkillPass, bool, Chip, chip);
    SkillProperty(CSkillPass, bool, IsForceSpeed, IsForceSpeed);
    SkillProperty(CSkillPass, int, KickSpeed, kickSpeed);
    SkillProperty(CSkillPass, int, Spin, spin);
    SkillProperty(CSkillPass, bool, Slow, slow);
    SkillProperty(CSkillPass, bool, AutoChipSpeed, autoChipSpeed);
    SkillProperty(CSkillPass, bool, AvoidPenaltyArea, avoidPenaltyArea);
    SkillProperty(CSkillPass, bool, DontRecvPass, dontRecvPass);
    SkillProperty(CSkillPass, bool, RecvChip, recvChip);
    SkillProperty(CSkillPass, Vector2D, Target, target);
    SkillProperty(CSkillPass, bool, IsGotoPointAvoid, isGotoPointAvoid);
    SkillProperty(CSkillPass, bool, PassSpeed, passSpeed);
    SkillProperty(CSkillPass, bool, PassComplited, passComplited);
    SkillProperty(CSkillPass, bool, PassProfiler, passProfiler);


};



////////////////////////////////////////////////////
enum kkOTMode {
    OTWAITPOS = 1,
    OTINTERSECT = 2,
    OTKICK = 3,
    OTKOSKHOL = 4
};

class CSkillKickOneTouch : public CSkill
{
protected:
    int p_area_avoidance_state;
    CSkillGotoPointAvoid* gotopointavoid;
    CSkillTurn turn;
    bool oneTouched;
    CSkillKick* kick;
    QTime* timeAfterForceKick;
    bool forceKicked;
    Vector2D pos0,pos1;
    CPID *pidW,*pidP;
private:
    Vector2D kkBallPos;
    Vector2D kkAgentPos;
    kkOTMode oneTouchMode;
    kkOTMode decideMode();
    double cirThresh;
    double kickCirThresh;
    double velThresh;
    Vector2D findMostPossible();
public:
    double ballRealVel;
    static Vector2D newOneTouchAng(CAgent *oneTouchAgent,Vector2D target,double kickSpeed,double alpha,double beta,double gama);
    static double oneTouchAngle(Vector2D pos, Vector2D vel, Vector2D ballVel, Vector2D ballDir, Vector2D goal, double landa, double gamma);
    static double oneTouchAngle(Vector2D pos, Vector2D dir, Vector2D vel, Vector2D ballVel, Vector2D ballDir, Vector2D goal, double landa, double gamma);
    DEF_SKILL(CSkillKickOneTouch);
    virtual void generateFromConfig(CAgent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    SkillProperty(CSkillKickOneTouch, bool, Chip, chip);
    SkillProperty(CSkillKickOneTouch, int, KickSpeed, kickSpeed);
    SkillProperty(CSkillKickOneTouch, Vector2D, WaitPos, waitpos);
    SkillProperty(CSkillKickOneTouch, Vector2D, Target, target);
    SkillProperty(CSkillKickOneTouch, double, DistToBallLine, distToBallLine);
    SkillProperty(CSkillKickOneTouch, double, VelocityToBallLine, velToBallLine);
    SkillProperty(CSkillKickOneTouch, bool, RecvChip, recvChip);
    SkillProperty(CSkillKickOneTouch, bool, AvoidPenaltyArea, avoidPenaltyArea);
    SkillProperty(CSkillKickOneTouch, bool, MoveTowardTheBall, moveTowardTheBall);
    SkillProperty(CSkillKickOneTouch, bool, ShotToEmptySpot, shotToEmptySpot);
};



#endif // KICK_H
