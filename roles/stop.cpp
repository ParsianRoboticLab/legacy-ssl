#include "stop.h"

INIT_ROLE(CRoleStop, "stop");  


const double StopRadius = 0.55;//0.70;//1.02;//0.78;
CRoleStopInfo::CRoleStopInfo(QString _roleName)
	: CRoleInfo(_roleName)
{
	//	inCorner = -1;
}

void CRoleStopInfo::findPositions()
{
	//	if (inCorner == -1)
	//	{
	//        if ((wm->ball->pos.x < -_FIELD_WIDTH/2.0 + 0.7) && (fabs(wm->ball->pos.y) > _FIELD_HEIGHT/2.0 - 0.5))
	//		{
	//			inCorner = 1;
	//		}
	//		else inCorner = -1;
	//	}
	//	else {
	//		if ((wm->ball->pos.x < -_FIELD_WIDTH/2.0 + 0.6) && (fabs(wm->ball->pos.y) > _FIELD_HEIGHT/2.0 - 0.6))
	//		{
	//			inCorner = 1;
	//		}
	//		else if ((wm->ball->pos.x > -_FIELD_WIDTH/2.0 + 0.8) && (fabs(wm->ball->pos.y) > _FIELD_HEIGHT/2.0 - 0.4))
	//		{
	//			inCorner = 0;
	//		}
	//	}

	//	if (inCorner == 1)
	//		draw("Trapped in corner, now what?", Vector2D(-1.2, 0.0), "red");

	double sRadius = StopRadius;
	Vector2D c = wm->ball->pos;
  const double radius = 1.8 + 2.0*CRobot::robot_radius_new;
  const double MARGIN = 0.01;

	TA = wm->field->ourGoal();
	if((wm->ball->pos - wm->field->ourGoal()).length() < radius )
		TA = wm->field->oppGoal();
	else
		TA = wm->field->ourGoal();

	Vector2D baseDirVec;
	baseDirVec = (TA - c).norm();


	Ps.clear();
	int coef = 1;
	if( count() > 0 ){
		Ps.append( c + baseDirVec * (sRadius+CRobot::robot_radius_new));
	}
	for( int i=1 ; i<count() ; i++ ){
		Ps.append( c + baseDirVec.rotatedVector((coef*((i+1)/2))*_RAD2DEG*0.2/0.6) * (sRadius+CRobot::robot_radius_new));
		coef *= -1;
	}

	thR.clear();
	thP.clear();
	robotId.clear();
	for (int i=0; i<count(); i++)
	{
		thP.append(Vector2D::angleBetween(Ps[i] - c,baseDirVec).degree());
		robotId.append(robot(i)->id());
		thR.append(Vector2D::angleBetween(robot(i)->pos() - c,baseDirVec).degree());
	}

	for( int i = 0; i < thP.size(); i++)
	{
		for( int j = i + 1; j < thP.size(); j++)
		{
			if( thP[i] < thP[j])
			{
				thP.swap(i, j);
				Ps.swap(i, j);
			}
		}
	}
	for( int i = 0; i < thR.size(); i++)
	{
		for( int j = i + 1; j < thR.size(); j++)
		{
			if( thR[i] < thR[j])
			{
				thR.swap(i, j);
				robotId.swap(i, j);
			}
		}
	}

}

CRoleStop::CRoleStop(CAgent *_agent) : CRole(_agent)
{
	gotopoint = new CSkillGotoPointAvoid(_agent);
}

CRoleStop::~CRoleStop()
{
	delete gotopoint;
}

void CRoleStop::execute()
{        

	if ( knowledge->switchState == 0 && switchAgent)
	{
		knowledge->selectedId = agent->id();
		knowledge->switchState = 1;
		knowledge->setSwapDefAndAtt(true);
	}


	Vector2D target;
	info()->findPositions();
	int kkk;
	for (int k=0;k<info()->count();k++)
	{
		if (agent->id()==info()->robotId[k])
		{
			target = info()->Ps[k];
			kkk = k;
		}
	}


	gotopoint->setSlowMode(true);
	gotopoint->setAgent(agent);
	gotopoint->setMaxVelocity(2.0);
	gotopoint->setMaxVelocityNormal(1.0);
	if (wm->ball->inSight<=0 || !wm->ball->pos.valid() || !wm->field->isInField(wm->ball->pos)) {agent->waitHere();return;}
	info()->findPositions();
	gotopoint->setTargetLook(target, wm->ball->pos);
	gotopoint->setAvoidPenaltyArea(true);

    gotopoint->setBallObstacleRadius(0.50);
    gotopoint->setAvoidBall(true);
	gotopoint->execute();
	draw(Circle2D(target , 0.03) , "magenta" , true);
    draw(QString("%1").arg(kkk) , target);
}

void CRoleStop::parse(QStringList params)
{
	switchAgent = false;
	for (int i=0;i<params.length();i++)
	{
		if (params[i].toLower()=="switch") switchAgent = true;
	}
}

double CRoleStop::progress()
{
	return 0.0;
}

CSkillConfigWidget* CRoleStop::generateConfigWidget(QWidget */*parent*/)
{
	return NULL;
}

void CRoleStop::generateFromConfig(CAgent *a)
{
	agent = a;
}

//--------------------------------Halt--------------------------------//
INIT_ROLE(CRoleHalt, "halt");  

CRoleHaltInfo::CRoleHaltInfo(QString _roleName) : CRoleInfo(_roleName)
{

}

CRoleHalt::CRoleHalt(CAgent *_agent) : CRole(_agent)
{

}

CRoleHalt::~CRoleHalt()
{
}

void CRoleHalt::execute()
{
  agent->waitHere();
}

double CRoleHalt::progress()
{
  return 1.0;
}

CSkillConfigWidget* CRoleHalt::generateConfigWidget(QWidget */*parent*/)
{
  return NULL;
}

void CRoleHalt::generateFromConfig(CAgent *a)
{
  agent = a;
}
