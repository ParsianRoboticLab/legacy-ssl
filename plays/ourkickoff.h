#ifndef OURKICKOFF_H
#define OURKICKOFF_H

#include "masterplay.h"

class COurKickOff : public CMasterPlay{
public:
    COurKickOff();
    ~COurKickOff();
    void execute_0();
    void execute_1();
    void execute_2();
    void execute_3();
    void execute_4();
    void execute_5();
    void execute_6();
    void init(QList <int> _agents , QMap<QString , EditData*> *_editData);

    //////////MAHI


    /////////MAHI


private:


    ////////////MAHI


    //////////////MAHI


    int oneToucherID;
    Vector2D pointForPass;
    QList<Vector2D> staticVec[MAX_POSITIONERS];
    QList<int> cyclesToWait[MAX_POSITIONERS];
    QList<double> escapeRadius[MAX_POSITIONERS];
    QList<positioningType> facePoints[MAX_POSITIONERS];
    QList<holdingPoints> staticPoints;
    int waitBeforeExecution;

    void reset();
    void chipToOppGoal();
    void swapKickoff2(int symmetry);
    void kickoffOnlyTwo2(int symmetry);
    void swapKickoff3(int symmetry);
    void (COurKickOff::*lastPlayExeced)(int symmetry);
    void passOrNot();
    void setPosIntentions();
    void addNearestRobot2Mid(Vector2D p);
    void swapNearestDefAndMid(Vector2D p, int midIdx);
    int getAgent(int id);
    int getNearestAgent(Vector2D p);
};
typedef void (COurKickOff::*kickoffPlayFunction)(int symmetry);

#endif // OURKICKOFF_H
