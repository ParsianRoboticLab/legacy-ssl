#ifndef MovingObject_H
#define MovingObject_H

#include <base.h>
#include <predictor.hpp>
#include "obstacle.h"
#include <ostream>
#include <QQueue>
#include <util/tracker.h>


class CMovingObject;

class CRawObject
{
public:
    CRawObject(int frameCnt, Vector2D _pos, double orientation, int _ID, double _confidence,CMovingObject* _ref=NULL, int _cam_id=0, double t=0.0);
    CRawObject();
    Vector2D pos, dir;
    int frameCount;
    int ID;
    int cam_id;
    CMovingObject* ref;
    bool updated;
    double confidence;
    double time;
    bool merged;
    int mergeCount;
};

class CMovingObject
{
protected:
    int vanishedCounter;
    double lastInsight;
    double delayTime;
    bool kalmanEnabled;
    int lastFrameKalmanReset;
    int frameCounter;
    int stoppedFrames;
	QList<Vector2D> lastSpeeds;
	QList<double> lastAngularSpeeds;
public:    		
    CMovingObject(bool resetToZero=true);

    CRawObject observation;
    QQueue<CRawObject> hist;

    //Final Specifications
    Vector2D pos;
    Vector2D dir;
    Vector2D vel;
    Vector2D acc;
    bool shootSensor;

    double angularVel;    

    double inSight;
    double obstacleRadius;
    int cam_id;
	int lastFrameUpdated;
    bool modelObjStopped;
    Vector2D modelObjStopPos;
    Vector2D modelDir,modelCurDir;
    double modelSampleTime;
    double modelC0,modelC1,modelC2; //c2 * x^2 + c1 * x + c0
    //finding model variables
    int modelFrameCnt;
    double modelC2Sum, modelC2Ave;
    int modelC2Count;
    double modelDirC0, modelDirC1;
    Vector2D ballStopPos;
    QQueue<double> modelTimeBuffer;
    QQueue<CRawObject> modelObjBuffer;


    //Functions

    virtual void filter(int vanished);
    virtual void init();
    virtual void resetKalman();

    void updateDelayTime(double newDelayTime);

    void update(CRawObject raw);
    void update(CMovingObject* obj);
    void findModel(double dt);
    void kalmanFilter();
    Vector2D predict(double time);
    Vector2D predictV(double time);
    Vector2D whereIsAtVel(Vector2D V);
    double whenIsAtVel(double L);
};

extern double observeTimeStep;

#endif // MovingObject_H
