#ifndef TEAM_H
#define TEAM_H
#include "base.h"
#include "robot.h"

struct CTeamData
{
    CRobot* teamMembers[_MAX_NUM_PLAYERS];
    QQueue<int> activeAgents;
    ETeamColorType color;
    ETeamSideType side;
};

class CTeam
{
public:
    CTeamData *t;
    CTeam(bool isOurTeam,bool noKalman);
    ~CTeam();
    CRobot* operator [](const int i);
	CRobot* active(const int i);
    int activeAgentsCount();
    int activeAgentID(int i);
	void update();
};

#endif // TEAM_H
