#ifndef CAGENT_H
#define CAGENT_H

#include "base.h"
#include "geom.h"
#include "worldmodel.h"
#include "varswidget.h"
#include "util/matrix.h"
#include "planner.h"
#include <fstream>
#include "intentions/intentions.h"

using namespace std;

#define MAX_KICK_SPEED 1023
#define new_com_test_robot_id -1

struct Fault {

};

class CSkill;
class CAgent : public QObject
{
    Q_OBJECT
public:
    class Abilities {
    public:
        Abilities();
        bool canChip;
        bool canKick;
        bool canSpin;
        bool highBattery;
        bool hasGyro;
    } abilities;

    class Status {
    public:
        Status();
        double battery;
        double capCharge; // the charge of shot capacities
        double dataLost;
        bool spin;
        bool shotSensor;
        bool fault;
        bool faild;
        bool halt;
        bool shotBoard;
        bool kickFault;
        bool chipFault;
        bool encoderFault[4];
        bool motorFault[5];
        bool beep;
        bool shotSensorFault;
        int boardID;

    } status;

    bool changeIsNeeded;

    SoccerIntention *intention;
    IntentionDefense defIntent;
    IntentionMark markIntent;
    IntentionPlayMake playMakeIntent;
    IntentionBlock blockIntent;
    IntentionPosition positionIntent;
    Vector2D homePos;
    void accelerationLimiter(double vf,bool diveMode = false);
    double goalVisibility;
    QTime agentStopTime;
    bool timerReset;
    CAgent(short int _ID);
    bool startTrain;bool stopTrain;double wh1,wh2,wh3,wh4;
    bool starter;
    bool canRecvPass;
    bool idle;
    CSkill* skill;
    QString skillName;
    int id();
    int commandId();
    void generateRobotCommand();
    bool trajectory(double& vf,double& vn,double& va,double w1,double w2,double w3,double w4,bool &stop);
    char* getOutputBuffer();
    void loadProfiles();
    double getVisibility();
    void setVisibility(const double &inSight);
    bool isVisible();
    bool notVisible();

    void setInPlayState(bool state);
    void setOnOffState(bool state);
    void setCommandID  (int ID);

    bool shootSensor();
    void setShootSensor(bool b);
    bool canOneTouch();

    //position and velocity
    Vector2D pos();
    Vector2D dir();
    double dirDegree();
    Vector2D vel();
    double angularVel();
    CRobot* self();
    Vector2D distToBall();

    //Low Level Commands
    void setRobotVel(double _vtan, double _vnorm, double _w); //vtan, vnorm in m/s and w in rad/s
    void setRobotAbsVel(double _vx, double _vy, double _w);
    void addRobotVel(double _vtan, double _vnorm, double _w);

    void setKick(double _kickSpeed);
    void setChip(double _kickSpeed);
    void setBeep(bool _beep);
    void setForceKick(bool _force);
    void setRoller(int state);
    char getRoller();
    void waitHere();

    double v1L,v2L,v3L,v4L;
    double vforwardL, vnormalL, vangularL;

    QString localName, prevLocalName;

    bool kickState, chipState, onOffState , inOutState;
    int commandID;
    float getMotorMaxRadPerSec();
    float getvLimit();
    float getwLimit();
    double v1,v2,v3,v4;
    int roller;
    double kickSpeed;

    fstream ffout;

    int sumEX,sumEY,lastEX,lastEY;

    double shotProfile[32][2];
    double chipProfile[32][2];

    double getKickValue(bool chip, bool spin, double v);
    double kickValueSpeed(double value,bool spinner);
    double kickSpeedValue(double speed,bool spinner);
    double chipValueDistance(double value,bool spinner);
    double chipDistanceValue(double distance,bool spinner);

    int kickValueForDistance( double dist, double finalVel);

    bool requestBit , chip, forceKick, forceKickCounter, lowSpeedMode;
    bool beep;
    double vforward, vnormal, vangular;
    double _ACC,_DEC;
    Vector2D oneTouchCheck(Vector2D positioningPos, Vector2D* oneTouchDir);
    vector<Vector2D> pathPlannerResult;
    Vector2D plannerAverageDir;

    void setGyroZero();
    void runPlanner(int agentId, Vector2D target, bool avoidPenaltyArea, bool avoidCenterCircle);
    void initPlanner( const int &_id , const Vector2D &_target , const QList<int> &_ourRelaxList , const QList<int> &_oppRelaxList , const bool &_avoidPenaltyArea , const bool &_avoidCenterCircle , const double &_ballObstacleRadius);
    Vector2D agentAngelForGyro;
    int calibrated;
    void jacobian(double _vx, double _vy, double _w, double &v1, double &v2, double &v3, double &v4);
private:
    void jacobianInverse(double _v1, double _v2, double _v3, double _v4,double &_vx, double &_vy, double &_w);
    bool calibrateGyro;
    unsigned int packetNum;
    double lastVf,lastVn;

    char outputBuffer[_NEW_PACKET_SIZE];
    short int selfID;
public slots:
    void getPathPlannerResult(int id , vector<Vector2D> _result , Vector2D _averageDir);
signals:
    void initPathPlanning(int agentId, Vector2D target, QList<int> _ourRelaxList, QList<int> _oppRelaxList ,  bool avoidPenaltyArea, bool avoidCenterCircle, double ballObstacleRadius);
};

#endif // CAGENT_H
