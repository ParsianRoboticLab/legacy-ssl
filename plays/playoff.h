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
    int Data[2];
};

struct playOffRobot {
    Vector2D pos;
    AngleDeg angle;
    double tolerance;
    int targetIndex;
    int targetAgent;
    QList<playOffSkill> skill;
//    POffSkills skill[3];
//    int skillData[3][2];
//    int skillSize;
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


struct positioningArg {
    Vector2D staticPos;
    Vector2D staticAng;
    long staticWait;
    long staticTime;
    double staticEscapeRadius;
    POffSkills staticSkill;
    int PassToId;
    int PassToState;
    int currentIndex;

};

struct positioningAgent {
    QList<positioningArg> positionArg;
    long mahiLastTime;
    int stateNumber;
    bool flag;

    positioningArg getArgs(int _state = 0) {
        return this->positionArg.at(stateNumber + _state);
    }

};

struct POOwnerReceive {
    int ballOwnerAgent;
    int ballOwnerIndex;
    int receiveAgent;
    int receiveIndex;
    POffSkills skill;
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


struct SCommon {
    int agentSize;
    double chance;
    double lastDist;
    POMODE planMode;
    QStringList tags;
    int succesRate;
//    QMap<int, int> matchedID;
};

struct SMatching {
    struct SInitPos {
        Vector2D ball;
        QList<Vector2D> agents;
    };
    SInitPos initPos;
    SCommon *common;
};

struct SExecution {

    SExecution() = default;

//    SExecution(const SExecution &_toCopy) {
//        for(int i = 0;i < 6;i++) {
//            this->AgentPlan[i].clear();
//            for(int j = 0; j < _toCopy.AgentPlan[i].size(); j++) {
//                this->AgentPlan[i].append(_toCopy.AgentPlan[i].at(j));
//            }
//        }
//    }

//    SExecution& operator =(const SExecution &_insert) {
//        for(int i = 0;i < 6;i++) {
//            this->AgentPlan[i].clear();
//            for(int j = 0;j < _insert.AgentPlan[i].size();j++) {
//                this->AgentPlan[i].append(_insert.AgentPlan[i].at(j));
//            }
//        }
//    }

    QList< QList<playOffRobot> > AgentPlan;
    SCommon *common;
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
    ~CPlayOff();

    void execute_0();
    void execute_1();
    void execute_2();
    void execute_3();
    void execute_4();
    void execute_5();
    void execute_6();
    void init(QList <int> _agents , QMap<QString , EditData*> *_editData);
    virtual QString whoami() {return "PlayOff";}
    bool firstTime;
    bool kickOffFirstTimeFlag;
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

    void setMasterPlan(const SPlan* _thePlan);
private:

    /////////////*NEW*/////////////
    const SPlan* masterPlan;

    //////////Dynamic Plan////////////

    SPlayOffPlan* DynamicPlay();

    //////////////////////////////////
    void globalExecute(int agentCnt);

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

    positioningAgent positionAgent[6];

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
    void kickOffStopModePlay(int tagentSize);
    POMODE getPlayOffMode();
    void assinID();
    int insertActiveAgentsToList();
    void getCostRec(double costArr[][6], int arrSize, QList<kkValue> &valueList, kkValue value, int size, int aId = 0);
    int kkGetIndex(kkValue &value, int cIndex);
    void assignTasks();
    void fillRolesProperties();
    void findPasserIndex();
    void initilizePositions(QList<positioningArg> _posArg[]);
    void assignTask(int agentID,POffSkills agentSkill);
    bool chipOrNot(int passerID,int ReceiverID,int ReceiverState);
    bool kkCheckIntersectWithAgents(Segment2D tSeg);
    Vector2D getGoalTarget(int shoterID,int shoterState);
    double getMaxVel(int agentID,int agentState);
    Vector2D getMoveTarget(int agentID,int agentState);
    void checkEndState();
    bool isTaskDone(int agentID);
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

    bool setTimer;
    long tempStart;
    ////////////////////////////




protected:
    void reset();
};


///////////OverLoading Operators
QDebug operator<< (QDebug d, const NGameOff::SPlan _plan);








#endif // CPLAYOFF_H
