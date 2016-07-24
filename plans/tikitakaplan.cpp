#include "tikitakaplan.h"

#include <vector>
#include "gotopoint.h"
#include <cmath>

using namespace std;

TikiTakaPlan::TikiTakaPlan()
{
    for(int i = 0; i < 10; i++)
        gp.push_back(new CSkillGotoPointAvoid(NULL));
}

void TikiTakaPlan::execute()
{

    //wm->opp.activeAgentsCount();
    //wm->opp.active(id);
    //CAgent* agent(int i);
    //count();

    circleCenter = Vector2D(0, 0); // The circle is located at the center of the field, for now.
    circleRadius = 1.5;

    int closestTmm = closestToBall();
    //The closest teammate to the ball should be assigned an interception task.
    CSkillIntercept *ic = new CSkillIntercept(agent(closestTmm));

    // Find positions for the players so that they
    // are farthest from opponents while staying on the circle.
    // Each player is assigned a segment of the circle
    // to cover.

    for(int i = 0; i < count(); i++)
    {

    }
}

int TikiTakaPlan::closestToBall()
{
    int closestTmm = -1;

    for(int i = 0; i < count(); i++)
        if((agent(i)->pos() - wm->ball->pos).length() < (agent(closestTmm)->pos() - wm->ball->pos).length())
            closestTmm = i;
    return closestTmm;
}
