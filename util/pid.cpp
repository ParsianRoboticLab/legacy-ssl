#include "pid.h"
#include "logger.h"






CPID::CPID()
{
	dt = 1;outputLimit = -1;
    lastRetVal = 0;
    reset();
    setParams(0,0,0);
}

CPID::CPID(double _kp, double _ki, double _kd)
{
	dt = 1;outputLimit = -1;
    lastRetVal = 0;
    reset();
    setParams(_kp,_ki,_kd);
}

void CPID::reset()
{
    sumE = lastE = lastRetVal = 0;
}

void CPID::resetI()
{
    sumE = 0;
}

void CPID::setParams(double _kp, double _ki, double _kd)
{
    kP = _kp;
    kI = _ki;
    kD = _kd;
}

double CPID::step(long double current)
{
	if (dt <= 0.04) dt = 0.04;
	double error = - current;
	double diff = 0.5 * diff + 0.5 * (error -lastE)/dt;
    sumE += (error + lastE)*dt;
    lastE = error;
	double retVal = (kP*error + kI*sumE + kD*diff);

	if (outputLimit>0)
	{
		if (retVal > outputLimit) retVal = outputLimit;
		else if (retVal < -outputLimit) retVal = -outputLimit;
	}
/*

    if(outputLimit>0 && fabs(retVal)>outputLimit)
    {
        retVal = outputLimit*sign(retVal);
	}*/

	if (sumLimit>0)
	{
		if (sumE > sumLimit) sumE = sumLimit;
		else if (sumE < -sumLimit) sumE = -sumLimit;
	}

/*    if(sumLimit>0 && fabs(sumE)>sumLimit)
    {
		sumE = sumLimit * sign(sumE);
	}*/
    lastRetVal = retVal;

    return retVal;

}
