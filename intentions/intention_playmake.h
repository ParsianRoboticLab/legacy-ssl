#ifndef INTENTION_PLAYMAKE_H
#define INTENTION_PLAYMAKE_H
#include "soccer_intention.h"

//#include "knowledge.h"

class IntentionPlayMake : public SoccerIntention
{
public:
	IntentionPlayMake(){
		M_last_playmake_ID = -1;
	}
	void assign( int agent_id , const int & set_time , const int & cycle_wait=30 )
	{
		M_type = "playmake";
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

#endif // INTENTION_PLAYMAKE_H
