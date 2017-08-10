#include "playoffrole.h"


CRolePlayOff::CRolePlayOff() {
    deleted = false;
    agent = NULL;
    gotoPointAvoidSkill = new CSkillGotoPointAvoid(NULL);
    kickSkill = new CSkillKick(NULL);
    oneTouchSkill = new CSkillKickOneTouch(NULL);
    receivePassSkill = new CSkillReceivePass(NULL);
    updated = true;
    roleUpdate = false;
    noAvoid = false;
    timer.start();
    agentID = -1;
    lookForward = true;
    spin=0;
}

CRolePlayOff::~CRolePlayOff() {
    delete gotoPointAvoidSkill;
    delete kickSkill;
    delete oneTouchSkill;
    delete receivePassSkill;
}

void CRolePlayOff::reset()
{

    // TODO : JUST FOR TEST ! IF NOTHING GOES WRONG WE'LL W+BE SAFE !
//    delete gotoPointAvoidSkill;
//    delete kickSkill;
//    delete oneTouchSkill;
//    delete receivePassSkill;
//    gotoPointAvoidSkill = new CSkillGotoPointAvoid(NULL);
//    kickSkill = new CSkillKick(NULL);
//    oneTouchSkill = new CSkillKickOneTouch(NULL);
//    receivePassSkill = new CSkillReceivePass(NULL);

    updated = true;
    deleted = false;
    roleUpdate = false;
    timer.start();
    agentID = -1;
    lookForward = true;
    ballIsNear = false;
    noAvoid = false;
    spin=0;
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
        gotoPointAvoidSkill->setNoAvoid(noAvoid);
        gotoPointAvoidSkill->setBallObstacleRadius(1);
        gotoPointAvoidSkill->setAgent(agent);

        updated = false;
        break;
    case roleSkill::Kick:
        kickSkill->setTarget(target);
        kickSkill->setAvoidPenaltyArea(avoidPenaltyArea);
        kickSkill->setInterceptMode(intercept);
        //debug(QString("[playoffRole] profile kickSpeed : %1 %2").arg(agent->id()).arg(knowledge->getProfile(agent->id(), static_cast<double>(kickSpeed)/130.0, !chip, false)), D_MAHI);
        if (wm->getIsSimulMode())
            kickSkill->setKickSpeed(4);
        else
            kickSkill->setKickSpeed(kickSpeed);
        kickSkill->setChip(chip);
        kickSkill->setAgent(agent);
        kickSkill->setDontKick(!doPass);
        kickSkill->setAgent(agent);
        kickSkill->setTolerance(0.5);
        kickSkill->setPassProfiler(false);
        kickSkill->setKickWithCenterOfDribbler(false);
        kickSkill->setSpin(spin);

        if(!doPass && !chip && lookForward) {
            kickSkill->setTarget(Vector2D(1000, 0));
        }
        updated = false;
        break;
    case roleSkill::Mark:
        break;
    case roleSkill::OneTouch:
        oneTouchSkill->setTarget(target);
        oneTouchSkill->setWaitPos(waitPos);
        oneTouchSkill->setAgent(agent);
        oneTouchSkill->setChip(false);
        oneTouchSkill->setShotToEmptySpot(false);
        if (wm->getIsSimulMode())
            oneTouchSkill->setKickSpeed(8);
        else
            oneTouchSkill->setKickSpeed(1023);
        updated = false;
        break;
    case roleSkill::ReceivePass:
//        oneTouchSkill->setTarget(targetDir);
//        oneTouchSkill->setWaitPos(target);
//        oneTouchSkill->setAgent(agent);
//        oneTouchSkill->setChip(false);
//        oneTouchSkill->setShotToEmptySpot(false);
//        if (wm->getIsSimulMode())
//            oneTouchSkill->setKickSpeed(8);
//        else
//            oneTouchSkill->setKickSpeed(1023);
//        updated = false;
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
        debug(QString("[playoffrole] kickEXE : %2").arg(kickSpeed), D_MAHI);
        break;
    case roleSkill::Mark:
        break;
    case roleSkill::OneTouch:
        oneTouchSkill->execute();
        break;
    case roleSkill::ReceivePass:
//        oneTouchSkill->execute();
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

int CRolePlayOff::resetTime() {
    return timer.restart();
}

int CRolePlayOff::getElapsed() const{
    return timer.elapsed();
}
