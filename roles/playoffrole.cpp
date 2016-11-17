#include "playoffrole.h"


CRolePlayOff::CRolePlayOff() {
    gotoPointAvoidSkill = new CSkillGotoPointAvoid(NULL);
    kickSkill = new CSkillKick(NULL);
    oneTouchSkill = new CSkillKickOneTouch(NULL);
    receivePassSkill = new CSkillReceivePass(NULL);
    updated = true;
    roleUpdate = false;
}

CRolePlayOff::~CRolePlayOff() {
    delete gotoPointAvoidSkill;
    delete kickSkill;
    delete oneTouchSkill;
    delete receivePassSkill;
}

void CRolePlayOff::update() {
    switch(selectedSkill) {
    case roleSkill::Gotopoint:

        break;
    case roleSkill::GotopointAvoid:
        gotoPointAvoidSkill->setAgent(agent);
        gotoPointAvoidSkill->init(target, targetDir);
        gotoPointAvoidSkill->setAvoidPenaltyArea(avoidPenaltyArea);
        gotoPointAvoidSkill->setMaxVelocity(maxVelocity);
        gotoPointAvoidSkill->setAvoidBall(avoidBall);
        gotoPointAvoidSkill->setBallObstacleRadius(1);
        updated = false;
        break;
    case roleSkill::Kick:
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
        if(!doPass && !chip) {
            kickSkill->setTarget(Vector2D(1000, 0));
        }
        updated = false;
        break;
    case roleSkill::Mark:
        break;
    case roleSkill::OneTouch:
        oneTouchSkill->setTarget(target);
        oneTouchSkill->setWaitPos(waitPos);
        if (wm->getIsSimulMode()) oneTouchSkill->setKickSpeed(8);
        else oneTouchSkill->setKickSpeed(kickSpeed);
        oneTouchSkill->setAgent(agent);
        updated = false;
        break;
    case roleSkill::ReceivePass:
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
        break;
    default:
        break;
    }
}

void CRolePlayOff::execute() {

    if (updated) {
        update();
    }

    switch (selectedSkill) {
    case roleSkill::Gotopoint:
        break;
    case roleSkill::GotopointAvoid:
        gotoPointAvoidSkill->execute();
        break;
    case roleSkill::Kick:
        kickSkill->execute();
        break;
    case roleSkill::Mark:
        break;
    case roleSkill::OneTouch:
        oneTouchSkill->execute();
        break;
    case roleSkill::ReceivePass:
        receivePassSkill->execute();
        break;
    default:
        break;
    }

}

void CRolePlayOff::setUpdated(bool _updated) {
    updated = _updated;
}
bool CRolePlayOff::getUpdated() {
    return updated;
}
