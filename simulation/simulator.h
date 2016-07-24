#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "base.h"
#include "agent.h"
#include "worldmodel.h"
#include "knowledge.h"
#include "skills.h"

struct CWorldData
{
    CRobot* our[_MAX_NUM_PLAYERS],opp[_MAX_NUM_PLAYERS];
    CBall* ball;
};

class CSimulator
{
private:
    CWorldModel *wm0, *wm;
	CSkillGotoPoint *gotopoint;
    double angTol, posTol;
public:
    CSimulator();
    void step(double dt);
    void load(); //loads wm with real worldmodel
    void startSimulation();
    void doneSimulation();
    void velocityTransferFunction(Vector2D v_desired, double w_desired, Vector2D v_current, double w_current, double dt, Vector2D& v_actual, double &w_actual);
    double timeNeededForGotoPoint(Vector2D target, Vector2D targetDir, double posTolerance, double angTolerance, int agent, double dt=0.02, int maxIters=10000);
    bool isReached(Vector2D target, Vector2D targetDir, int agent); //uses tolerances set in timeNeededForGotoPoint
};

extern CSimulator* simulator;

#endif // SIMULATOR_H
