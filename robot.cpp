#include "robot.h"
#include "varswidget.h"
#include "logger.h"
#include "knowledge.h"
#include <QFile>
#include <QDebug>
#include <fstream>



const double CRobot::robot_radius_old = 0.090;
//const double CRobot::robot_radius_new = 0.080;
const double CRobot::robot_radius_new = 0.0890;
const double CRobot::center_from_kicker_old = 0.075;
//const double CRobot::center_from_kicker_new = 0.070;
const double CRobot::center_from_kicker_new = 0.06;
const double CRobot::kicker_width_old = 0.074;
//const double CRobot::kicker_width_new = 0.080;
const double CRobot::kicker_width_new = 0.082;
const double CRobot::wheel_rad_old = 0.035;
//const double CRobot::wheel_rad_new = 0.03;
const double CRobot::wheel_rad_new = 0.027;

void CRobot::setNewRobot(bool _new)
{
    newRobot = _new;
}

bool CRobot::isNewRobot()
{
    return newRobot;
}

double CRobot::robotRadius()
{
    return (newRobot) ? robot_radius_new : robot_radius_old;
}

double CRobot::kickerWidth()
{
    return (newRobot) ? kicker_width_new : kicker_width_old;
}

double CRobot::centerFromKicker()
{
    return (newRobot) ? center_from_kicker_new : center_from_kicker_old;
}

double CRobot::wheelRadius()
{
    return (newRobot) ? wheel_rad_new : wheel_rad_old;
}


CRobot::CRobot(int _id, bool isOurTeam, bool noKalman) : CMovingObject(true)
{
    /*////////////////////*/


    lastFrameKalmanReset = 0;
    newRobot = false;
    repl = false;
    lastFrameAngle = turns = 0.0;
    id = _id;
    inOurTeam = isOurTeam;
    vel.assign(0,0);
    angularVel = 0;
    lastInsight = 0;
    inSight = 0.0;
    motionEstimator = NULL;
    elementNotInSight = true;
    if (!noKalman)
    {
        tracker = new RobotTracker(getLatencyDelay(), inOurTeam);
        tracker->reset();
    }
    else
    {
        tracker = NULL;
    }
    blindness = 100;
    ANGULAR_DIRECTION = 0.0;
    ADD_TO_ANGULAR_DIRECTION = 0.0;

    markedByDefense = false;
    markedByMark = false;
}

CRobot::~CRobot()
{
    if (motionEstimator!=NULL) delete motionEstimator;
}

void CRobot::resetKalman()
{
    if (tracker != NULL)
    {
        delete tracker;
        tracker = new RobotTracker(getLatencyDelay() , inOurTeam );
        tracker->reset();
    }
}

void CRobot::init()
{
    //    motionEstimator = new CMotionEstimator();
    //    motionEstimator->svm->load("model.svm");
}

void CRobot::filter(int vanished)
{

    //	bool all = false;

    if (tracker == NULL)
    {
        CMovingObject::filter(vanished);
        return;
    }

    if( ANGULAR_DIRECTION > 120.0 && observation.dir.th().degree() < -120.0 )
    {
        ADD_TO_ANGULAR_DIRECTION += 360.0;
    }

    if( ANGULAR_DIRECTION < -120.0 && observation.dir.th().degree() > 120.0 )
    {
        ADD_TO_ANGULAR_DIRECTION -= 360.0;
    }

    ANGULAR_DIRECTION = observation.dir.th().degree();
    ////////////////////mhmmd
    int kalmanVelTune = 8;
    if (vanished<=0)
    {
        vraw v;

        v.angle = ANGULAR_DIRECTION + ADD_TO_ANGULAR_DIRECTION;
        v.conf  = observation.confidence;
        v.pos   = observation.pos;
        v.timestamp = observation.time;
        if(inOurTeam)
        {
            tracker->command(v.timestamp,Vector2D(wm->our[id]->kalman_velocs.vx,wm->our[id]->kalman_velocs.vy),wm->our[id]->kalman_velocs.vw);
        }
        tracker->observeNew(v,0,0);
        pos = v.pos;
        dir = observation.dir;
        //pos = tracker->position(getFramePeriod());
        vel = tracker->velocity(kalmanVelTune*getFramePeriod());
        acc = tracker->acceleration(kalmanVelTune*getFramePeriod());
        //dir = Vector2D::unitVector(tracker->direction(getFramePeriod()));
        angularVel = tracker->angular_velocity(kalmanVelTune*getFramePeriod());

        inSight = 1.0;
    }
    else
    {
        if (vanished<blindness)
        {
            /*
            vraw v;
            v.conf  = 1;//observation.confidence;
            v.angle = observation.dir.th().degree();// + angularVel*getFramePeriod();
//            vel *= 0.5;
            vel *= 0.0;
            v.pos   = observation.pos;// + vel*getFramePeriod();
            v.timestamp = observation.time;
//            tracker->observe(v, v.timestamp);
//            tracker->observeNew(v);
            pos = tracker->position(getFramePeriod());
            pos = v.pos;

            vel = tracker->velocity(getFramePeriod());
//            dir = Vector2D::unitVector(tracker->angular_velocity(getFramePeriod()));
//            angularVel = tracker->angular_velocity(getFramePeriod());
//            tracker->reset();
            angularVel *= 0.8;
            dir = Vector2D::unitVector(tracker->direction(getFramePeriod()));
//            dir = Vector2D::unitVector(v.angle);
            inSight = 0.5;
            */

            vraw v;
            v.conf  = 1;
            v.angle = ANGULAR_DIRECTION + ADD_TO_ANGULAR_DIRECTION;
            v.pos   = observation.pos;
            v.timestamp = observation.time;

            if(inOurTeam)
            {
                tracker->command(v.timestamp,Vector2D(kalman_velocs.vx,kalman_velocs.vy),kalman_velocs.vw);
            }

            pos = v.pos;
            dir = observation.dir;
            //			pos = tracker->position((8+vanished)*getFramePeriod());
            vel = tracker->velocity((kalmanVelTune+vanished-1)*getFramePeriod());
            acc = tracker->acceleration((kalmanVelTune+vanished-1)*getFramePeriod());
            //			dir = Vector2D::unitVector(tracker->direction((8+vanished)*getFramePeriod()));
            angularVel = tracker->angular_velocity((kalmanVelTune+vanished-1)*getFramePeriod());
            inSight = 0.5;
        }

        else
        {
            tracker->reset();
            vel *= 0.8;
            acc *= 0.8;
            angularVel *= 1;
            inSight = 0.0;
        }
    }


#ifdef velProblem1
    ////////////////////////////////////// if vel problem :(((((((
    vel = vel *2;
#endif
#ifdef velProblem2
    vel = vel /2;
#endif

}

void CRobot::setReplace(Vector2D newPos, float newDirection)
{
    replPos = newPos;
    replDir = newDirection;
    repl = true;
}

int CRobot::replacementPacket(char* buf)
{
    if (repl) buf[0] = 100;
    else buf[0] = 10;
    float x;
    x = replPos.x;
    buf[1] = *((char*) (& (x)) );
    buf[2] = *((char*) (& (x)) + 1);
    buf[3] = *((char*) (& (x)) + 2);
    buf[4] = *((char*) (& (x)) + 3);
    x = replPos.y;
    buf[5] = *((char*) (& (x)) );
    buf[6] = *((char*) (& (x)) + 1);
    buf[7] = *((char*) (& (x)) + 2);
    buf[8] = *((char*) (& (x)) + 3);
    x = replDir;
    buf[9]  = *((char*) (& (x)) );
    buf[10] = *((char*) (& (x)) + 1);
    buf[11] = *((char*) (& (x)) + 2);
    buf[12] = *((char*) (& (x)) + 3);
    repl = false;
    return 13;
}

double distFunction(Vector2D pos1,Vector2D dir1,Vector2D pos2,Vector2D dir2)
{
    return hypot(((pos1-pos2).length() / hypot(_FIELD_WIDTH,_FIELD_HEIGHT)),Vector2D::angleBetween(dir1,dir2).degree()/360.0);
}



bool CRobot::isActive()
{
    return (this->inSight > 0.0);
}

Vector2D CRobot::getKickerPos(double margin)
{
    return pos + dir*(centerFromKicker()+margin);
}

Circle2D CRobot::getCirle()
{
    return Circle2D(pos, robotRadius());
}

bool CRobot::isBallOwner(double verticaldist, double horizontaldist)
//default horizontal dist is kickerWidth()
{
    double d1 = horizontaldist;
    if (d1 < 0) d1 = kickerWidth();
    return (this->kickSensor && conf()->Common_KickSensor()) ||
            ((fabs((ball->pos - pos).innerProduct(dir))<centerFromKicker()+CBall::radius+verticaldist)
             && (fabs((ball->pos - pos).outerProduct(dir))<d1*0.5));
}

double CRobot::ballComingSpeed()
{
    return ball->vel * (pos - ball->pos).norm();
}

bool CRobot::isBallComingTowardMe(double factor,double vBallMin)
{
    Vector2D v1=pos-ball->pos;
    Vector2D v2=ball->vel.norm();
    double k = 1.0 + factor*(v1.length()-centerFromKicker())/(hypot(_FIELD_WIDTH,_FIELD_HEIGHT));
    return (fabs(v1.outerProduct(v2)) < k*kickerWidth()*0.5) && (v1.innerProduct(v2) > 0) && (ball->vel.length()>vBallMin);
}

bool CRobot::isBallGoingFartherFrom(double factor,double vBallMin)
{
    Vector2D v1=pos-ball->pos;
    Vector2D v2=ball->vel.norm();
    double k = 1.0 + factor*(v1.length()-robotRadius())/(hypot(_FIELD_WIDTH,_FIELD_HEIGHT));
    return (fabs(v1.outerProduct(v2)) < k*kickerWidth()*0.5) && (v1.innerProduct(v2) < 0) && (ball->vel.length()>vBallMin);
}

bool CRobot::isBallBack()
{
    return (((ball->pos-pos).outerProduct(dir) < 2.0*robotRadius())
            &&
            ((ball->pos-(dir*robotRadius())).innerProduct(dir)<0));
}

bool CRobot::isBallAside()
{
    //atan(3.5/8.7)  ~ 25 degrees
    if ((ball->pos - pos).length() < robotRadius() + CBall::radius + 0.03)
    {
        if (fabs(Vector2D::angleBetween(dir, ball->pos - pos).degree())>20)
            return true;
    }
    else
        return false;
    return false;
}

bool CRobot::collidesBall(Vector2D target)
{
    Line2D l(pos, target);
    double s = sign((ball->pos - pos) * (target - pos))*sign((ball->pos - target) * (target - pos));
    if (s>0) return false;
    return l.dist(ball->pos) < CBall::radius + robotRadius() + 0.01;
}

bool CRobot::isBallBack(Vector2D goal)
{
    Vector2D target = -(goal - ball->pos).norm()*(centerFromKicker() + CBall::radius+0.020) + ball->pos;
    Segment2D l1(this->pos+Vector2D::unitVector((this->pos-target).th().degree()+90)*robotRadius(),target+Vector2D::unitVector(this->dir.th().degree()+90)*robotRadius());
    Segment2D l2(this->pos+Vector2D::unitVector((this->pos-target).th().degree()-90)*robotRadius(),target+Vector2D::unitVector(this->dir.th().degree()-90)*robotRadius());
    Segment2D l3(this->pos,target);
    Segment2D l4(this->pos+Vector2D::unitVector((this->pos-target).th().degree()+90)*robotRadius()*0.5,target+Vector2D::unitVector(this->dir.th().degree()+90)*robotRadius()*0.5);
    Segment2D l5(this->pos+Vector2D::unitVector((this->pos-target).th().degree()-90)*robotRadius()*0.5,target+Vector2D::unitVector(this->dir.th().degree()-90)*robotRadius()*0.5);
    Circle2D cc(ball->pos,robotRadius());
    Vector2D s1,s2;
    if (Vector2D::angleOf(goal,this->pos,ball->pos).degree()<30) return false;
    bool f = ((cc.intersection(l1,&s1,&s2)!=0) ||
              (cc.intersection(l2,&s1,&s2)!=0) ||
              (cc.intersection(l3,&s1,&s2)!=0) ||
              (cc.intersection(l4,&s1,&s2)!=0) ||
              (cc.intersection(l5,&s1,&s2)!=0));
    return f;
}

void CRobot::recvData(char Data)
{
    kickSensor = (((Data & 0x08) >> 4) == 1);
}
