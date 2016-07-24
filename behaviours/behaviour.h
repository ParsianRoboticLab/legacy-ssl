#ifndef BEHAVIOUR_H
#define BEHAVIOUR_H

#include <QString>
#include <QList>
#include <knowledge.h>
struct BehaviourValue
{
    double reward, penalty;
    BehaviourValue();
};

class CBehaviour
{
protected:    
public:
    CBehaviour();
	virtual ~CBehaviour();
    void updateIdleStatus();
    virtual CBehaviour* allocate();
    virtual QString getName();
    virtual void execute();        
    virtual double probability();    
    QList<CAgent*> agents; //agents[0] is doing the action, others may be the co-operators; [e.g. pass behaviour]
    QStringList roles;
};

class CBehaviours
{
public:
    CBehaviours();
    ~CBehaviours();
    static QMap<QString, BehaviourValue> values;
    static bool registerBehaviour(const char *name, CBehaviour* Behaviour);
    static CBehaviour* initBehaviour(const char *name);
    static int BehavioursCount();
    static CBehaviour* Behaviour(int i);
    static void loadValues(QString filename);
private:
    struct RegisteredBehaviour
    {
        const char *name;
        CBehaviour *Behaviour;
    };
    static QList<RegisteredBehaviour>* Behaviours;
    static bool inited;
};



#define DEF_BEHAVIOUR(behaviour) \
    behaviour();  \
	virtual ~behaviour(); \
    static const char *Name;\
    virtual CBehaviour* allocate(); \
    virtual void execute(); \
    virtual double probability(); \
    virtual QString getName(); \
    static  double probability(QList<CAgent*> agents)
#define INIT_BEHAVIOUR(Behaviour,name) \
    bool Behaviour##_registered \
            = CBehaviours::registerBehaviour(Behaviour::Name,new Behaviour); \
    CBehaviour* Behaviour::allocate() \
    {return new Behaviour;} \
    QString Behaviour::getName() {return QString(Name);} \
	double Behaviour::probability(QList<CAgent*> agents) {Behaviour b;b.agents.append(agents);double p=b.probability();return p;} \
    const char* Behaviour::Name = name


#endif // BEHAVIOUR_H
