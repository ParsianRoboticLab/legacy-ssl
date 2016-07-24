#ifndef JOYSTICKHANDY_H
#define JOYSTICKHANDY_H
#include "mainapplication.h"



using namespace std;

void CMainApplication::JsHandy()
{
	static int robotNum = 0;
	static bool changeFlaged = false;
	bool nospeed = knowledge->joystick->getAxes(0)==0 && knowledge->joystick->getAxes(1)==0 && knowledge->joystick->getAxes(2)==0 && knowledge->joystick->getAxes(3)==0;

	if(knowledge->joystick->getAxes(5)==-1 && changeFlaged == false && nospeed)
	{
		robotNum ++;
		changeFlaged = true;
	}
	if(knowledge->joystick->getAxes(5)==1 && changeFlaged == false && nospeed)
	{
		robotNum --;
		changeFlaged = true;
	}

	if(robotNum <0)
		robotNum = 15;
	else if(robotNum >15)
		robotNum =0;
	if(knowledge->joystick->getAxes(5)==0)
	{
		changeFlaged = false;
	}

	soccer->agents[robotNum]->setRobotVel(knowledge->joystick->getAxes(1) / -1, knowledge->joystick->getAxes(0) / -1, knowledge->joystick->getAxes(2) / -0.5);

        draw(QString("Robot Number: %1").arg(robotNum),Vector2D(3,3));
	return;
}

#endif // MERGECAMERASEXPERIMENT_H

