#ifndef PLAYOFF_H
#define PLAYOFF_H

#include "masterplay.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <QMessageBox>

#define POBALLPOS Vector2D(1234, 8456)

#define ClassProperty(skill,type,name,local,chflag) \
    public: inline type get##name() {return local;} \
    public: inline skill* set##name(type val) {local = val;chflag = true;return this;} \
    protected: type local



struct robotAttr {
    int index;
    int agent;
    int skillNum;
    bool isAng;
};

enum POffSkills {
    NoSkill = 0,
    PassSkill = 1,
    ReceivePassSkill = 2,
    ShotToGoalSkill = 3,
    ChipToGoalSkill = 4,
    OneTouchSkill = 5,
    MoveSkill = 6,
    ReceivePassIASkill = 7
};

struct playOffSkill {
    POffSkills name;
    int data[2];
    int targetIndex;
    int targetAgent;
};

struct playOffRobot {
    Vector2D pos;
    AngleDeg angle;
    double tolerance;
    QList<playOffSkill> skill;
};

struct POInitPos {
    Vector2D ball;
    Vector2D Agent[6];
};

enum POMODE {
    DIRECT   = 1,
    INDIRECT = 2,
    KICKOFF  = 3
};

struct STuneParams {
    double lastDist = 0.5;
};

struct SPlanConfig {
    int         chance = 1;
    QString     name   = "mahi";
    STuneParams tuneParams;
};

struct SPlayOffPlan {

    SPlayOffPlan() = default;

    SPlayOffPlan(const SPlayOffPlan &_toCopy) {
        this->planMode  = _toCopy.planMode ;
        this->agentSize = _toCopy.agentSize;
        this->initPos   = _toCopy.initPos  ;
        this->config    = _toCopy.config   ;
        for(int i = 0;i < 6;i++) {
            this->AgentPlan[i].clear();
            for(int j = 0; j < _toCopy.AgentPlan[i].size(); j++) {
                this->AgentPlan[i].append(_toCopy.AgentPlan[i].at(j));
            }
        }
    }

    SPlayOffPlan& operator =(const SPlayOffPlan &_insert) {
        this->planMode  = _insert.planMode ;
        this->agentSize = _insert.agentSize;
        this->initPos   = _insert.initPos  ;
        this->config    = _insert.config   ;
        for(int i = 0;i < 6;i++) {
            this->AgentPlan[i].clear();
            for(int j = 0;j < _insert.AgentPlan[i].size();j++) {
                this->AgentPlan[i].append(_insert.AgentPlan[i].at(j));
            }
        }
    }

    QList<playOffRobot> AgentPlan[6];
    POMODE planMode;
    int agentSize;
    POInitPos initPos;
    SPlanConfig config;
    QString tags;
};

struct distAndId {
    double dist;
    int id;
};


struct SPositioningArg {

    Vector2D staticPos;
    Vector2D staticAng;
    long rightData;
    long leftData;
    double staticEscapeRadius;
    POffSkills staticSkill;
    int PassToId = -1;
    int PassToState = -1;

    // TODO : Have Reciver Pointer
};

struct SPositioningAgent {

    // TODO : Make positionArg a List of Pointers

    QList<SPositioningArg> positionArg;
    int stateNumber = 0;

    SPositioningArg getArgs(const int& _state = 0) const {
        if ((_state + stateNumber) < positionArg.size()) {
            return positionArg.at(stateNumber + _state);

        } else {

            debug(QString("getArgs : wrong arg %1 < %2").arg(positionArg.size()).arg(_state + stateNumber), D_ERROR);
            qWarning() << QString("getArgs : wrong arg %1 < %2").arg(positionArg.size()).arg(_state + stateNumber);
            SPositioningArg null;
            return null;

        }
    }

    SPositioningArg getAbsArgs(const int& _state = 0) const {
        if (_state < positionArg.size()) {
            return positionArg.at(_state);

       } else {

            debug(QString("getArgs : wrong absarg %1 < %2").arg(positionArg.size()).arg(_state), D_ERROR);
            qWarning() << QString("getArgs : wrong absarg %1 < %2").arg(positionArg.size()).arg(_state);
            SPositioningArg null;
            return null;

        }
    }

};

struct POOwnerReceive {
    int ballOwnerAgent;
    int ballOwnerIndex;
    int receiveAgent;
    int receiveIndex;
    POffSkills skill;
};

struct SBallOwner {
    int id;
    int state;
};

struct kkTimeAndIndex {
    long time;
    int index;
    int agent;
    POffSkills skill;
};



////Play Off Plans
namespace NGameOff {

enum EType {
    None   = 0,
    File   = 1,
    Link   = 2,
    SubDir = 3
};

enum EMode {
    FirstPlay   = 0,
    FastPlay    = 1,
    StaticPlay  = 2,
    DynamicPlay = 3
};

struct SFail {
    bool fail;
    int agentID, roleID, planID, taskID;
    EMode mode;
    roleSkill::ESkill skill;
    int succesRate;
};

struct SCommon {
    int agentSize;
    int currentSize;
    double chance;
    double lastDist;
    POMODE planMode;
    QStringList tags;
    int succesRate = 0; // {},{},{},{},{},{},{}
    QMap<int, int> matchedID;

    void addHistory(const int _story) {
        int tempSucces = _story - succesRate;
        history.append(_story);
        succesRate += tempSucces/history.size();
    }

private:
    QList<int> history;

};

struct SMatching {
    struct SInitPos {
        Vector2D ball;
        QList<Vector2D> agents;
    };
    SInitPos initPos;
    SCommon *common;
};

struct AgentPoint {
    int id    = -1;
    int state = -1;
};

struct SExecution {

    QList< QList<playOffRobot> > AgentPlan;
    SCommon *common;
    int symmetry     =  1;
    int theLastAgent = -1;
    int theLastState = -1;
    AgentPoint passer;
    AgentPoint reciver;
};

struct SGUI {

    QString name;
    QString planFile;
    QString package;
    bool active = true;
    bool master = false;
    unsigned int index[3]; // {package_index, file_index, plan_index}
    SCommon *common;
};


struct SPlan {

    SPlan() {
        gui.common       = &common;
        matching.common  = &common;
        execution.common = &common;
    }

    SGUI       gui;
    SCommon    common;
    SMatching  matching;
    SExecution execution;

    //    friend QDebug operator<< (QDebug d, const SPlan plan);
};

}

using namespace NGameOff;

class CPlayOff : public CMasterPlay {

public:
    CPlayOff();
    virtual ~CPlayOff();

    void execute_0();
    void execute_1();
    void execute_2();
    void execute_3();
    void execute_4();
    void execute_5();
    void execute_6();
    void init(QList <int> _agents , QMap<QString , EditData*> *_editData);
    virtual QString whoami() {return "PlayOff";}
    bool firstTime = true;
    bool kickOffFirstTimeFlag = true;
    //GUI

    QList< QList<SPlayOffPlan*> > updatePlans();
    QList< QList<SPlayOffPlan*> > loadSQLs(QList<QString> _directorys);
    void addSQL(QString _directory);
    QList< QList<SPlayOffPlan*> > addSQLs(QStringList _directorys);
    void debugDirs();

    QList<QString> dirList;
    QList< QList< SPlayOffPlan*> > fullPlans;
    void clear();
    void fullClear();
    QString getModeStr(POMODE _mode);

    //////////

    void setMasterPlan(SPlan* _thePlan);
    void analyseShoot();
    void analysePass();

    void setMasterMode(EMode _mode);
    EMode getMasterMode();

    void setInitial(bool _init);
private:
    bool initial = true;

    /////////////*NEW*/////////////
    SPlan* masterPlan = NULL;
    EMode masterMode;
    //////////Dynamic Plan////////////

    SPlayOffPlan* DynamicPlay();

    //////////////////////////////////
    void globalExecute();

    ///////////////////////////////////////////////////////////////
    /////////////////////////MAHI POSITIONING//////////////////////
    ///////////////////////////////////////////////////////////////
    bool isPathClear(Vector2D _pos1, Vector2D _pos2, double _radius, double treshold);

    void posExecute();
    void setAgentSize(int _agentSize);
    bool isPlanEnd();
    bool isKickFaild(int agent);
    bool isPassFaild(int agent);
    bool isReceiveFaild(int agent);
    bool isTaskFaild(int agent);
    bool isTasksDone();

    SPositioningAgent positionAgent[6];

    int agentSize;
    bool ballEnteredKickerFlag;
    bool ballEnteredKickerChipFlag;
    bool passReceivedFlag;
    bool isPassDoneflag;
    bool isFirstTime[6];

    /////////////////////////////////////////////////////////////////
    ////////////////////////////////////////KK SQL & MATCHIN'////////
    /////////////////////////////////////////////////////////////////
    void setPlanDir(QString directory);
    void loadSQL();
    int loadPlan();
    Vector2D convertPos(int _x, int _y, int _symmetry);
    void loadEachPlan(SPlayOffPlan *_plan, QString _name, int _symmetry);
    bool loadSQLtoStruct(QSqlQuery _query,
                         int _rIndex,
                         playOffRobot &temp,
                         int _symmetry);

    bool getMatchedPlan(POMODE _mode, int agentSize, bool _rand = true);
    void getPassTimeline(SPlayOffPlan *tCurrentPlan, QList<POOwnerReceive> &tList);

    QList<SPlayOffPlan*> planListKickOff;
    QList<SPlayOffPlan*> planListDirect;
    QList<SPlayOffPlan*> planListIndirect;
    QList<POOwnerReceive> ownerReceiveList;

    QString directory;
    void resetP();
    QSqlDatabase kkPOPlanSQL;
    QSqlDatabase planSql;
    double radLimit;
    /////////////////////////////////////////////////////////////////////
    /////////////////////////MAHI PLANNER////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void mainPlanner(int tAgentSize);
    void mainExecute();
    void staticExecute();
    void dynamicExecute();
    void fastExecute();
    void firstExecute();
    void kickOffStopModePlay(int tagentSize);
    POMODE getPlayOffMode();
    void assinID();
    int insertActiveAgentsToList();
    void getCostRec(double costArr[][6], int arrSize, QList<kkValue> &valueList, kkValue value, int size, int aId = 0);
    int kkGetIndex(kkValue &value, int cIndex);
    void assignTasks();
    void fillRolesProperties();
    void findPasserIndex();
    void initilizePositions(QList<SPositioningArg> _posArg[]);
    void assignTask(int agentID,POffSkills agentSkill);
    bool chipOrNot(int passerID,int ReceiverID,int ReceiverState);
    bool chipOrNot(const SPositioningArg& _posArg);
    bool kkCheckIntersectWithAgents(Segment2D tSeg);
    Vector2D getGoalTarget(int shoterID,int shoterState);
    Vector2D getGoalTarget(const long& _posArg);
    double getMaxVel(int agentID,int agentState);
    double getMaxVel(const CRolePlayOff* _roleAgent, const SPositioningArg& _posArg);
    Vector2D getMoveTarget(int agentID,int agentState);
    Vector2D getMoveTarget(const SPositioningArg& _posArg);
    void checkEndState();
    bool isTaskDone(int agentID);
    bool isTaskDone(CRolePlayOff*);
    bool isKickDone(CAgent* _agent, int agentID);
    bool isReceiveDone(CAgent* _agent);
    bool isMoveDone(int agentID);
    long timeTillPass();
    long timeTillReceive();
    void findCurrentPassReceiver();
    void terminateReceiverTasks();
    void passManager();
    bool isBallMoved();
    /////////////////////////////////////
    void oneBehindBall();
    void oneLeftOneCentre();
    void oneRightOneCentre();
    void twoSidesOneCentre();
    void twoSideOneCentreOneDef();
    ////////////////////////////////////
    int matchKickOffID(int _agentSize);
    bool isFinalShotDone();

    void kickOffExecute();

    long lastDecideTime;
    Vector2D lastBallPos;
    bool decidePlan;
    int kkAgentsID[6];
    //    QList<positioningArg> positionArg[6];
    int taskDoneCnt;
    int cnt;

    SPlayOffPlan* currentPlan;
    QList<CAgent*> activeAgents;
    QList<kkRobot> agentList;
    CRolePlayOff *roleAgent[6];
    CRolePlayOff *tempAgent;
    CRolePlayOff *newRoleAgent[6];

    Vector2D kickOffPos[6];

    int lastAgentCount;
    bool isBallNearRobot[6];
    bool isBallNearRobotF[6];
    bool isBallIn;
    int firstPasserID;
    bool bugflag;
    /////////////////////////////////////////
    /////////////////////////////////////////
    /////////////////////////////////////////

    bool hasPassInSkills(int _agent, int _index);

    double debugs[10];
    Vector2D draws[10];
    Circle2D circles[10];

    void mahiDebug(int limit);
    void mahiCircle(int limit);
    void mahiVector(int limit);

    bool doPass;

    //////////////End  Plan
    bool isTimeOver();
    bool isBallDirChanged();
    bool isFinalShotDone1();
    bool isPassChiped();

    SFail isAnyTaskFaild();
    bool isAllTasksDone();
    bool isPlanDone();
    bool isPlanFaild();
    bool setTimer;
    long tempStart;
    ////////////////////////////

    bool isKickDone    (CRolePlayOff*);
    bool isOneTouchDone(CRolePlayOff*);
    bool isMoveDone    (const CRolePlayOff*);
    bool isReceiveDone (const CRolePlayOff*);
    //////////////////////////
    //////////NEW ONE/////////
    //////////////////////////
    void newAssignTasks();
    void connectPasserAndReciever();
    void newFillRoleProperties();
    void newPosExecute();
    void newCheckEndState();
    bool newIsPlanEnd();
    void newAssignTask  (CRolePlayOff*, const SPositioningAgent&);
    void assignPass     (CRolePlayOff*, const SPositioningAgent&);
    void assignMove     (CRolePlayOff*, const SPositioningAgent&);
    void assignOneTouch (CRolePlayOff*, const SPositioningAgent&);
    void assignAfterLife(CRolePlayOff*, const SPositioningAgent&);
    void assignKick     (CRolePlayOff*, const SPositioningAgent&, bool _chip);
    void assignReceive  (CRolePlayOff*, const SPositioningAgent&, bool _ignoreAngle);
    int findFirstPasser();
    QPair<int, int> findTheLastShoot(const SExecution& _plan);
    QPair<AgentPoint, AgentPoint> findThePasserandReciver(const SExecution&);
    int findReciver(int _passer, int _state);
    QList<SBallOwner> ownerList;
    bool havePassInPlan;

protected:
    void reset();
};


///////////OverLoading Operators
QDebug operator<< (QDebug d, const NGameOff::SPlan _plan);




#endif // CPLAYOFF_H
