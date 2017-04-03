#ifndef COLLECTPROFILEDATA_H
#define COLLECTPROFILEDATA_H

#include <QObject>
#include <knowledge.h>
#include "roles.h"
#include <geom/triangle_2d.h>

enum ProfilerState{
    InitState,
    Pos1,
    Pos2,
    Pos3,
    ChangeRobots,
    goOutState,
    StartLow,
    StartHigh,
    SaveProf,
    IsChip,
    endState
};

enum KickerStatus{
    prfl1_Iskicking,
    prfl2_Iskicking,
    ChangeStat
};

class CollectProfileData : public QObject
{
    Q_OBJECT
public:
    CRolePlayOn *rcvr;
    CRolePlayOn *pss;

    CRolePlayOn * prfl1;
    CRolePlayOn * prfl2;

    CRolePlayOn * boroKenar1;
    CRolePlayOn * boroKenar2;

    Vector2D kickerPos;
    CNewProfiler *profiler;

//    Triangle2D downTri;
//    Triangle2D upTri;

    int  realSpeed  , p1 , p2, g1, g2 , counter1 , counter2 , repeat;

    int activeRobots[10],activeRobotsCount;

    double kickSpeed1 , kickSpeed2 , MaxSpeed , MinSpeed , speedStep , middleSpeed , ballSpeed , waitKickSpeed;
//    double xpos2 , ypos2 , xpos1 , ypos1 , gx, gy;
    bool prfl1_Kicked;

    double lowPosX1, lowPosY1, lowPosX2, lowPosY2;

    double highPosX1, highPosY1, highPosX2, highPosY2;

    double chipPosX , chipPosY;
    double chipTargetX , chipTargetY;

    bool isChip , spinOn, ChipPosCalculated ,chipAgain;

    QString filename;

    QTime kickerWait;

    QList<double> p1RealSpeedRec , p2RealSpeedRec;
    QList<int> p1KickSpeed , p2KickSpeed;

    QList<Vector2D> ballposss;
    CollectProfileData();

    void init(int p1 , int p2);
    void goOut();
    void positioning(double xpos1 , double ypos1 , double xpos2 , double ypos2);
    void positioning(double xpos1 , double ypos1);
    void start();
    bool BallIsNear(CRolePlayOn * agent , double rad);
    void LowSpeed1();
    void LowSpeed();
    void HighSpeed();
    void saveMaxBallSpeed();
    void profilerDraws();
    void ChipProfiling();
    void CalcChipPos();

    ProfilerState prfState;
    KickerStatus kickStat;

signals:

public slots:

};
extern CollectProfileData *collectKickProfile;
extern bool ProfilerExecute;

#endif // COLLECTPROFILEDATA_H
