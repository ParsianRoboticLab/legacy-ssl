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


    ////////slave part/////
    void slave(bool isM);
    void initialReadPacket();
    void task3ReadPacket();
    void executeMasterOffense(int, Vector2D, Vector2D, int);
    void executeSlaveOffense(int, Vector2D, Vector2D);
    //    void execute();
    //    CAgent **agents;
};


class CMixTeamCoach{
public:
    CKnowledge::ballPossesionState lastBallPossess;
    int defenseCount, markCount;
    int goalieID;

    QList<int> ourAgents;

    struct SPosAndHeading{
        Vector2D position;
        Vector2D heading;
    };

    struct SDangerousOpp{
        Vector2D Pos;
        double danger;
    };

    CDefPos defPos;
    kkDefPos tempDefPos;
    QList<Vector2D> DefensePos;
    QList<SPosAndHeading> markPos;
    QList<SDangerousOpp> sortedDangerousOpp;
    QList<Vector2D> oppPos;

    double markRadiusStrict;

    CMixTeamCoach();
    CKnowledge::ballPossesionState ballPossess();
    void decideMarkAndDefenseCount();
    void setPositions();
    void makePlanPacket();

    CMixTeamCoach::SPosAndHeading ShootBlockRatio(double ratio, Vector2D opp);
    CMixTeamCoach::SPosAndHeading PassBlockRatio(double ratio, Vector2D opp);

    QList<CMixTeamCoach::SDangerousOpp > sortdangerpassplayon(QList<Vector2D> oppposdanger);
    QList<CMixTeamCoach::SDangerousOpp > sortdangerpassplayoff(QList<Vector2D> oppposdanger);


};

#endif // MIXTEAMHANDLER_H
