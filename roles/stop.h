#ifndef STOP_H
#define STOP_H

#include <role.h>

class CRoleStopInfo : public CRoleInfo
{
public:
    CRoleStopInfo(QString _roleName);
    void findPositions();
	int inCorner;
    Vector2D TA;
    QList <Vector2D> Ps;
    QList <double> thR;
    QList <double> thP;
    QList <int> robotId;
    void reset(){}
};

class CRoleStop : public CRole
{
protected:
    CSkillGotoPointAvoid* gotopoint;
	bool switchAgent;
public:
    DEF_ROLE(CRoleStop);
    virtual void generateFromConfig(CAgent *a);
	virtual void parse(QStringList params);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
};

class CRoleHaltInfo : public CRoleInfo
{
public:
    CRoleHaltInfo(QString _roleName);
};

class CRoleHalt : public CRole
{
public:
    DEF_ROLE(CRoleHalt);
    virtual void generateFromConfig(CAgent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
};

#endif // STOP_H
