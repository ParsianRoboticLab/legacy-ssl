#ifndef VARSWIDGET_H
#define VARSWIDGET_H

#include <QWidget>

#define DEF_VALUE(parents,type,Type,name)  \
    VarTypes::Var##Type* v_##parents##_##name; \
    type parents##_##name();
#define DEF_ENUM(parents,type,name)  \
    VarTypes::VarStringEnum* v_##parents##_##name; \
    type parents##_##name();
#define DEF_TREE(name)  \
    VarTypes::VarList* name;
#define DEF_PTREE(parents, name)  \
    VarTypes::VarList* parents##_##name;


namespace VarTypes {
class VarTreeView;
class VarTreeModel;
class VarType;
class VarDouble;
class VarBool;
class VarInt;
class VarTrigger;
class VarString;
class VarStringEnum;
class VarList;
}

using namespace VarTypes;

class CVarsWidget : public QWidget
{
    Q_OBJECT
    VarTypes::VarTreeView *w;
    std::vector<VarTypes::VarType *> world,localWorld,globalWorld;
    VarTypes::VarTreeModel * tmodel;
    //VarTypes::VarList* root;
public:
    CVarsWidget();
    virtual ~CVarsWidget();
    VarTypes::VarTrigger *v_trigXMLLoad;
    VarTypes::VarTrigger *v_trigXMLSave;
    DEF_TREE(LocalSettings)
    DEF_VALUE(LocalSettings,std::string,String,SSLVisionMulticastAddr)
    DEF_VALUE(LocalSettings,int,Int,SSLVisionMulticastPort)
    DEF_VALUE(LocalSettings,std::string,String,RefereeMulticastAddr)
    DEF_VALUE(LocalSettings,int,Int,RefereeMulticastPort)
    DEF_VALUE(LocalSettings,std::string,String,SimulatorAddr)
    DEF_VALUE(LocalSettings,int,Int,SimulatorPort)
    DEF_VALUE(LocalSettings,std::string,String,SerialDev)
    DEF_VALUE(LocalSettings,std::string,String,SerialRec)
    DEF_VALUE(LocalSettings,std::string,String,SharedRadioMulticastAddr)
    DEF_VALUE(LocalSettings,int,Int,SharedRadioMulticastPort)
    DEF_VALUE(LocalSettings,bool,Bool,SharedRadioEnable)
    DEF_VALUE(LocalSettings,bool,Bool,SharedRadioReceive)
    DEF_ENUM(LocalSettings,std::string,OurTeamColor)
    DEF_ENUM(LocalSettings,std::string,OurTeamSide)
    DEF_ENUM(LocalSettings,std::string,LineUpPosition)

    DEF_VALUE(LocalSettings,bool, Bool, ParsianWorkShop)
    DEF_VALUE(LocalSettings,std::string,String,MixTeamIP)
    DEF_VALUE(LocalSettings,int,Int,MixTeamPort)
    DEF_VALUE(LocalSettings,std::string,String,MixTeamIDs)

    DEF_VALUE(LocalSettings,bool, Bool, sendRobotStatus)
    DEF_VALUE(LocalSettings,std::string, String, robotStatusIP)
    DEF_VALUE(LocalSettings,int, Int, robotStatusPort)

    DEF_TREE(Common)
    DEF_VALUE(Common,int,Int,Viewport_Width)
    DEF_VALUE(Common,int,Int,Command_Interval)
    DEF_VALUE(Common,int,Int,Monitor_Interval)
    DEF_VALUE(Common,int,Int,Main_Loop_Interval)
    DEF_VALUE(Common,bool,Bool,KickSensor)
    DEF_TREE(BangBang)
    DEF_VALUE(BangBang,double,Double,AccMaxForward)
    DEF_VALUE(BangBang,double,Double,AccMaxNormal)
    DEF_VALUE(BangBang,double,Double,DecMax)
    DEF_VALUE(BangBang,double,Double,VelMax)
    DEF_VALUE(BangBang,double,Double,posKP)
    DEF_VALUE(BangBang,double,Double,posKI)
    DEF_VALUE(BangBang,double,Double,posKD)
    DEF_VALUE(BangBang,double,Double,thKP)
    DEF_VALUE(BangBang,double,Double,thKI)
    DEF_VALUE(BangBang,double,Double,thKD)
    DEF_TREE(Kalman)
    DEF_VALUE(Kalman,bool,Bool,UseKalman)
    DEF_VALUE(Kalman,int,Int,Blindness)
    DEF_VALUE(Kalman,double,Double,Delay_Time)
    DEF_TREE(ERRT)
    DEF_VALUE(ERRT,bool,Bool,Draw_Path)
    DEF_VALUE(ERRT,double,Double,Goal_Probablity)
    DEF_VALUE(ERRT,double,Double,Waypoint_Catch_Probablity)
    DEF_VALUE(ERRT,double,Double,Extend_Step)
    DEF_VALUE(ERRT,double,Double,Target_Distance_Threshold)
    DEF_TREE(Pass_Positioning)
    DEF_VALUE(Pass_Positioning,double,Double,Big_Step_X)
    DEF_VALUE(Pass_Positioning,double,Double,Big_Step_Y)
    DEF_VALUE(Pass_Positioning,double,Double,Small_Step_X)
    DEF_VALUE(Pass_Positioning,double,Double,Small_Step_Y)
    DEF_VALUE(Pass_Positioning,double,Double,Ball_Decel)
    DEF_VALUE(Pass_Positioning,double,Double,ChipPass_Recv_StayBackFactor)
    DEF_TREE(Coach)
    DEF_VALUE(Coach,double,Double,kickThreshold)
    DEF_VALUE(Coach,double,Double,oneTouchKickThreshold)
    DEF_VALUE(Coach,double,Double,oneTouchAngleThreshold)
    DEF_VALUE(Coach,double,Double,kickClosedAngle)
    DEF_TREE(Plotter)
    DEF_VALUE(Plotter,int,Int,repaint_framerate)
    DEF_VALUE(Plotter,double,Double,data_time)
    DEF_VALUE(Plotter,int,Int,net_port)
    DEF_TREE(BallTracker)
    DEF_VALUE(BallTracker,unsigned char,Int,activeCamNum)
    DEF_VALUE(BallTracker,bool,Bool,onoffState)
    DEF_VALUE(BallTracker,bool,Bool,selectBall)
    DEF_VALUE(BallTracker,int,Int,Insistance)
    DEF_VALUE(BallTracker,bool,Bool,cam1on)
    DEF_VALUE(BallTracker,bool,Bool,cam2on)
    DEF_VALUE(BallTracker,bool,Bool,cam3on)
    DEF_VALUE(BallTracker,bool,Bool,cam4on)
    DEF_VALUE(BallTracker,bool,Bool,cam5on)
    DEF_VALUE(BallTracker,bool,Bool,cam6on)
    DEF_VALUE(BallTracker,bool,Bool,cam7on)
    DEF_VALUE(BallTracker,bool,Bool,cam8on)
    DEF_TREE(MotionProfile)
    DEF_VALUE(MotionProfile,bool,Bool,active)
    DEF_VALUE(MotionProfile,bool,Bool,autoreset)
    DEF_VALUE(MotionProfile,bool,Bool,drawpath)
    DEF_VALUE(MotionProfile,bool,Bool,drawvel)
    DEF_VALUE(MotionProfile,bool,Bool,drawdir)
    DEF_TREE(SkillsParams)
    DEF_PTREE(SkillsParams, bangbangTrajectory)
    DEF_VALUE(SkillsParams_bangbangTrajectory,double,Double,firstDegThreshold)
    DEF_VALUE(SkillsParams_bangbangTrajectory,double,Double,firstDegCoeffecient)
    DEF_VALUE(SkillsParams_bangbangTrajectory,double,Double,secondDegThreshold)
    DEF_VALUE(SkillsParams_bangbangTrajectory,double,Double,secondDegCoeffecient)
    DEF_VALUE(SkillsParams_bangbangTrajectory,double,Double,thirdDegThreshold)
    DEF_VALUE(SkillsParams_bangbangTrajectory,double,Double,thirdDegCoeffecient)
    DEF_VALUE(SkillsParams_bangbangTrajectory,double,Double,forthDegCoeffecient)
    DEF_PTREE(SkillsParams, traj_bangbang_tangent)
    DEF_VALUE(SkillsParams_traj_bangbang_tangent,double,Double,firstDistThreshold)
    DEF_VALUE(SkillsParams_traj_bangbang_tangent,double,Double,firstDistCoeffecient)
    DEF_VALUE(SkillsParams_traj_bangbang_tangent,double,Double,secondDistThreshold)
    DEF_VALUE(SkillsParams_traj_bangbang_tangent,double,Double,secondDistCoeffecient)
    DEF_VALUE(SkillsParams_traj_bangbang_tangent,double,Double,thirdDistThreshold)
    DEF_VALUE(SkillsParams_traj_bangbang_tangent,double,Double,thirdDistCoeffecient)
    DEF_VALUE(SkillsParams_traj_bangbang_tangent,double,Double,forthDistThreshold)
    DEF_VALUE(SkillsParams_traj_bangbang_tangent,double,Double,forthDistCoeffecient)
    DEF_VALUE(SkillsParams_traj_bangbang_tangent,double,Double,fifthDistThreshold)
    DEF_VALUE(SkillsParams_traj_bangbang_tangent,double,Double,fifthDistCoeffecient)
    DEF_PTREE(SkillsParams, KickOneTouch)
    DEF_VALUE(SkillsParams_KickOneTouch,double,Double,Landa)
    DEF_VALUE(SkillsParams_KickOneTouch,double,Double,Gamma)
    DEF_VALUE(SkillsParams_KickOneTouch,double,Double,Delay)
    DEF_VALUE(SkillsParams_KickOneTouch,double,Double,TimeFactor)
    DEF_PTREE(SkillsParams,Mark)
    DEF_PTREE(SkillsParams,ClearBall)
    DEF_VALUE(SkillsParams_ClearBall,double,Double,TargetMargin)
    DEF_VALUE(SkillsParams_ClearBall,double,Double,PointClearMargin)
    DEF_VALUE(SkillsParams_ClearBall,double,Double,DegThreshold)
    DEF_VALUE(SkillsParams_ClearBall,double,Double,ClearSuccessDist)
    DEF_VALUE(SkillsParams_ClearBall,double,Double,ClearSuccessTargetDist)
    DEF_VALUE(SkillsParams_ClearBall,double,Double,FollowBallDist)
    DEF_PTREE(SkillsParams,Push)
    DEF_VALUE(SkillsParams_Push,double,Double,PushVel)
    DEF_TREE(RolesParams)
    DEF_PTREE(RolesParams,Mark)
    DEF_VALUE(RolesParams_Mark,double,Double,DistToMarkedOpp)
    DEF_VALUE(RolesParams_Mark,double,Double,Gamma)
    DEF_PTREE(RolesParams, Goalie)
    DEF_VALUE(RolesParams_Goalie, int, Int, GoalieID)
    DEF_TREE(Performance)
    DEF_PTREE(Performance, Monitor)
    DEF_VALUE(Performance_Monitor, int, Int, drawLevel  )
    DEF_VALUE(Performance_Monitor, bool, Bool, drawRobots)
    DEF_VALUE(Performance_Monitor, bool, Bool, drawArcs)
    DEF_VALUE(Performance_Monitor, bool, Bool, drawPolygons)
    DEF_VALUE(Performance_Monitor, bool, Bool, drawRects)
    DEF_VALUE(Performance_Monitor, bool, Bool, drawSegments)
    DEF_VALUE(Performance_Monitor, bool, Bool, drawPoints)
    DEF_VALUE(Performance_Monitor, bool, Bool, drawTexts)
    DEF_PTREE(Performance, Debug)
    DEF_VALUE(Performance_Debug, bool, Bool, debugGame)
    DEF_VALUE(Performance_Debug, bool, Bool, debugExperiment)
    DEF_VALUE(Performance_Debug, bool, Bool, debugDebug)
    DEF_VALUE(Performance_Debug, bool, Bool, debugNadia)
    DEF_VALUE(Performance_Debug, bool, Bool, debugKK)
    DEF_VALUE(Performance_Debug, bool, Bool, debugDONMHMMD)
    DEF_VALUE(Performance_Debug, bool, Bool, debugERF)
    DEF_VALUE(Performance_Debug, bool, Bool, debugMAHI)
    DEF_VALUE(Performance_Debug, bool, Bool, debugMani)
    DEF_VALUE(Performance_Debug, bool, Bool, debugArash)
    DEF_VALUE(Performance_Debug, bool, Bool, debugAli)
    DEF_VALUE(Performance_Debug, bool, Bool, debugSepehr)
    DEF_VALUE(Performance_Debug, bool, Bool, debugMasood)
    DEF_VALUE(Performance_Debug, bool, Bool, debugMohammed)
    DEF_VALUE(Performance_Debug, bool, Bool, debugHossein)
    DEF_VALUE(Performance_Debug, bool, Bool, debugMahmood)
    DEF_VALUE(Performance_Debug, bool, Bool, debugAHZ)
    DEF_VALUE(Performance_Debug, bool, Bool, debugFatemeh)
    DEF_VALUE(Performance_Debug, bool, Bool, debugAtousa)
    DEF_VALUE(Performance_Debug, bool, Bool, debugParsa)
    DEF_VALUE(Performance_Debug, bool, Bool, debugAmin)
    DEF_VALUE(Performance_Debug, bool, Bool, debugHamed)

    DEF_TREE(Experiments)
    DEF_PTREE(Experiments, AutoReferee)
    DEF_VALUE(Experiments_AutoReferee, std::string, String, autorefereefMulticastAddr)
    DEF_VALUE(Experiments_AutoReferee, int, Int, autorefereefMulticastPort)


    public slots:
        void save();
    void load();
};

class CPolicyWidget : public QWidget
{
    Q_OBJECT
    VarTypes::VarTreeView *w;
    std::vector<VarTypes::VarType *> world,localWorld,globalWorld;
    VarTypes::VarTreeModel * tmodel;
    //VarTypes::VarList* root;
public:
    CPolicyWidget();
    virtual ~CPolicyWidget();
    VarTypes::VarTrigger *v_trigXMLLoad;
    VarTypes::VarTrigger *v_trigXMLSave;
    VarTypes::VarTrigger *v_trigUpdateCoach;
    DEF_TREE(Formation)
    DEF_VALUE(Formation, bool, Bool, StrictFormation)
    DEF_VALUE(Formation, bool, Bool, GoalieFromGUI)
    DEF_VALUE(Formation, int, Int, Goalie)
    DEF_VALUE(Formation, int, Int, Defense)
    DEF_TREE(PlayMaker)
    DEF_VALUE(PlayMaker, bool, Bool, JustKickToGoal)
    DEF_VALUE(PlayMaker, bool, Bool, JustChipToGoalInBelowDist)
    DEF_VALUE(PlayMaker, double, Double, ChipToGoalDist)
    DEF_VALUE(PlayMaker, double, Double, KickThreshold)
    DEF_VALUE(PlayMaker, double, Double, OneTouchKickThreshold)
    DEF_VALUE(PlayMaker, double, Double, OneTouchAngleThreshold)
    DEF_VALUE(PlayMaker, double, Double, OneTouchabilityAngleThreshold)
    DEF_VALUE(PlayMaker, double, Double, OppObstacleFactorInChip)
    DEF_VALUE(PlayMaker, double, Double, UnderEstimateTheirGoalie)
    DEF_TREE(OurKickOff)
    DEF_VALUE(OurKickOff, bool, Bool, ChipToGoal)
    DEF_TREE(OurIndirect)
    DEF_VALUE(OurIndirect, bool, Bool, ChipToGoal)
    DEF_VALUE(OurIndirect, bool, Bool, NoPass)
    DEF_VALUE(OurIndirect, bool, Bool, ChipToGoalInOurField)
    DEF_VALUE(OurIndirect, bool, Bool, ShadowyPosition)
    DEF_VALUE(OurIndirect, int, Int, IndirectType)
    DEF_TREE(OurDirect)
    DEF_VALUE(OurDirect, bool, Bool, ChipToGoal)
    DEF_VALUE(OurDirect, bool, Bool, ShadowyPosition)
    DEF_VALUE(OurDirect, bool, Bool, ChipToGoalInOurField)
    DEF_VALUE(OurDirect, bool, Bool, CornerChip)
    DEF_VALUE(OurDirect, bool, Bool, CornerKick)
    DEF_TREE(Defense)
    DEF_VALUE(Defense, bool, Bool, NoClear)
    DEF_VALUE(Defense, bool, Bool, NoPass)
    DEF_VALUE(Defense, bool, Bool, ChipClear)
    DEF_VALUE(Defense, bool, Bool, SwapClearer)
    DEF_VALUE(Defense, double, Double, OppObstacleFactorInChip)
    DEF_VALUE(Defense, double, Double, KickThreshold)
    DEF_VALUE(Defense, double, Double, ChipThreshold)
    DEF_VALUE(Defense, int, Int, MarkingDef)
    DEF_TREE(KKPlayOn)
    DEF_VALUE(KKPlayOn, std::string, String, KKPlanSQL)
    DEF_VALUE(KKPlayOn, int, Int, KKDefaultCycle)
    DEF_VALUE(KKPlayOn, int, Int, KKPassSpeed)
    DEF_VALUE(KKPlayOn, int, Int, KKKickSpeed)
    DEF_VALUE(KKPlayOn, int, Int, KKChipSpeed)
    DEF_VALUE(KKPlayOn, int, Int, KKShotSpeed)
    DEF_VALUE(KKPlayOn, int, Int, KKChipToGoalSpeed)
    DEF_TREE(PlayOff)
            DEF_VALUE(PlayOff, bool, Bool, IDBasePasser)
            DEF_VALUE(PlayOff, int, Int, PasserID)
            DEF_VALUE(PlayOff, bool, Bool, IDBaseOneToucher)
            DEF_VALUE(PlayOff, int, Int, OneToucherID)
            DEF_VALUE(PlayOff, bool, Bool, UseFastPlay)
            DEF_VALUE(PlayOff, bool, Bool, UseFirstPlay)
            DEF_VALUE(PlayOff, bool, Bool, UseBlockBlocker)
            DEF_VALUE(PlayOff, bool, Bool, UseForcedBlock)


    DEF_TREE(DynamicPlay)
    DEF_VALUE(DynamicPlay, double , Double , LowSpeedPass)
    DEF_VALUE(DynamicPlay, double , Double , MediumSpeedPass)
    DEF_VALUE(DynamicPlay, double , Double , HighSpeedPass)
    DEF_VALUE(DynamicPlay, double , Double , LowSpeedChip)
    DEF_VALUE(DynamicPlay, double , Double , MediumSpeedChip)
    DEF_VALUE(DynamicPlay, double , Double , HighSpeedChip)
    DEF_VALUE(DynamicPlay, bool, Bool, FarForward)
    DEF_VALUE(DynamicPlay, bool, Bool, NearForward)
    DEF_VALUE(DynamicPlay, double, Double, Area)
    DEF_VALUE(DynamicPlay, double, Double, DirectTrsh)
    DEF_VALUE(DynamicPlay, int, Int, SupportPriority)
    DEF_VALUE(DynamicPlay, bool, Bool, DribbleEveryWhere)
    DEF_VALUE(DynamicPlay, bool, Bool, DribbleInFast)
    DEF_VALUE(DynamicPlay, bool, Bool, ChipForward)



    DEF_TREE(Mark)
    DEF_VALUE(Mark, bool , Bool, PlayOffManToMan)
    DEF_VALUE(Mark, bool , Bool, PlayOnManToMan)
    DEF_VALUE(Mark, bool , Bool, ManToManAllTransiant)
    DEF_VALUE(Mark, bool , Bool, ManToManSomeTransiant)
    DEF_VALUE(Mark, bool , Bool, OmmitWhoCouldNotReceivePass)
    DEF_VALUE(Mark, double , Double, OppOmitLimitPlayon)
    DEF_VALUE(Mark, double , Double, OppOmitLimitPlayoff)
    DEF_VALUE(Mark, double , Double, OppOmitLimitKickOff)
    DEF_VALUE(Mark, double, Double, ShootRatioBlock)
    DEF_VALUE(Mark, double, Double, PassRatioBlock)
    DEF_VALUE(Mark, bool, Bool, OmmitNearestToBallPlayon)
    DEF_VALUE(Mark, bool, Bool, IntelligentMarkType)
    DEF_VALUE(Mark, bool, Bool, IntelligentMarkPrediction)
    DEF_VALUE(Mark, double, Double, VelReliability)



    public slots:
        void save();
    void load();
signals:

public slots:

};


void initVars(CVarsWidget* v);
CVarsWidget* conf();
void initPolicy(CPolicyWidget* v);
CPolicyWidget* policy();

#endif // VARSWIDGET_H
