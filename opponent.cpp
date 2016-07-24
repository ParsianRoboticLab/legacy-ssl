#include "opponent.h"
#include <knowledge.h>
#include <QVector>
#include <QDebug>

COpponent* opponent;

COpponent::COpponent()
{
	registerOppRole(new COpponentGoalie());
	registerOppRole(new COpponentDefender());
	registerOppRole(new COpponentDefender());
	registerOppRole(new COpponentNearestToBall());
	registerOppRole(new COpponentSupporter1());
	registerOppRole(new COpponentSupporter2());
	registerOppRole(new COpponentDummy());
	registerOppRole(new COpponentDummy());
}

void COpponent::registerOppRole(COpponentRole *role)
{
	roles.append(role);
}

bool COpponent::findRoles()
{
	for (int i=0;i<_MAX_NUM_PLAYERS;i++)
	{
		wm->opp[i]->role = "";
	}
	//Nearest to ball
	int nearestToBall = knowledge->getNearestOppToPoint(wm->ball->pos);
	if( nearestToBall != -1)
	{
		wm->opp[nearestToBall]->role = "nearest_to_ball";
		knowledge->nearestOppToBall=nearestToBall;
	}

	//defenders
	QList<int> defs;
	int goalie;
	knowledge->findOppDefenders( defs, goalie);
	for ( int i = 0; i < defs.count(); i++)
	{
		wm->opp[defs.at(i)]->role = "defender";
	}

	//Goalie
	goalie = knowledge->findOppGoalie();
	if ( goalie != -1 )
		wm->opp[goalie]->role = "goalie";

	//The rest of robots has no role
	QVector <int> noRoles;
	noRoles.clear();
	for ( int i = 0; i < wm->opp.activeAgentsCount(); i++)
	{
		if ( wm->opp.active(i)->role == "")
		{
			noRoles.push_back(wm->opp.active(i)->id);
		}
	}
	//puts in ascending order
	for( int i=0 ; i<noRoles.size() ; i++ )
		for( int j=1 ; j<noRoles.size() ; j++ )
			if( wm->opp[noRoles[j]]->pos.dist(wm->field->ourGoal()) < wm->opp[noRoles[j-1]]->pos.dist(wm->field->ourGoal()) ){
				swap(noRoles[j],noRoles[j-1]);
//				tmp = noRoles[j];
//				noRoles[j] = noRoles[j-1];
//				noRoles[j-1] = tmp;
			}
	for ( int i = 0; i < noRoles.count(); i++){
		wm->opp[noRoles.at(i)]->role = QString("supporter%1").arg(i+1);
	}
	return true;
}


double COpponentDummy::distance(CRobot *r)
{
	return 100;
}

QString COpponentDummy::name()
{
	return "dummy";
}

int COpponentDummy::priority() {return -1;}

double COpponentGoalie::distance(CRobot *r)
{
	double d = (r->pos - wm->field->oppGoal()).length();
	if ( d > 1.5)
		return 100;
	return d;
}

QString COpponentGoalie::name()
{
	return "goalie";
}



double COpponentDefender::distance(CRobot *r)
{
	Vector2D p;
	p = r->pos;
	p *= -1.0; //treat him as a teammate
	//we don't have proper functions for oppPenalty, thus we use functions developed for ourPenalty
	QList<Vector2D> q = wm->field->ourPAreaIntersect(Segment2D ((p-wm->field->ourGoal()).norm() * 2.0 + wm->field->ourGoal(), wm->field->ourGoal()) );
	if (q.length()==0)
	{
		return (p - wm->field->ourPenalty()).length();
	}
	return (q[0] - p).length();
}

QString COpponentDefender::name()
{
	return "defender";
}



double COpponentNearestToBall::distance(CRobot *r)
{
	return (wm->ballCatchTarget(r) - r->getKickerPos()).length();
}

QString COpponentNearestToBall::name()
{
	return "nearest_to_ball";
}


double COpponentSupporter1::distance(CRobot *r)
{
	//    return (r->pos-wm->field->ourGoal()).length();
	return knowledge->findDangerPercent(r);
}

QString COpponentSupporter1::name()
{
	return "supporter1";
}

double COpponentSupporter2::distance(CRobot *r)
{
	//    return (r->pos-wm->field->ourGoal()).length();
	return 1-knowledge->findDangerPercent(r);
}

QString COpponentSupporter2::name()
{
	return "supporter2";
}


int COpponentGoalie::priority() {return 0;}
int COpponentDefender::priority() {return 2;}
int COpponentNearestToBall::priority() {return 1;}
int COpponentSupporter1::priority() {return 3;}
int COpponentSupporter2::priority() {return 3;}
