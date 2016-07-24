#ifndef TRAJECTORYPLANNER_H
#define TRAJECTORYPLANNER_H

#include <base.h>
#include "logger.h"
#include "varswidget.h"

class CTrajectoryPlanner
{
public:
    CTrajectoryPlanner();
    CTrajectoryPlanner(double _x, double _v1, double _v2, double _amax, double _dmax, double _vmax, double _dt, double _tDesired=-1);
    void init(double _x, double _v1, double _v2, double _amax, double _dmax, double _vmax, double _dt, double _tDesired=-1);
    void plan();
	void plan2();
    static double plan(double _x, double _v1, double _v2, double _amax, double _dmax, double _vmax, double _dt, double &a, double &t, double _tDesired=-1, bool fastEnd=false, bool end = true, bool newGenerationRobots = true, bool _oneTouch = false, bool _slow = false, bool _NoPID = false);
    static double motionTime1d(float dx,float vel0,float vel1,
                                float max_vel,float max_accel,
                                float &t_accel,float &t_cruise,float &t_decel);

    static double max_speed(double dx,double max_a);
    static void computeMotion1d(double x0, double v0, double v1,
                                  double a_max, double v_max, double a_factor,
                                  double &traj_accel, double &traj_time,double FRAME_PERIOD);
    static void computeMotion2d(Vector2D x0, Vector2D v0, Vector2D v1,
                                  double a_max, double v_max, double a_factor,
								  Vector2D &traj_accel, double &time,double FRAME_PERIOD);
    static double compute_stop(double v, double max_a, double FRAME_PERIOD);

    void trajectory1D( double deltaS, double startVel, double finalVel, double framerate, double accMax, double velMax, double &trajAcc, double &trajTime);
    void trajectory2D(Vector2D startPos, Vector2D startVel, Vector2D finalPos, Vector2D finalVel, double startAngle, double startAngleVel, double finalAngle, double finalAngleVel, double framerate, double maxAcc, double maxVel, double maxAngAcc, double maxAngVel, Vector2D &desiredVel, double &desiredW, double &t);

	double lastV;
	double lastV1;
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

#endif // TRAJECTORYPLANNER_H
