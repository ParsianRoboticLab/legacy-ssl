#include "intention_position.h"
#include "knowledge.h"


/*-------------------------------------------------------------------*/
/*!

*/
bool
IntentionPosition::finished()
{
	if( wm->our.t->activeAgents.contains(M_agent_id) == false )
	{
		return true;
	}

	if( positioningSequenceDone ){
		return true;
	}

//	Check velocity and direction of ball
//	if( wm->ball->vel.valid() && wm->ball->vel.length() > 1 ){
//		Segment2D ballSeg(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm()*10);
//		Circle2D circle(wm->our[M_agent_id]->pos , 1);
//		Vector2D sol1, sol2;
//		int cnt = circle.intersection(ballSeg, &sol1 , &sol2);
//		if( cnt == 0 ){
//			return true;
//		}
//	}


//	int playMakeID=-1;
//	for( int i=0 ; i<knowledge->agentsWithIntention.size() ; i++ ){
//		int id = knowledge->agentsWithIntention.at(i);
//		CAgent *agnt = knowledge->getAgent(id);
//		if( agnt->intention->M_type == "playmake" ){
//			playMakeID = id;
//		}
//	}
//	if( playMakeID != M_last_playmake_ID )
//	{
//		M_last_playmake_ID = playMakeID;
//		return true;
//	}
//	M_last_playmake_ID = playMakeID;



//	if ( knowledge->frameCount - M_set_time > M_cycle_wait )
//	{
//		return true;
//	}

    return false;
}


