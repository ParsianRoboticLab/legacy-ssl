#ifndef INTENTION_DEFENSE_H
#define INTENTION_DEFENSE_H
#include "soccer_intention.h"
//#include "knowledge.h"

class IntentionDefense : public SoccerIntention
{
public:
	IntentionDefense(){
		M_last_playmake_ID = -1;
	}
	void assign( int agent_id , const int & set_time , const int & cycle_wait=100 )
	{
		M_type = "defense";
		M_agent_id = agent_id;
		M_set_time = set_time;
		M_cycle_wait = cycle_wait;
	}

	bool finished();

private:
	/*!
	  \brief clear the action queue
	 */
	void clear()
	{
		M_agent_id = M_set_time = -1;
	}
};

#endif // INTENTION_DEFENSE_H
