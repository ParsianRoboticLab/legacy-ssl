#ifndef PID_H
#define PID_H
#include "base.h"

class CPID
{
public:
    CPID();
    CPID(double _kp,double _ki,double _kd);
    void reset();
    void resetI();
    void setParams(double _kp,double _ki,double _kd);
    double step(long double current);
    long double sumE, lastE;
    double lastRetVal;
    double kP,kI,kD;
    Property(long double,Target,target);
    Property(double,DT,dt);
    Property(long double,SumLimit,sumLimit);
    Property(long double,OutputLimit,outputLimit);
};



#endif // PID_H
