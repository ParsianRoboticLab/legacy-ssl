#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "base.h"

#define OBS_RECTANGLE  0
#define OBS_CIRCLE     1

#include "geom.h"
#include <QList>
#include <vector>

using namespace std;

struct state{
	int depth;
	double dist;
	Vector2D pos;
	state *parent , *next;

	state *left , *right;

	state(Vector2D _pos  , state *_parent , vector<Vector2D> &res){
		pos = _pos;
		parent = _parent;
		if( parent ){
			depth = parent->depth + 1;
			if( depth < res.size() )
				dist = parent->dist + pos.dist(res[depth]);
			else if( res.size() )
				dist = parent->dist + pos.dist(res[res.size()-1]);
			else
				dist = parent->dist + pos.dist(parent->pos);
		}
		else{
			depth = 0;
			if( depth < res.size() )
				dist = pos.dist(res[depth]);
			else if( res.size() )
				dist = pos.dist(res[res.size()-1]);
			else
				dist = 0;
		}

		left = right = next = NULL;
	}
	state(){
		pos.invalidate();
		parent = left = right = next = NULL;
	}
};


class obstacle{
public:
	Vector2D pos;
	Vector2D rad;
	Vector2D vel;  // object velocity
	int type;
public:
  double margin(state s , double obsMargin);
  bool check(state s , double obsMargin);
  bool check(state s0,state s1 , double obsMargin);
};

class CObstacles{
public:
  QList <obstacle> obs;
public:
  CObstacles();

  void clear() {obs.clear();}
  void add_rectangle(double cx,double cy,double w,double h);
  void add_circle(double x,double y,double radius,
				  double vx,double vy);
  bool check(Vector2D p);
  bool check(Vector2D p,QList<int> &id);
  bool check(state s);
  bool check(state s,QList<int> &id);
  bool check(state s0,state s1);
  bool check(state s0,state s1,QList<int> &id);
  bool check(Vector2D p1, Vector2D p2);
  void draw();

  double obsMargin;
};

double boundTo(double x,double low,double high);

#endif // OBSTACLE_H
