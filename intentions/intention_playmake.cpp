#include "intention_playmake.h"
#include "knowledge.h"


bool IntentionPlayMake::finished()
{
	if( wm->our.data->activeAgents.contains(M_agent_id) == false )
		return true;

	if( knowledge->frameCount - M_set_time < M_cycle_wait )
	{
		return false;
	}

	if( knowledge->isOurNonPlayOnKick() )
	{
		return false;
	}

////// BUG: when playmake is stopped, playmake never changed!
//	Vector2D nextBallPos = wm->ball->pos + wm->ball->vel;
//	Vector2D nextAgentPos = wm->our[M_agent_id]->pos + wm->our[M_agent_id]->vel;
//	double curDist = wm->ball->pos.dist(wm->our[M_agent_id]->pos);
//	double nextDist = nextBallPos.dist(nextAgentPos);

//	debug(QString("DISTS: %1 %2").arg(nextDist).arg(curDist) , D_MASOOD , "red");

//	if( nextDist-0.6 > curDist ){
//		return true;
//	}
////////////////////////////////////////////////////////////

	return true;
}
