#include "callibration.h"

CKickCallibrator::CKickCallibrator()
{
    kick = new CSkillKick(NULL);
    gotopoint = new CSkillGotoPointAvoid(NULL);
    kicking = 0;
    kickSpeedCalculated = false;
}

void CKickCallibrator::calcKickSpeed()
{
    if (kickSpeedCalculated) return;
    double vball = wm->ball->vel.length();
    if (vball > 0.2)
    {
        if (ballhist.count() == 0)
        {
            kickStartPoint = wm->ball->pos;
        }
        BallDistVel b;
        b.dist = (wm->ball->pos - kickStartPoint).length();
        b.vel  = vball;
        ballhist.append(b);
    }
    double s = (wm->ball->pos - kickPoint).innerProduct(wm->ball->pos - target);
    if (vball < 0.2 || s > 0)
    {
        if (ballhist.count() > 10)
        {
            for (int i=0;i<ballhist.count()*3/4;i++)
            {
                double v2s = 0.0;
                double vs = 0.0;
                for (int j=i+1;j<ballhist.count();j++)
                {
                    double v = sqrt (ballhist[j].vel*ballhist[j].vel + 2.0 * ballhist[j].dist * BallFriction() * Gravity);
                    v2s += v*v;
                    vs  += v;
                }
                double n = ballhist.count() - (i + 1);
                double e = (vs / n);
                double s = v2s / n - e * e;
                kickspeed = e;
                if (s < 0.05)
                {
                    break;
                }
            }
            kickSpeedCalculated = true;
            debug(QString("Kick speed: %1 ;n = %2").arg(kickspeed).arg(ballhist.count()), D_ALI);
            ballhist.clear();
        }
    }
    draw(kickStartPoint, 1, "red");
}

void CKickCallibrator::execute(int speed, bool chip, bool spin)
{
    kick->setAgent(agent);
    gotopoint->setAgent(agent);
    if (kicking == 0)
    {
        gotopoint->setTarget(kickPoint, (target - kickPoint).norm());
        gotopoint->execute();
        if (((wm->ball->pos - agent->pos()).length() < 0.3) && (wm->ball->vel.length() < 0.2))
        {
            kicking = 1;
        }
    }
    if (kicking == 1)
    {
        kick->setTarget(target);
        kick->setKickSpeed(speed);
        kick->setChip(chip);
        kick->setSlow(true);
        kick->setSpin(spin);
        kick->setInterceptMode(false);
        kick->execute();
        if (wm->ball->vel.length() > 0.2)
        {
            kicking = 2;
            kickSpeedCalculated = false;
            ballhist.clear();
        }
    }
    if (kicking == 2)
    {
        agent->waitHere();
        calcKickSpeed();
        if (kickSpeedCalculated)
        {
            kicking = 0;
        }
    }
}

CKickCallibratorManiMasoud::CKickCallibratorManiMasoud()
{
    kick = new CSkillKick(NULL);
    gotopoint = new CSkillGotoPointAvoid(NULL);
    kickSpeedCalculated = false;
}


CKickCallibratorManiMasoud::~CKickCallibratorManiMasoud()
{

}

void CKickCallibratorManiMasoud::execute(int speed, bool chip, bool spin)
{
return;
    kickSpeedCalculated = false;/*

    if(agent->pos().dist(kickPoint)>0.3 || Vector2D::angleBetween(agent->dir(),target-kickPoint).degree()>20)
    {
        gotopoint->setAgent(agent);
        gotopoint->setTarget(kickPoint, (target - kickPoint).norm());
        gotopoint->execute();
        firstTimeBallPos = true;
        finish = false;
        return;
    }*/

    if( firstTimeBallPos == true ){
        draw(QString("HI"),Vector2D(0.0,0.0),QColor("blue"),36);
        firstTimeBallPos = false;
        BallPos = wm->ball->pos;
        timeStart.start();
        debug("Start " , D_MASOOD , QColor("red"));
        Values.clear();
        ballStopCntr = 0;
        Values.append(qMakePair(0.0,0.0));
        draw(BallPos, 0, QColor("red"));
        finish=false;
    }
    else
    {
        draw(BallPos, 0, QColor("red"));
        if( finish == false){
            if( !chip ){
                agent->setKick(speed);
            }
            else
            {
                agent->setChip(speed);
            }
            if(spin)
            {
                agent->setRoller(5);
            }else
            {
                agent->setRoller(0);
            }
            double DIST = wm->ball->pos.dist(BallPos);
//            draw(wm->ball->pos, 0, QColor("blue"));
            if(wm->ball->hist.count()!=0 && wm->ball->hist.last().pos.dist(wm->ball->pos) < 0.01)
            {
                ballStopCntr++;
            }
            else
            {
                ballStopCntr=0;
            }
//            debug(QString("%1,%2").arg(timeStart.elapsed()/1000.0).arg(DIST), D_MASOOD);
            if((DIST>wm->ball->radius)&&(DIST!=Values.last().second))
                Values.append(qMakePair(timeStart.elapsed()/1000.0,DIST));
            if (((DIST > 2.0 /*(target-kickPoint).length()*/) || (wm->ball->inSight == 0) || (ballStopCntr > 5)) && (Values.count()>10))
            {
//                qDebug()<<wm->ball->pos.x<<wm->ball->pos.y;
                finish = true;
            }
        }
        else
        {
            debug("Finish" , D_MASOOD);
            kickSpeedCalculated = true;

            int n  = Values.count();

            double *xx = new double[n];
            double *yy = new double[n];
            double a,b,c;

            for (int i = 0; i < n; i++)
            {
                xx[i] = Values[i].first;
                yy[i] = Values[i].second;
                debug(QString::number(i)+","+QString::number(xx[i],'f',3)+","+QString::number(yy[i],'f',3),D_ARASH);
            }

            squarefit(n, xx, yy, a, b, c);
            debug(QString(" Mani : KickSpeed:%1  ACC: %2  VEL: %3 Samples Count:%4").arg(speed).arg(2*c).arg(b).arg(n) , D_MASOOD );
////            qDebug()<<a<<b<<c;
            CPolynomialRegression shotModel;
            shotModel.fitToDataSet(Values,1);
            QList<double> coefs = shotModel.getCoefs();
//            a=coefs[];
//            b=coefs[1];
//            c=coefs[0];
            b=coefs[1];
//            qDebug()<<a<<b<<c;
            //debug(QString("Square Fit : ACC:%1 VEL:%2 X0: %3").arg(2.0*c).arg(b).arg(a), D_EXPERIMENT | D_MASOOD, QColor("green"));
            debug(QString(" KickSpeed:%1  VEL: %3 Samples Count:%4").arg(speed).arg(b).arg(n) , D_MASOOD );
            kickspeed = b;
            endFlag = true;
            delete [] xx;
            delete [] yy;
        }
    }

}
