#include "playoffrole.h"


CRolePlayOff::CRolePlayOff() {
    deleted = false;
    gotoPointAvoidSkill = new CSkillGotoPointAvoid(NULL);
    kickSkill = new CSkillKick(NULL);
    oneTouchSkill = new CSkillKickOneTouch(NULL);
    receivePassSkill = new CSkillReceivePass(NULL);
    updated = true;
    roleUpdate = false;
    timer.start();
    agentID = -1;
    lookForward = true;
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
        gotoPointAvoidSkill->setAgent(agent);

        updated = false;
        break;
    case roleSkill::Kick:
        int tempK;
        kickSkill->setTarget(target);
        kickSkill->setAvoidPenaltyArea(avoidPenaltyArea);
        kickSkill->setInterceptMode(intercept);
        debug(QString("[playoffRole] kickSpeed : %1").arg(kickSpeed), D_MAHI);
//        if (kickSpeed > 10) {
//            tempK = knowledge->getProfile(agent->id(), static_cast<double>(kickSpeed)/100.0, !chip, false);
//        }
        debug(QString("[playoffRole] profiler kickSpeed : %1").arg(tempK), D_MAHI);

        //debug(QString("[playoffRole] profile kickSpeed : %1 %2").arg(agent->id()).arg(knowledge->getProfile(agent->id(), static_cast<double>(kickSpeed)/130.0, !chip, false)), D_MAHI);
        kickSkill->setKickSpeed(kickSpeed);

        kickSkill->setChip(chip);
        kickSkill->setAgent(agent);
        kickSkill->setDontKick(!doPass);
        kickSkill->setAgent(agent);

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
        oneTouchSkill->setShotToEmptySpot(true);
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
        receivePassSkill->setAgent(agent);
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

//    debug(QString("%1 %2 %3 %4 %5").arg(this->agent->id())
//          .arg(selectedSkill)
//          .arg(this->target.x)
//          .arg(this->target.y)
//          .arg(this->updated), D_HOSSEIN);

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

int CRolePlayOff::resetTime() {
    return timer.restart();
}

int CRolePlayOff::getElapsed() const{
    return timer.elapsed();
}
