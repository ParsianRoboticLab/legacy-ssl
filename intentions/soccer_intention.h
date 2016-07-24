#ifndef SOCCER_INTENTION_H
#define SOCCER_INTENTION_H

#include <boost/shared_ptr.hpp>

#include <string>
#include <queue>
#include <geom.h>
//#include "skills.h"
class CSkill;

/*!
  \class SoccerIntention
  \brief last action of each agent
*/
class SoccerIntention {
private:
    //! not used
    SoccerIntention( const SoccerIntention & );
    //! not used
    SoccerIntention & operator=( const SoccerIntention & );

protected:
    /*!
      \brief constructor is used only from derived class.
     */
    SoccerIntention()
      { }

public:
    /*!
      \brief virtual destructor
     */
    virtual
    ~SoccerIntention()
      { }

    ///////////////////////////////////////////////

    // virtual functions

    /*!
      \brief (pure virtual) check if this intention is finished or not
      \return true if intention is finished
     */
    virtual
    bool finished() = 0;

	/*!
	  \brief intention's type
	 */
	std::string M_type;
	int M_agent_id;
	int M_set_time;
	int M_cycle_wait;
	int M_last_playmake_ID;

};

#if 0
/////////////////////////////////////////////////////////////

/*!
  \class SoccerIntentionQueue
  \brief intention queue
*/
class SoccerIntentionQueue
    : public SoccerIntention {
private:

    typedef boost::shared_ptr< SoccerIntention > IntentionPtr;


    //! intention queue
    std::queue< IntentionPtr >  M_internal_q;

public:

    /*!
      \brief check if this intention queue is finished or not
      \return true if intention is finished
     */
    bool finished()
      {
          while ( ! M_internal_q.empty()
                  && M_internal_q.front()->finished() )
          {
              M_internal_q.pop();
          }

          if ( M_internal_q.empty() )
          {
              return true;
          }
          return false;
      }

    /*!
      \brief execute queued intention
      \return result of action
     */
    bool execute()
      {
          return pop();
      }

    /*!
      \brief push new intention
      \param intention dynamic allocated intention pointer
     */
    void push( IntentionPtr intention )
      {
          M_internal_q.push( intention );
      }

    /*!
      \brief execute queued intention.
      \retval true if action is executed
      \retval false if queue is empty, or action is failed.
     */
    bool pop()
      {
          while ( ! M_internal_q.empty()
                  && M_internal_q.front()->finished() )
          {
              M_internal_q.pop();
          }

          if ( M_internal_q.empty() )
          {
              return false;
          }

          return M_internal_q.front()->execute();
      }

    /*!
      \brief clear queued intention.
     */
    void clear()
      {
          while ( ! M_internal_q.empty() )
          {
              M_internal_q.pop();
          }
      }
};
#endif

#endif // SOCCER_INTENTION_H
