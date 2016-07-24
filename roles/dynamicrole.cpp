#include "dynamicrole.h"

CRoleDynamic::CRoleDynamic() {
    shotSkill    = new CSkillKick(NULL);
    keepSkill    = new CSkillKeep(NULL);
    passSkill    = new CSkillNewPass(NULL);
    receiveSkill = new CSkillReceivePass(NULL);
    moveSkill    = new CSkillGotoPointAvoid(NULL);

    selectedSkill = DynamicEnums::NoSkill;
}

CRoleDynamic::~CRoleDynamic() {
    delete shotSkill;
    delete keepSkill;
    delete passSkill;
    delete receiveSkill;
    delete moveSkill;
}

void CRoleDynamic::execute() {

    if(selectedSkill == DynamicEnums::NoSkill) {
        return;
    }
    else if(selectedSkill == DynamicEnums::Ready) {
        if(updated) {
            receiveSkill->setAgent(agent);
            receiveSkill->setTarget(target);
            receiveSkill->setReceiveRadius(receiveRadius);
            receiveSkill->setAvoidOppPenaltyArea(avoidPenaltyArea);
            receiveSkill->setAvoidOurPenaltyArea(avoidPenaltyArea);
            updated = false;
        }
        receiveSkill->execute();
    }
    else if(selectedSkill == DynamicEnums::Pass) {
        if(updated) {
            shotSkill->setAgent(agent);
            shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidPenaltyArea(true);
            shotSkill->setChip(chip);
            shotSkill->setDontKick(noKick);
            shotSkill->setVeryFine(veryFine);
            shotSkill->setShotToEmptySpot(false);
            if(wm->getIsSimulMode()) {
                shotSkill->setKickSpeed(kickSpeed);
            }
            else {
                shotSkill->setKickSpeed(kickSpeed);
            }
            updated = false;
        }
        shotSkill->execute();
    }
    else if(selectedSkill == DynamicEnums::Mark) {
        if(updated) {
            receiveSkill->setAgent(agent);
            receiveSkill->setTarget(target);
            receiveSkill->setReceiveRadius(receiveRadius);
            receiveSkill->setAvoidOppPenaltyArea(avoidPenaltyArea);
            receiveSkill->setAvoidOurPenaltyArea(avoidPenaltyArea);
            updated = false;
        }
        receiveSkill->execute();
    }
    else if(selectedSkill == DynamicEnums::CatchBall) {
        if(updated) {
            shotSkill->setAgent(agent);
            shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidPenaltyArea(avoidPenaltyArea);
            shotSkill->setChip(chip);
            shotSkill->setVeryFine(veryFine);
            shotSkill->setShotToEmptySpot(emptySpot);
            if(wm->getIsSimulMode()) {
                shotSkill->setKickSpeed(kickSpeed);
            }
            else {
                shotSkill->setKickSpeed(kickSpeed);
            }
            updated = false;
        }
        shotSkill->execute();
    }
    else if(selectedSkill == DynamicEnums::Shot) {
        if(updated) {
            shotSkill->setAgent(agent);
            if(emptySpot)
                shotSkill->setTarget(shotSkill->findMostPossible());
            else
                shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidPenaltyArea(true);
            shotSkill->setChip(chip);
            shotSkill->setVeryFine(veryFine);
            shotSkill->setShotToEmptySpot(emptySpot);
            shotSkill->setDontKick(false);
            if(wm->getIsSimulMode()) {
                shotSkill->setKickSpeed(kickSpeed);
            }
            else {
                shotSkill->setKickSpeed(kickSpeed);
            }
            updated = false;
        }
        shotSkill->execute();
    }    
    else if(selectedSkill == DynamicEnums::Chip) {
        if(updated) {
            shotSkill->setAgent(agent);
            shotSkill->setTarget(target);
            shotSkill->setTolerance(tolerance);
            shotSkill->setAvoidPenaltyArea(true);
            shotSkill->setChip(true);
            shotSkill->setVeryFine(false);
            shotSkill->setDontKick(noKick);
            if (wm->getIsSimulMode())
                shotSkill->setKickSpeed(kickSpeed);
            else
                shotSkill->setKickSpeed(kickSpeed);
            updated = false;
        }
        shotSkill->execute();
    }
    else if(selectedSkill == DynamicEnums::Move) {
        if(updated) {

            moveSkill->setAgent(agent);
            moveSkill->setFinalPos(target);
            moveSkill->setFinalDir(targetDir);
            moveSkill->setAvoidPenaltyArea(avoidPenaltyArea);
            moveSkill->setSlowMode(false);
            updated = false;
        }
        moveSkill->execute();
    }

}

