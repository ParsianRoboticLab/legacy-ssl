#include "skill.h"
#include "roles/role.h"
#include <QWidget>
CSkillConfigWidget::CSkillConfigWidget()
{

}

CSkillConfigWidget::CSkillConfigWidget(QWidget *parent)
    : QWidget(parent)
{

}

int CSkill::level()
{
    return 1;
}

CSkill::CSkill(CAgent* _agent)
{
    agent = _agent;
}

CSkill::~CSkill()
{
}

double CSkill::timeNeeded()
{
    return 0;

}

double CSkill::successRate()
{
    return 1;
}

bool CSkill::done()
{
	if (agent == NULL) return true;
    return progress()>=1.0;
}

bool CSkill::failed()
{
    return progress()<0;
}

void CSkill::parse(QStringList /*params*/)
{

}

CSkillConfigWidget* CSkill::generateConfigWidget(QWidget* /*parent*/)
{
    return NULL;
}

void CSkill::generateFromConfig(CAgent *a)
{
    agent = a;
    return;
}


QString CSkill::stateName()
{
    return sm.currentStateName();
}

void CSkill::assign(CAgent* _agent)
{

    if (_agent!=NULL)
    {
        agent = _agent;
        agent->skill = this;
        agent->skillName = getName();
        agent->localName = localAgentName;
    }
}

//--------------------

QList<CSkills::RegisteredSkill>* CSkills::Skills;  
bool CSkills::inited=false;  

CSkills::CSkills()
{
}

CSkills::~CSkills()
{
    delete Skills;
}

bool CSkills::registerSkill(const char *name, CSkill* Skill)
{
    if (!inited)
    {
        Skills = new QList<CSkills::RegisteredSkill>;
        inited = true;
    }
    void* info = NULL;
    if (Skill->level()==2)
    {
        CRole* role = static_cast <CRole*> (Skill);
        info = (void*) (role->generateInfoClass());
    }    
    Skills->append((RegisteredSkill) { name, Skill, info });
    return true;
}

CSkill* CSkills::initSkill(const char *name,CAgent* _agent)
{    
    for (int i=0;i<Skills->size();i++)
        if (strcmp((*Skills)[i].name,name)==0)
            return (*Skills)[i].Skill->allocate(_agent);
    return NULL;
}

void* CSkills::getInfo(const char *name)
{
    for (int i=0;i<Skills->size();i++)
        if (strcmp((*Skills)[i].name,name)==0)
            return (*Skills)[i].Info;
    return NULL;
}

int CSkills::skillsCount()
{
    return Skills->count();
}

CSkill* CSkills::skill(int i)
{
    if (i>=0 && i<skillsCount())
    {
        return (*Skills)[i].Skill;
    }
    return NULL;
}
