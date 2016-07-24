#include "trackcurve.h"
#include "trajectoryplanner.h"
#include <QDebug>

INIT_SKILL(CSkillTrackCurve, "trackcurve");  

CSkillTrackCurve::CSkillTrackCurve(CAgent *_agent) : CSkill(_agent)
{
    pos0 = Vector2D(0.0, 0.0);
    r_max = -1.0;
    constDir.invalidate();
    delta_angle = 0.0;
    initialvel = 0.0;
    lookat.invalidate();
    gotopointavoid = new CSkillGotoPointAvoid(agent);
    curve = NULL;
}

CSkillTrackCurve::~CSkillTrackCurve()
{
    delete gotopointavoid;
}

void CSkillTrackCurve::execute()
{
    if (curve==NULL) return;
    pos0.assign(0,0);
    Vector2D position0 = pos0;
    double x    = agent->pos().x - position0.x;
    double y    = agent->pos().y - position0.y;
    double vx   = agent->vel().x;
    double vy   = agent->vel().y;
    double ang  = agent->dirDegree();
    if (!trackstarted)
    {
        gotopointavoid->setAgent(agent);
        Vector2D startpoint(pos0.x + curve->X(curve->last_t), pos0.y + curve->Y(curve->last_t));
        Vector2D startdir, curvedir;
        curvedir.assign(curve->derivX(curve->last_t), curve->derivY(curve->last_t));
        curvedir.normalize();
        if (lookat.valid())
        {
            startdir = (lookat - startpoint).norm();
        }
        else if (constDir.valid()){
            startdir = constDir;
        }
        else
        {
            startdir = curvedir;
            startdir.rotate(delta_angle);
        }        
        gotopointavoid->setFinalVel(initialvel * curvedir);
        gotopointavoid->setFinalDir(startdir);
        gotopointavoid->setFinalPos(startpoint);
        if (((agent->pos() - startpoint).length()<0.1) && fabs(Vector2D::angleBetween(agent->dir(), startdir).degree())<20.0)
            trackstarted = true;
        else
        {
            gotopointavoid->execute();
            return;
        }
    }
    double t  = curve->invXY(x,y,curve->last_t);    
    curve->last_t = t;
    double tf = curve->tFinal();
    double lx,ly,xx,yy;
    for (int i=0;i<200;i++)
    {
        double qt = (tf  -  t )*(double)i/200.0f  +  t ;
        xx = curve->X(qt)+pos0.x;
        yy = curve->Y(qt)+pos0.y;

        if (i>0) draw(Segment2D(lx,ly,xx,yy));
        lx = xx;
        ly = yy;
    }
    double dx=curve->derivX(t);
    double dy=curve->derivY(t);
    double d2x=curve->deriv2X(t);
    double d2y=curve->deriv2Y(t);
    double ss=hypot(dx,dy);
    double pp=dx*d2y-dy*d2x;
    double rsign=sign(pp);
    double r=fabs(ss*ss*ss/pp);
//    qDebug() << r;
    double pf=curve->curveLength(t,tf);
    if (fabs(pf)<0.05) {
        Vector2D target(curve->X(tf) + pos0.x,curve->Y(tf) + pos0.y);
        Vector2D targetdir(curve->derivX(tf), curve->derivY(tf));
        targetdir.rotate(delta_angle);
        if (lookat.valid()) targetdir = (lookat - agent->pos()).norm();
        else if (constDir.valid()) targetdir = constDir;
        gotopointavoid->setAgent(agent);
        gotopointavoid->init(target, targetdir);
        gotopointavoid->execute();
        return;
    }    
    double vLimit=agent->getvLimit();
	double vmax = conf()->BangBang_VelTangent_Max();
    double rmax_turning = r_max,vmax_turning=vmax;
    if (r_max>0)
    {
        vmax_turning = vmax*r/rmax_turning;
        if (vmax_turning>vmax) vmax_turning = vmax;
        if (vmax_turning<vLimit) vmax_turning = vLimit;
    }
    double vsign=1;
    if (pf<0) {pf=-pf;vsign=-1;}
    double dt = 0.2;
    double tt,aa;
    double v=vsign*CTrajectoryPlanner::plan(pf,hypot(vx,vy),0.0,
											conf()->BangBang_AccCurve_Max(),
											conf()->BangBang_DecCurve_Max(),
                                            vmax_turning,dt,aa,tt);
    double posgain = -1.0;//-4.0; //2.0
    double wgain   = -1.3;//-0.5;//-1.3; //1.0
    double tangent_ang = AngleDeg::atan2_deg(dy,dx);
    double xcurve  = curve->X(t);
    double ycurve  = curve->Y(t);
    Vector2D err(x-xcurve, y-ycurve);
    Vector2D dperp(-dy, dx);
    dperp.normalize();
    if ((err * dperp) > 0) dperp = -dperp;
    double poserror = err * dperp;
    double vnormal = fabs(poserror*posgain);
    bool rotating = true;
    double delta_ang = AngleDeg::normalize_angle(ang-tangent_ang-delta_angle);
    if (lookat.valid())
    {
        delta_ang = AngleDeg::normalize_angle(ang- (lookat-agent->pos()).th().degree());
        rotating = false;
    }
    if (constDir.valid())
    {
        delta_ang = AngleDeg::normalize_angle(ang- constDir.th().degree());
        rotating = false;
    }
    double w = delta_ang*(M_PI/180.0f)*wgain + ((rotating) ? (rsign*v/r) : 0.0);
    Vector2D vv = v * Vector2D(dx, dy).norm() + vnormal * dperp;
    agent->setRobotAbsVel(vv.x, vv.y, w);    
}

double CSkillTrackCurve::progress()
{
    return 0.0;
}
