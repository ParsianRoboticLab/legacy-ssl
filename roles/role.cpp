#include "role.h"

CRole::CRole() : CSkill()
{
    currentBehaviour = NULL;
}

CRole::CRole(CAgent* _agent) : CSkill(_agent)
{
    currentBehaviour = NULL;
}

CRole* CRole::fromString(QString roleName)
{
    return static_cast<CRole*> (CSkills::initSkill(roleName.toLower().toAscii().data(), NULL));
}

CRoleInfo* CRole::generateInfoClass()
{
    return NULL;
}

int CRole::level()
{
    return 2;
}

void CRole::selectBehaviour()
{
    double bestValue = -10000.0;
    int bestBehaviour = -1;
    knowledge->behavioursDebug = "";
    for (int i=0;i<behaviours.count();i++)
    {
        double p = fabs(behaviours[i]->probability());
        BehaviourValue v = CBehaviours::values[behaviours[i]->getName()];
        double value = p * v.reward + (1.0 - p) * v.penalty;
        if (value >= bestValue)
        {
            bestValue = value;
            bestBehaviour = i;
        }
        if (behaviours[i]->getName() == "pass" && behaviours[i]->agents.count()>=2)
        {
            knowledge->behavioursDebug += QString("%1(%2): %3,%4 = %5 : %6\n").arg(behaviours[i]->getName()).arg(behaviours[i]->agents[1]->id()).arg(v.reward).arg(v.penalty).arg(value).arg(p);
			debug(QString("%1(%2): %3,%4 = %5 : %6\n").arg(behaviours[i]->getName()).arg(behaviours[i]->agents[1]->id()).arg(v.reward).arg(v.penalty).arg(value).arg(p),D_SEPEHR);
        }
        else if (behaviours[i]->getName() == "chippass" && behaviours[i]->agents.count()>=2)
        {
            knowledge->behavioursDebug += QString("%1(%2): %3,%4 = %5 : %6\n").arg(behaviours[i]->getName()).arg(behaviours[i]->agents[1]->id()).arg(v.reward).arg(v.penalty).arg(value).arg(p);
			debug(QString("%1(%2): %3,%4 = %5 : %6\n").arg(behaviours[i]->getName()).arg(behaviours[i]->agents[1]->id()).arg(v.reward).arg(v.penalty).arg(value).arg(p),D_SEPEHR);
        }
        else knowledge->behavioursDebug += QString("%1: %2,%3 = %4 : %5\n").arg(behaviours[i]->getName()).arg(v.reward).arg(v.penalty).arg(value).arg(p);
		debug(QString("%1: %2,%3 = %4 : %5\n").arg(behaviours[i]->getName()).arg(v.reward).arg(v.penalty).arg(value).arg(p),D_SEPEHR);
    }
    if (bestBehaviour != -1)
    {
        currentBehaviour = behaviours[bestBehaviour];
        knowledge->behavioursDebug += QString("Selected: %1 ; value=%2").arg(behaviours[bestBehaviour]->getName()).arg(bestValue);
		debug(QString("Selected: %1 ; value=%2").arg(behaviours[bestBehaviour]->getName()).arg(bestValue),D_SEPEHR);
    }
    else {
        currentBehaviour = NULL;
        knowledge->behavioursDebug += QString("Selected: NULL").arg(behaviours[bestBehaviour]->getName());
    }


    /*double bestValue = -10000.0;
    int bestBehaviour = -1;
    knowledge->behavioursDebug = "";
    for (int i=0;i<behaviours.count();i++)
    {
        double p = fabs(behaviours[i]->probability());
        BehaviourValue v = CBehaviours::values[behaviours[i]->getName()];
        double value = p * v.reward + (1.0 - p) * v.penalty;
        if (value >= bestValue)
        {
            bestValue = value;
            bestBehaviour = i;
        }
        if (behaviours[i]->getName() == "pass" && behaviours[i]->agents.count()>=2)
        {
            knowledge->behavioursDebug += QString("%1(%2): %3,%4 = %5 : %6\n").arg(behaviours[i]->getName()).arg(behaviours[i]->agents[1]->id()).arg(v.reward).arg(v.penalty).arg(value).arg(p);
        }
        else if (behaviours[i]->getName() == "chippass" && behaviours[i]->agents.count()>=2)
        {
            knowledge->behavioursDebug += QString("%1(%2): %3,%4 = %5 : %6\n").arg(behaviours[i]->getName()).arg(behaviours[i]->agents[1]->id()).arg(v.reward).arg(v.penalty).arg(value).arg(p);
        }
        else knowledge->behavioursDebug += QString("%1: %2,%3 = %4 : %5\n").arg(behaviours[i]->getName()).arg(v.reward).arg(v.penalty).arg(value).arg(p);
    }    
    if (bestBehaviour != -1)
    {
        currentBehaviour = behaviours[bestBehaviour];
        knowledge->behavioursDebug += QString("Selected: %1 ; value=%2").arg(behaviours[bestBehaviour]->getName()).arg(bestValue);
    }
    else {
        currentBehaviour = NULL;
        knowledge->behavioursDebug += QString("Selected: NULL").arg(behaviours[bestBehaviour]->getName());
    }*/
}

CRoleInfo::CRoleInfo(QString _roleName)
{
    InitVal(roleName);    
}

CAgent* CRoleInfo::robot(int i)
{
    int k=0;
    for (int j=0;j<knowledge->agentCount();j++)
		if (knowledge->getAgent(j)->skillName == roleName
				&& (wm->our[j]->inSight > 0)
				)
        {
            if (k==i) return knowledge->getAgent(j);
            k++;
        }
    return NULL;
}

int CRoleInfo::count()
{
    int k = 0;
    for (int j=0;j<knowledge->agentCount();j++)
		if ((knowledge->getAgent(j)->skillName == roleName)
			&& (wm->our[j]->inSight > 0))
        {
            k++;
        }
    return k;
}

int CRoleInfo::index(CRole *me)
{    
    int k = 0;
    for (int j=0;j<knowledge->agentCount();j++)
        if (knowledge->getAgent(j)->skillName == roleName)
        {
            if (knowledge->getAgent(j)->skill == me)
            {
                return k;
            }
            k++;
        }
    return -1;
}

void CRoleInfo::initBehaviours()
{
    if (!behaviours.empty()) return;
    behaviours.clear();
    for (int i=0;i<CBehaviours::BehavioursCount();i++)
        if (CBehaviours::Behaviour(i)->roles.contains(roleName))
        {
            behaviours.append(CBehaviours::Behaviour(i)->allocate());
        }
}
