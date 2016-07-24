#include "behaviour.h"
#include <role.h>

CBehaviour::CBehaviour()
{
}

CBehaviour::~CBehaviour()
{
}


CBehaviour* CBehaviour::allocate()
{
    return new CBehaviour;
}

QString CBehaviour::getName()
{
    return "";
}

void CBehaviour::updateIdleStatus()
{
    for (int i=0;i<agents.count();i++)
    {
        agents[i]->idle = false;
    }
}

void CBehaviour::execute()
{

}


double CBehaviour::probability()
{
    return 0;
}


//--------------------

QList<CBehaviours::RegisteredBehaviour>* CBehaviours::Behaviours;
QMap<QString, BehaviourValue> CBehaviours::values;
bool CBehaviours::inited=false;

CBehaviours::CBehaviours()
{
}

CBehaviours::~CBehaviours()
{
    delete Behaviours;
}
#include <QDebug>

void CBehaviours::loadValues(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        debug(QString("Could not Load behaviours: %1").arg(filename), D_ERROR, "red");
        qDebug() << "Could not load";
        return;
    }
	values.clear();
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        str = str.trimmed();
		if (str.length() > 0)
        {
            if (str[0] != '#')
            {
                QStringList b = str.split('=');
                if (b.length() == 2)
                {
                    QStringList v = b[1].split(',');
                    if (v.length() == 2)
                    {
                        double reward = v[0].trimmed().toDouble();
                        double penalty = v[1].trimmed().toDouble();
                        values[b[0].trimmed()].reward = reward;
                        values[b[0].trimmed()].penalty = penalty;
                    }
                }
            }
        }        
    }
    file.close();
}

bool CBehaviours::registerBehaviour(const char *name, CBehaviour* Behaviour)
{
    if (!inited)
    {
        Behaviours = new QList<CBehaviours::RegisteredBehaviour>;
        inited = true;
    }
    Behaviours->append((RegisteredBehaviour) { name, Behaviour });
    return true;
}

CBehaviour* CBehaviours::initBehaviour(const char *name)
{
    for (int i=0;i<Behaviours->size();i++)
        if (strcmp((*Behaviours)[i].name,name)==0)
            return (*Behaviours)[i].Behaviour->allocate();
    return NULL;
}

int CBehaviours::BehavioursCount()
{
    return Behaviours->count();
}

CBehaviour* CBehaviours::Behaviour(int i)
{
    if (i>=0 && i<BehavioursCount())
    {
        return (*Behaviours)[i].Behaviour;
    }
    return NULL;
}

BehaviourValue::BehaviourValue()
{
    reward  = 1.0;
    penalty = 0.0;
}
