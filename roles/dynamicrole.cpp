#include "dynamicrole.h"

CRoleDynamic::CRoleDynamic() {
    shotSkill     = new CSkillKick(NULL);
    keepSkill     = new CSkillKeep(NULL);
    passSkill     = new CSkillNewPass(NULL);
    receiveSkill  = new CSkillReceivePass(NULL);
    moveSkill     = new CSkillGotoPointAvoid(NULL);
    oneTouchSkill = new CSkillKickOneTouch(NULL);
    selectedSkill = DynamicEnums::NoSkill;
    agent = NULL;
}

CRoleDynamic::~CRoleDynamic() {
    delete shotSkill;
    delete keepSkill;
    delete passSkill;
    delete receiveSkill;
    delete moveSkill;
    delete oneTouchSkill;
}

void CRoleDynamic::update() {
   updated = false;

   switch(selectedSkill) {
   case DynamicEnums::Ready:
       receiveSkill->setAgent(agent);
       receiveSkill->setTarget(target);
       receiveSkill->setReceiveRadius(receiveRadius);
       receiveSkill->setAvoidOppPenaltyArea(true);
       receiveSkill->setAvoidOurPenaltyArea(true);
       break;
   case DynamicEnums::Shot:
       shotSkill->setAgent(agent);
       shotSkill->setTarget(target);
       shotSkill->setTolerance(tolerance);
       shotSkill->setAvoidPenaltyArea(true);
       shotSkill->setChip(chip);
       shotSkill->setVeryFine(veryFine);
       shotSkill->setShotToEmptySpot(emptySpot);
       shotSkill->setDontKick(false);
       if(wm->getIsSimulMode()) {
           shotSkill->setKickSpeed(kickSpeed/120);
       } else {
           shotSkill->setKickSpeed(kickSpeed);
       }

       break;
   case DynamicEnums::Chip:
       shotSkill->setAgent(agent);
       shotSkill->setTarget(target);
       shotSkill->setTolerance(tolerance);
       shotSkill->setAvoidPenaltyArea(true);
       shotSkill->setChip(true);
       shotSkill->setVeryFine(veryFine);
       shotSkill->setDontKick(false);
       if (wm->getIsSimulMode()) {
           shotSkill->setKickSpeed(kickSpeed/120);
       } else {
           shotSkill->setKickSpeed(kickSpeed);
       }
       break;

   case DynamicEnums::Pass:
       shotSkill->setAgent(agent);
       shotSkill->setTarget(target);
       shotSkill->setTolerance(tolerance);
       shotSkill->setAvoidPenaltyArea(true);
       shotSkill->setChip(chip);
       shotSkill->setDontKick(noKick);
       shotSkill->setVeryFine(veryFine);
       shotSkill->setShotToEmptySpot(false);
       if(wm->getIsSimulMode()) {
           shotSkill->setKickSpeed(kickSpeed/120);
       }
       else {
           shotSkill->setKickSpeed(kickSpeed);
       }
       break;
   case DynamicEnums::CatchBall:
       shotSkill->setAgent(agent);
       shotSkill->setTarget(target);
       shotSkill->setTolerance(tolerance);
       shotSkill->setAvoidPenaltyArea(true);
       shotSkill->setChip(chip);
       shotSkill->setVeryFine(false);
       shotSkill->setShotToEmptySpot(emptySpot);
       if(wm->getIsSimulMode()) {
           shotSkill->setKickSpeed(kickSpeed/120);
       }
       else {
           shotSkill->setKickSpeed(kickSpeed);
       }
       updated = false;
       shotSkill->execute();
       break;
   case DynamicEnums::Move:
       moveSkill->setAgent(agent);
       moveSkill->init(target, targetDir);
       moveSkill->setAvoidPenaltyArea(true);
       moveSkill->setSlowMode(false);
       break;
   case DynamicEnums::OneTouch:
       oneTouchSkill->setAgent(agent);
       oneTouchSkill->setWaitPos(waitPos);
       oneTouchSkill->setTarget(target);
       oneTouchSkill->setAvoidPenaltyArea(avoidPenaltyArea);
       break;
   case DynamicEnums::Keep:
       break;
   case DynamicEnums::NoSkill:
   default:
       break;
   }

}

void CRoleDynamic::execute() {

    if (updated) {
        update();
    }

    switch(selectedSkill) {
    case DynamicEnums::Ready:
        receiveSkill->execute();
        break;
    case DynamicEnums::Shot:
    case DynamicEnums::Chip:
    case DynamicEnums::Pass:
    case DynamicEnums::CatchBall:
        shotSkill->execute();
        break;
    case DynamicEnums::Move:
        moveSkill->execute();
        break;
    case DynamicEnums::OneTouch:
        break;
    case DynamicEnums::Keep:
        keepSkill->execute();
        break;
    case DynamicEnums::NoSkill:
    default:
        break;
    }

}

