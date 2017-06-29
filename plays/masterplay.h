#ifndef MASTERPLAY_H
#define MASTERPLAY_H

#include "formation/edit_data.h"
#include "roles.h"
#include "skills.h"
#include "plans/plans.h"

#define MAX_POSITIONERS 4
#define MAX_WEIGHT_CYCLES 600
#define jalle 0

enum PlaysEnum{
    OurKickOffPlay,
    OurIndirectPlay,
    OurDirectPlay,
    OurPenaltyPlay,
    OurBallPlacement,//added

    TheirKickOffPlay,
    TheirIndirectPlay,
    TheirDirectPlay,
    TheirPenaltyPlay,
    TheirBallPlacement,//added
    StartPlay,
    StopPlay,
    HaltPlay,
    HalfTimeLineUp
};

////////////////////////IO 2016
struct SStaticPlan {
    int agentSize;
    int ballRegion;
    int agentsRegion[5];

    /// '==' Operator
    bool operator ==(const SStaticPlan& _that) {
        if (this->agentSize == _that.agentSize) {
            if(this->ballRegion == _that.ballRegion) {
                for(size_t i = 0;i < 5;i++) {
                    if(this->agentsRegion[i] != _that.agentsRegion[i]) {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
    }

    void clean() {
        ballRegion = -1;
        agentSize = -1;
        for(size_t i = 0;i < 5;i++) {
            agentsRegion[i] = 0;
        }
    }
};

///////////////////////////////////
class CMasterPlay {

public:

    bool playOnFlag;
    bool lockAgents;
    CMasterPlay();
    virtual ~CMasterPlay();
    int getDefenseNum();
    virtual void init(QList <int> _agents , QMap<QString , EditData*> *_editData) = 0;
    virtual void execute_0() = 0;
    virtual void execute_1() = 0;
    virtual void execute_2() = 0;
    virtual void execute_3() = 0;
    virtual void execute_4() = 0;
    virtual void execute_5() = 0;
    virtual void execute_6() = 0;
    void execute();
    int playOffLocation();
    virtual QString whoami() {return "MasterPlay";}
    int choosePlayoffAgent(int ballState);
    bool canScore();
    QList <CAgent *> markAgents;
    //////////////////////////////////////
    bool noPlanException;
    QList<SStaticPlan> staticPlans;
    //////////////////////////////////////

    static PositioningPlan position;
    virtual void reset() = 0;
private:
    void execPlay();
    QString formationName;
    bool staticInited;
    QList< holdingPoints > masterStaticPoints;

protected:

    QMap<QString , EditData*> *editData;
    QList <int> agentsID;

    int executedCycles;
    int defenseN ;
    bool f;

    QList <CAgent *> positionAgents;

    QList <CAgent *> stopAgents;
    CAgent *playMakeAgent;
    CAgent *blockAgent;

    static CRolePlayMake playMakeRole;
    static CMarkPlan markPlan;
    static CRoleBlock blockRole;

    void initMaster();
    void setEditData(QMap<QString , EditData*> *_editData);
    void setAgentsID(QList <int> _agentsID);
    void setFormation(QString _formationName);
    void setStaticPoints(QList< holdingPoints > _staticPoints);
    void resetPositioning();
    void resetPlayMaker();
    void appendRemainingsAgents(QList <CAgent *> &_list);

    void choosePlayMaker();
    void chooseBlocker();
    bool canOneTouch(QList<CAgent*> positionAgents , CAgent *playMake);
    double getOpenness(Vector2D from, Vector2D p1, Vector2D p2, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs);
    double coveredArea( std::priority_queue < QPair< edgeMode , double > , vector< QPair< edgeMode , double > > , Comparar >& obstacles );
};

#define PLAY(playname,Playname) \
    void playname(int symmetry); \
    bool condition##Playname(int symmetry);

#define decideDefOrOff(defPlays,defPlaysCount,offPlays,offPlaysCount) \
    wm->our.activeAgentsCount() - jalle >= wm->opp.activeAgentsCount() ? choosePlay(offPlays,offPlaysCount) : choosePlay(defPlays,defPlaysCount);

#define choosePlay(plays,count) \
    runWithSymmetry((this->*plays[rand()%count]))

#define runWithSymmetry(funcName) \
    wm->ball->pos.y < 0 ? funcName(-1) : funcName(1)

#endif // MASTERPLAY_H
