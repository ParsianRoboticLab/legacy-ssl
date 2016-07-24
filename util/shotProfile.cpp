#include "shotProfile.h"
#include <QDebug>

CShotProfile::CShotProfile()
{
    endFlag = true;
}
CShotProfile::~CShotProfile(){

}

void CShotProfile::execute( CAgent **agents , int id , int speed , double sampleDist )
{
    if( wm->our.ActiveAgentsCount() == 0 || wm->our[id]->inSight <= 0 )
        return;
    if( firstTimeBallPos == true ){
        firstTimeBallPos = false;
        BallPos = wm->ball->pos;
        timeStart.start();
        debug("Start " , D_MASOOD , QColor("red"));
        TValues.clear();
        RValues.clear();
        ballStopCntr = 0;
        TValues.append(0.0);
        RValues.append(0.0);
        debug(QString("%1,%2").arg(0).arg(0).arg(0) , D_MASOOD);
        draw(BallPos, 0, QColor("red"));
    }
    else
    {
        if( finish == false){
            if( kickOrChip ){
                agents[id]->setKick(true, speed);
            }
            else
                agents[id]->setChip(true, speed);
            double DIST = wm->ball->pos.dist(BallPos);
            double DIST2 = 0;
            halfworldMutex->lock();
            DIST2 = halfworld->ball.at(0)->pos.dist(BallPos);
            halfworldMutex->unlock();

            draw(wm->ball->pos, 0, QColor("blue"));
            if( wm->ball->hist.last().pos.dist(wm->ball->pos) < 0.01)
            {
                ballStopCntr++;
            }
            else
            {
                ballStopCntr=0;
            }
            debug(QString("%1,%2").arg(timeStart.elapsed()/1000.0).arg(DIST), D_MASOOD);
            TValues.append(timeStart.elapsed()/1000.0);
            RValues.append(DIST);
            if (((DIST > sampleDist ) || (wm->ball->inSight == 0) || (ballStopCntr > 10)) && (RValues.count() > 50) )
                finish = true;
        }
        else
        {
            debug("Finish" , D_MASOOD);
            int n  = TValues.count();

            double *xx = new double[n];
            double *yy = new double[n];
            double a,b,c;

            for (int i = 0; i < n; i++)
            {
                xx[i] = TValues.at(i);
                yy[i] = RValues.at(i);
            }

            squarefit(n, xx, yy, a, b, c);
            profileShot.agent[id].speed[speed].a = 2.0*c;
            profileShot.agent[id].speed[speed].v = b;
            //debug(QString("Square Fit : ACC:%1 VEL:%2 X0: %3").arg(2.0*c).arg(b).arg(a), D_EXPERIMENT | D_MASOOD, QColor("green"));
            debug(QString("Agent:%1  KickSpeed:%2  ACC: %3  VEL: %4").arg(id).arg(speed).arg(2*c).arg(b) , D_MASOOD );
            endFlag = true;
            delete [] xx;
            delete [] yy;
        }
    }
}
