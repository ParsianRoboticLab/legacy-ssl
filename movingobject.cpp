#include "movingobject.h"
#include <QDebug>
#include <logger.h>
#include <mathtools.h>
#include <robottracker.h>
#include <knowledge.h>

double observeTimeStep = 0.02;
double lastObserveTimeStep = 0.01;

CRawObject::CRawObject(int frameCnt, Vector2D _pos, double orientation, int _ID, double _confidence, CMovingObject *_ref, int _cam_id, double t)
{
    frameCount = frameCnt;
    pos = _pos;
    ID = _ID;
    dir = Vector2D::unitVector(orientation);
    confidence = _confidence;
    ref = _ref;
    cam_id = _cam_id;
    time = t;
    merged = false;
}

CRawObject::CRawObject()
{
    frameCount = 0;
    pos.invalidate();
    ID = -1;
    confidence = 0;
    time = 0.0;
    ref = NULL;
    merged = false;
    mergeCount = 0;
}

CMovingObject::CMovingObject(bool resetToZero)
{
    frameCounter = 0;
    lastFrameKalmanReset = 0;
    lastFrameUpdated = 0;
    cam_id = -1;
    inSight = 0;
    vel.assign(0,0);
    acc.assign(0,0);
    modelObjStopped = true;
    modelObjStopPos.invalidate();
    modelC0 = modelC1 = modelC2 = 0.0;
    modelCurDir.invalidate();
    modelSampleTime = 0.02;
    if (!resetToZero)
    {
        pos.invalidate();
    }
    else pos.assign(0,0);
}

void CMovingObject::update(CMovingObject* obj)
{
    if (obj==NULL) return;
    cam_id = obj->cam_id;
    pos = obj->pos;
    vel = obj->vel;
    acc = obj->acc;
    dir = obj->dir;
    angularVel = obj->angularVel;
    inSight = obj->inSight;
    hist.clear();
    hist.append(obj->hist);
    modelObjStopped = obj->modelObjStopped;
    modelObjStopPos = obj->modelObjStopPos;
    modelC0 = obj->modelC0;
    modelC1 = obj->modelC1;
    modelC2 = obj->modelC2;
    modelCurDir = obj->modelCurDir;
    modelSampleTime = obj->modelSampleTime;
}

void CMovingObject::update(CRawObject raw)
{
    cam_id = raw.cam_id;
    observation = raw;


    //pos = observation.pos;
    //vanishedCounter = 0;
    //inSight = 1.0;
    kalmanFilter();
//    findModel(dt);
}

void CMovingObject::updateDelayTime(double newDelayTime)
{
    delayTime = newDelayTime;
}

void CMovingObject::init()
{

}

void CMovingObject::findModel(double dt)
{
    if(hist.count() > 100)
        hist.dequeue();
    hist.append(CRawObject(0, pos, dir.th().degree(), -1, 1.0));


    double aa =BallFriction() * Gravity; // acc.length();
    double vv = vel.length()*vel.length();
    if (aa<0.3) aa = 0.3;
    if (acc*vel.norm()>0.3) ballStopPos.invalidate();
    else ballStopPos = pos + vel.norm()*(vv/(2.0*aa));
    if (vv>0.01)
    {
//        if (ballStopPos.valid())
//            draw(Segment2D(ballStopPos-vel.norm().rotatedVector(90)*0.500,ballStopPos+vel.norm().rotatedVector(90)*0.500),QColor("pink"));
//            draw(Segment2D(ballStopPos-vel.norm().rotatedVector(90)*0.500,ballStopPos+vel.norm().rotatedVector(90)*0.500),QColor("pink"));
    }
    //draw(Segment2D(pos, pos + vel), QColor("purple"));
    modelFrameCnt ++;
    modelSampleTime = dt;
    return;
//    if (modelObjBuffer.count()>1)
//    {
//        double dlast = (modelObjBuffer.last().pos - pos).length();
//        bool flag=false;
//        if ((vel.length()>0.500) && (modelObjBuffer.count()>20))
//        {
//            if (fabs(AngleDeg::normalize_angle(modelDir.th().degree() - vel.th().degree()))>30)
//                flag = true;
//        }
//        if (flag
//            || (dlast>1.0000 || dlast<0.002))
//        {
//            modelObjBuffer.clear();
//            modelTimeBuffer.clear();
//        }
//    }
    modelObjBuffer.enqueue(CRawObject(0,this->pos,0,-1,this->inSight));
    modelTimeBuffer.enqueue(modelFrameCnt);
    if (modelObjBuffer.count()>=100)
    {
        modelObjBuffer.dequeue();
        modelTimeBuffer.dequeue();
    }
    int k = 0;
    for (int i = 0;i < modelObjBuffer.count(); i++)
    {
        if ((modelObjBuffer.first().pos - modelObjBuffer[i].pos).length() < 0.003)
            k ++;
        else
            break;
    }
    if (k > modelObjBuffer.count() - 3)
        k = modelObjBuffer.count() - 3;
    for (int i = 0;i < k-1; i++)
    {
        modelObjBuffer.dequeue();
        modelTimeBuffer.dequeue();
    }
    if (modelObjBuffer.count()>4)
    {
        {
            Vector2D pos0 = modelObjBuffer.last().pos;
            double t0 = modelTimeBuffer.last();
            int bufLength = modelObjBuffer.length();
            double *x = new double[bufLength];
            double *y = new double[bufLength];
            double *d = new double[bufLength];
            double *t = new double[bufLength];

            if (bufLength>=10)
            {
                for (int i=0;i<bufLength;i++)
                {
                    d[i] = -(modelObjBuffer[i].pos - pos0).length();
                    t[i] = modelTimeBuffer[i]-t0;
                    x[i] = modelObjBuffer[i].pos.x;
                    y[i] = modelObjBuffer[i].pos.y;
                }
                int tail = (bufLength*4.0)/5.0;
                linefit(bufLength-tail-1, x+tail, y+tail, modelDirC0, modelDirC1);
                modelCurDir = Vector2D::unitVector(AngleDeg::atan_deg(modelDirC1));
                modelCurDir = modelCurDir*((double)sign(modelCurDir.innerProduct(vel)));
                linefit(bufLength, x, y, modelDirC0, modelDirC1);
                modelDir = Vector2D::unitVector(AngleDeg::atan_deg(modelDirC1));
                modelDir = modelDir*((double)sign(modelDir.innerProduct(vel)));
                modelObjStopped = false;
            }
            else
            {
                modelDir.invalidate();
                modelObjStopped = true;
            }
            if (!modelObjStopped)
            {
                if (modelDir.innerProduct(pos - pos0) < 0.0)
                    modelDir = modelDir * (-1.0);
                squarefit(bufLength, t, d, modelC0, modelC1, modelC2);
                if (modelC2>=0 || bufLength<=10)
                {
                    if (bufLength>10)
                    {
                        linefit(10, t+bufLength-11, d+bufLength-11, modelC0, modelC1);
                    }
                    else linefit(bufLength, t, d, modelC0, modelC1);
                    modelC2 = 0.0;
                }
                else {
                    double tmax = -modelC1 / (2.0*modelC2);
                    tmax *= modelSampleTime;
                    modelObjStopPos = vel.norm()*
                                  (tmax*tmax*modelC2 + tmax*modelC1 + modelC0) + pos;
                }
                if (true)
                {
                    double dd,ldd;
                    ldd = 0;
                    if (modelC2<0)
                    {
                        Vector2D normal(-modelDir.y,modelDir.x);
                        normal.normalize();
//                        draw(Segment2D(modelObjStopPos-normal*0.500,modelObjStopPos+normal*0.500),QColor("blue"));
                        if (vel.length()>0.1)
                        {
                        modelC2Sum += modelC2;
                        modelC2Count ++;
                        modelC2Ave = modelC2Sum/(double)modelC2Count;
                        }
                    }
                    double lti=0;
                    double dtt = (t[bufLength-1]-t[0])/((double)(bufLength));
                    for (int i=0;i<100;i++)
                    {
                        double ti = i*dtt;
                        dd = ti*ti*modelC2 + ti*modelC1 + modelC0;
                        ldd = dd;
                        lti = ti;
                    }
                }
            }
            delete []x;
            delete []y;
            delete []d;
            delete []t;
        }
    }


}

void CMovingObject::resetKalman()
{
//overloaded by CBall & CRobot
}

void CMovingObject::filter(int vanished)
{
    pos = observation.pos;
    dir = observation.dir;
    vel.assign(0,0);
    angularVel = 0.0;
    acc.assign(0,0);
}

void CMovingObject::kalmanFilter()
{
    //Kalman Watchdog
#ifndef Q_WS_MAC
    if (std::isnan(vel.x) || std::isnan(vel.y)) /////WHAT THE HELLLL!!!!!!
        {
                resetKalman();
        }
#endif
    if (vel.length() < 0.01)
    {
        if (frameCounter - lastFrameKalmanReset > 30)
        {
            lastFrameKalmanReset = frameCounter;
//            resetKalman();
        }
    }
        if (lastSpeeds.length() > 50)
                lastSpeeds.pop_back();

        if (lastAngularSpeeds.length() > 50)
                lastAngularSpeeds.pop_back();

        lastSpeeds.push_front(vel);
        lastAngularSpeeds.push_front(angularVel);

        if (lastSpeeds.count() >= 10)
        {
                Vector2D mr(0,0);
                for (int i=0;i<lastSpeeds.count();i++)
                {
                        mr.x += lastSpeeds[i].x;
                        mr.y += lastSpeeds[i].y;
                }
                mr /= lastSpeeds.count();
                bool ok = false;
                for (int i=0;i<lastSpeeds.count();i++)
                {
                        if ((lastSpeeds[i] - mr).length() > 0.01)
                        {
                                ok = true;
                                break;
                        }
                }
                double ma = 0;
                for (int i=0;i<lastAngularSpeeds.count();i++)
                {
                        ma += lastAngularSpeeds[i];
                }
                ma /= lastAngularSpeeds.count();
                for (int i=0;i<lastAngularSpeeds.count();i++)
                {
                        if (fabs(lastAngularSpeeds[i] - ma) > 1)
                        {
                                ok = true;
                                break;
                        }
                }
                if (!ok)
                {
                        lastSpeeds.clear();
                        lastAngularSpeeds.clear();
                        resetKalman();
                }
        }

//	debug(QString("ass hole=%1").arg(lastSpeeds.count()), D_ERROR);

    //
    frameCounter ++;
    try {
        inSight = observation.confidence;
        if (observation.confidence<=0)
        {
            vanishedCounter ++;
            filter(vanishedCounter);
        }
        else {
            vanishedCounter = 0;
            inSight = observation.confidence;
            filter(0);
        }
        lastInsight = inSight;
    }
    catch (...)
    {
        qDebug()<<"Warning: Kalman library throwed an exception.";
    }
}


Vector2D CMovingObject::predict(double time)
{
        //must be checked if it works precisely or not
    if (time < 0.001)
            return pos;

        if (acc.length() < 0.01 || vel.length() < 0.1)
        {
                return pos;
        }

    if (acc.valid() && (acc*vel<0))
    {
        double vf = (vel.length() - acc.length()*time);
        if (vf < 0) return pos + vel*vel.length() / (2.0*acc.length());
        return pos - 0.5*acc.length()*vel.norm()*time*time + vel*time;
    }
    return pos + vel*time;
}


Vector2D CMovingObject::predictV(double time)
{
    if (acc.valid() && (acc*vel<0))
    {
        double vf = vel.length() - (acc.length()*time);
        if (vf < 0.0) vf = 0.0;
        return vel.norm() * vf;
    }
    else return vel;
}

Vector2D CMovingObject::whereIsAtVel(Vector2D V)
{
    double dx=(V.x*V.x - vel.x*vel.x)/(2*acc.x);
    double dy=(V.y*V.y - vel.y*vel.y)/(2*acc.y);
    return Vector2D(pos.x + dx, pos.y + dy);
}

double CMovingObject::whenIsAtVel(double L)
{
    double A=acc.x*acc.x + acc.y*acc.y;
    double B=2*(vel.x*acc.x + vel.y*acc.y);
    double C=vel.x*vel.x+vel.y*vel.y-L*L;

    double delta=B*B - 4*A*C;
    if(delta<0)
        return -1;
    double T1 = (-B  + sqrt(delta))/(2*A);
    double T2 = (-B  - sqrt(delta))/(2*A);

    if(T1>0)
    {
        if(T2>0)
        {
            return min(T1,T2);
        }else
        {
            return T1;
        }
    }else
    {
        if(T2>0)
        {
            return T2;
        }else
        {
            return -2;
        }
    }

    return -3;
}

