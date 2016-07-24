#ifndef INTENTION_POSITION_H
#define INTENTION_POSITION_H

#include "soccer_intention.h"
//#include "knowledge.h"

class IntentionPosition
        : public SoccerIntention {

public:
	IntentionPosition(){
		M_last_playmake_ID = -1;
	}

	void assign( const int & agent_id, const int & set_time , const int & cycle_wait=100 )
	{
		M_type = "position";
		M_agent_id = agent_id;
		M_set_time = set_time;
		M_cycle_wait = cycle_wait;
		positioningSequenceDone = false;
	}

    bool finished();

    bool positioningSequenceDone;

private:
    /*!
      \brief clear the action queue
     */
    void clear()
    {
		M_agent_id = M_set_time = -1;
    }

};

#endif // INTENTION_POSITION_H
