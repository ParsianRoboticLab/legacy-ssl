#ifndef MIXTEAMHANDLER_H
#define MIXTEAMHANDLER_H

#include <mainapplication.h>
#include <base.h>
#include "plans/defenseplan.h"
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


    ////////slave part/////////
    void slave(bool isM);
    void initialReadPacket();
    void task3ReadPacket();
    void executeTask3MasterOffense(int, Vector2D, Vector2D, int);
    void executeTask3SlaveOffense(int, Vector2D, Vector2D);

    ///////mixed team game//////

    CRolePlayOff *robots[6];
    DefensePlan *goaliM;

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
    void oneTouchExecute(multi_team_comm::RobotPlan plan);
    bool isPathClear(Vector2D _pos1, Vector2D _pos2, double _radius, double treshold);
    void executeSlaveGoalie(multi_team_comm::RobotPlan plan);

    ////////////////test slave///////////
    bool testSlaveFlag;
    void testALL();
    void testGPA();
    void packet1();
    void packet2();
};


class CMixTeamCoach{

#define _INVALID_HEADING 100000
#define _INVALID_ID      -1

public:

    struct SPosAndHeading{
        Vector2D position;
        float heading;
    };

    struct SDangerousOpp{
        Vector2D Pos;
        double danger;
    };

    enum Role{
        DefaultRole = 0,
        GoalieRole  = 1,
        DefenseRole = 2,
        OffenseRole = 3
    };

    struct SRobotPlan{
        multi_team_comm::RobotPlan::RobotRole role;
        int id;
        Vector2D location;
        float heading;
        Vector2D shotTarget;
    };

    CKnowledge::ballPossesionState lastBallPossess;
    int defenseCount, markCount;
    int goalieID, playMakeID;

    QTime playMakeIntentionTimer;
    QList<int> ourAgents;

    CDefPos defPos;
    kkDefPos defensePos;

    QList<SPosAndHeading > markPos;

    QList<Vector2D> positioningPos;

    QList<SDangerousOpp> sortedDangerousOpp;
//    QList<Vector2D> oppPos;
    Vector2D oppPos[11];

    double markRadiusStrict;
    double ShootRatioBlock, PassRatioBlock;

    QQueue<int> ids;
    QList<int> defIds;

    CMixTeamCoach();
    void goaliePacket();
    void decideMarkAndDefenseCount();
    void setDefPositions();
    void defDynamicAssigning();
    void choosePlayMake();
    void positioning();

    void nonsenseOffense();

    void makeMasterPlanPacket();

    void testDefense();

    CKnowledge::ballPossesionState ballPossess();
    CMixTeamCoach::SPosAndHeading ShootBlockRatio(double ratio, Vector2D opp);
    CMixTeamCoach::SPosAndHeading PassBlockRatio(double ratio, Vector2D opp);

    QList<CMixTeamCoach::SDangerousOpp > sortdangerpassplayoff();

    QList<SRobotPlan> robotsPlan;

};

#endif // MIXTEAMHANDLER_H
