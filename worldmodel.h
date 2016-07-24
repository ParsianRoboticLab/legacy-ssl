#ifndef WORLDMODEL_H
#define WORLDMODEL_H

#include <QQueue>
#include <QColor>
#include <QByteArray>

#include "ball.h"
#include "team.h"
#include "gamestate.h"
#include "Field.h"

#include "visionclient.h"

#include "varswidget.h"
#include "mathtools.h"
#include "motionestimator.h"
#include "logger.h"
#include "profiler.h"
#include "obstacle.h"
//#include "statisticalanalysis.h"
#include <QMutex>
#include "automatedreferee.h"

#include "util/hysteresisedcheck.h"
#include <QMap>

#define MAX_OBJECTS 3

struct SNewWorldModelStructRobot{
	Vector2D pos;
	Vector2D vel;
	int id;

	SNewWorldModelStructRobot( Vector2D _pos=Vector2D(0,0) , Vector2D _vel=Vector2D(0,0) , int _id =-1 ){
		pos = _pos;
		vel = _vel;
		id = _id;
	}
};

struct SNewWorldModelStruct{
	SNewWorldModelStructRobot ball; // USE AS BALL
	QList<SNewWorldModelStructRobot> our , opp;
	CField field;
};

class CHalfWorld
{
public:
    Vector2D positioningPoints[_MAX_NUM_PLAYERS];
	int positioningPointsCount;

	quint8 game_state;
	quint8 game_mode;

    CVisionBelief belief;
	QList<CRobot*> ourTeam[_MAX_NUM_PLAYERS];
	QList<CRobot*> oppTeam[_MAX_NUM_PLAYERS];
    QList<CBall*> ball;
	QString ourRole[_MAX_NUM_PLAYERS];
	QString oppRole[_MAX_NUM_PLAYERS];
    CVisionBelief* c;
    GameStatePacket gsp;
    int currentFrame;
    int playmakerID;
    bool closing;
    CHalfWorld();
    void track(QList<CRawObject>& p0, QList<CRawObject>& p);
    void update(QList<CRobot*>& robot, CVisionBelief* v, QList<CRawObject>& robot0, int id, bool our);
    void update(QList<CBall*>& ball, CVisionBelief* v);
    void update(CVisionBelief* v);
    void update(CHalfWorld* w);
    void merge();
    void vanishOutOfSights();
    void selectBall(Vector2D pos);
	QMap<QString, QString> knowledgeVars;
};


class CWorldModel : public QObject
{
	Q_OBJECT
private:

    ETeamColorType teamColor;
    ETeamSideType teamSide;
    bool visionOn;
    double commandTimeStep;    
    bool _gl_enabled;
    double _ode_dt;
    int _fps_desired;    
    void initStatAnalMani();    
public slots:
	void recvByte(char byte);
public:
        /// ---HMD ---///
        QList<Vector2D> markposes;
        /////////////////////
	QString ourRole[_MAX_NUM_PLAYERS];
	QString oppRole[_MAX_NUM_PLAYERS];

    CHalfWorld w;
    CTeam our,opp;
    CBall* ball;

    QQueue<QString> refCommand;
    QQueue<int> oppSupporterHist[_MAX_NUM_PLAYERS+2];

	GameState *gs;
    CField *field;
    CProfiler *profiler;
    //CAutomatedReferee *autoRef;

	enum TeamType{OURTEAM,OPPTEAM,BOTHTEAMS};

	CWorldModel(bool noKalman=false);
    ~CWorldModel();

//    CStatisticalAnalysis *statAnal;

    // controling simulator
    void setSimulatorOpenGLState(bool gl_enabled);
    void setSimulatorOpenDETimeStep(double ode_dt);
    void setSimulatorDesiredFPS(int fps_desired);
    int getSimulatorStateBuffer(char* buf);

    // command interval
    double commandSampleTime();

	CRobot *getFastestPlayerToPoint(QQueue<int> ids , TeamType teamType,Vector2D target);

    //side and color
    void setTeamColor(ETeamColorType _teamColor=_COLOR_YELLOW);
    ETeamColorType getTeamColor();
    void setTeamSide(ETeamSideType _teamSide);
    ETeamSideType getTeamSide();
    QColor getTeamQColor();
    QColor getOppQColor();
	double openness(Vector2D point, int selfId, double &minDist, double &angle, bool relaxOur=true, double radFactor=1.0);
	double getEmptyAngle(Vector2D p,Vector2D p1, Vector2D p2, QList<Circle2D> obs, double& percent, double &mostOpenAngle, double& biggestAngle, bool oppGoal, bool _draw = false);
    void generateObstacleSpace(CObstacles &obs, QList<int> &ourRelaxList, QList<int> &oppRelaxList, bool avoidPenaltyArea = true, bool avoidCenterCircle = true, double ballObstacleRadius = 0);

    static Vector2D whereBallFalls(Vector2D ballPos0, int speed, Vector2D dir);
    Vector2D ballCatchTarget(CRobot *r);
	void unmaskOccludedBall();
    //update functions
    void update(CHalfWorld* w0);
    void update(CVisionBelief res);
    void update(CTeam& _our, CTeam& _opp, CBall& _ball, double _visionLatency, double _timeStep);
    void updateFromString(std::string s,float logVersion);

    void setCommandTimeStep(double step);
    //config vars for pass positioning
    inline double passBigStepX() {return conf()->Pass_Positioning_Big_Step_X();}
    inline double passBigStepY() {return conf()->Pass_Positioning_Big_Step_Y();}
    inline double passSmallStepX() {return conf()->Pass_Positioning_Small_Step_X();}
    inline double passSmallStepY() {return conf()->Pass_Positioning_Small_Step_Y();}
    Property(float, VisionLatancy, visionLatency);
    Property(float, VisionTimeStep, visionTimeStep);
    Property(float, VisionFPS, visionFPS);
    Property(bool, IsSimulMode, isSimulMode);
};


extern CWorldModel* wm;
extern CHalfWorld* halfworld;
extern QMutex* halfworldMutex;

#endif // WORLDMODEL_H
