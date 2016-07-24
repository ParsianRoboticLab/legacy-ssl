#include "conditions.h"
#include "worldmodel.h"
#include "knowledge.h"
QList<CGameCondition*> CGameConditions::conditions;  
QString CGameCondition::getName() {return name;}
CGameCondition::CGameCondition() {lastResult = false;}
bool CGameCondition::check(QStringList /*params*/) {return false;}
bool CGameConditions::registerCondition(CGameCondition* condition) {conditions.append(condition);return true;}
bool CGameConditions::check(QString condition, QStringList params)
{
    if (condition.length()>0)
    {
        bool n=false;
        if (condition[0]=='!') {n=true;condition = condition.right(condition.length()-1);}
        for (int i=0;i<conditions.count();i++)
        {
            if (conditions[i]->getName() == condition.toLower())
                return ((n) ? (!conditions[i]->check(params)) : conditions[i]->check(params));
        }
        return false;
    }
    return false;
}
//----------------------------------------------------------------

CONDITION(CConditionAlways, "always") {params.isEmpty();return true;}
CONDITION(CConditionDone, "done") {params.isEmpty();return false;}
CONDITION(CConditionNever, "never") {params.isEmpty();return false;}
CONDITION(CConditionOurDirect, "ourdirect") {params.isEmpty();return (knowledge->getGameState() == CKnowledge::OurDirectKick) || (knowledge->getGameMode() == CKnowledge::OurDirectKick);}
CONDITION(CConditionOurIndirect, "ourindirect") {params.isEmpty();return (knowledge->getGameState() == CKnowledge::OurIndirectKick) || (knowledge->getGameMode() == CKnowledge::OurIndirectKick); }
CONDITION(CConditionOurKickOff, "ourkickoff") {params.isEmpty();return knowledge->getGameState() == CKnowledge::OurKickOff;}
CONDITION(CConditionOurPenalty, "ourpenalty") {params.isEmpty();return knowledge->getGameState() == CKnowledge::OurPenaltyKick;}
CONDITION(CConditionTheirDirect, "theirdirect") {params.isEmpty();return knowledge->getGameState() == CKnowledge::TheirDirectKick;}
CONDITION(CConditionTheirIndirect, "theirindirect") {params.isEmpty();return knowledge->getGameState() == CKnowledge::TheirIndirectKick;}
CONDITION(CConditionTheirKickOff, "theirkickoff") {params.isEmpty();return knowledge->getGameState() == CKnowledge::TheirKickOff;}
CONDITION(CConditionTheirPenalty, "theirpenalty") {params.isEmpty();return knowledge->getGameState() == CKnowledge::TheirPenaltyKick;}
CONDITION(CConditionStop, "stop") {params.isEmpty();return knowledge->getGameState() == CKnowledge::Stop;}
CONDITION(CConditionStart, "start") {params.isEmpty();return knowledge->getGameState() == CKnowledge::Start;}
CONDITION(CConditionHalt, "halt") {params.isEmpty();return knowledge->getGameState() == CKnowledge::Halt;}


CONDITION(CConditionTimeOut, "timeout")
{
    double t = 10;
    if (params.count()>0) t = params[0].toDouble();
    double tt = fabs(knowledge->currentTime() - knowledge->getLastTimeGameStateChanged());
    return (tt > t);
}

CONDITION(CConditionTechnical, "technical") {
    debug(knowledge->getTechnicalMode(), D_ERROR);
    if (params.length() == 0) return knowledge->getTechnicalMode() != "";
    else return knowledge->getTechnicalMode() == params[0];
}

CONDITION(CConditionBallInOurField, "ballinourfield")        
{
    bool nohist = false;
    if (params.count()>0)
        if (params[0]=="nohist")
            nohist = true;
    nohist = true;
    if (nohist)
    {
        if (wm->ball->pos.x < 0.0)
            return true;
        else
            return false;
    }
    if (wm->ball->pos.x < -0.40) lastResult = true;
    if (wm->ball->pos.x >  0.40) lastResult = false;
    return lastResult;
}

CONDITION(CConditionBallInTopField, "ballintopfield")
{
    bool nohist = false;
    if (params.count()>0)
        if (params[0]=="nohist")
            nohist = true;
    if (nohist)
    {
        if (wm->ball->pos.y < 0.0)
            return false;
        else
            return true;
    }
    if (wm->ball->pos.y < -0.70) lastResult = false;
    if (wm->ball->pos.y >  0.70) lastResult = true;
    return lastResult;
}


CONDITION(CConditionOppCorner, "oppcorner")
{
    return wm->field->getRegion(CField::OppCornerTop).contains(wm->ball->pos) || wm->field->getRegion(CField::OppCornerBottom).contains(wm->ball->pos);
}

CONDITION(CConditionOurCorner, "ourcorner")
{
    return wm->field->getRegion(CField::OurCornerTop).contains(wm->ball->pos) || wm->field->getRegion(CField::OurCornerBottom).contains(wm->ball->pos);
}


CONDITION(CConditionBallMoved, "ballmoved")
{
    params.isEmpty();

	if (wm->ball->hist.length() == 0)
	{
		if (wm->ball->vel.length()>0.30)
			return true;
		else
			return false;
	}
	else
	{
		for (int i=0;i<wm->ball->hist.count();i++)
		{
			if ((wm->ball->hist[i].pos - wm->ball->hist[0].pos).length() > 0.12)
			{
				return true;
			}
		}
		return false;
	}

}

CONDITION(CConditionBallInside, "ballinside")
{    
    Rect2D r,r2;
    QString s="-";
    if (params.count()>0)
    {
        r = wm->field->getRegion(params[0]);
        s = params[0];
    }
    else r = wm->field->fieldRect();
    bool nohist = true;
    if (params.count()>1)
    {
        if (params[1]=="hist") nohist = false;
    }
    r2 = Rect2D::from_center(r.center(), r.size().length()-CBall::radius*2.0, r.size().width()-CBall::radius*2.0);
    if (nohist)
    {
        if (r.contains(wm->ball->pos))
            return true;
        else
            return false;
    }
    if (r2.contains(wm->ball->pos))
    {        
        lastResults[s]=true;
    }
    else {
        if (r.contains(wm->ball->pos))
        {
            if (lastResults.contains(s)) return lastResults[s];
            else {
                lastResults[s]=true;                
            }
        }
        else lastResults[s]=false;
    }
    return lastResults[s];
}

CONDITION(CConditionRectIsClear, "rectisclear")
{
    Rect2D r;
    if (params.count()>0)
    {
        r = wm->field->getRegion(params[0]);
    }
    else
        return false;
    for ( int i = 0 ; i < wm->opp.activeAgentsCount(); i++)
    {
		if (r.contains(wm->opp.active(i)->pos))
            return false;
    }
    return true;
}

CONDITION(CConditionCount, "count")
{
    if (params.count() != 2) return false;
    int att = 0, def = 0, goalie = 0;
    att = knowledge->formation.attackers;
    def = knowledge->formation.defenders;
    goalie= knowledge->formation.goalie;
    int cmp = params[1].toInt();
    if (params[0].trimmed().toLower() == "this")
    {
        if (cmp == knowledge->currentPlayAllowedAgents) return true;
    }
    if (params[0].toLower() == "a")
    {
        if (cmp == att) return true;
    }
    if (params[0].toLower() == "d")
    {
        if (cmp == def) return true;
    }
    if (params[0].toLower() == "g")
    {
        if (cmp == goalie) return true;
    }
    if (params[0].toLower() == "*")
    {
        if (cmp == att + def + goalie) return true;
    }
    return false;
}

CONDITION(CConditionIndirectType, "indirecttype")
{
    if (params.count()>=1)
    {
        return policy()->OurIndirect_IndirectType()==params[0].toInt();
    }
    else return true;
}

CONDITION(CConditionNoPlaymaker, "noplaymaker")
{
    params.isEmpty();
    return (knowledge->getPlayMaker() == NULL);
}

CONDITION(CConditionBallOurs, "ballours")
{    
    params.isEmpty();   
    return knowledge->isBallOurs();
}

CONDITION(CConditionOppDifendersCount, "difenderscount")
{
    int desiredDifenders;
    if (params.count()>0)
        desiredDifenders = params[0].toInt();
    int numOfDefs = 0;
    for(int i = 0; i < wm->opp.activeAgentsCount(); i++ )
    {
		if (wm->opp.active(i)->inSight <=0)
            continue;
		if (wm->opp.active(i)->pos.dist( wm->field->oppGoal()) <  1.3)
            numOfDefs++;
    }
//    if ( knowledge->findOppGoalie() != -1 )
//        numOfDefs--;
    return desiredDifenders == numOfDefs ? true : false;
}

CONDITION(CConditionExecuting, "executing")
{
    if (params.count()==0) return false; //AND the parameters    
    for (int i=0;i<params.count();i++)
    {
        if (!knowledge->executingPlays.contains(params[i], Qt::CaseInsensitive)) {
            return false;
        }
    }    
    return true;
}


CONDITION(CConditionTimeIn, "timein")
{

}


CONDITION(CConditionPlayMakerInside, "playmakerinside")
{
    if (knowledge->getPlayMaker() == NULL) return false;
    if (params.count()==0) return false;
    for (int i=0;i<params.count();i++)
        if (wm->field->getRegion(params[i]).contains(knowledge->getPlayMaker()->pos()))
            return true;
}

CONDITION(CConditionWarmup, "warmup")
{
    if( knowledge->warmup)
    {
        return true;
    }
    return false;
}


CONDITION(CConditionExists, "exists")
{
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
        for (int j=0;j<params.count();j++)
			if (params[j] == wm->opp.active(i)->role) return true;
    return false;
}

CONDITION(CConditionCheck, "check") {

    if (params.isEmpty()) return false;
    if (params.count() == 1)
    {
        return (knowledge->variables[params[0]].toLower() == "true");
    }
    else
    {
        if (params.count()==3)
        {
            double a1 = knowledge->variables[params[0]].toDouble();
            double a2 = params[1].toLower().toDouble();
            debug(QString("a : %1 , b : %2").arg(a1).arg(a2), D_SEPEHR);
            if (params[2] == ">=") return a1 >= a2;
            if (params[2] == "<=") return a1 <= a2;
            if (params[2] == ">") return a1 > a2;
            if (params[2] == "<") return a1 < a2;
            if (params[2] == "==") return a1 == a2;
            if (params[2] == "!=") return a1 != a2;
        }
        return (knowledge->variables[params[0]]==params[1].toLower());
    }
}

CONDITION(CConditionBallFromTop, "ballfromtop") {
	if ( wm->ball->pos.y > 0.45 || (wm->ball->pos.y > -0.45 && knowledge->getBallCommingFromTop()))
		return true;
	else
		return false;
}

CONDITION(CConditionBallFromBottom, "ballfrombottom") {
	if ( wm->ball->pos.y < -0.45 || (wm->ball->pos.y < 0.45 && !knowledge->getBallCommingFromTop()))
		return true;
	else
		return false;
}

CONDITION(CConditionTheyAreKhafan, "theyarekhafan") { //to block immortalz in iranopen2012
	bool khafan = false;
	int num = 0;
	for( int i = 0; i < wm->opp.activeAgentsCount(); i++)
	{
		if( wm->ball->pos.dist( wm->opp.active(i)->pos) < 0.9)
			num++;
	}
	if ( num > 1)
		return true;
	return false;
}

CONDITION(CConditionTheyAreShirje, "theyareshirje") { //to block MRL in iranopen2012 
    Rect2D shirjeRect;
    if (wm->opp.activeAgentsCount() < 4) return false;
    if (wm->ball->pos.x < -2.4) {knowledge->shirjezan=-1;return false;}
    if ((knowledge->frameCount - knowledge->lastFrameShirjeZanDetected < 180) && (knowledge->shirjezan != -1)

            ) //3 seconds
    {
        if (knowledge->shirjezan <= _MAX_NUM_PLAYERS)
        {
            if (wm->opp[knowledge->shirjezan]->inSight > 0) return true;
        }
    }
    if (wm->ball->pos.y > 0)
    {
        shirjeRect = Rect2D(Vector2D(1.5,-1), Vector2D(0.05, -1.95));
    }
    else {
        shirjeRect = Rect2D(Vector2D(1.5, 1), Vector2D(0.05,  1.95));
    }
    int shirjezans = 0;
    double minD = 1000;
    int nearestToBall = -1;
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
		double d_to_ball = (wm->opp.active(i)->pos - wm->ball->pos).length();
        if (d_to_ball < minD)
        {
            minD = d_to_ball;
            nearestToBall = i;
        }
		if (shirjeRect.contains(wm->opp.active(i)->pos))
        {
            shirjezans ++;
			knowledge->shirjezan = wm->opp.active(i)->id;
        }
    }
    int forwards = 0;
    if (nearestToBall != -1)
    {
        for (int i=0;i<wm->opp.activeAgentsCount();i++)
        {
            if (i!=nearestToBall)
            {
				if (wm->opp.active(i)->pos.x < wm->ball->pos.x)
                {
                    forwards ++;
                }
            }
        }
    }

	debug(QString("forwards=%1").arg(forwards), D_ERROR);

	if (forwards >= 1) {knowledge->shirjezan=-1;
	debug(QString("shirje nazan"), D_ERROR);
	return false;
	}
    if (shirjezans >= 1) {
        knowledge->lastFrameShirjeZanDetected = knowledge->frameCount;
        return true;
    }
    {knowledge->shirjezan=-1;return false;}
}

CONDITION(CConditionAgentSwitched, "agentswitched") {
	if ( 	knowledge->switchState > 1)
		return true;
	else
		return false;
}

CONDITION(CConditionTheyHaveBlocker, "theyhaveblocker") {
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
        Vector2D sol1, sol2;
        if (Circle2D(wm->opp.active(i)->pos, 0.3).intersection(Segment2D(wm->ball->pos, wm->field->oppGoal()), &sol1, &sol2) > 0)
        {
            if ((wm->opp.active(i)->pos - wm->ball->pos).length() < 1.0)
            {
                return true;
            }
        }
    }
    return false;
}
