#ifndef HYSTERESISEDCHECK_H
#define HYSTERESISEDCHECK_H

#include "base.h"

class HysteresisedCheck
{
private:
    bool inited;
public:  
    HysteresisedCheck();
    HysteresisedCheck(float _guardMin, float _guardMax);
    bool check(float x);
    Property(float, GuardMin, guardMin);
    Property(float, GuardMax, guardMax);
    PropertyGet(float, State, lastState);
};

typedef HysteresisedCheck Hyst;

#endif // HYSTERESISEDCHECK_H
