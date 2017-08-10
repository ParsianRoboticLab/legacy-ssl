#ifndef CHIPKICK_H
#define CHIPKICK_H
#include <knowledge.h>
#include "skills.h"

enum chipKickState{
    initChipKick,
    beforeChip,
    Placement,
    predictState,
    KickSkillSet
};

class chipKick
{
public:
    chipKick();
    void primaryPlacement(double r);
    void stateController();
    double predictChipPos();
    int robotId,refrenceRobotId;
    CSkillGotoPointAvoid *receiver;
    Vector2D pos1,pos2;
    double r , prevVel;
    chipKickState states;
    Polygon2D *up , *down , *left , *right;
    double rect_width;
    QList<Vector2D> ballposss;
    Vector2D v12 ,save;
    double tan , vel;
    bool foundPos,foundVel;
    QList<double> ballVel;
    Vector2D ballVel1;
    double velBallX,velBallX1;
    Vector2D ss;
    CRobot *refrenceRobot;
};
extern chipKick *chipKickRobot;

#endif // CHIPKICK_H
