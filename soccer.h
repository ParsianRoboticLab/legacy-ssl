#ifndef SOCCER_H
#define SOCCER_H

#include <worldmodel.h>
#include <knowledge.h>
#include <communicator.h>
#include <visionclient.h>
#include <net/netraw.h>
#include <net/robocup_ssl_client.h>
#include <joystick.h>
#include <simulation/simulator.h>
#include <coach.h>

#include "proto/radio_protocol_command.pb.h"
#include "proto/radio_protocol_wrapper.pb.h"
#include "time.h"
class CSoccer : public QObject
{
    Q_OBJECT
public:
    enum GameMode {
        Simulation = 1,
        Real = 2,
        Spy = 3
    };
    enum ControlMode
    {
        HandyControl = 1,
        AI   = 2,
        Stop = 3
    };
    CSoccer();
    ~CSoccer();
    void run();
    void closeAll();
    void mainLoop();
    void runMainLoop();
    void connectReferee();
    void connectVision();
    void connectSerial();
    void connectSimulation();
    void connectSharedRadio();

    void setMode(GameMode _mode);
    void setTeamColor(ETeamColorType _color);
    void setTeamSide(ETeamSideType _side);
    void selectBall(Vector2D pos);
    void selectRobot(Vector2D pos);
    virtual void monitorUpdate()=0;
    virtual void customControl(bool& custom)=0;

    CAgent **agents;
    CCoach *coach;
    CCommunicator *robotCom;
    CJoystick* joystick;
    QHostAddress refSender,visSender,sender;
    QHostAddress sharedRadioSender;
    Property(ControlMode, ControlMode, controlMode);
    PropertyGet(GameMode, Mode, mode);
    PropertyGet(ETeamSideType, TeamSide, teamSide);
    PropertyGet(ETeamColorType, TeamColor, teamColor);
    clock_t realTimeTime;
    struct timeval t1, t2;
private:

    /////////////////mhmmd thread
    skillsThread *thSkill[6];
    QTime tempTime[6];
    QTimer *mainLoopTimer;
    /////////////////
    int cmdCnt, lastCmdCnt;
    GameStatePacket gsp;
    Net::UDP* refereeMC;
    QUdpSocket* refSocket;
    QUdpSocket* simulationSocket;
    quint16 senderPort;
    quint16 sharedRadioPort;
    bool doClose;
    void primaryDraws();
    void resetRoles();
    void findSupporterRoles();
    void sendPacketToSimulator();
    void sendPacketToRealWorld();

    Net::UDP* sharedRadioMC;
    QUdpSocket* sharedRadioSocket;
    void updateRadioCommand(const RadioProtocolCommand& cmd);
    void ClearCommandProtbuf(RadioProtocolCommand* cmd_ptr);
    void FillCommandProtobuf(uint32_t _id,
                             float _Vx,
                             float _Vy,
                             float _Vr,
                             float _KickSpeed,
                             float _ChipSpeed,
                             float _SpinSpeed,
                             RadioProtocolCommand* cmd_ptr);

    void SRSendPacket();

public slots:
    void refUpdate();
    void sharedRadioUpdate();
    //////////////////mhmmd thread
    void getRobotVel(int _id, double _x, double _y, double _w);
    ////////////////
    void runMonitorUpdate();
signals:
    void newPacketHalfWordlsMerged(SNewWorldModelStruct worldModel);
};


class CVisionThread : public QThread
{
    Q_OBJECT
public:
    CVisionClient *vc;
    CHalfWorld* hw;
    RoboCupSSLClient* vision;
    CProfiler profiler;
    CVisionThread();
    bool simulationMode;
    void reconnect();
    void run();
    void testFunc(SSL_WrapperPacket & packet);
    void printRobotInfo(const SSL_DetectionRobot & robot);
public slots:
    void terminated();
signals:
    void newVisionPacketReceived();
};

extern CVisionThread* visionThread;

#endif // SOCCER_H
