#ifndef ROLE_H
#define ROLE_H

#include <skills.h>
#include <knowledge.h>
#include <behaviours/behaviour.h>

namespace roleSkill {

enum ESkill {
    Gotopoint      = 0,
    GotopointAvoid = 1,
    Kick           = 2,
    ReceivePass    = 3,
    OneTouch       = 4,
    Mark           = 5,  // After Life (Steady)
    Support        = 6,  // After Life (Move for ball)
    Defense        = 7   // After Life (Move back to our field)
};

}

class CRoleInfo;

class CRole : public CSkill
{
protected:
    virtual int level();    
    CBehaviour* currentBehaviour;
    void selectBehaviour();
public:    
    QList<CBehaviour*> behaviours;
    CRole();
    CRole(CAgent* _agent);        
    virtual CRoleInfo* generateInfoClass();
    static CRole* fromString(QString roleName);
};

class CRoleInfo
{    
protected:
    QString roleName;
public:        
    bool calculated;
    CRoleInfo(QString _roleName);    
    virtual CAgent* robot(int i);
    virtual int count();
    virtual int index(CRole* me);    
    void initBehaviours();
    QList<CBehaviour*> behaviours;
    void reset(){}
};

#define DEF_ROLE(role) \
    role() {} \
	role(CAgent* _agent); \
    ~role(); \
    static const char *Name;\
    virtual CSkill* allocate(CAgent* _agent); \
    virtual void execute(); \
    virtual double progress(); \
    virtual QString getName(); \
    static role* set(CAgent* a); \
    static role* get(CAgent* a); \
    virtual CRoleInfo* generateInfoClass(); \
    static role##Info* info();


#define INIT_ROLE(Role,name) \
    bool Role##_registered \
            = CSkills::registerSkill(Role::Name,new Role(NULL)); \
    CSkill* Role::allocate(CAgent* _agent) \
    {return new Role(_agent);} \
    QString Role::getName() {return QString(Name);} \
    Role* Role::set(CAgent* a) \
    { \
        if (QString(Role::Name)!=a->skillName) \
        { \
            if (a->skill!=NULL && a->skillName!="") delete ((Role*) a->skill); \
            else a->skill = (CSkill*) (new Role(a)); \
            a->skillName = Role::Name; \
        } \
        return (Role*) a->skill; \
    } \
    Role* Role::get(CAgent* a) \
    { \
        return (Role*) a->skill; \
    } \
    Role##Info* Role::info() \
    { \
        return (Role##Info*) CSkills::getInfo(name); \
    } \
    CRoleInfo* Role::generateInfoClass() \
    { \
        return new Role##Info(QString(name)); \
    } \
    const char* Role::Name = name

#define ClassProperty(skill,type,name,local,chflag) \
        public: inline type get##name() const {return local;} \
        public: inline skill* set##name(type val) {local = val;chflag = true;return this;} \
        protected: type local



#endif // ROLE_H
