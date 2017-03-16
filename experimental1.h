#include<QTime>
#ifndef EXPERIMENTAL1_H
#define EXPERIMENTAL1_H

#include <QElapsedTimer>
CPolynomialFit        *pf;
CPolynomialRegression *pr;

#include "navigation.h"
#include "matrix.h"
#include "position.h"
#include <time.h>
#include "QDebug"

#include "mainapplication.h"
#include "gotopoint.h"
#include "kick.h"

#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <QThread>


using namespace std;

#ifndef GAME_MODE

/*#define ClassProperty(skill,type,name,local,chflag) \
        public: inline type get##name() {return local;} \
        public: inline skill* set##name(type val) {local = val;chflag = true;return this;} \
        protected: type local

enum PSkills {
    None = 0,
    MoveOffensive = 1,
    MoveDefensive = 2,
    PassOffensive = 3,
    PassDefensive = 4,
    KickOffensive = 5,
    KickDefensive = 6,
    ChipOffensive = 7,
    ChipDefensive = 8,
    MarkOffensive = 9,
    MarkDefensive = 10,
    OneTouch = 11,
    CatchBall = 12,
    ReceivePass = 13,
    Shot = 14,
    ChipToGoal = 15
};

enum PEndPolicy {
    Cycle = 1,
    ExactAgent = 2,
    AllAgents = 3,
    ExactDisturb = 4
};

struct kkRobot {
    int id;
    Vector2D pos;
    Vector2D vel;
    Vector2D dir;
};

struct kkAgent {
    int A, B;
    PSkills skill[4];
    int pSize;
};

struct kkAgentPlan {
    int planId;
    int ball;
    PEndPolicy endMode;
    int endPolicy;
    int possession;
    int agentsSize;
    //QString comment;
    kkAgent agents[5];
};

struct kkValue {
    int IDs[5];
    double value;
    int agentSize;
};

struct kk2Vector2D {
    Vector2D vec1;
    Vector2D vec2;
};

struct kk2Segment2D {
    Segment2D seg1;
    Segment2D seg2;
};


enum kkSkill {
    SkillGotopoint = 0,
    SkillGotopointAvoid = 1,
    SkillKick = 2,
    SkillReceivePass = 3,
    SkillOneTouch = 4
};

//class kkFindBestPos : QThread {
//    Q_OBJECT
//public:
//    void run() {
//         QString result;
//         result = "Hey you";
//         emit resultReady(result);
//    }

//signals:
//    void resultReady(const QString &s);
//};

class kkAgentTask {
    protected:
    CAgent *assignedAgent;
    public:
    kkAgentTask(kkSkill tSkill = SkillGotopoint);
    bool updated;

    void initTask();
    void execute();

    void setAgent(CAgent *_agent){
        assignedAgent = _agent;
    }

    CAgent* getAgent(){
        return assignedAgent;
    }
    ClassProperty(kkAgentTask, kkSkill, SelectedSkill, selectedSkill, updated);

    ClassProperty(kkAgentTask, Vector2D, Target, target, updated);
    ClassProperty(kkAgentTask, Vector2D, TargetDir, targetDir, updated);
    ClassProperty(kkAgentTask, Vector2D, TargetVel, targetVel, updated);

    ClassProperty(kkAgentTask, int, OppRelax, oppRelax, updated);
    ClassProperty(kkAgentTask, int, OurRelax, ourRelax, updated);
    ClassProperty(kkAgentTask, bool, AvoidPenaltyArea, avoidPenaltyArea, updated);
    ClassProperty(kkAgentTask, bool, AvoidCenterCircle, avoidCenterCircle, updated);

    ClassProperty(kkAgentTask, double, Tolerance, tolerance, updated);
    ClassProperty(kkAgentTask, bool, Chip, chip, updated);
    ClassProperty(kkAgentTask, bool, Spin, spin, updated);
    ClassProperty(kkAgentTask, int, KickSpeed, kickSpeed, updated);
    ClassProperty(kkAgentTask, bool, IsGotoPointAvoid, isGotoPointAvoid, updated);

    ClassProperty(kkAgentTask, bool, Slow, slow, updated);
    ClassProperty(kkAgentTask, float, ReceiveRadius, receiveRadius, updated);

    ClassProperty(kkAgentTask, Vector2D, WaitPos, waitPos, updated);

    CskillNewGotoPoint *gotoPointSkill;
    CSkillGotoPointAvoid *gotoPointAvoidSkill;
    CSkillNewKick *kickSkill;
    CSkillReceivePass *receivePassSkill;
    CSkillKickOneTouch *oneTouchSkill;
};

kkAgentTask::kkAgentTask(kkSkill tSkill)
{
    selectedSkill = tSkill;

    gotoPointSkill = new CskillNewGotoPoint(NULL);
    gotoPointAvoidSkill = new CSkillGotoPointAvoid(NULL);
    kickSkill = new CSkillNewKick(NULL);
    receivePassSkill =  new CSkillReceivePass(NULL);
    oneTouchSkill = new CSkillKickOneTouch(NULL);
}

void kkAgentTask::initTask()
{
    updated = false;
    switch(selectedSkill)
    {
        case SkillGotopoint:
            gotoPointSkill->setFinalPos(target);
            gotoPointSkill->setFinalDir(targetDir);
            gotoPointSkill->setFinalVel(targetVel);
            gotoPointSkill->setAgent(assignedAgent);
        break;
        case SkillGotopointAvoid:
            gotoPointAvoidSkill->setFinalPos(target);
            gotoPointAvoidSkill->setFinalDir(targetDir);
            gotoPointAvoidSkill->setFinalVel(targetVel);
            gotoPointAvoidSkill->oppRelax(oppRelax);
            gotoPointAvoidSkill->ourRelax(ourRelax);
            gotoPointAvoidSkill->setAvoidCenterCircle(avoidCenterCircle);
            gotoPointAvoidSkill->setAvoidPenaltyArea(avoidPenaltyArea);
            gotoPointAvoidSkill->setAgent(assignedAgent);
            gotoPointAvoidSkill->setMaxVelocity(3.0);
        break;
        case SkillKick:
            kickSkill->setTarget(target);
            kickSkill->setTolerance(tolerance);
            kickSkill->setChip(chip);
            kickSkill->setSpin(spin);
            kickSkill->setKickSpeed(kickSpeed);
            kickSkill->setAvoidOppPenaltyArea(avoidPenaltyArea);
            kickSkill->setAvoidOurPenaltyArea(avoidPenaltyArea);
            kickSkill->setIsGotoPointAvoid(isGotoPointAvoid);
            kickSkill->setAgent(assignedAgent);
        break;
        case SkillReceivePass:
            receivePassSkill->setTarget(waitPos);
            receivePassSkill->setSlow(slow);
            receivePassSkill->setReceiveRadius(receiveRadius);
            receivePassSkill->setAvoidOppPenaltyArea(avoidPenaltyArea);
            receivePassSkill->setAvoidOurPenaltyArea(avoidPenaltyArea);
            receivePassSkill->setAgent(assignedAgent);
        break;
        case SkillOneTouch:
            oneTouchSkill->setWaitPos(waitPos);
            oneTouchSkill->setTarget(target);
            oneTouchSkill->setKickSpeed(kickSpeed);
            oneTouchSkill->setChip(chip);
            oneTouchSkill->setAvoidPenaltyArea(avoidPenaltyArea);
            oneTouchSkill->setAgent(assignedAgent);
        break;
    }
}

void kkAgentTask::execute()
{
    if(updated)
        initTask();
    switch(selectedSkill)
    {
        default:
        case SkillGotopoint:
            gotoPointSkill->execute();
        break;
        case SkillGotopointAvoid:
            gotoPointAvoidSkill->execute();
        break;
        case SkillKick:
            kickSkill->execute();
        break;
        case SkillReceivePass:
            receivePassSkill->execute();
        break;
        case SkillOneTouch:
            oneTouchSkill->execute();
        break;
    }
}

class kkPlayOn {
    private:
    CSoccer *soccer;
    public:
    Rect2D kkARegion[10];
    Rect2D kkBRegion[6];
    kkPlayOn();


    void kkSetSoccer(CSoccer *_soccer);


    QSqlDatabase kkPlanSQL;

    int kkGetIntByEnum(PEndPolicy tEnum);
    int kkGetIntByEnum(PSkills tEnum);
    PEndPolicy kkGetPolicyByIndex(int index);
    PSkills kkGetPSkillByIndex(int index);

    void kkDrawRect();
    int kkGetArea(Rect2D *rect, int size, Vector2D point);
    bool kkIsInRect(Rect2D rect, Vector2D point, double offset = 0.0);

    void kkInsertToList(QList<kkAgentPlan> **list, QSqlQuery query, int agentSize);
    void kkLoadPlan(QList<kkAgentPlan> **list, int agentSize);

    QList<kkAgentPlan> kkFillQList(QList<kkAgentPlan> *list);
    void kkGetBestPlan(int planBallPos, int agentSize = 1);

    bool kkIsPlanMatch(QList<kkRobot> aList, kkAgentPlan plan);

    QList<kkAgentPlan> *kk1AgentPlan[6];
    QList<kkAgentPlan> *kk2AgentPlan[6];
    QList<kkAgentPlan> *kk3AgentPlan[6];
    QList<kkAgentPlan> *kk4AgentPlan[6];
    QList<kkAgentPlan> *kk5AgentPlan[6];

    QList<kkRobot> kkAgentList;
    int kkInsertActiveAgentsToList();

    kkAgentPlan kkCurrentPlan;
    kkAgentPlan kkNextPlan;

    int kkAgentsId[5];
    void kkAssignId();

    int kkGetIndex(kkValue &value, int cIndex);
    void kkGetCostRec(double costArr[][5], int arrSize, QList<kkValue> &valueList, kkValue value, int size, int aId = 0);

    Vector2D kkBestMovePos;
    QList<Circle2D> kkMoveObstacle;
    int kkAgentToMove;
    bool kkCheckMovePos(double x, double y);
    void kkAmirSearch(double x, double y, short id, double step);
    void kkAmirSearch(double x, double y, short id, Polygon2D &tPoly, double step);
    void kkInsertObstacleToList(int rectIndex);
    Vector2D kkFindBestPosForMove(int fromAgent, int regionId, bool offensive);

    QList<Circle2D> kkPassObstacle;
    bool kkHasOverlap(Segment2D seg1, Segment2D seg2);
    kk2Segment2D kkMergeTwoSegments(kk2Segment2D seg1, kk2Segment2D seg2);
    void kkMergeSegments(QList<kk2Segment2D> &segList);
    void kkSortSegmentsByAngle(QList<Segment2D> &segList, Vector2D tBallPos);
    void kkInvertSegments(QList<kk2Segment2D> &segList, Segment2D tSeg, QList<kk2Vector2D> &vecList);
    kk2Segment2D kkBlockedByRobots(Circle2D tRobot, Vector2D tOrigin, Line2D tLine);
    bool kkFindBestRegionsToPass(int regionIndex, double reachTime, Polygon2D &tPoly, int tException = -1);
    void kkInsertPassObstacleToList(double reachTime, Polygon2D mPoly, Segment2D lLine, Segment2D rLine, int tException = -1);
    void kkSortObstacleByAngle(Vector2D tBallPos);
    Vector2D kkFindBestPosForPass(int fromAgent, int regionId, bool offensive, double reachTime, int tException = -1);

    bool kkCheckIntersectWithAgents(Segment2D tSeg);
    bool kkIsPathToOppGoaliClear();

    bool kkIsPathFromAToBClear(Vector2D tA, Vector2D tB, double tTolerance = 0.20);

    void kkAssignTasks();

    kkAgentTask *kkAgentTasks[5];

    bool kkHasReceiveOrOneTouch(int tAgent);
    void kkAssignTask(int tAgent, PSkills tSkill, int tA, int tB);
    Vector2D kkPointToPass;
    int kkPassReceiverAgent;

    //kkFindBestPos *myThread;
    void kkExecute();

};

void kkPlayOn::kkSetSoccer(CSoccer *_soccer)
{
    soccer = _soccer;
}

PEndPolicy kkPlayOn::kkGetPolicyByIndex(int index)
{
    switch(index)
    {
        default:
        case 1:
            return Cycle;
        break;
        case 2:
            return ExactAgent;
        break;
        case 3:
            return AllAgents;
        break;
        case 4:
            return ExactDisturb;
        break;
    }
}

int kkPlayOn::kkGetIntByEnum(PEndPolicy tEnum)
{
    switch (tEnum)
    {
        case Cycle:
            return 1;
        case ExactAgent:
            return 2;
        case AllAgents:
            return 3;
        case ExactDisturb:
            return 4;
    }
}

PSkills kkPlayOn::kkGetPSkillByIndex(int index)
{
    switch (index)
    {
        case 0:
            return None;
        case 1:
            return MoveOffensive;
        case 2:
            return MoveDefensive;
        case 3:
            return PassOffensive;
        case 4:
            return PassDefensive;
        case 5:
            return KickOffensive;
        case 6:
            return KickDefensive;
        case 7:
            return ChipOffensive;
        case 8:
            return ChipDefensive;
        case 9:
            return MarkOffensive;
        case 10:
            return MarkDefensive;
        case 11:
            return OneTouch;
        case 12:
            return CatchBall;
        case 13:
            return ReceivePass;
        case 14:
            return Shot;
        case 15:
            return ChipToGoal;
    }
}

int kkPlayOn::kkGetIntByEnum(PSkills tEnum)
{
    switch (tEnum)
    {
        case None:
            return 0;
        case MoveOffensive:
            return 1;
        case MoveDefensive:
            return 2;
        case PassOffensive:
            return 3;
        case PassDefensive:
            return 4;
        case KickOffensive:
            return 5;
        case KickDefensive:
            return 6;
        case ChipOffensive:
            return 7;
        case ChipDefensive:
            return 8;
        case MarkOffensive:
            return 9;
        case MarkDefensive:
            return 10;
        case OneTouch:
            return 11;
        case CatchBall:
            return 12;
        case ReceivePass:
            return 13;
        case Shot:
            return 14;
        case ChipToGoal:
            return 15;
    }
}

kkPlayOn::kkPlayOn() {
    kkARegion[0].assign(-_FIELD_WIDTH/2, _FIELD_HEIGHT/2, _FIELD_WIDTH/2-0.5, _FIELD_HEIGHT);
    kkARegion[1].assign(-0.5, _FIELD_HEIGHT/2, 1, _FIELD_HEIGHT/2 - (_PENALTY_WIDTH)/2);
    kkARegion[2].assign(0.5, _FIELD_HEIGHT/2, _FIELD_WIDTH/2 - (0.5 + _GOAL_RAD), _FIELD_HEIGHT/2 - (_PENALTY_WIDTH)/2);
    kkARegion[3].assign(_FIELD_WIDTH/2 - _GOAL_RAD, _FIELD_HEIGHT/2, _GOAL_RAD, _FIELD_HEIGHT/2 - (_PENALTY_WIDTH)/2);
    kkARegion[4].assign(-0.5, _PENALTY_WIDTH/2, 1, _PENALTY_WIDTH);
    kkARegion[5].assign(0.5, _PENALTY_WIDTH/2, _FIELD_WIDTH/2 - (0.5 + _GOAL_RAD), _PENALTY_WIDTH);
    kkARegion[6].assign(_FIELD_WIDTH/2 - _GOAL_RAD, _PENALTY_WIDTH/2, _GOAL_RAD, _PENALTY_WIDTH);
    kkARegion[7].assign(-0.5, -_PENALTY_WIDTH/2, 1, _FIELD_HEIGHT/2 - (_PENALTY_WIDTH)/2);
    kkARegion[8].assign(0.5, -_PENALTY_WIDTH/2, _FIELD_WIDTH/2 - (0.5 + _GOAL_RAD), _FIELD_HEIGHT/2 - (_PENALTY_WIDTH)/2);
    kkARegion[9].assign(_FIELD_WIDTH/2 - _GOAL_RAD, -_PENALTY_WIDTH/2, _GOAL_RAD, _FIELD_HEIGHT/2 - (_PENALTY_WIDTH)/2);

    kkBRegion[0].assign(-_FIELD_WIDTH/2, _FIELD_HEIGHT/2, _FIELD_WIDTH/2-0.5, _FIELD_HEIGHT);
    kkBRegion[1].assign(-0.5, _FIELD_HEIGHT/2, _FIELD_WIDTH/2 + 0.5, _FIELD_HEIGHT/2 - (_PENALTY_WIDTH)/2);
    kkBRegion[2].assign(-0.5, _PENALTY_WIDTH/2, 1, _PENALTY_WIDTH);
    kkBRegion[3].assign(0.5, _PENALTY_WIDTH/2, _FIELD_WIDTH/2 - (0.5 + _GOAL_RAD), _PENALTY_WIDTH);
    kkBRegion[4].assign(_FIELD_WIDTH/2 - _GOAL_RAD, _PENALTY_WIDTH/2, _GOAL_RAD, _PENALTY_WIDTH);
    kkBRegion[5].assign(-0.5, -_PENALTY_WIDTH/2, _FIELD_WIDTH/2 + 0.5, _FIELD_HEIGHT/2 - (_PENALTY_WIDTH)/2);

    QString tempDir = QString::fromStdString(policy()->KKPlayOn_KKPlanSQL());

    kkPlanSQL = QSqlDatabase::addDatabase("QSQLITE");
    kkPlanSQL.setDatabaseName(tempDir);
    if (!kkPlanSQL.open()) {
        QMessageBox::critical(0, "Cannot open database",
        "Unable to establish a database connection.\n"\
              "This example needs SQLite support. Please read "\
              "the Qt SQL driver documentation for information how "\
              "to build it.\n\n"\
              "Click Cancel to exit.", QMessageBox::Cancel);
    }

    for(int i = 0; i < 6; i++)
    {
        kk1AgentPlan[i] = new QList<kkAgentPlan>();
        kk2AgentPlan[i] = new QList<kkAgentPlan>();
        kk3AgentPlan[i] = new QList<kkAgentPlan>();
        kk4AgentPlan[i] = new QList<kkAgentPlan>();
        kk5AgentPlan[i] = new QList<kkAgentPlan>();
    }

    kkLoadPlan(kk1AgentPlan, 1);
    kkLoadPlan(kk2AgentPlan, 2);
    kkLoadPlan(kk3AgentPlan, 3);
    kkLoadPlan(kk4AgentPlan, 4);
    kkLoadPlan(kk5AgentPlan, 5);

    for(int i = 0; i < 5; i++)
        kkAgentTasks[i] = new kkAgentTask();

    //myThread = new kkFindBestPos();

}

void kkPlayOn::kkDrawRect()
{
    for(int i =0; i < 10; i++)
        draw(kkARegion[i], QColor(255, 0, 0));

    for(int i = 0; i < 6; i++)
        draw(kkBRegion[i], QColor(255, 255, 0, 128));

//    for(int i = 0; i < 6; i++)
//    {
//        for(int j = 0; j < _1AgentPlan[i]->length(); i++)
//          {
//              debug(QString::number(_1AgentPlan[i]->at(j).ball), 0);
//              debug("Size"+QString::number(_1AgentPlan[i]->length()), 0);
//          }
//    }
}

void kkPlayOn::kkInsertToList(QList<kkAgentPlan> **list, QSqlQuery query, int agentSize)
{
    bool errorCheck = true;
    while(query.next())
    {
        errorCheck = false;
        kkAgentPlan tempPlan;
        tempPlan.planId = query.value(0).toInt();
        tempPlan.ball = query.value(1).toInt();
        tempPlan.endMode = kkGetPolicyByIndex(query.value(2).toInt());
        tempPlan.endPolicy = query.value(3).toInt();
        tempPlan.possession = query.value(4).toInt();
        tempPlan.agentsSize = query.value(5).toInt();
        //tempPlan.comment = getCommentForPlanner(query.value(6).toString());
        for(int i = 0; i < agentSize; i++)
        {
            tempPlan.agents[i].A = query.value(7 + i*6).toInt();
            tempPlan.agents[i].B = query.value(8 + i*6).toInt();
            for(int j = 0; j < 4; j++)
            {
                tempPlan.agents[i].skill[j] = kkGetPSkillByIndex(query.value(9 + i*6 + j).toInt());
            }
        }
        for(int i = agentSize; i < 5; i++)
        {
            tempPlan.agents[i].A = -1;
            tempPlan.agents[i].B = -1;
            for(int j = 0; j < 4; j++)
            {
                tempPlan.agents[i].skill[j] = None;
            }
        }
        switch(tempPlan.ball)
        {
            default:
            case 1:
                list[0]->append(tempPlan);
            break;
            case 2:
                list[1]->append(tempPlan);
            break;
            case 3:
                list[2]->append(tempPlan);
            break;
            case 4:
                list[3]->append(tempPlan);
            break;
            case 5:
                list[4]->append(tempPlan);
            break;
        }
    }
    if(errorCheck)
    {
        qDebug()<<kkPlanSQL.databaseName();
        qDebug()<< "SqLite error:" << query.lastError().text() << ", SqLite error code:" << query.lastError().number();
    }
}

void kkPlayOn::kkLoadPlan(QList<kkAgentPlan> **list, int agentSize)
{
    QSqlQuery query;
    switch(agentSize)
    {
        default:
        case 1:
            query.exec("SELECT * FROM plan1agent ORDER BY id ASC");
            kkInsertToList(list, query, 1);
        break;
        case 2:
            query.exec("SELECT * FROM plan2agent ORDER BY id ASC");
            kkInsertToList(list, query, 2);
        break;
        case 3:
            query.exec("SELECT * FROM plan3agent ORDER BY id ASC");
            kkInsertToList(list, query, 3);
        break;
        case 4:
            query.exec("SELECT * FROM plan4agent ORDER BY id ASC");
            kkInsertToList(list, query, 4);
        break;
        case 5:
            query.exec("SELECT * FROM plan5agent ORDER BY id ASC");
            kkInsertToList(list, query, 5);
        break;

    }
}

int kkPlayOn::kkGetArea(Rect2D *rect, int size, Vector2D point)
{
    for(int i = 0; i < size; i++)
    {
        if(rect[i].contains(point))
            return i + 1;
    }
    return -1;
}

bool kkPlayOn::kkIsInRect(Rect2D rect, Vector2D point, double offset)
{
    Rect2D tempRect;
    tempRect.assign(rect.left() - offset, rect.top() + offset, rect.size().length() + offset*2, rect.size().width() + offset*2);
    return tempRect.contains(point);
}

QList<kkAgentPlan> kkPlayOn::kkFillQList(QList<kkAgentPlan> *list)
{
    QList<kkAgentPlan> tempList;
    for(int i = 0; i < list->length(); i++)
    {
        tempList.append(list->at(i));
    }
    return tempList;
}

bool kkPlayOn::kkIsPlanMatch(QList<kkRobot> aList, kkAgentPlan plan)
{
    int tempRegion;
    int regionCnt[10], agentCnt[10];
    for(int i = 0; i < 10; i++)
    {
        regionCnt[i] = 0;
        agentCnt[i] = 0;
    }

    for(int i = 0; i < plan.agentsSize; i++)
    {
        tempRegion = plan.agents[i].A - 1;
        if(tempRegion < 0) tempRegion = 0;
        if(tempRegion > 9) tempRegion = 9;
        regionCnt[tempRegion]++;
    }

    for(int i = 0; i < aList.length(); i++)
    {
        if(aList.at(i).id != -1)
        {
            tempRegion = kkGetArea(kkARegion, 10, aList.at(i).pos) - 1;
            if(tempRegion >= 0 && tempRegion <= 9)
                agentCnt[tempRegion]++;
        }
    }

    for(int i = 0; i < 10; i++)
    {
        if(agentCnt[i] != regionCnt[i])
            return false;
    }

    return true;
}

int kkPlayOn::kkInsertActiveAgentsToList()
{
    int activeAgentsCount = wm->our.activeAgentsCount();
    kkAgentList.clear();
    kkRobot tempRobot;
    for(int i = 0; i < activeAgentsCount; i++)
    {
         tempRobot.pos = wm->our.active(i)->pos;
         tempRobot.vel = wm->our.active(i)->vel;
         tempRobot.dir = wm->our.active(i)->dir;
         tempRobot.id = wm->our.active(i)->id;
         kkAgentList.append(tempRobot);
    }
    for(int i = activeAgentsCount; i < 5; i++)
    {
         tempRobot.pos = Vector2D(0,0);
         tempRobot.vel = Vector2D(0,0);
         tempRobot.dir = Vector2D(0,0);
         tempRobot.id = -1;
         kkAgentList.append(tempRobot);
    }
    return activeAgentsCount;
}

void kkPlayOn::kkGetBestPlan(int planBallPos, int agentSize)
{
    QList<kkAgentPlan> tempPlan;
    planBallPos--;
    if(planBallPos < 0) planBallPos = 0;
    if(planBallPos > 5) planBallPos = 0;
    switch(agentSize)
    {
        default:
        case 1:
            tempPlan = kkFillQList(kk1AgentPlan[planBallPos]);
        break;
        case 2:
            tempPlan = kkFillQList(kk2AgentPlan[planBallPos]);
        break;
        case 3:
            tempPlan = kkFillQList(kk3AgentPlan[planBallPos]);
        break;
        case 4:
            tempPlan = kkFillQList(kk4AgentPlan[planBallPos]);
        break;
        case 5:
            tempPlan = kkFillQList(kk5AgentPlan[planBallPos]);
        break;
    }
    QList<int> selectedPlans;

    kkInsertActiveAgentsToList();
    for(int i = 0; i < tempPlan.length(); i++)
    {
        if(kkIsPlanMatch(kkAgentList, tempPlan.at(i)))
            selectedPlans.append(i);
    }

    if(selectedPlans.length() == 1)
    {
        kkCurrentPlan = tempPlan.at(selectedPlans.at(0));
    }

    debug(QString("%1, %2, %3").arg(selectedPlans.length()).arg(agentSize).arg(tempPlan.length()), 0);

    kkAssignId();
}

void kkPlayOn::kkAssignId()
{
    QList<PSkills> twoPointSkills;
    twoPointSkills.append(MoveOffensive);
    twoPointSkills.append(MoveOffensive);
    twoPointSkills.append(PassOffensive);
    twoPointSkills.append(PassDefensive);
    twoPointSkills.append(KickOffensive);
    twoPointSkills.append(KickDefensive);
    twoPointSkills.append(ChipOffensive);
    twoPointSkills.append(ChipDefensive);
    twoPointSkills.append(MarkOffensive);
    twoPointSkills.append(MarkDefensive);
    QList<int> twoPointSkill;
    QList<int> onePointSkill;
    for(int i = 0; i < kkCurrentPlan.agentsSize; i++)
    {
        if(twoPointSkills.contains(kkCurrentPlan.agents[i].skill[0]))
            twoPointSkill.append(i);
        else
            onePointSkill.append(i);
    }

    double distPoint[5][5];

    for(int i = 0; i < 5; i++)
        for(int j = 0; j < 5; j++)
            distPoint[i][j] = 10000;

    for(int i = 0; i < kkCurrentPlan.agentsSize; i++)
    {
        for(int j = 0; j < kkAgentList.length(); j++)
        {
            distPoint[i][j] = kkARegion[kkCurrentPlan.agents[i].A - 1].center().dist(kkAgentList.at(j).pos);
        }
    }

    kkValue tempValue;
    for(int i = 0; i < 5; i++)
        tempValue.IDs[i] = -1;
    tempValue.value = 0;
    tempValue.agentSize = 3;

    QList<kkValue> tempListValue;
    kkGetCostRec(distPoint, 3, tempListValue, tempValue, 3, 0);

//    for(int i = 0; i < tempListValue.length(); i++)
//    {
//        debug(QString(" %1, %2, %3 val:%4").arg(tempListValue.at(i).IDs[0])
//                                            .arg(tempListValue.at(i).IDs[1])
//                                            .arg(tempListValue.at(i).IDs[2])
//                                            .arg(tempListValue.at(i).value), 0);
//    }

    double minValue = 10000;
    int bestMatchIndex = 0;
    for(int i = 0; i < tempListValue.length(); i++)
    {
        if(tempListValue.at(i).value < minValue)
        {
            minValue = tempListValue.at(i).value;
            bestMatchIndex = i;
        }
    }

    for(int i = 0; i < kkCurrentPlan.agentsSize; i++)
        kkAgentsId[i] = kkAgentList.at(tempListValue.at(bestMatchIndex).IDs[i]).id;
}

int kkPlayOn::kkGetIndex(kkValue &value, int cIndex)
{
    int cnt = 0;
    for(int i = 0; i < value.agentSize; i++)
    {
        if(value.IDs[i] == -1)
        {
            if(cnt == cIndex)
                return i;
            cnt++;
        }
    }
}

void kkPlayOn::kkGetCostRec(double costArr[][5], int arrSize, QList<kkValue> &valueList, kkValue value, int size, int aId)
{
    if(size <= 1)
    {
        int tIndex = kkGetIndex(value, 0);
        value.IDs[tIndex] = aId;
        value.value += costArr[tIndex][aId];
        valueList.append(value);
        return;
    }
    for(int i = 0; i < size; i++)
    {
        kkValue tValue = value;
        int tIndex = kkGetIndex(tValue, i);
        tValue.IDs[tIndex] = aId;
        tValue.value += costArr[tIndex][aId];
        //debug(QString::number(tValue.IDs[0])+QString::number(tValue.IDs[1])+QString::number(tValue.IDs[2]),0,QColor(Qt::blue));
        kkGetCostRec(costArr, arrSize, valueList, tValue, size - 1, aId + 1);
    }
}

bool kkPlayOn::kkCheckMovePos(double x, double y)
{
     Circle2D tempCircle(Vector2D(x, y), 0.15);
     Vector2D sol1, sol2;
     for(int i = 0; i < kkMoveObstacle.length(); i++)
     {
          if(tempCircle.intersection(kkMoveObstacle.at(i), &sol1, &sol2) > 0 || tempCircle.contains(kkMoveObstacle.at(i).center()))
              return false;
     }
     kkBestMovePos.assign(x, y);
     return true;
}

void kkPlayOn::kkAmirSearch(double x, double y, short id, double step)
{
     double tempDist[4];
     tempDist[0] = Vector2D(x, y).dist(kkARegion[id].topLeft());
     tempDist[1] = Vector2D(x, y).dist(kkARegion[id].topRight());
     tempDist[2] = Vector2D(x, y).dist(kkARegion[id].bottomRight());
     tempDist[3] = Vector2D(x, y).dist(kkARegion[id].bottomLeft());

     double maxRadius = 0;
     for(int i = 0; i < 4; i ++)
     {
          if(tempDist[i] > maxRadius)
              maxRadius = tempDist[i];
     }

     double xx, yy;

     for(double r = 0; r <= maxRadius; r += step)
     {
          for(double theta = 0; theta <= 2*_PI; theta += step/xx)
          {
                xx = r*cos(theta) + x;
                yy = r*sin(theta) + y;
                if(kkARegion[id].contains(Vector2D(xx, yy)))
                {
                    if(kkCheckMovePos(xx, yy))
                        return;
                }
          }
     }
}

void kkPlayOn::kkAmirSearch(double x, double y, short id, Polygon2D &tPoly, double step)
{
    double tempDist[4];
    tempDist[0] = Vector2D(x, y).dist(kkARegion[id].topLeft());
    tempDist[1] = Vector2D(x, y).dist(kkARegion[id].topRight());
    tempDist[2] = Vector2D(x, y).dist(kkARegion[id].bottomRight());
    tempDist[3] = Vector2D(x, y).dist(kkARegion[id].bottomLeft());

    double maxRadius = 0;
    for(int i = 0; i < 4; i ++)
    {
         if(tempDist[i] > maxRadius)
             maxRadius = tempDist[i];
    }

    double xx, yy;

    for(double r = 0; r <= maxRadius; r += step)
    {
         for(double theta = 0; theta <= 2*_PI; theta += step/xx)
         {
               xx = r*cos(theta) + x;
               yy = r*sin(theta) + y;
               if(kkARegion[id].contains(Vector2D(xx, yy)) && tPoly.contains(Vector2D(xx, yy)))
               {
                   if(kkCheckMovePos(xx, yy))
                       return;
               }
         }
    }
}

void kkPlayOn::kkInsertObstacleToList(int rectIndex)
{
    kkMoveObstacle.clear();
    int cnt = 0;
    for(int i = 0; i < wm->our.activeAgentsCount(); i++)
    {
        if(kkARegion[rectIndex].contains(wm->our.active(i)->pos))
            kkMoveObstacle.append(Circle2D(wm->our.active(i)->pos, 0.12));
        cnt++;
    }
    for(int i = 0; i < wm->opp.activeAgentsCount(); i++)
    {
        if(kkARegion[rectIndex].contains(wm->opp.active(i)->pos))
            kkMoveObstacle.append(Circle2D(wm->opp.active(i)->pos, 0.12));
        cnt++;
    }
    //debug(QString::number(cnt), 0);
}

Vector2D kkPlayOn::kkFindBestPosForMove(int fromAgent, int regionId, bool offensive)
{
    regionId--;
    if(regionId < 0) regionId = 0;
    if(regionId > 9) regionId = 9;
    kkInsertObstacleToList(regionId);
    Rect2D offRect, defRect;
    offRect.assign(kkARegion[regionId].center().x, kkARegion[regionId].top(), kkARegion[regionId].size().length()/2, kkARegion[regionId].size().width());
    defRect.assign(kkARegion[regionId].left(), kkARegion[regionId].top(), kkARegion[regionId].size().length()/2, kkARegion[regionId].size().width());
    if(offensive)
    {
        kkBestMovePos = Vector2D(offRect.center());
        kkAmirSearch(kkBestMovePos.x, kkBestMovePos.y, regionId, 0.08);
    }
    else
    {
        kkBestMovePos = Vector2D(defRect.center());
        kkAmirSearch(kkBestMovePos.x, kkBestMovePos.y, regionId, 0.08);
    }
    return kkBestMovePos;
}

bool kkPlayOn::kkFindBestRegionsToPass(int regionIndex, double reachTime, Polygon2D &tPoly, int tException)
{
    Vector2D tempBallPos = wm->ball->pos;
    Vector2D rectVertice[4];
    rectVertice[0] = kkARegion[regionIndex].topLeft();
    rectVertice[1] = kkARegion[regionIndex].topRight();
    rectVertice[2] = kkARegion[regionIndex].bottomRight();
    rectVertice[3] = kkARegion[regionIndex].bottomLeft();

    double verticeAngles[4][4];
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            verticeAngles[i][j] = 0.0;

    for(int i = 0; i < 4; i++)
    {
        for(int j = i; j < 4; j++)
        {
            verticeAngles[i][j] = Vector2D::angleOf(rectVertice[i], tempBallPos, rectVertice[j]).abs();
        }
    }

    double tempMaxAngle = 0.0;
    int aPoint = 0, bPoint = 0, farthestPoint;
    for(int i = 0; i < 4; i ++)
    {
        for(int j = i; j < 4; j++)
        {
            if(verticeAngles[i][j] > tempMaxAngle)
            {
                  tempMaxAngle = verticeAngles[i][j];
                  aPoint = i;
                  bPoint = j;
            }
        }
    }

    double maxLegth = 0;
    for(int i = 0; i < 4; i++)
    {
        if(tempBallPos.dist(rectVertice[i]) > maxLegth)
        {
            if(aPoint == i || bPoint == i)
                continue;
            maxLegth = tempBallPos.dist(rectVertice[i]);
            farthestPoint = i;
        }
    }

    //Segment2D widestView(rectVertice[aPoint], rectVertice[bPoint]);
    Line2D tempLine(rectVertice[farthestPoint], (rectVertice[aPoint] - rectVertice[bPoint]).dir());
    Line2D tempRLine(tempBallPos, rectVertice[aPoint]);
    Line2D tempLLine(tempBallPos, rectVertice[bPoint]);
    Vector2D tempRPoint, tempLPoint;
    tempRPoint = tempLine.intersection(tempRLine);
    tempLPoint = tempLine.intersection(tempLLine);

    Polygon2D tempPoly;
    tempPoly.addVertex(tempLPoint);
    tempPoly.addVertex(tempRPoint);
    tempPoly.addVertex(tempBallPos);
    tempPoly.addVertex(tempLPoint);

    kkInsertPassObstacleToList(reachTime, tempPoly, Segment2D(tempBallPos, tempLPoint), Segment2D(tempBallPos, tempRPoint), tException);
    //sortObstacleByAngle(tempBallPos);

    QList<kk2Segment2D> blockedSegs;
    for(int i = 0; i < kkPassObstacle.length(); i++)
        blockedSegs.append(kkBlockedByRobots(kkPassObstacle.at(i), tempBallPos, tempLine));

//   for(int i = 0; i < blockedSegs.length(); i++)
//        draw(blockedSegs.at(i).seg1, QColor(Qt::cyan));
    kkMergeSegments(blockedSegs);
    QList<kk2Vector2D> vectorList;
    kkInvertSegments(blockedSegs, Segment2D(tempRPoint, tempLPoint), vectorList);

    tPoly.addVertex(tempBallPos);
    bool firstDraw = true;
    for(int i = 0; i < vectorList.length()/2; i++)
    {
        if(!firstDraw)
        {
            tPoly.addVertex(vectorList.at(i*2 - 1).vec2);
            tPoly.addVertex(vectorList.at(i*2).vec2);
        }
        tPoly.addVertex(vectorList.at(i*2).vec1);
        tPoly.addVertex(vectorList.at(i*2 + 1).vec1);
        firstDraw = false;
    }
    tPoly.addVertex(tempBallPos);
    draw(tPoly, QColor(255, 0, 0, 32), true);
    return ~firstDraw;
}

bool kkPlayOn::kkHasOverlap(Segment2D seg1, Segment2D seg2)
{
    if(seg1.contains(seg2.a()) ||
       seg1.contains(seg2.b()) ||
       seg2.contains(seg1.a()) ||
       seg2.contains(seg1.b()))
    return true;
    return false;
}

kk2Segment2D kkPlayOn::kkMergeTwoSegments(kk2Segment2D seg1, kk2Segment2D seg2)
{
    kk2Segment2D temp2Seg;
    Vector2D vertice[4];
    vertice[0] = seg1.seg1.a();
    vertice[1] = seg1.seg1.b();
    vertice[2] = seg2.seg1.a();
    vertice[3] = seg2.seg1.b();

    Vector2D tangVertice[4];
    tangVertice[0] = seg1.seg2.a();
    tangVertice[1] = seg1.seg2.b();
    tangVertice[2] = seg2.seg2.a();
    tangVertice[3] = seg2.seg2.b();

    double distance[4][4];
    for(int i = 0; i < 4; i++)
        for(int j = i; j < 4; j++)
            distance[i][j] = vertice[i].dist(vertice[j]);

    double maxDist = 0.0;
    int tA = 0, tB = 3;

    for(int i = 0; i < 4; i++)
    {
        for(int j = i; j < 4; j++)
        {
            if(distance[i][j] > maxDist && i != j)
            {
                maxDist = distance[i][j];
                tA = i;
                tB = j;
            }
        }
    }
    temp2Seg.seg1 = Segment2D(vertice[tA], vertice[tB]);
    temp2Seg.seg2 = Segment2D(tangVertice[tA], tangVertice[tB]);
    return temp2Seg;
}

void kkPlayOn::kkMergeSegments(QList<kk2Segment2D> &segList)
{
    if(segList.length() <= 0)
        return;
    bool check = false;
    for(int i = 0; i < segList.length(); i++)
    {
        for(int j = i; j < segList.length(); j++)
        {
            if(kkHasOverlap(segList.at(i).seg1, segList.at(j).seg1) && i != j)
            {
                segList.replace(i ,kkMergeTwoSegments(segList.at(i), segList.at(j)));
                segList.removeAt(j);
                check = true;
                break;
            }
        }
        if(check) break;
    }

    if(check)
        kkMergeSegments(segList);
}

void kkPlayOn::kkInvertSegments(QList<kk2Segment2D> &segList, Segment2D tSeg, QList<kk2Vector2D> &vecList)
{
    kk2Vector2D temp2Vec;
    if(segList.length() <= 0)
    {
        temp2Vec.vec1 = tSeg.a();
        temp2Vec.vec2 = tSeg.a();
        vecList.append(temp2Vec);
        temp2Vec.vec1 = tSeg.b();
        temp2Vec.vec2 = tSeg.b();
        vecList.append(temp2Vec);
        return;
    }
    QList<double> distList;
    QList<int> idList;
    QList<kk2Vector2D> vectorList;
    AngleDeg tempDegree = (tSeg.a() - tSeg.b()).dir();
    for(int i = 0; i < segList.length(); i++)
    {
        if(tempDegree == (tSeg.a() -  segList.at(i).seg1.a()).dir() )
            distList.append(tSeg.a().dist(segList.at(i).seg1.a()));
        else
            distList.append(-tSeg.a().dist(segList.at(i).seg1.a()));
        temp2Vec.vec1 = segList.at(i).seg1.a();
        temp2Vec.vec2 = segList.at(i).seg2.a();
        vectorList.append(temp2Vec);
        idList.append(i*2);

        if(tempDegree == (tSeg.a() -  segList.at(i).seg1.b()).dir())
            distList.append(tSeg.a().dist(segList.at(i).seg1.b()));
        else
            distList.append(-tSeg.a().dist(segList.at(i).seg1.b()));
        temp2Vec.vec1 = segList.at(i).seg1.b();
        temp2Vec.vec2 = segList.at(i).seg2.b();
        vectorList.append(temp2Vec);
        idList.append(i*2 + 1);
    }

    double temp;
    int tempID;
    for(int i = 0; i < distList.length(); i++)
    {
        for(int j = i; j < distList.length(); j++)
        {
             if(distList.at(i) > distList.at(j))
             {
                  temp = distList.at(j);
                  distList.replace(j, distList.at(i));
                  distList.replace(i, temp);

                  tempID = idList.at(j);
                  idList.replace(j, idList.at(i));
                  idList.replace(i, tempID);
             }
        }
    }

    for(int i = 0; i < idList.length(); i++)
    {
        temp2Vec.vec1 = vectorList.at(idList.at(i)).vec1;
        temp2Vec.vec2 = vectorList.at(idList.at(i)).vec2;
        vecList.append(temp2Vec);
    }

    bool checkA = false, checkB = false;

    if(vecList.length() >= 2)
    {
        if(Segment2D(vecList.at(0).vec1, vecList.at(1).vec1).contains(tSeg.a()))
           checkA = true;

        if(Segment2D(vecList.at(vecList.length() - 2).vec1, vecList.at(vecList.length() - 1).vec1).contains(tSeg.b()))
           checkB = true;
    }

    if(checkA)
        vecList.removeAt(0);
    else
    {
        temp2Vec.vec1 = tSeg.a();
        temp2Vec.vec2 = tSeg.a();
        vecList.prepend(temp2Vec);
    }

    if(checkB)
        vecList.removeLast();
    else
    {
        temp2Vec.vec1 = tSeg.b();
        temp2Vec.vec2 = tSeg.b();
        vecList.append(temp2Vec);
    }
}

kk2Segment2D kkPlayOn::kkBlockedByRobots(Circle2D tRobot, Vector2D tOrigin, Line2D tLine)
{
    kk2Segment2D temp2Seg;
    Vector2D tangentPointR, tangentPointL;
    tRobot.tangent(tOrigin, &tangentPointR, &tangentPointL);
    Line2D tempRLine(tOrigin, tangentPointR);
    Line2D tempLLine(tOrigin, tangentPointL);
    Vector2D tempRPoint, tempLPoint;
    tempRPoint = tLine.intersection(tempRLine);
    tempLPoint = tLine.intersection(tempLLine);
    temp2Seg.seg1 = Segment2D(tempLPoint, tempRPoint);
    temp2Seg.seg2 = Segment2D(tangentPointL, tangentPointR);
    return temp2Seg;
}

void kkPlayOn::kkInsertPassObstacleToList(double reachTime, Polygon2D mPoly, Segment2D lLine, Segment2D rLine, int tException)
{
    kkPassObstacle.clear();
    Circle2D oppCircle;
    double addedRadius;
    Vector2D sol1, sol2;
    for(int i = 0; i < wm->our.activeAgentsCount(); i++)
    {
        if(tException != -1)
            if(wm->our.active(i)->id == kkAgentsId[tException])
                continue;
        oppCircle.assign(wm->our.active(i)->pos, 0.12);
        if(mPoly.contains(wm->our.active(i)->pos))
        {
            kkPassObstacle.append(oppCircle);
        }
        else
        {
            if(oppCircle.intersection(lLine, &sol1, &sol2) > 0 || oppCircle.intersection(rLine, &sol1, &sol2))
            {
                kkPassObstacle.append(oppCircle);
            }
        }
    }
    for(int i = 0; i < wm->opp.activeAgentsCount(); i++)
    {
        addedRadius = wm->opp.active(i)->vel.length()*reachTime;
        oppCircle.assign(wm->opp.active(i)->pos, 0.15 + addedRadius);
        if(mPoly.contains(wm->opp.active(i)->pos))
        {
            kkPassObstacle.append(oppCircle);
        }
        else
        {
            if(oppCircle.intersection(lLine, &sol1, &sol2) > 0 || oppCircle.intersection(rLine, &sol1, &sol2))
            {
                kkPassObstacle.append(oppCircle);
            }
        }
    }
}

void kkPlayOn::kkSortObstacleByAngle(Vector2D tBallPos)
{
    QList<double> agentAngles;
    QList<int> agentIDs;
    for(int i = 0; i < kkPassObstacle.length(); i++)
    {
        agentAngles.append((kkPassObstacle.at(i).center() - tBallPos).th().degree());
        agentIDs.append(i);
    }

    double temp;
    int tempID;
    for(int i = 0; i < agentAngles.length(); i++)
    {
        for(int j = i; j < agentAngles.length(); j++)
        {
             if(agentAngles.at(i) > agentAngles.at(j))
             {
                  temp = agentAngles.at(j);
                  agentAngles.replace(j, agentAngles.at(i));
                  agentAngles.replace(i, temp);

                  tempID = agentIDs.at(j);
                  agentIDs.replace(j, agentIDs.at(i));
                  agentIDs.replace(i, tempID);
             }
        }
    }

    QList<Circle2D> tempObstacle;
    for(int i = 0; i < agentIDs.length(); i++)
        tempObstacle.append(kkPassObstacle.at(agentIDs.at(i)));

    kkPassObstacle.clear();
    for(int i = 0; i < agentIDs.length(); i++)
        kkPassObstacle.append(tempObstacle.at(i));
}

Vector2D kkPlayOn::kkFindBestPosForPass(int fromAgent, int regionId, bool offensive, double reachTime, int tException)
{
    regionId--;
    if(regionId < 0) regionId = 0;
    if(regionId > 9) regionId = 9;
    kkInsertObstacleToList(regionId);
    Rect2D offRect, defRect;
    offRect.assign(kkARegion[regionId].center().x, kkARegion[regionId].top(), kkARegion[regionId].size().length()/2, kkARegion[regionId].size().width());
    defRect.assign(kkARegion[regionId].left(), kkARegion[regionId].top(), kkARegion[regionId].size().length()/2, kkARegion[regionId].size().width());

    Polygon2D tempPoly;
    bool canKick = kkFindBestRegionsToPass(regionId, reachTime, tempPoly, tException);

    if(offensive)
        kkBestMovePos = Vector2D(offRect.center());
    else
        kkBestMovePos = Vector2D(defRect.center());
    if(canKick)
        kkAmirSearch(kkBestMovePos.x, kkBestMovePos.y, regionId, tempPoly, 0.08);
    else
        kkAmirSearch(kkBestMovePos.x, kkBestMovePos.y, regionId, 0.08);
    return kkBestMovePos;
}

bool kkPlayOn::kkCheckIntersectWithAgents(Segment2D tSeg)
{
    Vector2D sol1, sol2;
    for(int i = 0; i < wm->our.activeAgentsCount(); i++)
        if(Circle2D(wm->our.active(i)->pos, 0.10).intersection(tSeg, &sol1, &sol2) > 0)
            return true;

    for(int i = 0; i < wm->opp.activeAgentsCount(); i++)
        if(Circle2D(wm->opp.active(i)->pos, 0.10).intersection(tSeg, &sol1, &sol2) > 0)
            return true;

    return false;
}

bool kkPlayOn::kkIsPathToOppGoaliClear()
{
    Segment2D segToOppGoali[5];
    Vector2D tempMousePos = wm->ball->pos;
    int tCnt = 0;
    int intersectCnt = 0;
    for(double y = wm->field->oppGoalR().y; y <= wm->field->oppGoalL().y; y += _GOAL_WIDTH/4.01)
    {
        segToOppGoali[tCnt].assign(tempMousePos, Vector2D(wm->field->oppGoalR().x, y));
        if(kkCheckIntersectWithAgents(segToOppGoali[tCnt]))
            intersectCnt++;
        draw(segToOppGoali[tCnt], QColor(Qt::green));
        tCnt++;
    }
    if(intersectCnt < 5)
        return true;
    else
        return false;
}

bool kkPlayOn::kkIsPathFromAToBClear(Vector2D tA, Vector2D tB, double tTolerance)
{
    Line2D tempLine(tA, tB);
    Line2D perpenLine = tempLine.perpendicular(tB);
    Circle2D tempCircle(tB, tTolerance);
    Vector2D sol1, sol2;
    tempCircle.intersection(perpenLine, &sol1, &sol2);

    int intersectCnt = 0;

    if(kkCheckIntersectWithAgents(Segment2D(tA, tB)))
        intersectCnt++;

    draw(Segment2D(tA, tB),QColor(Qt::cyan));

    if(kkCheckIntersectWithAgents(Segment2D(tA, sol1)))
        intersectCnt++;

    draw(Segment2D(tA, sol1),QColor(Qt::cyan));

    if(kkCheckIntersectWithAgents(Segment2D(tA, sol2)))
        intersectCnt++;

    draw(Segment2D(tA, sol2),QColor(Qt::cyan));

    if(intersectCnt < 3)
        return true;
    else
        return false;
}

void kkPlayOn::kkAssignTask(int tAgent, PSkills tSkill, int tA, int tB)
{
    //agentTask[tAgent]->setAgent(soccer->agents[agentsId[tAgent]]);

    debug(QString::number(kkGetIntByEnum(tSkill)), 0);
    Vector2D tempA, tempB;
    switch(tSkill)
    {
        case None:
        break;
        case MoveOffensive:
            tempB = kkFindBestPosForMove(tAgent, tB, true);
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTargetVel(wm->ball->pos);
            kkAgentTasks[tAgent]->setTargetVel(Vector2D(0, 0));
            kkAgentTasks[tAgent]->setSelectedSkill(SkillGotopointAvoid);
        break;
        case MoveDefensive:
            tempB = kkFindBestPosForMove(tAgent, tB, false);
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTargetVel(wm->ball->pos);
            kkAgentTasks[tAgent]->setTargetVel(Vector2D(0, 0));
            kkAgentTasks[tAgent]->setSelectedSkill(SkillGotopointAvoid);
        break;
        case PassOffensive:
            tempB = kkFindBestPosForPass(tAgent, tB, true, 0, kkPassReceiverAgent);
            kkPointToPass = tempB;
            tempA = wm->ball->pos;
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTolerance(0.1);
            if(kkIsPathFromAToBClear(tempA, tempB, 0.05))
            {
                kkAgentTasks[tAgent]->setChip(false);
                if(wm->getIsSimulMode())
                    kkAgentTasks[tAgent]->setKickSpeed(8);
                else
                    kkAgentTasks[tAgent]->setKickSpeed(500);
            }
            else
            {
                kkAgentTasks[tAgent]->setChip(true);
                if(wm->getIsSimulMode())
                    kkAgentTasks[tAgent]->setKickSpeed(10);
                else
                    kkAgentTasks[tAgent]->setKickSpeed(550);
            }
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
            kkAgentTasks[tAgent]->setAvoidCenterCircle(false);
            kkAgentTasks[tAgent]->setIsGotoPointAvoid(true);
            kkAgentTasks[tAgent]->setSlow(true);
            kkAgentTasks[tAgent]->setSelectedSkill(SkillKick);
        break;
        case PassDefensive:
            tempB = kkFindBestPosForPass(tAgent, tB, false, 0, kkPassReceiverAgent);
            kkPointToPass = tempB;
            tempA = wm->ball->pos;
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTolerance(0.1);
            if(kkIsPathFromAToBClear(tempA, tempB, 0.05))
            {
                kkAgentTasks[tAgent]->setChip(false);
                if(wm->getIsSimulMode())
                    kkAgentTasks[tAgent]->setKickSpeed(8);
                else
                    kkAgentTasks[tAgent]->setKickSpeed(500);
            }
            else
            {
                kkAgentTasks[tAgent]->setChip(true);
                if(wm->getIsSimulMode())
                    kkAgentTasks[tAgent]->setKickSpeed(10);
                else
                    kkAgentTasks[tAgent]->setKickSpeed(550);
            }
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
            kkAgentTasks[tAgent]->setAvoidCenterCircle(false);
            kkAgentTasks[tAgent]->setIsGotoPointAvoid(true);
            kkAgentTasks[tAgent]->setSlow(true);
            kkAgentTasks[tAgent]->setSelectedSkill(SkillKick);
        break;
        case KickOffensive:
            tempB = kkFindBestPosForPass(tAgent, tB, true, 0.0);
            tempA = wm->ball->pos;
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTolerance(0.2);
            kkAgentTasks[tAgent]->setChip(false);
            if(wm->getIsSimulMode())
                kkAgentTasks[tAgent]->setKickSpeed(8);
            else
                kkAgentTasks[tAgent]->setKickSpeed(550);
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
            kkAgentTasks[tAgent]->setAvoidCenterCircle(false);
            kkAgentTasks[tAgent]->setIsGotoPointAvoid(true);
            kkAgentTasks[tAgent]->setSlow(false);
            kkAgentTasks[tAgent]->setSelectedSkill(SkillKick);
        break;
        case KickDefensive:
            tempB = kkFindBestPosForPass(tAgent, tB, false, 0.0);
            tempA = wm->ball->pos;
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTolerance(0.2);
            kkAgentTasks[tAgent]->setChip(false);
            if(wm->getIsSimulMode())
                kkAgentTasks[tAgent]->setKickSpeed(8);
            else
                kkAgentTasks[tAgent]->setKickSpeed(550);
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
            kkAgentTasks[tAgent]->setAvoidCenterCircle(false);
            kkAgentTasks[tAgent]->setIsGotoPointAvoid(true);
            kkAgentTasks[tAgent]->setSlow(false);
            kkAgentTasks[tAgent]->setSelectedSkill(SkillKick);
        break;
        case ChipOffensive:
            tempB = kkFindBestPosForPass(tAgent, tB, true, 0.0);
            tempA = wm->ball->pos;
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTolerance(0.2);
            kkAgentTasks[tAgent]->setChip(true);
            if(wm->getIsSimulMode())
                kkAgentTasks[tAgent]->setKickSpeed(8);
            else
                kkAgentTasks[tAgent]->setKickSpeed(550);
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
            kkAgentTasks[tAgent]->setAvoidCenterCircle(false);
            kkAgentTasks[tAgent]->setIsGotoPointAvoid(true);
            kkAgentTasks[tAgent]->setSlow(false);
            kkAgentTasks[tAgent]->setSelectedSkill(SkillKick);
        break;
        case ChipDefensive:
            tempB = kkFindBestPosForPass(tAgent, tB, false, 0.0);
            tempA = wm->ball->pos;
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTolerance(0.2);
            kkAgentTasks[tAgent]->setChip(true);
            if(wm->getIsSimulMode())
                kkAgentTasks[tAgent]->setKickSpeed(8);
            else
                kkAgentTasks[tAgent]->setKickSpeed(550);
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
            kkAgentTasks[tAgent]->setAvoidCenterCircle(false);
            kkAgentTasks[tAgent]->setIsGotoPointAvoid(true);
            kkAgentTasks[tAgent]->setSlow(false);
            kkAgentTasks[tAgent]->setSelectedSkill(SkillKick);
        break;
        case MarkOffensive:
        break;
        case MarkDefensive:
        break;
        case CatchBall:
            tempB = wm->field->oppGoal();
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTolerance(0.2);
            kkAgentTasks[tAgent]->setChip(false);
            kkAgentTasks[tAgent]->setKickSpeed(0);
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
            kkAgentTasks[tAgent]->setAvoidCenterCircle(false);
            kkAgentTasks[tAgent]->setIsGotoPointAvoid(false);
            kkAgentTasks[tAgent]->setSlow(false);
            kkAgentTasks[tAgent]->setSelectedSkill(SkillKick);
        break;
        case Shot:
            tempB = wm->field->oppGoal();
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTolerance(0.2);
            kkAgentTasks[tAgent]->setChip(false);
            if(wm->getIsSimulMode())
                kkAgentTasks[tAgent]->setKickSpeed(10);
            else
                kkAgentTasks[tAgent]->setKickSpeed(1023);
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
            kkAgentTasks[tAgent]->setAvoidCenterCircle(false);
            kkAgentTasks[tAgent]->setIsGotoPointAvoid(false);
            kkAgentTasks[tAgent]->setSlow(false);
            kkAgentTasks[tAgent]->setSelectedSkill(SkillKick);
        break;
        case ChipToGoal:
            tempB = wm->field->oppGoal();
            kkAgentTasks[tAgent]->setTarget(tempB);
            kkAgentTasks[tAgent]->setTolerance(0.2);
            kkAgentTasks[tAgent]->setChip(true);
            if(wm->getIsSimulMode())
                kkAgentTasks[tAgent]->setKickSpeed(10);
            else
                kkAgentTasks[tAgent]->setKickSpeed(650);
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
            kkAgentTasks[tAgent]->setAvoidCenterCircle(false);
            kkAgentTasks[tAgent]->setIsGotoPointAvoid(false);
            kkAgentTasks[tAgent]->setSlow(false);
            kkAgentTasks[tAgent]->setSelectedSkill(SkillKick);
        break;
        case ReceivePass:
            tempA = kkPointToPass;
            kkAgentTasks[tAgent]->setWaitPos(tempA);
            kkAgentTasks[tAgent]->setSlow(false);
            kkAgentTasks[tAgent]->setReceiveRadius(1.75);
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
            kkAgentTasks[tAgent]->setAvoidCenterCircle(false);
        break;
        case OneTouch:
            tempA = kkPointToPass;
            tempB = wm->field->oppGoal();
            kkAgentTasks[tAgent]->setWaitPos(tempA);
            kkAgentTasks[tAgent]->setTarget(tempB);
            if(wm->getIsSimulMode())
                kkAgentTasks[tAgent]->setKickSpeed(10);
            else
                kkAgentTasks[tAgent]->setKickSpeed(1023);
            kkAgentTasks[tAgent]->setChip(false);
            kkAgentTasks[tAgent]->setAvoidPenaltyArea(true);
        break;
    }

}

bool kkPlayOn::kkHasReceiveOrOneTouch(int tAgent)
{
    for(int i = 0; i < 4; i++)
    {
        if(kkCurrentPlan.agents[tAgent].skill[i] != None)
        {
            if(kkCurrentPlan.agents[tAgent].skill[i] == ReceivePass ||
               kkCurrentPlan.agents[tAgent].skill[i] == OneTouch )
                  return true;
        }
    }
    return false;
}

void kkPlayOn::kkAssignTasks()
{
    int receiveIndex = -1;
    for(int i = 0; i < kkCurrentPlan.agentsSize; i++)
    {
        if(kkHasReceiveOrOneTouch(i))
        {
            kkPassReceiverAgent = i;
            break;
        }
    }
    for(int i = 0; i < kkCurrentPlan.agentsSize; i++)
    {
        if(!kkHasReceiveOrOneTouch(i))
        {
            kkAssignTask(i,
                       kkCurrentPlan.agents[i].skill[0],
                       kkCurrentPlan.agents[i].A,
                       kkCurrentPlan.agents[i].B);
        }
        else
            receiveIndex = i;
    }
    if(receiveIndex != -1)
    {
        kkAssignTask(receiveIndex,
                   kkCurrentPlan.agents[receiveIndex].skill[0],
                   kkCurrentPlan.agents[receiveIndex].A,
                   kkCurrentPlan.agents[receiveIndex].B);
    }
}

void kkPlayOn::kkExecute()
{
    for(int i = 0; i < kkCurrentPlan.agentsSize; i++)
    {
        kkAgentTasks[i]->execute();
    }
}*/

struct robotPos {
    Vector2D targetPos;
    Vector2D targetDir;
    long wait;
    double speed;
    bool avoid;
};

class paladiumDemo {
public:
    paladiumDemo(int _size);
    //~paladiumDemo();

    QList<robotPos> pointList[6];
    void appendPos(int index,Vector2D _Pos, Vector2D _Dir, long wait = 100, double speed = 4.0, bool _avoid = false);
    void prependPos(int index,Vector2D _Pos, Vector2D _Dir, long wait = 100, double speed = 4.0, bool _avoid = false);
    void reset(int index);

    bool doneTasks[6];
    bool isTaskDone(int index);
    bool isAllTasksDone();
    bool decidePos;
    void execute();

    bool doneWait[6];
    long lastTime;
    bool isWaitDone(int index);
    bool isAllWaitingsDone();

    int currentIndex;

    void drawPoints();

    int agentSize;
    CSkillGotoPointAvoid *gpa[6];
};

paladiumDemo::paladiumDemo(int _size)
{
    agentSize = _size;
    for(int i = 0; i < _size; i++)
    {
        gpa[i] = new CSkillGotoPointAvoid(knowledge->getAgent(i));
        gpa[i]->setNoAvoid(true);
        gpa[i]->setSlowMode(false);
    }
    for(int i = 0; i < 6; i++)
    {
        doneTasks[i] = false;
        doneWait[i] = false;
    }
    currentIndex = 0;
    decidePos = true;
}

void paladiumDemo::appendPos(int index, Vector2D _Pos, Vector2D _Dir, long _Wait, double _Speed, bool _Avoid)
{
    robotPos tempRobotPos;
    tempRobotPos.targetPos = _Pos;
    tempRobotPos.targetDir = _Dir;
    tempRobotPos.wait = _Wait;
    tempRobotPos.speed  = _Speed;
    tempRobotPos.avoid = _Avoid;

    pointList[index].append(tempRobotPos);

}

void paladiumDemo::prependPos(int index, Vector2D _Pos, Vector2D _Dir, long _Wait, double _Speed, bool _Avoid)
{
    robotPos tempRobotPos;
    tempRobotPos.targetPos = _Pos;
    tempRobotPos.targetDir = _Dir;
    tempRobotPos.wait = _Wait;
    tempRobotPos.speed  = _Speed;
    tempRobotPos.avoid = _Avoid;

    pointList[index].prepend(tempRobotPos);

}

void paladiumDemo::reset(int index){
    if(index < 0) index = 0;
    if(index > 5) index = 5;
    pointList[index].clear();
}

bool paladiumDemo::isTaskDone(int index)
{
    if(index < 0) index = 0;
    if(index > 5) index = 5;
    if(wm->our.active(index)->pos.dist(pointList[index].at(currentIndex).targetPos) < 0.07 &&
       wm->our.active(index)->dir.angleWith(pointList[index].at(currentIndex).targetDir).degree() < 10)
        return true;
    else
        return false;
}

bool paladiumDemo::isAllTasksDone()
{
    for(int i = 0; i < agentSize; i++)
    {
        if(isTaskDone(i))
            doneTasks[i] = true;
    }
    int tempCnt = 0;
    for(int i = 0; i < agentSize; i++)
        if(doneTasks[i])
            tempCnt++;

    if(tempCnt >= agentSize)
        return true;
    else
        return false;
}

bool paladiumDemo::isWaitDone(int index)
{
    if(index < 0) index = 0;
    if(index > 5) index = 5;
    if(knowledge->getCurrentTime() - lastTime > pointList[index].at(currentIndex).wait)
        return true;
    else
        return false;
}

bool paladiumDemo::isAllWaitingsDone()
{
    for(int i = 0; i < agentSize; i++)
    {
        if(isWaitDone(i))
            doneWait[i] = true;
    }
    int tempCnt = 0;
    for(int i = 0; i < agentSize; i++)
        if(doneWait[i])
            tempCnt++;

    if(tempCnt >= agentSize)
        return true;
    else
        return false;
}

void paladiumDemo::drawPoints()
{
    for(int i = 0; i < agentSize; i++)
    {
//        if(pointList[i].length() > currentIndex - 1)
//        {
            draw(Circle2D(pointList[i].at(currentIndex).targetPos, 0.10), QColor(Qt::cyan));
            Segment2D tempSeg(pointList[i].at(currentIndex).targetPos, pointList[i].at(currentIndex).targetPos + pointList[i].at(currentIndex).targetDir.norm()*0.11);
            draw(tempSeg, QColor(Qt::red));
//        }
    }
}

void paladiumDemo::execute()
{
    if(decidePos)
    {
        for(int i = 0; i < 6; i++)
        {
            doneTasks[i] = false;
            doneWait[i] = false;
        }
        for(int i = 0; i < agentSize; i++)
        {
            gpa[i]->setTarget(pointList[i].at(currentIndex).targetPos,
                              pointList[i].at(currentIndex).targetDir);
            gpa[i]->setMaxVelocity(pointList[i].at(currentIndex).speed);
            gpa[i]->setNoAvoid(pointList[i].at(currentIndex).avoid);
        }
        decidePos = false;
    }

    for(int i = 0; i < agentSize; i++)
        gpa[i]->execute();

    drawPoints();

    /*CSkillGotoPointAvoid gpa2(knowledge->getAgent(i));
    gpa2.setTarget(pointList[i].at(currentIndex).targetPos,
                   pointList[i].at(currentIndex).targetDir);
    gpa2.setMaxVelocity(pointList[i].at(currentIndex).speed);
    gpa2.setNoAvoid(pointList[i].at(currentIndex).avoid);*/

    if(isAllTasksDone())
    {
        debug("check",D_KK);
        if(isAllWaitingsDone())
        {
            debug("wait check",D_KK);
            decidePos = true;
            if(pointList[0].length() - 1 > currentIndex)
                currentIndex++;
        }
    }
    else
        lastTime = knowledge->getCurrentTime();
}

void CMainApplication::Experimental1()
{
   static paladiumDemo demo1(3);
   static bool check = false;
   if(!check)
   {
       Vector2D dir1 = Vector2D(-1.7, -1) - Vector2D(1.7, -1);
       Vector2D dir2 = Vector2D(0, 2) - Vector2D(-1.7, -1);
       Vector2D dir3 = Vector2D(1.7, -1) - Vector2D(0, 2);

       demo1.appendPos(0, Vector2D(1.7, -1), dir1);
       demo1.appendPos(1, Vector2D(-1.7, -1), dir2);
       demo1.appendPos(2, Vector2D(0, 2), dir3);

       demo1.appendPos(0, Vector2D(0, 2), dir3);
       demo1.appendPos(1, Vector2D(1.7, -1), dir1);
       demo1.appendPos(2, Vector2D(-1.7, -1), dir2);

       Vector2D dir4 = Vector2D(0, 2) - Vector2D(0, 0);
       Vector2D dir5 = Vector2D(1.7, -1) - Vector2D(0, 0);
       Vector2D dir6 = Vector2D(-1.7, -1) - Vector2D(0, 0);

       demo1.appendPos(0, Vector2D(0, 2), dir4);
       demo1.appendPos(1, Vector2D(1.7, -1), dir5);
       demo1.appendPos(2, Vector2D(-1.7, -1), dir6);

       demo1.appendPos(0, Vector2D(0, 0.2), dir4);
       demo1.appendPos(1, Vector2D(0.17, -0.1), dir5);
       demo1.appendPos(2, Vector2D(-0.17, -0.1), dir6);

       Vector2D dir7 = Vector2D(-0.17, 0.1) - Vector2D(0, 0);
       Vector2D dir8 = Vector2D(0, -0.2) - Vector2D(0, 0);
       Vector2D dir9 = Vector2D(0.17, 0.1) - Vector2D(0, 0);

       demo1.appendPos(0, Vector2D(-0.17, 0.1), dir7);
       demo1.appendPos(1, Vector2D(0, -0.2), dir8);
       demo1.appendPos(2, Vector2D(0.17, 0.1), dir9);

       demo1.appendPos(0, Vector2D(-1.7, 1), dir7);
       demo1.appendPos(1, Vector2D(0, -2), dir8);
       demo1.appendPos(2, Vector2D(1.7, 1), dir9);


       check = true;
   }
   else
       demo1.execute();
    /*
   static bool kkCheck = false;
   static kkPlayOn myAttackPlan;

   myAttackPlan.setSoccer(soccer);

   myAttackPlan.drawRect();


   draw(mousePosition);
   int temp = myAttackPlan.getArea(myAttackPlan.bRegion, 6, mousePosition);

   draw(QString::number(temp),Vector2D(-2, 1));


       myAttackPlan.getBestPlan(2, 3);
       kkCheck = true;


   //bool Check = myAttackPlan.isPlanMatch(myAttackPlan.agentList, myAttackPlan._3AgentPlan[1]->at(0));
   bool Check = myAttackPlan.isPathFromAToBClear(wm->ball->pos, mousePosition);
   draw(Check?"true":"false",Vector2D(-2, 1.2));

   myAttackPlan.assignId();

   Vector2D tempPoint = myAttackPlan.findBestPosForPass(0, 9, false, 0, 1);

   draw(tempPoint);*/

   /*int temp = myAttackPlan.kkGetArea(myAttackPlan.kkBRegion, 6, wm->ball->pos);
   if(!kkCheck)
   {
      myAttackPlan.kkSetSoccer(soccer);
      myAttackPlan.kkGetBestPlan(temp, wm->our.activeAgentsCount());
      myAttackPlan.kkAssignTasks();
      kkCheck = true;
   }
   draw(myAttackPlan.kkPointToPass);









                  int i = 2;
                  CSkillGotoPointAvoid *gotoPointAvoidSkill = new CSkillGotoPointAvoid(soccer->agents[myAttackPlan.kkAgentsId[2]]);
                  gotoPointAvoidSkill->setFinalPos(myAttackPlan.kkAgentTasks[i]->getTarget());
                  gotoPointAvoidSkill->setFinalDir(myAttackPlan.kkAgentTasks[i]->getTargetDir());
                  gotoPointAvoidSkill->setFinalVel(myAttackPlan.kkAgentTasks[i]->getTargetVel());
                  gotoPointAvoidSkill->oppRelax(myAttackPlan.kkAgentTasks[i]->getOppRelax());
                  gotoPointAvoidSkill->ourRelax(myAttackPlan.kkAgentTasks[i]->getOurRelax());
                  gotoPointAvoidSkill->setAvoidCenterCircle(myAttackPlan.kkAgentTasks[i]->getAvoidPenaltyArea());
                  gotoPointAvoidSkill->setAvoidPenaltyArea(myAttackPlan.kkAgentTasks[i]->getAvoidCenterCircle());
                  gotoPointAvoidSkill->setMaxVelocity(3.0);
                  gotoPointAvoidSkill->execute();

                  i = 0;
                  CSkillKick *kickSkill = new CSkillKick(soccer->agents[myAttackPlan.kkAgentsId[0]]);
                  kickSkill->setTarget(myAttackPlan.kkAgentTasks[i]->getTarget());
                  kickSkill->setTolerance(myAttackPlan.kkAgentTasks[i]->getTolerance());
                  kickSkill->setChip(false);
                  kickSkill->setSlow(true);
                  kickSkill->setSpin(myAttackPlan.kkAgentTasks[i]->getSpin());
                  kickSkill->setKickSpeed(4);
                  kickSkill->setAvoidPenaltyArea(myAttackPlan.kkAgentTasks[i]->getAvoidPenaltyArea());
                  kickSkill->setIsGotoPointAvoid(myAttackPlan.kkAgentTasks[i]->getIsGotoPointAvoid());
                  kickSkill->execute();

                  i = 1;
                  CSkillKickOneTouch *oneTouchSkill = new CSkillKickOneTouch(soccer->agents[myAttackPlan.kkAgentsId[1]]);
                  oneTouchSkill->setWaitPos(myAttackPlan.kkAgentTasks[i]->getWaitPos());
                  oneTouchSkill->setTarget(myAttackPlan.kkAgentTasks[i]->getTarget());
                  oneTouchSkill->setKickSpeed(myAttackPlan.kkAgentTasks[i]->getKickSpeed());
                  oneTouchSkill->setChip(myAttackPlan.kkAgentTasks[i]->getChip());
                  oneTouchSkill->setAvoidPenaltyArea(myAttackPlan.kkAgentTasks[i]->getAvoidPenaltyArea());
                  oneTouchSkill->execute();





   myAttackPlan.kkDrawRect();*/






   #if 0


   /*
   for(int i = 0; i < myAttackPlan.agentList.length(); i++)
   {
      debug(QString::number(myAttackPlan._3AgentPlan[1]->at(0).agents[i].A), 0);
      debug(QString::number(myAttackPlan.getArea(myAttackPlan.aRegion, 10, myAttackPlan.agentList.at(i).pos)), 0, QColor(Qt::blue));
   }*/

   static QTime loopTimer;
   static QTime derivTimer,accTimer;
   static Circle2D ballMargin,virtualBallMargin,realMargin;
   static AngleDeg finalDir= 0;
   finalDir = (wm->field->ourGoal() - wm->ball->pos).th();
   bool farFlag = true;
   Segment2D goalline;
   goalline.assign(wm->ball->pos,wm->field->ourGoal());
   draw(goalline,"black");
    ////////////////find ball margin
   ballMargin.assign(wm->ball->pos,0.119);
   realMargin.assign(wm->ball->pos,0.09);

   if(fabs(((wm->ball->pos - soccer->agents[6]->pos()).th() - finalDir).degree() ) > 130 && soccer->agents[6]->pos().dist(wm->ball->pos) > 0.5)
   virtualBallMargin.assign(wm->ball->pos,0.3);
   else
     virtualBallMargin.assign(wm->ball->pos,0.15);
   draw(realMargin,"blue");
   ///////////////find the place to stop
   Vector2D target, virtualTarget;
   virtualTarget.assign(wm->ball->pos.x - 0.2*cos(finalDir.radian()), wm->ball->pos.y - 0.2*sin(finalDir.radian()));
   target.assign(wm->ball->pos.x - 0.12*cos(finalDir.radian()) + 0.08*wm->ball->vel.x, wm->ball->pos.y - 0.12*sin(finalDir.radian())+ 0.08*wm->ball->vel.y);
    draw(target,D_HOSSEIN,"black");
   ///////////////find the path
   Segment2D straightPath;
   straightPath.assign(target,soccer->agents[6]->pos());

   draw(straightPath,"red");
   Vector2D *path1 = new Vector2D(0,0);
   Vector2D *path2 = new Vector2D(0,0);
   Vector2D destin(-1.5,1);
   _PID angPidd(3,0,0,0,0);

   if(ballMargin.intersection(straightPath,path1,path2) > 1)
   {
     virtualBallMargin.tangent(soccer->agents[6]->pos(),path1,path2);
     draw(*path1,D_HOSSEIN,"green");
     draw(*path2,D_HOSSEIN,"blue");
     if(target.dist(*path1) < target.dist(*path2))
       destin = *path1;
     else
       destin = *path2;
   }
   else
   {
     destin= target ;
   }
   CskillNewGotoPoint *first = new CskillNewGotoPoint(soccer->agents[6]);
   if(soccer->agents[6]->pos().dist(wm->ball->pos) >0.3)
   {

     first->init(destin,finalDir,Vector2D(0,0));
     first->execute();
     soccer->agents[6]->setKick(0);

   }
   else
   {
     double movement_theta;


     if( fabs(soccer->agents[6]->dir().th().degree() - finalDir.degree()) < 5  )
       soccer->agents[6]->setKick(1000);
     else
     {
       angPidd.kp = 3;
       soccer->agents[6]->setKick(0);
     }


     debug(QString("angle turn : %1").arg((soccer->agents[6]->dir().th() - finalDir).degree()),D_HOSSEIN);
     if ((soccer->agents[6]->dir().th() - finalDir).degree()  <-40 )
     {
       angPidd.error = (wm->ball->pos - soccer->agents[6]->pos()).th().radian();

       soccer->agents[6]->setRobotVel(cos((wm->ball->pos - soccer->agents[6]->pos()).th().radian() - _PI/2),sin((wm->ball->pos - soccer->agents[6]->pos()).th().radian() - _PI/2),angPidd.PID_OUT());


     }else if ((soccer->agents[6]->dir().th() - finalDir).degree()  >40 )
     {
       angPidd.error = (wm->ball->pos - soccer->agents[6]->pos()).th().radian();

       soccer->agents[6]->setRobotAbsVel(cos((wm->ball->pos - soccer->agents[6]->pos()).th().radian() + _PI/2),sin((wm->ball->pos - soccer->agents[6]->pos()).th().radian() + _PI/2),angPidd.PID_OUT());

     }
       else
     {
       angPidd.error = (finalDir - soccer->agents[6]->dir().th()).radian();

       if( ((wm->ball->pos - soccer->agents[6]->pos()).th() - finalDir).degree()  > 90  )
         movement_theta = ((wm->ball->pos - soccer->agents[6]->pos()).th().radian() + 1- soccer->agents[6]->dir().th().radian());
       else if(((wm->ball->pos - soccer->agents[6]->pos()).th() - finalDir).degree() <- 90)
         movement_theta = ((wm->ball->pos- soccer->agents[6]->pos()).th().radian() - 1- soccer->agents[6]->dir().th().radian());
       else if(((wm->ball->pos - soccer->agents[6]->pos()).th() - finalDir).degree() >30)
         movement_theta = ((wm->ball->pos- soccer->agents[6]->pos()).th().radian() +0.8- soccer->agents[6]->dir().th().radian());
       else if(((wm->ball->pos - soccer->agents[6]->pos()).th() - finalDir).degree() < -30)
         movement_theta = ((wm->ball->pos- soccer->agents[6]->pos()).th().radian() - 0.8- soccer->agents[6]->dir().th().radian());
       else if(((wm->ball->pos - soccer->agents[6]->pos()).th() - finalDir).degree() >10)
         movement_theta = ((wm->ball->pos- soccer->agents[6]->pos()).th().radian() +0.4- soccer->agents[6]->dir().th().radian());
       else if(((wm->ball->pos - soccer->agents[6]->pos()).th() - finalDir).degree() < -10)
         movement_theta = ((wm->ball->pos- soccer->agents[6]->pos()).th().radian() - 0.4- soccer->agents[6]->dir().th().radian());
       else
         movement_theta = ((wm->ball->pos- soccer->agents[6]->pos()).th().radian() - soccer->agents[6]->dir().th().radian());





       debug(QString("mt : %1 ").arg(movement_theta),D_HOSSEIN);

       double ballx= (wm->ball->vel.x)*cos(soccer->agents[6]->dir().th().radian()) + (wm->ball->vel.y)*sin(soccer->agents[6]->dir().th().radian());
       double bally= -1*(wm->ball->vel.x)*sin(soccer->agents[6]->dir().th().radian()) + (wm->ball->vel.y)*cos(soccer->agents[6]->dir().th().radian());
       soccer->agents[6]->setRobotVel(1.2*cos(movement_theta) + 0.3*ballx,1.2*sin(movement_theta) + 0.3*bally,angPidd.PID_OUT());
     }

   }




  static bool _flag = true;
  if( _flag ){
    loopTimer.start();
    accTimer.start();
    derivTimer.start();
    _flag = false;
  }

  debug(QString("time: %1").arg(loopTimer.elapsed()),D_HOSSEIN);
  loopTimer.restart();
  return;

  // code by DON Shirazi
  Vector2D dest(-1.5,1);
  draw(dest,D_HOSSEIN,"red");
  static double dist=0;
  static double _Vi=0,_Vf=0,_VmDesire=1,vDesired=0,_Acc=4,_Dec=-3,_VmReal=0,_x1=0,_x2=0;
  static AngleDeg movement_th =0,desiredAng =0;
  static double angleError=0;
  ///////////speed PID
  static double s_kp=0.8,s_Kd=0.05,s_KI=0.1,s_error=0,s_D=0,SP_error=0,S_I=0,sPidOut=0;
  ///////////pid params
  static double _kp=1,_Kd=0,_KI=0,_error=0,_D=0,P_error=0;
  ///calculate loop time

  _PID angPid(2,0.01,0,0,0);


  angPid.error = desiredAng.radian()-soccer->agents[6]->dir().th().radian();




  /////////read mouse

  dest.x = knowledge->getMousePos().x;
  dest.y = knowledge->getMousePos().y;
  //calculate distance to target in every frame
  dist = soccer->agents[6]->pos().dist(dest) ;
  //calculate movement angle to target in every frame
  movement_th=(dest-soccer->agents[6]->pos()).th();
  //////PID deriv

  if(derivTimer.elapsed() > 100)
  {
    P_error = dist;
    SP_error =s_error;
    derivTimer.restart();
    angPid.pError = angPid.error;
    //vDesired = _Vi;
  }


  ///// calculate maximum speed


  _VmReal= fabs(  (  (_Dec-_Acc)*_Vf + sqrt(    (_Dec-_Acc)*(_Dec-_Acc)*(_Vf*_Vf-4*_Vf)
                                      + 4*(_Dec-_Acc)*(_Dec*_Vi*_Vi + _Acc*_Vf*_Vf)
                                      + 8*_Acc*_Dec*(_Dec-_Acc)*dist )
              ) / (2*(_Dec-_Acc)) );

  _VmDesire = 4;
  if(_VmDesire > _VmReal)
    _VmDesire =_VmReal;



  //// find robot vel in direction of destination
  _Vi = cos(  movement_th.radian()  -  soccer->agents[6]->vel().dir().radian()   )*(soccer->agents[6]->vel().length()/2);

  _x1 = (dist)-(_VmDesire*_VmDesire - _Vi*_Vi)/(2*_Acc);
  _x2 = ( _Vf*_Vf - _VmDesire*_VmDesire)/(2*_Dec);


  Vector2D ourGoal(-3,0);

  desiredAng =90;//AngleDeg::normalize_angle((ourGoal-wm->our[0]->pos).th().degree());


  static bool Acc_flag=false,Dec_flag=false;
  //////////movement

  if(dist<0.2)
  {
    Dec_flag = false;
    Acc_flag = false;
    _error = dist;
    _D = _error - P_error;
    vDesired = _kp*_error + _Kd*_D;
    soccer->agents[6]->setRobotAbsVel(vDesired*cos(movement_th.radian()),vDesired*sin(movement_th.radian()),angPid.PID_OUT() );
  }
  else
  {

    if(dist<=_x2)
    {
      s_KI = 0;
      if(Dec_flag == false)
      {
        vDesired = _Vi;
        Dec_flag = true;
      }
      Acc_flag = false;

      debug(QString("dec:: %1").arg(_Dec*((double)(loopTimer.elapsed())/1000)),D_HOSSEIN);
      draw(QString("Vdesired : %1").arg(vDesired),Vector2D(0,0),"red");
      if(vDesired > 0)
        vDesired += (_Dec)*((double)(loopTimer.elapsed())/1000);
      else
        vDesired =0;

    }
    else if(dist>_x2 && dist <= _x1)
    {
      s_KI = 0.1;
      Dec_flag = false;
      Acc_flag = false;
      debug(QString("const"),D_HOSSEIN);
      vDesired =_VmDesire;
      draw(QString("Vdesired : %1").arg(vDesired),Vector2D(0,0),"red");

    }
    else
    {
      s_KI = 0.1;

      if(Acc_flag == false)
      {
        vDesired = _Vi;
        Acc_flag = true;
      }

    /*  if(accTimer.elapsed() > 300)
      {

        accTimer.restart();
        vDesired = _Vi;
      }
*/
      Dec_flag = false;
      if(vDesired <= _VmDesire)
        vDesired = (_Vi*0.08 +vDesired*0.92)+_Acc*((double)(loopTimer.elapsed())/1000);
      else
        vDesired = _VmDesire;
      draw(QString("Vdesired : %1").arg(vDesired),Vector2D(0,0),"red");
      debug(QString("acc :: %1 time: %2 Vd: %3").arg(_Acc*(loopTimer.elapsed()/1000)).arg(loopTimer.elapsed()).arg(vDesired),D_HOSSEIN);

    }

    s_error = vDesired - _Vi;
    if(S_I*s_KI < 4)
    {
      S_I += s_error;
    }
    s_D = s_error - SP_error;
    sPidOut =s_kp*s_error + s_Kd*s_D + vDesired ;//+ s_KI*S_I;

    soccer->agents[6]->setRobotAbsVel(sPidOut*cos(movement_th.radian()),sPidOut*sin(movement_th.radian()),angPid.PID_OUT()  );

  }

  debug(QString("mouse: %1  %2 ").arg(knowledge->getMousePos().x).arg(knowledge->getMousePos().y),D_HOSSEIN);

  debug(QString("theta: %1  av : %2").arg((ourGoal-wm->our[0]->pos).th().degree()).arg(wm->our[0]->angularVel),D_HOSSEIN);

  debug(QString("Vmax: %1").arg(_VmReal),D_HOSSEIN);

  debug(QString("x1: %1").arg(_x1),D_HOSSEIN);

  debug(QString("x2: %1").arg(_x2),D_HOSSEIN);

  debug(QString("Dec: %1").arg(_Dec),D_HOSSEIN);

  debug(QString("loop : %1").arg(loopTimer.elapsed()),D_HOSSEIN);


  loopTimer.restart();
  return;












  ///Code By Mahi
  Vector2D bp = wm->ball->pos;

  Vector2D upGoal(-4.5,0.5),downGoal(-4.5,-0.5);
  draw(Segment2D(upGoal,downGoal),"Red");
  draw(Segment2D(bp,upGoal),"Red");
  draw(Segment2D(bp,downGoal),"Red");
  //static CSkillGotoPoint* gtp0 = new CSkillGotoPoint();



  return;
  ///////////
        soccer->agents[4]->setRobotVel(0 , 0 , 2*_PI);
        return;

	static int id = 0;
	static int n = 500;
	int range = 1000;
	static QTime time;
	static bool flag = true;
	if( flag ){
		time.start();
		flag = false;
	}
	if( n ){
		if( time.elapsed() > 1500 ){
			if( time.elapsed() < 2000 )
				soccer->agents[id]->setKick(range);
			else{
				cout << n << endl;
				n--;
				time.restart();
			}
		}
	}
	return;
		Vector2D desired_pos(-1,-1);
	Vector2D new_pos = soccer->agents[3]->pos();
	Vector2D error = desired_pos-new_pos;
	double vel_x,vel_y,vel_w=0,desired_theta=0;
	debug(QString("pos: %1 , %2 , %3").arg(new_pos.x).arg(new_pos.y).arg(soccer->agents[3]->dir().th().degree()),D_HOSSEIN);
	if(error.x<0.1)
		vel_x=error.x*3;
	else if(error.x < 0.1)
		vel_x=0;
	else
		vel_x=error.x;

	if(error.y<0.1)
		vel_y=error.y*2;
	else if(error.y < 0.1)
		vel_y=0;
	else
		vel_y=error.y;

	vel_w=((soccer->agents[3]->dir().th().degree())-desired_theta)*0.05;
	soccer->agents[3]->setRobotAbsVel(vel_x,vel_y,-1*vel_w);
	return;
	//experimental for positioning
	Vector2D position;
	static CSkillGotoPoint* gtp1 = new CSkillGotoPoint(soccer->agents[0]);
	for( int i = 1 ; i < 7 ; i++ )
	{
	position = CCoach::editData["TheirFreeKick6"]->formation()->getPosition(i, wm->ball->pos);
		gtp1->setAgent(soccer->agents[i-1]);
		gtp1->setMaxVelocity(1.5);
		gtp1->init(position,(wm->ball->pos-position));
		if( soccer->agents[i-1]->pos().dist(position) > 0.01 )
		{
			gtp1->execute();
		}
	}
	return;

	soccer->agents[0]->setRobotVel(2,0,0);
//	knowledge->plotWidgetCustom[0] = soccer->agents[0]->vforward;
	return;
	static bool run = true;
	if(run)
	{
		Matrix iden(3);
		fstream file,file2,file3;
		file.open("test/wheels.txt",fstream::in);
		file2.open("test/input.txt",fstream::in);
		file3.open("test/observed.txt",fstream::in);
		Matrix phi(1,4);
		Matrix zita_captured(3,1);
		Matrix zita_desired(3,1);
		Matrix Epsilon(12,1);
		while( file>>phi.e(0,0)>>phi.e(0,1)>>phi.e(0,2)>>phi.e(0,3) )
		{
			file3>>zita_captured.e(0,0)>>zita_captured.e(1,0)>>zita_captured.e(2,0);
			file2>>zita_desired.e(0,0)>>zita_desired.e(1,0)>>zita_desired.e(2,0);
			Epsilon = pseudoinverse(kron(iden,phi))*(zita_captured-zita_desired);
			for( int i=0; i<12; i++)
			{
				cout<<Epsilon.e(i,0)<<" ";
			}
			cout<<endl;
		}
		run = false;
	}

	return;

//	static CSkillGotoPoint* gtp1 = new CSkillGotoPoint(soccer->agents[3]);
//	static Vector2D des1=Vector2D(0,0);
	static double v_t=0,v_n=0,v_w=0;
	static int state = 0,counter=0;
	if( state == 0 )
	{
//		gtp1->setAgent(soccer->agents[3]);
//		gtp1->setMaxVelocity(0.5);
//		gtp1->init(des1,Vector2D(0,0));
//		if( soccer->agents[3]->pos().dist(des1) > 0.01 )
//		{
//			gtp1->execute();
//			v_t = soccer->agents[3]->vforward;
//			v_n = soccer->agents[3]->vnormal;
//			v_w = soccer->agents[3]->vangular;

//		}
//		else
//		{
			state = 1;
			counter=0;
			v_t = randomize(-10000,10000)*0.000100*0.5;
			v_n = randomize(-10000,10000)*0.000075*0.5;
			v_w = randomize(-10000,10000)*0.0001*180.0*0.5;
//		}
	}
	if( state == 1 )
	{
		counter++;
		if(counter>=90)
		{
			state=2;
		}
	}
	if(state==2)
	{
		v_t=0;v_n=0;v_w=0;
	}
//	double v_t=0,v_n=1,v_w=0;
	float robotRadius = 0.0795;
	float wheelRadius = 0.0275;
	double ang = -soccer->agents[3]->dir().th().radian();
	double motorAlpha[4] = {60.0 * _DEG2RAD, 135.0 * _DEG2RAD, 225.0 * _DEG2RAD, 300 * _DEG2RAD};
	soccer->agents[3]->wh1 =  (1.0 / wheelRadius) * (( (robotRadius * v_w*_DEG2RAD) - (v_t * sin(motorAlpha[0])) + (v_n * cos(motorAlpha[0]))) );
	soccer->agents[3]->wh2 =  (1.0 / wheelRadius) * (( (robotRadius * v_w*_DEG2RAD) - (v_t * sin(motorAlpha[1])) + (v_n * cos(motorAlpha[1]))) );
	soccer->agents[3]->wh3 =  (1.0 / wheelRadius) * (( (robotRadius * v_w*_DEG2RAD) - (v_t * sin(motorAlpha[2])) + (v_n * cos(motorAlpha[2]))) );
	soccer->agents[3]->wh4 =  (1.0 / wheelRadius) * (( (robotRadius * v_w*_DEG2RAD) - (v_t * sin(motorAlpha[3])) + (v_n * cos(motorAlpha[3]))) );

	knowledge->plotWidgetCustom[0] = v_t;
	knowledge->plotWidgetCustom[1] = (cos(ang) * soccer->agents[3]->vel().x) - (sin(ang) * soccer->agents[3]->vel().y);
	knowledge->plotWidgetCustom[2] = v_n;
	knowledge->plotWidgetCustom[3] = (sin(ang) * soccer->agents[3]->vel().x) + (cos(ang) * soccer->agents[3]->vel().y);
	//Here
	static double w1, w2, w3, w4;
//	static double v_x=0.5516 ,v_y=-0.0725 ,v_w=-31.8565;
	static double v_x=0.0 ,v_y=1.0 ,v_w=0.0;
	float robot_Radius = 0.0795;
	float wheel_Radius = 0.0275;
	double motor_Alpha[4] = {60.0 * _DEG2RAD, 135.0 * _DEG2RAD, 225.0 * _DEG2RAD, 300 * _DEG2RAD};
	w1 =  (1.0 / wheel_Radius) * (( (robot_Radius * v_w*_DEG2RAD) - (v_x * sin(motor_Alpha[0])) + (v_y * cos(motor_Alpha[0]))) );
	w2 =  (1.0 / wheel_Radius) * (( (robot_Radius * v_w*_DEG2RAD) - (v_x * sin(motor_Alpha[1])) + (v_y * cos(motor_Alpha[1]))) );
	w3 =  (1.0 / wheel_Radius) * (( (robot_Radius * v_w*_DEG2RAD) - (v_x * sin(motor_Alpha[2])) + (v_y * cos(motor_Alpha[2]))) );
	w4 =  (1.0 / wheel_Radius) * (( (robot_Radius * v_w*_DEG2RAD) - (v_x * sin(motor_Alpha[3])) + (v_y * cos(motor_Alpha[3]))) );
//	w1 =  (( (2.8909 * v_w*_DEG2RAD) - (v_x * 75.8044) + (v_y * 18.1819)) );
//	w2 =  (( (2.8909 * v_w*_DEG2RAD) + (v_x * 34.7068) - (v_y * 25.7130)) );
//	w3 =  (( (2.8909 * v_w*_DEG2RAD) + (v_x * 87.4943) - (v_y * 25.7129)) );
//	w4 =  (( (2.8909 * v_w*_DEG2RAD) - (v_x * 11.1527) + (v_y * 18.1818)) );

	static int error_set = true;
	static double e[100];
	if(error_set)
	{
		error_set = false;
		for(int i=0 ; i<100 ; i++)
			e[i] = 10.0;
	}
	static Vector2D lastVel(0,0);
	static double lastOmega = 0;
	static double lastAng = 0;

	double temp = 0;
	for( int counter = 0 ; counter < 100 ; counter++ )
	{
		temp += e[counter];
	}

	if( temp < 100)
	{
			QFile file1("./test/wheels.txt");
			QFile file2("./test/observed.txt");
			QFile file3("./test/input.txt");
			file1.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append );
			file2.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append );
			file3.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append );
			QTextStream ts1(&file1);
			QTextStream ts2(&file2);
			QTextStream ts3(&file3);

			ts1 << w1 << " " << w2 << " " << w3 << " " << w4 << endl;
			ts2 << (cos(-lastAng) * lastVel.x) - (sin(-lastAng) * lastVel.y) << " " << (sin(-lastAng) * lastVel.x) + (cos(-lastAng) * lastVel.y) << " " << lastOmega << endl;
			ts3 << v_x << " " << v_y << " " << v_w << endl;

			file1.close();
			file2.close();
			file3.close();

		for( int i = 0 ; i < 100 ; i++ )
			e[i] = 10.0;

	}
	else
	{
		for( int counter = 98 ; counter > -1 ; counter-- )
			e[counter+1] = e[counter];
		e[0] = fabs(lastVel.x-wm->our[3]->vel.x)+fabs(lastVel.y-wm->our[3]->vel.y);
	}
	lastVel = wm->our[3]->vel;
	lastOmega = wm->our[3]->angularVel;
	lastAng = wm->our[3]->dir.th().radian();

	soccer->agents[3]->wh1 =  w1;
	soccer->agents[3]->wh2 =  w2;
	soccer->agents[3]->wh3 =  w3;
	soccer->agents[3]->wh4 =  w4;
	return;

	static bool b1 = false;//arash
//	static double v_x=0 ,v_y=0 ,v_w=0;

	if(b1){
		static CSkillGotoPoint* gtp1 = new CSkillGotoPoint(soccer->agents[3]);
		static Vector2D des1=Vector2D(1.5,0);
		gtp1->setAgent(soccer->agents[3]);
		gtp1->setMaxVelocity(1.5);
		gtp1->init(des1,Vector2D(1,0));
		if( soccer->agents[3]->pos().dist(des1) > 0.01 )
		{
			gtp1->execute();
			v_x = soccer->agents[3]->vforward;
			v_y = soccer->agents[3]->vnormal;
			v_w = soccer->agents[3]->vangular;
		}
		else
		{
			v_x = 0;
			v_y = 0;
			v_w = 0;
		}
	}
	else
	{
		static clock_t begin = clock();
		static int state = 0;
		if( state == 1 && (  soccer->agents[3]->pos().x < 0 || soccer->agents[3]->pos().x > 2.5 || soccer->agents[3]->pos().absY() > 1.5 ) )
		{
			Vector2D inner = Vector2D(2,0)-soccer->agents[3]->pos();
			v_w = 0;
			double ang = -soccer->agents[3]->dir().th().radian();
			v_x = cos(ang)*inner.x-sin(ang)*inner.y;
			v_y = sin(ang)*inner.x+cos(ang)*inner.y;
			state = 0;
		}
		if( ((double)(clock()- begin) / CLOCKS_PER_SEC) > 2 )
		{
			if( (  soccer->agents[3]->pos().x < 0 || soccer->agents[3]->pos().x > 2.5 || soccer->agents[3]->pos().absY() > 1.5 ) )
			{
				//			state = 1;
				Vector2D inner = Vector2D(2,0)-soccer->agents[3]->pos();
				v_w = 0;
				double ang = -soccer->agents[3]->dir().th().radian();
				v_x = cos(ang)*inner.x-sin(ang)*inner.y;
				v_y = sin(ang)*inner.x+cos(ang)*inner.y;

				state = 0;
			}
			else
			{
				v_x = randomize(-10000,10000)*0.0001;
				v_y = randomize(-10000,10000)*0.0001;
				v_w = randomize(-360,360);
				state = 1;
			}
			begin = clock();
		}
		QFile file("./test/data.txt");
		file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append );
		QTextStream ts(&file);
		double ang = soccer->agents[3]->dir().th().radian();
		ts << v_x << " " << v_y << " " << v_w*_DEG2RAD << " " << (cos(ang) * v_x) - (sin(ang) * v_y) << " " << (sin(ang) * v_x) + (cos(ang) * v_y) << " " << v_w*_DEG2RAD << " " << soccer->agents[3]->pos().x-1.5 << " " << soccer->agents[3]->pos().y << " " << soccer->agents[3]->dir().th().radian();
		file.close();
	}
	//	static int constant_vel = 0;
	//	if(constant_vel > 80)
	//	{
	//		if( v_x > 0.0 )
	//		{
	//			v_x -= 0.09;
	//		}
	//		else
	//		{
	//			v_x = 0.0;
	//		}
	//	}
	//	else if( v_x < 2.5 )
	//	{
	//		v_x += 0.03;
	//	}
	//	else
	//	{
	//		constant_vel++;
	//	}
	float robotRadius = 0.0795;
	float wheelRadius = 0.0275;
	double motorAlpha[4] = {60.0 * _DEG2RAD, 135.0 * _DEG2RAD, 225.0 * _DEG2RAD, 300 * _DEG2RAD};
	soccer->agents[3]->wh1 =  (1.0 / wheelRadius) * (( (robotRadius * v_w*_DEG2RAD) - (v_x * sin(motorAlpha[0])) + (v_y * cos(motorAlpha[0]))) );
	soccer->agents[3]->wh2 =  (1.0 / wheelRadius) * (( (robotRadius * v_w*_DEG2RAD) - (v_x * sin(motorAlpha[1])) + (v_y * cos(motorAlpha[1]))) );
	soccer->agents[3]->wh3 =  (1.0 / wheelRadius) * (( (robotRadius * v_w*_DEG2RAD) - (v_x * sin(motorAlpha[2])) + (v_y * cos(motorAlpha[2]))) );
	soccer->agents[3]->wh4 =  (1.0 / wheelRadius) * (( (robotRadius * v_w*_DEG2RAD) - (v_x * sin(motorAlpha[3])) + (v_y * cos(motorAlpha[3]))) );
	return;

	static CSkillGotoPoint* gtp = new CSkillGotoPoint(soccer->agents[3]);
	static bool _resid = true;
	static Vector2D des=Vector2D(1,0);
	gtp->setAgent(soccer->agents[3]);
	gtp->setMaxVelocity(1.5);
	//	gtp->init(Vector2D(1,0),Vector2D(1,0));
	if( !_resid || (soccer->agents[3]->pos().absX() > 2.9 || soccer->agents[3]->pos().absY() > 1.7) )
	{
		_resid = false;
		soccer->agents[3]->stopTrain = true;
		if( soccer->agents[3]->pos().x > 2.9 )
		{
			des =Vector2D(-2.5,0);
			gtp->init(des,Vector2D(1,0));
		}
		else if( soccer->agents[3]->pos().x < -2.9 )
		{
			des = Vector2D(2.5,0);
			gtp->init(des,Vector2D(1,0));
		}
		else if( soccer->agents[3]->pos().y > 1.7 )
		{
			des = Vector2D(1,-1.3);
			gtp->init(des,Vector2D(1,0));
		}
		else if( soccer->agents[3]->pos().y < -1.7 )
		{
			des =Vector2D(1,1.3);
			gtp->init(des,Vector2D(1,0));
		}
		gtp->execute();
	}
	else
	{
		soccer->agents[3]->stopTrain = false;
	}
	if( soccer->agents[3]->pos().dist(des) < 0.2 )
		_resid = true;

	return;

	static CSkillTurn* turn = new CSkillTurn( soccer->agents[3]);
	turn->setAgent(soccer->agents[3]);
	turn->setTurnMode(CSkillTurn::Slow);
	static int counter = 0;
	static double z = 1;
	static int mode = 0;
	if( counter < 9 )
	{soccer->agents[3]->setRobotVel(z,0.0,0.0);counter++;}

	else if( mode != 1 && (soccer->agents[3]->pos().x > 1) )
	{
		if( ((Vector2D(0,0)-soccer->agents[3]->pos()).th()-soccer->agents[3]->dir().th()).abs() > 1 )
		{
			turn->setDirection((Vector2D(0,0)-soccer->agents[3]->pos()));
			turn->execute();
			//			draw(Segment2D( Vector2D(0,0) ,soccer->agents[3]->pos() ),QColor(30,24,123));
			//			draw(Segment2D( soccer->agents[3]->pos() + Vector2D::polar2vector( 10 , soccer->agents[3]->dir().rotatedVector(-90).th() ) ,
			//							soccer->agents[3]->pos() + Vector2D::polar2vector( 10 , soccer->agents[3]->dir().rotatedVector(90).th() )),
			//				 QColor("cyan"));
		}
		else{
			mode = 1;
			soccer->agents[3]->setRobotVel(z,0.0,0.0);
			counter = 0;}
	}
	else if( mode != 2 && (soccer->agents[3]->pos().y < -1 && soccer->agents[3]->pos().absX() < 1 ) )
	{
		if( ((Vector2D(0,0)-soccer->agents[3]->pos()).th()-soccer->agents[3]->dir().th()).abs() > 1 )
		{
			turn->setDirection((Vector2D(0,0)-soccer->agents[3]->pos()));
			turn->execute();
			//			draw(Segment2D( Vector2D(0,0) ,soccer->agents[3]->pos() ),QColor(30,24,123));
			//			draw(Segment2D( soccer->agents[3]->pos() + Vector2D::polar2vector( 10 , soccer->agents[3]->dir().rotatedVector(-90).th() ) ,
			//							soccer->agents[3]->pos() + Vector2D::polar2vector( 10 , soccer->agents[3]->dir().rotatedVector(90).th() )),
			//				 QColor("cyan"));
		}
		else{
			mode = 2;
			soccer->agents[3]->setRobotVel(z,0.0,0.0);
			counter = 0;}
	}
	else if( mode != 3 && (soccer->agents[3]->pos().x < -1) )
	{
		if( ((Vector2D(0,0)-soccer->agents[3]->pos()).th()-soccer->agents[3]->dir().th()).abs() > 1 )
		{
			turn->setDirection((Vector2D(0,0)-soccer->agents[3]->pos()));
			turn->execute();
			//			draw(Segment2D( Vector2D(0,0) ,soccer->agents[3]->pos() ),QColor(30,24,123));
			//			draw(Segment2D( soccer->agents[3]->pos() + Vector2D::polar2vector( 10 , soccer->agents[3]->dir().rotatedVector(-90).th() ) ,
			//							soccer->agents[3]->pos() + Vector2D::polar2vector( 10 , soccer->agents[3]->dir().rotatedVector(90).th() )),
			//				 QColor("cyan"));
		}
		else
		{
			mode = 3;
			soccer->agents[3]->setRobotVel(z,0.0,0.0);
			counter = 0;}
	}
	else if( mode != 4 && (soccer->agents[3]->pos().y > 1 && soccer->agents[3]->pos().absX() < 1 ) )
	{
		if( ((Vector2D(0,0)-soccer->agents[3]->pos()).th()-soccer->agents[3]->dir().th()).abs() > 1 )
		{
			turn->setDirection((Vector2D(0,0)-soccer->agents[3]->pos()));
			turn->execute();
			//			draw(Segment2D( Vector2D(0,0) ,soccer->agents[3]->pos() ),QColor(30,24,123));
			//			draw(Segment2D( soccer->agents[3]->pos() + Vector2D::polar2vector( 10 , soccer->agents[3]->dir().rotatedVector(-90).th() ) ,
			//							soccer->agents[3]->pos() + Vector2D::polar2vector( 10 , soccer->agents[3]->dir().rotatedVector(90).th() )),
			//				 QColor("cyan"));
		}
		else{
			mode = 4;
			soccer->agents[3]->setRobotVel(z,0.0,0.0);
			counter = 0;}
	}
	else
	{
		soccer->agents[3]->setRobotVel(z,0.0,0.0);
		counter = 0;
		//		draw(Segment2D( Vector2D(0,0) ,soccer->agents[3]->pos() ),QColor(30,24,123));
		//		draw(Segment2D( soccer->agents[3]->pos() + Vector2D::polar2vector( 10 , soccer->agents[3]->dir().rotatedVector(-90).th() ) ,
		//						soccer->agents[3]->pos() + Vector2D::polar2vector( 10 , soccer->agents[3]->dir().rotatedVector(90).th() )),
		//			 QColor("cyan"));
	}



	//	draw(QString("1%").arg(mode));
	//	( fabs( soccer->agents[3]->pos().x ) > 1 || fabs( soccer->agents[3]->pos().y ) > 1 ) ? soccer->agents[3]->setRobotVel(-z,0.0,0.0) : soccer->agents[3]->setRobotVel(z,0.0,0.0) ;


	//			soccer->agents[3]->setRobotVel(0.0,-1.8,0.0);

	//				soccer->agents[3]->setRobotAbsVel(0.0,1.8,0.0);

	double angle = -wm->our[3]->dir.th().radian();
	knowledge->plotWidgetCustom[0] = z;
	knowledge->plotWidgetCustom[1] = (cos(angle) * wm->our[3]->vel.x) - (sin(angle) * wm->our[3]->vel.y);
	return;

	int _id = 4;

	double v1 , v2 , v3 , v4;
	soccer->agents[_id]->setRobotVel(1 , 1 , 0);
	jacobian(1 , 1 , 0 , v1 , v2 , v3 , v4 , soccer->agents[_id]);
	qDebug() << "HERE " << v1 << v2 << v3 << v4 << endl;
	return;

	//	soccer->agents[7]->setRobotVel(0 , 0 , 2*_PI);

	//	qDebug() << "OMEGA " << soccer->agents[7]->angularVel();
	//	return;

	//	soccer->agents[_id]->setRobotVel(0 , 0 , _PI);
	////	knowledge->plotWidgetCustom[0] = soccer->agents[_id]->angularVel();
	//	qDebug() << "w  " << soccer->agents[_id]->angularVel();

	static int cnt = 0;
	static Matrix epsSum;
	Matrix I3 , Acomm , Acapt , Aideal , epsBar;
	I3.resize(3,3);
	Acomm.resize(1,4);
	Acapt.resize(3,1);
	Aideal.resize(3,1);
	for( int i=0 ; i<3 ; i++ ){
		for( int j=0 ; j<3 ; j++ ){
			if( i == j )
				I3.e(i,j) = 1;
			else
				I3.e(i,j) = 0;
		}
	}

	double a=2 , b=0 , c=0;

	soccer->agents[_id]->setRobotVel(a , b , c);

	Aideal.e(0,0) = a; Aideal.e(1,0) = b; Aideal.e(2,0) = c;
	jacobian(a , b , c , Acomm.e(0,0) , Acomm.e(0,1) , Acomm.e(0,2) , Acomm.e(0,3) , soccer->agents[_id]);


	double vx = soccer->agents[_id]->vel().x , vy = soccer->agents[_id]->vel().y , w = soccer->agents[_id]->angularVel()*2*_PI/(360.0);
	//	double ang=-soccer->agents[_id]->dir().th().radian();
	//	Acapt.e(0,0) = (cos(ang) * vx) - (sin(ang) * vy);
	//	Acapt.e(1,0) = (sin(ang) * vx) + (cos(ang) * vy);
	Acapt.e(2,0) = w;

	cnt++;
	cout << cnt << endl;

	if( cnt <= 100 ){
		epsSum.resize(12,1);
		for( int i=0 ; i<12 ; i++ )
			epsSum.e(i , 0) = 0;
	}
	else if( cnt > 100 && cnt <= 150 ){
		//		cout << Acapt.e(0,0) << " " << Acapt.e(1,0) << " " << w  << " " << ang << endl;
		epsBar.resize(12,1);
		epsBar = kron(I3 , Acomm).pseudoinverse() * (Acapt - Aideal);
		epsSum = epsSum + epsBar;
		//		cout << cnt << endl;
		//		cout << Acapt.e(0,0) << " " << Acapt.e(1,0) << " " << w  << " " << ang << endl;
		//		cout << "Matrix " << " ---- " ;
		//		cout.flush();
		//		epsBar.transpose().print();
		//		cout << endl << endl << endl;
		//		cout.flush();
	}
	else if( cnt == 151 ){
		epsSum.scale(0.02);
		cout << "Average Matrix " << " ---- " ;
		cout.flush();
		epsSum.transpose().print();
		epsSum.transpose();
		cout << endl << endl << endl;
		cout.flush();
	}

	if( cnt == 200 )
		cnt = 0;



	return;

	int id = 1;

	knowledge->plotWidgetCustom[7] = 2.0;
	static bool pekh = false;
	if( pekh )
	{
		soccer->agents[id]->setKick(1023);
	}
	else
		soccer->agents[id]->setKick(0);
	pekh = !pekh;

	//	static CSkillTurn* turn = new CSkillTurn( soccer->agents[id]);
	turn->setAgent(soccer->agents[id]);
	turn->setTurnMode(CSkillTurn::Slow);
	static CSkillGotoPoint* gp = new CSkillGotoPoint( soccer->agents[id]);
	gp->init(Vector2D( 1.5,0),Vector2D(1,0));
	gp->setFinalPos(Vector2D( 1.5,0));
	static bool resid = true;
	if ( !resid )
	{
		gp->execute();
	}
	if ( soccer->agents[id]->pos().dist(Vector2D( 1.5,0)) < 0.03)
		resid = true;
	if (resid)
	{
#if 1
		static double vx , vy = 0;
		static double z = 25;
		static double vw = 0.0;
		z+=0.3;

                /*	vy = sin(z/20.00)*1.5;
 if( z > 1220)
 {
  vy = 0;
  if( z > 1220)
   z = 0;
 }*/
                if( std::fmod(z,100) < 50 )
                {
                        vy += 1.5 * 0.3 / 50.0;
                }
                else
                {
                        vy -= 1.5 * 0.3 / 50.0;
                }
                debug(QString("Z : %1").arg(z), D_ERROR);
                //	if( soccer->agents[2]->pos().x  > 2.0)
                //		vx = -1.0;
                //	else if( soccer->agents[2]->pos().x  < -2.0)
                //		vx = 1.0;

		vx = 0;
		//vx = 2;
		soccer->agents[id]->setRobotAbsVel(vx,vy,0);
		turn->setDirection(Vector2D::polar2vector( 1.0 , ( turn->getDirection().th() + vw)) );
		turn->execute();
		knowledge->plotWidgetCustom[0] = vy;
		knowledge->plotWidgetCustom[4] = wm->our[id]->vel.y;
		knowledge->plotWidgetCustom[5] = wm->our[id]->acc.y;

		//	knowledge->plotWidgetCustom[6] = wm->our[2]->pos.x;
		//	knowledge->plotWidgetCustom[7] = wm->our[2]->pos.y;
#else
		static double vx = 0;
		static double z = 25;
		static double vy = 0.0;
		z+=0.3;

		//	vx = sin(z/20.00)*1.5;
		//	if( z > 1220)
		//	{
		//		vx = 0;
		//		if( z > 1220)
		//			z = 0;
		//	}
		if( std::fmod(z,100) < 50 )
		{
			vy += 3.0 * 0.3 / 50.0;
		}
		else
		{
			vy -= 3.0 * 0.3 / 50.0;
		}
		debug(QString("Z : %1").arg(z), D_ERROR);
		//	if( soccer->agents[2]->pos().x  > 2.0)
		//		vx = -1.0;
		//	else if( soccer->agents[2]->pos().x  < -2.0)
		//		vx = 1.0;




		vx = 0;
		soccer->agents[2]->setRobotAbsVel(vx,vy,0);
		turn->setDirection(Vector2D(0,1));
		turn->execute();

		knowledge->plotWidgetCustom[0] = vy;
		knowledge->plotWidgetCustom[1] = wm->our[2]->vel.y;
		knowledge->plotWidgetCustom[2] = wm->our[2]->acc.y;

		knowledge->plotWidgetCustom[3] = wm->ball->vel.y;
		knowledge->plotWidgetCustom[4] = wm->ball->acc.y;

		//	knowledge->plotWidgetCustom[6] = wm->our[2]->pos.x;
		//	knowledge->plotWidgetCustom[7] = wm->our[2]->pos.y;
#endif
	}
	//	qDebug() << "---> " << vx << " " << vy;
	//	qDebug() << wm->our[3]->calc_acc.x << " " << wm->our[3]->calc_acc.y;
	//	static CSkillGotoPointAvoid* gp = new CSkillGotoPointAvoid(soccer->agents[0]);
	//	Vector2D pos = (wm->ball->pos - wm->field->oppGoal()).norm()*0.1+wm->ball->pos;
	//	gp->setFinalPos( pos);
	//	gp->setFinalDir( wm->field->oppGoal() - wm->ball->pos);
	//	gp->setFinalVel(wm->ball->vel);
	//	gp->execute();


	//	static CSkillKick* k0 = new CSkillKick( soccer->agents[0]);
	//	k0->setTarget( wm->field->oppGoal());
	//	k0->setKickSpeed(15);
	//	k0->setInterceptMode(true);
	//	k0->setTolerance(0.2);
	//	k0->setAvoidPenaltyArea(true);
	//	k0->execute();

	//	static CSkillKick* k1 = new CSkillKick( soccer->agents[1]);
	//	k1->setTarget( wm->field->ourGoal());
	//	k1->setKickSpeed(15);
	//	k1->setInterceptMode(true);
	//	k1->setTolerance(0.2);
	//	k0->setAvoidPenaltyArea(true);
	//	k1->execute();

	//	static int i = 0;
	//	i++;
	//	soccer->agents[0]->setRobotVel(sin(i/300.0)*3.0,0,0);
	//	debug(QString("%1").arg(sin(i/300.0)*3.0),D_ERROR);

	//	static int counter = 0;
	//	static CSkillGotoPointAvoid* gp0 = new CSkillGotoPointAvoid( soccer->agents[6]);
	//	if( counter < 200)
	//	{
	////		static CSkillGotoPointAvoid* gp0 = new CSkillGotoPointAvoid( soccer->agents[6]);
	//		gp0->setTarget( Vector2D(2.1,1.7), Vector2D(0,0));
	//		gp0->execute();
	//	}else if( counter < 400)
	//	{
	////		static CSkillGotoPointAvoid* gp1 = new CSkillGotoPointAvoid( soccer->agents[6]);
	//		gp0->setTarget( Vector2D(-2.1,-1.7), Vector2D(0,0));
	//		gp0->execute();
	//	}else if( counter < 600)
	//	{
	////		static CSkillGotoPointAvoid* gp2 = new CSkillGotoPointAvoid( soccer->agents[6]);
	//		gp0->setTarget( Vector2D(2.1,-1.7), Vector2D(0,0));
	//		gp0->execute();
	//	}else if( counter < 800)
	//	{
	////		static CSkillGotoPointAvoid* gp3 = new CSkillGotoPointAvoid( soccer->agents[6]);
	//		gp0->setTarget( Vector2D(-2.1,1.7), Vector2D(0,0));
	//		gp0->execute();
	//	}
	//	else
	//		counter = 0;
	//	counter++;

	//	draw(wm->field->ourPenaltyRect(), "pink", true);
	//	draw(wm->field->oppPenaltyRect(), "pink", true);
	//	soccer->agents[4]->setRobotVel(1.5,0,0);
	//	debug(QString("vel = %1").arg(wm->our[4]->vel.length()),D_ERROR, "brown");
	//	static QTime timer;
	//	static bool kicked = false, printed = false;
	//	if ( !kicked)
	//	{
	//		timer.restart();
	//		timer.start();
	//		soccer->agents[2]->setChip(31);
	//		kicked = true;
	//		debug("yoy !",D_ERROR);
	//	}

	//	if (kicked)
	//		soccer->agents[2]->setChip(31);
	//	if ( wm->ball->inSight > 0.3 && !printed)
	//	{
	//		int t = timer.elapsed();
	//		debug(QString("delay : %1").arg(t),D_ERROR);
	//		printed = true;
	//		debug("hoy !",D_ERROR);
	//	}


	//draw(Circle2D(Vector2D(0,0),0.60),0,9,"purple",true);

	//	draw(wm->field->getRegion("theirpenaltybottom"), "red");

	//	static CSkillKick* kick = new CSkillKick( soccer->agents[4]);
	//	kick->setTarget( Vector2D(wm->field->oppGoalL().x - 0.2, wm->field->oppGoalL().y));
	//	kick->setChip( true);
	//	kick->setSpin(true);
	//	kick->setSlow( true);
	//	kick->setKickSpeed( soccer->agents[4]->chipDistanceValue(wm->ball->pos.dist( wm->field->oppGoalL()) - 0.6, true));
	//	kick->execute();



	//	static CSkillSpinBack* spin = new CSkillSpinBack( soccer->agents[6]);
	//	Vector2D target = wm->field->oppGoal();
	//	spin->setAgent(soccer->agents[6]);
	//	spin->setTakeBack( false);
	//	spin->setTarget( target);
	//	spin->setAnglularVelocity(120.0);
	//	spin->setCorrectAngleTowardTarget(false);
	//	spin->setLinearAcceleration(600.0);
	//	spin->setLinearVelocity(0.0);
	//	spin->setWaitFrames(30);
	//	spin->execute();
	//	static CBehaviourSpinPass* spinPass = new CBehaviourSpinPass();
	//	spinPass->set

	//	draw(Rect2D(Vector2D(2.2,-0.35),wm->field->oppCornerR()), "red",true);
	//	draw(Rect2D(Vector2D(1.3,1.7),Vector2D(2.7,0.3)), "orange",true);
	//	draw(Rect2D(Vector2D(0,0),Vector2D(2,-1.8)), "blue",true);


	//	draw(Rect2D(Vector2D(2.2,wm->field->oppCornerL().y),Vector2D(wm->field->oppGoal().x,0.35)), "red",true);
	//	draw(Rect2D(Vector2D(1.3,-0.2),Vector2D(2.9,-1.7)), "orange",true);
	//	draw(Rect2D(Vector2D(0,1.8),Vector2D(2,0)), "blue",true);

	//    draw(Rect2D(Vector2D(0.9,0),Vector2D(2.2,wm->field->oppCornerR().y)), "red",true);
	//    draw(Rect2D(Vector2D(1.8,wm->field->oppCornerL().y-0.45),Vector2D(wm->field->oppGoal().x-0.1, 0)), "orange",true);
	//    draw(Rect2D(Vector2D(0,1.8),Vector2D(1.5,0.3)), "blue",true);


	//	draw(Rect2D(Vector2D(0.9,wm->field->oppCornerL().y),Vector2D(2.2,0)), "red",true);
	//	draw(Rect2D(Vector2D(1.8,0),Vector2D(wm->field->oppGoal().x-0.1, wm->field->oppCornerR().y+0.45)), "orange",true);
	//	draw(Rect2D(Vector2D(0,-0.3),Vector2D(1.5,-1.8)), "blue",true);

	//    draw(Circle2D(wm->field->oppGoal(), 2.1),0,360,"white");
	//    draw(Circle2D(wm->field->oppGoal(), 0.6),0,360,"white");
	//	draw(wm->field->getRegion(CField::AttackRecCornerBottomB),"red",true);
	//	draw(wm->field->getRegion(CField::AttackRecCornerTopB),"red",true);
	//	draw(wm->field->getRegion(CField::AttackRecMidTopB),"red",true);
	//	draw(wm->field->getRegion(CField::AttackRecMidBottomB),"red",true);

	//    draw(Circle2D(wm->field->oppGoal(), 2.1),0,360,"white");
	//    draw(Circle2D(wm->field->oppGoal(), 0.6),0,360,"white");


	//    double rw;
	//    Vector2D p = knowledge->getEmptyPosOnGoal(knowledge->getMousePos(), rw, false, QList<int> (), QList<int> (), 1.0);
	//    debug(QString("Region Width=%1").arg(rw), D_ERROR);
	//    draw(Segment2D(knowledge->getMousePos(), p), "red");
	//    return;


	//	static CSkillSpinBack* spin = new CSkillSpinBack( soccer->agents[6]);
	//	spin->setTarget(2*soccer->agents[6]->pos()-wm->field->oppGoal());
	//	spin->setWaitFrames(0);
	//	spin->setTakeBack(true);
	//	spin->setAnglularVelocity(0);
	//	spin->setCorrectAngleTowardTarget(false);
	//	spin->setLinearVelocity(0.0);
	//	spin->setLinearAcceleration(0.9);
	//	spin->execute();
	//	draw("Spin 1",Vector2D(0,0),"pink",50);

	//	soccer->agents[6]->setRobotVel(0,0,300);


	//		static CSkillTurn* turn = new CSkillTurn( soccer->agents[6]);
	//		turn->setAgent(soccer->agents[6]);
	//		Vector2D direc = soccer->agents[6]->dir();
	//		direc.rotate(120);
	//		turn->setDirection(direc);
	//		turn->setTurnMode(CSkillTurn::Fast);
	//		turn->execute();

	//    static Navigation nav;



	//  nav.setAgent(soccer->agents[2]);
	//  nav.setTarget(Vector2D(-1.0, 0.0), Vector2D(1.0, 0.0));
	//  nav.setTarget(knowledge->getMousePos(), Vector2D(-1.0, 0.0));
	//  nav.run();
	//  draw(Vector2D(1,1), 1, "purple");
	/*draw(wm->field->getRegion("fieldgrid5bottom"),"blue");
 Vector2D sol1, sol2;
 for (int i=0;i<180;i++)
 {
  intersect_ellipse_line(wm->field->ourGoal() + Vector2D::unitVector(i+90) , wm->field->ourGoal(), wm->field->ourGoal(), 0.9, 1.2, &sol1, &sol2);
  draw(sol1, 1, "red");
  draw(sol2, 1, "red");
 }*/

	//    QList<Vector2D> d;
	//    Vector2D g;
	//    knowledge->generateDefensePositions(2, true, QList<Circle2D>() , d, g);


	//    int id=0;
	//    if(wm->field->fieldRect().contains(wm->our[id]->pos))
	//        soccer->agents[id]->setRobotVel(-0.1780, -0.6020, 4.0900);
	//    else
	//        soccer->agents[id]->setRobotVel(0.0, 0.0, 0.0);
	//    double vfc,vnc,vac;
	//    vfc = (soccer->agents[id]->vel() * soccer->agents[id]->dir().norm());
	//    vnc = (soccer->agents[id]->vel() * soccer->agents[id]->dir().rotatedVector(90).norm());
	//    vac = soccer->agents[id]->self()->angularVel * _DEG2RAD;
	//    char *line,*line2;
	//    line = new char[500];
	//    line2 = new char[500];
	////    sprintf(line,"vel0 = %.2f, f=%.2f, n=%.2f, a=%.2f",(float)soccer->agents[id]->self()->vel.length(),(float)vfc,(float)vnc,(float)vac);
	//    sprintf(line,"%.2f;%.2f;%.2f;%.2f",(float)soccer->agents[id]->self()->vel.length(),(float)vfc,(float)vnc,(float)vac);
	//    sprintf(line2,"%.2f %.2f %.2f %.2f",(float)soccer->agents[id]->v1, (float)soccer->agents[id]->v2, (float)soccer->agents[id]->v3, (float)soccer->agents[id]->v4);
	////    debug(QString("vel0 = %1, f=%2, n=%3, a=%4").arg(soccer->agents[0]->self()->vel.length()).arg(vfc).arg(vnc).arg(vac), D_ARASH, "red");
	//    debug(QString(line)/*+" "+QString(line2)*/, D_ARASH, "red");
	//    soccer->agents[id]->generateEMatrix(-0.1780, -0.6020, 4.0900);
	//    QString S="";
	//    for(int i=0;i<soccer->agents[id]->E.nrows();i++)
	//    {
	//        for(int j=0;j<soccer->agents[id]->E.ncols();j++)
	//        {
	//            S += QString::number(soccer->agents[id]->E.e(i,j),'f',5)+";";
	//        }
	//    }
	//    qDebug()<<S;
	//    delete line,line2;

	//    pf = new CPolynomialFit();

        /*
 pr = new CPolynomialRegression();


 QList< QPair<double, double> > ds;

//    for(int i=0; i<3; i++)
//    {
//        ds.append(qMakePair((double)i,(double)(-2*i*i+i-3)));
//    }
 ds.append(qMakePair((double)0,(double)0));
 ds.append(qMakePair((double)0.01,(double)0));
 ds.append(qMakePair((double)0.02,(double)0));
 ds.append(qMakePair((double)0.03,(double)0));
 ds.append(qMakePair((double)0.04,(double)0));
 ds.append(qMakePair((double)0.05,(double)0));
 ds.append(qMakePair((double)0.06,(double)0));
 ds.append(qMakePair((double)0.071,(double)0));
 ds.append(qMakePair((double)0.081,(double)0.013));
 ds.append(qMakePair((double)0.091,(double)0.049));
 ds.append(qMakePair((double)0.101,(double)0.049));
 ds.append(qMakePair((double)0.111,(double)0.103));
 ds.append(qMakePair((double)0.121,(double)0.103));
 ds.append(qMakePair((double)0.131,(double)0.18));
 ds.append(qMakePair((double)0.141,(double)0.27));
 ds.append(qMakePair((double)0.151,(double)0.27));
 ds.append(qMakePair((double)0.161,(double)0.374));
 ds.append(qMakePair((double)0.171,(double)0.374));
 ds.append(qMakePair((double)0.181,(double)0.494));
 ds.append(qMakePair((double)0.191,(double)0.62));
 ds.append(qMakePair((double)0.201,(double)0.62));
 ds.append(qMakePair((double)0.211,(double)0.757));
 ds.append(qMakePair((double)0.221,(double)0.901));
 ds.append(qMakePair((double)0.231,(double)0.901));
 ds.append(qMakePair((double)0.241,(double)1.048));
 ds.append(qMakePair((double)0.251,(double)1.048));
 ds.append(qMakePair((double)0.261,(double)1.201));
 ds.append(qMakePair((double)0.271,(double)1.355));
 ds.append(qMakePair((double)0.281,(double)1.355));
 ds.append(qMakePair((double)0.292,(double)1.511));
 ds.append(qMakePair((double)0.302,(double)1.668));
 ds.append(qMakePair((double)0.312,(double)1.668));
 ds.append(qMakePair((double)0.322,(double)1.823));
 ds.append(qMakePair((double)0.332,(double)1.823));
 ds.append(qMakePair((double)0.342,(double)1.979));
 ds.append(qMakePair((double)0.352,(double)1.979));
 ds.append(qMakePair((double)0.362,(double)2.134));
 ds.append(qMakePair((double)0.372,(double)2.284));
 ds.append(qMakePair((double)0.382,(double)2.284));
 ds.append(qMakePair((double)0.392,(double)2.435));
 ds.append(qMakePair((double)0.402,(double)2.581));


 pr->fitToDataSet(ds,1);

 QList<double> coefs = pr->getCoefs();
 qDebug()<<"me"<<(float)coefs[0]<<(float)coefs[1];//<<(float)coefs[2];


 delete pr;
//    delete pf;

 int n  = ds.count();

 double *xx = new double[n];
 double *yy = new double[n];
 double a,b,c;

 for (int i = 0; i < n; i++)
 {
  xx[i] = ds[i].first;
  yy[i] = ds[i].second;
//        qDebug()<<QString::number(i)+","+QString::number(xx[i],'f',3)+","+QString::number(yy[i],'f',3);
 }

 squarefit(n, xx, yy, a, b, c);
 qDebug()<<"mani"<<(float)a<<(float)b<<(float)c;

*/
	//    for (int i=-90;i<90;i+=10)
	//    {
	//        draw(intersect_ellipse_dir(Vector2D::unitVector(i), wm->field->ourGoal(), 0.5, (wm->field->ourGoalL()-wm->field->ourGoalR()).length()/2.0,0.2), 1, "red");
	//    }
	//    double w;
	//    Vector2D t = knowledge->goalVisiblity(0, w, false);
	//    draw(t, 1, "red");
	//    debug(QString("w=%1").arg(w), D_ALI, "red");
	//    QList<Circle2D> c;
	//    for (int i=0;i<wm->our.activeAgentsCount();i++)
	//    {
	//        c.append(Circle2D(wm->our.active(i)->pos, CRobot::robot_radius_old));
	//    }
	//    double percent;
	//    double ang = knowledge->getEmptyAngle(wm->ball->pos, wm->field->ourGoalL(), wm->field->ourGoalR(), c, percent);
	//    debug(QString("percent=%1, ang=%2").arg(percent).arg(ang), D_ALI, "blue");
	//    bool f = knowledge->canSendPass(0, 1, soccer->agents[1]->pos(), 4);
	//    if (f)
	//        debug("yeah!", D_ALI, "green");
	//    else
	//        debug("no :(", D_ALI, "red");
	//Vector2D q = knowledge->getReflectPos(mousePos, wm->field->oppGoal(), soccer->agents[0]->pos());
	//draw(Circle2D(q, 0.1), 0, 360, "purple", true );

	//    double w;
	//    knowledge->goalVisiblity(0, w, policy()->PlayMaker_UnderEstimateTheirGoalie());
	//    debug(QString("w=%1").arg(w),D_ALI);
	//    bool flag = ! (knowledge->canSendPass(0, 1, agents[1]->pos(), 1));
	//    if (flag)
	//        debug("pori",D_ALI);
	//    else
	//        debug("khalii",D_ALI);
	// QString("pos=%1").arg(wm->field->ourPAreaPos((mousePos-wm->field->ourGoal()).th().degree())),D_ALI);
	//    Vector2D q,v;
	//    v = wm->field->ourPAreaPerpendicularVector((mousePos-wm->field->ourGoal()).th().degree(), q);
	//    draw(Segment2D(q, q+v), "red");
	//wm->setSimulatorDesiredFPS(10);
	//without graphics every thing will be faster
	//wm->setSimulatorOpenGLState(false);
	//wm->setSimulatorOpenDETimeStep(0.005); //it should be proportional to desired FPS
	//wm->setSimulatorDesiredFPS(200);
	//    if (!wm->field->isInField(wm->ball->pos))
	//    {
	//        wm->ball->setReplace(Vector2D(0,0),Vector2D(0,0));
	//        for (int i=0;i<5;i++)
	//            wm->opp[i]->setReplace(Vector2D(((float)(i+1))*0.5,0),90);
	//    }
#endif
}
#endif

#endif // EXPERIMENTAL1_H

/*
void jacobian(double vx, double vy, double w, double &v1, double &v2, double &v3, double &v4 , CAgent *agent)
{
	// Calculate Motor Speeds
	double motorAlpha[4] = {60.0 * _DEG2RAD, 135.0 * _DEG2RAD, 225.0 * _DEG2RAD, 300 * _DEG2RAD};

	double dw1 =  (1.0 / agent->self()->wheelRadius()) * (( (agent->self()->robotRadius() * w) - (vx * sin(motorAlpha[0])) + (vy * cos(motorAlpha[0]))) );
	double dw2 =  (1.0 / agent->self()->wheelRadius()) * (( (agent->self()->robotRadius() * w) - (vx * sin(motorAlpha[1])) + (vy * cos(motorAlpha[1]))) );
	double dw3 =  (1.0 / agent->self()->wheelRadius()) * (( (agent->self()->robotRadius() * w) - (vx * sin(motorAlpha[2])) + (vy * cos(motorAlpha[2]))) );
	double dw4 =  (1.0 / agent->self()->wheelRadius()) * (( (agent->self()->robotRadius() * w) - (vx * sin(motorAlpha[3])) + (vy * cos(motorAlpha[3]))) );

	float motorMaxRadPerSec = agent->getMotorMaxRadPerSec();

	dw1 = (-dw1 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
	dw2 = (-dw2 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
	dw3 = (-dw3 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
	dw4 = (-dw4 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;

	v1 = (char) (round(dw1));
	v2 = (char) (round(dw2));
	v3 = (char) (round(dw3));
	v4 = (char) (round(dw4));
}

double randomize(int Min, int Max)
{
	qsrand(QTime::currentTime().msec());

	if (Min > Max)
	{
		int Temp = Min;
		Min = Max;
		Max = Temp;
	}
	return ((rand()%(Max-Min+1))+Min);
}
*/

