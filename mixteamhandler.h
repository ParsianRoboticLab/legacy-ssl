#ifndef MIXTEAMHANDLER_H
#define MIXTEAMHANDLER_H

#include <mainapplication.h>
#include "proto/multi_team_communication.pb.h"
#include "mixteamsender.h"
#include "mixteamreader.h"

#define MAX_OUR_ROBOTS_IN_FIELD 6//dont change
#define MAX_ROBOTS_IN_FIELD 11//dont change


class CMixTeamHandler
{
public:
    CMixTeamHandler();

    int goalieID;
    QList<int> ourAgentIDs;
    int numOfAllOurRobots;

    Vector2D goaliePos;
    Vector2D allPositions[MAX_ROBOTS_IN_FIELD];

    MixTeamReader *reader;

    CSkillGotoPoint *ourRols[MAX_OUR_ROBOTS_IN_FIELD];

    ////////Constructor/////
    void setOurRobotIDs();

    ////////master part/////

    void master();
    void initialPositioning();
    void initialSlaveMakePacket();
    void task1positioning();
    void task1MakePacket();
    void task2positioning();
    void task2MakePacket();
    void task3positioning();
    void task3MakePacket();

    Vector2D getXYByAngleOurGoal(double _angle, double _radius);


    ////////slave part/////
    void slave();
    void initialReadPacket();
//    void execute();
//    CAgent **agents;
};

#endif // MIXTEAMHANDLER_H
