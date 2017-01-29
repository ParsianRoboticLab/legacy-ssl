#ifndef MERGECAMERASEXPERIMENT_H
#define MERGECAMERASEXPERIMENT_H
#include "mainapplication.h"
#include "position.h"


using namespace std;

bool CMainApplication::MergeCamerasExperiment()
{
#ifndef GAME_MODE
	static double x = -1.3, y = -2;
	static CSkillGotoPoint* goToPoint = new CSkillGotoPoint(soccer->agents[0]);
	Vector2D pos(x,y);
	if( y <= 2 )
	{
		goToPoint->setAgent(soccer->agents[0]);
		goToPoint->setMaxVelocity(0.6);
		goToPoint->init(pos,Vector2D(0,0));
		if( soccer->agents[0]->pos().dist(pos) > 0.01 )
			goToPoint->execute();
		else
		{
			x *= -1;
			y += .1;
		}
		return true;
	}
#endif
	return false;
}

#endif // MERGECAMERASEXPERIMENT_H
