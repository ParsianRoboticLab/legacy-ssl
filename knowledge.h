#ifndef KNOWLEDGE_H
#define KNOWLEDGE_H

#include <agent.h>
#include <worldmodel.h>
#include "movingobject.h"
#include <joystick.h>
#include <QMap>
#include <QDebug>
#include <QVector2D>

#include "defpos.h"
#include "proto/multi_team_communication.pb.h"

//#include "simulation/simulator.h"
//#include <widgets.h>

struct DefensePositions
{
    QList<Vector2D> defense;
    Vector2D goalie;
};

struct AngleRange {
    AngleDeg begin;
    AngleDeg end;
};

struct NewFastestToBall
{
    double catch_time;
    bool isFastestOurs;
    QList< pair<double , int> > ourF;
    QList< pair<double , int> > oppF;
    NewFastestToBall(){
        catch_time = 10000;
        isFastestOurs = false;
        ourF.clear();
        oppF.clear();
    }
    int ourFastest(){
        if(ourF.size())
            return ourF.first().second;
        return -1;
    }
    double ourFastestTime(){
        if(ourF.size()){
            return ourF.first().first;
        }
        return -1;
    }
    int oppFastest(){
        if( oppF.size() )
            return oppF.first().second;
        return -1;
    }
    double oppFastestTime(){
        if(oppF.size())
            return oppF.first().first;
        return -1;
    }
};

struct FastestToBall
{
    double catch_time;
    int ourFastest;
    double ourFastestTime;
    int oppFastest;
    double oppFastestTime;
    FastestToBall(){
        catch_time = 1000;
        ourFastest = -1;
        ourFastestTime = -1;
        oppFastest = -1;
        oppFastestTime = -1;
    }
};

class FormationCounts
{
public:
    int goalie;
    int defenders;
    int attackers;
    FormationCounts();
};

struct velAndAccByKK {
    double vel;
    double acc;
};

struct SRAgentArgs {
    double Vx;
    double Vy;
    double Vr;
    double KickSpeed;
    double ChipSpeed;
    double SpinSpeed;
    bool updated;
};


class CKnowledge
{
private:
    //added
    Vector2D bpPosition;


protected:
    CAgent** agents;
    int m_ballOwner;
    bool m_ballOurs;
    int defense_state;
public:

    enum State
    {
        Halt = 0,
        Stop = 1,
        OurKickOff = 2,
        TheirKickOff = 3,
        OurDirectKick = 4,
        TheirDirectKick = 5,
        OurIndirectKick = 6,
        TheirIndirectKick = 7,
        OurPenaltyKick = 8,
        TheirPenaltyKick = 9,
        Start = 10,
        NormalStart = 11,
        //added
        OurBallPlacement = 12,
        TheirBallPlacement = 13,
        HalfTimeLineUp=14
    };
    enum Support
    {
        Back = 0,
        Front = 1,
        Right = 2,
        Left = 3
    };
    enum ballPossesionState {
        WEDONTHAVETHEBALL = 0,
        WEHAVETHEBALL = 1,
        SOSOOUR = 2,
        SOSOTHEIR = 3
    };

    ballPossesionState ballPossesion;


    //added
    Vector2D getBPPosition();
    void setBPPosition(float x, float y);

    //added for mixteam TC
    multi_team_comm::TeamPlan *kPlans;
    int ssize;
    bool ready = false;
    QList<CAgent*> activesInField;
    int mixGoaleID;
    QList<int> ourAgentIDsMixTeam;

    void getOurRobotIDsFromGUIMixTeam();



    class PlaymakerSelector
    {
    private:


        int passRecvTarg;
        int passRecvTargFrame;
        int playmaker;
        int lastPlaymakerSelectFrame;
        double distance;
        QList<CAgent*> agents;
        QList<CAgent*> lastAgents;

    public:

        PlaymakerSelector();

        int getPlayMakerId();
        void setAgents(QList<CAgent*> agents);
        int select();
        void setPassRecvTarget(int id);
    } playmakerSelector;
    bool transientFlag;

    int newFastestSelector(QList <CAgent*> _agents);
    int nonPlayOnFastestSelector(QList <CAgent*> _agents);
    void sortByX(QList <CAgent *> &_agents );
    void sortByY(QList <CAgent *> &_agents );
    int Matching(const QList <CAgent*> robots, const QList <Vector2D> pointsToMatch, QList <int> &matchPoints);
    void fastMatching(const QList <CAgent*> robots, const QList <Vector2D> pointsToMatch, QList <int> &matchPoints);
    int factorial(int a);

    QMap<QString, int> formationChanges;
    QMap<QString, int> formationChangeFrames;

    QList<int> ourAgents, assistingAgents; //For mixed-team

    Vector2D positioningPoints[_MAX_NUM_PLAYERS];
    int positioningPointsCount;
    static QStringList TechnicalModes;

    int closeGoalie;
    int lastFrameShirjeZanDetected;
    CKnowledge(CAgent** _agents);
    ~CKnowledge();
    void resetEssentialVars();
    ////////////////////mhmmd time
    double mainLoopTime;
    double getRealBallVel();
    Vector2D ballPosHis[5];
    double ballVelLowPass;
    /////////////////////////
    ///////////////////////////////// AHZ //////////////////////////////////////
    Vector2D getPointInDirection(Vector2D firstPoint , Vector2D secondPoint , double proportion);
    //////////////////////////////// end of AHZ
    QList<int> oppBlockers;
    QString stateToString(State s);
    int ourShirjeBlocker;
    int lastFrameShirjeBlock;
    bool isSimulMode;
    void updateGameState();
    //////////////Mahmoud
    //chip dir predict
    Vector2D getChipDir();
    double chipperDistance=100;
    qint32 chipperIDD=-1,chipperID=-1;
    bool DirFound=false;
    Vector2D chipperDir=Vector2D(0,0);
    Vector2D prevBallPos,chipperPoint=Vector2D(0,0);
    QMap<qint32,QList<Vector2D> > lastDirs;
    //chip predict
    QList<Vector2D> predictedBallPoses;
    bool flagN = true;
    QList<double> prev_Ball_pos;
    Vector2D dir;
    Vector2D startChipPoint ;
    double startBallVel;
    double previousPos=0.0;
    double prevPos=0.0;
    double chipPredictCounter = 1;
    Vector2D predictedPosition=Vector2D(0,0);
    double refiner=1.3;

    double refine(double x);
    double predictPos();
    Vector2D getChipPredict();
    //
    int getChipArea(double);
    //
    Vector2D getBestPosToShootToGoal(Vector2D from, double &regionWidth, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, bool oppGaol);
    Vector2D getBestPosForPassReciever(Rect2D searchRegion, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, Rect2D avoidRect);
    Vector2D getBestPosForPassReciever(QList<Rect2D> searchRegions, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, QList<Rect2D> avoidRects,int passRecieverID, int passSenderID,double angleFactor,double angle0);
    Vector2D getBestPosForPassThrough(int throughlevel, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs);
    Vector2D findPosition(QList<Rect2D> searchRegions,  QList<Rect2D> avoidRects, int agent, int passSender);
    bool isPointInOurPenaltyArea(Vector2D point);
    int getNearestAgentToPoint(Vector2D point, QList<int>* agentIDs);
    int getNearestOppToPoint(Vector2D point);
    int findNearestopp();
    int findOppGoalie();
    void findOppDefenders(QList<int> &defenders, int &golie);
    bool isCrowdedInFrontOfAgent(int id, double radius);
    bool isCrowdedInFrontOfBallOwner(int ballowner);
    void findDangerousOpps(QList<int> &opps);
    int findKickerAgent();
    int findOppBallOwner();
    int findOurBallOwner();
    void findBallOwners();
    float oneTouchAngleThreshold();
    float oneTouchKickThreshold();
    float kickThreshold();
    float kickClosedAngle();
    Vector2D getReflectPos(Vector2D goal, double dist);
    ////////////////////////////////<Mahi>
    Vector2D getReturnPos(Vector2D _goal);
    ///////////////////////////////////</Mahi>
    bool canSendPass(int sender, int receiver, Vector2D point, double factor);
    int getBallOwner(bool& ours);
    bool isBallOurs();
    bool isPointClear(Vector2D point, Vector2D from, double radBig, double radSmall, bool considerRelaxedIDs, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, QList<int> ourSmallIDs, QList<int> oppSmallIDs);
    bool isPointClear(Vector2D point, Vector2D from, double rad, bool considerRelaxedIDs=false, QList<int> ourRelaxedIDs=QList<int>(), QList<int> oppRelaxedIDs=QList<int>());
    Vector2D onetouchablity(int agentId, double &goalWidth, double &angle, double &coming,int senderId=-1, double underestimateTheirGoalie=1.0);
    Vector2D getEmptyPosOnGoalForPenalty(double n, bool oppGoal, double th, CAgent* ourAgent=NULL);
    Vector2D goalVisiblity(int agentId, double &regionWidth, double underestimateTheirGoalie);
    Vector2D getEmptyPosOnGoal(Vector2D from, double &regionWidth, bool oppGoal, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs, double wOpenness = 1.0, bool _draw = false);
    Vector2D getEmptyPosOnPoints(Vector2D from, double &regionWidth, QList<Vector2D> points, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs);
    Vector2D findBestPosToCatchTheBall(int agentID, Vector2D& lastBestPos);
    Vector2D getBestShadowPoint(Vector2D pos, Vector2D goal);
    QString getMarkableNumber();
    double matchPositions(QList<int> ids, QList<Vector2D> points, QList<int>& bestPermutation);
    int desiredDefCount;
    /* New Simple getOpen() for mani-thesis */
    Vector2D findGetOpenPointForAgent(int id, Rect2D searchSpace, double res);
    CAgent* getAgent(int i);
    QList<CAgent*> getActiveAgents();
    Vector2D ballCatchTarget(CRobot *);
    int agentCount();
    Vector2D Desired_defPos1, Desired_defPos2, Desired_goaliePos;
    int defenseClearingFrame;
    bool thePassProgress;
    double passProgressTime;

    int lastFrameTheirNonPlayKick;
    int defenseClearer;
    int toBeSwitchedAttacker;
    int lastFrameSwithed;

    QList<double> lastFrameRates;
    double commandFrameRate;
    int frameCount;
    bool warmup;
    int ballOwner;
    int nearestToBall;
    int nearestToBallNotGoalie;
    int nearestAttackerToBall;
    int nearestOppToBall;
    int oppGoalieIndex;
    int oppBallOwner;
    int cornerChipPhase;
    int disturbationPhase;
    bool roleOptimization;
    Vector2D shootPos;
    Vector2D passPos;
    double goalProbablity;
    bool clearing;
    bool defensePassSent;
    bool theyOwnBall;
    bool gameInKhafanMode;
    double khafanTime;
    int khowf;
    int selectedId;
    bool selectedOur;
    int shirjezan; //shirje zan e mrl
    double currentTime();
    long getCurrentTime();
    void playChanged();
    double findDangerPercent(CRobot *);
    double getEmptyAngle(Vector2D p,Vector2D p1, Vector2D p2, QList<Circle2D> obs, double& percent, double &mostOpenAngle, double& biggestAngle, bool oppGoal = true, bool _draw = false);
    void generateDefensePositions(int defenses, bool goalie, QList<Circle2D> avoidCircles, QList<Vector2D> &defendersPos, Vector2D &goaliePos, QList<DefensePositions> neighbourStates, double distanceFactor, Vector2D goalieCurrentPos = Vector2D::INVALIDATED);
    void generateDefensePositions(int defenses, bool goalie, QList<Circle2D> avoidCircles, QList<Vector2D> &defendersPos, Vector2D &goaliePos, Vector2D goalieCurrentPos = Vector2D::INVALIDATED);
    void checkShootDanger();
    void Aminshoot(Vector2D ball, QList<Circle2D> obstacles, double& _empty, Vector2D& _best);
    int getProfile(int agentId, double realParameter, bool isKick=true, bool spinOn=false);
    double chipGoalPropability(bool isOurChip);
    void calculateCommandFrameRate();
    double lastTimeCommandFPSCalced;
    FormationCounts formation;
    CAgent* goalie;
    QList <CAgent*> defenseAgents;
    double goalie_ellipse_a, goalie_ellipse_b;
    Vector2D goalie_shr;
    float getAxes(int i);
    QPair<QList<int>, QList<int> > currentFormation;

    QList<AngleRange> emptyAngles; //emptyAngle fills this list

    Vector2D getPotentialOnPoint(Vector2D base, Vector2D obstacle, double k1, double k2);

    CJoystick* joystick;
    void setJoystic( CJoystick* _joystic);
    // Temp added for old plotter
    void updatePlotterData();
    CMovingObject plotterBall;
    QStringList executingPlays;
    QMap<QString, QString> variables;
    QMap<int,int> markDocument;
    QMap<QString, QList<CAgent*> > roleAssignments;
    QList<bool> ownership;
    bool ownerShipCalculated;
    bool ballOurs;
    int lastBallOwner;
    bool lastBallOurs;
    QString behavioursDebug;

    bool sentChipPass;
    bool sendingChipPass;
    Vector2D chipPassFallPoint;
    int chipPassSender;
    CAgent* gameStarter;
    int lastFramePAreaAvoided;
    int currentPlayAllowedAgents;
    FastestToBall findFastestToBall(QList<int> ourList=QList<int>(), QList<int> oppList=QList<int>());
    NewFastestToBall newFastestToBall(double timeStep = 0.1, QList<int> ourList=wm->our.data->activeAgents, QList<int> oppList=wm->opp.data->activeAgents);
    bool matchdebug;
    double loopTime, maxLoopTime, visionProcessTime;
    double *plotWidgetCustom;
    Rect2D antiKhafanRect;
    Vector2D fixedPassPoint;
    Vector2D fixedPositionPoint;
    Vector2D fixedPassPointDef;
    Vector2D fixedPassPointsw;
    Vector2D fixedPositionPointDef;

    bool shirjeBlocking;
    int switchState;

    int khafanMarker;
    int khafanMarkFrame;


    bool isStop();
    bool isStart();
    bool isOurNonPlayOnKick();
    bool isTheirNonPlayOnKick();

    Property(QString, TechnicalMode, technicalMode);
    Property(CKnowledge::State, GameMode, gameMode);
    Property(CKnowledge::State, GameState, gamestate);
    Property(CKnowledge::Support, SupporPlaymaker, supporPlaymaker);
    Property(bool, Attaking3Attaker, attaking);
    Property(double, LastTimeGameStateChanged, lasttimegschanged);
    Property(int , LastPlayExecuted , lastPlayExecuted);
    PropertyGet(int, LastFramePlayChanged, lastFrameplaychanged);
    PropertyGet(CKnowledge::State, LastGameState, lastgamestate);
    PropertyGet(bool, GameStateChanged, gamestatechanged);
    Property(Vector2D, MousePos, mousePos);
    Property(int, NumOfAttackers, numofattackers);
    Property(int, NumOfDeffenders, numofdeffenders);
    Property(int, NumOfGoalie, numofgoalie);
    Property(CAgent*, PlayMaker, playMaker);
    Property(bool, BallChiped, ballChiped);
    Property(bool, BallCommingFromTop, ballCommingFromTop);

    Property(bool, NonPlayOnDefenseUse, nonPlayOnDefenseUse);
    Property(int, NonPlayOnDefenderID, nonPlayOnDefenderID);
    Property(int , ExperimentalMode , experimentalMode);

    Property(bool , RefShortcuts , refShortcuts);
    Property(bool , SwapDefAndAtt , swapDA);
    Property(bool , RushInPenaltyArea , rushInPenaltyArea);
    Property(bool , IndirectSafe , indirectSafe);


public:
    QList <int> onlineRobots;
    QList <CRobot*> toBeMopps;
    QList <CRobot*> toBeMoppsPast;
    QList <int> agentsWithIntention;
    QList<Vector2D> ballPosHistory;
    velAndAccByKK getVelocityByPos();

private:
    //added by KK
    SRAgentArgs CRAgent[_MAX_NUM_PLAYERS];
    bool necessaryDefKick;
    QList<Vector2D> staticPoses;

    int refRobotID;
    double RobotsCoeff[16][4];
    double ProfilerResult[16][4][81];
public:

    //added by Mahi
    CNewProfiler *profiler;
    double getKickSpeedProfile(int agentId,double kickSpeedInput);

    Vector2D getStaticPoses(int num);
    void setNecessaryDefKick(bool tempNcssryDefKick);
    bool getNecessaryDefKick();
    bool SRGetAgentArg(int _id, SRAgentArgs &_arg) ;
    void SRSetAgentArg(int _id, double _Vx, double _Vy, double _Vr, double _KickSpeed = 0, double _ChipSpeed = 0, double _SpinSpeed = 0);
    void SRSetAgentAbsArg(int _id, double _Vx, double _Vy, double _Vr, double _KickSpeed = 0, double _ChipSpeed = 0, double _SpinSpeed = 0);
    bool SRIsUpdated(int _id);

    //ABP....................ROBO:{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11}
    int agentSetRollerABPbB[12] = { 4, 4, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4};
    //completed with profiler



};





QList<Vector2D> getListOfPointsOnPerpendicularLine(Vector2D Point, Vector2D Q, double lLength,int n);


extern CKnowledge* knowledge;


#define DefenseGap 0.1

#endif // KNOWLEDGE_H
