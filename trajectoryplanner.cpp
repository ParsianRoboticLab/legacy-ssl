#include "trajectoryplanner.h"
#include "geom.h"
#include <cstdlib>
#include <cstdio>




#include <QDebug>

using namespace std;

double sq(double x) {return x*x;}

CTrajectoryPlanner::CTrajectoryPlanner() {
	diff = 0;
}

CTrajectoryPlanner::CTrajectoryPlanner(double _x, double _v1, double _v2, double _amax, double _dmax, double _vmax, double _dt, double _tDesired)
{
    init(_x,_v1,_v2,_amax,_dmax,_vmax,_dt,_tDesired);
}

void CTrajectoryPlanner::init(double _x, double _v1, double _v2, double _amax, double _dmax, double _vmax, double _dt, double _tDesired)
{
    InitVal(x);InitVal(v1);InitVal(v2);InitVal(vmax);InitVal(amax);InitVal(dmax);InitVal(vmax);InitVal(dt);InitVal(tDesired);
}

double CTrajectoryPlanner::plan(double _x, double _v1, double _v2, double _amax, double _dmax, double _vmax, double _dt, double &a, double &t, double _tDesired, bool fastEnd, bool end, bool newBots, bool _oneTouch, bool _slow, bool _NoPID)
{
    CTrajectoryPlanner p;
    p.setDistance(_x);
    p.setCurrentVelocity(_v1);
    p.setFinalVelocity(_v2);
    p.setAccMax(_amax);
    p.setDecMax(_dmax);
    p.setVelMax(_vmax);
    p.setTimeStep(_dt);
    p.setOneTouch( _oneTouch);
    p.setDesiredTime(_tDesired);
    p.setFastEnd(fastEnd);
    p.setEndPoint( end);
    p.setNoPid( _NoPID);
	p.plan();
    p.setNewRobots(newBots);
    p.setSlow(_slow);
    a = p.getAcceleration();
    t = p.getTimeNeeded();
	return p.getVelocity();

}

void CTrajectoryPlanner::plan()
{
//    qDebug() << v2;
    v = v1;
    double amax1;
    amax1 = amax;
//    if (v2 > 0.3)
//    {
//        if ( x < 0.3)
//        {
//            amax1 = 3.0*( v2*v2 - v1*v1) / 2.0*x;
//            dmax = 3.0*( v2*v2 - v1*v1) / 2.0*x;
//        }
//        if ( amax1 < 0 )
//        {
//            amax1 = fabs(amax1);
//            dmax = fabs(dmax);
//        }
//    }
    double t0,at,st;
    if(dmax!=0 && amax1*(amax1+dmax)!=0)
        t0 = (sqrt((amax1+dmax)*(2*amax1*x*dmax+amax1*v2*v2+dmax*v1*v1))-amax1*v1-dmax*v1)/(amax1*(amax1+dmax));
    else {v = 0;return;}
    if (x < 0)
        at = -t0;
    else
        at = t0;
    st = (amax1 * at + v - v2) / dmax;
    if (v < 0)
    {
        swap(amax1,dmax);
    }
    if (st * at < 0)
    {
        if ( st < 0)
            std::swap(at,st);
        at = -at;
        st += at;
        if (dt < at)
            v -= dt * amax1;
        else
            v -= at * amax1;
        if (dt > st)
            v += (dt - st) * dmax;
        t = -st+at;
    }
    else
    {
        float badat = at,badst = st;
        badat -= min (at,(vmax - v) / amax1);
        badst -= min (st,(vmax - v2) / dmax);
        float staticTime = ((((badat * amax1) / 2.0 + vmax) * badat) + (((badst * dmax) / 2.0) + vmax) * badst) / vmax;
        float time = at + st + staticTime - badat - badst;
        st = at;
        at -= badat;
        st += badst;
        if (dt < at)
            v += dt * amax1;
        else
            v += at * amax1;
        if (dt > st)
            v -= (dt - st) * dmax;
        t = time;
    }

//    if (x < 0.5)
//    {
//        v = sqrt(x * 2.0 * dmax);
//    }
//    return;
	if( v2 < 0.2 && x < 0.5 && !oneTouch && !noPid)
    {
        double DT = conf()->Common_Command_Interval()/1000.0;
        sumErr += (x + lx)/2.0f * DT;
		diff = 0.5*diff+0.5*((x - lx) / DT);
		double kp = conf()->BangBang_KP();
		double kd = conf()->BangBang_KD();
		double ki = conf()->BangBang_KI();
		double gain = conf()->BangBang_Gain();
		v = x * kp + sumErr * ki + diff * kd +gain * v;//+ gain*v1/(v1+1);
        lx = x;
    }
	else if( v2 < 0.2 && x < 0.5 && fastend && !noPid)
	{
		double DT = conf()->Common_Command_Interval()/1000.0;
		sumErr += (x + lx)/2.0f * DT;
		diff = 0.5*diff+0.5*((x - lx) / DT);
		double kp = 6;
		double kd = 0;
		double ki = 0;
		double gain = conf()->BangBang_Gain();
		v = x * kp + sumErr * ki + diff * kd +gain * v;//+ gain*v1/(v1+1);
		lx = x;
	}
    else if( v2 < 0.2 && x < 0.5 && oneTouch && !noPid)
    {
        double DT = conf()->Common_Command_Interval()/1000.0;
        sumErr += (x + lx)/2.0f * DT;
        double diff = (x - lx) / DT;
		double kp = conf()->BangBang_OneKP();
		double kd = conf()->BangBang_OneKD();
		double ki = conf()->BangBang_OneKI();
		double gain = conf()->BangBang_Gain();
        v = x * kp + sumErr * ki + diff * kd +gain * v;//+ gain*v1/(v1+1);
        lx = x;
    }
    else if( v2 < 0.2 && x < 0.5 && slow && !noPid)
    {
        double DT = conf()->Common_Command_Interval()/1000.0;
        sumErr += (x + lx)/2.0f * DT;
        double diff = (x - lx) / DT;
		double kp = conf()->BangBang_KP();
		double kd = conf()->BangBang_KD();
		double ki = conf()->BangBang_KI();
		double gain = conf()->BangBang_Gain();
        double kp1,ki1,kd1,g1;
        kp1 = kp * 0.6;
        ki1 = ki * 0.6;
        kd1 = kd * 0.6;
        g1 = gain * 0.6;
        v = x * kp1 + sumErr * ki1 + diff * kd1 +g1 * v;//+ gain*v1/(v1+1);
        lx = x;
    }
    else
        sumErr = 0.0;



}

void CTrajectoryPlanner::plan2()
{

	if( v2 < 0.2 && x < 0.3 && !oneTouch && !noPid)
	{
		double DT = conf()->Common_Command_Interval()/1000.0;
		sumErr += (x + lx)/2.0f * DT;
		diff = 0.5*diff+0.5*((x - lx) / DT);
		double kp = conf()->BangBang_KP();
		double kd = conf()->BangBang_KD();
		double ki = conf()->BangBang_KI();
		double gain = conf()->BangBang_Gain();
		v = x * kp + sumErr * ki + diff * kd +gain * v;//+ gain*v1/(v1+1);
		lx = x;
		return;
	}
	else if( v2 < 0.2 && x < 0.3 && fastend && !noPid)
	{
		double DT = conf()->Common_Command_Interval()/1000.0;
		sumErr += (x + lx)/2.0f * DT;
		diff = 0.5*diff+0.5*((x - lx) / DT);
		double kp = 6;
		double kd = 0;
		double ki = 0;
		double gain = conf()->BangBang_Gain();
		v = x * kp + sumErr * ki + diff * kd +gain * v;//+ gain*v1/(v1+1);
		lx = x;
		return;
	}
	else if( v2 < 0.2 && x < 0.3 && oneTouch && !noPid)
	{
		double DT = conf()->Common_Command_Interval()/1000.0;
		sumErr += (x + lx)/2.0f * DT;
		double diff = (x - lx) / DT;
		double kp = conf()->BangBang_OneKP();
		double kd = conf()->BangBang_OneKD();
		double ki = conf()->BangBang_OneKI();
		double gain = conf()->BangBang_Gain();
		v = x * kp + sumErr * ki + diff * kd +gain * v;//+ gain*v1/(v1+1);
		lx = x;
		return;
	}
	else if( v2 < 0.2 && x < 0.3 && slow && !noPid)
	{
		double DT = conf()->Common_Command_Interval()/1000.0;
		sumErr += (x + lx)/2.0f * DT;
		double diff = (x - lx) / DT;
		double kp = conf()->BangBang_KP();
		double kd = conf()->BangBang_KD();
		double ki = conf()->BangBang_KI();
		double gain = conf()->BangBang_Gain();
		double kp1,ki1,kd1,g1;
		kp1 = kp * 0.6;
		ki1 = ki * 0.6;
		kd1 = kd * 0.6;
		g1 = gain * 0.6;
		v = x * kp1 + sumErr * ki1 + diff * kd1 +g1 * v;//+ gain*v1/(v1+1);
		lx = x;
		return;
	}
	else
		sumErr = 0.0;



	//kalman reset check :

//	if ( fabs(lastV1 - v1) > 0.6)
//		v1 = lastV1;
	double tAcc = 0, tDec = 0, tCruise = 0;
	double xAcc = 0, xDec = 0, xCruise = 0;
	tAcc = fabs( vmax - v1) / amax;
	xAcc = tAcc * ( v1 + vmax)/2.0;
	tDec = fabs( v1 - v2) / dmax;
	xDec = tDec * ( v1 + v2)/2.0;
	double time_to_v1 = fabs(v1 - v2) / amax;
	double x_to_v1 = fabs((v1 + v2) / 2.0) * time_to_v1;
	x = x-lastV*0.2;
	double tempDist = x - ( xAcc + xDec);
//	debug(QString("tdec : %1 xdec : %2").arg(tDec).arg(xDec),D_SEPEHR,"blue");
	if ( tempDist >= 0){
		xCruise = tempDist;
		tCruise = tempDist / vmax;
	}
	else{
		xCruise = 0;
		tCruise = -1;
	}
	if( v1 < 0)
	{
		v = sign(v1)*(fabs(v1) - dt * dmax);
	}
	else
	{
		if( tCruise >= 0){
			if ( fabs(v1) < vmax)
				v = v1 + dt * amax;
			else
				v = vmax;
		}else if (xDec >= x){
//			debug("decing !",D_SEPEHR,"red");
			v = sign(v1)*(fabs(v1) - dt * dmax);
		}else{
//			debug("accing !",D_SEPEHR);
			v = lastV;//v1 + dt * amax;
		}/*
		else{
			double t_a, t_accel, t_decel;

			if (fabs(v1) > fabs(v2)) {
				t_a = (sqrt((v1 * v1 + v2 * v2) / 2.0 + fabs(x) * amax) - fabs(v1)) / amax;
				if (t_a < 0.0) t_a = 0;
				t_accel = t_a;
				t_decel = t_a + time_to_v1;
			} else if (x_to_v1 > fabs(x)) {
				t_a = (sqrt(v1 * v1 + 2 * amax * fabs(x)) - fabs(v1)) / amax;
				t_accel = t_a;
				t_decel = 0.0;
			} else {
				t_a = (sqrt((v1 * v1 + v2 * v2) / 2.0 + fabs(x) * amax) - fabs(v2)) / amax;
				if (t_a < 0.0) t_a = 0;
				t_accel = t_a + time_to_v1;
				t_decel = t_a;
			}
			if ( t_accel > 0)
				v = v1 + dt * amax;
			else
				v = sign(v1)*(fabs(v1) - dt * dmax);
		}*/
	}
//	if ( v1*v < 0 )
//		v = 0;

	lastV = v;
	lastV1 = v1;
	v = v > vmax ? vmax : v ;
	qDebug() << lastV << " " << v << "   " << lastV1 << " " << v1;
//	qDebug() << "TC : " << tCruise << "TD : " << tDec << " XD : " << xDec << " Dist : " << x;
//	debug(QString("TA: %1 TD: %2 TC:%3 V:%4 Xa:%5 Xd:%6 X:%7 XT:%8").arg(tAcc).arg(tDec).arg(tCruise).arg(v).arg(xAcc).arg(xDec).arg(x).arg(tempDist),D_SEPEHR);




}

double CTrajectoryPlanner::motionTime1d(float dx,float vel0,float vel1,
                                        float max_vel,float max_accel,
                                        float &t_accel,float &t_cruise,float &t_decel)
{
    float tmp,vmax;
    if(dx < 0){
        dx = -dx;
        vel0 = -vel0;
        vel1 = -vel1;
    }

    if(vel0 > max_vel) vel0 = max_vel;
    if(vel1 > max_vel) vel1 = max_vel;

    // stop
    if(vel0 > vel1){
        t_decel = (vel0 + vel1) / 2*dx;
        if(fabs(vel1 - vel0)/t_decel > max_accel){
            t_accel = t_cruise = 0;
            return(t_decel);
        }
    }

    // calculate time spent at max velocity
    tmp = 2*max_accel*dx + sq(vel0) + sq(vel1);
    t_cruise = (tmp - 2*sq(max_vel)) / (2*max_vel*max_accel);

    if(t_cruise > 0){
        vmax = max_vel;
    }else{
        vmax = sqrt((max_accel*dx + sq(vel0) + sq(vel1))/2);
        t_cruise = 0;
    }

    t_accel = max(vmax - vel0,(float)0.0) / max_accel;
    t_decel = fabs(vmax - vel1) / max_accel;

    return(t_accel + t_cruise + t_decel);
}

double CTrajectoryPlanner::max_speed(double dx,double max_a)
{
    return(sqrt(2.0*max_a*dx));
}

void CTrajectoryPlanner::computeMotion1d(double x0, double v0, double v1,
                                         double a_max, double v_max, double a_factor,
                                         double &traj_accel, double &traj_time,double FRAME_PERIOD)
{
    // First check to see if nothing needs to be done...
    if (x0 == 0 && v0 == v1) { traj_accel = 0; traj_time = 0;  return; }

    if(!finite(x0) || !finite(v0) || !finite(v1)){
		printf("Robot::compute_motion_1d: NANs!\n");
        traj_accel = 0; traj_time = 0;  return;
    }

    // Need to do some motion.
    a_max /= a_factor;

	double time_to_v1 = fabs(v0 - v1) / a_max;
    double x_to_v1 = fabs((v0 + v1) / 2.0) * time_to_v1;

    double period = 2.0 * FRAME_PERIOD;

    v1 = copysign(v1, -x0);

    if (v0 * x0 > 0 || (fabs(v0) > fabs(v1) && x_to_v1 > fabs(x0))) {
        // Time to reach goal after stopping + Time to stop.
        double time_to_stop = fabs(v0) / a_max;
		double x_to_stop = v0 * v0 / (2 * a_max);

        computeMotion1d(x0 + copysign(x_to_stop, v0), 0, v1, a_max * a_factor,
                        v_max, a_factor, traj_accel, traj_time, FRAME_PERIOD);
        traj_time += time_to_stop;

        // Decelerate
        if (traj_time < period)
            traj_accel = compute_stop(v0, a_max * a_factor, FRAME_PERIOD);
        else if (time_to_stop < period)
            traj_accel = time_to_stop / period * - copysign(a_max * a_factor, v0) +
                    (1.0 - time_to_stop / period) * traj_accel;
        else traj_accel = -copysign(a_max * a_factor, v0);

        return;
    }
    // At this point we have two options.  We can maximally accelerate
    // and then maximally decelerate to hit the target.  Or we could
    // find a single acceleration that would reach the target with zero
    // velocity.  The later is useful when we are close to the target
    // where the former is less stable.

    // OPTION 1
    // This computes the amount of time to accelerate before decelerating.
    double t_a, t_accel, t_decel;

    if (fabs(v0) > fabs(v1)) {
        t_a = (sqrt((v0 * v0 + v1 * v1) / 2.0 + fabs(x0) * a_max)
               - fabs(v0)) / a_max;
        if (t_a < 0.0) t_a = 0;
        t_accel = t_a;
        t_decel = t_a + time_to_v1;
    } else if (x_to_v1 > fabs(x0)) {
        t_a = (sqrt(v0 * v0 + 2 * a_max * fabs(x0)) - fabs(v0)) / a_max;
        t_accel = t_a;
        t_decel = 0.0;
    } else {
        t_a = (sqrt((v0 * v0 + v1 * v1) / 2.0 + fabs(x0) * a_max)
               - fabs(v1)) / a_max;
        if (t_a < 0.0) t_a = 0;
        t_accel = t_a + time_to_v1;
        t_decel = t_a;
    }

    // OPTION 2
    double a_to_v1_at_x0 = (v0 * v0 - v1 * v1) / (2 * fabs(x0));
    double t_to_v1_at_x0 =
            (-fabs(v0) + sqrt(v0 * v0 + 2 * fabs(a_to_v1_at_x0) * fabs(x0))) /
            fabs(a_to_v1_at_x0);

    // We follow OPTION 2 if t_a is less than a FRAME_PERIOD making it
    // difficult to transition to decelerating and stopping exactly.
    if (0 && a_to_v1_at_x0 < a_max && a_to_v1_at_x0 > 0.0 &&
            t_to_v1_at_x0 < 2.0 * FRAME_PERIOD && 0) {

        // OPTION 2
        // Use option 1 time, even though we're not following it.
        traj_time = t_accel + t_decel;;

        // Target acceleration to stop at x0.
        traj_accel = -copysign(a_to_v1_at_x0, v0);

        return;
    } else {

        // OPTION 1
        // Time to accelerate and decelerate.
        traj_time = t_accel + t_decel;

        // If the acceleration time would get the speed above v_max, then
        //  we need to add time to account for cruising at max speed.
        if (t_accel * a_max + fabs(v0) > v_max) {
            traj_time +=
                    pow(v_max - (a_max * t_accel + fabs(v0)), 2.0) / a_max / v_max;
        }

        // Accelerate (unless t_accel is less than FRAME_PERIOD, then set
        // acceleration to average acceleration over the period.)
        if (t_accel < period && t_decel == 0.0) {
            traj_accel = copysign(a_max * a_factor, -x0);
        } else if (t_accel < period && t_decel > 0.0) {
            traj_accel = compute_stop(v0, a_max * a_factor, FRAME_PERIOD);
        } else if (t_accel < period) {
            traj_accel = copysign((2.0 * t_accel / (period) - 1) * a_max * a_factor, v0);
        } else {
            traj_accel = copysign(a_max * a_factor, -x0);
        }
    }
}

void CTrajectoryPlanner::computeMotion2d(Vector2D x0, Vector2D v0, Vector2D v1, double a_max, double v_max, double a_factor, Vector2D &traj_accel, double &time, double FRAME_PERIOD)
{
    double time_x, time_y;
    double rotangle;

	if (v1.length() > 0.0) rotangle = v1.th().degree();
	else rotangle = 0.0;
//	rotangle = agentDir.dir().degree();

//    x0 = x0.rotate(-rotangle);
//    v0 = v0.rotate(-rotangle);
//    v1 = v1.rotate(-rotangle);

    computeMotion1d(x0.x, v0.x, v1.x, a_max, v_max, a_factor,
                    traj_accel.x, time_x,FRAME_PERIOD);
    computeMotion1d(x0.y, v0.y, v1.y, a_max, v_max, a_factor,
                    traj_accel.y, time_y,FRAME_PERIOD);

    if (v1.length() == 0.0) {
        double rx = time_x / hypot(time_x, time_y);
        double ry = time_y / hypot(time_x, time_y);

        computeMotion1d(x0.x, v0.x, v1.x, a_max * rx, v_max * rx, a_factor,
                        traj_accel.x, time_x, FRAME_PERIOD);
        computeMotion1d(x0.y, v0.y, v1.y, a_max * ry, v_max * ry, a_factor,
                        traj_accel.y, time_y, FRAME_PERIOD);
    } else {
        if (time_x < time_y * 1.5) {
            double rx_a = pow(time_x / (time_y * 1.5), 2.0);
            double rx_v = time_x / (time_y * 1.5);

            computeMotion1d(x0.x, v0.x, v1.x, a_max * rx_a, v_max * rx_v, a_factor,
                            traj_accel.x, time_x, FRAME_PERIOD);
        }
    }

    traj_accel = traj_accel.rotate(rotangle);

    time = max(time_x, time_y);
}

double CTrajectoryPlanner::compute_stop(double v, double max_a, double FRAME_PERIOD)
{
    if (fabs(v) > max_a * FRAME_PERIOD) return copysign(max_a, -v);
    else return -v / FRAME_PERIOD;
}

void CTrajectoryPlanner::trajectory1D(double deltaS, double startVel, double finalVel, double framerate, double accMax, double velMax, double &trajAcc, double &trajTime)
{
//    double timeToFinalVel, distanceToFinalVel, timeTemp, timeAcc, timeDec;
//    if(deltaS == 0 && startVel == finalVel)
//    {
//        trajAcc = 0;
//        trajTime = 0;
//        return;
//    }
//    timeToFinalVel = fabs(startVel - finalVel) / accMax;
//    distanceToFinalVel = ( fabs( finalVel + startVel) / 2.0) * timeToFinalVel;
//    if(startVel<0)
//    {
//        trajAcc = -accMax;
//        trajTime = timeToFinalVel-startVel/accMax;
//        return;
//    }
//    if( fabs(startVel) > fabs(velMax) )
//    {
//        timeTemp = (sqrt((startVel*startVel + velMax*velMax) / 2.0 + fabs(deltaS) * accMax) - fabs(startVel)) / accMax;
//        if (timeTemp < 0.0)
//            timeTemp = 0.0;
//        timeAcc = 0;
//        timeDec = timeTemp + timeToFinalVel;
//    }
//    else if( fabs(startVel) > fabs(finalVel) )
//    {
//        timeTemp = (sqrt((startVel*startVel + finalVel*finalVel) / 2.0 + fabs(deltaS) * accMax) - fabs(startVel)) / accMax;
//        if (timeTemp < 0.0)
//            timeTemp = 0.0;
//        timeAcc = timeTemp;
//        timeDec = timeTemp + timeToFinalVel;
//    }
//    else if( distanceToFinalVel > fabs(deltaS))
//    {
//        timeTemp = (sqrt( startVel*startVel + 2 * accMax * fabs(deltaS)) - fabs(startVel)) / accMax;
//        timeAcc = timeTemp;
//        timeDec = 0.0;
//    }
//    else
//    {
//        timeTemp = (sqrt(( startVel*startVel + finalVel*finalVel) / 2.0 + fabs(deltaS) * accMax) - fabs(finalVel)) / accMax;
//        if (timeTemp < 0.0)
//            timeTemp = 0;
//        timeAcc = timeTemp + timeToFinalVel;
//        timeDec = timeTemp;
//    }
//    trajTime = timeAcc + timeDec;
//    if ( timeAcc * accMax + fabs(startVel) > velMax)
//        trajTime += (velMax - (accMax * timeAcc + fabs(startVel))) * (velMax - (accMax * timeAcc + fabs(startVel))) / (accMax * velMax);
//    if ( timeAcc < 1.0/framerate && timeDec == 0)
//        trajAcc = ( finalVel - startVel)/framerate;
//    else if( timeAcc < (1.0/framerate) && timeDec > 0)
//        trajAcc = (accMax * timeAcc) + (-accMax * ((1.0/framerate) - timeAcc));
//    else
//    {
//        trajAcc = accMax;
//    }
//    return;



    double decMax = 100.0*accMax;
    double vC=sqrt((2*accMax*decMax*deltaS + startVel*startVel*decMax + finalVel*finalVel*accMax) / (decMax + accMax));
    long double T;
    long double Tr,Tf,Tc;
    T=Tr=Tf=Tc = 0;
    if (vC>velMax)
    {
        vC=velMax;
        Tc = (deltaS-( (vC*vC-startVel*startVel)/(2*accMax) + (vC*vC-finalVel*finalVel)/(2*decMax) ))/vC;
    }
    else
    {
        Tc = 0;
    }
    Tr = (vC-startVel)/accMax;
    if(Tr<0)
        Tr=0;
    Tf = (vC-finalVel)/decMax;
    if(Tf<0)
        Tf=0;

    T = Tr + Tc + Tf;

    trajTime=T;

    double v=startVel;

//    if(fabs(startVel)>fabs(velMax))
//    {
//        trajAcc = -0.2*/*sign(startVel)**/accMax;//fabs(vC-startVel)*0.2*framerate;
//        return;
//    }


        if (startVel<-(velMax))//(v<0)
        {
            trajAcc = accMax;
        }else if((startVel*startVel - finalVel*finalVel)/(2*decMax) >= deltaS)//((v0*v0 - ve*ve)/(2*aDec) >= d)
        {
            trajAcc = -decMax;
        }else if( (startVel<vC) && ( ((0.5*accMax/framerate + startVel)/framerate) < deltaS) )//(v<vC)
        {
            trajAcc = accMax;
        }else
        {
    //            v = 0;
        }
        if((startVel/framerate>deltaS))
        {
    //            v=d/dt;
        }


//    if(Tr > 1/framerate)
//    {
////        v += dt * accMax;
//        trajAcc = accMax;
//    }else if (Tc > 1/framerate)
//    {
////        v = v + (vC-v)*0.2;
////        trajAcc = (VC-v)/;
//        trajAcc=(vC-v)*0.2*framerate;
//    }else if (Tf > 2/framerate)
//    {
////        v -= dt * accMax;
//        trajAcc = -decMax;
//    }



//        qDebug()<<"desired V:"<<v<<" Decision:"<<Decsision<<" dt:"<<dt<<" vC:"<<vC<<" finalVel:"<<finalVel;
    return;

//    double dmax,amax;
//    dmax=amax=accMax;
//    double v1=startVel;
//    double v2=finalVel;
//    double amax1;
//    amax1 = amax;
//    double v=v1;
//    double t0,at,st;
//    if(dmax!=0 && amax1*(amax1+dmax)!=0)
//        t0 = (sqrt((amax1+dmax)*(2*amax1*x*dmax+amax1*v2*v2+dmax*v1*v1))-amax1*v1-dmax*v1)/(amax1*(amax1+dmax));
//    else {v = 0;return;}
//    if (x < 0)
//        at = -t0;
//    else
//        at = t0;
//    st = (amax1 * at + v - v2) / dmax;
//    if (v < 0)
//    {
//        swap(amax1,dmax);
//    }
//    if (st * at < 0)
//    {
//        if ( st < 0)
//            std::swap(at,st);
//        at = -at;
//        st += at;
//        if (dt < at)
//            v -= dt * amax1;
//        else
//            v -= at * amax1;
//        if (dt > st)
//            v += (dt - st) * dmax;
//        t = -st+at;
//    }
//    else
//    {
//        float badat = at,badst = st;
//        badat -= min (at,(vmax - v) / amax1);
//        badst -= min (st,(vmax - v2) / dmax);
//        float staticTime = ((((badat * amax1) / 2.0 + vmax) * badat) + (((badst * dmax) / 2.0) + vmax) * badst) / vmax;
//        float time = at + st + staticTime - badat - badst;
//        st = at;
//        at -= badat;
//        st += badst;
//        if (dt < at)
//            v += dt * amax1;
//        else
//            v += at * amax1;
//        if (dt > st)
//            v -= (dt - st) * dmax;
//        t = time;
//    }
//    trajTime = t;
//    trajAcc = (v1-v)*framerate;
}

void CTrajectoryPlanner::trajectory2D(Vector2D startPos, Vector2D startVel, Vector2D finalPos, Vector2D finalVel, double startAngle, double startAngleVel, double finalAngle, double finalAngleVel, double framerate, double maxAcc, double maxVel, double maxAngAcc, double maxAngVel, Vector2D &desiredVel, double &desiredW, double &t)
{
//    qDebug() << " Agent Pos : " << startPos.x << startPos.y << startPos.dir().degree();
//    qDebug() << " Final Pos : " << finalPos.x << finalPos.y << finalPos.dir().degree();
//    qDebug()<<"Agent Vel:" << startVel.x << startVel.y;
    //    double rotangle;
    //    if (finalVel.length() > 0.0) rotangle = finalVel.th().degree();
    //    else rotangle = 0.0;    
//    startPos = startPos.rotate(-rotangle);

//    finalPos = (finalPos-startPos).rotatedVector(startAngle) + startPos;
//    startVel =  startVel.rotatedVector(startAngle);
//    finalVel =  finalVel.rotatedVector(startAngle);

    //    startVel = startVel.rotate(-rotangle);
//    finalVel = finalVel.rotate(-rotangle);


    //    startPos = startPos.rotate(-rotangle);
    //    startVel = startVel.rotate(-rotangle);
    //    finalVel = finalVel.rotate(-rotangle);


//    double u = _PI / 2.0;
//    double du = -_PI / 2.0;
//    double alpha , axMax, ayMax, vxMax, vyMax, deltaX, deltaY, xAcc , tx, yAcc, ty, trajTime, deltaAng, angAcc, tAng;
//    for( int i = 0; i < 11; i++)
//    {
//        alpha = u + (du *= 0.5);
//        axMax = sin(alpha)*maxAcc;
//        ayMax = cos(alpha)*maxAcc;
//        vxMax = sin(alpha)*maxVel;
//        vyMax = cos(alpha)*maxVel;
//        deltaX = finalPos.x - startPos.x;
//        deltaY = finalPos.y - startPos.y;
//        trajectory1D( fabs(deltaX), sign(deltaX)*startVel.x, sign(deltaX)*finalVel.x, framerate, axMax, vxMax, xAcc, tx);
//        trajectory1D( fabs(deltaY), sign(deltaX)*startVel.y, sign(deltaX)*finalVel.y, framerate, ayMax, vyMax, yAcc, ty);
////        trajectory1D( deltaX, startVel.x, finalVel.x, framerate, axMax, vxMax, xAcc, tx);
////        trajectory1D( deltaY, startVel.y, finalVel.y, framerate, ayMax, vyMax, yAcc, ty);
//        if( tx - ty <= 0.0)
//            u = alpha;
//    }
//    debug(QString("tx=%1,ty=%2,aplha=%3").arg(tx).arg(ty).arg(alpha),D_ARASH);
//    if ( tx > ty)
//        trajTime = tx;
//    else
//        trajTime = ty;
//    deltaAng = AngleDeg::normalize_angle(finalAngle - startAngle);
//    double angSign = sign(deltaAng);
//    deltaAng = fabs(deltaAng);
//    for ( double factor = 0.1; factor < 1; factor+= 0.05)
//    {
//        trajectory1D( deltaAng, startAngleVel*angSign, finalAngleVel*angSign, framerate, maxAngAcc*factor, maxAngVel*factor, angAcc, tAng);
//        if ( tAng < trajTime)
//            break;
//    }

//    qDebug()<<xAcc<<yAcc;

////    Vector2D acc(xAcc*sign(deltaX), yAcc*sign(deltaY));
//    Vector2D acc(xAcc, yAcc);
////    startVel =  startVel.rotatedVector(-startAngle);
////    acc = acc.rotatedVector(-startAngle);

//    desiredVel.x = startVel.x + /*sign(deltaX)**/acc.x/framerate;
//    desiredVel.y = startVel.y + /*sign(deltaY)**/acc.y/framerate;
//    desiredW = startAngleVel + angAcc*angSign/framerate;
//    //desiredW = 10*angSign;
//    t = trajTime;


    double rx=finalPos.x - startPos.x;
    double ry=finalPos.y - startPos.y;
    double v0x=startVel.x;
    double v0y=startVel.y;
    double vex=finalVel.x;
    double vey=finalVel.y;
    double vMax=maxVel;
    double aMax=maxAcc;
    double durationx,durationy;
    double alpha = 0;
    double UBound = M_PI / 2.0, LBound = 0;
    if(rx!=0)
        alpha = atan(fabs(ry/rx));
    else
        alpha = M_PI / 2.0;
    int it;
    double durationDiff=0;
    durationx=10;
    durationy=-10;
    double xAcc,yAcc;

    for(it=0;it<20 && (fabs(durationx - durationy)>0.001);it++)
    {
        trajectory1D(fabs(rx), v0x*sign(rx), vex*sign(rx) , framerate, cos(alpha) * aMax, cos(alpha) * vMax, xAcc, durationx);
        trajectory1D(fabs(ry), v0y*sign(ry), vey*sign(ry) , framerate, sin(alpha) * aMax, sin(alpha) * vMax, yAcc, durationy);

        if(durationx > durationy)
        {
            UBound = alpha;
        }
        if(durationy > durationx)
        {
            LBound = alpha;
        }

        if(fabs(durationDiff) - fabs(durationx - durationy)>0)
        {
            qDebug()<<"bad Search!!";
//            emit stopSim();
        }
        alpha = (UBound + LBound) / 2.0;

        durationDiff = durationx - durationy;
    }
    if(it==40){
        qDebug()<<"not good!";
    }
//    qDebug()<<"durationx"<<durationx<<" durationy"<<durationy;

    //qDebug()<<"(x,y), ,r.(x,y)="<<vx<<","<<vy<<" v.(x,y)="<<vx*sign(rx)<<","<<vy*sign(ry);

//    desiredVel =  CTrajectoryPlanner::plan(fabs(r.x), v.x*sign(r.x), getFinalVel().x*sign(r.x) , cos(alpha) * aMax, cos(alpha) * dMax, cos(alpha) * velMax, dt, acc, duration1) * Vector2D(1,0)*sign(r.x) +
//                  CTrajectoryPlanner::plan(fabs(r.y), v.y*sign(r.y), getFinalVel().y*sign(r.y) , sin(alpha) * aMax, sin(alpha) * dMax, sin(alpha) * velMax, dt, acc, duration2) * Vector2D(0,1)*sign(r.y);
    trajectory1D(fabs(rx), v0x*sign(rx), vex*sign(rx) , framerate, cos(alpha) * aMax, cos(alpha) * vMax, xAcc, durationx);
    trajectory1D(fabs(ry), v0y*sign(ry), vey*sign(ry) , framerate, sin(alpha) * aMax, sin(alpha) * vMax, yAcc, durationy);
    Vector2D acc(xAcc*sign(rx),yAcc*sign(ry));
    qDebug() << startVel.x << acc.x << startVel.y << acc.y;
    desiredVel=startVel + 1.0/framerate*acc;
    t = (durationx + durationy) / 2.0;
}
