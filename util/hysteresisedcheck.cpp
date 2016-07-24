#include "hysteresisedcheck.h"

HysteresisedCheck::HysteresisedCheck()
{    
    inited = false;    
    guardMin = guardMax = 0;
}

HysteresisedCheck::HysteresisedCheck(float _guardMin, float _guardMax)
{
    guardMin = _guardMin;
    guardMax = _guardMax;
    inited = false;
}

bool HysteresisedCheck::check(float x)
{
    if (!inited)
    {
        if (x > 0) lastState = true;
        if (x < 0) lastState = false;
        inited = true;
        return lastState;
    }
    if (x > guardMax) {
        lastState = true;
    }
    if (x < guardMin) {
        lastState = false;
    }
    return lastState;
}
