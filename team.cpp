#include "team.h"
#include <QDebug>

CTeam::CTeam(bool isOurTeam, bool noKalman)
{
    data = new CTeamData;
    for(int i=0;i<_MAX_NUM_PLAYERS;i++)
        data->teamMembers[i] = new CRobot(i, isOurTeam, noKalman);
    data->activeAgents.clear();
}

CTeam::~CTeam()
{
    for(int i=0;i<_MAX_NUM_PLAYERS;i++)
        delete data->teamMembers[i];
    delete data;
}

int CTeam::activeAgentsCount()
{
    return data->activeAgents.count();
}

void CTeam::update()
{
    data->activeAgents.clear();
    for( int i = 0; i < _MAX_NUM_PLAYERS; i++ )
    {
        if( data->teamMembers[i]->isActive() )
            data->activeAgents.append(i);
    }
}

int CTeam::activeAgentID(int i)
{    
    if((i<data->activeAgents.count())&&(i>=0))
    {
        return data->activeAgents[i];
    }else{
        qDebug()<<QString("request for id %1 that does not exist in team").arg(i);
        qDebug()<<"Active Agents:";for(int k=0;k<data->activeAgents.count();k++)
            qDebug()<<data->activeAgents[k];
        return -1;
    }
}
CRobot* CTeam::operator [](const int i)
{
    if (i>=0 && i<_MAX_NUM_PLAYERS) return data->teamMembers[i];
    qDebug()<<QString("id out of range %1").arg(i);
    return NULL;
}

CRobot* CTeam::active(const int i)
{
        if((i<data->activeAgents.count())&&(i>=0))
        {
            return data->teamMembers[data->activeAgents[i]];
        }else{
            qDebug()<<QString("request for id %1 that does not exist in team").arg(i);
            qDebug()<<"Active Agents:";
            for(int k=0;k<data->activeAgents.count();k++)
                qDebug()<<data->activeAgents[k];
            return NULL;
        }
}


void CTeam::setColor(ETeamColorType c)
{
    data->color = c;
}

void CTeam::updateGoaliID(int id)
{
    data->goalieID = id;
}
