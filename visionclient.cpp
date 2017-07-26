#include "visionclient.h"
#include "proto/messages_robocup_ssl_detection.pb.h"
#include "proto/messages_robocup_ssl_geometry.pb.h"
#include "proto/messages_robocup_ssl_wrapper.pb.h"
#include "proto/messages_robocup_ssl_refbox_log.pb.h"


#include <QDebug>
#include <QTime>
#include <logger.h>
#include <varswidget.h>
#include <ostream>
#include <fstream>
#include <mathtools.h>


CVisionBelief::CVisionBelief()
{
    reset();
    updated = false;
}

void CVisionBelief::reset()
{
    timeStep = 0;
    visionLatency = 0;
    for(int i=0;i<_MAX_NUM_PLAYERS;i++){
        ourTeam[i].clear();
        outofsight_ourTeam[i]=0;
        oppTeam[i].clear();
        outofsight_oppTeam[i]=0;
    }
    ball.clear();
    outofsight_ball=0;
}

CVisionClient::CVisionClient()
{
    ourColor = _COLOR_BLUE;
    vcTimer = new QTime();
    vcTimer->start();
    lastCamera = -1;
    activeCameras = 1;
        frameCnt = 0;
}

CVisionClient::~CVisionClient()
{
    delete vcTimer;
}
#define MAX_OBJECT 5
#define __RECIEVE_ROBOTS_DATA(__COLOR__,__TEAM__)\
for (int i=0;i<packet.detection().robots_##__COLOR__##_size();i++) \
{ \
  int rob_id = packet.detection().robots_##__COLOR__(i).robot_id(); \
  if (v[id].__TEAM__##Team[rob_id].count()>=MAX_OBJECT) continue; \
  if ((packet.detection().robots_##__COLOR__(i).has_orientation())&&(packet.detection().robots_##__COLOR__(i).has_confidence())){ \
        CRawObject raw = CRawObject(frameCnt, Vector2D(packet.detection().robots_##__COLOR__(i).x()*ourTeamSide / 1000.0f,packet.detection().robots_##__COLOR__(i).y()*ourTeamSide / 1000.0f), \
                                                                                                                 packet.detection().robots_##__COLOR__(i).orientation()*180.0f/M_PI+(1.0-ourTeamSide)*90.0 \
                                                                                                                 ,i,packet.detection().robots_##__COLOR__(i).confidence(),NULL,id); \
        for (int k=0;k<v[id].__TEAM__##Team[rob_id].count();k++) \
        { \
                if ((v[id].__TEAM__##Team[rob_id][k].pos - raw.pos).length() < 0.5) \
                { \
                        v[id].__TEAM__##Team[rob_id].removeAt(k); \
                } \
        }	\
        v[id].__TEAM__##Team[rob_id].append(raw); \
    v[id].outofsight_##__TEAM__##Team[rob_id] = 0; \
    __TEAM__##_insight[rob_id] = true; \
 } \
}

void CVisionClient::parse(SSL_WrapperPacket& packet)
{
    //lastCamera = -1;
    float ourTeamSide=(ourSide==_SIDE_RIGHT)? -1.0f : 1.0f;
    if (packet.has_detection())
    {
        if(!conf()->BallTracker_cam1on()){
                if (packet.detection().camera_id()==0) return;
        }
        if(!conf()->BallTracker_cam2on()){
                if (packet.detection().camera_id()==1) return;
        }
        if(!conf()->BallTracker_cam3on()){
                if (packet.detection().camera_id()==2) return;
        }
        if(!conf()->BallTracker_cam4on()){
                if (packet.detection().camera_id()==3) return;
        }


        frameCnt ++;
        int id = packet.detection().camera_id();
        lastCamera = id;
        v[id].cam_id = id;
                for (int i=0;i<v[id].ball.count();i++)
                {
                        if (frameCnt - v[id].ball[i].frameCount > 2)
                        {
                                v[id].ball.removeAt(i);
                        }
                }
                for (int t=0;t<_MAX_NUM_PLAYERS;t++)
                {
                        for (int i=0;i<v[id].ourTeam[t].count();i++)
                        {
                                if (frameCnt - v[id].ourTeam[t][i].frameCount > 15)
                                {
                                        v[id].ourTeam[t].removeAt(i);
                                }
                        }
                        for (int i=0;i<v[id].oppTeam[t].count();i++)
                        {
                                if (frameCnt - v[id].oppTeam[t][i].frameCount > 15)
                                {
                                        v[id].oppTeam[t].removeAt(i);
                                }
                        }
                }
/*
                v[id].ball.clear();
        for(int i=0;i<_MAX_NUM_PLAYERS;i++){
            v[id].ourTeam[i].clear();
            v[id].oppTeam[i].clear();
                }*/
        v[id].lastUpdateTime = vcTimer->elapsed();
        double t = packet.detection().t_capture();
        double dt =  t - v[id].ltcapture;
        if (dt < 0.0) dt = 0.05;
        v[id].time = t;
        v[id].timeStep = dt;
        v[id].ltcapture = t;
        v[id].visionLatency = (packet.detection().t_sent()-packet.detection().t_capture());
        v[id].updated = true;
        for (int i=0;i<min(MAX_OBJECT,packet.detection().balls_size());i++)
        {
            if (packet.detection().balls(i).has_confidence() &&
                packet.detection().balls(i).has_x() &&
                packet.detection().balls(i).has_y())
            {
                                CRawObject raw = CRawObject(frameCnt, Vector2D(packet.detection().balls(i).x()*ourTeamSide,packet.detection().balls(i).y()*ourTeamSide)/1000.0f,0,i,packet.detection().balls(i).confidence(),NULL,id);
                                for (int k=0;k<v[id].ball.count();k++)
                                {
                                        if ((v[id].ball[k].pos - raw.pos).length() < 0.5)
                                        {
                                                v[id].ball.removeAt(k);
                                        }
                                }
                                v[id].ball.append(raw);
            }
        }
        if(packet.detection().balls_size()>0)
            v[id].outofsight_ball=0;

        bool our_insight[_MAX_NUM_PLAYERS];
        bool opp_insight[_MAX_NUM_PLAYERS];
        for (int i=0;i<_MAX_NUM_PLAYERS;i++) {our_insight[i]=opp_insight[i]=false;}

        if (ourColor==_COLOR_BLUE)
        {
            __RECIEVE_ROBOTS_DATA(blue,our);
            __RECIEVE_ROBOTS_DATA(yellow,opp);
        }
        else if (ourColor==_COLOR_YELLOW)
        {
            __RECIEVE_ROBOTS_DATA(yellow,our);
            __RECIEVE_ROBOTS_DATA(blue,opp);
        }
        for (int i=0;i<_MAX_NUM_PLAYERS;i++) {
            if (!our_insight[i]) {
                v[id].outofsight_ourTeam[i]++;
            }
            if (!opp_insight[i]) {
                v[id].outofsight_oppTeam[i]++;
            }
        }
    }
}

inline float inSightReduce(float v,int n)
{
    if (n>0) return v/((float) n*n);
    else return v;
}

void CVisionClient::countActiveCameras()
{
    if(!conf()->BallTracker_cam1on()){
        v[0].updated = false;
    }
    if(!conf()->BallTracker_cam2on()){
        v[1].updated = false;
    }
    if(!conf()->BallTracker_cam3on()){
        v[2].updated = false;
    }
    if(!conf()->BallTracker_cam4on()){
        v[3].updated = false;
    }

    int now = vcTimer->elapsed();
    for (int i=0;i<CAMERA_NUM;i++)
    {
        if (now - v[i].lastUpdateTime>100){
            v[i].updated=false;
        }
    }
    activeCameras = 0;
    for (int i=0;i<CAMERA_NUM;i++)
    {
        if (v[i].updated)
        {
            activeCameras ++;
        }
    }
}


///////////////////////////////////////////////
void CVisionClient::newVision()
{

}

///////////////////////////////////////////

void CVisionClient::merge(int camera_count)
{

    res.reset();
    res.time = 0.0;
    res.timeStep = 0;
    res.ltcapture = 0;
    for (int i=0;i<camera_count;i++)
    {
        res.time += v[i].time;
        res.timeStep += v[i].timeStep;
        res.ltcapture += v[i].ltcapture;
        res.visionLatency += v[i].visionLatency;
        //Match between res and v[i]
        for (int j=0;j<v[i].ball.count();j++)
        {
            //if (v[i].ball[j].confidence <= 0) continue;
            int best = -1;
            double dist = 0;
            double minDist = 0;
            for (int k=0;k<res.ball.count();k++)
            {
                dist = (v[i].ball[j].pos - res.ball[k].pos).length();
                if ((dist < minDist) || (best == -1))
                {
                    minDist = dist;
                    best = k;
                }
            }
            if ((best == -1) || (minDist > 0.5))
            {
                res.ball.append(v[i].ball[j]);
                res.ball.last().mergeCount = 0;
            }
            else {
                res.ball[best].pos += v[i].ball[j].pos;
                res.ball[best].mergeCount ++;
            }
        }

        for (int t=0;t<_MAX_NUM_PLAYERS;t++)
        {
            for (int j=0;j<v[i].ourTeam[t].count();j++)
            {
                //if (v[i].ourTeam[t][j].confidence <= 0) continue;
                int best = -1;
                double dist = 0;
                double minDist = 0;
                for (int k=0;k<res.ourTeam[t].count();k++)
                {
                    dist = (v[i].ourTeam[t][j].pos - res.ourTeam[t][k].pos).length();
                    if ((dist < minDist) || (best == -1))
                    {
                        minDist = dist;
                        best = k;
                    }
                }
                if ((best == -1) || (minDist > 0.5))
                {
                    res.ourTeam[t].append(v[i].ourTeam[t][j]);
                    res.ourTeam[t].last().mergeCount = 0;
                }
                else {
                    res.ourTeam[t][best].pos += v[i].ourTeam[t][j].pos;
                    res.ourTeam[t][best].mergeCount ++;
                }
            }
        }
        for (int t=0;t<_MAX_NUM_PLAYERS;t++)
        {
            for (int j=0;j<v[i].oppTeam[t].count();j++)
            {
                //if (v[i].oppTeam[t][j].confidence <= 0) continue;
                int best = -1;
                double dist = 0;
                double minDist = 0;
                for (int k=0;k<res.oppTeam[t].count();k++)
                {
                    dist = (v[i].oppTeam[t][j].pos - res.oppTeam[t][k].pos).length();
                    if ((dist < minDist) || (best == -1))
                    {
                        minDist = dist;
                        best = k;
                    }
                }
                if ((best == -1) || (minDist > 0.5))
                {
                    res.oppTeam[t].append(v[i].oppTeam[t][j]);
                    res.oppTeam[t].last().mergeCount = 0;
                }
                else {
                    res.oppTeam[t][best].pos += v[i].oppTeam[t][j].pos;
                    res.oppTeam[t][best].mergeCount ++;
                }
            }
        }
    }
    for (int k=0;k<res.ball.count();k++)
        res.ball[k].pos /= (float) (res.ball[k].mergeCount+1);
    for (int t=0;t<_MAX_NUM_PLAYERS;t++)
    {
        for (int k=0;k<res.ourTeam[t].count();k++)
            res.ourTeam[t][k].pos /= (float) (res.ourTeam[t][k].mergeCount+1);
        for (int k=0;k<res.oppTeam[t].count();k++)
            res.oppTeam[t][k].pos /= (float) (res.oppTeam[t][k].mergeCount+1);
    }

    res.time /= camera_count;
    res.timeStep /= camera_count;
    res.ltcapture /= camera_count;
    res.visionLatency /= camera_count;

}

