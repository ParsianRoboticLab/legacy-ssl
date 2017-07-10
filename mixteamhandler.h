#ifndef MIXTEAMHANDLER_H
#define MIXTEAMHANDLER_H

#include <mainapplication.h>
#include "proto/multi_team_communication.pb.h"
#include "mixteamsender.h"
#include "mixteamreader.h"

#define MAX_OUR_ROBOTS_IN_FIELD 5//dont change
#define MAX_ROBOTS_IN_FIELD 10//dont change

//number of robots except goalie
#define ROBOTS_IN_FIELD 8//ino baadan t GUI bayad bezarm
#define OUR_ROBOTS_IN_FIELD 4//ino baadan t GUI bayad bezarm


class CMixTeamHandler
{
public:
    CMixTeamHandler();

    int goalieID;
    int allAgentIDs[MAX_ROBOTS_IN_FIELD];
    int ourAgentIDs[MAX_OUR_ROBOTS_IN_FIELD];

    Vector2D goaliePos;
//    Vector2D ourPos[5];
//    Vector2D slavePos[5];
    Vector2D allPositions[MAX_ROBOTS_IN_FIELD];

    MixTeamReader *reader;

//    CRolePlayOff *ourRols[MAX_OUR_ROBOTS_IN_FIELD];
    CSkillGotoPoint *ourRols[MAX_OUR_ROBOTS_IN_FIELD];
    CRolePlayOff *goalieRole;

    ////////master part/////

    void master();
    void initialPositioning();
    void initialSlaveMakePacket();
    void task1positioning();
    void task1MakePacket();
    void task2positioning();
    void task2MakePacket();
    Vector2D getXYByAngleOurGoal(double _angle, double _radius);

    ////////slave pasrt/////
    void slave();
    void initialReadPacket();
    bool isOurAgent(int id);

};

#endif // MIXTEAMHANDLER_H
