#include "dynamicrole.h"

CRoleDynamic::CRoleDynamic() {
    shotSkill     = new CSkillKick(NULL);
    keepSkill     = new CSkillKeep(NULL);
    passSkill     = new CSkillNewPass(NULL);
    receiveSkill  = new CSkillReceivePass(NULL);
    moveSkill     = new CSkillGotoPointAvoid(NULL);
    oneTouchSkill = new CSkillKickOneTouch(NULL);
    dribbleSkill  = new CSkillDribble(NULL);
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
    delete dribbleSkill;
}

void CRoleDynamic::update() {
   updated = false;

   shotSkill->setPlayMakeMode(true);
   shotSkill->setKickWithCenterOfDribbler(true);

   switch(selectedSkill) {
   case DynamicEnums::Dribble:
       dribbleSkill->setAgent(agent);
       dribbleSkill->setTarget(targetDir);
             debug(QString("target is : %1 %2").arg(target.x).arg(target.y), D_PARSA);
       dribbleSkill->setInitialLook(target);
       dribbleSkill->setChip(chip);
       dribbleSkill->setDoPass(true);
       dribbleSkill->setKickTol(10);
       dribbleSkill->setKickSpeed(kickSpeed);
       break;
   case DynamicEnums::Ready:
       receiveSkill->setAgent(agent);
       receiveSkill->setTarget(target);
       receiveSkill->setReceiveRadius(receiveRadius);
       receiveSkill->setAvoidOppPenaltyArea(true);
       receiveSkill->setAvoidOurPenaltyArea(true);
       break;
//   case DynamicEnums::Dribble:
//       dribbleSkill->setAgent(agent);
//       dribbleSkill->setTarget(target);
//       dribbleSkill->setInitialLook(wm->field->oppGoal());
//       dribbleSkill->setKickTol(tolerance);
//       dribbleSkill->setChip(chip);
//       if(wm->getIsSimulMode())
//           dribbleSkill->setKickSpeed(kickSpeed);
//       else if (chip) {
//               dribbleSkill->setKickSpeed(max(200, kickSpeed));
//       } else {
//               dribbleSkill->setKickSpeed(max(300, kickSpeed));
//       }
//       break;
   case DynamicEnums::Shot:
       shotSkill->setAgent(agent);
       shotSkill->setTarget(target);
       shotSkill->setTolerance(tolerance);
       shotSkill->setAvoidPenaltyArea(true);
       shotSkill->setChip(chip);
       shotSkill->setVeryFine(veryFine);
       shotSkill->setShotToEmptySpot(emptySpot);
       shotSkill->setDontKick(false);
       if(wm->getIsSimulMode())
           shotSkill->setKickSpeed(kickSpeed / 50);
       else
           shotSkill->setKickSpeed(max(900, kickSpeed));
       break;
   case DynamicEnums::Chip:
       shotSkill->setAgent(agent);
       shotSkill->setTarget(target);
       shotSkill->setTolerance(tolerance);
       shotSkill->setAvoidPenaltyArea(true);
       shotSkill->setChip(true);
       shotSkill->setVeryFine(veryFine);
       shotSkill->setDontKick(false);
       if(wm->getIsSimulMode())
           shotSkill->setKickSpeed(kickSpeed / 100);
       else
           shotSkill->setKickSpeed(max(200, kickSpeed));
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
       if(wm->getIsSimulMode())
           shotSkill->setKickSpeed(kickSpeed / 100);
       else if (chip) {
               shotSkill->setKickSpeed(max(200, kickSpeed));
       } else {
               shotSkill->setKickSpeed(max(300, kickSpeed));
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
       if(wm->getIsSimulMode())
           shotSkill->setKickSpeed(kickSpeed / 50);
       else
           shotSkill->setKickSpeed(1023);
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
        debug(QString("[dynamicRole] kickSpeed : %1").arg(kickSpeed), D_MAHI);
        shotSkill->execute();
        break;
    case DynamicEnums::Dribble:
        draw(Circle2D(Vector2D(0,0), 3),QColor(Qt::cyan));
        dribbleSkill->execute();
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

