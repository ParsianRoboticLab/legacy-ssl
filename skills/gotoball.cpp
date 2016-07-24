#include "gotoball.h"
#include "knowledge.h"
#include <QDebug>


INIT_SKILL(CSkillGotoBall, "gotoball");

CSkillGotoBall::CSkillGotoBall(CAgent *_agent) : CSkill(_agent)
{
	//    CCurveSpline* c = new CCurveSpline();
	gotopointavoid = new CSkillGotoPointAvoid(agent);
	//    trackcurve = new CSkillTrackCurve(agent);
	//    followpoints = new CSkillFollowPoints(agent);
	gotopoint = new CSkillGotoPoint(agent);
	pidP = new CPID();
	pidW = new CPID();
	//    trackcurve->setCurve(c);
	//    c->last_t = 1.0;
	goal.invalidate();
	//    rollingBallOnly = false;
	//    useCurve = false;
	//    usePoints = false;
	//    useDynamicPoints = false;
	useControler = true;
	//    tangentToCircle.invalidate();
	spin = 3;
	//    wasFollowing = false;
	//    aroundBallVel = 0.4; ///TODO: optimize this value
	ballmargin = 0.00;
	finalVel = 0.0;
	stepback = 0.0;
	chip = false;
	//    sz = 0;
	//    ar.last_t = 0.0;
	//    calculated = false;
	clear = false;
	turn = false;
	block = false;
	reached = false;
	pidP->reset();
	pidW->reset();
	noKick = true;
	GTPM = false;
	throughC = new CSkillThroughModeController(agent);
	throughMode = false;
	struggle = false;
	avoidPenaltyArea = false;
	rot = false;
	penaltyMode = false;
        rotate = false;
}

CSkillGotoBall::~CSkillGotoBall()
{
	delete gotopointavoid;
	//    delete trackcurve;
	//    delete followpoints;
	delete gotopoint;
	delete pidP;
	delete pidW;
	delete throughC;
}

void CSkillGotoBall::resetI()
{

}

Vector2D CSkillGotoBall::behindTheBall(double dist)
{
	return wm->ball->pos - behindTheBallDir() * (agent->self()->centerFromKicker() + CBall::radius + dist + /*stepback*/ - 0.002);
}

Vector2D CSkillGotoBall::behindTheBallDir()
{
	return (goal - wm->ball->pos).norm();
}

CSkillGotoBall* CSkillGotoBall::setGoal(Vector2D val)
{
	goal = val;
	gotopointavoid->setAgent(agent);
	gotopointavoid->init(behindTheBall(), behindTheBallDir());
	return this;
}

void CSkillGotoBall::execute()
{
	draw(Circle2D(goal, 0.05), 0, 360, "yellow", true);
	bool no_intercept = false;
	if ((agent->pos()-wm->field->ourGoal()).length()<1.0)
		no_intercept = true;
	bool spin0 = false;
	if (wm->ball->inSight < 0) {agent->waitHere();return;}
	gotopointavoid->setAgent(agent);
	throughC->setAgent(agent);
	catchingBall = false;
	gotopointavoid->setFastW(true);
	if(throughMode)
	{
		debug("Through Mode !", D_ERROR, "red");
		throughC->execute();
	}else
	{
		if (agent->self()->ballComingSpeed()>0.3)
		{
			Vector2D catchPoint = wm->ballCatchTarget(agent->self());
			//            if ((wm->ball->pos - agent->pos()).length() > 0.5)
			//                catchPoint += wm->ball->vel / 40.0;
			catchPoint = catchPoint - (wm->ball->pos-agent->self()->pos)*(CRobot::center_from_kicker_old+CBall::radius/1.0);
			if (!wm->field->fieldRect().contains(catchPoint))
			{
				Vector2D sol1, sol2;
				int s = wm->field->fieldRect().intersection(Segment2D(catchPoint, catchPoint - wm->ball->vel.norm()*10.0), &sol1, &sol2);
				if (s==2)
				{
					if ((sol2-catchPoint).length()<(sol1-catchPoint).length())
						sol1 = sol2;
				}
				else if (s==0) sol1 = catchPoint;
				catchPoint = sol1;
			}

			draw(catchPoint, 1, "pink");
			gotopointavoid->setFastW(false);
			gotopointavoid->setInterceptMode(true);
			gotopointavoid->setBallMode(false);
			gotopointavoid->setAgent( agent);
			gotopointavoid->setTargetLook( catchPoint, wm->ball->pos);
			gotopointavoid->setKeepLooking(true);
			gotopointavoid->execute();
			if (agent->self()->ballComingSpeed()>0.3)
				catchingBall = true;
		}
		else
		{
			gotopointavoid->setInterceptMode(false);
			goBehindTheBall(Vector2D(0,0));
		}
	}
	if ((spin || spin0) && (agent->distToBall().length() < 0.5) && (fabs(Vector2D::angleBetween(agent->dir(), wm->ball->pos - agent->pos()).degree())<60.0 ) && (agent->self()->isBallOwner(0.06)) )
	{
		agent->setRoller(3);
	}
	else
		agent->setRoller(0);
}


void CSkillGotoBall::goBehindTheBall(Vector2D bias)
{
	//    debug(QString("%1").arg(useRRT), D_);

	if ( struggle)
	{
		debug("Sag Mode !", D_ERROR, "red");
		double w;
		QList<int> ourRel, oppRel;
		ourRel.clear();
		ourRel.append(agent->self()->id);
		oppRel.clear();
		gotopointavoid->setAgent(agent);
		gotopointavoid->setBallMode(true);
		gotopointavoid->init(behindTheBall(0), knowledge->getEmptyPosOnGoal(wm->ball->pos, w, true, ourRel, oppRel, 0.6) - gotopointavoid->getFinalPos());
		//		gotopointavoid->setTargetLook(behindTheBall(0), knowledge->getEmptyPosOnGoal(wm->ball->pos, w, true, ourRel, oppRel, 0.6));
		gotopointavoid->execute();


		debug(QString("X = %1 Y =%2").arg(gotopointavoid->getFinalPos().x).arg(gotopointavoid->getFinalPos().y), D_ERROR);

		turnskill.setAgent(agent);
		turnskill.setTurnMode(CSkillTurn::Intercept);
		//		turnskill.setDirection( wm->field->oppGoal());
		turnskill.setDirection(/*knowledge->getEmptyPosOnGoal(wm->ball->pos, w, true, ourRel, oppRel, 0.6) - */wm->ball->pos);
		turnskill.execute();
		debug(QString("vx = %1 vy=%2 w=%3").arg(agent->vforward).arg(agent->vnormal).arg(agent->vangular), D_ERROR);

		return;

	}
	if ( clear)
	{
		debug("Clear !", D_ERROR, "red");
		double w;
		QList<int> ourRel, oppRel;
		ourRel.clear();
		ourRel.append(agent->self()->id);
		oppRel.clear();
		gotopointavoid->setAgent(agent);
		gotopointavoid->setBallMode(true);
		Vector2D targ = behindTheBall(0);
		Vector2D goalTarg = knowledge->getEmptyPosOnGoal(wm->ball->pos, w, true, ourRel, oppRel, 0.6);
		debug(QString("targ=%1 %2").arg(targ.x).arg(targ.y), D_ERROR);
		gotopointavoid->init(targ, goalTarg - targ);
		//		gotopointavoid->setTargetLook(behindTheBall(0), knowledge->getEmptyPosOnGoal(wm->ball->pos, w, true, ourRel, oppRel, 0.6));
		gotopointavoid->execute();


		debug(QString("X = %1 Y =%2").arg(gotopointavoid->getFinalPos().x).arg(gotopointavoid->getFinalPos().y), D_ERROR);

		turnskill.setAgent(agent);
		turnskill.setTurnMode(CSkillTurn::Intercept);
		//		turnskill.setDirection( wm->field->oppGoal());
		turnskill.setDirection(goalTarg - wm->ball->pos);
		turnskill.execute();
		debug(QString("vx = %1 vy=%2 w=%3").arg(agent->vforward).arg(agent->vnormal).arg(agent->vangular), D_ERROR);

		return;

	}

	//    useDynamicPoints = false;
	useControler = true;
	if( useControler )
	{

		double BVC=(wm->ball->vel.length()/3.0)+1;//ball vel coef 0 1; 3 2;
		double distH1 = 0.5*BVC;
		double distH2 = 0.3*BVC;
		double distT = 0.10*BVC;
		double degH = 10*BVC;

		//        if((wm->ball->pos - agent->pos()).length() > 0.6) GTPM = true;
        if( rotate)
        {
            double ang = (agent->pos() - wm->ball->pos).dir().degree();
            int s = 1;
            Vector2D dir1(1,0),dir2(1,0);
            dir1.rotate(ang+6);
            dir2.rotate(ang - 6);
            if( fabs(Vector2D::angleBetween( dir1 , goal - wm->ball->pos).degree()) > fabs(Vector2D::angleBetween( dir2 , goal - wm->ball->pos).degree())  )
                s = 1;
            else
                s = -1;
            Circle2D cir( wm->ball->pos, 0.24);
            Vector2D dire ( 1 , 0), targTemp;
            //charkhidan
            int inc = min(fabs(Vector2D::angleBetween( wm->ball->pos - agent->pos() , goal - wm->ball->pos).degree()),75);
            ang+= s*inc;
            dire.rotate( ang);
            Segment2D seg( wm->ball->pos, wm->ball->pos+(dire.norm()*0.5));
            draw( seg, "orange");
            Vector2D s1,s2;
            cir.intersection( seg, &s1, &s2);
            if ( s1.valid())
                targTemp = s1;
            else if ( s2.valid())
                targTemp = s2;
            gotopointavoid->setAgent(agent);
            gotopointavoid->setTargetLook(targTemp, wm->ball->pos);
            gotopointavoid->setBallMode(false);
            gotopointavoid->setAvoidPenaltyArea(true);
            gotopointavoid->execute();

            turnskill.setAgent(agent);
            turnskill.setTurnMode(CSkillTurn::Slow); turnskill.setDirection(wm->ball->pos - targTemp);
            turnskill.execute();

            return;
        }
        else if(slow)
		{
            if(fabs(Vector2D::angleBetween( wm->ball->pos - agent->pos() , goal - wm->ball->pos).degree()) < 6)
                 turnEnough = true;
            else if(fabs(Vector2D::angleBetween( wm->ball->pos - agent->pos() , goal - wm->ball->pos).degree()) > 12)
                turnEnough = false;

            if( !turnEnough)
			{
                double ang = (agent->pos() - wm->ball->pos).dir().degree();
				int s = 1;
				Vector2D dir1(1,0),dir2(1,0);
				dir1.rotate(ang+6);
				dir2.rotate(ang - 6);
				if( fabs(Vector2D::angleBetween( dir1 , goal - wm->ball->pos).degree()) > fabs(Vector2D::angleBetween( dir2 , goal - wm->ball->pos).degree())  )
					s = 1;
				else
					s = -1;
                Circle2D cir( wm->ball->pos, 0.24);
				Vector2D dire ( 1 , 0), targTemp;
				//charkhidan
                int inc = min(fabs(Vector2D::angleBetween( wm->ball->pos - agent->pos() , goal - wm->ball->pos).degree()),75);
                ang+= s*inc;
				dire.rotate( ang);
				Segment2D seg( wm->ball->pos, wm->ball->pos+(dire.norm()*0.5));
				draw( seg, "orange");
				Vector2D s1,s2;
				cir.intersection( seg, &s1, &s2);
				if ( s1.valid())
					targTemp = s1;
				else if ( s2.valid())
					targTemp = s2;              
				gotopointavoid->setAgent(agent);
				gotopointavoid->setTargetLook(targTemp, wm->ball->pos);
				gotopointavoid->setBallMode(false);
				gotopointavoid->setAvoidPenaltyArea(true);
				gotopointavoid->execute();

				turnskill.setAgent(agent);
				turnskill.setTurnMode(CSkillTurn::Slow); turnskill.setDirection(wm->ball->pos - targTemp);
				turnskill.execute();

				return;
			}
			if((wm->ball->pos - agent->pos()).length() < distH2 && fabs(Vector2D::angleBetween( wm->ball->pos - agent->pos() , goal - agent->pos()).degree()) < degH  )GTPM = false;
			else if ((wm->ball->pos - agent->pos()).length() > distH1 || agent->self()->isBallBack(goal)) GTPM = true;
		}
		else
		{
			/*if((wm->ball->pos - agent->pos()).length() < 0.3 && fabs(( wm->ball->pos - agent->pos()).th().degree() - (goal - wm->ball->pos).th().degree()) < 10.0  || agent->self()->isBallOwner(0.1, CRobot::robot_radius_old*2.0))GTPM = false;
			 else*/ GTPM = true;
		}
		{
			//            debug("gotopoint avoid MODE");

			double rad = CBall::radius + CRobot::robot_radius_old;
			double dist = (agent->pos() - wm->ball->pos).length();
			double psi = fabs(Vector2D::angleBetween(wm->ball->pos - goal, agent->pos() - wm->ball->pos).degree());
			if (false)// dist < rad*1.2 && psi>150)
			{
				double phi = psi;
				double rp  = rad;
				double vel = 0.1;
				if ((psi < 25 /*&& (!slow)) && (psi < 20 && (slow))*/))
				{
					if (slow)
					{
						if (psi<8)
						{
							phi = psi;
						}
					}
					else {
						if (psi<10) phi = psi;
						else phi = 10;
					}
					if (psi<10)
					{
						if (slow)
							rp = dist - 0.02;
						else
							rp = dist - 0.05;
					}
					else
					{
						if (slow)
							rp = dist - 0.02;
						else
							rp = dist - 0.03;
					}
					if (rp < 0) rp = 0;
					if (dist < CRobot::center_from_kicker_old+CBall::radius+stepback+0.05)
					{
						rp = CRobot::center_from_kicker_old+CBall::radius-0.02 + stepback;
						vel = finalVel;
					}
					else
					{
						vel = 0.3;
					}
				}
				else {
					phi = 30;
					vel = 0.30;
				}
				Vector2D q  = rp*(agent->pos() - wm->ball->pos).norm().rotatedVector(phi)+wm->ball->pos;
				double psi1 = fabs(Vector2D::angleBetween(wm->ball->pos - goal, q - wm->ball->pos).degree());
				if (psi1>psi)
					q = rp*(agent->pos() - wm->ball->pos).norm().rotatedVector(-phi)+wm->ball->pos;
				gotopoint->setAgent(agent);
				if (vel<0.01)
				{
					gotopoint->setConstantVelocity(-1);
					gotopoint->setFinalVel(finalVel * (goal - wm->ball->pos).norm());
				}
				else
				{
					gotopoint->setConstantVelocity(vel/* + velBias.length()*/);
				}
				gotopoint->init(q, Vector2D(1.0, 0.0));
				gotopoint->setFinalVel(Vector2D(0.0, 0.0));
				if (turn && psi<20)
					gotopoint->setLookAt(wm->ball->pos);
				else gotopoint->setLookAt(goal);
				gotopoint->setBiasVel(bias);
				gotopoint->setBallMode(false);
				gotopoint->execute();
			}
			else {
				gotopointavoid->setPenaltyKick(penaltyMode);
				gotopointavoid->setFasterEnd(true);
				gotopointavoid->setAgent(agent);
				if (slow){
					spin = 1;
					gotopointavoid->setMaxVelocity(1.5);
					gotopointavoid->setTargetLook(behindTheBall(distT), wm->ball->pos);
					if ( chip)
						gotopointavoid->setSlowShot(true);
					else
						gotopointavoid->setSlowShot(false);
				} else {
					gotopointavoid->setFinalVel((goal - wm->ball->pos).norm()*1.0);
					gotopointavoid->setDynamicStart(false);
					gotopointavoid->setTargetLook(behindTheBall(0.3), goal);
					gotopointavoid->setSlowShot(false);
				}
				gotopointavoid->setBallMode(true);
				if (avoidPenaltyArea)
				{
					if (wm->field->isInOurPenaltyArea(wm->ballCatchTarget(agent->self())) || (wm->ballCatchTarget(agent->self()).x < -2.0 && fabs( wm->ballCatchTarget(agent->self()).y) < 1.2))
					{
						Vector2D q = (agent->self()->pos-wm->field->ourGoal()).norm()*2.0+wm->field->ourGoal();
						QList<Vector2D> isect = wm->field->ourPAreaIntersect(Segment2D(wm->field->ourGoal(), q));
						if (isect.count()>0)
						{
							gotopointavoid->setTargetLook(isect[0], wm->field->oppGoal());
							gotopointavoid->setBallMode(false);
							gotopointavoid->setSlowShot(false);
						}
					}
				}
				gotopointavoid->setShotTarget( goal);
				gotopointavoid->setKeepLooking(true);
				draw(Circle2D(behindTheBall(distT),distT),0,360,QColor("blue"));
				//            debug(QString("%1 %2").arg(behindTheBall(0.15).x).arg(behindTheBall(0.15).y));
				//            gotopointavoid->setFinalPos(behindTheBall(0.12))->setFinalDir(goal);
				//            gotopointavoid->setFastW(false);
				//            if (knowledge->getGameMode()==CKnowledge::Stop) TEMP
				gotopointavoid->setPlan2(true);
				//            gotopointavoid->setTurningDist(0.5);
				gotopointavoid->execute();
			}
		}
	}
}


double CSkillGotoBall::progress()
{
	if (agent->self()->isBallOwner(0.0) &&
			(fabs(Vector2D::angleBetween(agent->dir(), goal - agent->pos()).degree()) < 20))
		return 1.01;
	return gotopointavoid->progress();
}

