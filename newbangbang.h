#ifndef NEWBANGBANG_H
#define NEWBANGBANG_H

#include <base.h>
#include "logger.h"
#include "varswidget.h"


class CNewBangBang
{
public:
    CNewBangBang();
	double plan(Vector2D _dir , double _x, double _v1, double _v2, double _amax, double _dmax, double _vmax, double _dt, double &a, double &t, int id,double _tDesired=-1, bool fastEnd=false, bool end = true, bool newGenerationRobots = true, bool _oneTouch = false, bool _slow = false, bool _NoPID = false);

	QList<double> lastV;
	QList<double> lastV1;

	double diff;

	Property(double, Distance, x);
	Property(double, LastDistance, lx);
	Property(double, DistanceError, sumErr);
	Property(double, CurrentVelocity, v1);
	Property(double, FinalVelocity, v2);
	Property(double, AccMax, amax);
	Property(double, DecMax, dmax);
	Property(double, VelMax, vmax);
	Property(double, DesiredTime, tDesired);
	Property(double, TimeStep, dt);
	Property(bool, FastEnd, fastend);
	Property(bool, EndPoint, endPoint);
	Property(bool, OneTouch, oneTouch);
	Property(bool, Slow, slow);
	Property(bool, NoPid, noPid);
	Property(bool, NewRobots, newRobots);
	PropertyGet(double, Velocity, v);
	PropertyGet(double, Acceleration, a);
	PropertyGet(double, TimeNeeded, t);
};

#endif // NEWBANGBANG_H
