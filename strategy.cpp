#include "coach.h"
#include "roles.h"
#include <passevaluation.h>

#define LOG(key, value) debug(QString("%1:: %2").arg(key).arg(value), D_ALI);

void CCoach::swapAgents()
{
    //refer to selectedId in knowledge
    if (lastSelected == -1)
    {
        if (knowledge->selectedOur)
            lastSelected = knowledge->selectedId;
    }
    else {
        if (knowledge->selectedOur && knowledge->selectedId != -1)
        {
            swapAgents(lastSelected, knowledge->selectedId);
        }
        knowledge->selectedId = -1;
        lastSelected = -1;
    }
  knowledge->setSwapDefAndAtt(false);
}

bool CCoach::swapAgents(int i, int j)
{
  if( i == policy()->Formation_Goalie() || j == policy()->Formation_Defense() )
    return false;

  int i1 = knowledge->defenseAgents.indexOf(knowledge->getAgent(i));
  int j1 = knowledge->defenseAgents.indexOf(knowledge->getAgent(j));

  if( i1 != -1 && j1 != -1 ){
    return false;
  }

  if( i1 == -1 && j1 == -1 ){
    return false;
  }

  if( i1 == -1 && j1 != -1 ){
    /// j is defense, i is not
    CAgent *agnt;

    agnt = knowledge->getAgent(i);
    agnt->defIntent.assign(i , knowledge->frameCount);
    agnt->intention = &agnt->defIntent;

    agnt = knowledge->getAgent(j);
    agnt->intention = NULL;
  }
  else if( i1 != -1 && j1 == -1 ){
    /// i is defense, j is not
    CAgent *agnt;

    agnt = knowledge->getAgent(j);
    agnt->defIntent.assign(j , knowledge->frameCount);
    agnt->intention = &agnt->defIntent;

    agnt = knowledge->getAgent(i);
    agnt->intention = NULL;
  }

  //    int i1 = lastAssign.first.indexOf(knowledge->getAgent(i));
  //    int i2 = lastAssign.second.indexOf(knowledge->getAgent(i));
  //    int j1 = lastAssign.first.indexOf(knowledge->getAgent(j));
  //    int j2 = lastAssign.second.indexOf(knowledge->getAgent(j));
//    if (i1 != -1 && j1 != -1)
//    {
//        lastAssign.first.swap(i1, j1);
//    }
//    else if (i2 != -1 && j2 != -1)
//    {
//        lastAssign.second.swap(i2, j2);
//    }
//    else if (i1 != -1 && j2 != -1)
//    {
//        CAgent* tmp = lastAssign.first[i1];
//        lastAssign.first[i1] = lastAssign.second[j2];
//        lastAssign.second[j2] = tmp;
//    }
//    else if (i2 != -1 && j1 != -1)
//    {
//        CAgent* tmp = lastAssign.first[j1];
//        lastAssign.first[j1] = lastAssign.second[i2];
//        lastAssign.second[i2] = tmp;
//    }
//    else {
//        return false;
//    }
//	debug(QString("Swapping %1 with %2").arg(i).arg(j), D_ERROR);
//    return true;
}


void CCoach::setOpponents()
{
	QList <CRobot*> opp;
	int supporterID;
	for (int i = 0 ; i < _MAX_NUM_PLAYERS ; i++){
		if( wm->opp[i]->role.startsWith("supporter") ){
			supporterID = i;
			double ang = fabs(Vector2D::angleBetween(wm->opp[supporterID]->pos - wm->field->ourGoal() , Vector2D(0,0) - wm->field->ourGoal()).degree());
			Vector2D oppPoint = wm->opp[supporterID]->pos + (wm->opp[supporterID]->pos - wm->field->ourGoal()).norm()*0.09;
			if( wm->opp[supporterID]->pos.x < 2 && ang < 100.0 && wm->field->isInOurPenaltyArea(oppPoint) == false ){
				opp.append(wm->opp[supporterID]);
			}
		}
	}

	for (int i=0;i<_MAX_NUM_PLAYERS;i++)
		wm->opp[i]->danger = 0;

	QList<Circle2D> obs;
	if( goalieAgent != NULL ){
		if( goalieAgent->isVisible() )
			obs.append(goalieAgent->self()->getCirle());
	}
	for( int i=0 ; i<defenseAgents.size() ; i++ ){
		if( defenseAgents.at(i)->isVisible() ){
			obs.append(defenseAgents.at(i)->self()->getCirle());
		}
	}

	for (int i=0;i<opp.size();i++)
	{
		double posDanger = 0, percent, mostOpenAngle,biggestAngle;
		posDanger = (opp[i]->pos - (wm->field->ourGoal() + Vector2D( 1.3,0))).length() +
				(opp[i]->pos - (wm->field->ourGoal() + Vector2D(-1.3,0))).length();
		posDanger = exp(-posDanger*posDanger/(2.0*25));
		posDanger = posDanger * (1.0+tanh(((opp[i]->pos - wm->field->oppGoal()).length()-1.4)*5.0))*0.5;
		double openness = knowledge->getEmptyAngle(opp[i]->getKickerPos(), wm->field->ourGoalL(), wm->field->ourGoalR(), obs, percent, mostOpenAngle, biggestAngle, false);
		double danger = posDanger*(percent*0.9+0.1);
		draw(QString("Danger=%1").arg(danger, 3, 'f', 3), opp[i]->pos + Vector2D(0,0.3), "cyan");
		wm->opp[opp[i]->id]->danger = danger;
	}
	for( int i=0 ; i<opp.size() ; i++ ){
		for( int j=i+1 ; j<opp.size() ; j++ ){
			double xx_i = wm->opp[opp[i]->id]->danger;//opp[i]->pos.dist(wm->field->ourGoal()  + Vector2D(1.3, 0)) + opp[i]->pos.dist(wm->field->ourGoal() + Vector2D(-1.3, 0));
			double xx_j = wm->opp[opp[j]->id]->danger;//opp[j]->pos.dist(wm->field->ourGoal()  + Vector2D(1.3, 0)) + opp[j]->pos.dist(wm->field->ourGoal() + Vector2D(-1.3, 0));
			if( xx_i < xx_j )
				swap(opp[i] , opp[j]);
		}
	}

	for( int i=0 ; i<_MAX_NUM_PLAYERS ; i++ )
		if( wm->opp[i]->role.startsWith("supporter") )
			wm->opp[i]->role = "IDLE";

	for( int i=0 ; i<opp.size() ; i++ ){
		opp[i]->role = QString("supporter%1").arg(i+1);
		opp[i]->markedByDefense = false;
		opp[i]->markedByMark = false;
	}
}


/////////////! mhmmd: opponent agents to mark
int CCoach::mostSupporterNumber(int num){
	CRoleMarkInfo *markInfo = (CRoleMarkInfo*) CSkills::getInfo("mark");
	int mostSupporter[_MAX_NUM_PLAYERS+2] = {0} , best= 0 , toBeM = -1;
	for( int i=0 ; i<wm->oppSupporterHist[num].size() ; i++ ){
	   mostSupporter[wm->oppSupporterHist[num].at(i)]++;
	}

	for( int i=0 ; i<_MAX_NUM_PLAYERS+2 ; i++ ){
		if( best < mostSupporter[i] && markInfo->markedOpp.contains(i) == false ){
			best = mostSupporter[i];
			toBeM = i;
		}
	}
	if( toBeM == _MAX_NUM_PLAYERS+1 )
		toBeM = -1;
	return toBeM;
}
