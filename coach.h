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
#include "plays/ourkickoff.h"
#include "plays/ourpenalty.h"
#include "plays/ourindirect.h"

#ifndef LARGE_FIELD
#include "plays/ourdirect.h"
#else
#include "plays/doublesizeourdirect.h"
#endif

#include "plays/theirkickoff.h"
#include "plays/theirpenalty.h"
#include "plays/theirindirect.h"
#include "plays/theirdirect.h"
#include "plays/forcestart.h"
#include "plays/playoff.h"
#include "plays/dynamicattack.h"
#include "roles.h"

class CCoach {

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
    CMasterPlay *selectedPlay;
    COurKickOff *ourKickOff;
    COurIndirect *ourIndirect;
#ifndef LARGE_FIELD
    COurDirect *ourDirect;
#else
    CDoubleSizeOurDirect *ourDirect;
#endif
    QTime playOnExecTime;
    QTime intentionTimePossession;
    QTime playMakeIntention;
    double playMakeIntentionInterval;
    double possessionIntentionInterval;
    double playMakeIntended;
    COurPenalty *ourPenalty;
    CTheirKickOff *theirKickOff;
    CTheirIndirect *theirIndirect;
    CTheirDirect *theirDirect;
    CTheirPenalty *theirPenalty;
    CForceStart *forceStart;
    CPlayOff *ourPlayOff;
    CDynamicAttack *dynamicAttack;
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
    QTime defenseTimeForVisionProblem[2];
    double shotToGoalthr ;
    void virtualPlayOffState();
    bool translationFlag;
    CKnowledge::State lastState;
    QTime translateTimeOut;
    int translationTimeOutTime;
    bool isBallcollide();
    void calcDesiredMarkCounts();
    ///////////////////////new play make and supporter chooser
    int playmakeId,supporterId;
    double playMakeTh;
    int lastPlayMake;
    void choosePlaymakeAndSupporter(bool needSupporter,bool defenseFirst);

    ///////////////////////////////////////////////

    enum attackState
    {
        SAFE = 0,
        FAST = 1,
        CRITICAL =2
    };
    attackState ourAttackState;
    void updateAttackState();

public:
    CKnowledge::ballPossesionState GlobalBallPState;
    ////////GUI Needed
    CPlayOff* playOff();


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
};

#define GetRole(Role, number) static_cast<Role*> (getRole(Role::Name, number))
#define GetRoleVar(var, Role, number) Role* var = static_cast<Role*> (getRole(Role::Name, number))
#define SetRole(Role, task) static_cast<Role*> (assignRole(Role::Name, task))
#define SetRoleId(Role, task, id) static_cast<Role*> (assignRole(Role::Name, task, id))
#define SetRoleVar(var, Role, task) Role* var = static_cast<Role*> (assignRole(Role::Name, task))
#define SetRoleIdVar(var, Role, task, id) Role* var = static_cast<Role*> (assignRole(Role::Name, task, id))
#endif // CCOACH_H
