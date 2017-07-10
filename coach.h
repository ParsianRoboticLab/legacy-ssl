//be name khoda
#ifndef CCOACH_H
#define CCOACH_H

#include <QObject>
#include <QtScript>
#include <QScriptEngine>
#include <QtScriptTools>

#include "base.h"
#include "worldmodel.h"
#include "agent.h"
#include "role.h"
#include "conditions.h"
#include "plans/plans.h"
#include "plays/plays.h"
#include "roles.h"
#include "tools/planloader.h"
#include "tools/loadplayoffjson.h"
#include "util/rng.h"
#include "chalftimelineup.h"

class CCoach {

public:

    ////////GUI Needed
    CPlayOff* playOff();
    CLoadPlayOffJson* getPlanLoader();
    bool inited;
    double playOnTime;
    CCoach(CAgent** _agents);
    ~CCoach();
    static ShotSpot getShotSpot(const Vector2D& _ball, const Vector2D& _shotPos);
    void execute();
    void saveGoalie();
    DefensePlan& getDefense();
    void swapAgents();   //refer to selectedId in knowledge
    bool swapAgents(int i, int j);
    void setOpponents();
    int mostSupporterNumber(int num);
    QList<int> findBestPoses(int numberOfPositionAgents);
    CKnowledge::ballPossesionState lastBallPossesionState;
    CKnowledge::ballPossesionState isBallOurs();
    CKnowledge::ballPossesionState ballPStateIntented;
    static QMap<QString, EditData*> editData; //Contains Formations
  /*  ClassProperty(CCoach, Vector2D, LastBallVelPm, lastBallVelPM, updated);
    ClassProperty(CCoach, Vector2D, LastBallPos, lastBallPos, updated);*/

private:
    Vector2D passPos;
    bool passPlayMake;
    Vector2D lastBallVelPM;
    Vector2D lastBallPos;
    bool updated;
    double findMostPossible(Vector2D agentPos);
    CKnowledge::State kkLastState;
    CAgent *goalieAgent;
    CAgent *exeptionPlayMake;
    double exeptionPlayMakeThr;
    QList<CAgent*> defenseAgents;
    DefensePlan defenses;
    int preferedDefenseCounts ,lastPreferredDefenseCounts;
    int preferedGoalieAgent;
    Vector2D defenseTargets[12];
    QTime intentionTimePossession;
    QTime playMakeIntention;
    QTime playOnExecTime;
    QTime playMakeSelectTime;
    double playMakeIntentionInterval;
    double possessionIntentionInterval;
    double playMakeIntended;

    CMasterPlay          *selectedPlay;

    CPlayOff             *ourPlayOff;
    COurPenalty          *ourPenalty;
    COurKickOff          *ourKickOff;
    COurIndirect         *ourIndirect;
    COurBallPlacement    *ourBallPlacement;
    CHalftimeLineup      *halfTimeLineup;




    CTheirDirect         *theirDirect;
    CTheirPenalty        *theirPenalty;
    CTheirKickOff        *theirKickOff;
    CTheirIndirect       *theirIndirect;
    CTheirBallPlacement  *theirBallPlacement;
    CDoubleSizeOurDirect *ourDoubleSizeDirect;

    CForceStart          *forceStart;
    CDynamicAttack       *dynamicAttack;

    CStopPlay            *stopPlay;


    CRoleStop *stopRoles[_MAX_NUM_PLAYERS];
    QTime goalieTimer;
    bool goalieTrappedUnderGoalNet;

    CAgent** agents;
    int lastAssignCycle;

    QPair<QList<CAgent *>, QList<CAgent *> > lastAssign;
    void savePostAssignment();
    void loadPostAssignment();
    void checkRoleAssignments();
    ///////manage over number of agents
    ///
    ///
    QList <int> robotsIdHist;
    bool first;
    QList <int> missMatchIds;
     ///////////////////////////////////////
    int lastSelected;
    int cyclesWaitAfterballMoved;
    QList <CAgent*> lastDefenseAgents;
    QList <int> lastDefenderAgents;
    void checkTransitionToForceStart();
    void updateKnowledgeVars();
    void doIntention();
    void clearIntentions();
    void assignGoalieAgent(int goalieID);
    void assignDefenseAgents(int defenseCount);
    void checkGoalieInsight();
    void decidePreferedDefenseAgentsCountAndGoalieAgent();
    void decideAttack();
    void decideDefense();
    void decidePlayOff(QList<int>& _ourplayers, POMODE _mode = INDIRECT);
    void decidePlayOn(QList<int>& ourPlayers, QList<int>& lastPlayers);
    QTime defenseTimeForVisionProblem[2];
    double shotToGoalthr ;
    void virtualTheirPlayOffState();
    bool transientFlag;
    CKnowledge::State lastState;
    QTime trasientTimeOut;
    int translationTimeOutTime;
    bool isBallcollide();
    void calcDesiredMarkCounts(); // not used at all
    ///////////////////////new play make and supporter chooser
    int playmakeId,supporterId;
    double playMakeTh;
    int lastPlayMake;
    void choosePlaymakeAndSupporter(bool defenseFirst);

    ///////////////////////////////////////////////

    enum attackState
    {
        SAFE     = 0,
        FAST     = 1,
        CRITICAL = 2,
        BallInOppJaw  = 3
    };
    attackState ourAttackState;
    void updateAttackState();


    /////////////////////////New Play Off
    void selectPlayOffMode(int agentSize, NGameOff::EMode& _mode);
    void initPlayOffMode(const NGameOff::EMode _mode,
                         const POMODE _gameMode,
                         const QList<int>& _agentSize);
    void setPlayOff(NGameOff::EMode _mode);
    int PlayoffLFUPolicy(QList<SPlan*> prevValidPlans, QList<SPlan*> validPlans);
    int PlayoffShufflePolicy(QList<SPlan*> prevValidPlans, QList<SPlan*> validPlans);
    int LFUPlan(QList<SPlan*> validPlans);
    void MinChanceOfValidplans(QList<SPlan*> validPlans);
    void initStaticPlay(const POMODE _mode, const QList<int>& _agentSize);
    void initDynamicPlay(QList<int> _ourplayers);
    void initFastPlay(QList<int> _ourplayers);
    void initFirstPlay(QList<int> _ourplayers);
    void setStaticPlay();
    void setDynamicPlay();
    void setFirstPlay();
    void setFastPlay();

    QList<SPlan*> getValidPlans(const POMODE _mode, const QList<int> &_ourPlayers);

    QList<SPlan *> getMatchedPlans(const QStringList& _tags, const QList<SPlan *> &_plans);
    QList<SPlan *> getMatchedPlans(int _shotSpot, const QList<SPlan*> &_plans);

    CLoadPlayOffJson* m_planLoader;
    bool firstTime, firstPlay, firstIsFinished;

    bool isTagsMatched(const QStringList& base, const QStringList& required);
    bool isTagsNearMatched(const QStringList& base, const QStringList& required);

    bool isRegionMatched(const Vector2D& _ball, const double& _radius = 1.0); //circular Matching
    //TODO : squere or Liner matching
    NGameOff::SPlan* chooseMostSuccecfull(const QList<NGameOff::SPlan*>& plans);
    void LFUInit(QList<NGameOff::SPlan*> allPlans);
    void saveLFUReapeatData(QList<SPlan*> plans);
    void ShufflePlanIndexing(QList<SPlan*> Plans);
    void matchPlan(NGameOff::SPlan* _plan, const QList<int>& _ourplayers);
    void checkGUItoRefineMatch(NGameOff::SPlan* _plan, const QList<int> &_ourplayers);
    QStringList currentTags;
    int preferedShotSpot;
    QStringList guiTags; // TODO : add tags to gui playoffTab

    NGameOff::SPlan* lastPlan;
    QList<int> lastPlayers;
    CKnowledge::ballPossesionState ballPState;
    Vector2D lastBallVel;
    //////////////Decide Attack functions
    void decideHalt               (QList<int>&);
    void decideStop               (QList<int>&);
    void decideOurKickOff         (QList<int>&);
    void decideTheirKickOff       (QList<int>&);
    void decideOurIndirect        (QList<int>&);
    void decideTheirIndirect      (QList<int>&);
    void decideOurDirect          (QList<int>&);
    void decideTheirDirect        (QList<int>&);
    void decideOurPenalty         (QList<int>&);
    void decideTheirPenalty       (QList<int>&);
    void decideNormalStart        (QList<int>&);
    void decideStart              (QList<int>&);
    void decideOurBallPlacement   (QList<int>&);
    void decideTheirBallPlacement (QList<int>&);
    void decideHalfTimeLineUp   (QList<int>&);
    void decideNull               (QList<int>&);
    /////////////////////////////////////
    unsigned int staticPlayoffPlansCounter;
    unsigned int shuffleCounter, shuffleSize;
    bool shuffled;
    double LFUPlanID,maxLFU, LFU;
    QList<SPlan *> LFUList;
    bool firstPlanRepeatInit;
    QTextStream out;
    QFile playoffPlanSelectionDataFile;
    QList<int> staticPlayoffPlansShuffleIndexing;
    int minChance, minChanceRepeat;

    bool isFastPlay();
    ///HMD
    bool checkOverdef();
    double overDefThr;
};



#define GetRole(Role, number) static_cast<Role*> (getRole(Role::Name, number))
#define GetRoleVar(var, Role, number) Role* var = static_cast<Role*> (getRole(Role::Name, number))
#define SetRole(Role, task) static_cast<Role*> (assignRole(Role::Name, task))
#define SetRoleId(Role, task, id) static_cast<Role*> (assignRole(Role::Name, task, id))
#define SetRoleVar(var, Role, task) Role* var = static_cast<Role*> (assignRole(Role::Name, task))
#define SetRoleIdVar(var, Role, task, id) Role* var = static_cast<Role*> (assignRole(Role::Name, task, id))
#endif // CCOACH_H
