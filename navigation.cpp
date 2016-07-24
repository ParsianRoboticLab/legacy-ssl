#include "navigation.h"
#include "knowledge.h"

double t0;

Navigation::Navigation()
{
    pathGenerated = -1;
    currentPos = -1;
    frame = 0;
    pathError = 0.0;
    t0 = CProfiler::getTime();
}

void Navigation::setAgent(CAgent *_agent)
{
    agent = _agent;
}

void Navigation::setTarget(Vector2D pos, Vector2D dir)
{
    if (((pos-targetPos).length() > 0.01) || (fabs((Vector2D::angleBetween(dir, targetDir).degree()) > 5)))
    {
        pathGenerated = -1;
    }
    targetPos = pos;
    targetDir = dir;
}

ObjectPosVel objectPosVel(Vector2D pos, Vector2D dir, Vector2D vel, float rotVel)
{
    ObjectPosVel p;
    p.xPos = pos.x;
    p.yPos = pos.y;
    p.xVel = vel.x;
    p.yVel = vel.y;
    p.rotation = dir.th().degree();
    p.rotVel = rotVel;
    return p;
}

double Navigation::getTime()
{
    ObjectPosVel start;
    ObjectPosVel final;
    start.xPos = agent->pos().x;
    start.yPos = agent->pos().y;
    start.xVel = agent->vel().x;
    start.yVel = agent->vel().y;
    start.rotation = 0;
    start.rotVel = 0;

    final.xPos = targetPos.x;
    final.yPos = targetPos.y;
    final.xVel = 0;
    final.yVel = 0;
    final.rotation = 0;
    final.rotVel = 0;
    float pathTime ;
    expectedPathTime(start, final, 1000.0/conf()->Common_Command_Interval(), 5, 4.5, pathTime);
    return pathTime;
}

void Navigation::run()
{
    double delayTime = 0.05;
//    double dt = 0.72 / (knowledge->commandFrameRate);
    //double dt = 1.0 / (knowledge->commandFrameRate);
    double dt = 0.019; //1.0 / (50.0);
    qDebug() << "time= " << CProfiler::getTime() - t0 << " ; dt=" << dt;
    t0 = CProfiler::getTime();
    bool isNear = (agent->pos() - targetPos).length() < 0.7;
    double ew = -Vector2D::angleBetween(targetDir, agent->dir()).degree()* M_PI / 180.0;
    if (isNear)
    {
        double ex = -agent->pos().x + targetPos.x;
        double ey = -agent->pos().y + targetPos.y;
        double d  = hypot(ex, ey);
        double dmax = 2;
        double v = sqrt(2.0*dmax*d);
//        agent->setRobotAbsVel(v * ex / d, v * ey / d, ew);
        agent->setRobotAbsVel(3*ex, 3*ey, ew);
       // agent->setMotorsVelControled(3*ex, 3*ey, ew);
    }
    else {
        if (pathGenerated < 0 || pathError > 1.0 || currentPos <0 || currentPos >= path.getNumSteps())
        {
           // ObjectPosVel start = objectPosVel(agent->pos(), agent->dir(), agent->vel(), agent->self()->angularVel);
           // ObjectPosVel final = objectPosVel(targetPos, targetDir, Vector2D(0.0, 0.0), 0.0);
            ObjectPosVel start = objectPosVel(agent->pos(), Vector2D(1.0, 0.0), agent->vel(), 0.0);
    //        ObjectPosVel start = objectPosVel(agent->pos(), Vector2D(1.0, 0.0), Vector2D(0.0, 0.0), 0.0);
             ObjectPosVel final = objectPosVel(targetPos, Vector2D(1.0, 0.0), Vector2D(0.0, 0.0), 0.0);

            if (isNear)
            {
                proportionalGainPath(
                  start,
                  final,
                  1.0 / dt, //1000.0/conf()->Common_Command_Interval(),
                  4.0,
                  1.0,
                  3.0,
                  180.0,
                  path
                );
            }
            else            
//            zeroFinalVelocityPath(
//                        start,
//                        final,
//                        1.0 / dt, //1000.0/conf()->Common_Command_Interval(),//   1.0/getFramePeriod(),
//                        4.0, //conf()->BangBang_newRobots_AccTangent_Max(),
//                        90.0, //theta-acc (deg/sec^2)
//                        4.0, //conf()->BangBang_newRobots_Vel_Max(),
//                        180.0, //theta-vel (deg/sec)
//                        path,
//              -1
//            );

                fastPath(
                            start,
                            final,
                            1.0 / dt, //1000.0/conf()->Common_Command_Interval(),//   1.0/getFramePeriod(),
                            4.0, //conf()->BangBang_newRobots_AccTangent_Max(),

                            4.0, //conf()->BangBang_newRobots_Vel_Max()
                            path,
                  -1
                );

                debug(QString("Command Frame Rate=%1").arg(knowledge->commandFrameRate), D_ERROR);
            pathGenerated = frame;
            currentPos = 1;
        }
        ObjectPosVel p;
        if (path.getNumSteps() > 0)
        {            
            double bestDist = 1e10;
            int k = -1;
            for (int i=0;i<path.getNumSteps();i++)
            {
                ObjectPosVel posvel;
                path.get(i, posvel);
                double dist = (Vector2D(posvel.xPos, posvel.yPos) - agent->pos()).length() /*+ fabs(currentPos - i)*0.3*/;
                if (dist < bestDist)
                {
                    bestDist = dist;
                    k = i;
                }
            }
            k++;
            if (k > currentPos) currentPos = k+1;
            if (currentPos >= path.getNumSteps()) currentPos = path.getNumSteps() - 1;
//currentPos = 5;
            int index = currentPos + 4;//delayTime / dt; //4 = 20ms / 5ms (delay effect)
            if (index >= path.getNumSteps()) index = path.getNumSteps() - 1;
            path.get(index, p);
            pathError = (Vector2D(p.xPos - agent->pos().x - agent->vel().x * delayTime, p.yPos - agent->pos().y - agent->vel().y * delayTime).length());

            p.rotVel = -Vector2D::angleBetween(targetDir, agent->dir()).degree();
            Vector2D v;
            v.assign(p.xVel, p.yVel);
//            v += Vector2D(p.xPos - agent->pos().x, p.yPos - agent->pos().y).norm() * 0.6;
            agent->setRobotAbsVel(v.x, v.y, ew);
            qDebug() << v.length() << " ; " << agent->vel().length();
            draw(Segment2D(agent->pos(), agent->pos() + v), "blue");
            currentPos ++;
        }
        else {
            agent->setRobotAbsVel(0.0, 0.0, 0.0);
        }
        for (int i=0;i<path.getNumSteps();i++){
            ObjectPosVel p;
            path.get(i, p);
            draw(Vector2D(p.xPos, p.yPos), 1, "red");
            //draw(Vector2D(p.xPos+p.xVel*dt, p.yPos+p.yVel*dt), 1, "black");
            draw(QString("count=%1").arg(path.getNumSteps()), Vector2D(1.0, -1.0), "red");
        }
        draw(Circle2D(Vector2D(p.xPos, p.yPos), 0.05), "blue", true);

    }
    frame ++;
}





//#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>

using namespace std;

float sqr(float a) {return a*a;}
float sgn(float a) {return (a>0) ? 1 : ((a<0) ? -1 : 0);}
#define PI M_PI
float __max(float a, float b) {return (a>b) ? a : b;}
float __min(float a, float b) {return (a<b) ? a : b;}
//************************************************************************
float distance(float x1, float y1, float x2, float y2)
{
  return (float)sqrt ( sqr(x2-x1) + sqr(y2-y1) );
}

//************************************************************************
void bangBangPathProperties(float z0,
                            float zf,
                            float vz0,
                            float ubar,
                            float &t1,
                            float &t2,
                            float &tf,
                            float &u)
{
    float c = z0-zf+vz0;

    if (ubar<0.01)
      u=0;
    else
      u = ubar*sgn(vz0/ubar-sgn(c)*((float)exp(fabs(c/ubar))-1));

    if (u==0)
      t1=t2=tf=u=0;
    else
    {
      t2 = (float)log(1+sqrt(1+exp(c/u)*(vz0/u-1)));
      t1 = t2-c/u;
      tf = t1+t2;
    }
}
//************************************************************************
float tFinal(float z0, float zf, float vz0, float ubar)
{
  float t1,t2,tf,u;
  bangBangPathProperties(z0,zf,vz0,ubar,t1,t2,tf,u);
  return tf;
}
//************************************************************************
float dtFinal(float x0, float xf, float vx0, float y0, float yf, float vy0,float u)
{
    return tFinal(x0,xf,vx0,u)-tFinal(y0,yf,vy0,(float)sqrt(1-sqr(u)));
}
//************************************************************************
// step returns position (z) and velocity (vz) at t=timestep*dt
// from the solution of
// z''(t)+z(t)'=uz 0<=t<t1z
// z''(t)+z(t)'=-uz t1z<=t<tfz
// z(0)=z0  z'(0)=vz0 z(tfz)=zf  z'(tfz)=0
// also z(t)=zf and z'(t)=0 tfz<t
//inputs (nondimensional)
//          z0 - starting point
//          zf - destination
//         vz0 - initial velocity (final velocity is zero)
//          uz - control effort
//         t1z - length of the first interval
//         tfz - total time
//          dt - length of timestep
//    timestep - number of timestep
//************************************************************************
void step(float z0,
          float zf,
          float vz0,
          float uz,
          float t1z,
          float tfz,
          float dt,
          int timestep,
          float& z,     // Return variable
          float& vz     // Return variable
          )
{
    float t=(float)timestep*dt;
    float expmt=(float)exp(-t);

    if (t<=t1z)
    {
        z=uz*(t-1)+expmt*(uz-vz0)+z0+vz0;
        vz=uz+expmt*(vz0-uz);
    }
    else if (t<=tfz)
    {
        float exptfz=(float)exp(tfz);
        z=uz*(1-t+tfz-expmt*exptfz)+zf;
        vz=uz*(expmt*exptfz-1);
    }
    else
    {
        z=zf;
        vz=0;
    }
}



//************************************************************************
// zeroFinalVelocityPath solves
// x''(t)+x'(t)=ux(t)
// y''(t)+y'(t)=uy(t)
// theta''(t)+theta'(t)=utheta(t)
// with boundary conditions
// x(0)=x0  x'(0)=vx0 x(tf)=xf  x'(tf)=0
// y(0)=y0  y'(0)=vy0 y(tf)=yf  y'(tf)=0
// theta(0)=theta0  theta'(0)=vtheta0 theta(tf)=thetaf  theta'(tf)=0
// and the constraint ux(t)^2+uy(t)^2=1
// and returns the path
//inputs (dimensional)
//               z0 - starting point
//               zf - destination
//              vz0 - initial velocity (final velocity is zero)
//        frameRate - points in the path are generated with stepsize 1/frameRate
//         maxAccel - maximum translational acceleration
//    maxThetaAccel - maximum angular acceleration
//      maxVelocity - maximum translational velocity
// maxThetaVelocity - maximum angular velocity
//    numberOfSteps - number of points generated along the path (-1 returns the whole path)
//************************************************************************
void zeroFinalVelocityPath( ObjectPosVel start,
                            ObjectPosVel final,
                            float frameRate,
                            float maxAccel,
                            float maxThetaAccel,
                            float maxVelocity,
                            float maxThetaVelocity,
                            ObjectPath& thePath,
                            int numberOfSteps
                          )
{
  int  i,j,numIter,maxStepNumber;
  float du,fmid,xmid,u;
  float t1x,t2x,tfx,ux;
  float t1y,t2y,tfy,uy;
  float t1theta,t2theta,tftheta,utheta;
  float dt,angulardt;
  float x,vx,y,vy,theta,vtheta;

  if (maxAccel==0 or maxThetaAccel==0 or maxVelocity==0 or maxThetaVelocity==0)
    cout << "Trajectory code error: invalid scale(s)" << endl;

  float timeScale             = maxVelocity/maxAccel;
  float angularTimeScale      = maxThetaVelocity/maxThetaAccel;
  float lengthScale           = maxVelocity*timeScale;
  float velocityScale         = maxVelocity;
  float angularScale          = maxThetaVelocity*angularTimeScale;
  float angularVelocityScale  = maxThetaVelocity;

//  nondimensionalize positions and velocities
  float oneoverlengthscale = 1/lengthScale;
  float x0      = start.xPos*oneoverlengthscale;
  float y0      = start.yPos*oneoverlengthscale;
  float xf      = final.xPos*oneoverlengthscale;
  float yf      = final.yPos*oneoverlengthscale;
  float vx0     = start.xVel/velocityScale;
  float vy0     = start.yVel/velocityScale;
  float vtheta0 = start.rotVel/angularVelocityScale;

  float theta0  = start.rotation;
  float thetaf  = final.rotation;

//finds the shortest "rotational path" between theta0 and thetaf
  float dtheta=thetaf-theta0;
  if (dtheta < -PI)
    dtheta = dtheta + 2*PI;
  if (dtheta > PI)
    dtheta = dtheta - 2*PI;
  thetaf=theta0 + dtheta;

  theta0 = theta0 / angularScale;
  thetaf = thetaf / angularScale;

//finds the zero of tf(x0,xf,vx0,u)-tf(y0,yf,vy0,sqrt(1-sqr(u))) in (0,1) by bisection
  numIter = 10;//number of iterations performed. error in u is 2^(-numIter)
  u   =  0.999f;
  du  = -0.998f;

  for (j=0; j<numIter; j++)
  {
    fmid = dtFinal(x0,xf,vx0,y0,yf,vy0,xmid=u+(du *= 0.5));
    if (fmid <= 0.0) u=xmid;
  }

//now we calculate the path properties
  bangBangPathProperties(x0,xf,vx0,u,t1x,t2x,tfx,ux);
  bangBangPathProperties(y0,yf,vy0,(float)sqrt(1-sqr(u)),t1y,t2y,tfy,uy);

  //theta is independent of x and y so we use bang-bang with u=1
  bangBangPathProperties(theta0,thetaf,vtheta0,1,t1theta,t2theta,tftheta,utheta);

// building the path
  thePath.clearPath();

  float nondimFrameRate = frameRate*timeScale;
  float nondimAngularFrameRate = frameRate*angularTimeScale;
  int tfint = (int)ceil(__max(tfx,tfy)*nondimFrameRate);
  int tfthetaint = (int)ceil(tftheta*nondimAngularFrameRate);

  dt = 1/nondimFrameRate;
  angulardt = 1/nondimAngularFrameRate;

  if (numberOfSteps == -1){
    maxStepNumber=__max(tfint,tfthetaint);
  }
  else {
    maxStepNumber=__min(numberOfSteps,__max(tfint,tfthetaint));
  }
  maxStepNumber=__min(maxStepNumber,MAX_TRAJECTORY_STEPS);

  for ( i = 0; i <= maxStepNumber; i++ )
  {
    // Calculate step along x-axis
    step(x0,xf,vx0,ux,t1x,tfx,dt,i, x, vx);
    x  = lengthScale*x;
    vx = velocityScale*vx;

    // Calculate step along y-axis
    step(y0,yf,vy0,uy,t1y,tfy,dt,i,y,vy);
    y  = lengthScale*y;
    vy = velocityScale*vy;

    // Calculate step along theta-axis
    step(theta0,thetaf,vtheta0,utheta,t1theta,tftheta,angulardt,i,theta,vtheta);
    theta  = angularScale*theta;
    vtheta = angularVelocityScale*vtheta;

    // Add step to the path
    thePath.addStep(x,y,theta,vx,vy,vtheta);
  }
}

//************************************************************************
void zeroFinalVelocityTheta(ObjectPosVel start,
                            ObjectPosVel final,
                            float frameRate,
                            float maxThetaAccel,
                            float maxThetaVelocity,
                            float & nextAngularVelocity
                          )
{
  float t1theta,t2theta,tftheta,utheta;

  float angularTimeScale      = maxThetaVelocity/maxThetaAccel;
  float angularScale          = sqr(maxThetaVelocity)/maxThetaAccel;
  float angularVelocityScale  = maxThetaVelocity;

  float theta0  = start.rotation;
  float thetaf  = final.rotation;

  float dtheta=thetaf-theta0;

  if (dtheta < -PI)
    dtheta = dtheta + 2*PI;
  if (dtheta > PI)
    dtheta = dtheta - 2*PI;

  thetaf=theta0 + dtheta;

  theta0 = theta0 / angularScale;
  thetaf = thetaf / angularScale;



  float vtheta0 = start.rotVel/angularVelocityScale;

  //use bang-bang with u=1 on the theta-axis
  bangBangPathProperties(theta0,thetaf,vtheta0,1,t1theta,t2theta,tftheta,utheta);

  float nondimAngularFrameRate = frameRate*angularTimeScale;
  float angulardt = 1/nondimAngularFrameRate;
  float temp=(float)exp(-angulardt);
  nextAngularVelocity=angularVelocityScale*(temp*vtheta0+utheta*(1-temp));

}

//************************************************************************
void fastPath( ObjectPosVel start,
               ObjectPosVel final,
               float frameRate,
               float maxAccel,
               float maxVelocity,
               ObjectPath& thePath,
               int numberOfSteps)
{
  int  i,maxStepNumber;
  float x,vx,y,vy;

  if (maxAccel==0 or maxVelocity==0)
    cout << "Trajectory code error: invalid scale(s)" << endl;

  float timeScale             = maxVelocity/maxAccel;
  float lengthScale           = sqr(maxVelocity)/maxAccel;
  float velocityScale         = maxVelocity;

  //  nondimensionalize positions and velocities
  float x0      = start.xPos/lengthScale;
  float y0      = start.yPos/lengthScale;
  float theta0  = start.rotation;
  float thetaf  = final.rotation;
  float vx0     = start.xVel/velocityScale;
  float vy0     = start.yVel/velocityScale;
  float xf      = final.xPos/lengthScale;
  float yf      = final.yPos/lengthScale;

  float dx=xf-x0;
  float dy=yf-y0;
  float t=0;
  float dt=0.01f;
  float func=0;
  float temp=0;
  while(func>=0)
  {
    t+=dt;
    temp=1-(float)exp(-t);
    func=sqr( dx-(vx0*temp) ) + sqr( dy- (vy0*temp) ) - sqr(temp-t);
  }
  float phi = (float)atan2((dy-vy0*temp) / (t-temp), (dx-vx0*temp) / (t-temp));

// building the path
  thePath.clearPath();

  float nondimFrameRate = frameRate*timeScale;
  int tfint = (int)ceil(t*nondimFrameRate);

  dt = 1/nondimFrameRate;

  if (numberOfSteps == -1){
    maxStepNumber=tfint;
  }
  else {
    maxStepNumber=__min(numberOfSteps,tfint);
  }
  maxStepNumber=__min(maxStepNumber,MAX_TRAJECTORY_STEPS);

  for ( i = 0; i <= maxStepNumber; i++ )
  {

    // Calculate step along x-axis
    step(x0,xf,vx0,(float)cos(phi),t,t,dt,i, x, vx);
    x  = lengthScale*x;
    vx = velocityScale*vx;

    // Calculate step along y-axis
    step(y0,yf,vy0,(float)sin(phi),t,t,dt,i,y,vy);
    y  = lengthScale*y;
    vy = velocityScale*vy;

    // Add step to the path
    thePath.addStep(x,y,theta0,vx,vy,0.0f);
  }
}
//************************************************************************
void expectedPathTime(  ObjectPosVel start,
                        ObjectPosVel final,
                        float frameRate,
                        float maxAccel,
                        float maxVelocity,
                        float& pathTime
                      )
{
  int  j,numIter;
  float du,fmid,xmid,u;
  float t1x,t2x,tfx,ux;
  float t1y,t2y,tfy,uy;

  if (maxAccel==0 or maxVelocity==0)
    cout << "Trajectory code error: invalid scale(s)" << endl;

  float timeScale             = maxVelocity/maxAccel;
  float lengthScale           = sqr(maxVelocity)/maxAccel;
  float velocityScale         = maxVelocity;

  //  nondimensionalize positions and velocities
  float x0      = start.xPos/lengthScale;
  float y0      = start.yPos/lengthScale;
  float vx0     = start.xVel/velocityScale;
  float vy0     = start.yVel/velocityScale;
  float xf      = final.xPos/lengthScale;
  float yf      = final.yPos/lengthScale;

//finds the zero of tf(x0,xf,vx0,u)-tf(y0,yf,vy0,sqrt(1-sqr(u))) in (0,1) by bisection
  numIter = 10;//number of iterations performed
  u   =  0.999f;
  du  = -0.998f;

  for (j=1; j<=numIter; j++)
  {
    fmid = dtFinal(x0,xf,vx0,y0,yf,vy0,xmid=u+(du *= 0.5));
    if (fmid <= 0.0) u=xmid;
  }

//now we calculate the path properties
  bangBangPathProperties(x0,xf,vx0,u,t1x,t2x,tfx,ux);
//  pathTime=timeScale*tfx;

  bangBangPathProperties(y0,yf,vy0,(float)sqrt(1 - u*u),t1y,t2y,tfy,uy);
  pathTime=timeScale*__max(tfx,tfy);
}


//=================================================
// PROPORTIONAL GAIN FUNCTIONS

void proportionalGainPath(ObjectPosVel start,
                          ObjectPosVel final,
                          float frameRate,
                          float translationalGain,
                          float angularGain,
                          float maxVelocity,
                          float maxThetaVelocity,
                          ObjectPath& thePath
                         )
{

  float x0      = start.xPos;
  float y0      = start.yPos;
  float theta0  = start.rotation;
  float xf      = final.xPos;
  float yf      = final.yPos;
  float thetaf  = final.rotation;
  float vx0     = start.xVel;
  float vy0     = start.yVel;
  float vtheta0 = start.rotVel;

  float dtheta=thetaf-theta0;//convert it!

  if (dtheta < -PI)
    dtheta = dtheta + 2*PI;
  if (dtheta > PI)
    dtheta = dtheta - 2*PI;

  thetaf = theta0 + dtheta;

  float dt = 1/frameRate;

  thePath.clearPath();
  thePath.addStep(x0,y0,theta0,vx0,vy0,vtheta0);//add the initial position

  float vx = translationalGain*(xf-x0);
  float vy = translationalGain*(yf-y0);
  float vtheta = angularGain*dtheta;

  float speed = (float)sqrt(vx*vx + vy*vy);

  // Saturate translational velocity
  if (speed > maxVelocity)
  {
    float scale=maxVelocity/speed;
    vx = scale*vx;
    vy = scale*vy;
  }

  // Saturate angular velocity
  if (vtheta > maxThetaVelocity)
  {
    vtheta = maxThetaVelocity;
  }
  else if (vtheta < -maxThetaVelocity)
  {
    vtheta = -maxThetaVelocity;
  }
  thePath.addStep(x0+dt*vx,y0+dt*vy,theta0+dt*vtheta,vx,vy,vtheta);
}
//=================================================
void proportionalGainTheta(ObjectPosVel start,
                           ObjectPosVel final,
                           float frameRate,
                           float angularGain,
                           float maxThetaVelocity,
                           float & nextAngularVelocity
                          )
{

  float theta0  = start.rotation;
  float thetaf  = final.rotation;

  float dtheta=thetaf-theta0;

  if (dtheta < -PI)
    dtheta = dtheta + 2*PI;
  if (dtheta > PI)
    dtheta = dtheta - 2*PI;

  nextAngularVelocity = angularGain*dtheta;

  // Saturate angular velocity
  if (nextAngularVelocity > maxThetaVelocity)
  {
    nextAngularVelocity = maxThetaVelocity;
  }
  else if (nextAngularVelocity < -maxThetaVelocity)
  {
    nextAngularVelocity = -maxThetaVelocity;
  }


}





void ObjectPath::clearPath()
{
  count = 0;
}

bool ObjectPath::get(int step, ObjectPosVel & nextStep)
{
  if (step >= count) return false;

  nextStep.xPos     = path[step].xPos;
  nextStep.yPos     = path[step].yPos;
  nextStep.rotation = path[step].rotation;
  nextStep.xVel     = path[step].xVel;
  nextStep.yVel     = path[step].yVel;
  nextStep.rotVel   = path[step].rotVel;

//  memcpy(&nextStep, &path[step], sizeof(ObjectPosVel));
  return true;
}

bool ObjectPath::set(int step, const ObjectPosVel & nextStep)
{
  if (step >= count) return false;

  path[step].xPos     = nextStep.xPos;
  path[step].yPos     = nextStep.yPos;
  path[step].rotation = nextStep.rotation;
  path[step].xVel     = nextStep.xVel;
  path[step].yVel     = nextStep.yVel;
  path[step].rotVel   = nextStep.rotVel;

//  memcpy(&path[step], &nextStep, sizeof(ObjectPosVel));
  return true;
}

bool ObjectPath::set(int step, const float xPos, const float yPos, const float rotation,
            const float xVel, const float yVel, const float rotVel)
{
  if (step >= count) return false;

  path[step].xPos     = xPos;
  path[step].yPos     = yPos;
  path[step].rotation = rotation;
  path[step].xVel     = xVel;
  path[step].yVel     = yVel;
  path[step].rotVel   = rotVel;
  return true;
}

bool ObjectPath::addStep(const ObjectPosVel & nextStep)
{
  if (MAX_TRAJECTORY_STEPS == count) return false;

  path[count].xPos     = nextStep.xPos;
  path[count].yPos     = nextStep.yPos;
  path[count].rotation = nextStep.rotation;
  path[count].xVel     = nextStep.xVel;
  path[count].yVel     = nextStep.yVel;
  path[count].rotVel   = nextStep.rotVel;

//  memcpy(&path[count], &nextStep, sizeof(ObjectPosVel));
  count++;
  return true;
}

bool ObjectPath::addStep(const float xPos, const float yPos, const float rotation,
            const float xVel, const float yVel, const float rotVel)
{
  if (MAX_TRAJECTORY_STEPS == count) return false;

  path[count].xPos      = xPos;
  path[count].yPos      = yPos;
  path[count].rotation  = rotation;
  path[count].xVel      = xVel;
  path[count].yVel      = yVel;
  path[count].rotVel    = rotVel;
  count++;
  return true;
}


bool ObjectPath::writeToFile(const char * fname)
{
    FILE * f = fopen(fname, "a");
    ObjectPosVel nextStep;

    if(!f) return false;

    fprintf(f,"%d\n",count);
    for (int i=0; i < count; i++)
    {
      get(i, nextStep);
      fprintf(f,"(%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f)\n",
        nextStep.xPos,
        nextStep.yPos,
        nextStep.rotation,
        nextStep.xVel,
        nextStep.yVel,
        nextStep.rotVel
      );
    }
    fclose(f);
    return true;
}



