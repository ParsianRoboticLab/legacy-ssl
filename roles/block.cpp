#include "block.h"

INIT_ROLE(CRoleBlock, "block");

CRoleBlock::CRoleBlock(CAgent *_agent) : CRole(_agent)
{
    gotopoint = new CSkillGotoPointAvoid(_agent);
}

CRoleBlock::~CRoleBlock()
{
    delete gotopoint;
}

void CRoleBlock::execute()
{
    gotopoint->setAgent(agent);
	info()->findPos(blockGoal);

	Vector2D dir, p;
	p = agent->oneTouchCheck(info()->blockPosition, &dir);
	if (p.valid())
	{
		gotopoint->init(p ,dir);
		gotopoint->execute();
	}
	else
	{
		gotopoint->init(info()->blockPosition ,dir);
		gotopoint->execute();
	}
}

double CRoleBlock::progress()
{
    return 0.0;
}

void CRoleBlock::parse(QStringList params)
{
    setStop(false);
    setBlockGoal(false);
    for (int i=0;i<params.count();i++)
    {
        if (params[i].trimmed().toLower()=="stop") setStop(true);
        else if (params[i].trimmed().toLower()=="goal") setBlockGoal(true);
    }
}

CRoleBlockInfo::CRoleBlockInfo(QString _roleName) : CRoleInfo(_roleName)
{

}

void CRoleBlockInfo::findPos(bool blockGoal)
{
	double blockDist = 0.5;

        if (wm->ball->pos.x > 1) {
            blockDist = 2.95;
        }

	if ( blockGoal ){
		blockPosition = ( wm->field->ourGoal() - wm->ball->pos).norm()*( blockDist + CRobot::robot_radius_old) + wm->ball->pos;
	}
	else{
		Vector2D goalPos = ( wm->field->ourGoal() - wm->ball->pos).norm()*( blockDist + CRobot::robot_radius_old) + wm->ball->pos;
        Vector2D pos;
		int kicker = knowledge->getNearestOppToPoint( wm->ball->pos );
		if (kicker == -1){
			blockPosition = goalPos;
		}
		else{
			pos = ( wm->ball->pos - wm->opp[kicker]->pos).norm()*( blockDist + CRobot::robot_radius_old) + wm->ball->pos;

            Rect2D checkRect = Rect2D( wm->field->fieldRect().left() + CRobot::robot_radius_old, wm->field->fieldRect().top() - CRobot::robot_radius_old , _FIELD_WIDTH- 2 *CRobot::robot_radius_old, _FIELD_HEIGHT-2*CRobot::robot_radius_old);
            if ( Vector2D::angleBetween( ( wm->ball->pos - wm->opp[kicker]->pos) , ( wm->field->ourGoal() - wm->ball->pos)).abs() < 30 && checkRect.contains( pos))
                blockPosition = pos;                
            else
                blockPosition = goalPos;
            if ( knowledge->getGameState()==CKnowledge::TheirKickOff)
            {
                if (blockPosition.x > -CRobot::robot_radius_old)
                {
                    if (blockPosition.y > 0)
                    {
                        blockPosition.x = -CRobot::robot_radius_old;
						blockPosition.y = blockDist + CRobot::robot_radius_old;
                    }
                    else
                    {
                        blockPosition.x = -CRobot::robot_radius_old;
						blockPosition.y = -1*blockDist - CRobot::robot_radius_old;
                    }
                }
            }
			if (wm->field->isInOurPenaltyArea(blockPosition) || wm->field->isInOurPenaltyArea(wm->ball->pos))
				blockPosition.x += 1.2;
        }
    }
}
