#ifndef PLAN_H
#define PLAN_H

#include <QList>
#include <agent.h>
#include <skills.h>

class Plan
{
protected:
    QList<CAgent*> agents;
public:
    Plan();
    CAgent* agent(int i);
    int count();
    void init(const QList<CAgent*> & _agents);
    void assignSkill(int i, CSkill* skill);
    int agentById(int id);
    QList<int> getAgentIdList();
	void debugAgents(QString text);

//abstract functions
    virtual void execute()=0;
};

#endif // PLAN_H
