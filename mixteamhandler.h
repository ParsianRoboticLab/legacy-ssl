#ifndef MIXTEAMHANDLER_H
#define MIXTEAMHANDLER_H

#include <mainapplication.h>
#include <base.h>
#include "proto/multi_team_communication.pb.h"
#include "mixteamsender.h"
#include "mixteamreader.h"

#define MAX_OUR_ROBOTS_IN_FIELD 6//dont change
#define MAX_ROBOTS_IN_FIELD 11//dont change
#define PI 3.141592


class CMixTeamHandler
{
public:
    CMixTeamHandler();

    int goalieID;
    QList<int> ourAgentIDs;
    int numOfAllOurRobots;
    bool isMaster;

    Vector2D goaliePos;
    Vector2D allPositions[MAX_ROBOTS_IN_FIELD];

    MixTeamReader *reader;

    CSkillGotoPoint *ourRols[MAX_OUR_ROBOTS_IN_FIELD];
    CSkillKick *kicker;
    CSkillKickOneTouch *oneToucher;
    CSkillGotoPointAvoid *gpa;

    int selectedIDS;//slave ID for task3
    int selectedIDM;//master ID for task3
    Vector2D selectedPosS;//slave pos for task3
    Vector2D selectedPosM;//master pos for task3

    ////////Constructor/////
    void setOurRobotIDs();

    ////////master part/////

    void master();
    void initialPositioning();
    void initialMakePacket();
    void task1positioning();
    void task1MakePacket();
    void task2positioning();
    void task2MakePacket();
    void task3positioning();
    void task3MakePacket();
    int chooseSlaveID(int);
    int chooseMasterID(int);

    Vector2D getXYByAngleOurGoal(double _angle, double _radius);


    ////////slave part/////
    void slave(bool isM);
    void initialReadPacket();
    void task3ReadPacket();
    void executeMasterOffense(int, Vector2D, Vector2D, int);
    void executeSlaveOffense(int, Vector2D, Vector2D);
//    void execute();
//    CAgent **agents;

    ///////mixed team game//////

    CRolePlayOff *robots[6];
//    CSkillGotoPointAvoid *robots[6];

    void initialSkills();
    void mixReadPacket();

    void executePlan(multi_team_comm::RobotPlan plan, int validation);
    int isPlanValid(multi_team_comm::RobotPlan plan);
    bool isPosValid(multi_team_comm::RobotPlan plan);
    bool isPosLocValid(multi_team_comm::Location loc);
    bool isShotTargetValid(multi_team_comm::RobotPlan plan);
    bool isRoleValid(multi_team_comm::RobotPlan plan);

    float refineHeading(float heading);
    void gotopointExecute(multi_team_comm::RobotPlan plan);
    void kickExecute(multi_team_comm::RobotPlan plan);
    void receiveExecute(multi_team_comm::RobotPlan plan);


};

#endif // MIXTEAMHANDLER_H
