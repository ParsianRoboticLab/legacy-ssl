#include "offensive.h"
#include "passevaluation.h"

const double Our_Run_Factor = 3.0;
//const double Opp_Big_Factor = 1.8;

const double Opp_Big_Factor = 2.5;

const double KickOffGoalVisibChip = 0.05;

INIT_BEHAVIOUR(CBehaviourKick, "kick");

CBehaviourKick::CBehaviourKick()
{
	onetouching = -1;
	kick = new CSkillKick(NULL);
	onetouch = new CSkillKickOneTouch(NULL);
	roles.append("playmake");
	slowKick = false;
	chipToGoal = false;
}

CBehaviourKick::~CBehaviourKick()
{
	delete kick;
	delete onetouch;
}

void CBehaviourKick::execute()
{    
	passEval.reset();
	if (agents.count() == 0)
		return;
	else
	{
		double w, ang, coming;
		Vector2D target = knowledge->onetouchablity(agents[0]->id(), w, ang, coming);
		if (knowledge->sentChipPass)
		{
			if (coming<-0.4)
			{
				knowledge->sentChipPass = false;
			}
		}

		if (onetouching == -1 && coming>0.3 && ang<policy()->PlayMaker_OneTouchAngleThreshold())
		{
			onetouching = 0;
			onetouch->setWaitPos(agents[0]->pos());
		}
		if (wm->ball->vel.length() < 0.4) onetouching = -1;
		if ((wm->ball->vel.length() > 0.4) && (wm->ball->vel.length() < 1.0) && wm->ball->modelObjStopPos.valid())
		{
			if ((wm->ball->modelObjStopPos-wm->ball->pos)*(wm->ball->modelObjStopPos-agents[0]->pos()) < 0.0)
			{
				onetouching = -1;
			}
		}
		AngleDeg th = (agents[0]->pos() - wm->ball->pos).th();
		double tol = 20.0;
		Segment2D r1 = Segment2D (agents[0]->pos(), agents[0]->pos() + Vector2D::unitVector(th + AngleDeg(tol)) * 100.0);
		Segment2D r2 = Segment2D (agents[0]->pos(), agents[0]->pos() + Vector2D::unitVector(th - AngleDeg(tol)) * 100.0);
		Segment2D seg = Segment2D(wm->ball->pos, wm->ball->modelObjStopPos);
		if (!wm->ball->modelObjStopPos.valid())
			seg = Segment2D(wm->ball->pos, wm->ball->vel.norm() * 100.0 + wm->ball->pos);
		if (!seg.intersection(r1).valid() && !seg.intersection(r2).valid() && (wm->ball->pos-agents[0]->pos()).length() > 0.5 && wm->ball->vel.length() > 0.3)
			onetouching = -1;
		if (onetouching >= 0)
		{
			draw(QString("1") , agents[0]->pos() , "white");
			onetouch->setAgent(agents[0]);
			onetouch->setChip(false);
			onetouch->setDistToBallLine(0.0);
			onetouch->setVelocityToBallLine(0.0);
			onetouch->setTarget(target);
			draw(QString("2") , target , "white");
			onetouch->setKickSpeed(agents[0]->kickSpeedValue(7.5,false));
			onetouch->setAvoidPenaltyArea(true);
			draw("oneTouch1", Vector2D(-1.0,0), "red" ,20);
			onetouch->execute();
			onetouching ++;
			if ((coming<-0.9) || (onetouching >= 500) || fabs(coming)<0.05)
			{
				onetouching = -1;
			}
		}
		if (onetouching == -1)
		{
			if(slowKick)
			{
				draw("Slow :)", Vector2D(-1.0,0));
				kick->setSlow(true);
				kick->setInterceptMode(false);
			}
			else
			{
				kick->setInterceptMode(true);
				kick->setSlow(false);
			}
			draw("Kick", Vector2D(-1.0,0), "red" ,20);
			kick->setAvoidPenaltyArea(true);
			//            if (knowledge->getGameState() == CKnowledge::Start) kick->setAvoidPenaltyArea(true);
			//            else kick->setAvoidPenaltyArea(false);
			kick->setChip(chipToGoal);
			kick->setAgent(agents[0]);
			if (chipToGoal)
			{
                kick->setKickSpeed(agents[0]->chipDistanceValue(wm->ball->pos.dist(wm->field->oppPenalty()), true));
			}
			else kick->setKickSpeed(agents[0]->kickSpeedValue(7.5,false));
			kick->setTarget(target);
			if( chipToGoal)
				target = (wm->field->oppGoal()+wm->field->oppGoalR())/2;
			kick->setTolerance(0.15);
			kick->execute();
		}
	}
}

double CBehaviourKick::probability()
{
	if (agents.count() == 0)
	{
		return 0;
	}
	if (agents.count() > 0)
	{
		if (knowledge->getGameMode()==CKnowledge::OurKickOff) {
			double rw = 0.0;
			knowledge->goalVisiblity(agents[0]->id(), rw, 1.0);
			if (rw < KickOffGoalVisibChip)
				return 0.1;
		}
	}
	if (knowledge->sentChipPass) return 1.0;
	double goal_p;
	QList<int> ourrelaxed;
	QList<int> opprelaxed;
	ourrelaxed.append(agents[0]->id());
        knowledge->getEmptyPosOnGoal(agents[0]->pos(), goal_p, true, ourrelaxed, opprelaxed, 1.0,
                                                          0.6); ///calculate empty angle     //p*empty_width + (1-p)*goalwidth
	debug(QString("goal Prob : %1 - %2").arg(goal_p).arg(agents[0]->id()),D_SEPEHR);
	double d = (agents[0]->pos() - wm->field->oppGoal()).length() / (wm->field->oppCornerL() - wm->field->ourCornerR()).length();

//	if (!knowledge->isCrowdedInFrontOfAgent(agents[0]->id(), 1.5))
//	{
//		goal_p += 0.5;
//	}
//	if ( (agents[0]->pos().dist(wm->field->oppGoal()) < 1.8) &&
//		 (fabs(Vector2D::angleBetween(wm->field->ourGoal() - wm->field->oppGoal(), agents[0]->pos() - wm->field->oppGoal() ).degree()) < 70 ) )
//	{
//		goal_p += 1.0;
//	}

	if (goal_p > 1.0)
		goal_p = 1.0;
//	return goal_p;
        return goal_p;
            //return (1.0 - d) * 0.1 + goal_p * 0.9;
}


INIT_BEHAVIOUR(CBehaviourPass, "pass");


CBehaviourPass::CBehaviourPass()
{
	allowonepass = false;
	onepassing = -1;
	kick = new CSkillKick(NULL);
	onetouch = new CSkillKickOneTouch(NULL);
	roles.append("playmake");
	slowKick = false;
	chipPass = false;
	chip = false;
	shadowPass = false;
	nochip = false;
	nokickprob = false;
	passModeDecideFrame = -1;
	through = -1;
}

CBehaviourPass::~CBehaviourPass()
{
	delete kick;
	delete onetouch;
}

void CBehaviourPass::execute()
{    
    shadowPass =false;
    if (agents.count() == 0)
        return;
	if (agents.count() == 1 && !fixedTarget.valid()) return;
    //send a pass from agents[0] to agents[1]

	if (agents.count() >= 2)
	{
                if ((wm->ball->pos - agents[0]->pos()).length() < 0.3) knowledge->playmakerSelector.setPassRecvTarget(agents[1]->id());
	}

    if (!fixedTarget.valid()) passEval.createPass(agents[0]->id(), agents[1]->id(), PassEvaluation::Simple); //inform the knowledge that a pass is being sent
    double w, ang, coming, w1, w2;
    Vector2D target;
    Vector2D target1, target2;
    if (!fixedTarget.valid())
    {
        w = openness(1, target);
        opennessThrough(1, target1, target2, w1, w2);
        target = agents[1]->self()->getKickerPos();
        w = 1;
    }
    else {
        target = fixedTarget;
        through = -1;
    }
    chip = false;
    if (!fixedTarget.valid())
    {
        if (fabs(knowledge->frameCount - passModeDecideFrame) > 60)
        {
			if (w < 0.2 || shadowPass)
            {
                if (shadowPass)
                {
					if (w1 < 0.2 && w2<0.2)
                    {
                        chip = true;
                        through = -1;
                    }
                    else {
                        if (w1 > w2)
                        {
                            through = 1;
                        }
                        else {
                            through = 2;
                        }
                    }
                }
                else {
                    chip = true;
                    through = -1;
                }
            }
            else {
                through = -1;
            }
        }
        chip = false;
    }
    else {
        //Check Chip conditions in IndirectSafe
        QList<Circle2D> obs;
        for (int j = 0;j < wm->our.activeAgentsCount();j++)
        {
                if (wm->our.active(j)->id == agents[0]->id()) continue;
                obs.append(Circle2D(wm->our.active(j)->pos, wm->our.active(j)->robotRadius()));
        }
        for (int j = 0;j < wm->opp.activeAgentsCount();j++)
        {
                obs.append(Circle2D(wm->opp.active(j)->pos, Opp_Big_Factor * wm->opp.active(j)->robotRadius()));
        }
        double angle, biggestAngle, prob;
        Vector2D r = fixedTarget-agents[0]->self()->getKickerPos();
        Vector2D n = r.norm().rotatedVector(90.0)*agents[0]->self()->robotRadius()*2.0 * Our_Run_Factor;
        knowledge->getEmptyAngle(agents[0]->self()->getKickerPos(), fixedTarget + n, fixedTarget - n, obs, prob, angle, biggestAngle);
		if (prob < 0.3)
            chip = true;
    }
    /*
        for (int i=0;i<wm->opp.activeAgentsCount();i++)
        {
            if ((fabs(Vector2D::angleBetween( (target-agents[0]->pos()),  wm->opp.active(i)->pos - agents[0]->pos()).degree())) < 15.0)
            {
                chip = true;
            }
        }
        */

	if (agents.count() > 1)
	{
		if (chip)
		{
			draw(Circle2D(agents[1]->pos(), 0.30), "red", false);
			draw(Circle2D(agents[1]->pos(), 0.32), "red", false);
		}
		else {
			draw(Circle2D(agents[1]->pos(), 0.30), "blue", false);
			draw(Circle2D(agents[1]->pos(), 0.32), "blue", false);
		}
	}
    /*
	if (through != -1)
	{
		if (through == 1)
		{
			target = target1;
			if (shadowPass) target = (target1-target).norm()*0.45 + target;
		}
		else if (through == 2)
		{
			target = target2;
			if (shadowPass) target = (target2-target).norm()*0.45 + target;
		}
	}
    */

    if (!fixedTarget.valid())
    {
        w = openness(1, target);
        opennessThrough(1, target1, target2, w1, w2);
        target = agents[1]->self()->getKickerPos();
        draw("not fixex", Vector2D(0,0.3)+target, "red");
        w = 1;
    }
    else {
        target = fixedTarget;
        through = -1;
        draw("fixed target :(", Vector2D(0,0.3)+target, "red");
    }


    draw(Circle2D(target, 0.050), "purple", false);
	draw(Circle2D(target, 0.025), "purple", false);
	draw(Circle2D(target, 0.075), "purple", false);
	draw(QString("%1").arg(w, 0, 'g', 2), target - Vector2D(0.0, 0.05), "purple", 10);

	int passspeed = 0;
	//passspeed = 6;//agents[0]->kickDistanceValue((target-agents[0]->self()->pos).length(), chip);
	//        passspeed = 6;//agents[0]->kickDistanceValue((target-agents[0]->self()->pos).length(), chip);

	if (nochip) chip = false;

	double dist = (target - agents[0]->self()->getKickerPos()).length();
	if ((!chipPass) && (!chip))
	{
		//passspeed = agents[0]->kickValueForDistance(dist, 3); // comment in telecomp 2011
		passspeed = agents[0]->kickValueForDistance(dist, 6.0);
		if ( knowledge->getGameMode()==CKnowledge::OurKickOff)
			passspeed = agents[0]->kickValueForDistance(dist, 5.5);


		debug("kick profil", D_ALI);
	}
	else{

/*		double bouncefactor = 1.0;
		if (id == 6) bouncefactor = 1;
		lse bouncefactor = 0.75;*/

		debug("chip profil", D_ALI);

//		if (dist > 1.0)
//		{
//			dist *= (dist - 1.0) * 0.6 + 1.0;
//		}



        passspeed = agents[0]->chipDistanceValue(dist, (onepassing==-1));

//		if (passspeed <= 1.0)
//		{
//			if (id == 2) passspeed = 18;
//		}
		double chip_range = agents[0]->chipValueDistance(passspeed, (onepassing==-1));
		knowledge->chipPassFallPoint = (target - agents[0]->self()->getKickerPos()).norm() * chip_range + agents[0]->self()->getKickerPos();
	}

	//passspeed -= 1;
	////CKS
	//to be checked:
	//        double dist=(target-agents[0]->self()->pos).length();
	//        if(!chip)
	//        {
	//            double velf=0;
	//            passspeed=agents[0]->kickSpeedValue(sqrt(velf*velf - 2*BallFriction()*Gravity*dist), false/**/);
	//        }
	//        else
	//        {
	//            passspeed=agents[0]->chipDistanceValue(dist,false/**/);
	//        }
	//        allowonepass = false;

	if (allowonepass )//&& (!chipPass) && (!chip))
	{
		double vball = agents[0]->self()->ballComingSpeed();
		double angle = fabs(Vector2D::angleBetween(wm->ball->pos-agents[0]->pos(), (target - agents[0]->pos())).degree());
		if ((angle < 90) && (vball > 0.5))
		{
			onepassing = 0;
			onetouch->setWaitPos(agents[0]->pos());
		}
		if (onepassing >= 0)
		{
			onetouch->setAgent(agents[0]);
			onetouch->setTarget(target);
			onetouch->setKickSpeed(passspeed);
			onetouch->setChip(chip);
			onetouch->setAvoidPenaltyArea(true);
			onetouch->execute();
			onepassing ++;
		}
	}
	else onepassing = -1;
	if (onepassing == -1)
	{
		if(slowKick)
		{
			kick->setSlow(true);
			kick->setInterceptMode(false);
		}
		else
		{
			kick->setInterceptMode(true);
			kick->setSlow(false);
		}
        kick->setSpin(0);
		kick->setAvoidPenaltyArea(true);
		kick->setAgent(agents[0]);
		kick->setTarget(target);
		if ( knowledge->getGameMode()==CKnowledge::OurKickOff)
			kick->setTarget(Vector2D(0.06,kick->getTarget().y));
		if (knowledge->getGameMode()==CKnowledge::OurKickOff)
		{
			target.x += 0.1;
		}
		//            kick->setTolerance(0.015);
		kick->setTolerance(0.15);
		kick->setWaitFrames(3);
		kick->setChip(chipPass || chip);
		kick->setWaitFrames(0);
		kick->setKickSpeed(passspeed);
		kick->execute();
	}
}

double CBehaviourPass::openness(int i, Vector2D& point)
{
	QList<Circle2D> obs;
	for (int j = 0;j < wm->our.activeAgentsCount();j++)
	{
		if (wm->our.active(j)->id == agents[i]->id() || wm->our.active(j)->id == agents[i-1]->id()) continue;
		obs.append(Circle2D(wm->our.active(j)->pos, wm->our.active(j)->robotRadius()));
	}
	for (int j = 0;j < wm->opp.activeAgentsCount();j++)
	{
		obs.append(Circle2D(wm->opp.active(j)->pos, Opp_Big_Factor * wm->opp.active(j)->robotRadius()));
		draw(obs.last(), "red");
	}
	double angle, biggestAngle, prob;
	Vector2D r = (agents[i]->self()->getKickerPos()-agents[i-1]->self()->getKickerPos());
	Vector2D n = r.norm().rotatedVector(90.0)*agents[i]->self()->robotRadius()*2.0 * Our_Run_Factor;
	knowledge->getEmptyAngle(agents[i-1]->self()->getKickerPos(), agents[i]->self()->getKickerPos() + n, agents[i]->self()->getKickerPos() - n, obs, prob, angle, biggestAngle);
	point = Segment2D(agents[i]->self()->getKickerPos() + n, agents[i]->self()->getKickerPos() - n).intersection(Line2D(agents[i-1]->self()->getKickerPos(), AngleDeg(angle)));
	draw(Segment2D(agents[i-1]->self()->getKickerPos(), agents[i]->self()->getKickerPos() + n), "purple");
	draw(Segment2D(agents[i-1]->self()->getKickerPos(), agents[i]->self()->getKickerPos() - n), "purple");
	draw(Segment2D(agents[i-1]->self()->getKickerPos(), agents[i-1]->self()->getKickerPos() + Vector2D::unitVector(AngleDeg(angle)) * 3.0) , "red");
	if (!point.valid() || prob < 0.001)
	{
		point = agents[i]->self()->getKickerPos();
	}
	return prob;
}

void CBehaviourPass::opennessThrough(int i, Vector2D& point1, Vector2D& point2, double &w1, double &w2)
{
	QList<Circle2D> obs;
	for (int j = 0;j < wm->our.activeAgentsCount();j++)
	{
		if (wm->our.active(j)->id == agents[i]->id() || wm->our.active(j)->id == agents[i-1]->id()) continue;
		obs.append(Circle2D(wm->our.active(j)->pos, wm->our.active(j)->robotRadius()));
	}
	for (int j = 0;j < wm->opp.activeAgentsCount();j++)
	{
		obs.append(Circle2D(wm->opp.active(j)->pos, wm->opp.active(j)->robotRadius()));
	}
	double angle, biggestAngle, prob;
	Vector2D r = (agents[i]->self()->getKickerPos()-agents[i-1]->self()->getKickerPos());
	Vector2D n = r.norm().rotatedVector(90.0)*agents[i]->self()->robotRadius()*2.0 * Our_Run_Factor;

	Vector2D p1 = agents[i]->self()->getKickerPos();
	Vector2D p2 = agents[i]->self()->getKickerPos();

	p1 += (wm->field->oppGoal() - p1).norm() * CRobot::robot_radius_old * 2.0 * Our_Run_Factor;
	p2 -= (wm->field->oppGoal() - p2).norm() * CRobot::robot_radius_old * 2.0 * Our_Run_Factor;

	double prob1, prob2;
	knowledge->getEmptyAngle(agents[i-1]->self()->getKickerPos(), p1 + n, p1 - n, obs, prob1, angle, biggestAngle);
	knowledge->getEmptyAngle(agents[i-1]->self()->getKickerPos(), p2 + n, p2 - n, obs, prob2, angle, biggestAngle);

	if (agents[i-1]->self()->getKickerPos().x < 0.0) //its dangerous to send a back pass in our field
	{
		if (agents[i-1]->self()->getKickerPos().x < -1.0)
		{
			prob2 *= 0.0;
		}
		else
		{
			prob2 *= 1.0 + agents[i-1]->self()->getKickerPos().x;
		}
	}

	prob2 *= 0.0;

	if (!point1.valid() || prob1 < 0.001)
	{
		point1 = agents[i]->self()->getKickerPos();
	}
	if (!point2.valid() || prob2 < 0.001)
	{
		point2 = agents[i]->self()->getKickerPos();
	}
	w1 = prob1;
	w2 = prob2;
}


double CBehaviourPass::probability()
{
	if (knowledge->sentChipPass) return 0.0;
	if (agents.count() <= 1)
	{
		return 0.0;
	}
	double pass_prob =  1.0;
	for (int i = 1;i < agents.count();i++)
	{
		Vector2D point;
		double prob = fabs(openness(i, point));

		if (nokickprob)
		{
			pass_prob *= prob;
		}
		else {
			double kickprob = CBehaviourKick::probability(QList<CAgent*> () << agents[i]);
			double d = (agents[i]->pos() - agents[i-1]->pos()).length();
			d = (d - 2.5) / 1.5;
			double ang = fabs(Vector2D::angleBetween( agents[i-1]->pos() - agents[i]->pos(), wm->field->oppGoal() - agents[i]->pos()).degree());
			ang = (ang - 60.0) / 20.0;
			ang *= -ang;
			d *= -d;
			//        prob = prob*fabs(exp(ang+d));
			agents[i]->goalVisibility = kickprob;
			if (kickprob<0.1) kickprob = 0.1;
			debug( QString("PassProb : %1 ,  Prob : %2 , kickProb : %3").arg(pass_prob).arg(prob).arg(kickprob),D_SEPEHR);
			pass_prob *= (prob*kickprob);
		}
    }
	return fabs(pass_prob);
}


INIT_BEHAVIOUR(CBehaviourSpinPass, "spinpass");


CBehaviourSpinPass::CBehaviourSpinPass() : CBehaviourPass()
{
	spin = new CSkillSpinBack(NULL);
	roles.append("playmake");
}

CBehaviourSpinPass::~CBehaviourSpinPass()
{
	delete spin;
}


void CBehaviourSpinPass::execute()
{
	if (agents.count() <= 1)
		return;
	else {
		passEval.createPass(agents[0]->id(), agents[1]->id(), PassEvaluation::Simple); //inform the knowledge that a pass is being sent
		double w;
		Vector2D target;
		w = openness(1, target);
		spin->setAgent(agents[0]);
		spin->setTakeBack( false);
		spin->setTarget( target);
		spin->setAnglularVelocity(90.0);
		spin->setCorrectAngleTowardTarget(false);
		spin->setLinearAcceleration(3.0);
		spin->setLinearVelocity(0.0);
		spin->setWaitFrames(0);
		spin->execute();
	}
}

double CBehaviourSpinPass::probability()
{
	return CBehaviourPass::probability();
}




INIT_BEHAVIOUR(CBehaviourChipToGoal, "chiptogoal");

CBehaviourChipToGoal::CBehaviourChipToGoal() : CBehaviourKick()
{
	chipToGoal = true;
}

CBehaviourChipToGoal::~CBehaviourChipToGoal()
{
	delete kick;
	delete onetouch;
}

void CBehaviourChipToGoal::execute()
{
	CBehaviourKick::execute();
}

double CBehaviourChipToGoal::probability()
{
	if (agents.count()==0) return 0.0;
	if (knowledge->getGameMode()==CKnowledge::OurKickOff) {
		double rw = 0.0;
		knowledge->goalVisiblity(agents[0]->id(), rw, 1.0);
		if (rw < KickOffGoalVisibChip)
			return 1.0;
		else
			return 0.25;
	}
	if (agents[0]->pos().x < 0)
	{
		double l = 1.0;
		for (int i=0;i<wm->opp.activeAgentsCount();i++)
		{
			double d = (wm->opp.active(i)->pos - agents[0]->pos()).length();
			if (fabs(Vector2D::angleBetween((wm->opp.active(i)->pos - agents[0]->pos()), (wm->field->oppGoal() - agents[0]->pos())).degree()) > 120)
				l *= tanh(d / 0.5);
		}
		double xfactor = (1.0 - (agents[0]->pos().x - wm->field->ourGoal().x) / (wm->field->oppGoal().x - wm->field->ourGoal().x) * 2.0);
		if (xfactor < 0) xfactor = 0.0;
		if (xfactor > 1) xfactor = 1.0;
		return l * xfactor;
	}
	return 0.0;
}


INIT_BEHAVIOUR(CBehaviourPassToDefense, "passtodefense");


CBehaviourPassToDefense::CBehaviourPassToDefense() : CBehaviourPass()
{
}

CBehaviourPassToDefense::~CBehaviourPassToDefense()
{
	delete kick;
	delete onetouch;
}


void CBehaviourPassToDefense::execute()
{
	CBehaviourPass::execute();
}

double CBehaviourPassToDefense::probability()
{
	return CBehaviourPass::probability();
}





INIT_BEHAVIOUR(CBehaviourChipPass, "chippass");


CBehaviourChipPass::CBehaviourChipPass()
	: CBehaviourPass()
{
}

CBehaviourChipPass::~CBehaviourChipPass()
{
}


void CBehaviourChipPass::execute()
{
	chipPass = true;
	CBehaviourPass::execute();
	if (agents.count() > 0)
	{
		knowledge->sendingChipPass = true;
		knowledge->chipPassSender = agents[0]->id();
	}
}

double CBehaviourChipPass::probability()
{
	if (knowledge->sentChipPass) return 0.0;
	if (agents.count() <= 1)
	{
		return 0.0;
	}
	double pass_prob =  1.0;
	for (int i = 1;i < agents.count();i++)
	{
		Vector2D point;
		double prob = openness(i, point);
		double d = (agents[i]->pos() - agents[i-1]->pos()).length();
		d = (d - 2.5) / 2.0;
		double ang = fabs(Vector2D::angleBetween( agents[i-1]->pos() - agents[i]->pos(), wm->field->oppGoal() - agents[i]->pos()).degree());
		ang = (ang - 60.0) / 20.0;
		if (prob>0.2)
		{
			pass_prob *= 0.6;//0.0;
		}
		else
		{
			pass_prob *= exp(-ang*ang-d*d) * (0.3 - prob);
		}
		double kickprob = CBehaviourKick::probability(QList<CAgent*> () << agents[i]);
		if (kickprob<0.1) kickprob = 0.1;
		pass_prob *= kickprob;
	}
	return pass_prob;
}


INIT_BEHAVIOUR(CBehaviourKickBetweenTheirDefenders, "kickbetweentheirdefenders");

CBehaviourKickBetweenTheirDefenders::CBehaviourKickBetweenTheirDefenders() : CBehaviourKick()
{

}

CBehaviourKickBetweenTheirDefenders::~CBehaviourKickBetweenTheirDefenders()
{
    //CBehaviourKick::~CBehaviourKick();
}


void CBehaviourKickBetweenTheirDefenders::execute()
{
    if (agents.count() == 0) return;
    if ((agents[0]->pos() - wm->field->oppGoal()).length() < 1.0) {CBehaviourKick::execute();return;}
    Vector2D p;
    int count=0;
    for (int i=0;i<_MAX_NUM_PLAYERS;i++)
    {
        if (wm->oppRole[i] == "defense" && wm->opp[i]->inSight > 0)
        {
            p+=wm->opp[i]->pos;
            count ++;
        }
    }
    if (count == 0) {CBehaviourKick::execute();return;}
    kick->setAgent(agents[0]);
    kick->setTarget(p);
    kick->setSlow(false);
    kick->setKickSpeed(MAX_KICK_SPEED);
    kick->setChip(false);
    kick->setInterceptMode(true);
    kick->setSpin(false);
    kick->execute();
}

double CBehaviourKickBetweenTheirDefenders::probability()
{
    return 0;
    /*
      This is a behaviour written for game with CMDragons in robocup 2013.
      Not useful in the game on, just for indirect kicks from our field
      */
}


