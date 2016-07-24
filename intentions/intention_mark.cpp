#include "intention_mark.h"
#include "knowledge.h"


/*-------------------------------------------------------------------*/
/*!

*/
bool
IntentionMark::finished()
{
	if( wm->our.t->activeAgents.contains(M_agent_id) == false )
		return true;

	if ( knowledge->frameCount - M_set_time > M_cycle_wait )
    {
        return true;
    }

    return false;
}
