#include "team.h"
#include <QDebug>

CTeam::CTeam(bool isOurTeam, bool noKalman)
{
    t = new CTeamData;
    for(int i=0;i<_MAX_NUM_PLAYERS;i++)
        t->teamMembers[i] = new CRobot(i, isOurTeam, noKalman);
    t->activeAgents.clear();
}

CTeam::~CTeam()
{
    for(int i=0;i<_MAX_NUM_PLAYERS;i++)
        delete t->teamMembers[i];
    delete t;
}

int CTeam::activeAgentsCount()
{
    return t->activeAgents.count();
}

void CTeam::update()
{
    t->activeAgents.clear();
    for( int i = 0; i < _MAX_NUM_PLAYERS; i++ )
    {
        if( t->teamMembers[i]->isActive() )
            t->activeAgents.append(i);
    }
}

int CTeam::activeAgentID(int i)
{    
    if((i<t->activeAgents.count())&&(i>=0))
    {
        return t->activeAgents[i];
    }else{
        qDebug()<<QString("request for id %1 that does not exist in team").arg(i);
        qDebug()<<"Active Agents:";for(int k=0;k<t->activeAgents.count();k++)
            qDebug()<<t->activeAgents[k];
        return -1;
    }
}
CRobot* CTeam::operator [](const int i)
{
    if (i>=0 && i<_MAX_NUM_PLAYERS) return t->teamMembers[i];
    qDebug()<<QString("id out of range %1").arg(i);
    return NULL;
}

CRobot* CTeam::active(const int i)
{
        if((i<t->activeAgents.count())&&(i>=0))
        {
            return t->teamMembers[t->activeAgents[i]];
        }else{
            qDebug()<<QString("request for id %1 that does not exist in team").arg(i);
            qDebug()<<"Active Agents:";
            for(int k=0;k<t->activeAgents.count();k++)
                qDebug()<<t->activeAgents[k];
            return NULL;
        }
}


void CTeam::setColor(ETeamColorType c)
{
    t->color = c;
}

void CTeam::updateGoaliID(int id)
{
    t->goalieID = id;
}
