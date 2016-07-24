#ifndef OFFENSIVE_H
#define OFFENSIVE_H

#include <behaviours/behaviour.h>
#include <skills.h>

class CBehaviourPass;

class CBehaviourKick : public CBehaviour
{
public:
    DEF_BEHAVIOUR(CBehaviourKick);
    int onetouching;           
    CSkillKick* kick;
    CSkillKickOneTouch* onetouch;
    Property(bool, SlowKick, slowKick);
    Property(bool, ChipToGoal, chipToGoal);
};

class CBehaviourKickBetweenTheirDefenders : public CBehaviourKick
{
public:
    DEF_BEHAVIOUR(CBehaviourKickBetweenTheirDefenders);
};

class CBehaviourPass : public CBehaviour
{
public:    
    DEF_BEHAVIOUR(CBehaviourPass);
    int onepassing;   
    CSkillKick* kick;
    CSkillKickOneTouch* onetouch;
    int passModeDecideFrame;
    int through;
    bool chip;
    double openness(int i, Vector2D& point);
    void opennessThrough(int i, Vector2D& point1, Vector2D& point2, double &w1, double &w2);	
    Property(Vector2D, FixedTarget, fixedTarget);
    Property(bool, AllowOnePass, allowonepass);
    Property(bool, SlowKick, slowKick);
    Property(bool, ChipPass, chipPass);
	Property(bool, NoChip, nochip);
	Property(bool, NoKickProb, nokickprob);
    Property(bool, ShadowPass, shadowPass);
};

class CBehaviourSpinPass : public CBehaviourPass
{
public:
    DEF_BEHAVIOUR(CBehaviourSpinPass);
    CSkillSpinBack* spin;    
};

class CBehaviourChipPass : public CBehaviourPass
{
public:
    DEF_BEHAVIOUR(CBehaviourChipPass);
};

class CBehaviourChipToGoal : public CBehaviourKick
{
public:
    DEF_BEHAVIOUR(CBehaviourChipToGoal);    
};

class CBehaviourPassToDefense : public CBehaviourPass
{
public:
    DEF_BEHAVIOUR(CBehaviourPassToDefense);
};




#endif // OFFENSIVE_H
