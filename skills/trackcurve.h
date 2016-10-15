#ifndef TRACKCURVE_H
#define TRACKCURVE_H

#include <curve.h>
#include <gotopoint.h>

class CSkillTrackCurve : public CSkill
{
protected:
    CSkillGotoPointAvoid* gotopointavoid;
    Vector2D lastMousePos;
public:
    DEF_SKILL(CSkillTrackCurve);
    virtual void generateFromConfig(CAgent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    SkillProperty(CSkillTrackCurve, Vector2D, Origin, orig);
    SkillProperty(CSkillTrackCurve, bool, ClockWise, CW);
    SkillProperty(CSkillTrackCurve, bool, Slow,slow);
    SkillProperty(CSkillTrackCurve, double , DesiredRad,desiredRad);
    SkillProperty(CSkillTrackCurve, AngleDeg, StepAngel, stepAngel);
    SkillProperty(CSkillTrackCurve, AngleDeg , EndAng, endAng);
    SkillProperty(CSkillTrackCurve, Vector2D, RobotAng,robotAng);
private:
    QList <Vector2D> path;
    Vector2D nextPoint;
    CNewBangBang *bangBang;
    double rad;
    Circle2D curve;
    Vector2D agentPos;
    Vector2D agentDir;
    Vector2D agentVel;

    Vector2D firstPos;
    Vector2D finalPos;
    _PID *curvePID;

};

#endif // TRACKCURVE_H
