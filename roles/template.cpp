#include "template.h"

INIT_ROLE(CRoleTemplate, "template");

CRoleTemplate::CRoleTemplate(CAgent *_agent) : CRole(_agent)
{

}

void CRoleTemplate::execute()
{
}

double CRoleTemplate::progress()
{
    return 0.0;
}

CSkillConfigWidget* CRoleTemplate::generateConfigWidget(QWidget *parent)
{
    return NULL;
}

void CRoleTemplate::generateFromConfig(CAgent *a)
{
    agent = a;
}

CRoleTemplateInfo::CRoleTemplateInfo(QString _roleName) : CRoleInfo(_roleName)
{

}
