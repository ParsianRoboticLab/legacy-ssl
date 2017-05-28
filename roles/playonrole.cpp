#include "playonrole.h"

CRolePlayOn::CRolePlayOn()
{
    updated = false;
    gotoPointSkill = new CSkillGotoPoint(agent);
    gotoPointAvoidSkill = new CSkillGotoPointAvoid(agent);
    kickSkill = new CSkillKick(agent);
    receivePassSkill =  new CSkillReceivePass(agent);
    oneTouchSkill = new CSkillKickOneTouch(agent);
}

CRolePlayOn::~CRolePlayOn()
{

}

void CRolePlayOn::setAgent(CAgent *_agent)
{
    agent = _agent;
}

void CRolePlayOn::initTask()
{
    updated = false;
    switch(selectedSkill)
    {
        case roleSkill::Gotopoint:
            gotoPointSkill->setAgent(agent);
            gotoPointSkill->setFinalPos(target);
            gotoPointSkill->setFinalDir(targetDir);
            gotoPointSkill->setFinalVel(targetVel);
        break;
        case roleSkill::GotopointAvoid:
            gotoPointAvoidSkill->setAgent(agent);
            gotoPointAvoidSkill->setFinalPos(target);
            gotoPointAvoidSkill->setFinalDir(targetDir);
            gotoPointAvoidSkill->setFinalVel(targetVel);
            gotoPointAvoidSkill->oppRelax(oppRelax);
            gotoPointAvoidSkill->ourRelax(ourRelax);
            gotoPointAvoidSkill->setAvoidCenterCircle(avoidCenterCircle);
            gotoPointAvoidSkill->setAvoidPenaltyArea(avoidPenaltyArea);
            gotoPointAvoidSkill->setMaxVelocity(3.0);
        break;
        case roleSkill::Kick:
            kickSkill->setAgent(agent);
            kickSkill->setTarget(target);
            kickSkill->setTolerance(1);
            kickSkill->setChip(chip);
            kickSkill->setSpin(spin);
            kickSkill->setKickSpeed(1023);
            kickSkill->setPassProfiler(true);
            kickSkill->setAvoidPenaltyArea(avoidPenaltyArea);
            kickSkill->setIsGotoPointAvoid(isGotoPointAvoid);
            //fateme
            kickSkill->setPassProfiler(false);
        break;
        case roleSkill::ReceivePass:
            receivePassSkill->setAgent(agent);
            receivePassSkill->setTarget(waitPos);
            receivePassSkill->setSlow(slow);
            receivePassSkill->setReceiveRadius(receiveRadius);
            receivePassSkill->setAvoidOppPenaltyArea(avoidPenaltyArea);
            receivePassSkill->setAvoidOurPenaltyArea(avoidPenaltyArea);
        break;
        case roleSkill::OneTouch:
            oneTouchSkill->setAgent(agent);
            oneTouchSkill->setWaitPos(waitPos);
            oneTouchSkill->setTarget(target);
            oneTouchSkill->setKickSpeed(1023);
            oneTouchSkill->setChip(chip);
            oneTouchSkill->setAvoidPenaltyArea(avoidPenaltyArea);
        break;
    }
}

void CRolePlayOn::execute()
{
    if(updated)
        initTask();
    if(selectedSkill ==  roleSkill::Gotopoint)
    {
            gotoPointSkill->execute();
    }
    else if(selectedSkill == roleSkill::GotopointAvoid)
    {
            gotoPointAvoidSkill->execute();
    }
    else if(selectedSkill == roleSkill::Kick)
    {
            kickSkill->execute();
            debug(QString("kick speed : %1").arg(kickSkill->getKickSpeed()), D_MAHI);
     }
     else if(selectedSkill == roleSkill::ReceivePass)
     {
            receivePassSkill->execute();
     }
    else if(selectedSkill == roleSkill::OneTouch)
    {
            oneTouchSkill->execute();
    }
    else if(selectedSkill == roleSkill::Mark) {

    }
}
