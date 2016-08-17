#ifndef AUTOBALLPLACEMENT_H
#define AUTOBALLPLACEMENT_H

#include "gotopoint.h"
#include "trackcurve.h"
#include "knowledge.h"
#include "pid.h"

class CSkillAutoBallPlacement : public CSkill
{
private:
    CSkillGotoPointAvoid *GPA;
    void gotoBall();
    void gotoTarget();
    Vector2D ballPos;
    Vector2D agentPos;
public:
    DEF_SKILL(CSkillAutoBallPlacement);


    SkillProperty(CSkillAutoBallPlacement, bool, IsFinished, isFinished);
    SkillProperty(CSkillAutoBallPlacement, Vector2D, Target, target);
};

#endif // AUTOBALLPLACEMENT_H
