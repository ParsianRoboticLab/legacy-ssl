#ifndef OPPONENT_H
#define OPPONENT_H

#include <worldmodel.h>

class COpponentRole
{
public:
    virtual double distance(CRobot* r) = 0;
    virtual QString name() = 0;   
    virtual int priority() = 0;
};

class COpponentDummy : public COpponentRole
{
public:
    virtual double distance(CRobot* r);
    virtual QString name();
    virtual int priority();
};


class COpponentGoalie : public COpponentRole
{
public:
    virtual double distance(CRobot* r);
    virtual QString name();
    virtual int priority();
};

class COpponentDefender : public COpponentRole
{
public:
    virtual double distance(CRobot* r);
    virtual QString name();
    virtual int priority();
};

class COpponentNearestToBall : public COpponentRole
{
public:    
    virtual double distance(CRobot* r);
    virtual QString name();
    virtual int priority();
};

class COpponentSupporter1 : public COpponentRole
{
public:
    virtual double distance(CRobot* r);
    virtual QString name();
    virtual int priority();
};

class COpponentSupporter2 : public COpponentRole
{
public:
    virtual double distance(CRobot* r);
    virtual QString name();
    virtual int priority();
};

class COpponent
{
private:    
    void registerOppRole(COpponentRole* role);    
    COpponentDummy dummy;
public:
    QList<COpponentRole*> roles;
    COpponent();    
    bool findRoles();
    void findMostDangerousSupporter( CRobot *);
};

extern COpponent* opponent;

#endif // OPPONENT_H
