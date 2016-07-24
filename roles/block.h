#ifndef Block_H
#define Block_H

#include <role.h>

class CRoleBlockInfo : public CRoleInfo
{
public:
    CRoleBlockInfo(QString _roleName);
	void findPos( bool blockGoal );
    Vector2D blockPosition;
    void reset(){}
};

class CRoleBlock : public CRole
{
protected:
    CSkillGotoPointAvoid* gotopoint;
public:
    DEF_ROLE(CRoleBlock);
    virtual void generateFromConfig(CAgent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
	virtual void parse(QStringList params = QStringList());
    SkillProperty(CRoleBlock, bool, BlockGoal, blockGoal);
    SkillProperty(CRoleBlock, bool, Stop, stop);
};

#endif // Block_H
