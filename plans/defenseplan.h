#ifndef DEFENSE_H
#define DEFENSE_H

#include <plans/plan.h>
#include <cmath>

//#define OLD_FASTEST 1

#define LOOP_TIME_BYKK 0.016
#define AGENT_SPEED_BYKK 1.5
//struct velAndAccByKK {
//  double vel;
//  double acc;
//};

enum { OneTouchState , ClearState , NoState };

class DefensePlan : public Plan
{
protected:
    bool cmMode;
    bool clearGoalieF;
    double *OneTBallVel;
    double ourAgentsRad, ourGoalAreaCircleRad, ourGoalAreaLength, goalieCircleRadius, goalieCircleX, defenseCircleRadius,
    goalieDegThreshold, MinDefenseDistance, defenseMaxDeg,
    firstDefenseKickLine, secondDefenseKickLine, goalieKickThreshold;
    float clearDistanceForGoalie;
    bool isDefenseUpperThanGoalie;
    float tooFarDiffAngle;
    int defenseCount;
    int chipGKCounter;
#ifndef OLD_FASTEST
    NewFastestToBall fastestToBall;
#else
    FastestToBall fastestToBall;
#endif
    bool isItPossibleToClear;
    int upper_player;
    double catch_time;
    CSkillGotoPoint* gps[_MAX_NUM_PLAYERS];
    CSkillGotoPointAvoid *gpa[_MAX_NUM_PLAYERS];
    CSkillKick* kickSkill;
    CSkillTurn turn;
    CDefPos defPos;
    Vector2D pointForKick, oneToucherDir;
    Vector2D topGoal, downGoal, midGoal, ballVel;
    Vector2D goalKeeperTarget, goalieDirection , defensePoints[12], defenseTargets[12];
    bool executeSkill[5];
    int blockPassID;
    int oneTouchPositioning();
    int blocker;
    bool oneDefenseAndGoalie();
    bool noDefenseNoGoalie();
    bool isInThePenaltyArea(Vector2D _posofsth);
    void setPointToKick();
    void checkGoalieTarget();
    Vector2D getIntersectionWithPenaltyAreaGk(Segment2D _seg);
    void setGoalKeeperState();
    void setGoalKeeperTargetPoint();
    bool ballBehindGoalie, goalieOneTouch, goalieInPenaltyAreaPrediction, goalieClearMode, goalieStrictFollow, goalieFollow, ballIsOutOfField;
    double strictfollowThr;
    double behindBallThr;    
    bool besidePoleFlag;
    bool dangerForGoalieClear;
    int oneTouchCnt;    
    ////////////////////////////// AHZ ///////////////////    
    Line2D getBisectorLine(Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    Segment2D getBisectorSegment(Vector2D firstPoint , Vector2D originPoint , Vector2D secondPoint);
    void manToManMarkBlockPassInPlayOff(QList<Vector2D> opponentAgentsToBeMarkePossition , int ourMarkAgentsSize , double proportionOfDistance);
    void manToManMarkBlockShotInPlayOff(int _markAgentSize);
    bool isAgentsStuckTogether(QList<Vector2D> agentsPosition , QList<Vector2D> &stuckPositions , QList<int> stuckIndexs);
    void correctingTheAgentsAreStuckTogether(QList<Vector2D> &agentsPosition,QList<Vector2D> stuckPositions);
    void getIntersectionWithPenaltyAreaAHZ(Segment2D , Vector2D sol1 , Vector2D sol2);
    bool isIndirectArea(Vector2D);    

    //atousa
    Vector2D getGoaliePositionInOneDef(Vector2D _ballPos, double _limit1, double _limit2);
    double goalieThr;

    int angleDegreeThrNotStop = 0;
    double threshOld = 0.0;
    double ballCircleR = 0.5;
    double xLimitForblockingPass;
    bool isCrowdedInFrontOfPenaltyAreaByOurAgents;
    bool isCrowdedInFrontOfPenaltyAreaByOppAgents;
    bool ballISInpenaltyAreaAndDangerCircle;
    bool ballIsNotInPenaltyAreaAndIsInDangerCircle;
    bool ballIsInPenaltyAreaAndIsNotInDangerCircle;
    bool dangerForGoalieClearByOurAgents;
    bool dangerForGoalieClearByOppAgents;
    bool dangerForInsideOfThePenaltyArea;
    bool stopMode;
    bool dangerModeThresholdForClear;
    bool dangerModeThresholdForDanger;
    bool changeInMarkPlanFlag;
    bool manToManMarkBlockPassFlag;
    bool ballBool;
    QString lastStateForMark;
    QString stateForMark;
    int lastOpponentAgentsToBeMarkSize;
    QList <QString> markRoles;
    QList <QString> lastMarkRoles;
    QList <Vector2D> tenLastOpponentDirection;    
    Vector2D opponentPasserDirection;
    Vector2D opponentPasserPossition;
    Vector2D sumOfLastOpponentDirection;
    Vector2D tempAHZ;
    Vector2D sumOfLastOpponentPosition;

    int AHZCount;        
    ///////////////////////////////////////////////////
    void executeGoalKeeper();    
    Vector2D strictFollowBall(Vector2D _ballPos);
    Vector2D followBall(Vector2D _ballPos);
    Vector2D checkDefensePoint(CAgent* agent, const Vector2D& point);
    rcsc::Vector2D avoidKicker(int i, int kicker);
    void announceClearing(bool state);
    int decideNumOfMarks();
    kkDefPos tempDefPos;
    void matchingDefPos(int _defenseNum);        
    bool defenseOneTouchOrNot();
    bool defenseClearOrNot();
    void runClear();
    Vector2D getGoalieShootOutTarget(bool isBallPath);
    bool canReachToBall(int agentId, int theirAgentId);
    int decideShootOutMode();
    QList <Vector2D> lastBallPos;
    int penaltyShootoutMode=beforeTouch;
    void penaltyShootOutMode();
    CSkillGotoPointAvoid* striker_Robot;
    void penaltyMode();
    bool isStopped();
    bool isTheirNonPlayKick();
    bool isOurNonPlayKick();    
    enum exepMode{
        defOneTouch = 1,
        defClear = 2,
        NoneExep = 3
    };
    enum shootOutMode{
        beforeTouch,
        shootOutClear,
        ballBisector,
        skyDive

    };

    struct defenseExeptions{
        bool active;
        exepMode exeptionMode;
        int exepAgentId;
    };
    Vector2D ballPrediction(bool _isGoalie);

public:
    DefensePlan();
    void execute();
    void initGoalKeeper(CAgent *_goalieAgent = NULL);
    void initDefense(const QList <CAgent*> &_defenseAgents = QList<CAgent*>());
    int getNumberofThreeDefense();
    bool isAnyDefenderMarking() const;    
    void fillDefencePositionsTo(Vector2D *poses);    

    //////////////////HMD/////////////////
    QList<Vector2D> markPoses;
      QList<Vector2D> markAngs;
      double markRadius;
      double markRadiusStrict;
      double segmentpershoot;
      double segmentperpass;
      bool MantoManAllTransientFlag;
      bool LastTS;
      Vector2D dir;
    ///////////////////////////////////


private:
      ///////////////////////HMD///////////////    
    void findPos(int _markAgentSize);
    void findOppAgentsToMark();   
    bool isInTheIndirectAreaShoot(Vector2D);
    bool isInTheIndirectAreaPass(Vector2D);    
    QList<Vector2D> ShootBlockRatio(double, Vector2D);
    QList<Vector2D> PassBlockRatio(double,Vector2D);
    QList<Vector2D> indirectAvoidShoot(Vector2D);
    QList<Vector2D> indirectAvoidPass(Vector2D);
    int numberOfMarkers;
    QList<Vector2D> oppAgentsToMarkPos;
    QList<Vector2D> oppmarkedpos;
    QList<CRobot*>  oppAgentsToMark;    
    Vector2D posvel(CRobot*, double);
    QList<QPair<Vector2D, double> > sortdangerpassplayon(QList<Vector2D> oppposdanger);
    QList<QPair<Vector2D, double> > sortdangerpassplayoff(QList<Vector2D> oppposdanger);    
    ////////////////////////////////////////
    rcsc::Circle2D defenseAreaBottomCircle, defenseAreaTopCircle;
    rcsc::Segment2D defenseAreaLine;
    rcsc::Vector2D* getIntersectWithDefenseArea(const Line2D& segment, const Vector2D& blockPoint);
    rcsc::Vector2D* getIntersectWithDefenseArea(const Segment2D& segment, const Vector2D& blockPoint);
    rcsc::Vector2D* getIntersectWithDefenseArea(const Circle2D& circle, bool upperPoint);
    rcsc::Vector2D getIntersexWithGoalieEllipse(rcsc::Segment2D seg1, rcsc::Segment2D seg2, bool isDefenseUpperThanGoalieAngle);
    void assignSkill(CAgent *_agent , CSkill *_skill);
    bool isValidPoint(const Vector2D& point);
    void initVars(float goalCircleRad = 0.9); // default is 0.8
    void preCalculate();
    bool defenderForMark;
    bool doubleMarking;
    bool isDefenseFastest;
    bool clearflag;
    CAgent *goalKeeperAgent;
    QList <CAgent *> defenseAgents;
    int oneDefUpOrDown;
    int twoDefCurState;
    int lastStateOffPlay;
    int lastMarker[10];
    int markPointNum;
    int oneToucher;
    Vector2D defenseDirs[_MAX_NUM_PLAYERS];
    Vector2D ballPos;
    bool doOneTouch;
    bool doClear;
    int lastClearID;
    int defenseClearIndex;
    double lastClearDist;
    int clearFrameCnt;
    int lastTouchTheGoalie;
    bool distClearHysteresis;
    int lastOneTouchClearState;
    int histOneTouchClearCnt;    
    double GOTThresh;
    int GOTCounter;
    double thr;
    double noDefThr;    
    QList<Vector2D> ballPosHistory;
    velAndAccByKK getBallVelocityByPos();
    void calcPointForOneTouch();
    bool checkBallDangerForOneTouch();
    bool isInOneTouch;
    bool isOnetouch;
    int oneTouchCycleTest;
    bool checkStillBeingInOneTouch();
    int cycleCounter;
    Vector2D oneTouchPoint[2];
    bool oneTouchPointFlag;
    bool oneTouchPointFlagG;
    bool isPathToOppGoalieClear();
    Vector2D findBestPointForChipTarget(double &chipDist,bool isGoalie);    
    bool doBlockPass;    
    double timeToReach;
    Vector2D blockPassPoint;
    QList<int> dangerousOpp;    
    double goalieAreaHis;
    Vector2D goalieTargetDir;
    bool isBallGoingToOppArea();
    int isBallGoingToOppAreaCnt;
    double pushBallHist;
    int failureAtempCnt;
    int clearCnt;
    double savedClearDist;
    Vector2D savedClearPos;
    int goaliePassBlockCnt;
    Vector2D gBassBlockTargetSave;
    double predictThresh;    
    bool goalieClearFlag;
    bool inPenaltyAreaFlag;
    int predictMostDangrousOppToBall();
    Vector2D NearestDistanceToBallSegment(Vector2D point);
    bool behindAgent;
    kkDefPos defPosDecision;
    defenseExeptions defExceptions;
    void checkDefenseExeptions();
    void runDefenseExeptions();
    Vector2D runDefenseOneTouch();
    double defClearThr;
    bool defenseCheckBallDangerForOneTouch();
    bool forceBeingInClear();
    bool defClearFlag;
    double overDefThr;    
    int decideNumOfMarksInPlayOff(int _defenseCount);
};

#endif // DEFENSE_H
