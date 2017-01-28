#include "VarTypes/gui/VarTreeModel.h"
#include "VarTypes/gui/VarItem.h"
#include "VarTypes/gui/VarTreeView.h"
#include "VarTypes/VarXML.h"
#include "VarTypes/primitives/VarList.h"
#include "VarTypes/primitives/VarDouble.h"
#include "VarTypes/primitives/VarBool.h"
#include "VarTypes/primitives/VarInt.h"
#include "VarTypes/primitives/VarTrigger.h"
#include "VarTypes/VarXML.h"
#include "VarTypes/VarTypes.h"

using namespace VarTypes;

#include "varswidget.h"
#include <QGridLayout>
#include <QDir>


#define IMPL_VALUE(Class,parents,type,Type,name)  \
    type Class::parents##_##name() {if(v_##parents##_##name!=NULL) return v_##parents##_##name->get##Type();return * (new type);}
#define IMPL_ENUM(Class,parents,type,name)  \
    type Class::parents##_##name() {if(v_##parents##_##name!=NULL) return v_##parents##_##name->getString();return * (new type);}

#define ADD_VALUE(parents,type,name,Defaultvalue,namestring) \
    v_##parents##_##name = new Var##type(namestring,Defaultvalue);     \
    parents->addChild(v_##parents##_##name);
#define ADD_ENUM(parents,type,name,Defaultvalue,namestring) \
    v_##parents##_##name = new Var##type(namestring,Defaultvalue);
#define END_ENUM(parents,name) \
    parents->addChild(v_##parents##_##name);
#define ADD_TO_ENUM(parents,name,str) \
    v_##parents##_##name->addItem(str);
#define ADD_TREE(name,namestring,isLocal) \
    name = new VarList(namestring); \
    if(isLocal){localWorld.push_back(name);}else{globalWorld.push_back(name);}
#define ADD_PTREE(parents,name,namestring) \
    parents##_##name = new VarList(namestring); \
    parents->addChild(parents##_##name);
CVarsWidget::CVarsWidget()
{
    tmodel=new VarTreeModel();
    w = new VarTreeView(tmodel);

    v_trigXMLLoad = new VarTrigger("Load XML Setting","Load");
    world.push_back(v_trigXMLLoad);
    v_trigXMLSave = new VarTrigger("Save XML Setting","Save");
    world.push_back(v_trigXMLSave);
    ADD_TREE(LocalSettings,"Local Settings",true)
            ADD_VALUE(LocalSettings,String,SSLVisionMulticastAddr,"224.5.23.2","SSL-Vision Multicast IP")
            ADD_VALUE(LocalSettings,Int,SSLVisionMulticastPort,10002,"SSL-Vision Multicast Port")
            ADD_VALUE(LocalSettings,String,RefereeMulticastAddr,"224.5.23.1","Referee-box Multicast IP")
            ADD_VALUE(LocalSettings,Int,RefereeMulticastPort,10001,"Referee-box Multicast Port")
            ADD_VALUE(LocalSettings,String,SimulatorAddr,"127.0.0.1","Simulator IP")
            ADD_VALUE(LocalSettings,Int,SimulatorPort,20011,"Simulator Port")
            ADD_VALUE(LocalSettings,String,SerialDev,"/dev/ttyS1","Serial Send Device")
            ADD_VALUE(LocalSettings,String,SerialRec,"/dev/ttyUSB2","Serial Recieve Device")
            ADD_VALUE(LocalSettings,String,SharedRadioMulticastAddr,"224.5.23.4","Shared Radio Multicast IP")
            ADD_VALUE(LocalSettings,Int,SharedRadioMulticastPort,10010,"Shared Radio Port")
            ADD_VALUE(LocalSettings,Bool,SharedRadioEnable,false,"Shared Radio Enable")
            ADD_VALUE(LocalSettings,Bool,SharedRadioReceive,false,"Shared Radio Receive")
            ADD_ENUM(LocalSettings,StringEnum,OurTeamColor,"Yellow","Our Team Color")
            ADD_TO_ENUM(LocalSettings,OurTeamColor,"Yellow")
            ADD_TO_ENUM(LocalSettings,OurTeamColor,"Blue")
            END_ENUM(LocalSettings,OurTeamColor)
            ADD_ENUM(LocalSettings,StringEnum,OurTeamSide,"Left","Our Team Side")
            ADD_TO_ENUM(LocalSettings,OurTeamSide,"Left")
            ADD_TO_ENUM(LocalSettings,OurTeamSide,"Right")
            END_ENUM(LocalSettings,OurTeamSide)
            ADD_TREE(Common,"Common",true)
            ADD_VALUE(Common,Int,Viewport_Width,800,"Viewport Width")
            ADD_VALUE(Common,Int,Command_Interval,10,"Command Sending Interval(ms)")
            ADD_VALUE(Common,Int,Monitor_Interval,50,"Monitor Update Interval(ms)")
            ADD_VALUE(Common,Int,Main_Loop_Interval,16,"Main Loop Interval(ms)")
            ADD_VALUE(Common,Bool,KickSensor,true,"Consider received Kick Sensor")
            ///////////////////////////////////////////////////////bang bang
            ADD_TREE(BangBang,"Bang Bang",false)
            ADD_VALUE(BangBang,Double,AccMax,4.0,"Acc")
            ADD_VALUE(BangBang,Double,DecMax,4.0,"Dec")
            ADD_VALUE(BangBang,Double,VelMax,4.0,"Max Vel")
            ADD_VALUE(BangBang,Double,posKP,3.5,"POS PID KP")
            ADD_VALUE(BangBang,Double,posKI,0.0,"POS PID KI")
            ADD_VALUE(BangBang,Double,posKD,2.0,"POS PID KD")
            ADD_VALUE(BangBang,Double,thKP,1.5,"TH PID KP")
            ADD_VALUE(BangBang,Double,thKI,0.0,"TH PID KI")
            ADD_VALUE(BangBang,Double,thKD,0.0,"TH PID KD")
            ///////////////////////////////////////////////////////

            ADD_TREE(Kalman,"Kalman",false)
            ADD_VALUE(Kalman,Bool,UseKalman,true,"Use Kalman")
            ADD_VALUE(Kalman,Int,Blindness,40,"Blindness")
            ADD_VALUE(Kalman,Double,Delay_Time,0.0,"Delay Time")
            ADD_TREE(ERRT,"ERRT",false)
            ADD_VALUE(ERRT,Bool,Draw_Path,true,"Draw Path")
            ADD_VALUE(ERRT,Double,Goal_Probablity,0.3,"Goal Probablity")
            ADD_VALUE(ERRT,Double,Waypoint_Catch_Probablity,0.6,"Waypoint Catch Probablity")
            ADD_VALUE(ERRT,Double,Extend_Step,0.100,"Extend Step")
            ADD_VALUE(ERRT,Double,Target_Distance_Threshold,0.010,"Target Distance Threshold")
            ADD_TREE(Pass_Positioning,"Pass Positioning",false)
            ADD_VALUE(Pass_Positioning,Double,Big_Step_X,1.0000,"Big Step X")
            ADD_VALUE(Pass_Positioning,Double,Big_Step_Y,0.800,"Big Step Y")
            ADD_VALUE(Pass_Positioning,Double,Small_Step_X,0.300,"Small Step X")
            ADD_VALUE(Pass_Positioning,Double,Small_Step_Y,0.300,"Small Step Y")
            ADD_VALUE(Pass_Positioning,Double,Ball_Decel,2.0,"Ball deceleration")
            ADD_VALUE(Pass_Positioning,Double,ChipPass_Recv_StayBackFactor,2.0,"ChipPass Recv StayBackFactor");
    ADD_TREE(Coach,"Coach",false)
            ADD_VALUE(Coach,Double,kickThreshold,0.1,"Kick Threshold")
            ADD_VALUE(Coach,Double,oneTouchKickThreshold,0.05,"OneTouch Kick Threshold")
            ADD_VALUE(Coach,Double,oneTouchAngleThreshold,75,"OneTouch Angle Threshold")
            ADD_VALUE(Coach,Double,kickClosedAngle,40,"Kick Closed Angle")
    ADD_TREE(Plotter,"Plotter",false)
            ADD_VALUE(Plotter,Int,repaint_framerate,100,"Repaint FrameRate")
            ADD_VALUE(Plotter,Double,data_time,10,"Time")
            ADD_VALUE(Plotter,Int,net_port,20020,"Port for Network Mode")
    ADD_TREE(BallTracker,"Ball Tracker & Vision",false)
            ADD_VALUE(BallTracker,Int,activeCamNum,4,"active Cams")
            ADD_VALUE(BallTracker,Bool,onoffState,false,"On / Off")
            ADD_VALUE(BallTracker,Bool,selectBall,true,"Select Ball Mode")
            ADD_VALUE(BallTracker,Int,Insistance,200,"Ball Insistance")
            ADD_VALUE(BallTracker,Bool,cam1on,true,"Camera 1")
            ADD_VALUE(BallTracker,Bool,cam2on,true,"Camera 2")
            ADD_VALUE(BallTracker,Bool,cam3on,true,"Camera 3")
            ADD_VALUE(BallTracker,Bool,cam4on,true,"Camera 4")
    ADD_TREE(MotionProfile,"Motion Profiler",false)
            ADD_VALUE(MotionProfile,Bool,active,false,"Activate Profiler")
            ADD_VALUE(MotionProfile,Bool,autoreset,true,"Auto Reset")
            ADD_VALUE(MotionProfile,Bool,drawpath,false,"Draw Robot Path")
            ADD_VALUE(MotionProfile,Bool,drawvel,false,"Draw Robot Velocity")
            ADD_VALUE(MotionProfile,Bool,drawdir,false,"Draw Robot Direction")
    ADD_TREE(SkillsParams,"Skills Parameters",false)
            ADD_PTREE(SkillsParams, bangbangTrajectory, "bangbangTrajectory")
            ADD_VALUE(SkillsParams_bangbangTrajectory,Double,firstDegThreshold  ,10.0,"first Degree Threshold")
            ADD_VALUE(SkillsParams_bangbangTrajectory,Double,firstDegCoeffecient,1.0 ,"first Degree Coeffecient")
            ADD_VALUE(SkillsParams_bangbangTrajectory,Double,secondDegThreshold  ,20.0,"second Degree Threshold")
            ADD_VALUE(SkillsParams_bangbangTrajectory,Double,secondDegCoeffecient,2.0 ,"second Degree Coeffecient")
            ADD_VALUE(SkillsParams_bangbangTrajectory,Double,thirdDegThreshold  ,45.0,"third Degree Threshold")
            ADD_VALUE(SkillsParams_bangbangTrajectory,Double,thirdDegCoeffecient,4.0 ,"third Degree Coeffecient")
            ADD_VALUE(SkillsParams_bangbangTrajectory,Double,forthDegCoeffecient,4.0 ,"forth Degree Coeffecient")
            ADD_PTREE(SkillsParams, traj_bangbang_tangent, "traj_bangbang_tangent")
            ADD_VALUE(SkillsParams_traj_bangbang_tangent,Double, firstDistThreshold,0.10,"first Dist Threshold")
            ADD_VALUE(SkillsParams_traj_bangbang_tangent,Double, firstDistCoeffecient,8.0,"first Dist Coeffecient")
            ADD_VALUE(SkillsParams_traj_bangbang_tangent,Double, secondDistThreshold,0.05,"second Dist Threshold")
            ADD_VALUE(SkillsParams_traj_bangbang_tangent,Double, secondDistCoeffecient,4.0,"second Dist Coeffecient")
            ADD_VALUE(SkillsParams_traj_bangbang_tangent,Double, thirdDistThreshold,0.035,"third Dist Threshold")
            ADD_VALUE(SkillsParams_traj_bangbang_tangent,Double, thirdDistCoeffecient,2.0,"third Dist Coeffecient")
            ADD_VALUE(SkillsParams_traj_bangbang_tangent,Double, forthDistThreshold,0.025,"forth Dist Threshold")
            ADD_VALUE(SkillsParams_traj_bangbang_tangent,Double, forthDistCoeffecient,1.0,"forth Dist Coeffecient")
            ADD_VALUE(SkillsParams_traj_bangbang_tangent,Double, fifthDistThreshold, 0.005,"fifth Dist Threshold")
            ADD_VALUE(SkillsParams_traj_bangbang_tangent,Double, fifthDistCoeffecient,0.0 ,"fifth Dist Coeffecient")
            ADD_PTREE(SkillsParams,KickOneTouch,"Kick one Touch")
            ADD_VALUE(SkillsParams_KickOneTouch,Double,Landa,0.2,"Landa")
            ADD_VALUE(SkillsParams_KickOneTouch,Double,Gamma,0.9,"Gamma")
            ADD_VALUE(SkillsParams_KickOneTouch,Double,Delay,0.02,"Delay")
            ADD_VALUE(SkillsParams_KickOneTouch,Double,TimeFactor,1.0,"Time Factor")
            ADD_PTREE(SkillsParams,Mark,"Mark")
            ADD_PTREE(SkillsParams,ClearBall,"Clear Ball")
            ADD_VALUE(SkillsParams_ClearBall,Double,TargetMargin,0.150,"Margin for Target Behind the ball")
            ADD_VALUE(SkillsParams_ClearBall,Double,PointClearMargin,0.3,"margin for checking if point is clear")
            ADD_VALUE(SkillsParams_ClearBall,Double,DegThreshold,0.020,"Switching Deg Threshold")
            ADD_VALUE(SkillsParams_ClearBall,Double,ClearSuccessDist,1.000,"Successful clear dist")
            ADD_VALUE(SkillsParams_ClearBall,Double,ClearSuccessTargetDist,0.500,"Successful clear dist to target")
            ADD_VALUE(SkillsParams_ClearBall,Double,FollowBallDist,0.500,"max dist to push ball")
            ADD_PTREE(SkillsParams,Push,"Push")
            ADD_VALUE(SkillsParams_Push,Double,PushVel,1.000,"Velocity of Pushing")
            ADD_TREE(RolesParams,"Roles Parameters",false)
            ADD_PTREE(RolesParams,Mark,"Mark")
            ADD_VALUE(RolesParams_Mark,Double,DistToMarkedOpp,0.160,"Dist To Marked Opponent")
            ADD_VALUE(RolesParams_Mark,Double,Gamma,0.7,"Gamma")
            ADD_PTREE(RolesParams, Goalie, "Goalie")
            ADD_VALUE(RolesParams_Goalie, Int, GoalieID, 0, "ID")
            ADD_TREE(Performance,"Performance",false)
            ADD_PTREE(Performance, Monitor, "Monitor")
            ADD_VALUE(Performance_Monitor, Int, drawLevel, 10, "Max Debug Level")
            ADD_VALUE(Performance_Monitor, Bool, drawRobots, true, "Robots")
            ADD_VALUE(Performance_Monitor, Bool, drawArcs, true, "Arcs")
            ADD_VALUE(Performance_Monitor, Bool, drawPolygons, true, "Polygons")
            ADD_VALUE(Performance_Monitor, Bool, drawRects, true, "Rects")
            ADD_VALUE(Performance_Monitor, Bool, drawSegments, true, "Segments")
            ADD_VALUE(Performance_Monitor, Bool, drawPoints, true, "Points")
            ADD_VALUE(Performance_Monitor, Bool, drawTexts, true, "Texts")
            ADD_PTREE(Performance, Debug, "Debug")
            ADD_VALUE(Performance_Debug, Bool, debugGame, true, "Game")
            ADD_VALUE(Performance_Debug, Bool, debugExperiment, false, "Experiment")
            ADD_VALUE(Performance_Debug, Bool, debugDebug, true, "Debug")
            ADD_VALUE(Performance_Debug, Bool, debugNadia, false, "Nadia")
            ADD_VALUE(Performance_Debug, Bool, debugKK, false, "KK")
            ADD_VALUE(Performance_Debug, Bool, debugDONMHMMD, false, "DON_MHMMD")
            ADD_VALUE(Performance_Debug, Bool, debugERF, false, "Erfan")
            ADD_VALUE(Performance_Debug, Bool, debugMAHI, false, "Mahi")
            ADD_VALUE(Performance_Debug, Bool, debugMani, false, "Mani")
            ADD_VALUE(Performance_Debug, Bool, debugArash, false, "Arash")
            ADD_VALUE(Performance_Debug, Bool, debugAli, false, "Ali")
            ADD_VALUE(Performance_Debug, Bool, debugSepehr, false, "Sepehr")
            ADD_VALUE(Performance_Debug, Bool, debugMasood, false, "Masood")
            ADD_VALUE(Performance_Debug, Bool, debugMohammed, false, "Mohammed")
            ADD_VALUE(Performance_Debug, Bool, debugHossein, false, "Hossein")
            ADD_VALUE(Performance_Debug, Bool, debugFatemeh, false, "Fatemeh")
            ADD_VALUE(Performance_Debug, Bool, debugAHZ, false, "AHZ")
            ADD_VALUE(Performance_Debug, Bool, debugMahmood, false, "Mahmood")
            ADD_VALUE(Performance_Debug, Bool, debugAtousa, false, "Atousa")
            ADD_VALUE(Performance_Debug, Bool, debugAmin, false, "Amin")
            ADD_VALUE(Performance_Debug, Bool, debugAmiR, false, "AmiR")
            ADD_VALUE(Performance_Debug, Bool, debugHamed, false, "Hamed")
            ADD_TREE(Experiments,"Experiments",false)
            ADD_PTREE(Experiments, AutoReferee, "Automated Referee")
            ADD_VALUE(Experiments_AutoReferee, String, autorefereefMulticastAddr, "224.5.23.1", "Multicast Addr")
            ADD_VALUE(Experiments_AutoReferee, Int, autorefereefMulticastPort, 10011, "Multicast Port")


            //world.push_back(root);
            globalWorld=VarXML::read(globalWorld,"settings.xml");
    localWorld=VarXML::read(localWorld,"localsettings.xml");


    world.reserve(globalWorld.size() + localWorld.size());
    world.insert(world.end(), localWorld.begin(), localWorld.end());
    world.insert(world.end(), globalWorld.begin(), globalWorld.end());

    tmodel->setRootItems(world);

    connect(v_trigXMLLoad,SIGNAL(signalTriggered()),this,SLOT(load()));
    connect(v_trigXMLSave,SIGNAL(signalTriggered()),this,SLOT(save()));
    //    world.insert()
    //this->expandAndFocus(root);
    w->fitColumns();
    QGridLayout *l = new QGridLayout(this);
    l->addWidget(w, 0, 0);
    setLayout(l);
    // resize(320,400);
}

CVarsWidget::~CVarsWidget()
{
    delete w;
    delete tmodel;
}

void CVarsWidget::save()
{
    VarXML::write(globalWorld,"settings.xml");
    VarXML::write(localWorld,"localsettings.xml");
}

void CVarsWidget::load()
{
    globalWorld=VarXML::read(globalWorld,"settings.xml");
    localWorld=VarXML::read(localWorld,"localsettings.xml");
}


CVarsWidget* varswidget = NULL;
void initVars(CVarsWidget* v) {varswidget = v;}
CVarsWidget* conf() {return varswidget;}


//---------policy------------

CPolicyWidget::CPolicyWidget()
{
    tmodel=new VarTreeModel();
    w = new VarTreeView(tmodel);

    ADD_TREE(Formation,"Formation",false);
    ADD_VALUE(Formation, Bool, StrictFormation, false, "Strict Formation");
    ADD_VALUE(Formation, Int, Goalie, 1, "Goalie ID");
    ADD_VALUE(Formation, Int, Defense, 1, "Defense Count");
    ADD_TREE(PlayMaker,"Play Maker",false);
    ADD_VALUE(PlayMaker, Bool, JustKickToGoal, true, "Just Kick To Goal");
    ADD_VALUE(PlayMaker, Bool, JustChipToGoalInBelowDist, false, "Just Chip To Goal In Below Distance From Our Goal");
    ADD_VALUE(PlayMaker, Double, ChipToGoalDist, 0.0, "X Dist From Our Goal");
    ADD_VALUE(PlayMaker, Double, KickThreshold, 0.5, "Kick Threshold");
    ADD_VALUE(PlayMaker, Double, OneTouchKickThreshold, 0.5, "Onetouch Threshold");
    ADD_VALUE(PlayMaker, Double, OneTouchAngleThreshold, 80, "Onetouch Angle");
    ADD_VALUE(PlayMaker, Double, OneTouchabilityAngleThreshold, 100, "Onetouchability Angle");
    ADD_VALUE(PlayMaker, Double, OppObstacleFactorInChip, 2.0, "Opp Obstacle Factor in Chip");
    ADD_VALUE(PlayMaker, Double, UnderEstimateTheirGoalie, 0.5, "Under estimate their goalie");
    ADD_TREE(OurKickOff,"OurKickOff",false);
    ADD_VALUE(OurKickOff, Bool, ChipToGoal, false, "Chip to Goal");
    ADD_TREE(OurIndirect,"OurIndirect",false);
    ADD_VALUE(OurIndirect, Bool, ChipToGoal, false, "Chip to Goal");
    ADD_VALUE(OurIndirect, Bool, NoPass, false, "No pass");
    ADD_VALUE(OurIndirect, Bool, ChipToGoalInOurField, false, "Chip to Goal in Our field");
    ADD_VALUE(OurIndirect, Bool, ShadowyPosition, false, "Shadowy position");
    ADD_VALUE(OurIndirect, Int, IndirectType, 1, "Indirect Type");
    ADD_TREE(OurDirect,"OurDirect",false);
    ADD_VALUE(OurDirect, Bool, ChipToGoal, false, "Chip to Goal");
    ADD_VALUE(OurDirect, Bool, ShadowyPosition, false, "Shadowy position");
    ADD_VALUE(OurDirect, Bool, ChipToGoalInOurField, false, "Chip to Goal in Our field");
    ADD_VALUE(OurDirect, Bool, CornerChip, false, "Corner chip");
    ADD_VALUE(OurDirect, Bool, CornerKick, false, "Corner kick");
    ADD_TREE(Defense,"Defense",false);
    ADD_VALUE(Defense, Bool, NoClear, false, "No Clear");
    ADD_VALUE(Defense, Bool, NoPass, false, "No Pass");
    ADD_VALUE(Defense, Bool, ChipClear, false, "Chip Clear");
    ADD_VALUE(Defense, Bool, SwapClearer, true, "Swap Clearer")
            ADD_VALUE(Defense, Double, OppObstacleFactorInChip, 4.0, "Opp Obstacle Factor in Chip");
    ADD_VALUE(Defense, Double, KickThreshold, 0.2, "Kick threshold");
    ADD_VALUE(Defense, Double, ChipThreshold, 0.05,"Chip threshold");
    ADD_VALUE(Defense, Int, MarkingDef, 0,"Marking Defense");
    ADD_TREE(KKPlayOn,"PlayOn",false);
    ADD_VALUE(KKPlayOn, String, KKPlanSQL, QDir::currentPath().toStdString()+"/plan.db3", "SQL Directory");
    ADD_VALUE(KKPlayOn, Int, KKDefaultCycle, 130, "Default Cycle");
    ADD_VALUE(KKPlayOn, Int, KKPassSpeed, 500, "Pass Speed");
    ADD_VALUE(KKPlayOn, Int, KKKickSpeed, 550, "Kick Speed");
    ADD_VALUE(KKPlayOn, Int, KKChipSpeed, 550, "Chip Speed");
    ADD_VALUE(KKPlayOn, Int, KKShotSpeed, 1023, "Shot Speed");
    ADD_VALUE(KKPlayOn, Int, KKChipToGoalSpeed, 650, "Chip to Goal Speed");
    ADD_TREE(KKPlayOff,"PlayOff",false);
    ADD_VALUE(KKPlayOff, String, KKPOPlanSQL, QDir::currentPath().toStdString()+"/poplan.db3", "SQL Directory");
    ADD_VALUE(KKPlayOff, Bool, KKPOSymmetry, false, "Symmetry");
    ADD_VALUE(KKPlayOff, Bool, KKPOUseDef, false, "Use Def Robots");
    ADD_TREE(DynamicPlay, "DynamicPlay", false);
    ADD_VALUE(DynamicPlay, Int , LowSpeedPass   , 300, "Low Speed Pass");
    ADD_VALUE(DynamicPlay, Int , MediumSpeedPass, 600, "Medium Speed Pass");
    ADD_VALUE(DynamicPlay, Int , HighSpeedPass  , 800, "High Speed Pass");
    ADD_VALUE(DynamicPlay, Int , LowSpeedChip   , 300, "Low Speed Chip");
    ADD_VALUE(DynamicPlay, Int , MediumSpeedChip, 300, "Medium Speed Chip");
    ADD_VALUE(DynamicPlay, Int , HighSpeedChip  , 300, "High Speed Chip");
    ADD_VALUE(DynamicPlay, Bool, FarForward , false, "Far Forward");
    ADD_VALUE(DynamicPlay, Bool, NearForward, false, "Near Forward");
    ADD_VALUE(DynamicPlay, Double, Area, 0.3, "Pass Area");

    ADD_TREE(Mark, "Mark", false);
    ADD_VALUE(Mark, Bool , PlayOffManToMan   , false, "PlayOff Man To Man");
    ADD_VALUE(Mark, Bool , PlayOnManToMan, false, "PlayOn Man To Man");
    ADD_VALUE(Mark, Bool , ManToManAllTransiant  , false, "Man To Man All Transiant");
    ADD_VALUE(Mark, Bool , ManToManSomeTransiant , false, "Man To man Some Transiant");
    ADD_VALUE(Mark, Bool , OmmitWhoCouldNotReceivePass, false, "Ommit Who Could Not Recieve Pass");
    ADD_VALUE(Mark, Double , OppOmitLimitPlayon  , 2.0, "Opponent Ommit Limit Playon");
    ADD_VALUE(Mark, Double , OppOmitLimitPlayoff  , 2.0, "Opponent Ommit Limit Playoff");
    ADD_VALUE(Mark, Double , OppOmitLimitKickOff  , 2.0, "Opponent Ommit Limit KickOff");
    ADD_VALUE(Mark, Double, ShootRatioBlock, 0.33, "Shoot Ratio Block");
    ADD_VALUE(Mark, Double, PassRatioBlock, 0.33, "Pass Ratio Block");

    ADD_VALUE(Mark, Bool, OmmitNearestToBallPlayon, false, "Ommit Nearest To ball Playon");


    globalWorld=VarXML::read(globalWorld,"policy.xml");


    world.reserve(globalWorld.size() + localWorld.size());
    world.insert(world.end(), localWorld.begin(), localWorld.end());
    world.insert(world.end(), globalWorld.begin(), globalWorld.end());

    tmodel->setRootItems(world);
    w->fitColumns();
    QGridLayout *l = new QGridLayout(this);
    l->addWidget(w, 0, 0);
    setLayout(l);
    // resize(320,400);
}

CPolicyWidget::~CPolicyWidget()
{
}

void CPolicyWidget::save()
{
    VarXML::write(world,"policy.xml");
}

void CPolicyWidget::load()
{
    world=VarXML::read(world,"policy.xml");
}

CPolicyWidget* pol;
void initPolicy(CPolicyWidget* v) {pol = v;}
CPolicyWidget* policy() {return pol;}

IMPL_VALUE(CVarsWidget,LocalSettings,std::string,String,SSLVisionMulticastAddr)
IMPL_VALUE(CVarsWidget,LocalSettings,int,Int,SSLVisionMulticastPort)
IMPL_VALUE(CVarsWidget,LocalSettings,std::string,String,RefereeMulticastAddr)
IMPL_VALUE(CVarsWidget,LocalSettings,int,Int,RefereeMulticastPort)
IMPL_VALUE(CVarsWidget,LocalSettings,std::string,String,SimulatorAddr)
IMPL_VALUE(CVarsWidget,LocalSettings,int,Int,SimulatorPort)
IMPL_VALUE(CVarsWidget,LocalSettings,std::string,String,SerialDev)
IMPL_VALUE(CVarsWidget,LocalSettings,std::string,String,SerialRec)
IMPL_VALUE(CVarsWidget,LocalSettings,std::string,String,SharedRadioMulticastAddr)
IMPL_VALUE(CVarsWidget,LocalSettings,int,Int,SharedRadioMulticastPort)
IMPL_VALUE(CVarsWidget,LocalSettings,bool,Bool,SharedRadioEnable)
IMPL_VALUE(CVarsWidget,LocalSettings,bool,Bool,SharedRadioReceive)
IMPL_ENUM(CVarsWidget,LocalSettings,std::string,OurTeamColor)
IMPL_ENUM(CVarsWidget,LocalSettings,std::string,OurTeamSide)
IMPL_VALUE(CVarsWidget,Common,int,Int,Viewport_Width)
IMPL_VALUE(CVarsWidget,Common,int,Int,Command_Interval)
IMPL_VALUE(CVarsWidget,Common,int,Int,Monitor_Interval)
IMPL_VALUE(CVarsWidget,Common,int,Int,Main_Loop_Interval)
IMPL_VALUE(CVarsWidget,Common,bool,Bool,KickSensor)
////////////////////////////////////////////////////bang bang
IMPL_VALUE(CVarsWidget,BangBang,double,Double,AccMax)
IMPL_VALUE(CVarsWidget,BangBang,double,Double,DecMax)
IMPL_VALUE(CVarsWidget,BangBang,double,Double,VelMax)
IMPL_VALUE(CVarsWidget,BangBang,double,Double,posKP)
IMPL_VALUE(CVarsWidget,BangBang,double,Double,posKI)
IMPL_VALUE(CVarsWidget,BangBang,double,Double,posKD)
IMPL_VALUE(CVarsWidget,BangBang,double,Double,thKP)
IMPL_VALUE(CVarsWidget,BangBang,double,Double,thKI)
IMPL_VALUE(CVarsWidget,BangBang,double,Double,thKD)
////////////////////////////////////////////////////
IMPL_VALUE(CVarsWidget,Kalman,bool,Bool,UseKalman)
IMPL_VALUE(CVarsWidget,Kalman,int,Int,Blindness)
IMPL_VALUE(CVarsWidget,Kalman,double,Double,Delay_Time)
IMPL_VALUE(CVarsWidget,ERRT,bool,Bool,Draw_Path)
IMPL_VALUE(CVarsWidget,ERRT,double,Double,Goal_Probablity)
IMPL_VALUE(CVarsWidget,ERRT,double,Double,Waypoint_Catch_Probablity)
IMPL_VALUE(CVarsWidget,ERRT,double,Double,Extend_Step)
IMPL_VALUE(CVarsWidget,ERRT,double,Double,Target_Distance_Threshold)
IMPL_VALUE(CVarsWidget,Pass_Positioning,double,Double,Big_Step_X)
IMPL_VALUE(CVarsWidget,Pass_Positioning,double,Double,Big_Step_Y)
IMPL_VALUE(CVarsWidget,Pass_Positioning,double,Double,Small_Step_X)
IMPL_VALUE(CVarsWidget,Pass_Positioning,double,Double,Small_Step_Y)
IMPL_VALUE(CVarsWidget,Pass_Positioning,double,Double,Ball_Decel)
IMPL_VALUE(CVarsWidget,Pass_Positioning,double,Double,ChipPass_Recv_StayBackFactor)
IMPL_VALUE(CVarsWidget,Coach,double,Double,kickThreshold)
IMPL_VALUE(CVarsWidget,Coach,double,Double,oneTouchKickThreshold)
IMPL_VALUE(CVarsWidget,Coach,double,Double,oneTouchAngleThreshold)
IMPL_VALUE(CVarsWidget,Coach,double,Double,kickClosedAngle)
IMPL_VALUE(CVarsWidget,Plotter,int,Int,repaint_framerate)
IMPL_VALUE(CVarsWidget,Plotter,double,Double,data_time)
IMPL_VALUE(CVarsWidget,Plotter,int,Int,net_port)
IMPL_VALUE(CVarsWidget,BallTracker,unsigned char,Int,activeCamNum)
IMPL_VALUE(CVarsWidget,BallTracker,bool,Bool,onoffState)
IMPL_VALUE(CVarsWidget,BallTracker,bool,Bool,selectBall)
IMPL_VALUE(CVarsWidget,BallTracker,int,Int,Insistance)
IMPL_VALUE(CVarsWidget,BallTracker,bool,Bool,cam1on)
IMPL_VALUE(CVarsWidget,BallTracker,bool,Bool,cam2on)
IMPL_VALUE(CVarsWidget,BallTracker,bool,Bool,cam3on)
IMPL_VALUE(CVarsWidget,BallTracker,bool,Bool,cam4on)
IMPL_VALUE(CVarsWidget,MotionProfile,bool,Bool,active)
IMPL_VALUE(CVarsWidget,MotionProfile,bool,Bool,autoreset)
IMPL_VALUE(CVarsWidget,MotionProfile,bool,Bool,drawpath)
IMPL_VALUE(CVarsWidget,MotionProfile,bool,Bool,drawvel)
IMPL_VALUE(CVarsWidget,MotionProfile,bool,Bool,drawdir)
IMPL_VALUE(CVarsWidget,SkillsParams_bangbangTrajectory,double,Double,firstDegThreshold)
IMPL_VALUE(CVarsWidget,SkillsParams_bangbangTrajectory,double,Double,firstDegCoeffecient)
IMPL_VALUE(CVarsWidget,SkillsParams_bangbangTrajectory,double,Double,secondDegThreshold)
IMPL_VALUE(CVarsWidget,SkillsParams_bangbangTrajectory,double,Double,secondDegCoeffecient)
IMPL_VALUE(CVarsWidget,SkillsParams_bangbangTrajectory,double,Double,thirdDegThreshold)
IMPL_VALUE(CVarsWidget,SkillsParams_bangbangTrajectory,double,Double,thirdDegCoeffecient)
IMPL_VALUE(CVarsWidget,SkillsParams_bangbangTrajectory,double,Double,forthDegCoeffecient)
IMPL_VALUE(CVarsWidget,SkillsParams_traj_bangbang_tangent,double,Double,firstDistThreshold)
IMPL_VALUE(CVarsWidget,SkillsParams_traj_bangbang_tangent,double,Double,firstDistCoeffecient)
IMPL_VALUE(CVarsWidget,SkillsParams_traj_bangbang_tangent,double,Double,secondDistThreshold)
IMPL_VALUE(CVarsWidget,SkillsParams_traj_bangbang_tangent,double,Double,secondDistCoeffecient)
IMPL_VALUE(CVarsWidget,SkillsParams_traj_bangbang_tangent,double,Double,thirdDistThreshold)
IMPL_VALUE(CVarsWidget,SkillsParams_traj_bangbang_tangent,double,Double,thirdDistCoeffecient)
IMPL_VALUE(CVarsWidget,SkillsParams_traj_bangbang_tangent,double,Double,forthDistThreshold)
IMPL_VALUE(CVarsWidget,SkillsParams_traj_bangbang_tangent,double,Double,forthDistCoeffecient)
IMPL_VALUE(CVarsWidget,SkillsParams_traj_bangbang_tangent,double,Double,fifthDistThreshold)
IMPL_VALUE(CVarsWidget,SkillsParams_traj_bangbang_tangent,double,Double,fifthDistCoeffecient)
IMPL_VALUE(CVarsWidget,SkillsParams_KickOneTouch,double,Double,Landa)
IMPL_VALUE(CVarsWidget,SkillsParams_KickOneTouch,double,Double,Gamma)
IMPL_VALUE(CVarsWidget,SkillsParams_KickOneTouch,double,Double,Delay)
IMPL_VALUE(CVarsWidget,SkillsParams_KickOneTouch,double,Double,TimeFactor)
IMPL_VALUE(CVarsWidget,SkillsParams_ClearBall,double,Double,TargetMargin)
IMPL_VALUE(CVarsWidget,SkillsParams_ClearBall,double,Double,PointClearMargin)
IMPL_VALUE(CVarsWidget,SkillsParams_ClearBall,double,Double,DegThreshold)
IMPL_VALUE(CVarsWidget,SkillsParams_ClearBall,double,Double,ClearSuccessDist)
IMPL_VALUE(CVarsWidget,SkillsParams_ClearBall,double,Double,ClearSuccessTargetDist)
IMPL_VALUE(CVarsWidget,SkillsParams_ClearBall,double,Double,FollowBallDist)
IMPL_VALUE(CVarsWidget,SkillsParams_Push,double,Double,PushVel)
IMPL_VALUE(CVarsWidget,RolesParams_Mark,double,Double,DistToMarkedOpp)
IMPL_VALUE(CVarsWidget,RolesParams_Mark,double,Double,Gamma)
IMPL_VALUE(CVarsWidget,RolesParams_Goalie, int, Int, GoalieID)
IMPL_VALUE(CVarsWidget,Performance_Monitor, int, Int, drawLevel)
IMPL_VALUE(CVarsWidget,Performance_Monitor, bool, Bool, drawRobots)
IMPL_VALUE(CVarsWidget,Performance_Monitor, bool, Bool, drawArcs)
IMPL_VALUE(CVarsWidget,Performance_Monitor, bool, Bool, drawPolygons)
IMPL_VALUE(CVarsWidget,Performance_Monitor, bool, Bool, drawRects)
IMPL_VALUE(CVarsWidget,Performance_Monitor, bool, Bool, drawSegments)
IMPL_VALUE(CVarsWidget,Performance_Monitor, bool, Bool, drawPoints)
IMPL_VALUE(CVarsWidget,Performance_Monitor, bool, Bool, drawTexts)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugGame)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugExperiment)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugDebug)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugNadia)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugKK)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugDONMHMMD)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugERF)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugMAHI)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugMani)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugArash)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugAli)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugSepehr)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugMasood)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugMohammed)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugHossein)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugMahmood)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugAHZ)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugFatemeh)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugAtousa)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugAmin)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugAmiR)
IMPL_VALUE(CVarsWidget,Performance_Debug, bool, Bool, debugHamed)



IMPL_VALUE(CVarsWidget,Experiments_AutoReferee, std::string, String, autorefereefMulticastAddr)
IMPL_VALUE(CVarsWidget,Experiments_AutoReferee, int, Int, autorefereefMulticastPort)


IMPL_VALUE(CPolicyWidget,Formation, bool, Bool, StrictFormation)
IMPL_VALUE(CPolicyWidget,Formation, int, Int, Goalie)
IMPL_VALUE(CPolicyWidget,Formation, int, Int, Defense)
IMPL_VALUE(CPolicyWidget,PlayMaker, bool, Bool, JustKickToGoal)
IMPL_VALUE(CPolicyWidget,PlayMaker, bool, Bool, JustChipToGoalInBelowDist)
IMPL_VALUE(CPolicyWidget,PlayMaker, double, Double, ChipToGoalDist)
IMPL_VALUE(CPolicyWidget,PlayMaker, double, Double, KickThreshold)
IMPL_VALUE(CPolicyWidget,PlayMaker, double, Double, OneTouchKickThreshold)
IMPL_VALUE(CPolicyWidget,PlayMaker, double, Double, OneTouchAngleThreshold)
IMPL_VALUE(CPolicyWidget,PlayMaker, double, Double, OneTouchabilityAngleThreshold)
IMPL_VALUE(CPolicyWidget,PlayMaker, double, Double, OppObstacleFactorInChip)
IMPL_VALUE(CPolicyWidget,PlayMaker, double, Double, UnderEstimateTheirGoalie)
IMPL_VALUE(CPolicyWidget,OurKickOff, bool, Bool, ChipToGoal)
IMPL_VALUE(CPolicyWidget,OurIndirect, bool, Bool, ChipToGoal)
IMPL_VALUE(CPolicyWidget,OurIndirect, bool, Bool, NoPass)
IMPL_VALUE(CPolicyWidget,OurIndirect, bool, Bool, ChipToGoalInOurField)
IMPL_VALUE(CPolicyWidget,OurIndirect, bool, Bool, ShadowyPosition)
IMPL_VALUE(CPolicyWidget,OurIndirect, int, Int, IndirectType)
IMPL_VALUE(CPolicyWidget,OurDirect, bool, Bool, ChipToGoal)
IMPL_VALUE(CPolicyWidget,OurDirect, bool, Bool, ShadowyPosition)
IMPL_VALUE(CPolicyWidget,OurDirect, bool, Bool, ChipToGoalInOurField)
IMPL_VALUE(CPolicyWidget,Defense, bool, Bool, NoClear)
IMPL_VALUE(CPolicyWidget,Defense, bool, Bool, NoPass)
IMPL_VALUE(CPolicyWidget,Defense, bool, Bool, ChipClear)
IMPL_VALUE(CPolicyWidget,Defense, bool, Bool, SwapClearer)
IMPL_VALUE(CPolicyWidget,Defense, double, Double, OppObstacleFactorInChip)
IMPL_VALUE(CPolicyWidget,Defense, double, Double, KickThreshold)
IMPL_VALUE(CPolicyWidget,Defense, double, Double, ChipThreshold)
IMPL_VALUE(CPolicyWidget,Defense, int, Int, MarkingDef)

IMPL_VALUE(CPolicyWidget,KKPlayOn, std::string, String, KKPlanSQL)
IMPL_VALUE(CPolicyWidget,KKPlayOn, int, Int, KKDefaultCycle)
IMPL_VALUE(CPolicyWidget,KKPlayOn, int, Int, KKPassSpeed)
IMPL_VALUE(CPolicyWidget,KKPlayOn, int, Int, KKKickSpeed)
IMPL_VALUE(CPolicyWidget,KKPlayOn, int, Int, KKChipSpeed)
IMPL_VALUE(CPolicyWidget,KKPlayOn, int, Int, KKShotSpeed)
IMPL_VALUE(CPolicyWidget,KKPlayOn, int, Int, KKChipToGoalSpeed)

IMPL_VALUE(CPolicyWidget,KKPlayOff, std::string, String, KKPOPlanSQL)
IMPL_VALUE(CPolicyWidget,KKPlayOff, bool, Bool, KKPOSymmetry)
IMPL_VALUE(CPolicyWidget,KKPlayOff, bool, Bool, KKPOUseDef)

IMPL_VALUE(CPolicyWidget, DynamicPlay, int , Int , LowSpeedPass)
IMPL_VALUE(CPolicyWidget, DynamicPlay, int , Int , MediumSpeedPass)
IMPL_VALUE(CPolicyWidget, DynamicPlay, int , Int , HighSpeedPass)
IMPL_VALUE(CPolicyWidget, DynamicPlay, int , Int , LowSpeedChip)
IMPL_VALUE(CPolicyWidget, DynamicPlay, int , Int , MediumSpeedChip)
IMPL_VALUE(CPolicyWidget, DynamicPlay, int , Int , HighSpeedChip)
IMPL_VALUE(CPolicyWidget, DynamicPlay, bool, Bool, FarForward)
IMPL_VALUE(CPolicyWidget, DynamicPlay, bool, Bool, NearForward)
IMPL_VALUE(CPolicyWidget, DynamicPlay, double, Double, Area)


IMPL_VALUE(CPolicyWidget, Mark, bool , Bool , PlayOffManToMan)
IMPL_VALUE(CPolicyWidget, Mark, bool , Bool , PlayOnManToMan)
IMPL_VALUE(CPolicyWidget, Mark, bool , Bool , ManToManAllTransiant)
IMPL_VALUE(CPolicyWidget, Mark, bool , Bool , ManToManSomeTransiant)
IMPL_VALUE(CPolicyWidget, Mark, bool , Bool , OmmitWhoCouldNotReceivePass)
IMPL_VALUE(CPolicyWidget, Mark, double , Double , OppOmitLimitPlayon)
IMPL_VALUE(CPolicyWidget, Mark, double , Double , OppOmitLimitPlayoff)
IMPL_VALUE(CPolicyWidget, Mark, double , Double , OppOmitLimitKickOff)
IMPL_VALUE(CPolicyWidget, Mark, double, Double, ShootRatioBlock)
IMPL_VALUE(CPolicyWidget, Mark, double, Double, PassRatioBlock)
IMPL_VALUE(CPolicyWidget, Mark, bool , Bool , OmmitNearestToBallPlayon)




IMPL_VALUE(CPolicyWidget,OurDirect, bool, Bool, CornerChip)
IMPL_VALUE(CPolicyWidget,OurDirect, bool, Bool, CornerKick)
