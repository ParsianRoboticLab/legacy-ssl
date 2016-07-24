#include "plan.h"
#include "mathtools.h"
#include "exceptions.h"

Plan::Plan()
{

}

void Plan::init(const QList<CAgent*> & _agents)
{
    agents.clear();
    agents.append(_agents);
}

CAgent* Plan::agent(int i)
{
    if (i<0 || i>=agents.length())
    {
        throw new EOutOfBound(i, 0, agents.length()-1, "Plan::agent");
    }
    return agents[i];
}

int Plan::count()
{
    return agents.length();
}

/*!
 * @abstract assign a skill to some agent
 * @param i the number of agent in the local agents list
 * @param skill skill that is going to be assigned
 */
void Plan::assignSkill(int i, CSkill* skill)
{
    agent(i)->skill = skill;
    agent(i)->skillName = skill->getName();
    skill->setAgent(agent(i));
}

int Plan::agentById(int id)
{
    for (int i=0;i<count();i++)
    {
        if (agent(i)->id() == id) return i;
    }
    return -1;
}

QList<int> Plan::getAgentIdList()
{
    QList<int> l;
    for (int i=0;i<count();i++)
    {
        l.append(agent(i)->id());
    }
    return l;
}

void Plan::debugAgents(QString text){
	QString str;
	for( int i=0 ; i<agents.size() ; i++ )
		str += QString(" %1").arg(agents.at(i)->id());
	debug(QString("%1: Size: %2 --> (%3)").arg(text).arg(agents.size()).arg(str) , D_ERROR , "blue");
}
