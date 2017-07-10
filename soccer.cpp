#define PACK_MAX 2

#include <unistd.h>

#include "soccer.h"
#include "profiler.h"
#include <opponent.h>
#include <QString>

#include <gamelogger.h>
#include <defensepositioning.h>
#include "proto/grSim_Packet.pb.h"
#include "proto/grSim_Commands.pb.h"
#include "proto/grSim_Replacement.pb.h"
#include "exceptions.h"

#include "base.h"
#include "skillsthread.h"


QMutex worldModelMutex;
QMutex visionSocketMutex;
QMutex visionDataMutex;
QMutex visionCloseMutex;

CHalfWorld mergedHalfWorld;
double visionLatency;
double visionTimestep;
double visionFPS;
double visionProcessTime;
ETeamColorType teamColor;
ETeamSideType teamSide;
QHostAddress refSender,visSender,sender;
bool closeVision , visionClosed;

CVisionThread *visionThread;

class CVisionConfig
{
public:
    std::string addr;
    int port;
    bool reconnect;
} visionconfig;


//vision thread
//coach thread

CSoccer::CSoccer()
{
    setFramePeriod(1.0/65.0);
    setLatencyDelay(0.08);
    //    setFramePeriod(1.0/100.0);
    //    setLatencyDelay(0.0);
    halfworld = &mergedHalfWorld;
    refSocket=NULL;
    connectReferee();
    sharedRadioSocket = NULL;
    connectSharedRadio();
    wm = new CWorldModel;
    gameLogger = new CGameLogger;
    halfworldMutex = &visionDataMutex;
    loggerMutex = new QMutex;
    agents = new CAgent*[_MAX_NUM_PLAYERS];
    for(int i = 0; i < _MAX_NUM_PLAYERS; i++ )
    {
        agents[i] = new CAgent(i);
    }
    knowledge = new CKnowledge(agents);
    //    loadPlays();
    coach = new CCoach(agents);
    defensePositioning = new CDefensePositioning();
    defensePositioning->loadLookupTableFromFile("defense-1-2.lookup");
    opponent = new COpponent();

    visionThread = new CVisionThread;
    robotCom = new CCommunicator();
    robotCom->setSerialParams(115200, 8, 0, 1);
    mode = Simulation;
    controlMode = AI;
    teamColor = _COLOR_BLUE;
    teamSide = _SIDE_LEFT;
    lastCmdCnt = 0;
    cmdCnt = 0;
    //shared variables
    doClose = false;
    closeVision = false;
    visionClosed = false;
    visionconfig.reconnect = false;
    connectSimulation();
    joystick = new CJoystick();
    knowledge->setJoystic(joystick);
    pathPlanner = new CPlannerThread;
    simulator = new CSimulator;

    knowledge->antiKhafanRect = wm->field->getRegion("oppcornertop");

    qRegisterMetaType< SNewWorldModelStruct >("SNewWorldModelStruct");
    connect(this , SIGNAL(newPacketHalfWordlsMerged(SNewWorldModelStruct)) , pathPlanner , SLOT(updatePlannerWorldModel(SNewWorldModelStruct)) , Qt::QueuedConnection);
    //	connect(visionThread , SIGNAL(newVisionPacketReceived()) , this , SLOT(runMainLoop()));




    /////////////////////////////////////////////////////////////////////mhmmd thread
//    for (int i = 0; i < 6; i++) {
//            thSkill[i] = new skillsThread(this, i);
//            thSkill[i]->run();
//            thSkill[i]->setPriority(QThread::HighestPriority);
//            connect(thSkill[i],SIGNAL(robotVel(int,double,double,double)), this, SLOT(getRobotVel(int,double,double,double)));
//    tempTime[i].start();
//    }

    mainLoopTimer = new QTimer;
    mainLoopTimer->setInterval(16);
    mainLoopTimer->start();
    //connect(mainLoopTimer,SIGNAL(timeout()),this,SLOT(runMonitorUpdate()));
    /////////////////////////////////////////////////////////////////////////////////////////////////
}

void CSoccer::getRobotVel(int _id, double _x, double _y, double _w)
{

    //debug(QString("i: %1, x: %2, y: %3, w: %4, t: %5").arg(_id).arg(_x).arg(_y).arg(_w).arg(tempTime[_id].elapsed()), D_MHMMD);
    tempTime[_id].restart();
}


void CSoccer::runMonitorUpdate()
{
    monitorUpdate();
}

CSoccer::~CSoccer()
{


    //	qDebug () << "soccer closed";
    //   delete simulator;
    //   delete robCom;
    //   delete coach;
    //   delete knowledge;
    //   delete wm;
    //   delete visionThread;
    //   delete positioning;
    //   for (int i=0;i<_MAX_NUM_PLAYERS;i++)
    //	   delete agents[i];
    //   delete agents;

    // MASOUD: MUST BE CHECKED!!
    gameLogger->quit();
    pathPlanner->quit();
    joystick->quit();
    visionThread->quit();

    qDebug () << "soccer closed";
    delete simulator;
    delete pathPlanner;
    delete joystick;
    delete robotCom;
    delete visionThread;
    delete opponent;
    delete defensePositioning;
    delete coach;
    delete knowledge;
    for(int i = _MAX_NUM_PLAYERS-1; i >= 0; i-- ){
        delete agents[i];
    }
    delete agents;
    delete loggerMutex;
    delete gameLogger;


}

void CSoccer::connectSimulation()
{
    robotCom->connectUdp(conf()->LocalSettings_SimulatorAddr().c_str(),conf()->LocalSettings_SimulatorPort());
    robotCom->activateUdp();
}

void CSoccer::connectSerial()
{
    if(robotCom->isSerialConnected())
    {
        robotCom->closeSerial();
    }

    robotCom->connectSerial(conf()->LocalSettings_SerialDev().c_str());
    robotCom->activateSerial();
}

void CSoccer::connectVision()
{
    visionSocketMutex.lock();
    visionconfig.addr = conf()->LocalSettings_SSLVisionMulticastAddr();
    visionconfig.port = conf()->LocalSettings_SSLVisionMulticastPort();
    visionconfig.reconnect = true;
    visionSocketMutex.unlock();
}

void CSoccer::connectReferee()
{
    if (refSocket!=NULL)
    {
        QObject::disconnect(refSocket, SIGNAL(readyRead()), this, SLOT(refUpdate()));
        delete refSocket;
        delete refereeMC;
    }
    refSocket = new QUdpSocket(this);
    refSocket = new QUdpSocket(this);
    refereeMC = new Net::UDP();
    if(!refereeMC->open(conf()->LocalSettings_RefereeMulticastPort(),true,true,false))
        qDebug() << "Unable to open Referee UDP network port: " << conf()->LocalSettings_RefereeMulticastPort();
    else {
        Net::Address multiaddr,interface;
        multiaddr.setHost(conf()->LocalSettings_RefereeMulticastAddr().c_str(),conf()->LocalSettings_RefereeMulticastPort());
        interface.setAny();
        if(!refereeMC->addMulticast(multiaddr,interface))
            qDebug() << "Unable to setup UDP multicast for Referee";
        refSocket->setSocketDescriptor(refereeMC->getFd());
        refereeMC->setFd(-1);
    }
    connect(refSocket, SIGNAL(readyRead()), this, SLOT(refUpdate()));
    /* Qt does not support Multicast Join ... Here is a good trick using WinSocks */
    /*int sd = refSocket->socketDescriptor();
struct ip_mreq multicastRequest;
multicastRequest.imr_multiaddr.s_addr = inet_addr(_REF_MC_IP);
multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
         (char *) &multicastRequest,
         sizeof(multicastRequest));

*/
}




void CSoccer::connectSharedRadio()
{
    if (sharedRadioSocket !=NULL)
    {
        QObject::disconnect(sharedRadioSocket, SIGNAL(readyRead()), this, SLOT(sharedRadioUpdate()));
        delete sharedRadioSocket;
        delete sharedRadioMC;
    }
    sharedRadioSocket = new QUdpSocket(this);
    sharedRadioSocket = new QUdpSocket(this);
    sharedRadioMC = new Net::UDP();
    if(!sharedRadioMC->open(conf()->LocalSettings_SharedRadioMulticastPort(),true,true,false))
        qDebug() << "Unable to open Common Radio UDP network port: " << conf()->LocalSettings_SharedRadioMulticastPort();
    else {
        Net::Address multiaddr,interface;
        multiaddr.setHost(conf()->LocalSettings_SharedRadioMulticastAddr().c_str(),conf()->LocalSettings_SharedRadioMulticastPort());
        interface.setAny();
        if(!sharedRadioMC->addMulticast(multiaddr,interface))
            qDebug() << "Unable to setup UDP multicast for Common Radio";
        sharedRadioSocket->setSocketDescriptor(sharedRadioMC->getFd());
        sharedRadioMC->setFd(-1);
    }
    connect(sharedRadioSocket, SIGNAL(readyRead()), this, SLOT(sharedRadioUpdate()));
}

void CSoccer::setTeamColor(ETeamColorType _color)
{
    teamColor = _color;
    wm->setTeamColor(_color);
    visionSocketMutex.lock();
    visionThread->vc->ourColor = _color;
    visionSocketMutex.unlock();
}

void CSoccer::setTeamSide(ETeamSideType _side)
{
    teamSide = _side;
    wm->setTeamSide(_side);
    visionSocketMutex.lock();
    visionThread->vc->ourSide = _side;
    visionSocketMutex.unlock();
}

void CSoccer::setMode(GameMode _mode)
{
    mode = _mode;
    visionSocketMutex.lock();
    visionThread->simulationMode = (mode==Simulation);
    visionSocketMutex.unlock();
    if (mode==Real)
    {
        robotCom->activateSerial();
        robotCom->deactivateUdp();
    }
    else if(mode==Simulation){
        robotCom->deactivateSerial();
        robotCom->activateUdp();
    }
    else{
        robotCom->deactivateSerial();
        robotCom->deactivateUdp();
    }
    if (robotCom->errorOccured()) {
        debug(QString(robotCom->getError()), D_ERROR, QColor("red"));
    }
}

void CSoccer::primaryDraws(){

    ////////////////////////////////// Draw Section ///////////////////////////////////
    static int ballDrawVel = 0;
    ballDrawVel++;
    if( ballDrawVel == 5 ){
        ballDrawVel = 0;
        double ballDrawVelocity = wm->ball->vel.length();
        int ballVelInt = (int)ballDrawVelocity;
        int ballVelFloat = (ballDrawVelocity-ballVelInt)*100.0;
        draw(QString("BV: %1.%2").arg(ballVelInt).arg(ballVelFloat) , Vector2D(-0.5,-2.2) , "blue" , 15);
    }

    draw(QString("GS: ") + knowledge->stateToString(knowledge->getGameState()) , Vector2D(2.5, 2.55), QColor("red") , 15);
    draw(QString("GM: ") + knowledge->stateToString(knowledge->getGameMode()) , Vector2D(2.5, 2.35), QColor("blue") , 15);

    double regionWidth = 0;
    QList<int> relaxIDS;
    knowledge->getEmptyPosOnGoal(wm->ball->pos, regionWidth,true,relaxIDS,relaxIDS,1.0, true);
    knowledge->getEmptyPosOnGoal(wm->ball->pos, regionWidth,false,relaxIDS,relaxIDS,1.0, true);
    ///////////////////////////////////////////////////////////////////////////////////

}

void CSoccer::resetRoles(){
    ///////////////////////////////// Reset Mark Info /////////////////////////////////
    CRoleMarkInfo *markInfo = (CRoleMarkInfo*) CSkills::getInfo("mark");
    markInfo->markedOpp.clear();
    ///////////////////////////////////////////////////////////////////////////////////
}

void CSoccer::findSupporterRoles(){

    QList <CRobot*> oppsToMark;
    oppsToMark.clear();
    for(int i = 0 ; i < wm->opp.activeAgentsCount() ; i++)
        oppsToMark.append(wm->opp.active(i));


    QList <int> oppDefenses;
    int dummy;

    knowledge->findOppDefenders(oppDefenses,dummy);

    for(int i =0; i < oppDefenses.count() ; i++)
    {
        oppsToMark.removeOne(wm->opp[oppDefenses[i]]);
    }

    if(knowledge->findOppGoalie() != -1)
        oppsToMark.removeOne(wm->opp[knowledge->findOppGoalie()]);

    int nearestToBall = 0;
    double nearestToBallDist = 100000;

    for(int i = 0 ; i < oppsToMark.count() ; i++)
    {
        if(oppsToMark[i]->pos.dist(wm->ball->pos) < nearestToBallDist)
        {
            nearestToBall = oppsToMark[i]->id;
            nearestToBallDist = oppsToMark[i]->pos.dist(wm->ball->pos);
        }
    }
    if(wm->opp[nearestToBall]->pos.dist(wm->ball->pos) && !knowledge->transientFlag)
    {
        oppsToMark.removeOne(wm->opp[nearestToBall]);
    }
    knowledge->toBeMopps.clear();
    knowledge->toBeMopps.append(oppsToMark);


    knowledge->variables["markable"] = QString("%1").arg(knowledge->toBeMopps.size());
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    return;
    ///////////////////////////in the previous cycles choose supporters from history///////////////////////////////
    for( int j=1 ; j<=_MAX_NUM_PLAYERS ; j++ ){
        if( wm->oppSupporterHist[j].size() > 80 )
            wm->oppSupporterHist[j].pop_front();
        bool myFlag = true;
        for( int i=0 ; i<wm->opp.activeAgentsCount() ; i++ ){
            if( wm->opp.active(i)->role.toLower() == QString("supporter%1").arg(j) ){
                wm->oppSupporterHist[j].push_back(wm->opp.active(i)->id);
                myFlag = false;
                break;
            }
        }
        if( myFlag ){
            wm->oppSupporterHist[j].push_back(_MAX_NUM_PLAYERS+1);
        }
    }

    knowledge->toBeMopps.clear();
    for (int i = 0 ; i <= _MAX_NUM_PLAYERS ; i++){
        int j = coach->mostSupporterNumber(i);
        if( j != -1 ){
            knowledge->toBeMopps.append(wm->opp[j]);
            wm->opp[j]->markedByDefense = false;
            wm->opp[j]->markedByMark = false;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////if there's extra agent for mark send it to mark nearest_to_ball opponent agent///////////////////////////////////////////
    if( wm->ball->vel.length() > 0.5 && knowledge->nearestOppToBall>=0 && knowledge->nearestOppToBall<_MAX_NUM_PLAYERS && wm->opp[knowledge->nearestOppToBall]->isActive()){
        Ray2D ballRay(wm->ball->pos , wm->ball->vel.norm());
        Circle2D circ(wm->opp[knowledge->nearestOppToBall]->pos , 0.6);
        Vector2D inter[2] , intersect;
        int num = circ.intersection(ballRay , &inter[0] , &inter[1]);
        if( num == 2 ){
            intersect = inter[0].dist(wm->ball->pos) < inter[1].dist(wm->ball->pos) ? inter[0] : inter[1] ;
            knowledge->toBeMopps.append(wm->opp[knowledge->nearestOppToBall]);
        }
        else if( num == 1 ){
            intersect = inter[0];
            knowledge->toBeMopps.append(wm->opp[knowledge->nearestOppToBall]);
        }
    }


}

void CSoccer::sendPacketToSimulator(){
    grSim_Packet packet;
    bool yellow = false;
    if (wm->getTeamColor() == TEAM_YELLOW) yellow = true;
    packet.mutable_commands()->set_isteamyellow(yellow);
    packet.mutable_commands()->set_timestamp(0.0);

    for( int i = 0; i < _NUM_PLAYERS; i++ )
    {
        if( agents[i]->notVisible() )
            continue;
        grSim_Robot_Command* command = packet.mutable_commands()->add_robot_commands();
        command->set_id(i);
        const double gain = 1.013; //to match simulator with code [use parsianNew.ini in simulator 0.845 if you use parsian.ini (old robots)]
        //			const double gain = 2*1.068; //to match simulator with code [use parsianNew.ini in simulator 0.845 if you use parsian.ini (old robots)]


        double w1 = agents[i]->v1*gain;
        double w2 = agents[i]->v2*gain;
        double w3 = agents[i]->v3*gain;
        double w4 = agents[i]->v4*gain;

        command->set_wheelsspeed(false);
        command->set_wheel1(0);
        command->set_wheel2(0);
        command->set_wheel3(0);
        command->set_wheel4(0);

        command->set_velangular(agents[i]->vangular*_DEG2RAD);
        command->set_velnormal(agents[i]->vnormal);
        command->set_veltangent(agents[i]->vforward);
        command->set_kickspeedx(agents[i]->kickSpeed);
        if (agents[i]->chip){
            command->set_kickspeedz(agents[i]->kickSpeed);
        }
        else
            command->set_kickspeedz(0);
        if (0)//agents[i]->roller)
            command->set_spinner(true);
        else
            command->set_spinner(false);
    }
    if (wm->ball->getReplaced())
    {
        packet.mutable_replacement()->mutable_ball()->set_x(wm->ball->getReplPos().x);
        packet.mutable_replacement()->mutable_ball()->set_y(wm->ball->getReplPos().y);
        packet.mutable_replacement()->mutable_ball()->set_vx(wm->ball->getReplVel().x);
        packet.mutable_replacement()->mutable_ball()->set_vy(wm->ball->getReplVel().y);
        wm->ball->setReplaced(false);
    }
    for( int i = 0; i < _NUM_PLAYERS; i++ )
    {
        if (wm->our[i]->getReplaced())
        {
            grSim_RobotReplacement* repl = packet.mutable_replacement()->add_robots();
            repl->set_id(i);
            if (wm->getTeamColor() == TEAM_YELLOW)
                repl->set_yellowteam(true);
            else
                repl->set_yellowteam(false);
            repl->set_x(wm->our[i]->getReplPos().x);
            repl->set_y(wm->our[i]->getReplPos().y);
            repl->set_dir(wm->our[i]->getReplPos().dir().degree());
        }

        wm->our[i]->setReplaced(false);
    }
    for( int i = 0; i < _NUM_PLAYERS; i++ )
    {
        if (wm->opp[i]->getReplaced())
        {
            grSim_RobotReplacement* repl = packet.mutable_replacement()->add_robots();
            repl->set_id(i);
            if (wm->getTeamColor() == TEAM_YELLOW)
                repl->set_yellowteam(false);
            else
                repl->set_yellowteam(true);
            repl->set_x(wm->opp[i]->getReplPos().x);
            repl->set_y(wm->opp[i]->getReplPos().y);
            repl->set_dir(wm->opp[i]->getReplPos().dir().degree());
        }
        wm->opp[i]->setReplaced(false);
    }
    std::string s;
    packet.SerializeToString(&s);
    if (!robotCom->isSerialConnected())
        robotCom->sendString(s.c_str(), s.size());
}

void CSoccer::sendPacketToRealWorld(){
    QByteArray cmdPacket;
#ifndef newProtocol
    for( int i = 9; i > -1; i-- )
    {
        if(!agents[i]->onOffState){
            continue;
        }


        cmdPacket.append(agents[i]->getOutputBuffer(), _PACKET_SIZE);

        //Calibrating Gyro if robot is stopped
        //			debug(QString("if %1 Vel : %2 Angular : %3").arg(( agents[i]->abilities.hasGyro && agents[i]->vel().length() < 0.003 && fabs(agents[i]->angularVel()) < 0.0003)).arg(agents[i]->vel().length()).arg(fabs(agents[i]->angularVel())),D_SEPEHR);
        //		if( agents[i]->abilities.hasGyro && agents[i]->vel().length() < 0.03 && fabs(agents[i]->angularVel()) < 0.03)
        agents[i]->setGyroZero();
    }

    for (int i=0;i<cmdPacket.count();i+=_PACKET_SIZE)
    {
        robotCom->send->sendString(cmdPacket.data() + i, _PACKET_SIZE);
    }
#else
    for( int i = 9; i > -1; i-- )
    {
//        if(!agents[i]->onOffState){
//            continue;
//        }


        cmdPacket.append(agents[i]->getOutputBuffer(), _NEW_PACKET_SIZE);

        //Calibrating Gyro if robot is stopped
        //			debug(QString("if %1 Vel : %2 Angular : %3").arg(( agents[i]->abilities.hasGyro && agents[i]->vel().length() < 0.003 && fabs(agents[i]->angularVel()) < 0.0003)).arg(agents[i]->vel().length()).arg(fabs(agents[i]->angularVel())),D_SEPEHR);
        //		if( agents[i]->abilities.hasGyro && agents[i]->vel().length() < 0.03 && fabs(agents[i]->angularVel()) < 0.03)
        agents[i]->setGyroZero();
    }

    for (int i=0;i<cmdPacket.count();i+=_NEW_PACKET_SIZE)
    {
        robotCom->sendString(cmdPacket.data() + i, _NEW_PACKET_SIZE);
    }
#endif
}

void CSoccer::runMainLoop(){

    mainLoop();

}

void CSoccer::mainLoop()
{
    double t = CProfiler::getTime();
    static double lastCommandTime = CProfiler::getTime();
    double dt = t - lastCommandTime;
    lastCommandTime = t;

    QTime timer;
    timer.start();

    knowledge->frameCount++;

    primaryDraws();

    resetRoles();

    wm->setIsSimulMode(mode==Simulation);
    wm->setCommandTimeStep(dt);
    ////////////////set opponets roles (goalie, defense, supporter (naive))////////////////
    opponent->findRoles();
    ////////////////////////////////

    //////////////////set opponents roles more specificly and set priority for each of which////////////////////
    coach->setOpponents();
    //////////////////////////////////////

    findSupporterRoles();

    // when the area is occluded and the ball is invisible sets the ball position as nearest robot kickers position
    wm->unmaskOccludedBall();

    ///execute coach if there's no custom task(otherwise it will do the proper custom task) and controlMode is AI///
    int probeid = wm->profiler->putProbe("Cycle");
    knowledge->calculateCommandFrameRate();
    //  debug(QString("%1) MainLoop Time1: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
    timer.restart();
    if( mode != Spy && controlMode == AI && knowledge->frameCount > 50 )
    {
        for (int i=0;i<_NUM_PLAYERS;i++)
        {
            agents[i]->waitHere();
        }
        bool custom = false;
        customControl(custom);
        if (!custom)
        {
            try {
                coach->execute();
            } catch (Exception* e)
            {
                debug(QString("Exception: %1").arg(e->toString()), D_ERROR);
            }
        }
    }
    ////////////////////////////////////////////////////////////////////

    //  debug(QString("%1) MainLoop Time2: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
    timer.restart();


    ////////////////////////////////if simulation mode is running send packets to simulator/////////////////////////////////////
    if( mode == Simulation ){
        sendPacketToSimulator();
    }
    else//////if real mode is running send packets to agents by communication module
    {
        sendPacketToRealWorld();
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //  debug(QString("%1) MainLoop Time3: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
    timer.restart();

    knowledge->loopTime = wm->profiler->takeProbe(probeid);
    knowledge->visionProcessTime = visionProcessTime;
    if (knowledge->loopTime > knowledge->maxLoopTime)
        knowledge->maxLoopTime = knowledge->loopTime;
}


void CSoccer::run()
{
    visionconfig.port = conf()->LocalSettings_SSLVisionMulticastPort();
    visionconfig.addr = conf()->LocalSettings_SSLVisionMulticastAddr();
    visionThread->start(QThread::NormalPriority);
    CProfiler p;
    double lastMainLoopRunTime=-1;
    double lastMonitorUpdateTime=-1;
    pathPlanner->start(QThread::HighPriority);
#ifndef NO_JS
//    joystick->start(QThread::LowPriority);
#endif

    QTime timer;
    timer.start();
    while (!doClose)
    {
        usleep(1000);


        //  debug(QString("%1) Soccer Time1: %2").arg(knowledge->frameCount).arg(((quint64)(CProfiler::getTime()*1000))%1000000) , D_MASOOD);

        ///////////// Important line /////////////
        ////// slots will execute here ///////////
        ////// gui gets update here ///////////////
        qApp->processEvents();

        //    debug(QString("%1) Soccer Time2: %2").arg(knowledge->frameCount).arg(((quint64)(CProfiler::getTime()*1000))%1000000) , D_MASOOD);

        //    if( timer.elapsed()>100 ){
        //      debug(QString("%1) RunSoccer Time1: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
        //      timer.restart();
        //    }
        //    debug(QString("%1) RunSoccer Time2: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
        //    timer.restart();
        ///////////////////////////world model update////////////////////////////
        visionDataMutex.lock();
        mergedHalfWorld.game_state = knowledge->getGameState();
        mergedHalfWorld.game_mode = knowledge->getGameMode();
        mergedHalfWorld.closing = doClose;
        for (int i=0; i< knowledge->agentCount();i++)
        {
            mergedHalfWorld.ourRole[i] = knowledge->getAgent(i)->skillName;
        }
        for (int i=0; i< _MAX_NUM_PLAYERS;i++)
        {
            mergedHalfWorld.oppRole[i] = wm->opp[i]->role;
        }
        mergedHalfWorld.gsp = gsp;
        mergedHalfWorld.knowledgeVars = knowledge->variables;
        for (int i=0;i<_MAX_NUM_PLAYERS;i++)
            knowledge->positioningPoints[i] = mergedHalfWorld.positioningPoints[i];
        knowledge->positioningPointsCount = mergedHalfWorld.positioningPointsCount;
        wm->update(&mergedHalfWorld);
        wm->setVisionLatancy(visionLatency);
        wm->setVisionTimeStep(visionTimestep);
        wm->setVisionFPS(visionFPS);
        if (knowledge->getPlayMaker() == NULL)
        {
            mergedHalfWorld.playmakerID = -1;
        }
        else mergedHalfWorld.playmakerID = knowledge->getPlayMaker()->id();
        visionDataMutex.unlock();

        //////////////////


        //    debug(QString("%1) RunSoccer Time3: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
        //    timer.restart();


        SNewWorldModelStruct temp;
        temp.ball.pos = wm->ball->pos;
        temp.ball.vel = wm->ball->vel;
        for( int i=0 ; i<wm->our.activeAgentsCount() ; i++ ){
            SNewWorldModelStructRobot tempRobot(wm->our.active(i)->pos , wm->our.active(i)->vel , wm->our.active(i)->id);
            temp.our.append(tempRobot);
        }
        for( int i=0 ; i<wm->opp.activeAgentsCount() ; i++ ){
            SNewWorldModelStructRobot tempRobot(wm->opp.active(i)->pos , wm->opp.active(i)->vel , wm->opp.active(i)->id);
            temp.opp.append(tempRobot);
        }
        temp.field = *(wm->field);
        ///////////////////


        //    debug(QString("%1) RunSoccer Time4: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
        //    timer.restart();

        emit newPacketHalfWordlsMerged(temp);

        ////////////////// run mainLoop in Fixed-Interval ///////////////////////
        double current_time = p.getTime();

        if ( current_time-lastMainLoopRunTime > conf()->Common_Main_Loop_Interval()/1000.0)
        {


            double elapsedTime;
            ////////////////////////////////////////////////// by mhmmd
            runMainLoop();
            lastMainLoopRunTime = current_time;

            SRSendPacket();
            realTimeTime =  clock() -realTimeTime;
            gettimeofday(&t2, NULL);

            /////////////////////////////////////////////////
            elapsedTime = (t2.tv_sec - t1.tv_sec) ;      // sec to ms
            elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000000.0;   // us to ms
            knowledge->mainLoopTime  = elapsedTime;
            gettimeofday(&t1, NULL);
            knowledge->getRealBallVel();
        }


        //    debug(QString("%1) RunSoccer Time5: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
        //    timer.restart();

        ///////////////////// Monitor Update Section ///////////////////////////
        current_time = p.getTime();
        if (current_time-lastMonitorUpdateTime > conf()->Common_Monitor_Interval()/1000.0)
        {
            lastMonitorUpdateTime = current_time;
            monitorUpdate();
        }

        //    debug(QString("%1) RunSoccer Time6: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
        //    timer.restart();

    }


    if( loggerMutex->tryLock(20) ){
        gameLogger->closeLogger = true;
        gameLogger->setIsLogMode(false);
        gameLogger->logMode = false;
        gameLogger->closeLogFiles(false);
        loggerMutex->unlock();
        QTime tm;
        tm.start();
        while( tm.elapsed() < 20 )
        {
            bool flag=false;
            if( loggerMutex->tryLock(1) ){
                flag = gameLogger->loggerClosed;
                loggerMutex->unlock();
            }
            if (flag) break;
        }
    }


    visionSocketMutex.lock();
    closeVision = true;
    visionSocketMutex.unlock();
    while (1)
    {
        bool flag=false;
        visionCloseMutex.lock();
        flag = visionClosed;
        visionCloseMutex.unlock();
        if (flag) break;
    }


    qDebug () << "main thread ended.";
}

void CSoccer::refUpdate()
{
    QByteArray datagram;
    char refCommand;

    while (refSocket->hasPendingDatagrams())
    {
        datagram.resize(refSocket->pendingDatagramSize());
        refSocket->readDatagram(datagram.data(), datagram.size(), &refSender, &senderPort);
    }


    SSL_Referee referee;
    if( !referee.ParseFromArray(datagram, datagram.size())){
        debug("error parsing protobuf", D_GAME);
        return;
    }

    gsp.cmd = compute_command(referee);
    gsp.cmd_counter = referee.command_counter();
    gsp.goals_blue = referee.blue().score();
    gsp.goals_yellow = referee.yellow().score();
    gsp.time_remaining = referee.stage_time_left();
    cmdCnt = gsp.cmd_counter;

    if( gsp.cmd == 'b' || gsp.cmd == 'B' ){
        SSL_Referee_Point desPos;
        desPos = referee.designated_position();
        knowledge->setBPPosition(desPos.x(), desPos.y());
    }

    //gets goalie id for both teams
    if( wm->getTeamColor() == _COLOR_BLUE){
        wm->our.updateGoaliID((int)referee.blue().goalie());
        wm->opp.updateGoaliID((int)referee.yellow().goalie());
    }
    else{
        wm->our.updateGoaliID((int)referee.yellow().goalie());
        wm->opp.updateGoaliID((int)referee.blue().goalie());
    }

    if( cmdCnt == lastCmdCnt)
        return;

    lastCmdCnt = cmdCnt;
    refCommand = gsp.cmd;
    qDebug() << "REF: " << referee.command();
    qDebug() << "ref: " << refCommand;
    QFile file("./REF.dat");
    if (file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        QTextStream stream( &file );
        stream << referee.command() << endl;
    }

    wm->gs->transition(refCommand);
    debug(( "Referee : " + QString("%1 (%2)").arg(refCommand).arg((int) refCommand) + "  " + QString::number(wm->gs->get())), D_GAME);
    debug(("***** STATE *****"), D_GAME);
    debug(("Yellow : " + QString::number(gsp.goals_yellow) + "  Blue : " + QString::number(gsp.goals_blue)), D_GAME);
    debug(("Time remaining : " + QString::number(gsp.time_remaining)), D_GAME);
    while(wm->refCommand.count()>0) wm->refCommand.removeFirst();
    if( wm->gs->allowedNearBall() )
    {
        debug(("\tallowedNearBall"), D_GAME);
    }
    if(wm->gs->canKickBall())
    {
        debug(("\tcanKickBall"), D_GAME);
    }
    if(wm->gs->canMove())
    {
        debug(("\tcanMove"), D_GAME);
    }
    if(wm->gs->directKick())
    {
        debug(("\tdirectKick"), D_GAME);
    }
    if(wm->gs->freeKick())
    {
        debug(("\tfreeKick"), D_GAME);
    }
    if(wm->gs->gameOn())
    {
        debug(("\tgameOn"), D_GAME);
    }
    if(wm->gs->indirectKick())
    {
        debug(("\tindirectKick"), D_GAME);
    }
    if(wm->gs->ourKickoff())
    {
        debug(("\tourKickoff"), D_GAME);
    }
    if(wm->gs->ourPenaltyKick())
    {
        debug(("\tourPenaltyKick"), D_GAME);
    }
    if(wm->gs->ourRestart())
    {
        debug(("\tourRestart"), D_GAME);
    }
    if(wm->gs->penaltyKick())
    {
        debug(("\tpenaltyKick"), D_GAME);
    }
    if(wm->gs->theirFreeKick())
    {
        debug(("\ttheirFreeKick"), D_GAME);
    }
    if(wm->gs->theirDirectKick())
    {
        debug(("\ttheirDirectKick"), D_GAME);
    }
    if(wm->gs->theirIndirectKick())
    {
        debug(("\ttheirIndirectKick"), D_GAME);
    }
    if(wm->gs->theirKickoff())
    {
        debug(("\ttheirKickoff"), D_GAME);
    }
    if(wm->gs->theirPenaltyKick())
    {
        debug(("\ttheirPenaltyKick"), D_GAME);
    }
    if(wm->gs->theirRestart())
    {
        debug(("\ttheirRestart"), D_GAME);
    }

    debug(("****************"), D_GAME);

    //
    if (wm->gs->gameOn())
    {
        wm->refCommand.enqueue("Normal Play");
    }
    else
    {

        if (!wm->gs->canMove())
        {
            wm->refCommand.enqueue("Halt");
        }
        else
        {
            if (wm->gs->ourKickoff())
            {
                wm->refCommand.enqueue("Our KickOff");
            }
            else if (wm->gs->theirKickoff())
            {
                wm->refCommand.enqueue("Their KickOff");
            }
            else if (wm->gs->ourDirectKick())
            {
                wm->refCommand.enqueue("Our Indirect");
            }
            else if (wm->gs->theirDirectKick())
            {
                wm->refCommand.enqueue("Their Indirect");
            }
            else if (wm->gs->ourIndirectKick())
            {
                wm->refCommand.enqueue("Our Indirect");
            }
            else if (wm->gs->theirIndirectKick())
            {
                wm->refCommand.enqueue("Their Indirect");
            }
            else if (wm->gs->ourPenaltyKick())
            {
                wm->refCommand.enqueue("Our Penalty");
            }
            else if (wm->gs->theirPenaltyKick())
            {
                wm->refCommand.enqueue("Their Penalty");
            }
            //added
            else if (wm->gs->ourBallPlacement())
            {
                wm->refCommand.enqueue("Our BallPlacement");
            }
            else if (wm->gs->theirBallPlacement())
            {
                wm->refCommand.enqueue("Their BallPlacement");
            }
            else if (wm->gs->halfTimeLineUp())
            {
                wm->refCommand.enqueue("HalfTime LineUp");
            }

            else
            {
                wm->refCommand.enqueue("Make Circle");
            }
        }
    }
}


///////////////////MAPPING COMMAND AND STAGE//////////////
char CSoccer::map_stage(SSL_Referee::Stage stage){
    switch(stage){
        case SSL_Referee::NORMAL_FIRST_HALF_PRE: return '1';
        case SSL_Referee::NORMAL_FIRST_HALF: return ' ';
        case SSL_Referee::NORMAL_HALF_TIME: return 'h';
        case SSL_Referee::NORMAL_SECOND_HALF_PRE: return '2';
        case SSL_Referee::NORMAL_SECOND_HALF: return ' ';
        case SSL_Referee::EXTRA_TIME_BREAK: return 'h';
        case SSL_Referee::EXTRA_FIRST_HALF_PRE: return 'o';
        case SSL_Referee::EXTRA_FIRST_HALF: return ' ';
        case SSL_Referee::EXTRA_HALF_TIME: return 'h';
        case SSL_Referee::EXTRA_SECOND_HALF_PRE: return 'O';
        case SSL_Referee::EXTRA_SECOND_HALF: return ' ';
        case SSL_Referee::PENALTY_SHOOTOUT_BREAK: return 'h';
        case SSL_Referee::PENALTY_SHOOTOUT: return 'a';
        case SSL_Referee::POST_GAME: return 'h';
    }
    //return error
}

char CSoccer::map_command(SSL_Referee::Command command){
    switch(command){
        case SSL_Referee::HALT: return 'H';
        case SSL_Referee::STOP: return 'S';
        case SSL_Referee::NORMAL_START: return ' ';
        case SSL_Referee::FORCE_START: return 's';
        case SSL_Referee::PREPARE_KICKOFF_YELLOW: return 'k';
        case SSL_Referee::PREPARE_KICKOFF_BLUE: return 'K';
        case SSL_Referee::PREPARE_PENALTY_YELLOW: return 'p';
        case SSL_Referee::PREPARE_PENALTY_BLUE: return 'P';
        case SSL_Referee::DIRECT_FREE_YELLOW: return 'f';
        case SSL_Referee::DIRECT_FREE_BLUE: return 'F';
        case SSL_Referee::INDIRECT_FREE_YELLOW: return 'i';
        case SSL_Referee::INDIRECT_FREE_BLUE: return 'I';
        case SSL_Referee::TIMEOUT_YELLOW: return 't';
        case SSL_Referee::TIMEOUT_BLUE: return 'T';
        case SSL_Referee::GOAL_YELLOW: return 'g';
        case SSL_Referee::GOAL_BLUE: return 'G';
        case SSL_Referee::BALL_PLACEMENT_YELLOW: return 'b';
        case SSL_Referee::BALL_PLACEMENT_BLUE: return 'B';
    }
    //return error
}
char CSoccer::compute_command(SSL_Referee refSC){
    enum Disposition{
        STAGE,
        COMMAND,
        YELLOW_YCARD,
        BLUE_YCARD,
        YELLOW_RCARD,
        BLUE_RCARD,
        CACHE
    };

    Disposition disposition;

    if (refSC.stage() != last_stage) {
            disposition = STAGE;
    } else if (refSC.command() != last_command) {
            disposition = COMMAND;
    } else if (refSC.yellow().yellow_card_times_size() > last_yellow_ycards) {
            disposition = YELLOW_YCARD;
    } else if (refSC.blue().yellow_card_times_size() > last_blue_ycards) {
            disposition = BLUE_YCARD;
    } else if (refSC.yellow().red_cards() > last_yellow_rcards) {
            disposition = YELLOW_RCARD;
    } else if (refSC.blue().red_cards() > last_blue_rcards) {
            disposition = BLUE_RCARD;
    } else {
            disposition = CACHE;
    }

    last_stage = refSC.stage();
    last_command = refSC.command();
    last_yellow_ycards = refSC.yellow().yellow_card_times_size();
    last_blue_ycards = refSC.blue().yellow_card_times_size();
    last_yellow_rcards = refSC.yellow().red_cards();
    last_blue_ycards = refSC.blue().red_cards();

    switch (disposition) {
            case STAGE: return cached_command_char = map_stage(refSC.stage());
            case COMMAND: return cached_command_char = map_command(refSC.command());
            case YELLOW_YCARD: return cached_command_char = 'y';
            case BLUE_YCARD: return cached_command_char = 'Y';
            case YELLOW_RCARD: return cached_command_char = 'r';
            case BLUE_RCARD: return cached_command_char = 'R';
            case CACHE: return cached_command_char;
    }
    //return error
    return '!';
}
///////////////////////END MAPING////////////////////



void CSoccer::sharedRadioUpdate()
{
    if(!conf()->LocalSettings_SharedRadioReceive())
        return;
    QByteArray datagram;

    while (sharedRadioSocket->hasPendingDatagrams())
    {
        datagram.resize(sharedRadioSocket->pendingDatagramSize());
        sharedRadioSocket->readDatagram(datagram.data(), datagram.size(), &sharedRadioSender, &sharedRadioPort);
    }

    RadioProtocolWrapper radio_message;

    //debug(QString(datagram.data()).toLatin1(), D_KK);
    //strcpy(buffer, datagram.data());
    //radio_message.ParseFromString(
    std::string stdString(datagram.constData(), datagram.length());
    if( radio_message.ParseFromString(stdString) )
    {
        const int num_robots = radio_message.command_size();
        for (int i = 0; i < num_robots; ++i)
        {
            updateRadioCommand(radio_message.command(i));
        }
    }
    else
    {
        debug("Error parsing protobuf \n",D_KK);
    }
}

void CSoccer::updateRadioCommand(const RadioProtocolCommand &cmd)
{
    int tempId = cmd.robot_id();
    double tempVal[6];
    tempVal[0] = cmd.velocity_x();
    tempVal[1] = cmd.velocity_y();
    tempVal[2] = cmd.velocity_r();

    if (cmd.has_flat_kick())
        tempVal[3] = cmd.flat_kick();
    else
        tempVal[3] = 0;
    if (cmd.has_chip_kick())
        tempVal[4] = cmd.chip_kick();
    else
        tempVal[4] = 0;

    if (cmd.has_dribbler_spin())
        tempVal[5] = cmd.dribbler_spin();
    else
        tempVal[5] = 0;

    knowledge->SRSetAgentArg(tempId,
                             tempVal[0],
                             tempVal[1],
                             tempVal[2],
                             tempVal[3],
                             tempVal[4],
                             tempVal[5]);
}

void CSoccer::ClearCommandProtbuf(RadioProtocolCommand* cmd_ptr)
{
    RadioProtocolCommand& cmd = *cmd_ptr;
    cmd.set_robot_id(0);
    cmd.set_velocity_x(0);
    cmd.set_velocity_y(0);
    cmd.set_velocity_r(0);
}

void CSoccer::FillCommandProtobuf(uint32_t _id, float _Vx, float _Vy, float _Vr, float _KickSpeed, float _ChipSpeed, float _SpinSpeed, RadioProtocolCommand *cmd_ptr)
{
    RadioProtocolCommand& cmd = *cmd_ptr;
    cmd.set_robot_id(_id);
    cmd.set_velocity_x(_Vx);
    cmd.set_velocity_y(_Vy);
    cmd.set_velocity_r(_Vr);

    if(_KickSpeed != 0)
        cmd.set_flat_kick(_KickSpeed);

    if(_ChipSpeed != 0)
        cmd.set_chip_kick(_ChipSpeed);

    if(_SpinSpeed != 0)
        cmd.set_dribbler_spin(_SpinSpeed);
}

void CSoccer::SRSendPacket()
{
    if(!conf()->LocalSettings_SharedRadioEnable())
        return;

    if(sharedRadioSocket == NULL)
        return;

    bool check = false;
    for(int i =0; i < 8; i++)
        check = true;

    if(!check) return;

    SRAgentArgs temp;

    RadioProtocolWrapper wrapper;
    RadioProtocolCommand command;

    QHostAddress tempHost(QString::fromStdString(conf()->LocalSettings_SharedRadioMulticastAddr()));

    std::string buffer;

    wrapper.clear_command();
    command.Clear();
    for(int i = 0; i < _MAX_NUM_PLAYERS; i++)
    {
        if( knowledge->SRGetAgentArg(i, temp) )
        {

            ClearCommandProtbuf(&command);
            FillCommandProtobuf(i,
                                temp.Vx,
                                temp.Vy,
                                temp.Vr,
                                temp.KickSpeed,
                                temp.ChipSpeed,
                                temp.SpinSpeed,
                                &command);
            *wrapper.add_command() = command;
        }
    }

    wrapper.SerializeToString(&buffer);
    QByteArray tempBytes;
    tempBytes.append(QString::fromStdString(buffer));

    sharedRadioSocket->writeDatagram(tempBytes,
                                     qint64(tempBytes.size()),
                                     tempHost,
                                     quint16(conf()->LocalSettings_SharedRadioMulticastPort()) );

}

void CSoccer::closeAll()
{
    doClose = true;
}

void CSoccer::selectBall(Vector2D pos)
{
    visionDataMutex.lock();
    mergedHalfWorld.selectBall(pos);
    visionDataMutex.unlock();
}


void CSoccer::selectRobot(Vector2D pos)
{
    knowledge->selectedId = -1;
    for (int i=0;i<wm->our.activeAgentsCount();i++)
    {
        if ((pos - wm->our.active(i)->pos).length() < CRobot::robot_radius_old)
        {
            knowledge->selectedId = wm->our.active(i)->id;
            knowledge->selectedOur = true;
        }
    }
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
        if ((pos - wm->opp.active(i)->pos).length() < CRobot::robot_radius_old)
        {
            knowledge->selectedId = wm->opp.active(i)->id;
            knowledge->selectedOur = false;
        }
    }
}

void CVisionThread::reconnect()
{
    qDebug() << "Connecting!!!";
    visionDataMutex.lock();
    mergedHalfWorld.ball.clear();
    for (int i=0;i<_NUM_PLAYERS;i++)
    {
        mergedHalfWorld.ourTeam[i].clear();
        mergedHalfWorld.oppTeam[i].clear();
    }
    visionDataMutex.unlock();
    if (vision != NULL)
    {
        delete vision;
    }
    vision = new RoboCupSSLClient(visionconfig.port, visionconfig.addr.c_str());
    if (!vision->open(false))
    {
        qDebug () << "Connection Failed.";
    }
    else qDebug() << "Connected!";
}

CVisionThread::CVisionThread()
{
    vc = new CVisionClient();
    vc->ourSide = _SIDE_LEFT;
    simulationMode = true;
}

void CVisionThread::printRobotInfo(const SSL_DetectionRobot &robot)
{
    printf("CONF=%4.2f ", robot.confidence());
    if (robot.has_robot_id()) {
        printf("ID=%3d ",robot.robot_id());
    } else {
        printf("ID=N/A ");
    }
    printf(" HEIGHT=%6.2f POS=<%9.2f,%9.2f> ",robot.height(),robot.x(),robot.y());
    if (robot.has_orientation()) {
        printf("ANGLE=%6.3f ",robot.orientation());
    } else {
        printf("ANGLE=N/A    ");
    }
    printf("RAW=<%8.2f,%8.2f>\n",robot.pixel_x(),robot.pixel_y());
}

void CVisionThread::testFunc(SSL_WrapperPacket & packet)
{
    printf("-----Received Wrapper Packet---------------------------------------------\n");
    //see if the packet contains a robot detection frame:
    if (packet.has_detection()) {
        SSL_DetectionFrame detection = packet.detection();
        //Display the contents of the robot detection results:
        double t_now /*= GetTimeSec()*/;

        printf("-[Detection Data]-------\n");
        //Frame info:
        printf("Camera ID=%d FRAME=%d T_CAPTURE=%.4f\n",detection.camera_id(),detection.frame_number(),detection.t_capture());

        printf("SSL-Vision Processing Latency                   %7.3fms\n",(detection.t_sent()-detection.t_capture())*1000.0);
        printf("Network Latency (assuming synched system clock) %7.3fms\n",(t_now-detection.t_sent())*1000.0);
        printf("Total Latency   (assuming synched system clock) %7.3fms\n",(t_now-detection.t_capture())*1000.0);
        int balls_n = detection.balls_size();
        int robots_blue_n =  detection.robots_blue_size();
        int robots_yellow_n =  detection.robots_yellow_size();

        //Ball info:
        for (int i = 0; i < balls_n; i++) {
            SSL_DetectionBall ball = detection.balls(i);
            printf("-Ball (%2d/%2d): CONF=%4.2f POS=<%9.2f,%9.2f> ", i+1, balls_n, ball.confidence(),ball.x(),ball.y());
            if (ball.has_z()) {
                printf("Z=%7.2f ",ball.z());
            } else {
                printf("Z=N/A   ");
            }
            printf("RAW=<%8.2f,%8.2f>\n",ball.pixel_x(),ball.pixel_y());
        }

        //Blue robot info:
        for (int i = 0; i < robots_blue_n; i++) {
            SSL_DetectionRobot robot = detection.robots_blue(i);
            printf("-Robot(B) (%2d/%2d): ",i+1, robots_blue_n);
            printRobotInfo(robot);
        }

        //Yellow robot info:
        for (int i = 0; i < robots_yellow_n; i++) {
            SSL_DetectionRobot robot = detection.robots_yellow(i);
            printf("-Robot(Y) (%2d/%2d): ",i+1, robots_yellow_n);
            printRobotInfo(robot);
        }

    }

    //see if packet contains geometry data:
    if (packet.has_geometry()) {
        const SSL_GeometryData & geom = packet.geometry();
        printf("-[Geometry Data]-------\n");

        const SSL_GeometryFieldSize & field = geom.field();
        printf("Field Dimensions:\n");
        printf("  -field_length=%d (mm)\n",field.field_length());
        printf("  -field_width=%d (mm)\n",field.field_width());
        printf("  -boundary_width=%d (mm)\n",field.boundary_width());
        printf("  -goal_width=%d (mm)\n",field.goal_width());
        printf("  -goal_depth=%d (mm)\n",field.goal_depth());

        int calib_n = geom.calib_size();
        for (int i=0; i< calib_n; i++) {
            const SSL_GeometryCameraCalibration & calib = geom.calib(i);
            printf("Camera Geometry for Camera ID %d:\n", calib.camera_id());
            printf("  -focal_length=%.2f\n",calib.focal_length());
            printf("  -principal_point_x=%.2f\n",calib.principal_point_x());
            printf("  -principal_point_y=%.2f\n",calib.principal_point_y());
            printf("  -distortion=%.2f\n",calib.distortion());
            printf("  -q0=%.2f\n",calib.q0());
            printf("  -q1=%.2f\n",calib.q1());
            printf("  -q2=%.2f\n",calib.q2());
            printf("  -q3=%.2f\n",calib.q3());
            printf("  -tx=%.2f\n",calib.tx());
            printf("  -ty=%.2f\n",calib.ty());
            printf("  -tz=%.2f\n",calib.tz());

            if (calib.has_derived_camera_world_tx() && calib.has_derived_camera_world_ty() && calib.has_derived_camera_world_tz()) {
                printf("  -derived_camera_world_tx=%.f\n",calib.derived_camera_world_tx());
                printf("  -derived_camera_world_ty=%.f\n",calib.derived_camera_world_ty());
                printf("  -derived_camera_world_tz=%.f\n",calib.derived_camera_world_tz());
            }

        }
    }
}

void CVisionThread::run()
{
    visionFPS = 61.0;
    double lastSecond = 0.0, t=0.0;
    int frame=0;
    int lastSecondFrames=0;
    visionSocketMutex.lock();
    vision=NULL;reconnect();
    visionSocketMutex.unlock();
    SSL_WrapperPacket packet;
    int packs = 0;
    int packmax;
    double procTime = -1;
    while (true)
    {
        usleep(1000);
        packmax = conf()->BallTracker_activeCamNum();
        bool flag = false;
        visionSocketMutex.lock();
        flag = closeVision;
        if (visionconfig.reconnect) reconnect();
        visionconfig.reconnect = false;
        visionSocketMutex.unlock();
        if (flag) break;
        packet.Clear();
        double pt = -1;
        if (vision->receive(packet))
        {
            pt = CProfiler::getTime();
            frame ++;
            vc->parse(packet);
            packs ++;
            //			testFunc(packet);
        }

        t = CProfiler::getTime();
        if ( packs >= packmax )
        {
            packs = 0;
            visionDataMutex.lock();

            if (vc->lastCamera < CAMERA_NUM && vc->lastCamera>=0)
            {
                vc->merge(packmax);
                mergedHalfWorld.currentFrame = frame;
                mergedHalfWorld.update(&(vc->res));
                mergedHalfWorld.vanishOutOfSights();

                //				static double tim = CProfiler::getTime();
                ////				debug(QString("vision interval: %1").arg((int)((CProfiler::getTime() - tim)*1000.0)) , D_ERROR);
                //				qDebug() << QString("vision interval: %1").arg((int)((CProfiler::getTime() - tim)*1000.0)) << endl;
                //				tim = CProfiler::getTime();

            }
            if (t-lastSecond>1.0)
            {
                if (lastSecond>0.0)
                {
                    visionFPS = frame-lastSecondFrames;
                }
                lastSecond = t;
                lastSecondFrames = frame;
            }
            //            w.merge();
            visionLatency = vc->res.visionLatency;
            visionTimestep= vc->res.timeStep;
            if (procTime > 0) visionProcessTime = procTime;
            visionDataMutex.unlock();
        }
        //		msleep(1);
        if (pt > 0)
            procTime = CProfiler::getTime() - pt;
    }

    vision->close();
    delete vision;
    visionCloseMutex.lock();
    visionClosed = true;
    visionCloseMutex.unlock();
    qDebug () << "vision thread ended";
}

void CVisionThread::terminated()
{
}

