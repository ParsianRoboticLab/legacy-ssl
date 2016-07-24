#ifndef TIKITAKAPLAN_H
#define TIKITAKAPLAN_H

#include "plan.h"
#include <vector>


// Players should position along the perimeter of a circle
// which encloses the opponents. Our players' positions are
// calculated accordingly to that of the opponents.


class TikiTakaPlan : public Plan
{
private:
    double circleRadius;  // Radius of the central circle.
    Vector2D circleCenter;
    std::vector<CSkillGotoPointAvoid *> gp;
    int closestToBall();


public:

    void execute();


    TikiTakaPlan();
};

#endif // TIKITAKAPLAN_H
