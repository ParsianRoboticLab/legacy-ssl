#ifndef SPIN_H
#define SPIN_H

#include <gotoball.h>
#include <kick.h>

class CSkillSpinBack : public CSkill
{
protected:
    CSkillGotoBall* gotoball;
	CSkillKick* kick;
    int wentToBall;
    int frames;
    bool ready;
    int incSpeed;
    int speed;
    Vector2D lastPos;
    Vector2D lastDir;
public:
    DEF_SKILL(CSkillSpinBack);
    CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    void generateFromConfig(CAgent *a);
    SkillProperty(CSkillSpinBack, Vector2D, Target, target);
    SkillProperty(CSkillSpinBack, Vector2D, InitialTarget, initialtarget);
    SkillProperty(CSkillSpinBack, bool, CorrectAngleTowardTarget, correct);
    SkillProperty(CSkillSpinBack, int, WaitFrames, waitFrames);
    SkillProperty(CSkillSpinBack, double, LinearAcceleration, acc);
    SkillProperty(CSkillSpinBack, double, LinearVelocity, vel);
    SkillProperty(CSkillSpinBack, double, AnglularVelocity, w);
    SkillProperty(CSkillSpinBack, bool, TakeBack, takeBack);
};


class CSkillDribble : public CSkill
{
protected:
    CSkillKick* kick;
    CNewBangBang *bangBang;

    bool catchedBall;
    Vector2D lastPos;
    Vector2D lastDir;
    int CatchedCounter;
    Circle2D dribblerArea;
public:
    DEF_SKILL(CSkillDribble);
    //CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    //void generateFromConfig(CAgent *a);
    SkillProperty(CSkillDribble, Vector2D, Target, target);
//    SkillProperty(CSkillSpinBack, Vector2D, InitialTarget, initialtarget);
//    SkillProperty(CSkillSpinBack, bool, CorrectAngleTowardTarget, correct);
//    SkillProperty(CSkillSpinBack, int, WaitFrames, waitFrames);
//    SkillProperty(CSkillSpinBack, double, LinearAcceleration, acc);
//    SkillProperty(CSkillSpinBack, double, LinearVelocity, vel);
//    SkillProperty(CSkillSpinBack, double, AnglularVelocity, w);
//    SkillProperty(CSkillSpinBack, bool, TakeBack, takeBack);
};

#endif // SPIN_H
