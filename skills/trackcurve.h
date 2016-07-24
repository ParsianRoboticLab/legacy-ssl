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
    SkillProperty(CSkillTrackCurve, Vector2D, Origin, pos0);
    SkillProperty(CSkillTrackCurve, CCurve*, Curve, curve);
    SkillProperty(CSkillTrackCurve, double, RMax, r_max);
    SkillProperty(CSkillTrackCurve, double, DeltaAngle, delta_angle);
    SkillProperty(CSkillTrackCurve, Vector2D, LookAt, lookat);
    SkillProperty(CSkillTrackCurve, Vector2D, ConstantDir, constDir);
    SkillProperty(CSkillTrackCurve, double, Error, error);
    SkillProperty(CSkillTrackCurve, bool, TrackStarted, trackstarted);
    SkillProperty(CSkillTrackCurve, double, InitialVelocity, initialvel);
};

#endif // TRACKCURVE_H
