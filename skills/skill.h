#ifndef BASICSKILL_H
#define BASICSKILL_H

#include <agent.h>
#include <statemachine.h>
class QCheckBox;
class QComboBox;
class QLineEdit;
class CSkill;
class CSkills;
class CSkillConfigWidget : public QWidget
{    
public:
    CSkillConfigWidget();
    CSkillConfigWidget(QWidget* parent);
    QList<QLineEdit* > lineEdits;
    QList<QCheckBox* > checkBoxs;
    QList<QComboBox* > comboBoxs;
    Vector2D* mousePos;
    CSkill* skill;
friend class CSkill;
};

class CSkill
{
protected:
    CStateMachine sm;
    QString stateName();
public:
    QString localAgentName;
    CSkillConfigWidget* configWidget;
    CSkill() {}
    CSkill(CAgent* _agent);
    ~CSkill();
    virtual int level();
    bool done();
    bool failed();
    void assign(CAgent* _agent);
    virtual void parse(QStringList params);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget* parent);
    virtual void generateFromConfig(CAgent* a);
    virtual CSkill* allocate(CAgent* _agent)=0;
    virtual QString getName()=0;
    virtual double timeNeeded();  //in seconds
    virtual double successRate(); //between 0-1    

    //these functions should be defined in child classes    
    virtual double progress()=0;  //between 0-1 ; less than zero on failure
    virtual void execute()=0;

    Property(CAgent*, Agent, agent);
friend class CSkillConfigWidget;
friend class CSkills;
};

class CSkills
{
public:
    CSkills();
    ~CSkills();
    static bool registerSkill(const char *name, CSkill* Skill);
    static CSkill* initSkill(const char *name, CAgent* _agent);
    static int skillsCount();
    static CSkill* skill(int i);
    static void* getInfo(const char* name);
private:
    struct RegisteredSkill
    {
        const char *name;
        CSkill *Skill;
        void* Info;
    };
    static QList<RegisteredSkill>* Skills;
    static bool inited;
};

#define DEF_SKILL(skill) \
    skill() {skill(NULL);} \
    skill(CAgent* _agent); \
    ~skill(); \
    static const char *Name;\
    virtual CSkill* allocate(CAgent* _agent); \
    virtual void execute(); \
    virtual double progress(); \
    virtual QString getName(); \
    static skill* set(CAgent* a); \
    static skill* get(CAgent* a)
#define INIT_SKILL(Skill,name) \
    bool Skill##_registered \
            = CSkills::registerSkill(Skill::Name,new Skill(NULL)); \
    CSkill* Skill::allocate(CAgent* _agent) \
    {return new Skill(_agent);} \
    QString Skill::getName() {return QString(Name);} \
    Skill* Skill::set(CAgent* a) \
    { \
        if (QString(Skill::Name)!=a->skillName) \
        { \
            if (a->skill!=NULL && a->skillName!="") delete ((Skill*) a->skill); \
            else a->skill = (CSkill*) (new Skill(a)); \
            a->skillName = Skill::Name; \
        } \
        return (Skill*) a->skill; \
    } \
    Skill* Skill::get(CAgent* a) \
    { \
        return (Skill*) a->skill; \
    } \
    const char* Skill::Name = name

#define SkillState(classname,statenumber) void classname::State##statenumber ()
#define SkillInitState(statenumber,statename) \
        CState* st##statenumber = new CState; \
        QObject::connect(st##statenumber, SIGNAL(run()), this, SLOT(State##statenumber())); \
        st##statenumber -> name = statename; \
        sm.addState(st##statenumber)
#define SkillSwitchState(tostate) sm.setCurrentState(tostate)
#define SkillBreakState return
#define SkillRunStates sm.run();return

#define SkillProperty(skill,type,name,local) \
        public: inline type get##name() {return local;} \
        public: inline skill* set##name(type val) {local = val;return this;} \
        protected: type local

#define SkillPropertyNoSet(skill,type,name,local) \
        public: inline type get##name() {return local;} \
        public: skill* set##name(type val); \
        protected: type local

#endif // BASICSKILL_H
