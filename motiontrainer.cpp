#include "motiontrainer.h"
#include <QStringList>
#include <QDebug>

CMotionTrainer::CMotionTrainer(CWorldModel* _wm,CAgent **_agents)
{
    gotopoint = new CSkillGotoPointAvoid(NULL);

    wm = _wm;
    agents = _agents;
	id = 0;
    mode = -1;
	rmax = 2;
    rmin = 0;
	x1 = -2;
	y1 = -2;
	x2 = 2;
	y2 = 2;
    counter = 0;
    lpos.invalidate();
    onlyv2zero = false;    
}

CMotionTrainer::~CMotionTrainer()
{
    delete gotopoint;
}

void CMotionTrainer::moveRobotToRandomPoints()
{
	id=0;

    bool flag=false;
    if (mode==-1) flag = true;
	if (((wm->our[id]->pos - pos).length() < 0.02)
        && fabs(AngleDeg::normalize_angle(wm->our[id]->pos.dir().degree() - pos.dir().degree())) < 20)
        flag = true;
    if (mode!=0)
    {
        if ((mid-wm->our[id]->pos).length()<0.100)
            flag = true;
    }
    counter ++;
    if (flag)
    {
        if (mode==-1)
        {
            timer.start();
        }
        else {
            TrainStruct st = CMotionEstimator::trainStructOf(lpos, lvel, wm->our[id]->pos, wm->our[id]->vel);
            st.t = timer.elapsed()/1000.0f;
            if (st.t > 0.2)
                trainlist.append(st);
            timer.restart();
        }
        mode = random()%10;
        if (onlyv2zero) mode = 0;
		float rr = random();
		rr = rr/RAND_MAX*(rmax - rmin) + rmin;
		qDebug() << rr;
		float th = random()%360-180; // ((random()%2))*180;
		pos = wm->our[id]->pos + Vector2D::unitVector(AngleDeg(th))*rr;
        if (pos.x > x2) pos.x = x2;
        if (pos.y > y2) pos.y = y2;
        if (pos.x < x1) pos.x = x1;
        if (pos.y < y1) pos.y = y1;
        pos.setDir( random()%360 - 180);
        lvel  = wm->our[id]->vel;
        lpos  = wm->our[id]->pos;
        mid  = (pos - lpos)*((double)(random()%10)/10.0) + lpos;
    }
    if (mode!=0)
    {
        draw(mid,1,QColor("cyan"));
    }
    draw(pos,1,QColor("white"));
    draw(QString("%1").arg(trainlist.size()),Vector2D(-3,2),QColor("blue"));
    gotopoint->setAgent(agents[id]);
    gotopoint->setTarget(pos, Vector2D(1,0));
	gotopoint->setAvoidPenaltyArea(false);

    //gotopoint->setFinalPos(pos);

    gotopoint->execute();
//	qDebug()<<id<<" "<<agents[id]->v1<<" "<<agents[id]->v2<<" "<<agents[id]->v3<<" "<<agents[id]->v4<<" ";
    //agents[id]->skill->goToPointAvoid(pos);
}

void CMotionTrainer::save(QString filename)
{
    if (mode!=-1)
    {
     QFile file(filename);
     if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
         return;
     QTextStream out(&file);
//	 TrainStruct smin;smin.dist.assign(10,10);smin.dist.dir() = 180;smin.vel1.assign(10,10);smin.vel2.assign(10,10);smin.t=100;
//     TrainStruct smax;smax.dist.assign(-10,-10);smax.dist.dir() = -180;smax.vel1.assign(-10,-10);smax.vel2.assign(-10,-10);smax.t=-100;
     for (int i=0;i<trainlist.size();i++)
     {
//         double* l = CMotionEstimator::normalizedStruct(trainlist[i]);
//         out << i <<
//                "," << l[0] <<
//                "," << l[1] <<
//                "," << l[2] <<
//                "," << l[3] <<
//                "," << l[4] <<
//                "," << l[5] <<
//                "," << l[6] <<
//                "," << l[7] << "\n";
//         delete l;
		 out << i <<
				"," << trainlist[i].dist <<
				"," << trainlist[i].vel1.length() <<
//		        "," << trainlist[i].vel1.x <<
//		        "," << trainlist[i].vel1.y <<
				"," << trainlist[i].vel2.length() <<
//		        "," << trainlist[i].vel2.x <<
//		        "," << trainlist[i].vel2.y <<
				"," << trainlist[i].theta <<
				"," << trainlist[i].t << "\n";

     }
     file.close();
    }
}

//---------------------------

CBallTrainer::CBallTrainer(CWorldModel* _wm, CAgent **_agents)
{
    wm = _wm;
    agents = _agents;
    tt = 0;
    timer.start();
    num = 0;
}

void CBallTrainer::save(QString filename)
{
     QFile file(filename);
     if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
         return;
     QTextStream out(&file);
     for (int i=0;i<trainlist.size();i++)
     {
         if (isViaSim)
         {
            out << trainlist[i].num <<
					"," << trainlist[i].x <<
                    "," << trainlist[i].v <<
                    "," << trainlist[i].a <<
                    "," << trainlist[i].t << "\n";
         }
         else
			 out << trainlist[i].x <<
                     "," << trainlist[i].v <<
                     "," << trainlist[i].a <<
                     "," << trainlist[i].t << "\n";
     }
}

void CBallTrainer::record()
{
    BallTrainStruct sample;
    sample.num = num;
    sample.t = timer.elapsed() / 1000.0f;
    sample.x = (wm->ball->pos - pos0).length();
    sample.v = (wm->ball->pos - pos0).norm().innerProduct(wm->ball->vel);
    sample.a = (wm->ball->pos - pos0).norm().innerProduct(wm->ball->acc);
    trainlist.append(sample);
    draw(QString("%1:%2").arg(num).arg(sample.x),Vector2D(0,-700),QColor("blue"));
    if ((sample.x > 5000) || (fabs(sample.v)<10))
    {
        tt = -1;
    }
    isViaSim = false;
}

void CBallTrainer::reset(Vector2D p)
{
    pos0 = p;
    trainlist.clear();
    timer.restart();
}
