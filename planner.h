#ifndef PLANNER_H
#define PLANNER_H

#include "geom.h"
#include "QList"
#include <vector>
#include "worldmodel.h"
#include "obstacle.h"
#include <string.h>
#include <QThread>
#include <QMetaType>


#define _PLANNER_EXTEND_POINT_LIMIT 150
#define _PLANNER_EXTEND_MAX_ATTEMPT 200

using namespace std;

class C2DTree{
public:
	C2DTree();
	~C2DTree();
	void add(state* const);
	void removeAll();
	state *findNearest(Vector2D);
	void drawBranch(state* , state* , QColor );
	int size();
	state* makeBalanced(state** , int , int , int);

	state *head , *first;
	int k;
	state *allNodes[1000];

private:
	void addNode(state* const & , state*& , int);
	state *findNearestNode( Vector2D & , state * const & , int);
	void removeBranch(state *);

	int cnt , mod;
};

class CPlanner
{
public:
	vector<Vector2D> result , Rresult , resultModified , RresultModified;
	Vector2D goal , lastGoal , Rgoal , unchangedGoal;
	double goalProb , wayPointProb;
	double threshold , stepSize , pointStep;
	CObstacles obst;
	bool avoidPenaltyArea , avoidCenterArea;
	double ballObstacleRadius;
    QList <int> ourRelaxList , oppRelaxList;
	C2DTree nodes;
	C2DTree Rnodes;
	int ID , counter;
	Vector2D robotVel;
	Vector2D averageDir;
	bool readyToPlan;
	bool flag;

	CPlanner();
	~CPlanner();
	void runPlanner();
	void resetPlanner(Vector2D);


private:
	QList <Vector2D> dirs;
	Vector2D chooseTarget(Vector2D &);
	state *extendTree(state *& , Vector2D & , Vector2D & , vector<Vector2D> &);
	bool validState( state *& );
	void checkAgent();
	void Draw();

	double drawTimer;
	bool isRandomState;
};


struct plannerResult{
	QList <Vector2D> path;
};

//Q_DECLARE_METATYPE(plannerResult)

class CPlannerThread : public QThread{
	Q_OBJECT
private:
	CPlanner planner[_MAX_NUM_PLAYERS];
  SNewWorldModelStruct mywma;
	QMutex mutex;
    Segment2D agentPath;

public:
	CPlannerThread();
	~CPlannerThread();
    void generateObstacleSpace(CObstacles &obs, QList<int> &ourRelaxList, QList<int> &oppRelaxList, bool avoidPenaltyArea, bool avoidCenterCircle , double ballObstacleRadius, int id, Vector2D agentGoal);
    double timeEstimator(Vector2D _pos,Vector2D _vel,Vector2D _ang,Vector2D _goal);
    void createObstacleProb(CObstacles &obs, Vector2D _pos, Vector2D _vel, Vector2D _ang, Vector2D &_center, double &_rad, Vector2D agentPos, Vector2D agentVel, Vector2D agentGoal, Vector2D agentDir);
    void run();



signals:
    void pathPlannerResultReady(int _id , vector<Vector2D> _resultModified , Vector2D _averageDir);

public slots:
    void initPathPlanner( int _id , Vector2D _goal,QList<int> _ourRelaxList, QList<int> _oppRelaxList , bool _avoidPenaltyArea , bool _avoidCenterArea , double _ballObstacleRadius );
	void updatePlannerWorldModel(SNewWorldModelStruct newWorldModel);
};

extern CPlannerThread *pathPlanner;
extern QMutex plannerMutex;

#endif // PLANNER_H
