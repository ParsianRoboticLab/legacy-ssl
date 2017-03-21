#ifndef DYNAMICATTACK_H
#define DYNAMICATTACK_H

#include"masterplay.h"

#define SEMIDYNAMIC
#define _MAX_REGION 7

struct SDynamicAgent {

    void init(DynamicEnums::DynamicRole _role,
              DynamicEnums::DynamicSkill _skill,
              DynamicEnums::DynamicRegion _region) {
        role   = _role;
        skill  = _skill;
        region = _region;
    }



    inline Vector2D getTarget() {
        if(region >= 0)
            knowledge->getStaticPoses(region);
    }

    DynamicEnums::DynamicRole role;
    DynamicEnums::DynamicSkill skill;
    DynamicEnums::DynamicRegion region;
};

struct SDynamicPlan {
    int agentSize;
    DynamicEnums::DynamicMode mode;
    SDynamicAgent agents[5];
    Vector2D passPos;
    int passID;
    int positionCnt;
};


class CDynamicAttack : public CMasterPlay {

    typedef CMasterPlay super;

public:


    CDynamicAttack();
    ~CDynamicAttack();

    void execute_0();
    void execute_1();
    void execute_2();
    void execute_3();
    void execute_4();
    void execute_5();
    void execute_6();
    void init(QList <int> _agents ,
              QMap<QString , EditData*> *_editData);

    void setDefenseClear(bool _isDefenseClearing);
    void setDirectShot(bool _directShot);
    void setPositions(QList<int> _positioningRegion);
    void setWeHaveBall(bool _ballPoss);
    void setNoPlanException(bool _noPlanException);
    void setFast(bool _fast);
    void setPlayMake(int _playMake);
    void setCritical(bool _critical);

private:

    void playMake();
    void choosePlayMaker();
    void positioning(int starter,  DynamicEnums::DynamicSkill skills);
    void globalExecute(int agentSize);
    void dynamicPlanner(int agentSize);

    void makePlan(int agentSize);
    void assignId();
    void assignTasks();
    void checkPlanner();
    ///////////////////////30em 2015

    //[RegionCount][RegionIndex]
    Rect2D* guards[_MAX_REGION];
    inline void showRegions(unsigned int agentSize, QColor color = QColor(Qt::gray));
    inline void assignRegions();
    inline void assignRegion_0();
    inline void assignRegion_1();
    inline void assignRegion_2();
    inline void assignRegion_3();
    inline void assignRegion_4();
    inline void assignRegion_5();
    inline void assignRegion_6();
    QList<int> guardIndexList;
    QList<Vector2D> semiDynamicPosition;
    QList<Vector2D> markPositions;

    //[PositionAgentsCount][GuardIndex][LocationIndex]
    Vector2D** guardLocations[_MAX_REGION];
    inline void showLocations(unsigned int agentSize, QColor color = QColor(Qt::gray));
    inline void assignLocations();
    inline void assignLocations_0();
    inline void assignLocations_1();
    inline void assignLocations_2();
    inline void assignLocations_3();
    inline void assignLocations_4();
    inline void assignLocations_5();
    inline void assignLocations_6();
    bool isRightTimeToPass();
    int farGuardFromPoint(const int& _guardIndex, const Vector2D& _point);
    void chooseBestPosForPass();
    void chooseBestPositons();
    void chooseMarkPos();
    double getDynamicValue(const Vector2D& _dynamicPos) const;
    void checkPoints(QList<Vector2D>& _points);

    int minHorizontalDistID(const QList<Vector2D>& _points);
    int maxHorizontalDistID(const QList<Vector2D>& _points);

    ///////////////////
    bool isPathClear(Vector2D _pos1,Vector2D _pos2, double rad,double t);

    inline bool chipOrNot(Vector2D target,
                          double _radius = 1, double _treshold = .5);
    int appropriatePassSpeed();
    int appropriateChipSpeed();

    Vector2D neaerstGuardToPoint(const Vector2D& startVec) const;

    void managePasser();
    bool isPlayMakeChanged();


    void ballLocation();

    QString getString(const DynamicEnums::DynamicMode& _mode) const;    

    CRoleDynamic *roleAgents[5];

    SDynamicPlan currentPlan;

    ////////Plan Making
    bool isDefenseClearing,isWeHaveBall,noPlanException;
    bool directShot,fast,critical;
    ////////////////////

    double shotProb,shotAngle;
    bool passFlag,repeatFlag;
    int counter,passerID,lastPasserRoleIndex;
    long lastTime;
    QList<CAgent*> activeAgents;
    QList<CAgent*> mahiPoisitionAgents;
    QList<Vector2D> dynamicPosition;
    QList<int> regionsList;
    CAgent* mahiPlayMaker;
    int mahiAgentsID[5];
    bool isBallInOurField;

    int playmakeID;

    Vector2D ballPos;
    Vector2D ballVel;
    Vector2D OppGoal;

    bool goToDynamic[5];
    int lastPlayMakerId;


    bool keepOrNot();

    /////////Intentions
//    int intenHighProb;


protected:
    void reset();


};

#endif // DYNAMICATTACK_H
