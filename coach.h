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

class CCoach {

public:

    CKnowledge::ballPossesionState GlobalBallPState;
    ////////GUI Needed
    CPlayOff* playOff();
    CLoadPlayOffJson* getPlanLoader();
    bool inited;
    double playOnTime;
    CCoach(CAgent** _agents);
    ~CCoach();
    void execute();
    void saveGoalie();
    DefensePlan& getDefense();
    void swapAgents();   //refer to selectedId in knowledge
    bool swapAgents(int i, int j);
    void setOpponents();
    int mostSupporterNumber(int num);
    QList<int> findBestPoses(int numberOfPositionAgents,bool semiDynamic);
    CKnowledge::ballPossesionState lastBallPossesionState;
    CKnowledge::ballPossesionState isBallOurs();
    CKnowledge::ballPossesionState ballPStateIntented;
    static QMap<QString, EditData*> editData; //Contains Formations

private:
    double findMostPossible(Vector2D agentPos);
    CKnowledge::State kkLastState;
    CAgent *goalieAgent;
    CAgent *exeptionPlayMake;
    double exeptionPlayMakeThr;
    QList<CAgent*> defenseAgents;
    DefensePlan defenses;
    int preferedDefenseCounts , lastPreferredDefenseCounts;
    int preferedGoalieAgent;
    Vector2D defenseTargets[12];
    QTime intentionTimePossession;
    QTime playMakeIntention;
    QTime playOnExecTime;
    double playMakeIntentionInterval;
    double possessionIntentionInterval;
    double playMakeIntended;

    CMasterPlay          *selectedPlay;

    CPlayOff             *ourPlayOff;
    COurPenalty          *ourPenalty;
    COurKickOff          *ourKickOff;
    COurIndirect         *ourIndirect;
    COurBallPlacement    *ourBallPlacement;
    CTheirDirect         *theirDirect;
    CTheirPenalty        *theirPenalty;
    CTheirKickOff        *theirKickOff;
    CTheirIndirect       *theirIndirect;
    CTheirBallPlacement  *theirBallPlacement;
    CDoubleSizeOurDirect *ourDoubleSizeDirect;

    CForceStart          *forceStart;
    CDynamicAttack       *dynamicAttack;


    CRoleStop *stopRoles[_MAX_NUM_PLAYERS];
    QTime goalieTimer;
    bool goalieTrappedUnderGoalNet;

    CAgent** agents;
    int lastAssignCycle;

    QPair<QList<CAgent *>, QList<CAgent *> > lastAssign;
    void savePostAssignment();
    void loadPostAssignment();
    void checkRoleAssignments();

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
    bool decideAttack();
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
        SAFE = 0,
        FAST = 1,
        CRITICAL =2
    };
    attackState ourAttackState;
    void updateAttackState();


    /////////////////////////New Play Off
    void selectPlayOffMode(NGameOff::EMode& _mode);
    void initPlayOffMode(const NGameOff::EMode _mode,
                         const POMODE _gameMode,
                         const QList<int>& _agentSize);
    void setPlayOff(NGameOff::EMode _mode);
    void initStaticPlay(const POMODE _mode, const QList<int>& _agentSize);
    void initDynamicPlay();
    void initFastPlay();
    void initFirstPlay();
    void setStaticPlay();
    void setDynamicPlay();
    void setFirstPlay();
    void setFastPlay();
    CLoadPlayOffJson* m_planLoader;
    bool firstTime;

    bool isTagsMatched(const QStringList& base, const QStringList& required);
    bool isRegionMatched(const Vector2D& _ball, const double& _radius = 1.0); //circular Matching
    //TODO : squere or Liner matching
    NGameOff::SPlan* chooseMostSuccecfull(const QList<NGameOff::SPlan*>& plans);
    void matchPlan(NGameOff::SPlan* _plan, const QList<int>& _ourplayers);
    QStringList currentTags;

    NGameOff::SPlan* lastPlan;
    QList<int> lastPlayers;
    CKnowledge::ballPossesionState ballPState;
    //////////////Decide Attack functions
    bool decideHalt               (QList<int>&);
    bool decideStop               (QList<int>&);
    bool decideOurKickOff         (QList<int>&);
    bool decideTheirKickOff       (QList<int>&);
    bool decideOurIndirect        (QList<int>&);
    bool decideTheirIndirect      (QList<int>&);
    bool decideOurDirect          (QList<int>&);
    bool decideTheirDirect        (QList<int>&);
    bool decideOurPenalty         (QList<int>&);
    bool decideTheirPenalty       (QList<int>&);
    bool decideStart              (QList<int>&);
    bool decideNormalStart        (QList<int>&);
    bool decideOurBallPlacement   (QList<int>&);
    bool decideTheirBallPlacement (QList<int>&);
    bool decideNull               (QList<int>&);
    /////////////////////////////////////


};

#define GetRole(Role, number) static_cast<Role*> (getRole(Role::Name, number))
#define GetRoleVar(var, Role, number) Role* var = static_cast<Role*> (getRole(Role::Name, number))
#define SetRole(Role, task) static_cast<Role*> (assignRole(Role::Name, task))
#define SetRoleId(Role, task, id) static_cast<Role*> (assignRole(Role::Name, task, id))
#define SetRoleVar(var, Role, task) Role* var = static_cast<Role*> (assignRole(Role::Name, task))
#define SetRoleIdVar(var, Role, task, id) Role* var = static_cast<Role*> (assignRole(Role::Name, task, id))
#endif // CCOACH_H
