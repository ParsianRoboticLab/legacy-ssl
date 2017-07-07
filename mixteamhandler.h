#ifndef MIXTEAMHANDLER_H
#define MIXTEAMHANDLER_H

#include <mainapplication.h>
#include "proto/multi_team_communication.pb.h"
#include "mixteamsender.h"


class CMixTeamHandler
{
public:
    CMixTeamHandler();

    int goalieID;
    int agentIDs[10];

    Vector2D goaliePos;
    Vector2D ourPos[5];
    Vector2D slavePos[5];

    ////////master part/////

    void master();
    void initialPositioning();
    void task1positioning();
    void initialSlaveMakePacket();
    Vector2D getXYByAngleOurGoal(double _angle, double _radius);

    ////////slave pasrt/////
    void slave();
    void stopSlavePositioning();

};

#endif // MIXTEAMHANDLER_H
