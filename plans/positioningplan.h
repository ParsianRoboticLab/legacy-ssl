#ifndef POSITIONINGPLAN_H
#define POSITIONINGPLAN_H

#include <plans/plan.h>
#include "formation/edit_data.h"

enum positioningType{
    ONETOUCH,
	TOBALL,
	TOOPPGOAL,
	TOOURGOAL
};

enum edgeMode {
    TOP,
    BOT
};

class holdingPoints
{
public:
	CAgent *player;
	QList< Vector2D > points;
	QList< int > cyclesToWait;
	QList< double > escapeRadius;
	QList<positioningType> dir;
	holdingPoints(){}
	holdingPoints( CAgent *_player, QList< Vector2D > _points, QList< int > _cyclesToWait, QList< double > _escapeRadius , QList<positioningType> _dir )
	{
		player = _player;
		points.clear();
		points.append(_points);
		cyclesToWait.clear();
		cyclesToWait.append(_cyclesToWait);
		escapeRadius.clear();
		escapeRadius.append(_escapeRadius);
		dir.clear();
		dir.append(_dir);
	}
};

class Comparar
{
public:
    bool operator()(QPair< edgeMode , double >& s1, QPair< edgeMode , double >& s2)
    {
        if (s1.second < s2.second)
            return true;
        else
            return false;
    }
};

class PositioningPlan : public Plan
{
private:
	QList< holdingPoints > positionStaticPoints;

    CSkillGotoPoint* gps[_MAX_NUM_PLAYERS];
    CSkillGotoPointAvoid *gpa[_MAX_NUM_PLAYERS];
    Vector2D positioningTargets[_MAX_NUM_PLAYERS];
	Vector2D staticPositioningTargets[_MAX_NUM_PLAYERS];
	Vector2D staticPositioningTargetsInput[_MAX_NUM_PLAYERS];
	Vector2D staticPositioningFacePoints[_MAX_NUM_PLAYERS];
    Vector2D lastStaticPositioningTargets[_MAX_NUM_PLAYERS];
	EditData *editData;
	int posCount;
	double staticEscapeRadius[_MAX_NUM_PLAYERS];
	int executedCycles[_MAX_NUM_PLAYERS];
	Vector2D rolesPositions[_MAX_NUM_PLAYERS];
    QList<int> dynamicPositioners;
    QList<int> staticPositioners;
    int staticStateNo[_MAX_NUM_PLAYERS];

    int match[_MAX_NUM_PLAYERS];
    int last_match[_MAX_NUM_PLAYERS];

    bool dfs( int v , int n , int adj[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS] , int mark[] , int s[] , int t[] );
    bool matching( int n , int adj[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS] , int s[] , int t[] , int sa[] );
    void getBipartiteGraph( int n , int m , int weight[_MAX_NUM_PLAYERS][_MAX_NUM_PLAYERS] );

	bool isValidPoint( Vector2D target , bool callFromStaticPositioner = false );
  bool isDifferentSequence(QList<Vector2D> first , QList<Vector2D> second);

	Vector2D findFacePoint(positioningType type);

public:
    PositioningPlan();
	void init(const QList<CAgent *> &_agents , EditData *_editData , QString playMode );
	void staticInit( QList< holdingPoints > &_staticPoints );
	void staticExec();
	void execute();
    void reset();

    class PositioningObject
    {
    private:

        int positioner;
		Vector2D homePos, target, last_target;

        double score;

        static const double target_to_goal_openness_coeff = 100.0;
        static const double target_to_ball_openness_coeff = 100.0;
        static const double target_opp_mean_dist_coeff    = 250.0;
//        static const double target_to_home_pos_dist_coeff =-1.0;
		static const double target_to_last_target_coeff    =-10.0;//-100.0;

    public:

        PositioningObject( const int _positioner ,
                           const Vector2D _homePos ,
						   const Vector2D _target ,
                           const Vector2D _last_target );

        int Score()
        {
            return score;
        }

        double target_to_goal_openness();
        double target_to_ball_openness();
        double target_opp_mean_dist();
        double target_to_home_pos_dist();
        double target_to_last_target();
		double getOpenness(Vector2D from, Vector2D p1, Vector2D p2, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs);
		double coveredArea( std::priority_queue < QPair< edgeMode , double > , vector< QPair< edgeMode , double > > , Comparar >& obstacles );
    };
};

#endif // POSITIONINGPLAN_H
