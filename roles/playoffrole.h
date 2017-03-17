#ifndef PLAYOFFROLE_H
#define PLAYOFFROLE_H

#include "role.h"
#include <QTime>
class CRolePlayOff{
private:
    void update();
    bool updated;
    bool roleUpdate;
    bool dont;
    QTime timer;
public:

    CRolePlayOff();
    ~CRolePlayOff();
    void reset();
    void execute();
    int resetTime();
    int getElapsed() const;
    bool deleted;
    CSkillKick *kickSkill;
    CSkillReceivePass *receivePassSkill;
    CSkillKickOneTouch *oneTouchSkill;
    CSkillGotoPointAvoid *gotoPointAvoidSkill;

    ClassProperty(CRolePlayOff, int, AgentID, agentID, updated);
    ClassProperty(CRolePlayOff, CAgent*, Agent, agent, updated);
    ClassProperty(CRolePlayOff, roleSkill::ESkill, SelectedSkill, selectedSkill, updated);
    ClassProperty(CRolePlayOff, Vector2D, Target, target, updated);
    ClassProperty(CRolePlayOff, Vector2D, TargetDir, targetDir, updated);
    ClassProperty(CRolePlayOff, Vector2D, TargetVel, targetVel, updated);
    ClassProperty(CRolePlayOff, bool, AvoidPenaltyArea, avoidPenaltyArea, updated);
    ClassProperty(CRolePlayOff, bool, AvoidCenterCircle, avoidCenterCircle, updated);
    ClassProperty(CRolePlayOff, double, Tolerance, tolerance, updated);
    ClassProperty(CRolePlayOff, bool, Chip, chip, updated);
    ClassProperty(CRolePlayOff, int, KickSpeed, kickSpeed, updated);
    ClassProperty(CRolePlayOff, bool, Slow, slow, updated);
    ClassProperty(CRolePlayOff, float, ReceiveRadius, receiveRadius, updated);
    ClassProperty(CRolePlayOff, Vector2D, WaitPos, waitPos, updated);
    ClassProperty(CRolePlayOff, double, MaxVelocity, maxVelocity, updated);
    ClassProperty(CRolePlayOff, bool, Intercept, intercept, updated);
    ClassProperty(CRolePlayOff, bool, AvoidBall,avoidBall, updated);
    ClassProperty(CRolePlayOff, bool, IgnoreAngle, ignoreAngle, updated);

    ClassProperty(CRolePlayOff, bool, DoPass, doPass, updated);

    ////////Not Executive Property
    ClassProperty(CRolePlayOff, long long, Time, time, dont);
    ClassProperty(CRolePlayOff, bool, BallIsNear, ballIsNear, dont);
    ClassProperty(CRolePlayOff, bool, TimeBased, timeBased, dont);
    ClassProperty(CRolePlayOff, bool, FirstMove, firstMove, dont);


public:
    void setUpdated(bool _updated);
    bool getUpdated();
    inline void setRoleUpdate (bool _updated) {roleUpdate = _updated;}
    inline bool getRoleUpdate () {return roleUpdate;}
protected:
};

#endif // PLAYOFFROLE_H
