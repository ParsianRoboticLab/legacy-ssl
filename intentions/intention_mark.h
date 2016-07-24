#ifndef INTENTION_MARK_H
#define INTENTION_MARK_H

#include "soccer_intention.h"
//#include "knowledge.h"

class IntentionMark
        : public SoccerIntention {
private:
    int M_target_opponent; //!< marked opponent player

public:
	IntentionMark(){
		M_last_playmake_ID = -1;
	}

	void assign( int agent_id , const int & target_opponent,
				   const int & set_time ,  const int & cycle_wait=100 )
	{
		M_type = "mark";
		M_agent_id = agent_id;
		M_target_opponent = target_opponent;
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
		M_agent_id = M_target_opponent = M_set_time = -1;
    }

};

#endif // INTENTION_MARK_H
