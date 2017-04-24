#include "spin.h"
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QValidator>
#include <QLineEdit>
#include <QGridLayout>


INIT_SKILL(CSkillDribble, "dribble");
CSkillDribble::CSkillDribble(CAgent *_agent) : CSkill (_agent)
{
    kick = new CSkillKick(_agent);
    bangBang = new CNewBangBang();
    catchedBall = false;
    agent = _agent;
    catchedCounter = 0;
    readyForPass = false;
    doPass = false;
    kickSpeed = 200;
}

void CSkillDribble::setAgent(CAgent *val)
{
    agent = val;
    kick->setAgent(val);
}

void CSkillDribble::spinDribble()
{
    kick->setChip(chip);
    kick->setTarget(initLook);
    kick->setSlow(false);
    kick->setPassProfiler(false);
    if(catchedBall == false)
    {
        kick->execute();
        kick->setSpin(3);
    }
    else if(!(readyForPass&&doPass))
    {
        double vx,vy,vw;
        agent->setRoller(5);
        bangBang->setAngKp(4.1);
        bangBang->setAngInPath(true);
        bangBang->bangBangSpeed(agentPos,agent->vel(),agentDir,agentPos + (agentDir).norm()*1,target-agentPos,0,0.016,vx,vy,vw);
        agent->setRobotAbsVel(vx,vy,vw);
    }
    else
    {
        kick->setTarget(target);
        kick->execute();
        kick->setSpin(3);
    }

    if(fabs((agentDir.th() - (target- agentPos).th()).degree()) < tol )
        readyForPass = true;
    else
        readyForPass = false;

//    debug(QString("readyForPass : %1").arg(readyForPass),D_MHMMD);
    if(readyForPass && doPass)
        kick->setKickSpeed(kickSpeed);
    else
        kick->setKickSpeed(0);
}

void CSkillDribble::checkState()
{
    if((agentDir.th() - (target- agentPos).th()).degree() < tol )
        readyForPass = true;
    else
        readyForPass = false;
}

void CSkillDribble::execute()
{
    draw(target,0,QColor(Qt::black));
    agentPos = agent->pos();
    agentDir = agent->dir();
    ballPos = wm->ball->pos;
    ballVel = wm->ball->vel;
    dribblerArea.assign(agentPos + agentDir.norm()*0.09,0.08);
    draw(dribblerArea,QColor(Qt::blue));
    if(dribblerArea.contains(ballPos))
    {
        catchedCounter++;
    }
    else
    {
        catchedCounter = 0;
    }

    if(catchedCounter > 30)
    {
        catchedBall = true;
    }
    else
    {
        catchedBall = false;
    }

    ///////////////////////now here we go
    spinDribble();
}

double CSkillDribble::progress()
{
    return 0;
}

CSkillDribble::~CSkillDribble()
{
    delete kick;
    delete bangBang;
}

INIT_SKILL(CSkillSpinBack, "spinback");  

CSkillSpinBack::CSkillSpinBack(CAgent *_agent) : CSkill(_agent)
{
    gotoball = new CSkillGotoBall(_agent);
	kick = new CSkillKick(_agent);
    wentToBall = 0;
    waitFrames = 50;
    vel = -1;
    w = 90;
    acc = 0.0;
    initialtarget.invalidate();
    ready = false;
}

CSkillSpinBack::~CSkillSpinBack()
{
    delete gotoball;
//    delete kick;
}

double CSkillSpinBack::progress()
{
    return 0.0;
}

void CSkillSpinBack::execute()
{
	if ( !agent->self()->isBallOwner(0.1))
		wentToBall = 0;
	if ((wentToBall < waitFrames || (!agent->self()->isBallOwner(0.03))) && (!ready || takeBack))
    {
//		debug("here",D_SEPEHR);
        gotoball->setAgent(agent);
		kick->setAgent(agent);
//		kick->setDontKick(true);
		kick->setKickSpeed(0);
//        kick->setInterceptMode(true);
		kick->setSlow(false);
        if (correct)
		{
//			if (!initialtarget.valid()) gotoball->setGoal(target);
//            else gotoball->setGoal(initialtarget);
			if (!initialtarget.valid()) kick->setTarget(target);
			else kick->setTarget(initialtarget);
        }
//        else gotoball->setGoal(2.0*agent->pos() - target);
		  else kick->setTarget(2.0*agent->pos() - target);
//		gotoball->setSpin(true);
//		gotoball->setSlow(false);
//		gotoball->setUseControler(false);
//        gotoball->execute();
		kick->setInterceptMode(true);
		kick->setClear(false);
		kick->setSpin(true);
		kick->setDontKick(true);
		kick->execute();
		if (wentToBall==0 && agent->self()->isBallOwner(0)) wentToBall = 1;
        if (wentToBall>0) wentToBall ++;
        lastPos = agent->pos();
        lastDir = agent->dir();
        frames = 0;
//        ready = false;
        speed = 2;
        if (takeBack )
            speed = 7;
        incSpeed = 0;
    }
    else if (!ready)
    {
//		debug("There",D_SEPEHR);
        agent->setRoller(speed);
		incSpeed++;
		if( incSpeed > 3 && speed < 7 )
		{
			incSpeed = 0;
			speed++;
		}
		if ( speed == 7 && incSpeed > 12)
            ready = true;
    }
    else {
//		debug("Where ?!",D_SEPEHR);
        frames ++;
        double t = frames * wm->commandSampleTime();
        double psi = Vector2D::angleBetween(target - agent->pos(), agent->dir()).degree();
        if (acc*t > 1.0)
        {
            t = 1.0 / acc;
        }
        double dir = 1;
        if (correct)
        {
            if (psi > 0) dir = -1;
            else dir = 1;
            if (fabs(psi)<5) {dir = 0;wentToBall = 0;}
            takeBack = false;
            draw(Circle2D(target,0.2),0, 360, "cyan");
        }
        if (fabs(Vector2D::angleBetween(lastDir, agent->dir()).degree()) < 90)
        {
            if(!takeBack)
            {
				agent->addRobotVel(-vel - acc*t, 0, dir * w * /*M_PI /*/ 180.0);
                debug(QString("Vel to back: %1").arg(-vel - acc*t), int(D_SEPEHR),"red");
            }
            else {
                agent->setRobotVel(-vel - acc*t, 0, 0);
				debug(QString("Vel to back: %1").arg(-vel - acc*t), D_SEPEHR);
            }
        }
		else if ( !agent->self()->isBallOwner(0.1)){
            wentToBall = 0;
        }
        agent->setRoller(7);
        if (agent->distToBall().length() > 0.5) {
            wentToBall = 0;
            ready = false;
        }
    }    
}
