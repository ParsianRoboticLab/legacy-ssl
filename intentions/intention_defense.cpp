#include "intention_defense.h"
#include "knowledge.h"


bool IntentionDefense::finished()
{
	if( wm->our.data->activeAgents.contains(M_agent_id) == false )
		return true;

	if( knowledge->frameCount - M_set_time > M_cycle_wait )
	{
		return true;
	}

	return false;
}

