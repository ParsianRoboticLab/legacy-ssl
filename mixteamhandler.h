#ifndef MIXTEAMHANDLER_H
#define MIXTEAMHANDLER_H

#include <mainapplication.h>
#include "proto/multi_team_communication.pb.h"
#include "mixteamsender.h"
#include "mixteamreader.h"

#define ROBOTS_IN_FIELD 2
#define OUR_ROBOTS_IN_FIELD 4

class CMixTeamHandler
{
public:
    CMixTeamHandler();

    int goalieID;
    int allAgentIDs[ROBOTS_IN_FIELD];
    int ourAgentIDs[OUR_ROBOTS_IN_FIELD];

    Vector2D goaliePos;
//    Vector2D ourPos[5];
//    Vector2D slavePos[5];
    Vector2D allPositions[ROBOTS_IN_FIELD];

    MixTeamReader *reader;

    ////////master part/////

    void master();
    void initialPositioning();
    void task1positioning();
    void task1MakePacket();
    void initialSlaveMakePacket();
    Vector2D getXYByAngleOurGoal(double _angle, double _radius);

    ////////slave pasrt/////
    void slave();
    void stopSlavePositioning();
    bool isOurAgent(int id);

};

#endif // MIXTEAMHANDLER_H
