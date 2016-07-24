#include "newbangbang.h"
#include <QDebug>

#ifdef Q_OS_MAC
using namespace std;
#endif

CNewBangBang::CNewBangBang()
{
	lastV.clear();
	lastV1.clear();
	for( int i = 0; i < _MAX_NUM_PLAYERS; i++)
	{
		lastV.append(0.f);
		lastV1.append(0.f);
	}
}

double CNewBangBang::plan(Vector2D _dir , double _x, double _v1, double _v2, double _amax, double _dmax, double _vmax, double _dt, double &a, double &t, int id, double _tDesired, bool fastEnd, bool end, bool newBots, bool _oneTouch, bool _slow, bool _NoPID)
{
	x = _x;
	v1 = _v1;
	v2 = _v2;
	amax = _amax;
	dmax = _dmax;
	vmax = _vmax;
	dt = _dt;
	oneTouch = _oneTouch;
	tDesired = _tDesired;
	fastend = fastEnd;
	endPoint = end;
	noPid = _NoPID;
	newRobots = newBots;
	slow = _slow;

#if 0
	//////////////////////////// Alireza Modifications ////////////////////////////////

		double phi = 30*_DEG2RAD;
		double tetha = 45*_DEG2RAD;
		double alpha = 0;

		if( fabs(_dir.x) > 1e-9 )
			alpha = (atan(fabs(_dir.y/_dir.x)));
		else
			alpha = 0;

		double a_x = 2*(cos(phi)+cos(tetha));
		double a_y = 2*(sin(phi)+sin(tetha));
		double a_max = (a_x*cos(alpha)+a_y*sin(alpha))/3.966*conf()->BangBang_AccTangent_Max();
		double d_max = (a_x*cos(alpha)+a_y*sin(alpha))/3.966*conf()->BangBang_DecTangent_Max();
		amax = a_max;
		dmax = -d_max;

		//////////////////////////////////////////////////////////////////////////////////

		/////////////////////////////Modified Bang Bang///////////////////////////

		static double ttt = 0.0;
		double DT = conf()->Common_Command_Interval()/1000.0;
		double ta = (vmax - v1)/amax;
		double td = (v2 - vmax)/dmax;
		double ts = (x-(0.5*amax*ta*ta+fabs(v1)*ta)-(0.5*dmax*td*td+fabs(vmax)*td))/fabs(vmax);
		if(x<0.1)
			ttt=0;
		ttt += DT;
		if(ts >= 0)
		{
			if(ta > ttt)
				 v = sign(v2-v1)*(amax*ttt+v1);
			else if(ta < ttt && ttt < ta+ts)
				v =  sign(v2-v1)*vmax;
			else if(ta+ts < ttt)
				v = sign(v2-v1)*(dmax*(ttt-(ta+ts))+vmax);
		}
		else
		{
				v = 0.0;
		}

		qDebug() << "X DT " << x << " " << ttt << ta << " " << td << " " << ts;

		return v;

		/////////////////////////////////////////////////////////////////////////
#endif
	double vController = 0;

	if( v2 < 0.2 && /*x < 0.5 && */!oneTouch && !noPid)
	{
//		debug(QString("%1 NBB 1").arg(id),D_SEPEHR);
		double DT = conf()->Common_Command_Interval()/1000.0;
                if( isnan(sumErr))
			sumErr = (x + lx)/2.0f * DT;
		else
			sumErr += (x + lx)/2.0f * DT;
		if ( isnan(diff))
			diff = ((x - lx) / DT);
		else
			diff = 0.5*diff+0.5*((x - lx) / DT);
		double kp = conf()->BangBang_KP();
		double kd = conf()->BangBang_KD();
		double ki = conf()->BangBang_KI();
		double gain = conf()->BangBang_Gain();
		v = x * kp + sumErr * ki + diff * kd +gain * v;//+ gain*v1/(v1+1);
		lx = x;
//		return getVelocity();
		vController = getVelocity();
	}
	else if( v2 < 0.2 /*&& x < 0.5 */&& fastend && !noPid)
	{
//		debug(QString("%1 NBB 2").arg(id),D_SEPEHR);
		double DT = conf()->Common_Command_Interval()/1000.0;
		if( isnan(sumErr))
			sumErr = (x + lx)/2.0f * DT;
		else
			sumErr += (x + lx)/2.0f * DT;
		if ( isnan(diff))
			diff = ((x - lx) / DT);
		else
			diff = 0.5*diff+0.5*((x - lx) / DT);
		double kp = 6;
		double kd = 0;
		double ki = 0;
		double gain = conf()->BangBang_Gain();
		v = x * kp + sumErr * ki + diff * kd +gain * v;//+ gain*v1/(v1+1);
		lx = x;
//		return getVelocity();
		vController = getVelocity();
	}
	else if( v2 < 0.2 /*&& x < 0.5 */&& oneTouch && !noPid)
	{
//		debug(QString("%1 NBB 3").arg(id),D_SEPEHR);
		double DT = conf()->Common_Command_Interval()/1000.0;
		if( isnan(sumErr))
			sumErr = (x + lx)/2.0f * DT;
		else
			sumErr += (x + lx)/2.0f * DT;
		double diff = (x - lx) / DT;
		double kp = conf()->BangBang_OneKP();
		double kd = conf()->BangBang_OneKD();
		double ki = conf()->BangBang_OneKI();
		double gain = conf()->BangBang_Gain();
		v = x * kp + sumErr * ki + diff * kd +gain * v;//+ gain*v1/(v1+1);
		lx = x;
//		return getVelocity();
		vController = getVelocity();
	}
	else if( v2 < 0.2 && /*x < 0.5 && */slow && !noPid)
	{
//		debug(QString("%1 NBB 4").arg(id),D_SEPEHR);
		double DT = conf()->Common_Command_Interval()/1000.0;
		if( isnan(sumErr))
			sumErr = (x + lx)/2.0f * DT;
		else
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
//		return getVelocity();
		vController = getVelocity();
	}
	else
		sumErr = 0.0;

	//kalman reset check :

//	if ( fabs(lastV1 - v1) > 0.6)
//		v1 = lastV1;

//	if(v1 < 0.3)
//		amax/=2.f;
//	if ( sign(v2*v1) < 0)
//	{
//		x +=( v2*v2 / amax );
//	}
	double tAcc = 0, tDec = 0, tCruise = 0;
	double xAcc = 0, xDec = 0, xCruise = 0;



	if( v1 + 0.1< 0 )
	{
//		debug(QString("%1 NBB V=0").arg(id),D_SEPEHR);
		v = 0;
//		v = sign(v1)*(fabs(v1) - 1.5*dt * dmax);
	}
	else
	{
		if( vmax > v1 )
			tAcc = fabs( vmax - v1) / amax;
		else
			tAcc = 0;
		xAcc = tAcc * ( v1 + vmax)/2.0;

		if( vmax > v2 )
			tDec = fabs( vmax - v2) / dmax;
		else
			tDec = 0;
		xDec = tDec * ( vmax + v2)/2.0;

		x = x-lastV[id]*0.16;
		double tempDist = x - ( xAcc + xDec);
		if ( tempDist >= 0){
			xCruise = tempDist;
			tCruise = tempDist / vmax;
		}
		else{
			xCruise = 0;
			tCruise = -1;
		}

		if( tCruise >= 0){
//			debug(QString("%1 NBB Cruise").arg(id),D_SEPEHR);
//			t = tCruise+tAcc+tDec;
//			qDebug() << t;
			if ( fabs(v1) < vmax){
				v = v1 + dt * amax;
			}
			else{
				v = vmax;
			}
		}else{
			if( v1 > v2 )
				tDec = fabs( v1 - v2) / dmax;
			else
				tDec = 0;
			xDec = tDec * ( v1 + v2)/2.0;

			if(xDec >= x){
				v = sign(v1)*(fabs(v1) - dt * dmax);
			}else{
				double vDes , vLo=v1 , vHi=vmax;
				int cnt = 100;
				while( vLo < vHi && cnt>0 ){
					cnt--;
					vDes = (vLo+vHi)/2.0;

					tAcc = fabs( vDes - v1) / amax;
					xAcc = tAcc * ( v1 + vDes)/2.0;

					if( vDes > v2 )
						tDec = fabs( vDes - v2) / dmax;
					else
						tDec = 0;
					xDec = tDec * ( vDes + v2)/2.0;

					if( fabs((xAcc + xDec)-x) < EPSILON  ){
						v = v1 + dt * amax;
						v = min(vDes , v);
						break;
					}
					else if( xAcc+xDec < x ){
						vLo = vDes;
					}
					else
						vHi = vDes;
				}
			}
		}
	}

	lastV[id] = v;
	lastV1[id] = v1;
	v = v > vmax ? vmax : v ;

	if( x < 0.5){
		if( x > 0.05)
		{
//			debug(QString("%1 NBB vState = %2").arg(id).arg((( x - 0.05)/0.45)),D_SEPEHR);
			return ((( x - 0.05)/0.45) * v + (1-(( x - 0.05)/0.45))*vController);
		}
		else
			return vController;
	}

	return getVelocity();

}
