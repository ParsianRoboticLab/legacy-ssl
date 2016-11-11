#include "playoffrole.h"


CRolePlayOff::CRolePlayOff() {
    gotoPointAvoidSkill = new CSkillGotoPointAvoid(NULL);
    kickSkill = new CSkillKick(NULL);
    oneTouchSkill = new CSkillKickOneTouch(NULL);
    receivePassSkill = new CSkillReceivePass(NULL);
    updated = true;
}

CRolePlayOff::~CRolePlayOff() {
    delete gotoPointAvoidSkill;
    delete kickSkill;
    delete oneTouchSkill;
    delete receivePassSkill;
}

void CRolePlayOff::execute() {
    if(selectedSkill == SkillGotopointAvoid) {
        if(updated) {
            gotoPointAvoidSkill->setAgent(agent);
            gotoPointAvoidSkill->init(target, targetDir);
            gotoPointAvoidSkill->setAvoidPenaltyArea(avoidPenaltyArea);
            gotoPointAvoidSkill->setMaxVelocity(maxVelocity);
            gotoPointAvoidSkill->setAvoidBall(avoidBall);
            gotoPointAvoidSkill->setBallObstacleRadius(1);
            updated = false;
        }
        gotoPointAvoidSkill->execute();
    }
    if(selectedSkill == SkillKick) {
        if(updated) {
            kickSkill->setTarget(target);
            kickSkill->setAvoidPenaltyArea(avoidPenaltyArea);
            kickSkill->setInterceptMode(intercept);
            if(wm->getIsSimulMode()) {
                kickSkill->setKickSpeed(static_cast<int>(kickSpeed/170));
            }
            else {
                kickSkill->setKickSpeed(kickSpeed);
            }
            kickSkill->setChip(chip);
            kickSkill->setAgent(agent);
            kickSkill->setDontKick(!doPass);
            //Keep forward direction 'till correct moment to pass
            if(!doPass && !chip) {
                kickSkill->setTarget(Vector2D(1000, 0));
            }
            /////////////////////////////////////////////////////
            updated = false;
        }
        kickSkill->execute();
    }
    if(selectedSkill == SkillReceivePass) {
        if(updated) {
            receivePassSkill->setTarget(target);
            receivePassSkill->setAvoidOppPenaltyArea(avoidPenaltyArea);
            receivePassSkill->setReceiveRadius(receiveRadius);
            receivePassSkill->setAgent(agent);
            if(ignoreAngle)
            {
                receivePassSkill->setIATargetDir(targetDir);
                receivePassSkill->setIgnoreAngle(false);
            }
            updated = false;
        }
        receivePassSkill->execute();
    }
    if(selectedSkill == SkillOneTouch) {
        if(updated) {
            oneTouchSkill->setTarget(target);
            oneTouchSkill->setWaitPos(waitPos);
            if (wm->getIsSimulMode()) oneTouchSkill->setKickSpeed(8);
            else oneTouchSkill->setKickSpeed(kickSpeed);
            oneTouchSkill->setAgent(agent);
            updated = false;
        }
        oneTouchSkill->execute();
    }
    if(selectedSkill == SkillMark) {
        kkMarkAgents.append(agent->id());
    }
}

void CRolePlayOff::setUpdated(bool _updated) {
    updated = _updated;
}
bool CRolePlayOff::getUpdated() {
    return updated;
}
