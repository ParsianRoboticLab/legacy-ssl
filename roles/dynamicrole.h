#ifndef DYNAMICROLE_H
#define DYNAMICROLE_H

#include "role.h"

class CRoleDynamic
{
public:
    CRoleDynamic();
    ~CRoleDynamic();

    void setUpdated(bool _updated);
    bool getUpdated();


    void execute();

private:
    CSkillKick           *shotSkill;
    CSkillDribble        *dribbleSkill;
    CSkillKeep           *keepSkill;
    CSkillNewPass        *passSkill;
    CSkillReceivePass    *receiveSkill;
    CSkillGotoPointAvoid *moveSkill;
    CSkillKickOneTouch   *oneTouchSkill;
    // CSKill "KEEP"

    void update();
    bool updated;

    ClassProperty(CRoleDynamic, CAgent*, Agent, agent, updated);
    ClassProperty(CRoleDynamic, DynamicEnums::DynamicSkill, SelectedSkill, selectedSkill, updated);
    ClassProperty(CRoleDynamic, Vector2D, Target, target, updated);
    ClassProperty(CRoleDynamic, Vector2D, TargetDir, targetDir, updated);
    ClassProperty(CRoleDynamic, bool, AvoidPenaltyArea, avoidPenaltyArea, updated);
    ClassProperty(CRoleDynamic, double, Tolerance, tolerance, updated);
    ClassProperty(CRoleDynamic, bool, Chip, chip, updated);
    ClassProperty(CRoleDynamic, int, KickSpeed, kickSpeed, updated);
    ClassProperty(CRoleDynamic, float, ReceiveRadius, receiveRadius, updated);
    ClassProperty(CRoleDynamic, Vector2D, WaitPos, waitPos, updated);
    ClassProperty(CRoleDynamic, bool, VeryFine, veryFine, updated);
    ClassProperty(CRoleDynamic, bool, EmptySpot, emptySpot, updated);
    ClassProperty(CRoleDynamic, bool, NoKick, noKick, updated);
    ClassProperty(CRoleDynamic, int, AgentID, agentID, updated);

public:
    CRoleDynamic* setKickRealSpeed(double val) {
        kickSpeed = knowledge -> getProfile(agent->id(), val, !chip, false);
        debug(QString("setkickrealspeed : %1 %2").arg(val).arg(kickSpeed), D_MAHI);
        updated = true;
        return this;
    }


    CRoleDynamic* setAddKickRealSpeed(double val) {
        kickSpeed = knowledge -> getProfile(agent->id(), agent->pos().dist(target) + val, !chip, false);
        updated = true;
        return this;
    }

};

#endif // DYNAMICROLE_H
