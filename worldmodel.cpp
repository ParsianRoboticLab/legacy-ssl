#include <QList>

#include "worldmodel.h"
#include <QDateTime>
#include <QDebug>

CWorldModel *wm;
CHalfWorld* halfworld;
QMutex* halfworldMutex;


CWorldModel::CWorldModel(bool noKalman) : our(true, noKalman) , opp(false, noKalman)
{
    for( int i=0 ; i<_MAX_NUM_PLAYERS+2 ; i++ )
        oppSupporterHist[i].clear();
    ball = new CBall(noKalman);
    ball->init();
    for( int i = 0; i < _MAX_NUM_PLAYERS; i++ )
    {
        our[i]->init();
        our[i]->inOurTeam = true;
        our[i]->setBall(ball);
        opp[i]->init();
        opp[i]->inOurTeam = false;
        opp[i]->setBall(ball);
    }


    gs = new GameState();
    field = new CField();
    profiler = new CProfiler();

    setTeamSide(_SIDE_LEFT);
    setTeamColor(_COLOR_YELLOW);

    refCommand.clear();
    commandTimeStep = 0.05;
    _gl_enabled = true;
    _ode_dt = -1;
    _fps_desired = -1;
    visionOn = true;

//    statAnal = new CStatisticalAnalysis();
//    initStatAnalMani();
    visionLatency = visionTimeStep = 0.0;
    visionFPS = 60.0;
    isSimulMode = false;

    //autoRef = new CAutomatedReferee();
}

CWorldModel::~CWorldModel()
{
        delete ball;
        delete gs;
        delete field;
        delete profiler;
//    delete statAnal;
    //delete autoRef;
}


void CWorldModel::setTeamColor(ETeamColorType _teamColor)
{
    teamColor = _teamColor;
}

ETeamColorType CWorldModel::getTeamColor()
{
    return teamColor;
}

ETeamSideType CWorldModel::getTeamSide()
{
    return teamSide;
}

void CWorldModel::setTeamSide(ETeamSideType _teamSide)
{
    teamSide = _teamSide;
}

QColor CWorldModel::getTeamQColor()
{
    return QColor( (teamColor==_COLOR_YELLOW)? "yellow" : "blue" );
}

QColor CWorldModel::getOppQColor()
{
    return QColor( (teamColor!=_COLOR_YELLOW)? "yellow" : "blue" );
}

CRobot *CWorldModel::getFastestPlayerToPoint(QQueue<int> ids , TeamType teamType, Vector2D target){
        CRobot *res = NULL;

        if( teamType == OURTEAM ){
                double minDist = 1000;

                for( int i=0 ; i<ids.count() ; i++ ){
                        int id = ids.at(i);
                        double d = wm->our[id]->pos.dist(target);
                        if( minDist > d ){
                                res = wm->our[id];
                                minDist = d;
                        }
                }
        }
        else if( teamType == OPPTEAM ){
                double minDist = 1000;

                for( int i=0 ; i<ids.count() ; i++ ){
                        int id = ids.at(i);
                        double d = wm->opp[id]->pos.dist(target);
                        if( minDist > d ){
                                res = wm->opp[id];
                                minDist = d;
                        }
                }
        }
        else if( teamType == BOTHTEAMS ){
                double minDist = 1000;

                for( int i=0 ; i<ids.count() ; i++ ){
                        int id = ids.at(i);
                        double d = wm->our[id]->pos.dist(target);
                        if( minDist > d ){
                                res = wm->our[id];
                                minDist = d;
                        }
                }

                for( int i=0 ; i<ids.count() ; i++ ){
                        int id = ids.at(i);
                        double d = wm->opp[id]->pos.dist(target);
                        if( minDist > d ){
                                res = wm->opp[id];
                                minDist = d;
                        }
                }
        }

        return res;
}

void CWorldModel::update(CHalfWorld* w0)
{
    w.update(w0);
    if (w.ball.count()>0) {
        ball->update(w.ball[0]);
    }
    else ball->inSight = 0.0;
    for (int i = 0; i < _MAX_NUM_PLAYERS; i++)
    {
        if (w.ourTeam[i].count()>0) {
            our[i]->update(w.ourTeam[i][0]);
        }
        else our[i]->inSight = 0.0;
        if (w.oppTeam[i].count()>0) {
            opp[i]->update(w.oppTeam[i][0]);
        }
        else opp[i]->inSight = 0.0;
    }
        for (int i=0;i<_MAX_NUM_PLAYERS;i++)
        {
                oppRole[i] = w0->oppRole[i];
                ourRole[i] = w0->ourRole[i];
        }
    our.update();
    opp.update();
}

void CWorldModel::update(CVisionBelief res)
{
    if (!visionOn) return; //we have our local vision now

    visionLatency  = res.visionLatency;
    visionTimeStep = res.timeStep;
    observeTimeStep = res.timeStep;
//    ball->update(res.ball);
//
//    for (int i = 0; i < _MAX_NUM_PLAYERS; i++)
//    {
//        our[i]->update(res.ourTeam[i]);
//        opp[i]->update(res.oppTeam[i]);
//    }
//
//    our.update();
//    opp.update();
//
//    Rect2D fieldRect(this->field->ourCornerR() - Vector2D(0.300, 0.300) , this->field->oppCornerL() + Vector2D(0.300, 0.300));
//    if( ! fieldRect.contains(ball->pos) && ball->inSight )
//    {
//        ball->inSight = -1.0;
//    }
    //TODO: Maybe need to check robots oout of field
}

void CWorldModel::update(CTeam& _our, CTeam& _opp, CBall& _ball, double _visionLatency, double _timeStep)
{
    if (!visionOn) return; //we have our local vision now
    visionLatency  = _visionLatency;
    visionTimeStep = _timeStep;
    observeTimeStep = _timeStep;
    Rect2D fieldRect(this->field->ourCornerR() - Vector2D(0.300, 0.300) , this->field->oppCornerL() + Vector2D(0.300, 0.300));
    for (int i=0;i<_NUM_PLAYERS;i++)
    {
        our[i]->acc = _our[i]->acc;
        our[i]->vel = _our[i]->vel;
        our[i]->pos = _our[i]->pos;
        our[i]->dir = _our[i]->dir;
        our[i]->angularVel = _our[i]->angularVel;
        our[i]->inSight = _our[i]->inSight;
        opp[i]->acc = _opp[i]->acc;
        opp[i]->vel = _opp[i]->vel;
        opp[i]->pos = _opp[i]->pos;
        opp[i]->dir = _opp[i]->dir;
        opp[i]->angularVel = _opp[i]->angularVel;
        opp[i]->inSight = _opp[i]->inSight;
    }
    ball->acc = _ball.acc;
    ball->vel = _ball.vel;
    ball->pos = _ball.pos;
    ball->inSight = _ball.inSight;
    our.update();
    opp.update();
    if( ! fieldRect.contains(ball->pos) && ball->inSight )
    {
        ball->inSight = -1.0;
    }
}

void CWorldModel::updateFromString(std::string s,float logVersion)
{
    int offset = 0;
    static int lineNumber=0;
    if (logVersion == 1.0)
    {
        offset = 20;
        int num;
        for (int i=0;i<5;i++)
                our[i]->inSight = -1;
        for (int i=0;i<5;i++)
        {
                if (s[offset] !='-')
                        sscanf(((char*)(s.c_str())) + offset, "%2d,%lf,%lf,%lf,%lf,%lf,%lf,%lf;",&num,&our[num]->pos.x,&our[num]->pos.y,&our[num]->vel.x,&our[num]->vel.y,&our[num]->dir.x,&our[num]->dir.y,&our[num]->inSight);
                offset += 58;
        }
        for (int i=0;i<5;i++)
                opp[i]->inSight = -1;
        for (int i=0;i<5;i++)
        {
                if (s[offset] !='-')
                        sscanf(((char*)(s.c_str())) + offset, "%2d,%lf,%lf,%lf,%lf,%lf,%lf,%lf;",&num,&opp[num]->pos.x,&opp[num]->pos.y,&opp[num]->vel.x,&opp[num]->vel.y,&opp[num]->dir.x,&opp[num]->dir.y,&opp[num]->inSight);
                offset += 58;
        }
                sscanf(((char*)(s.c_str())) + offset, "%lf,%lf,%lf,%lf,%lf;%d,%x;%2d,%2d;",&ball->pos.x,&ball->pos.y,&ball->vel.x,&ball->vel.y,&ball->inSight,&gs->color,&gs->state,&gs->yellowscore,&gs->bluescore);
    }
}

void CWorldModel::recvByte(char byte)
{
        int id = byte & 7;
        if (id>=0 && id<_NUM_PLAYERS)
        {
                our[id]->recvData(byte);
        }
}

Vector2D CWorldModel::whereBallFalls(Vector2D /*ballPos0*/, int /*speed*/, Vector2D /*dir*/)
{
    //TODO : write this function
    return Vector2D::INVALIDATED;
}

void CWorldModel::setSimulatorOpenGLState(bool gl_enabled)
{
    _gl_enabled = gl_enabled;
}

void CWorldModel::setSimulatorOpenDETimeStep(double ode_dt)
{
    _ode_dt = ode_dt;
}

void CWorldModel::setSimulatorDesiredFPS(int fps_desired)
{
    _fps_desired = fps_desired;
}

int CWorldModel::getSimulatorStateBuffer(char* buf)
{
    buf[0] = 66;
    if (!_gl_enabled)
        buf[1] = 10;
    else buf[1] = 100;
    float x;
    x = _ode_dt;
    buf[2] = *((char*) (& (x)) );
    buf[3] = *((char*) (& (x)) + 1);
    buf[4] = *((char*) (& (x)) + 2);
    buf[5] = *((char*) (& (x)) + 3);
    int f;
    f = _fps_desired;
    buf[6] = *((char*) (& (f)) );
    buf[7] = *((char*) (& (f)) + 1);
    buf[8] = *((char*) (& (f)) + 2);
    buf[9] = *((char*) (& (f)) + 3);
    _fps_desired = -1;
    _ode_dt = -1;
    return 10;
}


double CWorldModel::commandSampleTime()
{
    return commandTimeStep;
}


void CWorldModel::setCommandTimeStep(double step)
{
    commandTimeStep = step*0.2 + commandTimeStep * 0.8;
}

void CWorldModel::generateObstacleSpace(CObstacles &obs, QList<int> &ourRelaxList, QList<int> &oppRelaxList, bool avoidPenaltyArea, bool avoidCenterCircle , double ballObstacleRadius)
{
        obs.clear();

        for (int j=0;j<our.activeAgentsCount();j++)
        {
        if( ourRelaxList.contains(our.active(j)->id) == false )
                {
                        double obstVelFactor = 0.06;
                        for(double vvv = 0; vvv <= our.active(j)->vel.length(); vvv+=0.5)
                        {
                                double ttt = 1;
                                if (our.active(j)->vel.length() >= 0.5 )
                                        ttt = ((our.active(j)->vel.length()-vvv)/our.active(j)->vel.length());
                                obs.add_circle(our.active(j)->pos.x+our.active(j)->vel.x*vvv*obstVelFactor , our.active(j)->pos.y+our.active(j)->vel.y*vvv*obstVelFactor , (our.active(j)->robotRadius()+0.03)*ttt , our.active(j)->vel.x , our.active(j)->vel.y);
                        }
                }
        }

        for (int j=0;j<opp.activeAgentsCount();j++)
        {
        if( oppRelaxList.contains(opp.active(j)->id) == false )
                {

                        double obstVelFactor = 0.06;
                        for(double vvv = 0; vvv <= opp.active(j)->vel.length(); vvv+=0.5)
                        {
                                double ttt = 1;
                                if (opp.active(j)->vel.length() >= 0.5 )
                                        ttt = ((opp.active(j)->vel.length()-vvv)/opp.active(j)->vel.length());
                                obs.add_circle(opp.active(j)->pos.x+opp.active(j)->vel.x*vvv*obstVelFactor , opp.active(j)->pos.y+opp.active(j)->vel.y*vvv*obstVelFactor , (opp.active(j)->robotRadius()+0.03)*ttt , opp.active(j)->vel.x , opp.active(j)->vel.y);
                        }

//			obs.add_circle(opp.active(j)->pos.x , opp.active(j)->pos.y , opp.active(j)->robotRadius()+0.03 , opp.active(j)->vel.x , opp.active(j)->vel.y);
                }
        }

    if( ballObstacleRadius > EPSILON )
        obs.add_circle(ball->pos.x,ball->pos.y,ballObstacleRadius,ball->vel.x , ball->vel.y);

        if( avoidPenaltyArea ){
//		obs.add_circle(field->ourGoal().x , field->ourGoal().y+0.15 , 0.800+CRobot::robot_radius_new , 0 , 0);
//		obs.add_circle(field->ourGoal().x , field->ourGoal().y-0.15 , 0.800+CRobot::robot_radius_new , 0 , 0);
                obs.add_circle(-3.20 , 0 , 1.1 , 0 , 0);
        }

        if (avoidCenterCircle)
        {
                obs.add_circle(0 , 0 , 0.5 , 0 , 0);
        }
}

void CWorldModel::initStatAnalMani()
{

}


CHalfWorld::CHalfWorld()
{
    c = new CVisionBelief();
    currentFrame = 0;
    playmakerID = -1;
}

bool dbg = false;


#if 1
//updates p0 by p
void CHalfWorld::track(QList<CRawObject>&p0, QList<CRawObject>&p)
{
    for (int i=0;i<p0.count();i++)
    {
        p0[i].updated = false;
    }
    //p0 => old data
    //p  => new data
    QList<QList<int> > subs;
    QList<int> u;
    int flag = 0;
    if (p0.count() == 0)
    {
        for (int k=0;k<p.count();k++)
        {
            p0.append(p[k]);
            p0.back().updated = true;
        }
        return;
    }
    if (p.count() > p0.count()) //more objects this frame
    {
        for (int k=0;k<p.count();k++)
        {
            u.append(k);
        }
        subs = generateSubsets(u, p0.count());
        flag = 1;
    }
    else //less or equal objects this frame
    {
        for (int k=0;k<p0.count();k++)
        {
            u.append(k);
        }
        subs = generateSubsets(u, p.count());
        flag = 2;
    }
    double bestDist = 0.0;
    QList<int> bestComb;
    bool firsttime = true;
    for (int k=0;k<subs.count();k++)
    {
        QList<QList<int> > comb = generateCombinations(subs[k]);
        for (int i=0;i<comb.count();i++)
        {
            double d = 0;
            if (flag==1)
            {
                for (int j=0;j<p0.count();j++)
                    d += (p0[j].pos - p[comb[i][j]].pos).r2();
            }
            else {
                for (int j=0;j<p.count();j++)
                    d += (p[j].pos - p0[comb[i][j]].pos).r2();
            }
            if ((firsttime) || (d<bestDist))
            {
                bestDist = d;
                bestComb = comb[i];
                firsttime = false;
            }
        }
    }
    //debug(QString("subs cnt=%1;p0=%2;p=%3;dist=%4").arg(subs.count()).arg(p0.count()).arg(p.count()).arg(bestDist), D_ALI);
//    QString ss;
//    ss = QString("l=");

    if (!firsttime)//there is a combination
    {
        if (flag == 1)
        {
            bool *updated = new bool [p.count()];
            for (int k=0;k<p.count();k++) updated[k] = false;
            for (int k=0;k<bestComb.count();k++)
            {
                //if ((p[bestComb[k]].pos - p0[k].pos).length() < 0.5)
                {
                    p0[k] = p[bestComb[k]];
                    p0[k].updated = true;
                    updated[bestComb[k]] = true;
                }
            }
            for (int k=0;k<p.count();k++)
            {
                if (updated[k] == false)
                {
                    p0.append(p[k]);
                    p0.back().updated = true;
                }
            }
            delete [] updated;
        }
        else {
            for (int k=0;k<bestComb.count();k++)
            {
                if (true) //(p[k].pos-p0[bestComb[k]].pos).length() < 0.5)
                //if ((p[k].pos-p0[bestComb[k]].pos).length() < 0.5)
                {
                    p0[bestComb[k]] = p[k];
                    p0[bestComb[k]].updated = true;
                }
                else {
                    p0.append(p[k]);
                    p0.back().updated = true;
                }
            }
        }
//        if (dbg)
//            qDebug() << "naridi";
    }
    else {
//        if (dbg)
//            qDebug() << "ridi: subs= " << subs.count() << "p0.count=" << p0.count() << "p.count=" << p.count() << " flag=" << flag;
    }
}

#else

void CHalfWorld::track(QList<CRawObject>&p0, QList<CRawObject>&p)
{
    for (int i=0;i<p0.count();i++)
    {
        p0[i].updated = false;
    }
    QList<int> l,u;
    int s=p0.count()-p.count();
    if (s<0) s=0;
    double bestDist = 0.0;
    bool flag = false;
    QList<int> bestComb;
    for (int k=0;k<p0.count();k++)
    {
        u.append(k);
    }
    for (int k=p0.count();k<p.count();k++)
    {
        u.append(p0.count());
    }
    QList<QList<int> > subs = generateSubsets(u, p.count());
    for (int k=0;k<subs.count();k++)
    {
        QList<QList<int> > comb = generateCombinations(subs[k]);
        for (int i=0;i<comb.count();i++)
        {
            double d = 0;
            for (int j=0;j<p.count();j++)
            {
                if (comb[i][j] < p0.count())
                {
                    d += (p[j].pos - p0[comb[i][j]].pos).r2();
                }
            }
            if ((!flag) || (d<bestDist))
            {
                bestDist = d;
                bestComb = comb[i];
                flag = true;
            }
        }
    }
    //    QString ss;
    //    ss = QString("l=");
    if (flag)
    {
        for (int k=0;k<bestComb.count();k++)
        {
            if (bestComb[k]>=p0.count())
            {
                p0.append(p[k]);
                p0.back().updated = true;
            }
            else {
                p0[bestComb[k]] = p[k];
                p0[bestComb[k]].updated = true;
            }
            //            ss = QString("%1 %2").arg(ss).arg(bestComb[k]);
        }
        //        qDebug() << ss;
    }
}

#endif

void CHalfWorld::vanishOutOfSights()
{
    if (ball.count()>0)
    {
        if (ball[0]->inSight<=0)
        {
            ball[0]->inSight = 0.5;
            if (ball.count()>1)
            {
                for (int i=1;i<ball.count();i++)
                {
                    if (ball[i]->inSight > 0.0)
                    {
                        ball.swap(0,i);
                        break;
                    }
                }
            }
        }
    }
    for (int i=1;i<ball.count();i++)
    {
        if (ball[i]->inSight<=0.0)
        {
            delete ball[i];
            ball.removeAt(i);
            i--;
        }
    }
    return;
#if 0
    for (int j=0;j<_MAX_NUM_PLAYERS;j++)
    {
        for (int i=0;i<ourTeam[j].count();i++)
        {
            if (ourTeam[j][i]->inSight<=0.0)
            {
                delete ourTeam[j][i];
                ourTeam[j].removeAt(i);
                i--;
            }
        }
    }
    for (int j=0;j<_MAX_NUM_PLAYERS;j++)
    {
        for (int i=0;i<oppTeam[j].count();i++)
        {
            if (oppTeam[j][i]->inSight<=0.0)
            {
                delete oppTeam[j][i];
                oppTeam[j].removeAt(i);
                i--;
            }
        }
    }
        //return;
#endif
    for (int j=0;j<_MAX_NUM_PLAYERS;j++)
    {
        if (ourTeam[j].count()>0)
        {
            if (ourTeam[j][0]->inSight<=0)
            {
                ourTeam[j][0]->inSight = 0.5;
                if (ourTeam[j].count()>1)
                {
                    for (int i=1;i<ourTeam[j].count();i++)
                    {
                        if (ourTeam[j][i]->inSight > 0.0)
                        {
                            ourTeam[j].swap(0,i);
                            break;
                        }
                    }
                }
            }
        }
        for (int i=1;i<ourTeam[j].count();i++)
        {
            if (ourTeam[j][i]->inSight<=0.0)
            {
                delete ourTeam[j][i];
                ourTeam[j].removeAt(i);
                i--;
            }
        }
    }
    for (int j=0;j<_MAX_NUM_PLAYERS;j++)
    {
        if (oppTeam[j].count()>0)
        {
            if (oppTeam[j][0]->inSight<=0)
            {
                oppTeam[j][0]->inSight = 0.5;
                if (oppTeam[j].count()>1)
                {
                    for (int i=1;i<oppTeam[j].count();i++)
                    {
                        if (oppTeam[j][i]->inSight > 0.0)
                        {
                            oppTeam[j].swap(0,i);
                            break;
                        }
                    }
                }
            }
        }
        for (int i=1;i<oppTeam[j].count();i++)
        {
            if (oppTeam[j][i]->inSight<=0.0)
            {
                delete oppTeam[j][i];
                oppTeam[j].removeAt(i);
                i--;
            }
        }
    }
}

void CHalfWorld::update(QList<CBall *>& ball, CVisionBelief* v)
{

    if (ball.count() == 0)
    {
        for (int i=0;i<v->ball.count();i++)
        {
            ball.append(new CBall(false));
            ball.last()->update(v->ball[i]);
        }
    }
    else {
        QList<bool> flag;
        for (int i=0;i<ball.count();i++)
            flag.append(false);
        for (int i=0;i<v->ball.count();i++)
        {
            double min_d = 1e5;
            int k = -1;
            for (int j=0;j<ball.count();j++)
            {
                if (flag[j]) continue;
                double d = (ball[j]->pos + ball[j]->vel * getFramePeriod() - v->ball[i].pos).length();
                if (d < min_d)
                {
                    min_d = d;
                    k = j;
                }
            }
            if (k != -1)
            {
                if (min_d > ball[k]->vel.length() * getFramePeriod()*2.0 + 1.0)
                {
                    k = -1;
                }
                        }
            if (k != -1)
            {
                flag[k] = true;
                ball[k]->update(v->ball[i]);
            }
            else {
                flag.append(true);
                ball.append(new CBall(false));
                ball.last()->update(v->ball[i]);
            }
        }
        for (int i=0;i<flag.count();i++)
        {
            if (!flag[i])
            {
                ball[i]->update(CRawObject(0, ball[i]->pos, 0.0, -1, 0.0, NULL, v->cam_id, v->time));
            }
        }
    }
        /////WatchDog
        if (ball.count() > 0)
        {
                double minVel = 0.6;
                if (ball[0]->vel.length() < minVel)
                for (int k=1;k<ball.count();k++)
                {
                        if (ball[k]->vel.length() >= minVel)
                        {
                                ball.swap(k, 0);
                                break;
                        }
                }
        }
}

void CHalfWorld::update(QList<CRobot*>& robot, CVisionBelief* v, QList<CRawObject>& robot0, int id, bool our)
{
    if (robot.count() == 0)
    {
        for (int i=0;i<robot0.count();i++)
        {
            robot.append(new CRobot(id, our, false));
            robot.last()->update(robot0[i]);
        }
    }
    else {
        QList<bool> flag;
        for (int i=0;i<robot.count();i++)
            flag.append(false);
        for (int i=0;i<robot0.count();i++)
        {
            double min_d = 1e5;
            int k = -1;
            for (int j=0;j<robot.count();j++)
            {
                if (flag[j]) continue;
                double d = (robot[j]->pos + robot[j]->vel * getFramePeriod() - robot0[i].pos).length();
                if (d < min_d)
                {
                    min_d = d;
                    k = j;
                }
            }
            if (k != -1)
            {
                if (min_d > robot[k]->vel.length() * getFramePeriod()*2.0 + 1.0)
                {
                    k = -1;
                }
            }
            if (k != -1)
            {
                flag[k] = true;
                robot[k]->update(robot0[i]);
            }
            else {
                flag.append(true);
                robot.append(new CRobot(id, our, false));
                robot.last()->update(robot0[i]);
            }
        }
        for (int i=0;i<flag.count();i++)
        {
            if (!flag[i])
            {
                robot[i]->update(CRawObject(0, robot[i]->pos, robot[i]->dir.th().degree(), -1, 0.0, NULL, v->cam_id, v->time));
            }
        }
    }
}




void CHalfWorld::update(CVisionBelief *v)
{

    belief = *v;
    QList<CRawObject> p0;
    update(ball, v);
/*    for (int k=0;k<_MAX_NUM_PLAYERS;k++)
    {
        update(ourTeam[k], v, v->ourTeam[k], k, true);
    }
    for (int k=0;k<_MAX_NUM_PLAYERS;k++)
    {
        update(oppTeam[k], v, v->oppTeam[k], k, false);
    }*/
#if 0
    for (int i=0;i<min(ball.count(), MAX_OBJECTS);i++)
    {
        p0.append(CRawObject(0, ball[i]->pos + ball[i]->vel * getFramePeriod(), 0.0, ball[i]->cam_id, ball[i]->inSight));
    }
    dbg = true;
    track(p0, v->ball);
    dbg = false;
/*
    for (int i=0;i<p0.count();i++)
     {
         if (p0[i].updated)
         {
             if (i>=ball.count())
             {
                 ball.append(new CBall(false));
             }
             p0[i].time = v->time;
             ball[i]->update(p0[i]);
             ball[i]->lastFrameUpdated = currentFrame;
         }
         else ball[i]->update(CRawObject(ball[i]->pos, 0.0, -1, 0.0, NULL, 0, v->time));
     }
*/


    for (int i=0;i<p0.count();i++)
    {
        if (p0[i].updated)
        {
            if (i>=ball.count())
            {
                ball.append(new CBall(false));
            }
            p0[i].time = v->time;
//            if (ball[i]->cam_id==v->cam_id)
                ball[i]->update(p0[i]);
//            qDebug() << "Time: " << p0[i].time;
            ball[i]->lastFrameUpdated = currentFrame;
            ball[i]->cam_id = v->cam_id;
        }
        else {
//            if (ball[i]->cam_id==v->cam_id)
                ball[i]->update(CRawObject(0, ball[i]->pos, 0.0, -1, 0.0, NULL, v->cam_id, v->time));
        }
    }
#endif

//    for (int i=0;i<ball.count();i++)
//    {

//    }

//    qDebug() << "Ball Count = " << ball.count();

        for (int j=0;j<_MAX_NUM_PLAYERS;j++)
        {
            if (ourTeam[j].count() == 0)
            {
                ourTeam[j].append(new CRobot(j,true,false));
            }
            else {
                if (v->ourTeam[j].count()>0)
                    ourTeam[j][0]->update(v->ourTeam[j][0]);
                else
                    if (ourTeam[j][0]->inSight > 0)
                    {
                        ourTeam[j][0]->update(CRawObject(0, ourTeam[j][0]->pos, ourTeam[j][0]->dir.th().degree(), -1, 0.0, NULL, v->cam_id, v->time));
                    }
            }
        }
        for (int j=0;j<_MAX_NUM_PLAYERS;j++)
        {
            if (oppTeam[j].count() == 0)
            {
                oppTeam[j].append(new CRobot(j,true,false));
            }
            else {
                if (v->oppTeam[j].count()>0)
                    oppTeam[j][0]->update(v->oppTeam[j][0]);
                else
                    if (oppTeam[j][0]->inSight > 0)
                    {
                        oppTeam[j][0]->update(CRawObject(0, oppTeam[j][0]->pos, oppTeam[j][0]->dir.th().degree(), -1, 0.0, NULL, v->cam_id, v->time));
                    }
            }
        }


/*    for (int j=0;j<_MAX_NUM_PLAYERS;j++)
    {
        p0.clear();
        for (int i=0;i<min(ourTeam[j].count(), MAX_OBJECTS);i++)
        {
            p0.append(CRawObject(0, ourTeam[j][i]->pos, ourTeam[j][i]->dir.th().degree(), j, ourTeam[j][i]->inSight));
        }
        track(p0, v->ourTeam[j]);
        for (int i=0;i<p0.count();i++)
        {
            if (p0[i].updated)
            {
                if (i>=ourTeam[j].count())
                {
                    ourTeam[j].append(new CRobot(j,true,false));
                }
                p0[i].time = v->time;
                //if (ourTeam[j][i]->cam_id==v->cam_id)
                    ourTeam[j][i]->update(p0[i]);
                ourTeam[j][i]->lastFrameUpdated = currentFrame;
                ourTeam[j][i]->cam_id = v->cam_id;
            }
            else {
//                if (ourTeam[j][i]->cam_id==v->cam_id)
                    ourTeam[j][i]->update(CRawObject(0, ourTeam[j][i]->pos, ourTeam[j][i]->dir.th().degree(), j, 0.0, NULL, v->cam_id, v->time));
            }
        }
    }
    for (int j=0;j<_MAX_NUM_PLAYERS;j++)
    {
        p0.clear();
        for (int i=0;i<min(oppTeam[j].count(), MAX_OBJECTS);i++)
        {
            p0.append(CRawObject(0, oppTeam[j][i]->pos, oppTeam[j][i]->dir.th().degree(), j, oppTeam[j][i]->inSight));
        }
        track(p0, v->oppTeam[j]);
        for (int i=0;i<p0.count();i++)
        {
            if (p0[i].updated)
            {
                if (i>=oppTeam[j].count())
                {
                    oppTeam[j].append(new CRobot(j,false,false));
                }
                p0[i].time = v->time;
              //  if (oppTeam[j][i]->cam_id==v->cam_id)
                    oppTeam[j][i]->update(p0[i]);
                oppTeam[j][i]->lastFrameUpdated = currentFrame;
                oppTeam[j][i]->cam_id = v->cam_id;
            }
            else
            {
//                if (oppTeam[j][i]->cam_id==v->cam_id)
                    oppTeam[j][i]->update(CRawObject(0, oppTeam[j][i]->pos, oppTeam[j][i]->dir.th().degree(), j, 0.0, NULL, v->cam_id, v->time));
            }
        }
    }*/

}

void CHalfWorld::merge()
{
    return;
#if 0
    QList<Vector2D> temp;
    QList < QPair<int, int> >  nears;
    temp.clear();
    nears.clear();
    for( int i = 0; i < ball.count(); i++)
    {
        for ( int j = i+1; j < ball.count();j++)
        {
            if ( ball[i]->pos.dist( ball[j]->pos) < 0.12)
            {
                QPair p;
                p.first = i;
                p.second = j;
                nears.append( p);
            }
        }
    }

#endif
    /*
    int removeObj[100];
    for (int i=0;i<100;i++) removeObj[i] = -1;
    QList<CBall*> ball;
    for (int i=0;i<ball.count();i++) removeObj[i] = i;
    for (int i=0;i<ball.count();i++)
    {
        for (int j=i+1;j<ball.count();j++)
        {
            if ((ball[i]->pos-ball[j]->pos).length()<0.1)
            {
                if (removeObj[j] == -1)
                    removeObj[j] = i;
                else
                {
                    removeObj[i] = removeObj[j];
                }
            }
        }
    }
    QList<CBall*> newBalls;
    for (int i=0;i<ball.count();i++)
    {
        if (removeObj[i] != -1)
        {
            Vector2D pos, vel, dir;
            int count;
            pos.assign(0.0, 0.0);
            vel.assign(0.0, 0.0);
            dir.assign(0.0, 0.0);
            for (int k=i+1;k<ball.count();k++)
            {
                if (removeObj[k]==i)
                {
                    pos += ball[i]->pos;
                    vel += ball[i]->vel;
                    dir += ball[i]->dir;
                    delete ball[i];
                    count ++;
                }
            }
            if (count == 0)
            {
                pos = ball[i]->pos;
                vel = ball[i]->vel;
                dir = ball[i]->dir;
            }
            else {
                pos /= count;
                vel /= count;
                dir /= count;
            }
            ball[i]->pos = pos;
            ball[i]->vel = vel;
            ball[i]->dir = dir;
            newBalls.append(ball[i]);
        }
    }
    ball.clear();
    ball.append(newBalls);
    for (int k=0;k<_MAX_NUM_PLAYERS;k++)
    {
        for (int i=0;i<100;i++) removeObj[i] = -1;
        for (int i=0;i<ourTeam[k].count();i++) removeObj[i] = i;
        for (int i=0;i<ourTeam[k].count();i++)
        {
            for (int j=i+1;j<ourTeam[k].count();j++)
            {
                if ((ourTeam[k][i]->pos-ourTeam[k][j]->pos).length()<0.1)
                {
                    if (removeObj[j] == -1)
                        removeObj[j] = i;
                    else
                    {
                        removeObj[i] = removeObj[j];
                    }
                }
            }
        }
        QList<CRobot*> newRobots;
        for (int i=0;i<ourTeam[k].count();i++)
        {
            if (removeObj[i] != -1)
            {
                Vector2D pos, vel, dir;
                int count;
                pos.assign(0.0, 0.0);
                vel.assign(0.0, 0.0);
                dir.assign(0.0, 0.0);
                for (int k=i+1;k<ourTeam[k].count();k++)
                {
                    if (removeObj[k]==i)
                    {
                        pos += ourTeam[k][i]->pos;
                        vel += ourTeam[k][i]->vel;
                        dir += ourTeam[k][i]->dir;
                        delete ourTeam[k][i];
                        count ++;
                    }
                }
                if (count == 0)
                {
                    pos = ourTeam[k][i]->pos;
                    vel = ourTeam[k][i]->vel;
                    dir = ourTeam[k][i]->dir;
                }
                else {
                    pos /= count;
                    vel /= count;
                    dir /= count;
                }
                ourTeam[k][i]->pos = pos;
                ourTeam[k][i]->vel = vel;
                ourTeam[k][i]->dir = dir;
                newRobots.append(ourTeam[k][i]);
            }
        }
        ourTeam[k].clear();
        ourTeam[k].append(newRobots);
    }*/
/*    for (int k=0;k<_MAX_NUM_PLAYERS;k++)
    {
        for (int i=0;i<100;i++) removeObj[i] = -1;
        for (int i=0;i<oppTeam[k].count();i++) removeObj[i] = i;
        for (int i=0;i<oppTeam[k].count();i++)
        {
            removeObj[i] = i;
            for (int j=i+1;j<oppTeam[k].count();j++)
            {
                if ((oppTeam[k][i]->pos-oppTeam[k][j]->pos).length()<0.1)
                {
                    if (removeObj[j] == -1)
                        removeObj[j] = i;
                    else
                    {
                        removeObj[i] = removeObj[j];
                    }
                }
            }
        }
        QList<CRobot*> newRobot;
        for (int i=0;i<oppTeam[k].count();i++)
        {
            if (removeObj[i] != -1)
            {
                Vector2D pos, vel, dir;
                int count;
                pos.assign(0.0, 0.0);
                vel.assign(0.0, 0.0);
                dir.assign(0.0, 0.0);
                for (int k=i+1;k<ball.count();k++)
                {
                    if (removeObj[i]==k)
                    {
                        pos += oppTeam[k][removeObj[i]]->pos;
                        vel += oppTeam[k][removeObj[i]]->vel;
                        dir += oppTeam[k][removeObj[i]]->dir;
                        delete oppTeam[k][removeObj[i]];
                        count ++;
                    }
                }
                if (count == 0)
                {
                    pos = oppTeam[k][i]->pos;
                    vel = oppTeam[k][i]->vel;
                    dir = oppTeam[k][i]->dir;
                }
                else {
                    pos /= count;
                    vel /= count;
                    dir /= count;
                }
                oppTeam[k][i]->pos = pos;
                oppTeam[k][i]->vel = vel;
                oppTeam[k][i]->dir = dir;
                newRobot.append(oppTeam[k][i]);
            }
        }
        oppTeam[k].clear();
        oppTeam[k].append(newRobot);
    }*/
}

void CHalfWorld::update(CHalfWorld *w)
{
    for (int k=0;k<ball.count();k++) delete ball[k];
    ball.clear();
    for (int k=0;k<w->ball.count();k++)
    {
        ball.append(new CBall(true));
        ball.back()->update(w->ball[k]);
    }
    for (int j=0;j<_MAX_NUM_PLAYERS;j++)
    {
        for (int k=0;k<ourTeam[j].count();k++) delete ourTeam[j][k];
        ourTeam[j].clear();
        for (int k=0;k<w->ourTeam[j].count();k++)
        {
            ourTeam[j].append(new CRobot(j, true, true));
            ourTeam[j].back()->update(w->ourTeam[j][k]);
        }
    }
    for (int j=0;j<_MAX_NUM_PLAYERS;j++)
    {
        for (int k=0;k<oppTeam[j].count();k++) delete oppTeam[j][k];
        oppTeam[j].clear();
        for (int k=0;k<w->oppTeam[j].count();k++)
        {
            oppTeam[j].append(new CRobot(j, false, true));
            oppTeam[j].back()->update(w->oppTeam[j][k]);
        }
    }
}

void CHalfWorld::selectBall(Vector2D pos)
{
    double minDist = 0.0;
    int bestBall = -1;
    for (int i=0;i<ball.count();i++)
    {
        double d = (ball[i]->pos-pos).length();
        if (d<minDist || bestBall==-1)
        {
            bestBall = i;
            minDist = d;
        }
    }
    if (bestBall != -1 && ball.count()>0)
    {
        ball.swap(0, bestBall);
    }
}

void CWorldModel::unmaskOccludedBall()
{
        for (int i=0;i<wm->our.activeAgentsCount();i++)
        {
                if ((wm->our.active(i)->pos - wm->ball->pos).length() < 0.065)
                {
                        wm->ball->pos = wm->our.active(i)->getKickerPos();
                        return;
                }
        }
        for (int i=0;i<wm->opp.activeAgentsCount();i++)
        {
                if ((wm->opp.active(i)->pos - wm->ball->pos).length() < 0.065)
                {
                        wm->ball->pos = wm->opp.active(i)->getKickerPos();
                        return;
                }
        }
}

Vector2D CWorldModel::ballCatchTarget(CRobot *r)
{
    if (ball->vel.length() < 0.1) return ball->pos;
    double e = ((ball->pos-r->pos).norm()*ball->vel.norm());
    Vector2D p = ball->pos;
    double minSpeed = 0.4;
    if (e<0)
    {
        p = ball->getProjectionOfPointOnBallVeclocityDirection(r->pos, false);
        if (ball->ballSpeedAt((p-ball->pos).length()).length() < minSpeed)
        {
            p = ball->whereBallSpeedIs(minSpeed);
            if (!field->fieldRect().contains(p))
            {
                Vector2D sol1, sol2;
                field->fieldRect().intersection(Segment2D(ball->pos, p), &sol1, &sol2);
                if (sol1.valid()) p = sol1;
            }
        }
        return p;
    }
    else
    {
        p = ball->whereBallSpeedIs(minSpeed);
        if (!field->fieldRect().contains(p))
        {
            Vector2D sol1, sol2;
            field->fieldRect().intersection(Segment2D(ball->pos, p), &sol1, &sol2);
            if (sol1.valid()) p = sol1;
        }
        return p;
    }
}


struct range {
  float a,b;
};

inline float getangle(float x1,float y1,float x2,float y2)
{
        return atan2(y2-y1,x2-x1);
}

inline float len(float x1,float y1,float x2,float y2)
{
        return hypot(x1-x2, y1-y2);
}

inline float len2(float x1,float y1,float x2,float y2)
{
        return (x1-x2) * (x1-x2) + (y1-y2) * (y1-y2);
}


inline float normalang(float dir)
{
        const float _2PI = 2.0 * M_PI;
          if ( dir < -2.0*M_PI || 2.0*M_PI < dir )
          {
                  dir = fmod( dir, _2PI );
          }
          if ( dir < -M_PI)
          {
                  dir += 2.0*M_PI;
          }
          if ( dir > M_PI)
          {
                  dir -= 2.0*M_PI;
          }
          return dir;
}

inline float normalangabs(float dir)
{
        const float _2PI = 2.0 * M_PI;
        if ( dir < -2.0*M_PI || 2.0*M_PI < dir )
          {
                  dir = fmod( dir, _2PI );
          }
          if ( dir < -M_PI)
          {
                  dir += 2.0*M_PI;
          }
          if ( dir > M_PI)
          {
                  dir -= 2.0*M_PI;
          }
          if (dir < 0) return -dir;
          return dir;
}

double CWorldModel::openness(Vector2D point, int selfId, double &minDist, double &angle, bool relaxOur, double radFactor)
{
        //how much of goal is visible from `point', ignoring our robot with `selfId'
        //it is a low level code , because it is the same as code that was recently on gpu (evalposition.c)
        float gx1 = field->oppGoalL().x;
        float gy1 = field->oppGoalL().y;
        float gx2 = field->oppGoalR().x;
        float gy2 = field->oppGoalR().y;
        float x = point.x;
        float y = point.y;
        float d, a1, a2, a, l, a0, q1, q2, al;
        float la, lb, lc;
        float ox, oy;
        la = gy2-gy1;
        lb = gx1-gx2;
        lc = -gx1*la-gy1*lb;
        bool inobs = false;
        int par = 0;
        bool tmp;
        struct range tmpr;
        int count = 0;
        int i,j;
        float radf = CRobot::robot_radius_old * radFactor;
        float minl = 10000;
        d = 0;
        struct range r[20];
        bool flag[20];
        for (i = 0;i<20;i++)
                flag[i] = false;
        //al = normalang(getangle(gx1, gy1, gx2, gy2) - M_PI*0.5);
        al = getangle(x, y, (gx1+gx2)*0.5, (gy1+gy2)*0.5);
        q1 = getangle(x,y,gx1,gy1)-al;
        q2 = getangle(x,y,gx2,gy2)-al;
        q1 = normalang(q1);
        q2 = normalang(q2);
        if (normalang(q1 - q2) > 0)
        {
                a = q1;
                q1 = q2;
                q2 = a;
        }
        if (!relaxOur)
        for (i = 0; i < our.activeAgentsCount();i++)
        {
                if (our.active(i)->id == selfId) break;
                ox = our.active(i)->pos.x;
                oy = our.active(i)->pos.y;
                l = len(x,y,ox,oy);
                if (l < minl) minl = l;
                if (l<radf) {inobs = true;break;}
                a1 = ox*la + oy*lb + lc;
                a2 = x*la + y*lb + lc;
                if (a1 > 0) a1 = 1;else a1 = -1;
                if (a2 > 0) a2 = 1;else a2 = -1;
                a1 = a1*a2;
                if (a1 > 0)
                {
                        a = normalang(getangle(x,y,ox,oy)-al);
                        a0 = asin(radf/l);
                        a1 = a - a0;
                        a2 = a + a0;
                        if (a1 < -M_PI*0.95*0.5) a1 = -M_PI*0.95*0.5;
                        if (a2 < -M_PI*0.95*0.5) a2 = -M_PI*0.95*0.5;
                        if (a1 > +M_PI*0.95*0.5) a1 =  M_PI*0.95*0.5;
                        if (a2 > +M_PI*0.95*0.5) a2 =  M_PI*0.95*0.5;
                        if (normalang(a1-a2)>0)
                        {
                                a = a1;
                                a1 = a2;
                                a2 = a;
                        }
                        a1 = normalang(a1);
                        a2 = normalang(a2);
                        if (normalang(a1-q1)<=0) a1 = q1;
                        if (normalang(a1-q2)>=0) a1 = q2;
                        if (normalang(a2-q1)<=0) a2 = q1;
                        if (normalang(a2-q2)>=0) a2 = q2;
                        if (normalangabs(a1-a2)>=0.001)
                        {
                                r[count].a = a1;
                                r[count].b = a2;
                                count ++;
                        }
                }
        }
        if (!inobs) {
                for (i = 0; i < opp.activeAgentsCount();i++) //Copy Pasted from the above block except for two first lines
                {
                        ox = opp.active(i)->pos.x;
                        oy = opp.active(i)->pos.y;
                        l = len(x,y,ox,oy);
                        if (l < minl) minl = l;
                        if (l<radf) {inobs = true;break;}
                        a1 = ox*la + oy*lb + lc;
                        a2 = x*la + y*lb + lc;
                        if (a1 > 0) a1 = 1;else a1 = -1;
                        if (a2 > 0) a2 = 1;else a2 = -1;
                        a1 = a1*a2;
                        if (a1 > 0)
                        {
                                a = normalang(getangle(x,y,ox,oy)-al);
                                a0 = asin(radf/l);
                                a1 = a - a0;
                                a2 = a + a0;
                                if (a1 < -M_PI*0.95*0.5) a1 = -M_PI*0.95*0.5;
                                if (a2 < -M_PI*0.95*0.5) a2 = -M_PI*0.95*0.5;
                                if (a1 > +M_PI*0.95*0.5) a1 =  M_PI*0.95*0.5;
                                if (a2 > +M_PI*0.95*0.5) a2 =  M_PI*0.95*0.5;
                                if (normalang(a1-a2)>0)
                                {
                                        a = a1;
                                        a1 = a2;
                                        a2 = a;
                                }
                                a1 = normalang(a1);
                                a2 = normalang(a2);
                                if (normalang(a1-q1)<=0) a1 = q1;
                                if (normalang(a1-q2)>=0) a1 = q2;
                                if (normalang(a2-q1)<=0) a2 = q1;
                                if (normalang(a2-q2)>=0) a2 = q2;
                                if (normalangabs(a1-a2)>=0.001)
                                {
                                        r[count].a = a1;
                                        r[count].b = a2;
                                        count ++;
                                }
                        }
                }
        }
        if (!inobs)
        {
                for (i=0;i<count;i++)
                        for (j=0;j<count-1;j++)
                        {
                                if (normalang(r[j].a-r[j+1].a) > 0)
                                {
                                        tmpr = r[j];
                                        r[j] = r[j+1];
                                        r[j+1] = tmpr;
                                }
                        }
                //quicksort(r, 0, count-1);
                for (i=0;i<count-1;i++)
                {
                        if (normalang(r[i+1].a - r[i].b) < 0)
                        {
                                r[i+1].a = r[i].a;
                                if (normalang(r[i+1].b - r[i].b) < 0)
                                {
                                        r[i+1].b = r[i].b;
                                }
                                flag[i] = true;
                        }
                }
                for (i=0;i<count;i++)
                {
                        if (flag[i] == false)
                        {
                                d += normalang(r[i].b - r[i].a);
                                flag[i] = true;
                        }
                }
                //d = (normalangabs(q2-q1) - d)*6.0f;
                angle = normalangabs(q2-q1);
                d = angle-d;

                //d = d*0.5 + normalang(q2-q1) * 0.5;
        }
        else d = 0.0;
        minDist = minl;
        //d = d*(1.0f-exp(-minl*minl/300.0f));//  tanh(minl/40.0f);
        if (d < 0.0f) d = 0.0f;
        return d;
}


double CWorldModel::getEmptyAngle(Vector2D p,Vector2D p1, Vector2D p2, QList<Circle2D> obs, double& percent, double &mostOpenAngle, double& biggestAngle, bool oppGoal, bool _draw)
{
        bool drawn = false;
        QColor rect_color;
        if(oppGoal)
        {
                int r , g , b;
                QColor("darkcyan").getRgb(&r , &g, &b);
                rect_color =  QColor(r, g, b, 21);
        }
        else
        {
                int r , g , b;
                QColor("magenta").getRgb(&r , &g, &b);
                rect_color = QColor(r , g, b, 21);
        }
        Vector2D goal_pos;

        if(oppGoal)
                goal_pos = field->oppGoal();
        else
                goal_pos = field->ourGoal();

        float gx1 = p1.x;
        float gy1 = p1.y;
        float gx2 = p2.x;
        float gy2 = p2.y;
        float x = p.x;
        float y = p.y;
        //similar codes can be found in cl/evalpos.c
        float d, a1, a2, a, l, a0, q1, q2, al;
        float la, lb, lc;
        float ox, oy;
        la = gy2-gy1;
        lb = gx1-gx2;
        lc = -gx1*la-gy1*lb;
        bool inobs = false;
        int par = 0;
        bool tmp;
        struct range tmpr;
        int count = 0;
        int i,j;
        d = 0;
        struct range r[20];
        bool flag[20];
        for (i = 0;i<20;i++)
                flag[i] = false;
        al = getangle(x, y, (gx1+gx2)*0.5, (gy1+gy2)*0.5);
        q1 = getangle(x,y,gx1,gy1)-al;
        q2 = getangle(x,y,gx2,gy2)-al;
        q1 = normalang(q1);
        q2 = normalang(q2);
        if (normalang(q1 - q2) > 0)
        {
                a = q1;
                q1 = q2;
                q2 = a;
        }
        float openangle = 0;
        for (i = 0; i < obs.count(); ++i) {
                ox = obs[i].center().x;
                oy = obs[i].center().y;
                float rad = obs[i].radius();
                l = len(x,y,ox,oy);
                if (l<rad) {inobs = true;break;}
                a1 = ox*la + oy*lb + lc;
                a2 = x*la + y*lb + lc;
                if (a1 > 0) a1 = 1;else a1 = -1;
                if (a2 > 0) a2 = 1;else a2 = -1;
                a1 = a1*a2;
                if (a1 > 0)
                {
                        a = normalang(getangle(x,y,ox,oy)-al);
                        a0 = asin(rad/l);
                        a1 = a - a0;
                        a2 = a + a0;
                        if (a1 < -M_PI*0.95*0.5) a1 = -M_PI*0.95*0.5;
                        if (a2 < -M_PI*0.95*0.5) a2 = -M_PI*0.95*0.5;
                        if (a1 > +M_PI*0.95*0.5) a1 =  M_PI*0.95*0.5;
                        if (a2 > +M_PI*0.95*0.5) a2 =  M_PI*0.95*0.5;
                        if (normalang(a1-a2)>0)
                        {
                                a = a1;
                                a1 = a2;
                                a2 = a;
                        }
                        a1 = normalang(a1);
                        a2 = normalang(a2);
                        if (normalang(a1-q1)<=0) a1 = q1;
                        if (normalang(a1-q2)>=0) a1 = q2;
                        if (normalang(a2-q1)<=0) a2 = q1;
                        if (normalang(a2-q2)>=0) a2 = q2;
                        if (normalangabs(a1-a2)>=0.001)
                        {
                                r[count].a = a1;
                                r[count].b = a2;
                                count ++;
                        }
                }
        }
        if (!inobs)
        {
                for (i=0;i<count;i++)
                        for (j=0;j<count-1;j++)
                        {
                                if (normalang(r[j].a-r[j+1].a) > 0)
                                {
                                        tmpr = r[j];
                                        r[j] = r[j+1];
                                        r[j+1] = tmpr;
                                }
                        }
                for (i=0;i<count-1;i++)
                {
                        if (normalang(r[i+1].a - r[i].b) < 0)
                        {
                                r[i+1].a = r[i].a;
                                if (normalang(r[i+1].b - r[i].b) < 0)
                                {
                                        r[i+1].b = r[i].b;
                                }
                                flag[i] = true;
                        }
                }
                bool changed = false;
                if (count > 0)
                {
                        float lastBlockedDir = q1;
                        int k = 0;
                        mostOpenAngle = 0;
                        biggestAngle = 0;
                        for (i=0;i<count;i++)
                        {
                                if (flag[i] == false)
                                {
                                        if ((k == 0) && (normalang(q1 - r[i].a) >= 0))
                                        {

                                        }
/*                    else if ((i == count-1) && (normalang(r[count-1].b - q2) >= 0))
                                        {

                                        }*/
                                        else {
                                                float dist = normalangabs(r[i].a - lastBlockedDir);
                                                float bisect = normalang(normalang(r[i].a - lastBlockedDir) / 2.0 + lastBlockedDir);
                                                if (dist >= biggestAngle)
                                                {
                                                        biggestAngle = dist;
                                                        mostOpenAngle = bisect;
                                                        changed = true;
                                                }
                                        }
                                        if(_draw)
                                        {
                                                Line2D line1(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI));
                                                Line2D line2(p, p+Vector2D::unitVector(((r[i].a+al) * 180.0 / M_PI)));
                                                Line2D goal_line(goal_pos, Vector2D(goal_pos.x, goal_pos.y+2.0));
                                                Vector2D p1(line1.intersection(goal_line));
                                                Vector2D p2(line2.intersection(goal_line));

                                                Polygon2D polygon_draw;
                                                polygon_draw.addVertex(p);
                                                polygon_draw.addVertex(p1);
                                                polygon_draw.addVertex(p2);
                                                polygon_draw.addVertex(p);
                                                draw( polygon_draw, rect_color, true);
                                                drawn = true;

                                                //draw(Segment2D(p, p+Vector2D::unitVector((bisect+al) * 180.0 / M_PI)*5 ), "black");
                                                //draw(Segment2D(p, p+Vector2D::unitVector((r[i].a+al) * 180.0 / M_PI)*5 ), "black");
                                                //draw(Segment2D(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI)*5 ), "purple");
                                        }
                                        lastBlockedDir = r[i].b;
                                        k ++;
                                }
                        }
                        if (normalang(r[count-1].b - q2) <= 0)
                        {
                                float dist = normalangabs(q2 - lastBlockedDir);
                                float bisect = normalang(normalang(q2 - lastBlockedDir) / 2.0 + lastBlockedDir);
                                if (dist >= biggestAngle)
                                {
                                        biggestAngle = dist;
                                        mostOpenAngle = bisect;
                                        changed = true;
                                }
                                if(_draw)
                                {
                                        Line2D line1(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI));
                                        Line2D line2(p, p+Vector2D::unitVector(((q2+al) * 180.0 / M_PI)));
                                        Line2D goal_line(goal_pos, Vector2D(goal_pos.x, goal_pos.y+2.0));
                                        Vector2D p1(line1.intersection(goal_line));
                                        Vector2D p2(line2.intersection(goal_line));

                                        Polygon2D polygon_draw;
                                        polygon_draw.addVertex(p);
                                        polygon_draw.addVertex(p1);
                                        polygon_draw.addVertex(p2);
                                        polygon_draw.addVertex(p);
                                        draw( polygon_draw, rect_color, true);
                                        drawn = true;

//                        draw(Segment2D(p, p+Vector2D::unitVector((bisect+al) * 180.0 / M_PI)*5 ), "blue");
//                        draw(Segment2D(p, p+Vector2D::unitVector((q2+al) * 180.0 / M_PI)*5 ), "red");
//                        draw(Segment2D(p, p+Vector2D::unitVector((lastBlockedDir+al) * 180.0 / M_PI)*5 ), "orange");
                                }
                        }
                }
                for (i=0;i<count;i++)
                {
                        if (flag[i] == false)
                        {
                                d += normalang(r[i].b - r[i].a);
                                flag[i] = true;
                        }
                }
                openangle = d;
                openangle = normalangabs(q2-q1) - openangle;
                d /= normalangabs(q2-q1);
                d = 1-d;
                if (!changed || (d<0.001)){
                        if (count==0) biggestAngle = normalangabs(q2 - q1);
                        else biggestAngle = 0;
                        mostOpenAngle = normalang(normalang(q2 - q1) / 2.0 + q1);
                }
                mostOpenAngle = normalang(mostOpenAngle + al);
                biggestAngle *= 180.0 / M_PI;
                mostOpenAngle *= 180.0 / M_PI;
        }
        else {
                mostOpenAngle = normalang(0.5*(q1 + q2) + al);
                biggestAngle = 0;
                mostOpenAngle *= 180.0 / M_PI;
//                         draw(Segment2D(p, p+Vector2D::unitVector((mostOpenAngle) )*5 ), "blue");

                d = 0.0;
        }
        percent = d;

        if(_draw && !drawn)
        {
                Vector2D p1, p2;
                if(oppGoal)
                {
                        p1 = field->oppGoalL();
                        p2 = field->oppGoalR();
                }
                else
                {
                        p1 = field->ourGoalL();
                        p2 = field->ourGoalR();
                }
                Polygon2D polygon_draw;
                polygon_draw.addVertex(p);
                polygon_draw.addVertex(p1);
                polygon_draw.addVertex(p2);
                polygon_draw.addVertex(p);
                drawn = true;

                draw( polygon_draw, rect_color, true);
        }
        return openangle * 180.0 / M_PI;
}
