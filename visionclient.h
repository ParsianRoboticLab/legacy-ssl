#ifndef VISIONCLIENT_H
#define VISIONCLIENT_H

#include <QByteArray>

#include "proto/messages_robocup_ssl_detection.pb.h"
#include "proto/messages_robocup_ssl_geometry.pb.h"
#include "proto/messages_robocup_ssl_wrapper.pb.h"
#include "proto/messages_robocup_ssl_refbox_log.pb.h"

#include "base.h"
#include "movingobject.h"


//in
#define CAMERA_NUM 8
#define OUT_OF_SIGHT_THRESHOLD 40

class QTime;
class CVisionBelief {
    public:
    QList<CRawObject> ourTeam[_MAX_NUM_PLAYERS];
    QList<CRawObject> oppTeam[_MAX_NUM_PLAYERS];
    QList<CRawObject> ball;
    double visionLatency;
    double timeStep,ltcapture;
    double time;    
    int cam_id;

    //count of frames that each object was out of sight
    int outofsight_ourTeam[_MAX_NUM_PLAYERS];
    int outofsight_oppTeam[_MAX_NUM_PLAYERS];
    int outofsight_ball;

    bool updated; //indicates that camera is ever updated or not
    int lastUpdateTime;    
    CVisionBelief();
    void reset();
};

class CVisionClient
{
public:
    QTime *vcTimer;
    ETeamColorType ourColor;
    ETeamSideType ourSide;
    CVisionBelief v[CAMERA_NUM];
//    CVisionBelief mv[CAMERA_NUM];
    CVisionBelief res;
    double sampleT;
    int lastCamera;
    int activeCameras;
	int frameCnt;

	vector<Vector2D> boundaries[CAMERA_NUM];

    CVisionClient();
    ~CVisionClient();

    void parse(SSL_WrapperPacket& packet);
	void merge(int camera_count=CAMERA_NUM);
	void countActiveCameras();

        void newVision();
};

#endif // VISIONCLIENT_H
