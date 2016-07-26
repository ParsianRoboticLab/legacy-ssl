#include "khersdefense.h"
#include <worldmodel.h>
#include <vector>
#include <WMatching.h>

using namespace std;

KhersDefense::KhersDefense()
{
	for (int i = 0; i < 10; i++)
		gp[i] = new CSkillGotoPoint(NULL);
	kick = new CSkillKick(NULL);

}

void KhersDefense::execute()
{
	float minDist = 20.0;
	CRobot* nearestOpp;
	Vector2D secondNearestPos;
	int secondNearestIndex=-1;
	Vector2D ballPos  = wm->ball->pos;

	for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++)
	{
		float dist = wm->opp.active(i)->pos.dist(ballPos);
		if (wm->opp.active(i)->role != "nearest_to_ball" and dist < minDist)
		{
			secondNearestIndex = i;
			minDist = dist;
			secondNearestPos = wm->opp.active(i)->pos;
		}
		else if (wm->opp.active(i)->role == "nearest_to_ball")
			nearestOpp = wm->opp.active(i);
	}
	debug(QString("IO case DEBUG: %1").arg(nearestOpp==NULL), D_DEBUG);
	debug(QString("NEAR_POS: %1 , %2").arg(nearestOpp->pos.x).arg(nearestOpp->pos.y), D_DEBUG);
//	vector<Vector2D> otherOpp;
//	for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++)
//		if (i != secondNearestIndex and wm->opp.active(i)->role != "nearest_to_ball")
//			otherOpp.push_back(wm->opp.active(i)->pos);

	Vector2D firstTarget = nearestOpp->dir.norm() * (0.5 + 0.4) + nearestOpp->pos;
	firstTarget.x = max(-3.0, min(firstTarget.x, 3.0));
	firstTarget.y = max(-2.0, min(firstTarget.y, 2.0));
	Vector2D secondTarget = (ballPos - secondNearestPos).norm() * 0.2 + secondNearestPos;;
	secondTarget.x = max(-3.0, min(secondTarget.x, 3.0));
	secondTarget.y = max(-2.0, min(secondTarget.y, 2.0));

	draw(Circle2D(firstTarget, 0.1), "white", true);
	debug(QString("FIRST: %1 %2").arg(firstTarget.x).arg(firstTarget.y), D_DEBUG);
	draw(Circle2D(secondTarget, 0.1), "cyan", true);

	int agentCount = min(count(), 2);
	vector< vector<int> > wvec;
	for (int i = 0 ; i < agentCount ; i++)
	{
		wvec.push_back(vector<int>());
		wvec[i].push_back(agent(i)->pos().dist(firstTarget));
		wvec[i].push_back(agent(i)->pos().dist(secondTarget));
	}

	WMatching wmatch(agentCount, wvec);
	wmatch.calculate();

//	vector<int> newTable(agentCount);
//	for (int i = 0 ; i < agentCount ; i++)
//		newTable[wmatch.match[i]]
	vector<Vector2D> targetVec;
	targetVec.push_back(firstTarget);
	targetVec.push_back(secondTarget);

	for (int i = 0 ; i < agentCount ; i++)
		assignSkill(i, gp[i]);
	for (int i = 0 ; i < agentCount ; i++)
	{
		bool safe = true;
		Vector2D pred = wm->ball->pos + wm->ball->vel*0.6;
		for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++)
		{
			float dist = wm->opp.active(i)->pos.dist(ballPos);
			float distP = wm->opp.active(i)->pos.dist(pred);
			if ( dist < 1.2 || distP < 1)
			{
				safe = false;
			}
		}
//		Vector2D target = (i == 0 ? firstTarget : secondTarget);
		Vector2D target = targetVec[i];
		if( target == firstTarget && safe)
			gp[/*wmatch.match[i]*/i]->init(pred, ballPos - target);
		else
			gp[/*wmatch.match[i]*/i]->init(target, ballPos - target);
		draw(Circle2D(agent(/*wmatch.match[i]*/i)->pos(), 0.2), "red");
	}

//	if (firstTarget.dist(taget) < secondTarget.dist())
}

bool KhersDefense::differ(const vector<int>& table1, const vector<int>& table2)
{
	if (table1.size() != table2.size())
		return true;
}
