#include "tracker.h"
double FramePeriod = 0.016;
double LatencyDelay = 0.04;

Tracker::Tracker(int state_n, int obs_n, double _stepsize)
    : Kalman(state_n, obs_n, _stepsize)
{
}



void setFramePeriod(double framePeriod)
{
    FramePeriod = framePeriod;
}

double getFramePeriod()
{
	return FramePeriod;
}

void setLatencyDelay(double latencyDelay)
{
    LatencyDelay = latencyDelay;
}

double getLatencyDelay()
{
    return LatencyDelay;
}
